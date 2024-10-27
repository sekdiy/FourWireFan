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

        FourWireFanModel* setCoefficient(uint8_t index, float rpm) { this->refRPM[index] = rpm; }
        FourWireFanModel* setCoefficients(float refRPM[10]) { *this->refRPM = *refRPM; }
};

/**
 * Pre-defined four wire fan model instances.
 */
extern FourWireFanModel DefaultFanModel;    //!< Default four wire fan model instance.

/**
 * Four wire fan driver class.
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
         * @param model The properties of a four wire fan.
         */
        FourWireFan(FourWireFanSettings* settings = &DefaultFanSettings, FourWireFanModel* model = &DefaultFanModel);


        void process(unsigned long duration = 1000);    //!< Processes fan speed from tachometer pulse input
        void reset();                                   //!< Resets measurement values (only)
        void count();                                   //!< Increments the internal pulse counter (and serves as interrupt callback routine)

        unsigned long getRPM();                         //!< Returns calculated RPM (i.e. fan speed) 
        float getSlippage();                            //!< Returns calculated slippage from current load (requires model)

        unsigned int getDebounceTime();                 //!< Returns current debounce time constant
        FourWireFan* setDebounceTime(unsigned long tau = 10000L); //!< Updates debounce time constant

        float getPWM();                                 //!< Returns current PWM set point
        FourWireFan* setPWM(float pwm);                 //!< Updates PWM set point

        FourWireFanModel* getModel();                   //!< Returns current four wire fan model
        FourWireFan* setModel(FourWireFanModel* model = &DefaultFanModel);  //!< Updates four wire fan model

    protected:
        FourWireFanSettings* _settings;                 //!< four wire fan settings
        FourWireFanModel* _model;                       //!< four wire fan model

        float _pwm = 255;                               //!< the set point for PWM output pin (default: 100%)
        unsigned long _rpm = 0;                         //!< the calculated RPM (i.e. fan speed)
        volatile unsigned long _blink = 0;              //!< the moment of the last interrupt 'wakeup'
        volatile unsigned long _pulses = 0;             //!< the pulses within the current sample period
        int16_t _spinup = 0;                            // the spinup condition counter

        void setup();                                   //!< initial internal pin setup
};

#endif   // __FOURWIREFAN_H__
