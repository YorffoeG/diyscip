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

#ifndef MQTTPACKET_H
#define MQTTPACKET_H

#include "Arduino.h"
#include <ESP8266WiFi.h>


#define MQTT_KEEPALIVE    60  // Keep Alive interval in Seconds

#define MQTT_CONNECT_USER             0x80
#define MQTT_CONNECT_PASS             0x40
#define MQTT_CONNECT_WILL_RETAIN      0x20
#define MQTT_CONNECT_WILL_QOS1        0x08
#define MQTT_CONNECT_WILL             0x04
#define MQTT_CONNECT_CLEANSESSION     0x02

#define MQTT_PUBLISH_DUP              0x08
#define MQTT_PUBLISH_QOS1             0x02
#define MQTT_PUBLISH_QOS2             0x04
#define MQTT_PUBLISH_RETAIN           0x01

#define MQTT_SUBSCRIBE_FLAGS          0x02
#define MQTT_SUBSCRIBE_QOS            0x01
#define MQTT_SUBACK_FAILURE           0x80                  

#define MQTT_CONNECT_WILL_TOPIC       "/spa/status"
#define MQTT_CONNECT_WILL_MESSAGE     "offline"

#define MQTT_MAX_PACKET_SIZE  127

class MQTTPacket {
  public:

    enum Type {
      INVALID     = 0x00,
      CONNECT     = 0x10,
      CONNACK     = 0x20,
      PUBLISH     = 0x30,
      PUBACK      = 0x40,
      PUBREC      = 0x50,
      PUBREL      = 0x60,
      PUBCOMP     = 0x70,
      SUBSCRIBE   = 0x80,
      SUBACK      = 0x90,
      UNSUBSCRIBE = 0xA0,
      UNSUBACK    = 0xB0,
      PINGREQ     = 0xC0,
      PINGRESP    = 0xD0,
      DISCONNECT  = 0xE0,
      RESERVED    = 0xF0
    };

  
    MQTTPacket(Client* client);

    void              reset(Type type, uint8_t flags = 0);
    void              buildConnect(const char* deviceID, const char* user, const char* pwd);

    size_t            getLength();
    Type              getType();
    uint8_t           getFlags();
    size_t            getRemainingLength();

    void append(uint8_t b);
    void append(uint16_t w);
    void append(const char* str);
    void append(const uint8_t* data, size_t len);
    
    void appendPayload(const char* str);

    bool      send();
    size_t    receive();

    uint8_t       read8bits(); // scan8bits OR read8bits
    uint16_t      read16bits();
    size_t        readString(char* str, size_t max_length);
    const char*   readPayload(); // append 0 at end of buffer to get string

    uint32_t  getLastActivityTime();
    void      serialDump();

  private:
    Client*   client;
    uint8_t   buffer[MQTT_MAX_PACKET_SIZE];
    size_t    length;
    size_t    index = 0;

    uint32_t  lastActivityTime = 0;
};



#endif // MQTTPACKET_H
