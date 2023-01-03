# Changelog
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
- update state machine graph in Drawio
- implement "Emergency stop" - pushbutton which changes a state to ALARM from any state. So stop motor, switch on red LED.
- implement motor functions (motorStop, motorOpen, motorClose).
- (maybe) add additional switch when door is almost closed/open to trigger slower motor motion. Plus voltage divider with potentimeter ...
- test with current RF kit. If not working, buy a new one (the current one is old as hell)
- update transmitter.c with new commands (see receiver.c), change settings (F_CPU, address, SYNC)
- find a solution for photo-eye:
  + https://www.amazon.de/-/en/Receiver-Non-Modulator-Detection-KY-008-650nm-Transmitter/dp/B09TK119QX/ref=sr_1_9?crid=27KSHFT81FDYG&keywords=KY-008&qid=1671995727&sprefix=ky-008%2Caps%2C98&sr=8-9
  + https://www.build-electronic-circuits.com/ldr-circuit-diagram/
  + https://www.amazon.de/-/en/sourcing-2pairs-Infrared-Optical-Sensor/dp/B07L4LTFJY/ref=sr_1_15?crid=2OSN2XEZIOHKX&keywords=1+paar+fotoelektrisch&qid=1672079354&sprefix=1+pair+photoelectric%2Caps%2C94&sr=8-15

- door mechanism:
 + (obsolete) https://mechamechanisms.com/folding-door-controlled-by-cable
 + sliding door with "normal" door with lock to access buttons inside
 + electro magnetic key/lock with emergency manual release + implementation (lockRelease(), lockHold())
- put uC to sleep (power-down), wake up with interrupt (button pressed) => may need to change pin settings for buttons, since now INT0 and INT1 are already used. Or use pin change interrupt.
