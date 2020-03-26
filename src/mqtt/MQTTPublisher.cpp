
#include "MQTTPublisher.h"
#include "debug.h"

void MQTTPublisher::setUpdateInterval(uint32_t interval) {
  updateInterval = interval;
}

void MQTTPublisher::reset() {
  lastUpdateTime  = 0;
  uint16Value     = 0xFFFF;
  uint8Value      = 0xFF;
  strValue[0]     = 0;        
}
  
void MQTTPublisher::setGetter(uint16_t (*getter)(void)) {
  uint16Getter = getter;  
}

void MQTTPublisher::setGetter(uint8_t (*getter)(void)) {
  uint8Getter = getter;
}

void MQTTPublisher::setGetter(const char* (*getter)(void)) {
  strGetter = getter;
}

const char* MQTTPublisher::getValue() {
  return strValue;  
}

bool MQTTPublisher::hasValue(uint32_t now) {

  if ((lastUpdateTime == 0) || (((now - lastUpdateTime) > updateInterval) && (updateInterval != UPDATE_ONCE_AT_BOOT))) {
    // its ok while 'now' goes back to 0 as we work with unsigned int32

    lastUpdateTime = now;
    
    if (uint16Getter != NULL) {
      uint16_t value = uint16Getter();

      if ((value != 0xFFFF) && (value != uint16Value)) { // 0xFFFF = unset value
        uint16Value = value;
        itoa(value, strValue, 10);
        valueIsSet = true;

        return true;        
      }      
      
    } else if (uint8Getter != NULL) {
      uint8_t value = uint8Getter();

      if (value != uint8Value) {
        uint8Value = value;
        strcpy(strValue, (value) ? "on" : "off");
        valueIsSet = true;

        return true;
      }

      
    } else if (strGetter != NULL) {
      const char* value = strGetter();

      if (strcmp(value, strValue)) {
        strncpy(strValue, value, PUBLISHER_VALUE_STR_MAX_LEN);
        strValue[PUBLISHER_VALUE_STR_MAX_LEN-1] = '\0';
        valueIsSet = true;

        return true;        
      }
    }
  }

  // true if never publish and value is set (happen at startup or when reconnect)
  return ((getTime() == 0) && valueIsSet);
}
