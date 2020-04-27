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

#ifndef MQTTSUBSCRIBER_H
#define MQTTSUBSCRIBER_H

#include "MQTTTopic.h"
#include "Arduino.h"


class MQTTSubscriber : public MQTTTopic {
  
  public:
    void        setSetter(bool  (*setter)(uint16_t));
    void        setSetter(bool  (*setter)(bool));

    bool        setValue(const char* value);

  private:
    bool        (*uint16Setter)(uint16_t) = NULL;
    bool        (*boolSetter)(bool)       = NULL;
};



#endif // MQTTSUBSCRIBER_H
