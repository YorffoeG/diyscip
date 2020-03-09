#include <Arduino.h>

#include "CTRLPanel.h"
#include "MQTTClient.h"
#include "WIFIManager.h"
#include "TEMPSensor.h"
#include "LEDManager.h"
#include "Debug.h"
#include "config.h"

#ifdef OTA_ENABLED
  #include <ArduinoOTA.h>
#endif


/******************************************************************/
CTRLPanel*    controlPanel  = CTRLPanel::getInstance();
TEMPSensor*   tempSensor    = TEMPSensor::getInstance();
LEDManager    ledBuiltin(LED_BUILTIN);

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


  mqttClient = new MQTTClient(&ledBuiltin);
  mqttClient->setHost(MQTT_HOST);
  mqttClient->setPort(MQTT_PORT);
  mqttClient->setClientID(MQTT_CLIENTID);

  mqttClient->connect();

  mqttClient->addPublisher("/spa/temp/water",         []() -> uint16_t { return controlPanel->getWaterTemperatureCelsius(); });
  mqttClient->addPublisher("/spa/temp/desired",       []() -> uint16_t { return controlPanel->getDesiredTemperatureCelsius(); });
  mqttClient->addPublisher("/spa/state/power",        []() -> uint8_t  { return controlPanel->isPowerOn(); });
  mqttClient->addPublisher("/spa/state/filter",       []() -> uint8_t  { return controlPanel->isFilterOn(); });
  mqttClient->addPublisher("/spa/state/heater",       []() -> uint8_t  { return controlPanel->isHeaterOn(); });
  mqttClient->addPublisher("/spa/state/bubble",       []() -> uint8_t  { return controlPanel->isBubbleOn(); });
  mqttClient->addPublisher("/spa/state/heatreached",  []() -> uint8_t  { return controlPanel->isHeatReached(); });
  mqttClient->addPublisher("/spa/state",              []() -> uint16_t { return controlPanel->getRawStatus(); });

  mqttClient->addSubscriber("/spa/temp/desired/set",  [](uint16_t v) -> bool { return controlPanel->setDesiredTemperatureCelsius(v); });
  mqttClient->addSubscriber("/spa/state/power/set",   [](bool v) -> bool { return controlPanel->setPowerOn(v); });
  mqttClient->addSubscriber("/spa/state/filter/set",  [](bool v) -> bool { return controlPanel->setFilterOn(v); });
  mqttClient->addSubscriber("/spa/state/heater/set",   [](bool v) -> bool { return controlPanel->setHeaterOn(v); });

  mqttClient->addPublisher("/spa/temp/board",         []() -> uint16_t { return tempSensor->getAverageTemperatureCelsius(); });
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

  ledBuiltin.loop();
  mqttClient->loop();

  delay(200);
}