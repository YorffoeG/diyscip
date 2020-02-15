
#include "MQTTTopic.h"


void MQTTTopic::setTopic(const char* topic) {
  this->topic = topic;
}

const char* MQTTTopic::getTopic() {
  return topic;
}

void MQTTTopic::setMessageID(uint16_t id) {
  messageID = id;  
}

uint16_t MQTTTopic::getMessageID() {
  return messageID;
}


void MQTTTopic::setTime(uint32_t time) {
  this->time = time;  
}

uint32_t MQTTTopic::getTime() {
  return time;
}
