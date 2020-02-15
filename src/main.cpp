#include <Arduino.h>

#include "CTRLPanel.h"
#include "MQTTClient.h"
#include "WIFIManager.h"
#include "TEMPSensor.h"
#include "Debug.h"
#include "config.h"

#ifdef OTA_ENABLED
  #include <ArduinoOTA.h>
#endif


/******************************************************************/
CTRLPanel*    controlPanel  = CTRLPanel::getInstance();
TEMPSensor*   tempSensor    = TEMPSensor::getInstance();

MQTTClient*   mqttClient;
WIFIManager*  wifiManager;

#ifndef NODEBUG
  #ifdef DBG_TCP_ENABLED
    WiFiServer  serverDebug(DBG_TCP_PORT);
    WiFiClient clientDebug;
  #endif // DBG_TCP
#endif // NODEBUG


void setup() {
  Serial.begin(SERIAL_DEBUG_SPEED);

  DBG("\n\n\nSETUP: starting");

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output

  wifiManager = new WIFIManager();
  //wifiManager->setSTASsid(WIFI_SSID);
  //wifiManager->setSTAPass(WIFI_PWD);
  wifiManager->startSTA();

  #ifdef OTA_ENABLED
  
    ArduinoOTA.onStart([]() {
      DBG("OTA: Start [%d]", ArduinoOTA.getCommand());
    });
    
    ArduinoOTA.onEnd([]() {
      DBG("OTA: End");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      DBG("OTA: Progress: %u%%\r", (progress / (total / 100)));
    });

    ArduinoOTA.onError([](ota_error_t error) {
      DBG("OTA: Error [%u]", error);
    });

    ArduinoOTA.begin();
    DBG("SETUP: OTA Enabled");

  #endif // OTA_ENABLED
  
  #if !defined(NODEBUG) && defined(DBG_TCP_ENABLED)
    serverDebug.begin();
  #endif // NODEBUG


  mqttClient = new MQTTClient(MQTT_HOST, MQTT_PORT);
  mqttClient->setClientID(MQTT_CLIENTID);

  mqttClient->connect();

  mqttClient->addPublisher("/spa/temp/water",     [controlPanel]() -> uint16_t { return controlPanel->getWaterTemperatureCelsius(); });
  mqttClient->addPublisher("/spa/temp/desired",   [controlPanel]() -> uint16_t { return controlPanel->getDesiredTemperatureCelsius(); });
  mqttClient->addPublisher("/spa/state/power",    [controlPanel]() -> uint8_t  { return controlPanel->isPowerOn(); });
  mqttClient->addPublisher("/spa/state",          [controlPanel]() -> uint16_t { return controlPanel->getRawStatus(); });

  mqttClient->addSubscriber("/spa/temp/desired/set", [controlPanel](uint16_t v) -> bool { return controlPanel->setDesiredTemperatureCelsius(v); });
  mqttClient->addSubscriber("/spa/state/power/set",  [controlPanel](bool v) -> bool { return controlPanel->setPowerOn(v); });

  mqttClient->addPublisher("/spa/temp/board",     [tempSensor]() -> uint16_t { return tempSensor->getAverageTemperatureCelsius(); });
  mqttClient->setLastAddedPublisherUpdateInterval(TEMP_UPDATE_INTERVAL);

}

void loop() {

  #ifdef OTA_ENABLED
    ArduinoOTA.handle();
  #endif

  #if !defined(NODEBUG) && defined(DBG_TCP_ENABLED)
    if (!clientDebug.connected()) {
      clientDebug.flush();
      clientDebug.stop();
      clientDebug = serverDebug.available();
    } else {
      if (clientDebug.available() && clientDebug.readString().equals(DBG_TCP_CLIENT_PING)) {
        clientDebug.write(DBG_TCP_CLIENT_PONG);
      }
    }
  #endif // NODEBUG

  digitalWrite(LED_BUILTIN, HIGH);

  mqttClient->loop();

  digitalWrite(LED_BUILTIN, LOW); 

  delay(200);                       
}