/**
 * Four Wire Fan
 *
 * An Arduino four-wire fan library that provides a PWM speed and tachometer interface.
 *
 * @author sekdiy (https://github.com/sekdiy/FourWireFan)
 * @date 22.07.2020 Initial release.
 * @version See git comments for changes.
 */

#include "Arduino.h"
#include "FourWireFan.h"    // https://github.com/sekdiy/FourWireFan
#include <math.h>

/** 
 * Simple constructor for a four wire fan
 * 
 * @since 2020-07-22
 * 
 * @param pwmPin The output pin where the fan's PWM signal input is connected (default: 3)
 * @param tachPin The input pin where the fan's tachometer signal output pin is connected (default: 2)
 * @param tachISR Tachometer interrupt service routine (handler function) callback reference (default: none)
 */
FourWireFan::FourWireFan(uint8_t pwmPin, uint8_t tachPin, void (*tachISR)(void))
{
    this->_settings = new FourWireFanSettings(pwmPin, tachPin, tachISR);  // default settings with supplied pwmPin
    this->_model = new FourWireFanModel();              // default fan model

    this->begin(); 
}

/**
 * Extended constructor for a four wire fan (requires prepared configuration)
 * 
 * @since 2020-07-22
 * 
 * @param settings The connection settings of a four wire fan. 
 * @param model The properties of a four wire fan.
 */
FourWireFan::FourWireFan(FourWireFanSettings* settings, FourWireFanModel* model) :
    _settings(settings),
    _model(model)
{
    this->setup();
    this->reset();
}

/**
 * initial internal pin setup
 * 
 * @since 2020-07-22
 */
void FourWireFan::setup()
{
    // analogWriteResolution(8);
    pinMode(this->_settings->tachPin, this->_settings->tachPU);
    attachInterrupt(digitalPinToInterrupt(this->_settings->tachPin), this->_settings->tachISR, this->_settings->tachMode);
}

/**
 * Processes fan speed from tachometer pulse input.
 * 
 * @since 2020-07-22
 * 
 * @param duration The duration of the measuring period since last call to `process()` in ms
 */
void FourWireFan::process(unsigned long duration)
{
    unsigned long pulses;

    /* sample */
    noInterrupts();                                 //!< going to change interrupt variable(s)
    pulses = this->_pulses;                         //!< save pulses counted during last current processing duration
    this->_pulses = 0;                              //!< reset pulse counter after successfully sampling it
    interrupts();                                   //!< never forget!

    /* normalise */
    // double seconds = (double) duration / 1000.0f;   //!< normalised duration (in s, i.e. per 1000ms)
    // double frequency = (double) pulses / seconds;   //!< normalised frequency (in 1/s)

    // /* calculate */
    // double rpm = frequency * 60.0f / 2.0f;          //!< convert from 1/s to 1/min and accont for two (2) pulses per revolution

    // /* store new rpm value */
    // this->_rpm = (unsigned long) rpm;               //!< explicit conversion from double to unsigned long

    /* alternatively: calculate, normalise and store new rpm value in one go */
    this->_rpm = (unsigned long) pulses * 60.0f / 2.0f / (duration / 1000.0f);  //!< convert two pulses per revolution to revolutions per minute and store result
}

/**
 * Resets measurement values (only)
 * 
 * @since 2020-07-22
 */
void FourWireFan::reset() 
{
    noInterrupts();                                 //!< going to change interrupt variable(s)
    this->_pulses = 0;                              //!< reset pulse counter
    interrupts();                                   //!< never forget!

    this->_rpm = 0;                                 //!< explicitly reset speed value
}

/**
 * Increments the internal pulse counter (and serves as interrupt callback routine).
 * 
 * This function should be called from an interrupt service routine (ISR).
 * 
 * @since 2020-07-22
 */
void FourWireFan::count()
{
    uint32_t now = micros();
    uint32_t interval = now - this->_blink;

    // debouncing (optional, if interval is longer than debounce timeout)
    if (interval >= this->_settings->tau) {
        this->_pulses++;
    }
    else {
        this->_blink = now;
    }
}

/**
 * Returns calculated RPM (i.e. fan speed).
 * 
 * @since 2020-07-22
 * 
 * @return unsigned long
 */
unsigned long FourWireFan::getRPM()
{
    return this->_rpm;
}

/**
 * Returns calculated load from current slippage (requires proper model values).
 * 
 * @since 2020-07-22
 * @todo rescale
 * 
 * @return float
 */
float FourWireFan::getSlippage()
{
    // get expected load for current rpm from model
    // get slippage between expected and actual rpm
    // derive load from slippage

    
    // float normalisedRPM = (float) this->_rpm / (float) this->_model.maxRPM;
    // float normalisedPWM = (float) this->_pwm / (float) this->_model.maxPWM;

    // float scaledRPM = (float) this->_rpm / (float) this->_model.maxRPM - (float) this->_model.minRPM;

    // float maxRPMcoefficient = (float) this->_model.maxRPM / (float) this->_model.maxPWM;
    // float minRPMcoefficient = (float) this->_model.minRPM / (float) this->_model.minPWM;
    // float currentRPMcoefficient = (float) this->_rpm / (float) this->_pwm;

    // float scaledRPMcoefficient = (maxRPMcoefficient - minRPMcoefficient) * currentRPMcoefficient;

    // float currentSlippage = currentRPMcoefficient / minRPMcoefficient;

    // return 100 * (1 - currentSlippage);

    float totalRPMspan = this->_model->maxRPM - this->_model->minRPM;
    float upperRPMspan = max(this->_model->maxRPM, this->_rpm) - this->_rpm;
    float lowerRPMspan = this->_rpm - min(this->_model->minRPM, this->_rpm);

    float totalPWMspan = this->_model->maxPWM - this->_model->minPWM;
    float upperPWMspan = max(this->_model->maxPWM, this->_pwm) - this->_pwm;
    float lowerPWMspan = this->_pwm - min(this->_model->minPWM, this->_pwm);

    float rpmRatio = lowerRPMspan / totalRPMspan;
    float pwmRatio = lowerPWMspan / totalPWMspan;

    return pwmRatio;
}

/**
 * Returns current debounce time constant.
 * 
 * @since 2020-08-12
 * 
 * @return unsigned int
 */
unsigned int FourWireFan::getDebounceTime()
{
    return this->_settings->tau;
}

/**
 * Updates debounce time constant.
 * 
 * @since 2020-08-12
 * 
 * @param tau The time constant to set
 * @return FourWireFan*
 */
FourWireFan* FourWireFan::setDebounceTime(unsigned long tau)
{
    this->_settings->tau = tau;

    return this;
}

/**
 * Returns current PWM set point.
 * 
 * @since 2020-07-22
 * 
 * @return float
 */
float FourWireFan::getPWM()
{
    return this->_pwm;
}

/**
 * Updates PWM set point.
 * 
 * @since 2020-07-22
 * 
 * @param pwm new target PWM value (i.e. speed)
 * @return FourWireFan*
 */
FourWireFan* FourWireFan::setPWM(float pwm) 
{
    this->_pwm = max(min(pwm, this->_model->maxPWM), this->_model->minPWM);

    analogWrite(this->_settings->pwmPin, 2.55f * this->_pwm);

    return this;
}

/**
 * Returns current four wire fan model.
 * 
 * @since 2020-07-22
 * 
 * @return FourWireFanModel*
 */
FourWireFanModel* FourWireFan::getModel() 
{
    return this->_model;
}

/**
 * Updates four wire fan model.
 * 
 * @since 2020-07-22
 * 
 * @param model The new model to set.
 * @return FourWireFan*
 * 
 * @todo sanity check best done in model class...
 */
FourWireFan* FourWireFan::setModel(FourWireFanModel* model) 
{
    if ((model->minPWM >= 0) && (model->maxPWM <= 100) && (model->maxPWM > model->minPWM)) {
        this->_model = model;
    }

    return this;
}

/**
 * Pre-defined four wire fan settings instances.
 */
FourWireFanSettings const DefaultFanSettings = FourWireFanSettings(); // Default four wire fan settings instance.

/**
 * Pre-defined four wire fan model instances.
 */
FourWireFanModel DefaultFanModel = FourWireFanModel();          //!< Default four wire fan model instance.
