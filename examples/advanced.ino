#include <FourWireFan.h>  // https://github.com/sekdiy/FourWireFan

// set the measurement update period to 1s (1000 ms)
const unsigned long period = 1000;

// declare the fan speed (and set default in %)
float fanPWM = 10.0f;

// (forward) declaration of 'interrupt service handler' routine (ISR)
void NFAFanISR();

// reference coefficients for fan 
float refRPM[10] = {220, 450, 720, 930, 1110, 1290, 1440, 1580, 1700, 1820};

// dynamic connection settings for fan
FourWireFanSettings* NFAFanSettings = new FourWireFanSettings(3, 2, RISING, &NFAFanISR, INPUT_PULLUP);

// dynamic control model for fan (using reference coefficients)
FourWireFanModel* NFAFanModel = new FourWireFanModel(10, 220, 100, 1820, true, refRPM);

// dynamic fan instance (using connection settings and control model)
FourWireFan* NFAFan = new FourWireFan(NFAFanSettings, NFAFanModel);

// definition of 'interrupt service handler' routine (ISR)
void NFAFanISR() {
    // let our NFAFan instance count the pulses
    NFAFan->count();
}

void setup() {
    // prepare serial communication
    Serial.begin(115200);

    // we could even recalibrate the fan by updating the model from within the application
    NFAFan->getModel()->setCoefficients(refRPM);
}

void loop() {
    // update fan speed
    NFAFan->setPWM(fanPWM);

    // wait between updates
    delay(period);

    // process the (possibly) counted ticks
    NFAFan->process(period);

    // output some measurement results
    Serial.println("Current speed: " + String(NFAFan->getRPM()) + " rpm (set point: " + String(NFAFan->getPWM()) + ", slippage: " + String(NFAFan->getSlippage()) + " %).");

    // change fan speed  
    fanPWM = (fanPWM + 1.0f);

    // roll over from fastest to slowest
    if (fanPWM - 100.0f > 0.0f)
     fanPWM = 10.0f;

    fanPWM = sin(millis() / PI) * 100.0f;
      
    //
    // any other code can go here
    //
}