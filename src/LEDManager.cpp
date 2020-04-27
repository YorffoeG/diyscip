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


#include "LEDManager.h"

LEDManager::LEDManager(uint8_t pin) {
    this->pin   = pin;

    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

void LEDManager::setMode(const std::initializer_list<uint16_t>& modePattern) {
    this->modeStep      = 0;
    this->modeCounter   = 0;

    uint32_t index = 0;
    for (auto pattern : modePattern) {
        this->modePattern[index++] = pattern;
        if (index >= MODE_PATTERN_MAX) {
            break;
        }
    }
    this->modePattern[index] = 0;

    digitalWrite(pin, LOW);
}

void LEDManager::loop() {
    this->modeCounter ++;

    if (this->modeCounter > this->modePattern[this->modeStep]) {
        this->modeCounter = 0;
        this->modeStep++;
        if (this->modePattern[this->modeStep] == 0) {
            this->modeStep = 0;
        }

        digitalWrite(this->pin, this->modeStep % 2 ? HIGH : LOW);
    }

}

