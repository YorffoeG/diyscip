#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

class WIFIManager {

  public:
    WIFIManager();

    void setSTASsid(const char* ssid);
    void setSTAPass(const char* ssid);
    void setSTAHostname(const char* hostname);

    void setAPName(const char* name);    

    bool autostart();
    bool startSTA();
    bool startAP();

  private:
    static const char HOME_HTML[] PROGMEM;
  
    DNSServer*        dnsServer;
    ESP8266WebServer* httpServer;

    const char* stassid     = NULL;
    const char* stapass     = NULL;
    const char* stahostname = NULL;
    
    const char* apname      = NULL;

    String  apip;
    String  spaip;
    
    bool exitAP  = false;

    void handleRoot();
    void handle404();
    bool redirect();
  
};


#endif // WIFIMANAGER_H
