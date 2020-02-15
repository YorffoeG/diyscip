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
