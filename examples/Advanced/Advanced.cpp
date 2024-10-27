#include "Arduino.h"
#include <FourWireFan.h>  // https://github.com/sekdiy/FourWireFan

// set the measurement update period to 1s (1000 ms)
const unsigned long period = 1000;

// declare fan speed and define default value (in %)
float fanPWM = 10.0f;

// declare and define speed reference points for the fan model in use
uint16_t fanRPM[10] = {220, 450, 720, 930, 1110, 1290, 1440, 1580, 1700, 1820};

// declare 'interrupt service handler' routine (ISR), one per fan instance
void fanISR();

// dynamic connection settings for fan
FourWireFanSettings* FanSettings = new FourWireFanSettings(3, 2, &fanISR);

// dynamic control model for fan
FourWireFanModel* FanModel = new FourWireFanModel(10, 220, 100, 1820, 6000, fanRPM);

// dynamic fan instance
FourWireFan* Fan;

// implement 'interrupt service handler' routine (ISR), one per fan instance
void fanISR() {
    // let our Fan instance handle the actual pulse counting
    Fan->count();
}

void setup() {
    // prepare serial communication
    Serial.begin(115200);

    // get a (new) fan instance using connection settings and control model
    Fan = new FourWireFan(FanSettings, FanModel);

    // we could even recalibrate the fan by updating the model from within the application
    // NFAFan->getModel()->setCoefficients(fanRPM);
}

void loop() {
    // update fan speed
    Fan->setPWM(fanPWM);

    // wait between updates
    delay(period);

    // process the (possibly) counted ticks
    Fan->update(period);

    // output some measurement results
    Serial.println("Current speed: " + String(Fan->getRPM()) + " rpm (set point: " + String(Fan->getPWM()) +").");

    // change fan speed  
    fanPWM = (fanPWM + 1.0f);

    // roll over from fastest to slowest
    if (fanPWM - 100.0f > 0.0f)
     fanPWM = 10.0f;

    // alternative: harmonic oscillation
    // fanPWM = sin(millis() / PI) * 100.0f;
      
    //
    // any other code can go here
    //
}
