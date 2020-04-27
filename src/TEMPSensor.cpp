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

#include "TEMPSensor.h"
#include "Debug.h"
#include "config.h"

#define VCC                 3.3
#define R2                  10000.0
#define ADC_RESOLUTION      1023.0

#define T_0                 273.15


TEMPSensor* TEMPSensor::getInstance() {
    if (instance == NULL) {
        instance = new TEMPSensor();
    }
    return instance;
}


uint16_t  TEMPSensor::getAverageTemperatureCelsius() {
    double      adcValue, Vout, Rth, tempAvg = 0;

    adcValue = analogRead(ADC_PIN);
    Vout     = (adcValue * VCC) / ADC_RESOLUTION;
    Rth      = (VCC * R2 / Vout) - R2;

    samples[sampleIndex++] = (uint16_t) round( (1 / ((log(Rth / NTC_R25) / NTC_BETA) + (1 / (NTC_T25 + T_0)) )) - T_0);
    sampleIndex = sampleIndex % TEMP_SAMPLE_MAX;

    for (int i=0; i<TEMP_SAMPLE_MAX; i++) {
        tempAvg += samples[i];
    }

    return round(tempAvg / TEMP_SAMPLE_MAX);


    // DBG("adc=%f - Vout=%f - Rth=%f - temp=%f\n", adcValue, Vout, Rth, temp);
    // return (uint16_t) round(temp);


    // R = R0 * exp(B(1/T - 1/T0))

    // Le coefficient bêta = 3950K (B25 / 50) et la résistance est de 10x103 = 10KΩ.
}

uint16_t TEMPSensor::getInstantTemperatureCelsius() {
    double      adcValue, Vout, Rth;

    adcValue = analogRead(ADC_PIN);
    Vout     = (adcValue * VCC) / ADC_RESOLUTION;
    Rth      = (VCC * R2 / Vout) - R2;

    return (uint16_t) round( (1 / ((log(Rth / NTC_R25) / NTC_BETA) + (1 / (NTC_T25 + T_0)) )) - T_0);
}


/************ PRIVATE ***********************************/
TEMPSensor::TEMPSensor() {
    uint16_t instantTemp = getInstantTemperatureCelsius();

    // initialize temperature samples with current temp.
    for (int i=0; i<TEMP_SAMPLE_MAX; i++) {
        samples[i] = instantTemp;
    }
}

TEMPSensor* TEMPSensor::instance   = NULL;