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

#define HTTPCLIENT_1_1_COMPATIBLE   0

#include <ESP8266httpUpdate.h>

#include "Debug.h"
#include "OTAUpdate.h"

bool OTAUpdate::fwImageURL(const char* url, MQTTClient* mqttClient) {
    WiFiClient          wifiClient;
    t_httpUpdate_return ret = ESPhttpUpdate.update(wifiClient, url );

    if (ret == HTTP_UPDATE_FAILED) {
        DBG("OTAUpdate Error %d: %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());

        mqttClient->publish("/spa/sys/update",  ESPhttpUpdate.getLastErrorString().c_str());

        return false;

    } else {
        DBG("OTAUpdate Result: %d", ret);
        return true;
    }
}