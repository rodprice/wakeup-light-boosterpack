wakeup-light-boosterpack
========================

A boosterpack for Texas Instruments' MSP430 Launchpad platform.

My body doesn't seem to be designed to get out of bed without loud
complaints.  This is especially true in the winter months, when I have
to be out of bed before the sun comes up.  This boosterpack is
designed to help, by tickling light-sensitive cells in the eye that
tell the body to wake up.

Blue light wakes you up?
------------------------

It
[seems](http://gizmodo.com/5701003/blue-lighting-wakes-schoolkids-up)
[so](http://www.dailymail.co.uk/sciencetech/article-1333029/Blue-lighting-trialled-British-school-wake-drowsy-pupils-thing-morning.html)
[1].  In the past two or three years, scientists have found that, in
addition to the rods and cones, the eye has a [third type of
photoreceptor](http://www.nature.com/news/2011/110119/full/469284a.html
"Vision sciene: Seeing without seeing") [2].  Among other things,
these cells send the brain signals about when to sleep and when to
wake up.  They respond to blue light, the bright blue from the sky
that surrounds us outdoors on a sunny day.  They take "rise and shine"
literally: the sun is up, the sky is blue, so wake up!

This boosterpack lets you experiment with adding a little blue light
into your life, even if you are asleep in bed before sunrise, or
working in a room with thick walls separating you from that lovely
blue sky.  It mounts 16 high-intensity LEDs, eight white and eight
blue, which are controlled with an MSP430 Launchpad.  The intensity of
the white lights is controlled with the TA0.1 PWM pin, while the
intensity of the blue lights is controlled with the TA0.2 PWM pin of
the MSP430G2452 chip.

How it works
------------

Code [hosted on
Github](https://github.com/rodprice/wakeup-light-boosterpack) turns
the boosterpack/Launchpad combination into a visual alarm clock of
sorts.  When the alarm "goes off," the light gradually begins to
increase in intensity over the space of about half an hour.  It stays
on for another half hour, long enough for you to get out of bed if
you're going to, then turns itself off.  Four buttons along the top of
the boosterpack allow you to control the lights.  "On/off" does just
what it says: it toggles the lights on and off.  This is what you use
in the morning if you want to turn the lights off without a lot of
fuss.  Holding down the "bright" button goes through a sequence of
four brightness levels.  Release the button when you get to the
maximum brightness level you are comfortable with.  This is the
maximum intensity you will see in the morning.  The "color" button
works similarly: hold it down to cycle through blue light only, white
light only, or both.  Finally, the "alarm" button toggles the alarm
indicator, a small green LED, on and off.  If the indicator light is
on, the alarm will go off in the morning.

The boosterpack gets its power from a 18V wall-mounted plugin power
supply (wall wart).  This powers four strings of four LEDs each.  Each
LED string is controlled with a very simple constant-current sink
built from two general-purpose NPN transistors.  A voltage regulator
on the board generates Vcc at 3.6V for the Launchpad, so you don't
have to keep the Launchpad plugged into a USB port for power.  Just
plug the Launchpad into the boosterpack, plug the wall wart into an
outlet, and connect the two, and you're set.

Except for one thing: setting the time for the alarm to go off.  I've
kept the pinout of the boosterpack compatible with the [43oh OLED
display](http://store.43oh.com/index.php?route=product/product&path=64&product_id=57)
("The Terminal") available at 43oh.com.  You should be able to stack
the OLED boosterpack on top of this one, and use the two buttons on
the OLED boosterpack to set the time and alarm.  I should warn you
that I haven't done this, however.  You'll have to write the code
yourself.

I set the time and alarm myself using the Launchpad USB port, writing
out the time directly in the MSP430 code.  A bit crude, I know, but
this is just experimentation, after all.  I have included a battery
backup in the boosterpack, so that the Launchpad can maintain enough
power to remember the time even when the wall wart is unplugged.  That
way, you can plug the Launchpad/boosterpack combination into your
computer's USB port, set the time and alarm, unplug it, carry it into
your bedroom, and plug it back in without losing the time.  The four
buttons on the boosterpack will give you enough control after that.

Experiences
-----------

I set this boosterpack up in my 8-year-old's bedroom during the month
of February.  The first two mornings, my normally somnolent son
bounded out of bed 15 minutes early, heading right into Mom and Dad's
bedroom.  Since then, it continues to help my son out of bed... just
not in such a forceful fashion.

Your mileage may vary.

A word of caution: blue light isn't always good for you.  Too much,
especially in the absence of other light, can
[damage your eyes](http://www.mdsupport.org/library/hazard.html) [3].
(Be especially careful if you have had cataract surgery.)  At the
wrong time of day, it will mess up your
[circadian rhythm](http://abclocal.go.com/ktrk/story?section=news/health&id=8779461)
(body clock) [4].  And if you have a tendency to migraine headaches,
you
[might not want to use this light](http://news.sciencemag.org/sciencenow/2010/01/11-01.html?etoc
"Why light makes migraines worse") [5] at all.  If in doubt, consult
your doctor.

1. [Blue lighting wakes schoolkids up](http://gizmodo.com/5701003/blue-lighting-wakes-schoolkids-up)
2. [Vision science: Seeing without seeing](http://www.nature.com/news/2011/110119/full/469284a.html)
3. [Artificial Lighting and the Blue Light Hazard](http://www.mdsupport.org/library/hazard.html)
4. [Blue light disrupts ability to sleep, research shows](http://abclocal.go.com/ktrk/story?section=news/health&id=8779461)
5. [Why light makes migraines worse](http://news.sciencemag.org/sciencenow/2010/01/11-01.html?etoc)
6. [Melanopsin](http://en.wikipedia.org/wiki/Melanopsin)

