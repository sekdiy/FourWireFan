#include <FourWireFan.h>  // https://github.com/sekdiy/FourWireFan

// connect a fan to an interrupt pin (see notes on your Arduino model for pin numbers)
FourWireFan Fan = FourWireFan();

// set the measurement update period to 1s (1000 ms)
const unsigned long period = 1000;
unsigned int fanPWM = 0;
unsigned int fanPIN = 2;

// define an 'interrupt service handler' (ISR) for every interrupt pin you use
void FanISR() {
    // let our Fan instance count the pulses
    Fan.count();
}

void setup() {
    // prepare serial communication
    Serial.begin(9600);

    // enable a call to the 'interrupt service handler' (ISR) on every falling edge at the interrupt pin
    // do this setup step for every ISR you have defined, depending on how many interrupts you use
    pinMode(fanPIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(fanPIN), FanISR, FALLING);

    // sometimes initializing the gear generates some pulses that we should ignore
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
    Fan.setPWM(++fanPWM % 100);

    //
    // any other code can go here
    //
}
