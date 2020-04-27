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

#ifndef MQTTTOPIC_H
#define MQTTTOPIC_H

#include "arduino.h"

class MQTTTopic {
  public:
    void        setTopic(const char* topic);
    const char* getTopic();

    uint16_t    getMessageID();
    void        setMessageID(uint16_t id);

    uint32_t    getTime();
    void        setTime(uint32_t time);

  private:
    const char* topic;

    uint16_t    messageID  = 0;
    uint32_t    time       = 0;
};


#endif // MQTTTOPIC_H
