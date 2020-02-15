#include "MQTTSubscriber.h"

#include "Debug.h"

void  MQTTSubscriber::setSetter(bool  (*setter)(uint16_t)) {
  uint16Setter = setter;  
}

void MQTTSubscriber::setSetter(bool  (*setter)(bool)) {
  boolSetter = setter;
}


bool MQTTSubscriber::setValue(const char* value) {
  if (boolSetter != NULL) {
    if (!strcmp(value, "on")) {
      return boolSetter(true);
      
    } else if (!strcmp(value, "off")) {
      return boolSetter(false);

    } else {
      DBG("MQTT: subscriber wrong bool value: %s", value);
    }

  } else if (uint16Setter != NULL) {
    if (*value != 0) {
      uint16_t uint16Value = 0;

      do {
        if (*value >= '0' && *value <= '9') {

          uint16Value = uint16Value*10 + (*value - '0');
          value ++;

        } else {
          DBG("MQTT: subscriber wrong uint16 value");
          return false;
        }
      } while (*value != 0);

      return uint16Setter(uint16Value);

    } // else empty string    
  }
  return false;
}
