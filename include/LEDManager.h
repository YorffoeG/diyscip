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

#ifndef LEDMANAGER_H
#define LEDMANAGER_H

#include "Arduino.h"


#define MODE_PATTERN_MAX    10

#define BLINK_1     {10,10,0}
#define BLINK_2     {10,2,0}
#define BLINK_3     {1,1,0}

class LEDManager {

public:
    LEDManager(uint8_t pin);

    void loop();
    void setMode(const std::initializer_list<uint16_t>& modePattern);


private:
    uint8_t             pin;
    uint16_t            modePattern[MODE_PATTERN_MAX +1] = {};
    uint32_t            modeStep    = 0;
    uint32_t            modeCounter = 0;
   
};


#endif // LEDMANAGER_H