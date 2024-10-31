/**
 * Four Wire Fan
 *
 * An Arduino four-wire fan library that provides a PWM speed and tachometer interface.
 *
 * @author sekdiy (https://github.com/sekdiy/FourWireFan)
 * @date 22.07.2020 Initial release.
 * @version See git comments for changes.
 */

#ifndef __FOURWIREFANSETTING_H__
#define __FOURWIREFANSETTING_H__

#include "Arduino.h"

/**
 * Connection settings of a four wire fan. 
 * These settings inform the driver about the electrical connection of a fan. 
 */
class FourWireFanSettings {
    public:
        // public properties to avoid the getter/setter pattern
        uint8_t pwmPin;        // The output pin where the fan's PWM signal input is connected
        uint8_t tachPin;       // The input pin where the fan's tachometer signal output pin is connected
        void (*tachISR)(void); // Tachometer interrupt service routine (handler function) callback reference
        uint8_t tachMode;      // Tachometer interrupt mode
        uint8_t tachPU;        // Pull up tach pin internally?
        uint32_t tau;          // The debounce timeout

        /**
         * Constructs a new four wire fan settings instance.
         * 
         * @param pwmPin       The output pin where the fan's PWM signal input is connected (default: 3)
         * @param tachPin      The input pin where the fan's tachometer signal output pin is connected (default: 2)
         * @param tachISR      Tachometer interrupt service routine (handler function) callback reference (default: none)
         * @param tachMode     Tachometer interrupt pin mode (default: rising)
         * @param tachPU       Pull-up tach pin internally? (default: no)    
         * @param tau          debounce timeout (default: 10000)
         */
        FourWireFanSettings(uint8_t pwmPin = 3, uint8_t tachPin = 2, void (*tachISR)(void) = nullptr, uint8_t tachMode = FALLING, uint8_t tachPU = INPUT_PULLUP, uint32_t tau = 10000L): 
            pwmPin(pwmPin), 
            tachPin(tachPin),
            tachISR(tachISR),
            tachMode(tachMode),
            tachPU(tachPU),
            tau(tau)
        { /* nop */ }
};

/**
 * Pre-defined four wire fan settings instances.
 */
extern FourWireFanSettings const DefaultFanSettings;  // Default four wire fan settings instance.

#endif  // __FOURWIREFANSETTING_H__
