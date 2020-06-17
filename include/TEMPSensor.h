/**
 * DIYSCIP (c) by Geoffroy HUBERT - yorffoeg@gmail.com
 * This file is part of DIYSCIP <https://github.com/yorffoeg/diyscip>.
 * 
 * DIYSCIP is licensed under a
 * Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 * 
 * You should have received a copy of the license along with this
 * work. If not, see <http://creativecommons.org/licenses/by-nc-sa/4.0/>.
 * 
 * DIYSCIP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;
 */ 

#ifndef TEMPSENSOR_H
#define TEMPSENSOR_H

#include <Arduino.h>

#define TEMP_UPDATE_INTERVAL    30000
#define TEMP_SAMPLE_MAX         10

class TEMPSensor {

    public:
        static TEMPSensor*  getInstance();

        uint16_t  getAverageTemperatureCelsius();

    private:
        static TEMPSensor*  instance;
        bool                initialized = false;
        double              samples[TEMP_SAMPLE_MAX] = {};
        uint16_t            sampleIndex = 0;

        TEMPSensor();
};


#endif // TEMPSENSOR_H