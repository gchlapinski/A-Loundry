## Arduino Loundry Project

Attiny85 microcontoller connected with relay and humidity sensor. There is also one led (inidcator).

Note: Some pnp transistor is used to turn on the relay.

The circuit is powered with LiPo cell (TP4056 circuit is added to ease charging).

Sleep and Watchdog alert is implemented. The circuit sleeps 110 seconds (saving battery power) then it enables humidity sensor for 10 seconds. If humidity is to big relay turns on the vent (if relay is turned on the circuit will not start sleep mode).  

Led presents status of the device. If it is turned off the device is sleeping. It repeats 5 times the sequence 2 fast blink followed by pause when it measures humidity. When the relay is on led indicates the value of humidity, e.g. it blinks 6 times followed by pause if the humidity is between 60% and 69%.
