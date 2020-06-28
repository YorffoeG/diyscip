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

#ifndef MQTTClient_H
#define MQTTClient_H

#include "CFGSettings.h"

#include "MQTTPublisher.h"
#include "MQTTSubscriber.h"
#include "MQTTPacket.h"

#include "Arduino.h"
#include <ESP8266WiFi.h>

#include "config.h"

#ifdef SSP_H
  #define MAX_PUBLISHER       11
  #define MAX_SUBSCRIBER      6
#endif

#ifdef SJB_HS
  #define MAX_PUBLISHER       12
  #define MAX_SUBSCRIBER      7
#endif

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
    void addSubscriber(const char* topic, bool (*setter)(const char* set));

    void setLastAddedPublisherUpdateInterval(uint32_t interval);

    void setSetupModeTrigger(bool (*trigger)(void));

    void connect();
    void loop();

    void publish(const char* topic, const char* payload);

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

    bool              isBackoffTimeout(uint32_t ms);
    void              resetBackoffTimeout(uint32_t ms);

    bool              waitingPINGRESP = false;
    bool              isMQTTConnected = false;
    uint32_t          lastCnxTry      = 0;
    uint32_t          backoffWait     = 0;
};


#endif  // MQTTClient_H
