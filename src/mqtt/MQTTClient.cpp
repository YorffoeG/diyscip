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

#include "MQTTClient.h"
#include "Version.h"
#include "Debug.h"

#include "Arduino.h"

#define MQTT_CONNECTION_TIMELAPSE     10000 // in ms

#define BACKOFF_TIMEOUT_MAX           32000 // in ms
#define BACKOFF_RANDOM_DELAY          2000 // in ms

#define MQTT_ACK_TIMEOUT              15000 // interval to receive ack to subscribe/publish in ms

#define MQTT_KEEPALIVE_MS_THRESHOLD   (MQTT_KEEPALIVE * 800)  // in ms
#define MQTT_KEEPALIVE_MS             (MQTT_KEEPALIVE * 1000) // in ms

#define MQTT_TOPIC_MAXLENGTH          100


MQTTClient::MQTTClient(CFGSettings &settings) : _settings(settings) {
  tcpClient = new WiFiClient();
  packet    = new MQTTPacket(tcpClient);
}

void MQTTClient::connect() {

  DBG("MQTT: TCP trying to connect to %s:%d", _settings.getBrokerHost(), _settings.getBrokerPort());

  this->isMQTTConnected = false;
    
  if (tcpClient->connect(_settings.getBrokerHost(), _settings.getBrokerPort())) {
    DBG("MQTT: TCP connected");
    DBG("MQTT< CONNECT");

    packet->buildConnect(_settings.getDeviceID(), _settings.getBrokerUser(), _settings.getBrokerPwd());
    packet->send();

  } else { DBG("MQTT: TCP connection failed"); }
}


void MQTTClient::addPublisher(const char* topic, uint16_t (*getter)(void)) {
  MQTTPublisher*  publisher = addPublisher(topic);
  
  if (publisher != NULL) {
    publisher->setGetter(getter);
  }
}

void MQTTClient::addPublisher(const char* topic, uint8_t (*getter)(void)) {
  MQTTPublisher*  publisher = addPublisher(topic);
  
  if (publisher != NULL) {
    publisher->setGetter(getter);
  }
}

void MQTTClient::addPublisher(const char* topic, const char* (*getter)(void)) {
  MQTTPublisher*  publisher = addPublisher(topic);
  
  if (publisher != NULL) {
    publisher->setGetter(getter);
  }
}

void MQTTClient::addSubscriber(const char* topic, bool (*setter)(uint16_t set)) {
  MQTTSubscriber* subscriber = addSubscriber(topic);

  if (subscriber != NULL) {
    subscriber->setSetter(setter);
  } 
}

void MQTTClient::addSubscriber(const char* topic, bool (*setter)(bool set)) {
  MQTTSubscriber* subscriber = addSubscriber(topic);

  if (subscriber != NULL) {
    subscriber->setSetter(setter);
  }  
}

void MQTTClient::addSubscriber(const char* topic, bool (*setter)(const char* set)) {
  MQTTSubscriber* subscriber = addSubscriber(topic);

  if (subscriber != NULL) {
    subscriber->setSetter(setter);
  }  
}

void MQTTClient::setLastAddedPublisherUpdateInterval(uint32_t interval) {
  if (publisherCount > 0) {
    publishers[publisherCount-1].setUpdateInterval(interval);    
  }
}

void MQTTClient::setSetupModeTrigger(bool (*trigger)(void)) {
  setupModeTrigger = trigger;
}

void MQTTClient::publish(const char* topic, const char* payload) {

  packet->reset(MQTTPacket::Type::PUBLISH, MQTT_PUBLISH_QOS0 | MQTT_PUBLISH_RETAIN);
  packet->append(topic);
  
  packet->appendPayload(payload);

  packet->send();
}

void MQTTClient::loop() {
  uint32_t now = millis();

  if ((setupModeTrigger != NULL) && setupModeTrigger()) {
    _settings.enterMode(CFGSettings::SETUP);
    return ;
  }

  if (!tcpClient->connected() || !isMQTTConnected) {
    if (isMQTTConnected) {
      DBG("MQTT: TCP lost connection");
      isMQTTConnected = false;

    } else {

      if (tcpClient->connected() && tcpClient->available()) {
        packet->receive();

        if ( packet->getType() == MQTTPacket::Type::CONNACK) {
          uint8_t result;
    
          packet->read8bits(); // Connect Acknowledge flags
          result = packet->read8bits();
          if (result == 0x00) {

            DBG("MQTT> CONNACK");

            resetBackoffTimeout(now);
            waitingPINGRESP = false;

            // reset Publishers/Subscribers
            for (uint32_t i=0; i< publisherCount; i++) {
              publishers[i].reset();
            }
            for (uint32_t i=0; i< subscriberCount; i++) {
              subscribers[i].setTime(0);
            }

            isMQTTConnected = true;

            this->publish("spa/status", "online");
            this->publish("spa/sys/version", FW_VERSION);
            this->publish("spa/sys/updatable", _settings.isUpdateEnabled() ? "true" : "false");

            return ;

          } else { DBG("MQTT> CONNACK error %d", result); }
        } else { DBG("MQTT> Bad type %d, expected CONNACK", packet->getType()); }

        tcpClient->flush();
        tcpClient->stop();

      } else {

        if (isBackoffTimeout(now)) {
          connect();
        }
      }
    }

    return ;
  }

  MQTTPublisher&      publisher  = publishers[publisherIndex];
  MQTTSubscriber&     subscriber = subscribers[subscriberIndex];

  if (publisher.hasValue(now)) {
    publisher.setMessageID(0);  // value has change so clean previous message even it was not ack  
    publish(publisher, now);
    
  } else if ((publisher.getMessageID() != 0) && ((now - publisher.getTime()) > MQTT_ACK_TIMEOUT)) {
     // if value has not been ack
     publish(publisher, now);         
  }
  publisherIndex = (publisherIndex + 1) % publisherCount;
  

  if ((subscriber.getTime() == 0) ||
      ((subscriber.getMessageID() != 0) && ((now - subscriber.getTime()) > MQTT_ACK_TIMEOUT))) {
    // if never subscribe or subscribe has not been ack
    subscribe(subscriber, now);   
  }
  subscriberIndex = (subscriberIndex +1) % subscriberCount;  

  if ((now - packet->getLastActivityTime()) > MQTT_KEEPALIVE_MS_THRESHOLD) {
    if (! waitingPINGRESP) {
      DBG("MQTT< PINGREQ");

      waitingPINGRESP = true;
      
      packet->reset(MQTTPacket::Type::PINGREQ);
      packet->send();

      return ;
      
    } else if ((now - packet->getLastActivityTime()) > MQTT_KEEPALIVE_MS) {
      DBG("MQTT: NO PINGRESP, discard connection");
      
      tcpClient->flush();
      tcpClient->stop();

      isMQTTConnected = false;
      return ;
    }
  }

  if (tcpClient->available()) {
    packet->receive();

    switch (packet->getType()) {
      case MQTTPacket::Type::PINGRESP: {
        DBG("MQTT> PINGRESP");
        waitingPINGRESP = false;
        
        break;
      }

      case MQTTPacket::Type::PUBACK: {
        uint16_t id = packet->read16bits();

        DBG("MQTT> PUBACK id=%d", id);
        
        for (uint32_t i=0; i<publisherCount; i++) {
          if (publishers[i].getMessageID() == id) {
            
            publishers[i].setMessageID(0); // ACK publish
            break;
          }
        }
        
        break;
      }

      case MQTTPacket::Type::SUBACK: {
        uint16_t id         = packet->read16bits();
        uint8_t  returnCode = packet->read8bits();

        DBG("MQTT> SUBACK id=%d - result=%d", id, returnCode);

        if (!(returnCode & MQTT_SUBACK_FAILURE)) {
          for (uint32_t i=0; i<subscriberCount; i++) {
            if (subscribers[i].getMessageID() == id) {
              
              subscribers[i].setMessageID(0); // ACK subscriber 
              break;
            }
          }          
        } // subscribe fail !

        break;
      }

      case MQTTPacket::Type::PUBLISH: {
        uint8_t   flags = packet->getFlags();
        uint16_t  messageID = 0;
        char      topic[MQTT_TOPIC_MAXLENGTH];

        packet->readString(topic, MQTT_TOPIC_MAXLENGTH);
        if (flags & (MQTT_PUBLISH_QOS1 | MQTT_PUBLISH_QOS2)) {
          
          messageID = packet->read16bits();

        } // else no messageID for QOS0
        

        for (uint32_t i=0; i<subscriberCount; i++) {
          if (!strcmp(subscribers[i].getTopic(), topic)) {

            const char* value = packet->readPayload();

            DBG("MQTT> PUBLISH %s=%s", topic, value);

            if (subscribers[i].setValue(value)) {
              
              // ack publish
              if (flags & MQTT_PUBLISH_QOS1) {
                DBG("MQTT< PUBACK %d", messageID);
                
                packet->reset(MQTTPacket::Type::PUBACK);
                packet->append(messageID);

                packet->send();              
                
              } else if (flags & MQTT_PUBLISH_QOS2) {
                DBG("MQTT< PUBREC %d", messageID);
                
                packet->reset(MQTTPacket::Type::PUBREC);
                packet->append(messageID);

                packet->send();
              }

            } else {
              DBG("MQTT: PUBLISH failed");
            }

            break;
          }
        }

        break;
      }

      case MQTTPacket::Type::PUBREL: { // should not append, only support QOS1, but...
        uint16_t  messageID = packet->read16bits();

        DBG("MQTT> PUBREL %d", messageID);
        DBG("MQTT< PUBCOMP %d", messageID);

        packet->reset(MQTTPacket::Type::PUBCOMP);
        packet->append(messageID);

        packet->send();
        break;
      }

      default:
        DBG("MQTT> packet type %d", packet->getType());
    }
  }

}

/***** PRIVATE ********************************/

uint16_t MQTTClient::getNextMessageID() {
  messageID ++;

  if (messageID == 0) { // avoid messageID == 0, this value is use to set ack message
    messageID ++;
  }

  return messageID;
}


void MQTTClient::publish( MQTTPublisher& publisher, uint32_t time ) {
  uint8_t flags = MQTT_PUBLISH_QOS1 | MQTT_PUBLISH_RETAIN;

  DBG("MQTT< PUBLISH %s => %s", publisher.getTopic(), publisher.getValue());
  
  if (publisher.getMessageID()) { //duplicate publish
    flags |= MQTT_PUBLISH_DUP;
  } else {
    publisher.setMessageID( getNextMessageID() );
  }

  publisher.setTime(time);

  packet->reset(MQTTPacket::Type::PUBLISH, flags);
  packet->append(publisher.getTopic());
  packet->append(publisher.getMessageID());
  
  packet->appendPayload(publisher.getValue());

  packet->send();
}

void MQTTClient::subscribe( MQTTSubscriber& subscriber, uint32_t time) {

  DBG("MQTT< SUBSCRIBE %s", subscriber.getTopic());

  subscriber.setMessageID( getNextMessageID() );
  subscriber.setTime(time);

  packet->reset(MQTTPacket::Type::SUBSCRIBE, MQTT_SUBSCRIBE_FLAGS);
  packet->append(subscriber.getMessageID());
  packet->append(subscriber.getTopic());
  packet->append((uint8_t)MQTT_SUBSCRIBE_QOS);

  packet->send();  
}



MQTTPublisher*  MQTTClient::addPublisher(const char* topic) {
  if (publisherCount < MAX_PUBLISHER) {
    MQTTPublisher*  publisher = & publishers[publisherCount];

    publisher->setTopic(topic);
    publisherCount++;

    return publisher;
    
  } else { DBG("MQTT: too many publisher"); }

  return NULL;
}

MQTTSubscriber* MQTTClient::addSubscriber(const char* topic) {
  if (subscriberCount < MAX_SUBSCRIBER) {
    MQTTSubscriber* subscriber = & subscribers[subscriberCount];

    subscriber->setTopic(topic);
    subscriberCount++;

    return subscriber;

  } else { DBG("MQTT: too many subscriber"); }

  return NULL;
}

bool MQTTClient::isBackoffTimeout(uint32_t ms) {
  if (ms > (lastCnxTry + backoffWait)) {
    if (backoffWait < BACKOFF_TIMEOUT_MAX) {
        backoffWait += random(BACKOFF_RANDOM_DELAY);
    }
    lastCnxTry = ms;
    return true;

  } else {
    return false;
  }
}

void MQTTClient::resetBackoffTimeout(uint32_t ms) {
  lastCnxTry  = ms;
  backoffWait = 0;
}
