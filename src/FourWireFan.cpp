/**
 * Four Wire Fan
 *
 * A four-wire fan driver that provides a PWM speed and tachometer interface.
 *
 * @author sekdiy (https://github.com/sekdiy/FourWireFan)
 * @date 22.07.2020 Initial release.
 * @version See git comments for changes.
 */

#include <math.h>
#include "Arduino.h"
#include "FourWireFan.h"    // https://github.com/sekdiy/FourWireFan

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
    this->begin();
}

/**
 * Setup physical connection and clear pulse counter
 * 
 * @since 2024-10-18
 */
void FourWireFan::begin()
{
    this->setup();                                  // connect and attach
    this->reset();                                  // clear pulses from 
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

    noInterrupts();                                 // going to change interrupt variable(s)
    attachInterrupt(digitalPinToInterrupt(this->_settings->tachPin), this->_settings->tachISR, this->_settings->tachMode);
    interrupts();                                   // never forget!
}

/**
 * Resets measurement values (only)
 * 
 * @since 2020-07-22
 */
void FourWireFan::reset() 
{
    noInterrupts();                                 // going to change interrupt variable(s)
    this->_blink = 0;                               // reset last debouncing interval
    this->_pulses = 0;                              // reset pulse counter
    interrupts();                                   // never forget!
    
    this->_spinup = 0;                              // explicitly stop spinup…
    this->_rpm = 0;                                 // …and reset speed value
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
 * Updates fan operation, e.g. spinning up, tachometer input, speed update, spindown detection.
 * 
 * @since 2020-07-22
 * 
 * @param duration The length of the measuring period since the last call to `update()` (in ms)
 */
void FourWireFan::update(uint16_t duration)
{
    uint32_t pulses;
    uint8_t targetPWM = this->_model->maxPWM;       // default to maximum fan speed (as a safety measure!)

    /* sample tachometer value */
    noInterrupts();                                 // going to change interrupt variables
    pulses = this->_pulses;                         // save pulses counted during duration
    this->_pulses = 0;                              // reset pulse counter after successfully sampling it
    this->_blink = 0;                               // reset debouncing interval so as not to bleed over into next interval
    interrupts();                                   // never forget!

    /**
     * Two impulses per revolution are converted to revolutions per minute.
     * The algorithm is very simple and assumess that any four wire fan adheres to at least the original Intel specification.
     * At this point most of the actual work has already been done via the `count()` method and its debouncing/filtering action.
     * 
     * // normalise:
     * double seconds = (double) duration / 1000.0f;   // normalised duration (in s, i.e. per 1000ms)
     * double frequency = (double) pulses / seconds;   // normalised frequency (in 1/s)
     * 
     * // calculate:
     * double rpm = frequency * 60.0f / 2.0f;          // convert from 1/s to 1/min and accont for two pulses per revolution
     * 
     * // store new rpm value:
     * this->_rpm = (uint32_t) rpm;                    // explicit conversion from double to unsigned long
     * 
     * @see "4-Wire Pulse Width Modulation (PWM) Controlled Fans", Intel Corporation September 2005, revision 1.3 
     * @see "Noctua PWM specifications white paper", www.noctua.at
     */

    /* alternatively: calculate, normalise and store new rpm value in one go */
    this->_rpm = (uint32_t) pulses * 60.0f / 2.0f / (duration / 1000.0f);  // (two) pulses per revolution to revolutions per minute

    /* detect spindown */
    if ((this->_rpm <= this->_model->minRPM) &&     // motor not faster than minimum speed
        (this->_pwm >= this->_model->minPWM)) {     // *and* set point higher than that?
        this->_spinup = this->_model->spinup;       // apply motor spin
    }

    /* handle spinup (see: https://en.wiktionary.org/wiki/percussive_maintenance) */
    if ((0 < this->_spinup) &&                      // spinup condition is met
        (this->getRPM() >= this->_model->minRPM)) { // *and* fan has started to move?
        // reduce remaining spinup duration (down to zero)
        this->_spinup -= duration;
        // in case the fan doesn't show signs of movement yet, keep trying…
    } else {
        // spinup condition not met (a.k.a. normal operation)
        targetPWM = this->_pwm;
    }

    /* update speed set point */
    analogWrite(this->_settings->pwmPin, 2.55f * targetPWM);
}

/**
 * Returns calculated RPM (i.e. fan speed).
 * 
 * @since 2020-07-22
 * @todo error repporting
 * 
 * @return uint32_t
 */
uint32_t FourWireFan::getRPM()
{
    return this->_rpm;
}

/**
 * Updates PWM (duty cycle) according to RPM (i.e. fan speed) via fan model.
 * 
 * @since 2024-10-06
 * 
 * @param rpm The new target rpm value (i.e. speed)
 * 
 * @return FourWireFan*
 */
FourWireFan* FourWireFan::setRPM(uint32_t rpm)
{
    // required pwm for a given rpm via fan model lookup
    uint32_t pwm = 0; // todo: implement magic here

    this->setPWM(pwm);

    return this;
}

/**
 * Returns current debounce time constant.
 * 
 * @since 2020-08-12
 * 
 * @return uint32_t
 */
uint32_t FourWireFan::getDebounceTime()
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
FourWireFan* FourWireFan::setDebounceTime(uint32_t tau)
{
    this->_settings->tau = tau;

    return this;
}

/**
 * Returns current PWM set point.
 * 
 * @since 2020-07-22
 * 
 * @return uint8_t
 */
uint8_t FourWireFan::getPWM()
{
    return this->_pwm;
}

/**
 * Updates PWM set point.
 * 
 * @since 2020-07-22
 * 
 * @param pwm new target PWM value (i.e. speed)
 * 
 * @return FourWireFan*
 */
FourWireFan* FourWireFan::setPWM(uint8_t pwm) 
{
    this->_pwm = max(this->_model->minPWM, min(this->_model->maxPWM, pwm)); // minPWM <= pwm <= maxPWM

    return this;
}

/**
 * Shows indication of spindown condition.
 * 
 * @return bool
 */
bool FourWireFan::isBlocked()
{
    return (0 < this->_spinup);
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
 * 
 * @return FourWireFan*
 */
FourWireFan* FourWireFan::setModel(FourWireFanModel* model) 
{
    // check for safety related out-of-bounds values
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
