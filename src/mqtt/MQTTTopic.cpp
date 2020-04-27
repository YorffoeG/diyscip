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
