/* 
 * Wake-up light
 * Rodney Price
 */

/* Use -mmcu= flag to specify MCU */
#include <msp430.h>

#define PC r0                   /* program counter */
#define SP r1                   /* stack pointer */
#define SR r2                   /* status register */

#define interrupt(x) void __attribute__((interrupt (x))) 

/* Prototypes */
void initTimerA0(void);
void updateLights(void);
void blink(void);
static void __inline__ delay(register unsigned int);

/************************************************************************
 * Globals
 ************************************************************************/

/* Continuously running counters */
#define TICK_PERIOD 20          /* must be >= 3 */

/* Button de-bouncing parameters */
#define PRESS_THRESHOLD 0x000f
#define RELEASE_THRESHOLD 0xfff0
#define PWM_PERIOD 128

/* Button events */
#define PRESSED 1
#define RELEASED 2
#define LONGPRESS 4
static unsigned int evButton1 = 0; /* change flag for button 1 */
static unsigned int evButton2 = 0; /* change flag for button 2 */
static unsigned int evButton3 = 0; /* change flag for button 3 */
static unsigned int evButton4 = 0; /* change flag for button 4 */

/* Button state */
static unsigned int srButton1 = 0xffff; /* shift register for button 1 */
static unsigned int srButton2 = 0xffff; /* shift register for button 2 */
static unsigned int srButton3 = 0xffff; /* shift register for button 3 */
static unsigned int srButton4 = 0xffff; /* shift register for button 4 */
static unsigned int dbButton1 = 0;    /* debounced state for button 1 */
static unsigned int dbButton2 = 0;    /* debounced state for button 2 */
static unsigned int dbButton3 = 0;    /* debounced state for button 3 */
static unsigned int dbButton4 = 0;    /* debounced state for button 4 */

/* Clock events */
#define DAY 8                    /* the day just changed */
#define HOUR 4                   /* the hour just changed */
#define MINUTE 2                 /* the minute just changed */
#define SECOND 1                 /* the second just changed */
#define TICK 16                  /* the timer tick just changed */
static unsigned int evClock = 0; /* change flag for the clock */

/* Clock state */
typedef struct {
  int ticks;
  unsigned int seconds;
  unsigned int minutes;
  unsigned int hours;
} time_t;
static time_t time;             /* current time */
static time_t alarm;            /* time when lights begin to brighten */

/* System state */
#define ALARM 0                 /* lights are off until the next alarm*/
#define ON 1                    /* lights are on continuously */
static unsigned int state = ALARM;
static unsigned int ostate = ALARM;

/* Light state */
#define NIGHT 3                 /* lights are at dimmest level */
#define DIM 2                   /* lights are dim */
#define NORMAL 1                /* lights are at normal intensity */
#define BRIGHT 0                /* lights are on all the way */
static unsigned int bright  = BRIGHT;

/* Color state */
#define BLUE 1                  /* blue lights are on */
#define WHITE 2                 /* white lights are on */
static unsigned int color  = BLUE | WHITE;

#define INTENSITY_ARRAY_END 179
static int tocks = INTENSITY_ARRAY_END; /* counter for ALARM state */
const unsigned int standard[]  = {83, 59, 35, 11};
const unsigned int intensity[] =
    { 0,   1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,
      2,   2,   2,   3,   3,   3,   3,   3,   3,   4,   4,   4,
      4,   4,   5,   5,   5,   6,   6,   6,   7,   7,   8,   8,
      8,   9,  10,  10,  11,  11,  12,  13,  13,  14,  15,  16,
     17,  18,  19,  20,  21,  23,  24,  25,  27,  29,  30,  32,
     34,  36,  38,  40,  43,  45,  48,  51,  54,  57,  60,  64,
     68,  72,  76,  81,  85,  91,  96, 102, 108, 114, 121, 128,
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,  
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,  
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,  
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,  
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,  
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,  
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,  
     96,  64,  48,  32,  24,  16,  12,   8,   4,   2,   1,   0
    };


int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;     /* stop watchdog timer */
  WDTCTL = WDT_ADLY_1000;       /* make watchdog an interval timer */
  IE1 |= WDTIE;

  /* Basic clock system configuration */
  DCOCTL  = CALDCO_1MHZ;
  BCSCTL1 = CALBC1_1MHZ;
  BCSCTL2 = SELM_0 | DIVM_0 | DIVS_0;
  BCSCTL3 = XCAP1;			// 10 pF crystal caps

  initTimerA0();

  P1OUT = 0;
  /* P1.0 set to ACLK to verify watch crystal oscillation */
  P1DIR  |=  BIT0;
  P1SEL  |=  BIT0;
  P1SEL2 &= ~BIT0;
  /* P1.1 is TXD */
  P1DIR  |=  BIT1;
  P1SEL  &= ~BIT1;
  P1SEL2 &= ~BIT1;
  /* P1.2 is RXD */
  P1DIR  &= ~BIT2;
  P1SEL  &= ~BIT2;
  P1SEL2 &= ~BIT2;
  /* P1.3 is button 1 (upper left) */
  P1OUT  |=  BIT3;
  P1REN  |=  BIT3;
  P1DIR  &= ~BIT3;
  P1SEL  &= ~BIT3;
  P1SEL2 &= ~BIT3;
  /* P1.4 set to TA0.2 is blue light on */
  P1OUT  |=  BIT4;
  P1DIR  |=  BIT4;
  P1SEL  |=  BIT4;
  P1SEL2 |=  BIT4;
  /* P1.5 is button 3 (lower left) */
  P1OUT  |=  BIT5;
  P1REN  |=  BIT5;
  P1DIR  &= ~BIT5;
  P1SEL  &= ~BIT5;
  P1SEL2 &= ~BIT5;
  /* P1.6 set to TA0.1 is white light on */
  P1OUT  &= ~BIT6;
  P1DIR  |=  BIT6;
  P1SEL  |=  BIT6;
  P1SEL2 &= ~BIT6;
  /* P1.7 is button 2 (middle left) */
  P1OUT  |=  BIT7;
  P1REN  |=  BIT7;
  P1DIR  &= ~BIT7;
  P1SEL  &= ~BIT7;
  P1SEL2 &= ~BIT7;

  /* P2.0 - 2.2 are unused */
  P2OUT = 0;
  P2DIR  |=   BIT0 | BIT1 | BIT2;
  P2SEL  &= ~(BIT0 | BIT1 | BIT2);
  P2SEL2 &= ~(BIT0 | BIT1 | BIT2);
  /* P2.3 and P2.5 run the alarm indicator LED */
  P2OUT  &= ~(BIT3 | BIT5);
  P2DIR  |=   BIT3 | BIT5;
  P2SEL  &= ~(BIT3 | BIT5);
  P2SEL2 &= ~(BIT3 | BIT5);
  /* P2.4 is button 4 (far left) */
  P2OUT  |=   BIT4;
  P2REN  |=   BIT4;
  P2DIR  &=  ~BIT4;
  P2SEL  &=  ~BIT4;
  P2SEL2 &=  ~BIT4;
  /* P2.6 - 2.7 are XIN and XOUT */
  P2DIR  &= ~(BIT6 | BIT7);
  P2SEL  |=   BIT6 | BIT7;
  P2SEL2 &= ~(BIT6 | BIT7);

  /* Initialize the real-time clock and alarm */
  time.ticks = 3327;
  time.seconds = 0;
  time.minutes = 29;
  time.hours = 18;
  alarm.ticks = 1170;
  alarm.seconds = 10;
  alarm.minutes = 30;
  alarm.hours = 06;

  /* Event loop */
  while(1) {
    __bis_SR_register(LPM3_bits + GIE);

    /* Button 1 turns the lights on and off. */
    if (evButton1 & PRESSED) {
      if (state == ON) {
        /* Lights out if lights were on */
        state = ALARM;
        tocks = INTENSITY_ARRAY_END;
      } else if (tocks < INTENSITY_ARRAY_END) {
        /* Lights out if alarm is running */
        tocks = INTENSITY_ARRAY_END;
      } else {
        /* Lights on */
        state = ON;
      }
      updateLights();
      evButton1 = 0;
    }

    /* Button 2 sets the maximum brightness. */
    if (evButton2 & PRESSED) {
      /* Show the current brightness value. */
      ostate = state;
      state = ON;
      updateLights();
      evButton2 = 0;
    }
    if (evButton2 & LONGPRESS) {
      /* Set a new brightness value. */
      switch (bright) {
      case NIGHT:
        bright = DIM;
        break;
      case DIM:
        bright = NORMAL;
        break;
      case NORMAL:
        bright = BRIGHT;
        break;
      case BRIGHT:
        bright = NIGHT;
        break;
      }
      updateLights();
      evButton2 = 0;
    }
    if (evButton2 & RELEASED) {
      /* Restore the system state. */
      state = ostate;
      updateLights();
      evButton2 = 0;
    }

    /* Button 3 sets the color. */
    if (evButton3 & PRESSED) {
      /* Show the current color value. */
      ostate = state;
      state = ON;
      updateLights();
      evButton3 = 0;
    }
    if (evButton3 & LONGPRESS) {
      /* Set a new color value. */
      switch (color) {
      case 0:
        color = BLUE;
        break;
      case BLUE:
        color = WHITE;
        break;
      case WHITE:
        color = BLUE | WHITE;
        break;
      case BLUE | WHITE:
        color = BLUE;
        break;
      }
      updateLights();
      evButton3 = 0;
    }
    if (evButton3 & RELEASED) {
      /* Restore the system state. */
      state = ostate;
      updateLights();
      evButton3 = 0;
    }

    /* Button 4 "arms" the alarm */
    if (evButton4 & PRESSED) {
      P2OUT ^= BIT3;
      evButton4 = 0;
    }

    /* Real-time clock */
    if (evClock & SECOND) {
      if ((alarm.seconds == time.seconds) && 
          (alarm.minutes == time.minutes) && 
          (alarm.hours   == time.hours)) {
        /* Start the timer for the lights */
        if ((state == ALARM) && (P2OUT & BIT3)) { /* fix: incorporate alarm indicator into state variable */
          tocks = 0;
          updateLights();
          evClock = 0;
        }
      }
    }
    if (evClock & TICK) {
      if (tocks < INTENSITY_ARRAY_END) {
        tocks++;
        updateLights();
      }
      evClock = 0;
    }
  }
}

void initTimerA0() {
  TA0CTL   = TASSEL_1		// use ACLK
           | ID_0			// divide by 1
           | MC_1   		// up mode
           | TACLR;         // start counter from 0
  TA0CCTL0 = OUTMOD_4;	    // 50% duty cycle
  TA0CCR0  = PWM_PERIOD;    // 256 Hz update rate
  TA0CCTL1 = OUTMOD_7;	    // set/reset PWM mode
  TA0CCR1  = 0;	            // white lights off
  TA0CCTL2 = OUTMOD_7;	    // set/reset PWM mode
  TA0CCR2  = 0;	            // blue lights off
  TA0CTL  |= TAIE;          // turn on interrupts
}

void blink() {
  /* blink the lights */
  unsigned int tmp1, tmp2;

  tmp1 = TA0CCR1;
  tmp2 = TA0CCR2;
  if (color & WHITE)
    TA0CCR1 = intensity[standard[bright]];
  if (color & BLUE)
    TA0CCR2 = intensity[standard[bright]];
  delay(40000);
  TA0CCR1 = tmp1;
  TA0CCR2 = tmp2;
}

void updateLights() {
  /* Update the blue lights */
  if (color & BLUE) {
    /* System state is not OFF and color blue is on */
    if (state) 
      TA0CCR2 = intensity[ standard[bright] ];
    else
      TA0CCR2 = intensity[tocks] >> (2*bright);
  } else {
    TA0CCR2 = 0;
  }
  /* Update the white lights */
  if (color & WHITE) {
    /* System state is not OFF and color white is on */
    if (state) 
      TA0CCR1 = intensity[ standard[bright] ];
    else
      TA0CCR1 = intensity[tocks] >> (2*bright);
  } else {
    TA0CCR1 = 0;
  }
}


unsigned int 
test_button(unsigned int *srButton, 
            unsigned int *dbButton, 
            unsigned int *duration,
            unsigned int bit) {
  /* Shift state of button into its shift register */
  *srButton >>= 1;
  if (((P1IN & bit & 0x00ff) && (BIT8 & bit)) || 
      ((P2IN & bit & 0x00ff) && (BIT9 & bit)))
    *srButton |= BITF;
  if (*dbButton) {
    /* Button state is down */
    if (*srButton >= RELEASE_THRESHOLD) {
      /* Low to high transition */
      *dbButton = 0;
      return RELEASED;
    }
    if (*srButton <= PRESS_THRESHOLD) {
      /* Look for long press event */
      (*duration)++;
      if ((*duration & 0x00ff) == 0)
        return (*duration & 0xff00) | LONGPRESS;
    } else {
      /* Button is still bouncing */
      *duration = 0;
      return 0;
    }
  } else {
    /* Button state is up */
    if (*srButton <= PRESS_THRESHOLD) {
    /* High to low transition */
      *duration = 0;
      *dbButton = 1;
      return PRESSED;
    }
  }
  /* No change */
  return 0;
}

interrupt(TIMER0_A1_VECTOR) debouncer() {
  /* Check button state every 4 ms */
  static unsigned int duration1 = 0;
  static unsigned int duration2 = 0;
  static unsigned int duration3 = 0;
  static unsigned int duration4 = 0;
  switch (TAIV) {
  case 0xA:
    evButton1 = test_button(&srButton1, &dbButton1, &duration1, BIT8 | BIT7);
    evButton2 = test_button(&srButton2, &dbButton2, &duration2, BIT8 | BIT5);
    evButton3 = test_button(&srButton3, &dbButton3, &duration3, BIT8 | BIT3);
    evButton4 = test_button(&srButton4, &dbButton4, &duration4, BIT9 | BIT4);
    if ( evButton1 || evButton2 || evButton3 || evButton4 )
      __bic_SR_register_on_exit(LPM3_bits);
    break;
  }
}

interrupt(WDT_VECTOR) WDT_ISR(void) {
  static int ticker = TICK_PERIOD;
  /* Update timer ticks */
  evClock = 0;
  ticker--;
  if (ticker <= 0) {
    ticker = TICK_PERIOD;
    time.ticks++;
    evClock |= TICK;
  }
  /* Update seconds, minutes, hours */
  time.seconds += 1;
  evClock |= SECOND;
  if (time.seconds == 60) {
    time.seconds = 0;
    time.minutes += 1;
    evClock |= MINUTE;
    if (time.minutes == 60) {
      time.minutes = 0;
      time.hours += 1;
      evClock |= HOUR;
      if (time.hours == 24) {
        time.hours = 0;
        time.ticks = 0;
        evClock |= DAY | TICK;
      }
    }
  }
  __bic_SR_register_on_exit(LPM3_bits);
}

// Delay Routine from mspgcc help file
static void __inline__ delay(register unsigned int n)
{
  __asm__ __volatile__ (
  "1: \n"
  " dec %[n] \n"
  " jne 1b \n"
        : [n] "+r"(n));
}

