/**
 * Four Wire Fan
 *
 * An Arduino four-wire fan library that provides a PWM speed and tachometer interface.
 *
 * @author sekdiy (https://github.com/sekdiy/FourWireFan)
 * @date 22.07.2020 Initial release.
 * @version See git comments for changes.
 */

#ifndef __FOURWIREFANMODEL_H__
#define __FOURWIREFANMODEL_H__

#include "Arduino.h"

/**
 * Specific properties of a four wire fan.
 * These properties form the 'model' of a fan, allowing better control and safer operation. 
 */
class FourWireFanModel {
    public:
        // public properties to avoid the getter/setter pattern
        uint8_t minPWM;      // minimum specified speed setting (default: 20%)
        uint16_t minRPM;     // specified speed at `minPWM` (default 400 rpm)
        uint8_t maxPWM;      // maximum sensible speed setting (default: 100%)
        uint16_t maxRPM;     // specified speed at `maxPWM` (default: 1900 rpm)
        uint16_t spinup;     // minimum full speed duration during spin up (default: 0s)
        uint16_t refRPM[10]; // speed reference values (default: none)

        /**
         * Constructs a new four wire fan settings instance.
         * 
         * @param minPWM     The minimum specified speed setting (default: 20%)
         * @param minRPM     The specified speed at `minPWM` (default 400 rpm)
         * @param maxPWM     The maximum sensible speed setting (default: 100%)
         * @param maxRPM     The specified speed at `maxPWM` (default: 1900 rpm)
         * @param spinUp     The minimum full speed duration during spin up (default: 0s)
         * @param refRPM     The fan speed reference values (default: none)
         */
        FourWireFanModel(uint8_t minPWM = 20, uint16_t minRPM = 400, uint8_t maxPWM = 100, uint16_t maxRPM = 2000, uint16_t spinup = 0, uint16_t refRPM[10] = nullptr):
            minPWM(minPWM),
            minRPM(minRPM),
            maxPWM(maxPWM),
            maxRPM(maxRPM),
            spinup(spinup)
        { *this->refRPM = *refRPM; }

        FourWireFanModel* setCoefficient(uint8_t index, float rpm) { this->refRPM[index] = rpm; return this; }
        FourWireFanModel* setCoefficients(uint16_t refRPM[10]) { *this->refRPM = *refRPM; return this; }
};

/**
 * Pre-defined fan model instances.
 */
extern FourWireFanModel DefaultThreeWireFanModel;   // Default *three* wire fan model instance.
extern FourWireFanModel DefaultFourWireFanModel;    // Default *four* wire fan model instance.
extern FourWireFanModel NF_A12_25_FanModel;         // Noctua NF-A12x25 model instance.

#endif  // __FOURWIREFANMODEL_H__
