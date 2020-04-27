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

#ifndef CFGSETTINGS_H
#define CFGSETTINGS_H

#include <stdint.h>

#define SSID_LEN_MAX        31
#define PSK_LEN_MAX         63
#define HOST_LEN_MAX        126
#define PORT_LEN_MAX        5
#define DEVICEID_LEN_MAX    63
#define USER_LEN_MAX        63
#define PWD_LEN_MAX         63

class CFGSettings {

public:
    enum MODE {
        NOT_INIT        = 0,

        NOT_CHECKED     = 0x02,
        SETUP           = 0x04,
        RUNNING         = 0X08,

        UNKNOWN         = 0XFF,
    };

    CFGSettings();

    MODE        getMode();
    void        setMode(MODE mode);
    bool        enterMode(MODE mode);

    bool        readHTTPPOST(const char* data);

    bool        save();

    const char* getSSID();
    const char* getPSK();
    const char* getBrokerHost();
    uint16_t    getBrokerPort();
    const char* getDeviceID();
    const char* getBrokerUser();
    const char* getBrokerPwd();

private:
    uint16_t    nextEEPROMField(char* field, uint16_t max_len);
    const char* nextHTTPField(const char* data, char* field, uint16_t max_size);
    uint16_t    writeField(const char* field);

    uint16_t    _datalen                = 0;
    uint16_t    _currentEEPROMAddress   = 0;

    MODE        _mode                           = MODE::NOT_INIT;
    char        _ssid[SSID_LEN_MAX+1]           = {0};
    char        _psk[PSK_LEN_MAX+1]             = {0};
    char        _host[HOST_LEN_MAX+1]           = {0};
    char        _port[PORT_LEN_MAX+1]           = {0};
    char        _deviceID[DEVICEID_LEN_MAX+1]   = {0};
    char        _user[USER_LEN_MAX+1]           = {0};
    char        _pwd[PWD_LEN_MAX+1]             = {0};

};


#endif // CFGSETTINGS_H
