# Changelog
2022-12-26
- moved definitions to header file
- added option to open/close the door during the closing/opening
- drawio schematic updated, new PNG exported

### TODO
- check if ALARM state is now "accessible" (after declaring volatile variable).
- implement "Emergency stop" - pushbutton which changes a state to ALARM from any state. So stop motor, switch on red LED.
- implement motor functions (motorStart, motorStop, motorOpenDirection, motorCloseDirection).
- (maybe) add additional switch when door is almost closed/open to trigger slower motor motion. Plus voltage divider with potentimeter ...
- find a solution for photo-eye:
  +  https://www.amazon.de/-/en/Receiver-Non-Modulator-Detection-KY-008-650nm-Transmitter/dp/B09TK119QX/ref=sr_1_9?crid=27KSHFT81FDYG&keywords=KY-008&qid=1671995727&sprefix=ky-008%2Caps%2C98&sr=8-9
  + https://www.build-electronic-circuits.com/ldr-circuit-diagram/
  + https://www.amazon.de/-/en/sourcing-2pairs-Infrared-Optical-Sensor/dp/B07L4LTFJY/ref=sr_1_15?crid=2OSN2XEZIOHKX&keywords=1+paar+fotoelektrisch&qid=1672079354&sprefix=1+pair+photoelectric%2Caps%2C94&sr=8-15

- door mechanism: https://mechamechanisms.com/folding-door-controlled-by-cable
- put uC to sleep (power-down), wake up with interrupt (button pressed) => may need to change pin settings for buttons, since now INT0 and INT1 are already used. Or use pin change interrupt.
