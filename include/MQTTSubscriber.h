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
