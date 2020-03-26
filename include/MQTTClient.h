#ifndef MQTTClient_H
#define MQTTClient_H

#include "CFGSettings.h"

#include "MQTTPublisher.h"
#include "MQTTSubscriber.h"
#include "MQTTPacket.h"

#include "Arduino.h"
#include <ESP8266WiFi.h>

#define MAX_PUBLISHER       10
#define MAX_SUBSCRIBER      5

#define MQTT_HOSTNAME_MAX   HOST_LEN_MAX
#define MQTT_CLIENTID_MAX   DEVICEID_LEN_MAX
#define MQTT_USER_MAX       USER_LEN_MAX
#define MQTT_PASS_MAX       PWD_LEN_MAX

/**
 * 
 *  MQTT Client
 *  
 *  Only support packet length < 127
 *  CONNECT -> with CleanSession only
 * 
 * 
 */

class MQTTClient {

  public:
    MQTTClient(CFGSettings &settings);

    /**  !!!! ATTENTION  !!!!!
     *   'topic' string MUST be in-memory persistant, no local copy (for memory consumption reason)
     */
    void addPublisher(const char* topic, uint16_t     (*getter)(void));
    void addPublisher(const char* topic, uint8_t      (*getter)(void)); // use for bool value with 0xFF as unset
    void addPublisher(const char* topic, const char*  (*getter)(void));
    
    void addSubscriber(const char* topic, bool (*setter)(uint16_t set));
    void addSubscriber(const char* topic, bool (*setter)(bool set));

    void setLastAddedPublisherUpdateInterval(uint32_t interval);

    void setSetupModeTrigger(bool (*trigger)(void));

    void connect();
    void loop();

  private:
    CFGSettings         _settings;

    MQTTPublisher*    addPublisher(const char* topic);
    MQTTSubscriber*   addSubscriber(const char* topic);
    
    MQTTPacket*       packet;
    WiFiClient*       tcpClient;
    
    MQTTPublisher     publishers[MAX_PUBLISHER];
    uint32_t          publisherIndex = 0;
    uint32_t          publisherCount = 0;

    MQTTSubscriber    subscribers[MAX_SUBSCRIBER];
    uint32_t          subscriberIndex = 0;
    uint32_t          subscriberCount = 0;

    bool              (*setupModeTrigger)(void) = NULL;

    uint32_t          messageID = 1;
    uint16_t          getNextMessageID();

    void              publish(MQTTPublisher& publisher, uint32_t time );
    void              subscribe(MQTTSubscriber& subscriber, uint32_t time);

    bool              waitingPINGRESP = false;
    bool              isMQTTConnected = false;
    uint32_t          lastCnxAttemptTime = 0;

    static const char*  getStatus();
};


#endif  // MQTTClient_H
