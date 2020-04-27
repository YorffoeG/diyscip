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


#include "WIFIManager.h"
#include "lwip/dns.h"

#include "config.h"
#include "Debug.h"

#include "../html/home.htm.h"

#define DNS_PORT    53
#define HTTP_PORT   80

#define SETUP_TIMEOUT           300000 // 5 min (5min * 60sec * 1000ms)

#define EEPROM_ADDR_MAX         512

#define CHECK_PENDING_LOOP_MAX  200

#define CHECK_ERROR       0x8000
#define CHECK_PENDING     0x4000
#define CHECK_RESTART     0xFF00

#define CHECK_WIFI        0x0100
#define CHECK_HOST        0x0200
#define CHECK_TCP         0x0400
#define CHECK_MQTT        0x0800

#define CHECK_EXTRA_CODE  0x00FF

#define IS_CHECK_FAILED(code)    (code & CHECK_ERROR)
#define IS_CHECK_PENDING(code)   (code & CHECK_PENDING)


void check_dns_found_callback(const char *name, CONST ip_addr_t *ipaddr, void *callback_arg);

const char* mime_json = "application/json; charset=UTF-8";


WIFIManager::WIFIManager(CFGSettings& settings) :
  _settings(settings)
{

}

void WIFIManager::setup() {
  if (_settings.getMode() != CFGSettings::RUNNING) { // start AP

    if (_settings.getMode() == CFGSettings::SETUP) {
      // entering setup mode from running request...
      // timeout back to running if no setup
      
      _setupModeTime = millis();
    }


    _mode = WIFIManager::AP;

    _dnsServer   = new DNSServer();
    _httpServer  = new ESP8266WebServer(HTTP_PORT);

    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_AP_NAME);

    //delay(500);

    IPAddress ipAddr = WiFi.softAPIP();
    for (int i = 0; i < 3; i++) {
      _ip += String((ipAddr >> (8 * i)) & 0xFF) + ".";
    }
    _ip += String(((ipAddr >> 8 * 3)) & 0xFF);

    DBG("AP ip = %s", _ip.c_str());

    _dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
    _dnsServer->start(DNS_PORT, "*", ipAddr);

    _httpServer->on(String(F("/")).c_str(),                   std::bind(&WIFIManager::handleRoot, this));
    _httpServer->on(String(F("/settings")).c_str(), HTTP_GET, std::bind(&WIFIManager::handleGetSettings, this));
    _httpServer->on(String(F("/settings")).c_str(), HTTP_POST,std::bind(&WIFIManager::handlePostSettings, this));
    _httpServer->on(String(F("/checks")).c_str(),   HTTP_GET, std::bind(&WIFIManager::handleGetChecks, this));
    _httpServer->on(String(F("/save")).c_str(),     HTTP_POST,std::bind(&WIFIManager::handlePostSave, this));

    _httpServer->onNotFound (std::bind(&WIFIManager::handle404, this));

    _httpServer->begin();

  } else { // start STA
    _mode = WIFIManager::STA;

    WiFi.mode(WIFI_STA);
    WiFi.enableAP(false);
    WiFi.hostname(WIFI_STA_HOSTNAME);
    WiFi.begin(_settings.getSSID(), _settings.getPSK());
  }
}

void WIFIManager::loop() {
  if (_mode == WIFIManager::AP) {

    _dnsServer->processNextRequest();
    _httpServer->handleClient();

    checks();
  }
}

bool WIFIManager::isSTA() {
  return _mode == WIFIManager::STA;
}

bool WIFIManager::isSTAConnected() {
  return (_mode == WIFIManager::STA) && (WiFi.status() == WL_CONNECTED);
}


void WIFIManager::handleRoot() {
  if (!redirect()) {

    _httpServer->sendHeader("Content-Encoding", String("gzip"));
    _httpServer->send_P(200, "text/html; charset=UTF-8", home, home_len);
  }
}

void WIFIManager::handleGetSettings() {

  if (_setupModeTime != 0) {
    _setupModeTime = millis();
  }

  String   json("{\"wifis\":[");
  int      nbWifis = WiFi.scanNetworks();

  if (nbWifis > 0) {
    for (int i = 0; i<nbWifis; i++) {
      if (i > 0) {
        json += ",";
      }

      json += "{\"ssid\":\"";
      json += WiFi.SSID(i);
      json += "\", \"rssi\":\"";
      json += ""; //WiFi.RSSI(i);
      json += "\",\"secure\":";
      json += (WiFi.encryptionType(i) != ENC_TYPE_NONE) ? "true" : "false";
      json += "}";      
    }
  }

  json += "],\"ssid\":\"";
  json += _settings.getSSID();
  json += "\",\"wifipwd\":\"";
  json += _settings.getPSK();
  json += "\",\"host\":\"";
  json += _settings.getBrokerHost();
  json += "\",\"port\":";
  json += _settings.getBrokerPort();
  json += ",\"deviceid\":\"";
  json += _settings.getDeviceID();
  json += "\",\"user\":\"";
  json += _settings.getBrokerUser();
  json += "\",\"pwd\":\"";
  json += _settings.getBrokerPwd();
  json += "\",\"check\":\"";
  json += _checkCode;
  json += "\"}";

  _httpServer->send(200, mime_json, json);
}

void WIFIManager::handlePostSettings() {

  if (!IS_CHECK_PENDING(_checkCode)) {
    if (_httpServer->hasArg("plain")) {
      if ( _settings.readHTTPPOST(_httpServer->arg("plain").c_str()) ) {

        _checkCode        = CHECK_PENDING | CHECK_WIFI;
        _checkLoopCounter = 0; 

        DBG("WiFi.begin %s=%s", _settings.getSSID(), _settings.getPSK());

        WiFi.hostname(WIFI_STA_HOSTNAME);
        WiFi.begin(_settings.getSSID(), _settings.getPSK());

        _httpServer->send(200, mime_json, "{}");

      } else {
        _httpServer->send(200, mime_json, "{\"error\": \"Error in settings data\"}");
      }
    } else {
      _httpServer->send(200, mime_json, "{\"error\": \"Missing settings data\"}");
    }
  } else {
    _httpServer->send(200, mime_json, "{\"error\": \"Previous settings are currently checking\"}");
  }
}

void WIFIManager::handleGetChecks() {

  if (_setupModeTime != 0) {
    _setupModeTime = millis();
  }

  String json("{\"checks\":");
  json += _checkCode;
  json += "}";  

  _httpServer->send(200, mime_json, json);
}

void WIFIManager::handlePostSave() {

  if (_checkCode == CHECK_MQTT) {

    _settings.setMode(CFGSettings::RUNNING);
    if (_settings.save()) {
      _httpServer->send(200, mime_json, "{}");

      // do not restart immediatly otherwise response will not be send
      _checkLoopCounter = 5;
      _checkCode = CHECK_RESTART;

    } else {
      _httpServer->send(200, mime_json, "{\"error\": \"Configuration commit failed\"}");
    }
  } else {
    _httpServer->send(200, mime_json, "{\"error\": \"Settings must pass checks\"}");
  }
}

void WIFIManager::handle404() {
  if (!redirect()) {
    _httpServer->send(404, "text/plain", "");
  }
}

bool WIFIManager::redirect() {
  const String host = _httpServer->hostHeader();
  
  if ((host != "") && host.compareTo(_ip)) {

    _httpServer->sendHeader("Location", String("http://") + _ip, true);
    _httpServer->send ( 302, "text/plain", ""); 
    _httpServer->client().stop();
    
    return true;
  }
  return false;
}

void WIFIManager::checks() {

  if (_checkCode == CHECK_RESTART) {
    _checkLoopCounter--;
    if (_checkLoopCounter <= 0) {
      ESP.restart();
    }
    return ;
  }

  if ((_setupModeTime != 0) && ((millis() - _setupModeTime) > SETUP_TIMEOUT)) {
    _settings.enterMode(CFGSettings::RUNNING);
    return ;
  }

  if (IS_CHECK_PENDING(_checkCode)) {
    if (_checkCode & CHECK_WIFI) {
      wl_status_t wifiStatus = WiFi.status();

      if (wifiStatus == WL_CONNECTED) {

        if (! _checkIPAddress.fromString(_settings.getBrokerHost())) { 
          err_t err = dns_gethostbyname(_settings.getBrokerHost(), &_checkip, &check_dns_found_callback, &_checkIPAddress);
          if (err == ERR_OK) {
              _checkIPAddress = IPAddress(_checkip);
              _checkCode = CHECK_PENDING | CHECK_TCP;

          } else if (err == ERR_INPROGRESS) {
              _checkCode = CHECK_PENDING | CHECK_HOST;

          } else {
                _checkCode &= ~CHECK_PENDING;
                _checkCode |= CHECK_ERROR;
          }
        } //  else  host is an ip
        
        _checkCode = CHECK_PENDING | CHECK_HOST;
        _checkLoopCounter = 0;

      } else if (wifiStatus == WL_CONNECT_FAILED) {
        _checkCode = CHECK_ERROR | CHECK_WIFI;
      }

    } else if (_checkCode & CHECK_HOST) {
      if (_checkIPAddress.isSet()) {
        if (_checkIPAddress != IPADDR_BROADCAST) {
          _checkClient = new WiFiClient();
          _checkClient->connect(_checkIPAddress, _settings.getBrokerPort());

          _checkCode = CHECK_PENDING | CHECK_TCP;
          _checkLoopCounter = 0;

        } else {
          _checkCode &= ~CHECK_PENDING;
          _checkCode |= CHECK_ERROR;
        }
      } 
    } else if (_checkCode & CHECK_TCP) {
      if (_checkClient->connected()) {
        _checkPacket = new MQTTPacket(_checkClient);
        _checkPacket->buildConnect(_settings.getDeviceID(), _settings.getBrokerUser(), _settings.getBrokerPwd());
        
        _checkPacket->send();

        _checkCode = CHECK_PENDING | CHECK_MQTT;
        _checkLoopCounter = 0;
      }

    } else if (_checkCode & CHECK_MQTT) {
      if (_checkClient->available()) {
        _checkPacket->receive();

        if ( _checkPacket->getType() == MQTTPacket::Type::CONNACK) {
          uint8_t result;
    
          _checkPacket->read8bits(); // Connect Acknowledge flags
          result = _checkPacket->read8bits();
          if (result != 0x00) {
            _checkCode |= CHECK_ERROR | (CHECK_EXTRA_CODE & result);
          }

        } else {
          _checkCode |= CHECK_ERROR;
        }

        _checkCode &= ~CHECK_PENDING;
      }
    }

    _checkLoopCounter ++;
    if (_checkLoopCounter >= CHECK_PENDING_LOOP_MAX) {
      _checkCode &= ~CHECK_PENDING;
      _checkCode |= CHECK_ERROR;
    }

    if (!IS_CHECK_PENDING(_checkCode)) {
      _checkLoopCounter = 0;
      _checkIPAddress   = static_cast<uint32_t>(0);

      if (_checkPacket != NULL) {
        delete _checkPacket;
        _checkPacket = NULL;
      }

      if (_checkClient != NULL) {
        _checkClient->stop();
        delete _checkClient;
        _checkClient = NULL;
      }

      WiFi.disconnect(true);
    }
  }
}



void check_dns_found_callback(const char *name, CONST ip_addr_t *ipaddr, void *callback_arg)
{
  if (ipaddr) {
    (*reinterpret_cast<IPAddress*>(callback_arg)) = IPAddress(ipaddr);
  } else {
    (*reinterpret_cast<IPAddress*>(callback_arg)) = IPADDR_BROADCAST;
  }
}
