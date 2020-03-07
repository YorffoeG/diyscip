#ifndef MQTTClient_H
#define MQTTClient_H

#include "LEDManager.h"

#include "MQTTPublisher.h"
#include "MQTTSubscriber.h"
#include "MQTTPacket.h"

#include "Arduino.h"
#include <ESP8266WiFi.h>

#define MAX_PUBLISHER       10
#define MAX_SUBSCRIBER      5

#define MQTT_HOSTNAME_MAX   126
#define MQTT_CLIENTID_MAX   31
#define MQTT_USER_MAX       31
#define MQTT_PASS_MAX       31

#define MQTT_KEEPALIVE    60  // Keep Alive interval in Seconds


/**
 * 
 *  MQTT Client
 *  
 *  Only support packet length < 127
 *  CONNECT -> with CleanSession only
 * 
 * 
 *    const static char* test = R"V0G0N(
      <html>
        <body>
          it works !
        </body>
      </html>


      )V0G0N";
 * 
 * 
 * 
 * 
 */

class MQTTClient {

  public:
    MQTTClient(LEDManager* led = NULL);

    void setHost(const char* host);
    void setPort(uint16_t port);
    void setClientID(const char* clientID);
    void setUser(const char* user);
    void setPassword(const char* pass);
    void setRootTopic(const char* root);

    /**  !!!! ATTENTION  !!!!!
     *   'topic' string MUST be in-memory persistant, no local copy (for memory consumption reason)
     */
    void addPublisher(const char* topic, uint16_t     (*getter)(void));
    void addPublisher(const char* topic, uint8_t      (*getter)(void)); // use for bool value with 0xFF as unset
    void addPublisher(const char* topic, const char*  (*getter)(void));
    
    void addSubscriber(const char* topic, bool (*setter)(uint16_t set));
    void addSubscriber(const char* topic, bool (*setter)(bool set));

    void setLastAddedPublisherUpdateInterval(uint32_t interval);

    bool connect();
    void loop();

  private:
    LEDManager*         led;
    static const char*  getStatus();

    MQTTPublisher*    addPublisher(const char* topic);
    MQTTSubscriber*   addSubscriber(const char* topic);
    
    MQTTPacket*       packet;
    WiFiClient*       tcpClient;
    
    // todo !!! dangereux pas de copy des valeurs... REFACTOR !!!
    char              brokerHost[MQTT_HOSTNAME_MAX +1];
    uint16_t          brokerPort;

    char              clientID[MQTT_CLIENTID_MAX +1];
    char              user[MQTT_USER_MAX +1];
    char              pass[MQTT_PASS_MAX +1];
  
    MQTTPublisher     publishers[MAX_PUBLISHER];
    uint32_t          publisherIndex = 0;
    uint32_t          publisherCount = 0;

    MQTTSubscriber    subscribers[MAX_SUBSCRIBER];
    uint32_t          subscriberIndex = 0;
    uint32_t          subscriberCount = 0;

    uint32_t          messageID = 1;
    uint16_t          getNextMessageID();

    void              publish(MQTTPublisher& publisher, uint32_t time );
    void              subscribe(MQTTSubscriber& subscriber, uint32_t time); 

    bool              waitingPINGRESP = false;
    bool              isMQTTConnected = false;
    uint32_t          lastCnxAttemptTime = 0;
};


#endif  // MQTTClient_H
