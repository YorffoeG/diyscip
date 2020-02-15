#ifndef MQTTPUBLISHER_H
#define MQTTPUBLISHER_H

#include "MQTTTopic.h"
#include "Arduino.h"


#define PUBLISHER_VALUE_STR_MAX_LEN   8           // max length for a string publish value
#define UPDATE_ONCE_AT_BOOT           0xFFFFFFFF  // the publisher only update once at boot


class MQTTPublisher : public MQTTTopic {
  
  public:
    void        setUpdateInterval(uint32_t interval);
    void        reset();

    void        setGetter(uint16_t    (*getter)(void));
    void        setGetter(uint8_t     (*getter)(void));
    void        setGetter(const char* (*getter)(void));

    bool        hasValue(uint32_t now);
    const char* getValue();

  private:
    uint32_t    updateInterval = 0;
    uint32_t    lastUpdateTime = 0;
    
    uint16_t    (*uint16Getter)(void) = NULL;
    uint8_t     (*uint8Getter)(void)  = NULL;
    const char* (*strGetter)(void)    = NULL;

    bool        valueIsSet  = false;
    uint16_t    uint16Value = 0xFFFF;
    uint8_t     uint8Value  = 0xFF;
    char        strValue[PUBLISHER_VALUE_STR_MAX_LEN] = {};
};


#endif // MQTTPUBLISHER_H
