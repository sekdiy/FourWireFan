/**
 * Four Wire Fan
 *
 * An Arduino four-wire fan library that provides a PWM speed and tachometer interface.
 *
 * @author sekdiy (https://github.com/sekdiy/FourWireFan)
 * @date 22.07.2020 Initial release.
 * @version See git comments for changes.
 */

#ifndef __FOURWIREFAN_H__
#define __FOURWIREFAN_H__

#include "FourWireFanSettings.h"
#include "FourWireFanModel.h"

/**
 * A four-wire fan driver that provides a PWM speed and tachometer interface.
 */
class FourWireFan {
    public:
        /** 
         * Simple constructor for a four wire fan (requires external interrupt handling)
         * 
         * @param pwmPin The output pin where the fan's PWM signal input is connected (default: 3)
         * @param tachPin The input pin where the fan's tachometer signal output pin is connected (default: 2)
         * @param tachISR Tachometer interrupt service routine (handler function) callback reference (default: none)
         */
        FourWireFan(uint8_t pwmPin = 3, uint8_t tachPin = 2, void (*tachISR)(void) = nullptr);

        /**
         * Extended constructor for a four wire fan (requires prepared configuration)
         * 
         * @param settings The connection settings of a four wire fan. 
         * @param model The properties of a four wire fan (optional).
         */
        FourWireFan(FourWireFanSettings* settings, FourWireFanModel* model = &DefaultFourWireFanModel);

        void begin();                                   // Setup physical connection and clear pulse counter
        void reset();                                   // Resets measurement values (only)
        void count();                                   // Increments the internal pulse counter (and serves as interrupt callback routine)
        void update(uint16_t duration = 1000);          // Updates fan speed from tachometer pulse input

        uint32_t getRPM();                              // Returns calculated RPM (i.e. fan speed) 
        FourWireFan* setRPM(uint32_t rpm);              // Updates PWM (duty cycle) according to RPM (i.e. fan speed) via fan model lookup

        uint32_t getDebounceTime();                     // Returns current debounce time constant
        FourWireFan* setDebounceTime(uint32_t tau);     // Updates debounce time constant

        uint8_t getPWM();                               // Returns current PWM set point
        FourWireFan* setPWM(uint8_t pwm);               // Updates PWM set point

        bool isBlocked();                               // Shows indication of spindown condition

        FourWireFanModel* getModel();                   // Returns current four wire fan model
        FourWireFan* setModel(FourWireFanModel* model); // Updates four wire fan model

       /* deprecated: */
        void process(uint16_t duration = 1000) { update(duration); }

    protected:
        FourWireFanSettings* _settings;                 // four wire fan settings
        FourWireFanModel* _model;                       // four wire fan model

        uint8_t _pwm = 255;                             // the set point for PWM output pin (default: 100%)
        uint32_t _rpm = 0;                              // the calculated RPM (i.e. fan speed)
        int16_t _spinup = 0;                            // the spinup condition counter
        volatile uint32_t _blink = 0;                   // the moment of the last interrupt 'wakeup'
        volatile uint32_t _pulses = 0;                  // the pulses within the current sample period

        void setup();                                   // initial internal pin setup
};

#endif   // __FOURWIREFAN_H__
