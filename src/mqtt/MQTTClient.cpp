
#include "MQTTClient.h"
#include "Debug.h"

#include "Arduino.h"


#define MQTT_KEEPALIVE_MS             (MQTT_KEEPALIVE * 1000) // in ms
#define MQTT_KEEPALIVE_MS_THRESHOLD   (MQTT_KEEPALIVE * 800)  // in ms

#define MQTT_ACK_TIMEOUT              15000 // interval to receive ack to subscribe/publish in ms

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

#define MQTT_TOPIC_MAXLENGTH          100

MQTTClient::MQTTClient(const char* host, uint16_t port) {
  this->brokerHost = host;
  this->brokerPort = port;

  tcpClient = new WiFiClient();
  packet    = new MQTTPacket(tcpClient);

  addPublisher("/spa/status", MQTTClient::getStatus);
  setLastAddedPublisherUpdateInterval(UPDATE_ONCE_AT_BOOT);
}

void MQTTClient::setClientID(const char* clientID) {
  this->clientID = clientID;  
}

void MQTTClient::setUser(const char* user) {
  this->user = user;  
}

void MQTTClient::setPassword(const char* pass) {
  this->pass = pass;  
}

void MQTTClient::setRootTopic(const char* root) {
  this->rootTopic = root;
}

bool MQTTClient::connect() {

  DBG("MQTT: TCP trying to connect to %s:%d", brokerHost, brokerPort);

  if (tcpClient->connect(brokerHost, brokerPort)) {
    DBG("MQTT: TCP connected");
    DBG("MQTT< CONNECT");
    
    packet->reset(MQTTPacket::Type::CONNECT);
    packet->append((const uint8_t []){0x00, 0x04, 'M', 'Q', 'T', 'T', 0x04}, 7);

    uint8_t flags = MQTT_CONNECT_WILL | MQTT_CONNECT_WILL_RETAIN | MQTT_CONNECT_WILL_QOS1 | MQTT_CONNECT_CLEANSESSION;
    if (user) {
      flags |= MQTT_CONNECT_USER;
      if (pass) {
        flags |= MQTT_CONNECT_PASS;
      }
    }
    packet->append(flags);
    packet->append((uint16_t)MQTT_KEEPALIVE);
    packet->append(clientID);
    
    packet->append(MQTT_CONNECT_WILL_TOPIC);
    packet->append(MQTT_CONNECT_WILL_MESSAGE);    
    
    if (user) {
      packet->append(user);
      if (pass) {
        packet->append(pass);
      }
    }
    
    packet->send();

    if ( packet->receive() > 0) {
      if ( packet->getType() == MQTTPacket::Type::CONNACK) {
        uint8_t result;
  
        packet->read8bits(); // Connect Acknowledge flags
        result = packet->read8bits();
        if (result == 0x00) {

          DBG("MQTT> CONNACK");

          waitingPINGRESP = false;

          // reset Publishers/Subscribers
          for (uint32_t i=0; i< publisherCount; i++) {
            publishers[i].reset();
          }
          for (uint32_t i=0; i< subscriberCount; i++) {
            subscribers[i].setTime(0);
          }

          return true;

        } else { DBG("MQTT> CONNACK error %d", result); }
      } else { DBG("MQTT> Bad type %d, expected CONNACK", packet->getType()); } 
    } else { DBG("MQTT> no response"); }  

    // discard connection
    tcpClient->flush();
    tcpClient->stop();
      
  } else { DBG("MQTT: TCP Failed"); }
 
  return false;
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


void MQTTClient::setLastAddedPublisherUpdateInterval(uint32_t interval) {
  if (publisherCount > 0) {
    publishers[publisherCount-1].setUpdateInterval(interval);    
  }
}


void MQTTClient::loop() {
  MQTTPublisher&      publisher  = publishers[publisherIndex];
  MQTTSubscriber&     subscriber = subscribers[subscriberIndex];
  uint32_t            now = millis();

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
  

  if (!tcpClient->connected()) {
    DBG("MQTT: TCP not connected");
    delay(2000);
    connect();
    return ;
  }

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

              // while setting a value, force publish the corresponding publisher
              // because phone app wait for the value to be confirm but it may not
              // happen if setValue is the same than the current value.
              for(uint32_t j=0; j<publisherCount; j++) {
                if (isPublisherTopicMatchSubscriberTopic(publishers[j].getTopic(), topic)) {
                  publishers[j].reset();
                  break;
                }
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
const char*  MQTTClient::getStatus() {
  return "online";  
}


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

bool MQTTClient::isPublisherTopicMatchSubscriberTopic(const char* publisherTopic, const char* subscriberTopic) {
  int i = 0;

  while (publisherTopic[i] != '\0') {
    if (subscriberTopic[i] != publisherTopic[i]) {
      return false;
    }
    i++;
  }
  return ((publisherTopic[i++] == '/') && 
          (publisherTopic[i++] == 's') && 
          (publisherTopic[i++] == 'e') && 
          (publisherTopic[i++] == 't') && 
          (publisherTopic[i++] == '\0')); 
}
