# Changelog
2023-02-04
- 16-bit timer removed, since it was causing troubles => didn't dig into it
- timeout implemented in the 8-bit timer ISR (the one also used for de-bouncing)
- ALARM state removed
- state machine map updated
- reciever.c updated
- settings.h updated
- comments cleanup
- locking will not be used
- NOTES moved from main.c to the end of this file


2023-01-27
- replaced switches with push buttons for Open and Close
- replaced L298 with a brand new VNH2SP30 module, soldered everything together, works like a charm
- updated schematics in Fritzing, exported in PNG and PDF, added to repo
- added second ATMEGA328P, wired for now and just OPEN_CMD implemented currently, just for testing. Seems to work,
  but there's an issue - it "freezes" in OPEN state after the MOTOR_OPEN_CMD is send over UART and the Close push
  button is not working. It starts to work again if I send multiple MOTOR_OPEN_CMD commands over UART - looks like 
  it doesn't land  in OPENING state for most of the times, have to investigate.
- another issue with solenoid. Idea is to use Timer0, compare vector B, but the current implementation is weird :/
  Commened out for now, have to think/try more.

2023-01-23
- git resolving conflicts stress :/

2023-01-17
- Smartgit installed, using branches for new, untested code (finally!)

2023-01-16
Lots of changes, fixes in the last week. Mainly:
- fixed the ALARM and LOCKED state transitions
- code formatting fixed
- tested logic with LEDs on outputs, where the motor and the solenoid will be connected
- tried to run the motor with some old L298, didn't work, as it's really ancient
- learned about power supplies, namely L7805, put together electronic parts to solder them in the future
- tried a spare ATX power supply, which provides 12 V and 5 V, works fine nd s powerful enough

2023-01-11
- added new states to the state machine (see PNG) for poor man's security
- updated drawio and PNG commited
- adjusted 16-bit timer for timeouts (256 pre-scaler instead of 1024, means 2 seconds instead of 8 seconds).
Because of this also extraTime comparison adjusted (from 1 (8 sec) to 5 (10 sec))
- implemented lock functions (draft): unlock() and lock() for Solenoid
- implement motor functions (draft): motorStop(), motorOpen(), motorClose()

2023-01-03
- re-mapped the button ports from DDRD to DDRC, because PD0 and PD1 will be used for RF over UART

2023-01-01
Happy new year ;)
- removed UART RF part to be able to clearly debug current issues
- removed LED_ON, LED_OFF states
- removed/replaced all _delay_ms functions
- cleansed the ALARM state
- added timer0 in CTC mode for button debouncing
- added debounce functionality
- tested state machine status transitions with LEDs (no motor yet)
- discussed door mechanism with father (engineer for life), agreed to switch to sliding door. It's much more simple, 
and there's enough space. He'll lead the mechanics department ;)

2022-12-29
- added UART RF part (receiver.c, transmitter.c)

2022-12-26
- moved definitions to header file
- added option to open/close the door during the closing/opening
- drawio schematic updated, new PNG exported

### TODO
- (maybe RF variable is not needed?) "extern" for RF variable? (because it's in multiple .c files) - https://www.avrfreaks.net/s/topic/a5C3l000000UljLEAS/t193424
- implement "Emergency stop" - pushbutton which changes a state to ALARM from any state. So stop motor, switch on red LED.
- (maybe) add additional switch when door is almost closed/open to trigger slower motor motion. Plus voltage divider with potentimeter ...
- test with current RF kit. If not working, buy a new one (the current one is old as hell)
- find a solution for photo-eye:
  + https://www.amazon.de/-/en/Receiver-Non-Modulator-Detection-KY-008-650nm-Transmitter/dp/B09TK119QX/ref=sr_1_9?crid=27KSHFT81FDYG&keywords=KY-008&qid=1671995727&sprefix=ky-008%2Caps%2C98&sr=8-9
  + https://www.build-electronic-circuits.com/ldr-circuit-diagram/
  + https://www.amazon.de/-/en/sourcing-2pairs-Infrared-Optical-Sensor/dp/B07L4LTFJY/ref=sr_1_15?crid=2OSN2XEZIOHKX&keywords=1+paar+fotoelektrisch&qid=1672079354&sprefix=1+pair+photoelectric%2Caps%2C94&sr=8-15

- door mechanism:
 + (obsolete) https://mechamechanisms.com/folding-door-controlled-by-cable
 + sliding door with "normal" door with lock to access buttons inside
 + electro magnetic key/lock with emergency manual release = Solenoid
- put uC to sleep (power-down), wake up with interrupt (button pressed) => may need to change pin settings for buttons, since now INT0 and INT1 are already used. Or use pin change interrupt.



# NOTES

/*
 * When we press the switch, the input pin is pulled to ground. Thus, we?re
 * waiting for the pin to go low.
 * The button is pressed when BUTTON1 bit is clear: if (!(PINB & (1<<BUTTON1)))
 
 
Debouncing is pretty straightforward, not sure why you're messing around so much.

Make it easy:
set up a timer irq that fires 200 times a sec
in the irq check switch A
if it is pressed:
	increment countA, limit the count to 40 max
else
	decrement countA, don't let it go below zero

Do the same thing for switch B (countB)
 
in your main code  if countA is > 20 then the switch is pressed & do what you want likewise for B
 */