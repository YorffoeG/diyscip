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

#include "MQTTPacket.h"
#include "Debug.h"

#define UINT16_MSB(w)   (uint8_t)(w >> 8)
#define UINT16_LSB(w)   (uint8_t)(w & 0xFF)

#define DBG_DUMP()          serialDump()

#define MQTT_READ_TIMEOUT   30000   // Maximum time interval in milliseconds to wait for server answer

MQTTPacket::MQTTPacket(Client* client) {
  this->client = client;
}

void MQTTPacket::reset(Type type, uint8_t flags) {
  buffer[0] = type | (flags & 0x0F);
  // reserve 1 byte for Remaining length, will be set when packet is send
  index   = 2;
  length  = 0;
}

void MQTTPacket::buildConnect(const char* deviceID, const char* user, const char* pwd) {
      
    reset(MQTTPacket::Type::CONNECT);
    append((const uint8_t []){0x00, 0x04, 'M', 'Q', 'T', 'T', 0x04}, 7);

    uint8_t flags = MQTT_CONNECT_WILL | MQTT_CONNECT_WILL_RETAIN | MQTT_CONNECT_WILL_QOS1 | MQTT_CONNECT_CLEANSESSION;
    if ((user != NULL) && (*user != '\0')) {
      flags |= MQTT_CONNECT_USER;
      if ((pwd != NULL) && (*pwd != '\0')) {
        flags |= MQTT_CONNECT_PASS;
      }
    }
    append(flags);
    append((uint16_t)MQTT_KEEPALIVE);
    append(deviceID);
    
    append(MQTT_CONNECT_WILL_TOPIC);
    append(MQTT_CONNECT_WILL_MESSAGE);    
    
    if ((user != NULL) && (*user != '\0')) {
      append(user);
      if ((pwd != NULL) && (*pwd != '\0')) {
        append(pwd);
      }
    }
}

size_t MQTTPacket::getLength() {
  return (length == 0) ? index : length;
}

MQTTPacket::Type MQTTPacket::getType() {
  return (index > 0) ? static_cast<MQTTPacket::Type>(buffer[0] & 0xF0) : INVALID;
}

uint8_t MQTTPacket::getFlags() {
  return (index > 0) ? buffer[0] & 0x0F : 0;
}

size_t MQTTPacket::getRemainingLength() {
  return (index > 1) ? buffer[1] : 0;
}

void MQTTPacket::append(uint8_t b) {
  if (index < MQTT_MAX_PACKET_SIZE) {
    buffer[index++] = b;
  }
}

void MQTTPacket::append(uint16_t w) {
  // !!!!!  store MSB before LSB in buffer !!!
  if ((index +1) < MQTT_MAX_PACKET_SIZE) {
    buffer[index++] = UINT16_MSB(w);
    buffer[index++] = UINT16_LSB(w);
  }
}

void MQTTPacket::append(const char* str) {
  const char* p   = str;
  uint16_t    len = 0;

  if ((index + 1) < MQTT_MAX_PACKET_SIZE) {
    
    index += 2; // reserve 2 bytes for string length
    while (*p && (index < MQTT_MAX_PACKET_SIZE)) {
      buffer[index++] = *p++;
      len++;
    }
  
    // set its length at beginning of string
    buffer[index -len -2] = UINT16_MSB(len);
    buffer[index -len -1] = UINT16_LSB(len);  
  }
}

void MQTTPacket::append(const uint8_t* data, size_t len) {
  if ((index + len) < MQTT_MAX_PACKET_SIZE) {
    while (len) {
      buffer[index++] = *data++;
      len--;
    }    
  }
}

void MQTTPacket::appendPayload(const char* str) {
  const char* p = str;

  while (*p && (index < MQTT_MAX_PACKET_SIZE)) {
      buffer[index++] = *p++;

  }
}

bool MQTTPacket::send() {
  buffer[1] = (uint8_t)(index -2);

  DBG("MQTT: Send packet");
  DBG_DUMP();
  
  return (client->write(buffer, index) == index);
}


size_t MQTTPacket::receive() {
  uint32_t  wait = millis();

  index = length = 0;
  do {
    if (client->available()) {
      buffer[index++] = client->read();
      if (index == 2) {
        length = buffer[1] + 2;
        if (length >= MQTT_MAX_PACKET_SIZE) {
          // received packet exceeding maximum size allowed
          // don't waste time with it and close the connexion

          DBG("MQTT: Received packet too large %d, discard connection", length);
          
          client->flush();
          client->stop();
          
          index = length  = 0;
          break;
        }
      }
      if ((index >= 2) && (index == length)) {
        index = 2; // seek after fixed header
        lastActivityTime = wait; // useless to ask for millis() again
        break;
      }
      wait = millis();
      
    } else { delay(100); }
    
  } while ((millis() - wait) < MQTT_READ_TIMEOUT);

  DBG("MQTT: Receive packet");
  DBG_DUMP();
  
  return length;
}

uint8_t   MQTTPacket::read8bits() {
  return (index < length) ? buffer[index++] : 0;
}

uint16_t  MQTTPacket::read16bits() {
  if ((index + 1) < length) {
    uint16_t v = buffer[index++] << 8;
    
    v += buffer[index++];
    return v;

  } else {

    return 0;
  }
}

size_t    MQTTPacket::readString(char* str, size_t max_length) {
  uint16_t  len     = read16bits();
  size_t    i       = 0;

  if (max_length > 0) {
    max_length--; // remove 1 byte for \0 terminal of str  
  }
  
  while ((index < length) && (len > 0)) {
    if (i < max_length) {
      str[i++] = buffer[index];
    }
    index++;
    len--;
  }

  if (max_length > 0) {
    str[i] = 0x00; // \0 terminal
  }
  
  return i;
}

const char*   MQTTPacket::readPayload() {
    // looks ugly but it's ok for usage
    // the payload while receive packet is supposed to be a string
    // set a zero terminal to buffer
    buffer[length] = 0x00;
    return (const char*)(buffer + index);
}

uint32_t MQTTPacket::getLastActivityTime() {  
  return lastActivityTime;
}

void MQTTPacket::serialDump() {
  char s[5];
  
  DBGNOLN("MQTT: Packet[%d]\n      [ ", getLength());
  for (size_t i=0; i<getLength(); i++) {
    sprintf(s, "%02x ", buffer[i]);
    DBGNOLN(s);
  }
  DBGNOLN("]\n");
}
