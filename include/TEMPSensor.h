#ifndef TEMPSENSOR_H
#define TEMPSENSOR_H

#include <Arduino.h>

#define TEMP_UPDATE_INTERVAL    30000
#define TEMP_SAMPLE_MAX         10

class TEMPSensor {

    public:
        static TEMPSensor*  getInstance();

        uint16_t  getAverageTemperatureCelsius();
        uint16_t  getInstantTemperatureCelsius();

    private:
        static TEMPSensor*  instance;
        uint16_t            samples[TEMP_SAMPLE_MAX] = {};
        uint16_t            sampleIndex = 0;

        TEMPSensor();
};


#endif // TEMPSENSOR_H