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

#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

#include "CFGSettings.h"
#include "MQTTClient.h"

#define WIFIQUALITY_UPDATE_INTERVAL 5000000

class WIFIManager {

  public:
    WIFIManager(CFGSettings& settings);

    void setup();
    void loop();

    bool isSTA();
    bool isSTAConnected();

    static uint16_t getWifiQuality();

  private:
    enum MODE {
      UNSET = 0,
      STA   = 1,
      AP    = 2
    };

    CFGSettings         _settings;

    MODE                _mode   = MODE::UNSET;

    volatile uint16_t   _checkCode        = 0;
    uint16_t            _checkLoopCounter = 0;
    IPAddress           _checkIPAddress   = static_cast<uint32_t>(0);
    ip_addr_t           _checkip;
    WiFiClient*         _checkClient      = NULL;
    MQTTPacket*         _checkPacket      = NULL;

    DNSServer*          _dnsServer;
    ESP8266WebServer*   _httpServer;

    String              _ip;

    unsigned long       _setupModeTime    = 0;

    void handleRoot();
    void handleGetSettings();
    void handlePostSettings();
    void handlePostSave();
    void handleGetChecks();
    void handle404();
    bool redirect();

    void checks();
};


#endif // WIFIMANAGER_H
