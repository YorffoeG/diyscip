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

#include <EEPROM.h>

#include "CFGSettings.h"
#include "Debug.h"

#define EEPROM_SIZE         512
#define INIT_SEQUENCE       "iN1t"
#define INIT_SEQUENCE_MAX   4


/**
 * 
 *  EEPROM address
 * 
 * |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  | ..... 
 * |     |     |     |     |           |     |       
 * |  i     N     1     t  |  datalen  | mode| ssid\0  psk\0  host\0  port\0  deviceID\0  user\0  pwd\0 
 * 
 * 
 * 
 */ 


#define INIT_SEQUENCE_ADDRESS   0
#define DATA_LEN_ADDRESS        4
#define MODE_ADDRESS            6
#define DATA_ADDRESS            7

CFGSettings::CFGSettings() {
    EEPROM.begin(EEPROM_SIZE);

    for (int i=0; i<INIT_SEQUENCE_MAX; i++) {
        if ((uint8_t)(INIT_SEQUENCE[i]) != (uint8_t)EEPROM.read(INIT_SEQUENCE_ADDRESS + i)) {
            // EEPROM does not contain init sequence
            return;
        }
    }

    this->_datalen  = (EEPROM.read(DATA_LEN_ADDRESS) << 8) + EEPROM.read(DATA_LEN_ADDRESS+1);
    this->_mode    = (MODE)EEPROM.read(MODE_ADDRESS);

    if (this->_mode > MODE::RUNNING) {
        this->_mode = MODE::UNKNOWN;
    }

    uint16_t len = INIT_SEQUENCE_MAX + 3;       // 3 = datalen(uint16) + mode(uint8)
    this->_currentEEPROMAddress = DATA_ADDRESS; 

    len += this->nextEEPROMField(this->_ssid,       SSID_LEN_MAX);
    len += this->nextEEPROMField(this->_psk,        PSK_LEN_MAX);
    len += this->nextEEPROMField(this->_host,       HOST_LEN_MAX);
    len += this->nextEEPROMField(this->_port,       PORT_LEN_MAX);
    len += this->nextEEPROMField(this->_deviceID,   DEVICEID_LEN_MAX);
    len += this->nextEEPROMField(this->_user,       USER_LEN_MAX);
    len += this->nextEEPROMField(this->_pwd,        PWD_LEN_MAX);
    len += this->nextEEPROMField(this->_update,     UPDATE_LEN_MAX);
}

CFGSettings::MODE CFGSettings::getMode() {
    return this->_mode;
}

void CFGSettings::setMode(MODE mode) {
    this->_mode = mode;
}

bool CFGSettings::enterMode(MODE mode) {

    if ((mode == CFGSettings::SETUP) || (mode == CFGSettings::RUNNING)) {
        EEPROM.write(MODE_ADDRESS, mode);

        if (EEPROM.commit()) {
            ESP.restart();
            return true;
        }
    }

    return false;
}

bool CFGSettings::readHTTPPOST(const char* data) {
    
    data = this->nextHTTPField(data, this->_ssid,   SSID_LEN_MAX);
    data = this->nextHTTPField(data, this->_psk,    PSK_LEN_MAX);
    data = this->nextHTTPField(data, this->_host,   HOST_LEN_MAX);
    data = this->nextHTTPField(data, this->_port,   PORT_LEN_MAX);
    data = this->nextHTTPField(data, this->_deviceID, DEVICEID_LEN_MAX);
    data = this->nextHTTPField(data, this->_user,   USER_LEN_MAX);
    data = this->nextHTTPField(data, this->_pwd,    PWD_LEN_MAX);
    data = this->nextHTTPField(data, this->_update, UPDATE_LEN_MAX);

    return !strcmp(data, "EOD");
}

bool CFGSettings::save() {
    uint16_t  len = DATA_ADDRESS, i;

    for (i=0; i<INIT_SEQUENCE_MAX; i++) {
        EEPROM.write(INIT_SEQUENCE_ADDRESS + i, INIT_SEQUENCE[i]);
    }

    this->_currentEEPROMAddress = MODE_ADDRESS;
    EEPROM.write(this->_currentEEPROMAddress++, this->_mode);

    len += this->writeField(this->_ssid);
    len += this->writeField(this->_psk);
    len += this->writeField(this->_host);
    len += this->writeField(this->_port);
    len += this->writeField(this->_deviceID);
    len += this->writeField(this->_user);
    len += this->writeField(this->_pwd);
    len += this->writeField(this->_update);

    if (this->_currentEEPROMAddress < EEPROM_SIZE) {

        EEPROM.write(4, (uint8_t)(len >> 8));
        EEPROM.write(5, (uint8_t)(len & 0xFF));

        return EEPROM.commit();

    } else {
        return false;
    }
}


const char* CFGSettings::getSSID() {
    return this->_ssid;
}

const char* CFGSettings::getPSK() {
    return this->_psk;
}

const char* CFGSettings::getBrokerHost() {
    return this->_host;
}

uint16_t CFGSettings::getBrokerPort() {
    char*    value = this->_port;
    uint16_t uint16Value = 0;

    while (*value != 0) {
        if (*value >= '0' && *value <= '9') {

          uint16Value = uint16Value*10 + (*value - '0');
          value ++;

        } else {
          DBG("BrokerPort wrong uint16 value");
          break;
        }
    } 

    return uint16Value;
}

const char* CFGSettings::getDeviceID() {
    return this->_deviceID;
}

const char* CFGSettings::getBrokerUser() {
    return this->_user;
}

const char* CFGSettings::getBrokerPwd() {
    return this->_pwd;
}

bool CFGSettings::isUpdateEnabled() {
    return *(this->_update) != '0';
}


///// PRIVATE METHODS


uint16_t  CFGSettings::nextEEPROMField(char* field, uint16_t max_size) {
    int     len = 0;
    uint8_t byte;

    while ((this->_currentEEPROMAddress < this->_datalen) && (len < max_size)) {
        byte = EEPROM.read(this->_currentEEPROMAddress++);
        if (byte != '\0') {
            field[len++] = byte;
        } else {
            break;
        }
    }

    field[len] = '\0';
    return len;
}

const char* CFGSettings::nextHTTPField(const char* data, char* field, uint16_t max_size) {
    int     len = 0;
    char    c;

    while (((c = *data) != '\0') && (len < max_size)) {
        data++;
        if (c != '\n') {
            field[len++] = c;
        } else {
            break;
        }
    }

    field[len] = '\0';
    return data;
}

uint16_t  CFGSettings::writeField(const char* field) {
    uint16_t len = 0;

    do {
        if (this->_currentEEPROMAddress >= EEPROM_SIZE) {
            break;
        }

        EEPROM.write(this->_currentEEPROMAddress++, field[len++]);

    } while (field[len-1] != '\0');

    return len;
}

