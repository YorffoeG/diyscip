#ifndef CONFIG_H
#define CONFIG_H

#define OTA_ENABLED
#define DBG_TCP_ENABLED
#define DBG_SERIAL_ENABLED

#define DBG_TCP_PORT        8888
#define SERIAL_DEBUG_SPEED  115200  

#define DATA_PIN    12    /*!< white wire   */
#define CLCK_PIN    14    /*!< yellow wire  */
#define HOLD_PIN    13    /*!< black wire   */

/* connected to CD4051 */
#define E_          16    /* enable at 0 CD4051 output */
#define S0          4
#define S1          5
#define S2          15  

/* temperature sensor */
#define ADC_PIN     A0
#define NTC_R25     8675.0
#define NTC_T25     25.0
#define NTC_BETA    3950.0

#define WIFI_SSID  "Livebox-3A80"
#define WIFI_PWD    "E6E04FA350A7AE17E01EEC047F"

#define MQTT_HOST   "192.168.1.9"
#define MQTT_PORT   8080

#define MQTT_CLIENTID   "E6C70616"


#endif // CONFIG_H
