/* 
 * Inspired by
 * https://github.com/tzapu/WiFiManager
 * 
 */

#include "WIFIManager.h"
#include "Debug.h"


#define DNS_PORT    53
#define HTTP_PORT   80


WIFIManager::WIFIManager() {

}

void WIFIManager::setSTASsid(const char* ssid) {
  this->stassid = ssid;
}

void WIFIManager::setSTAPass(const char* pass) {
  this->stapass = pass;
}

void WIFIManager::setSTAHostname(const char* name) {
  this->stahostname = name;
}

void WIFIManager::setAPName(const char* name) {
  this->apname  = name;
}

bool WIFIManager::autostart() {
  // try start SPA or AP 
  return true;
}

bool WIFIManager::startSTA() {
  uint32_t  cnxTimeout = 20;

  WiFi.mode(WIFI_STA);

  if (stahostname) {
    WiFi.hostname(stahostname);
  }
  
  if (stassid) {
    DBG("Try to connect to %s", stassid);
    WiFi.begin(stassid, stapass);
  } else {
    if (WiFi.SSID().length()) {
      DBG("Try to connect to %s", WiFi.SSID().c_str());
      WiFi.begin();
    } else {
      DBG("NO SSID, exit SPA");
      return false;
    }
  }

  while (cnxTimeout) {
    cnxTimeout--;
    delay(500);

    DBG("Waiting for connection");
    
    if (WiFi.status() == WL_CONNECTED) {
      DBG("Connected");
      return true;
    }
  }
  
  DBG("SPA Connection failed");
  return false;  
}

bool WIFIManager::startAP() {

  DBG("Starting AP: %s", apname);
  
  dnsServer   = new DNSServer();
  httpServer  = new ESP8266WebServer(HTTP_PORT);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(apname);

  delay(500);

  IPAddress ip = WiFi.softAPIP();
  for (int i = 0; i < 3; i++) {
    apip += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  apip += String(((ip >> 8 * 3)) & 0xFF);

  DBG("AP ip = %s", apip.c_str());

  dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer->start(DNS_PORT, "*", ip);

  httpServer->on(String(F("/")).c_str(), std::bind(&WIFIManager::handleRoot, this));
  httpServer->onNotFound (std::bind(&WIFIManager::handle404, this));

  httpServer->begin();

  while (!exitAP) {
    delay(500);
    
    dnsServer->processNextRequest();
    httpServer->handleClient();
  }

  DBG("Exiting AP");
  return true;
}

void WIFIManager::handleRoot() {
  DBG("HTTP Request /");
  
  if (!redirect()) {
    String home = FPSTR(HOME_HTML);

    //httpServer->sendHeader("Content-Length", String(home.length()));
    httpServer->send(200, "text/html", home);
    //httpServer->client().stop();

    //DBG("HTTP Response: %s", home.c_str());

    //exitAP = true;
  }
}

void WIFIManager::handle404() {
  DBG("HTTP Request 404: %s", httpServer->uri().c_str());
  
  if (!redirect()) {
    httpServer->send(404, "text/plain", "");
  }
}

bool WIFIManager::redirect() {
  const String host = httpServer->hostHeader();
  
  if ((host != "") && host.compareTo(apip)) {
    DBG("Redirect requested host %s to %s", host.c_str(), apip.c_str());
    
    httpServer->sendHeader("Location", String("http://") + apip, true);
    httpServer->send ( 302, "text/plain", ""); 
    httpServer->client().stop();
    
    return true;
  }
  
  DBG("NO Redirect for requested host %s", host.c_str());
  return false;
}

const char WIFIManager::HOME_HTML[] PROGMEM = "<html><body>It works !</body></html>";
