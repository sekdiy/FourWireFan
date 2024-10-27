#include "Arduino.h"
#include <FourWireFan.h>  // https://github.com/sekdiy/FourWireFan

// set the measurement update period to 1s (1000 ms)
const unsigned long period = 1000;

// set the initial fan speed to slow
unsigned int fanPWM = 0;

// connect a fan to an analog and an interrupt pin (see notes on your Arduino model for pin numbers)
FourWireFan Fan;

// define an 'interrupt service handler' (ISR) to help count the tach pulses
void tachISR() {
    // let our Fan instance handle the actual pulse counting
    Fan.count();
}

void setup() {
    // prepare serial communication
    Serial.begin(115200);

    // connect the fan to pins 3 (PWM) and 2 (tach)
    Fan = FourWireFan(3, 2, tachISR);

    // sometimes initializing the gear generates some initial pulses that we want to ignore
    Fan.reset();
}

void loop() {
    // wait between output updates
    delay(period);

    // process the (possibly) counted ticks
    Fan.process(period);

    // output some measurement results
    Serial.println("Currently " + String(Fan.getRPM()) + " 1/min");

    // change fan speed
    // Fan.setPWM(++fanPWM % 100);
    Fan.setPWM(20);

    //
    // any other code can go here
    //
}
