#ifndef DEBUG_H
#define DEBUG_H

#include "config.h"

#ifndef DBG_SERIAL_ENABLED
    #ifndef DBG_TCP_ENABLED
        #define NODEBUG
    #endif
#endif 

#ifndef NODEBUG

    #ifdef DBG_TCP_ENABLED
        #include "ESP8266WiFi.h"

        extern WiFiClient clientDebug;

        #define DBG_TCP_CLIENT_PING     "%!PING#"
        #define DBG_TCP_CLIENT_PONG     "%!PONG#"

        #ifdef DBG_SERIAL_ENABLED

            #define DBG(str, ...)   { \
                Serial.printf(str, ##__VA_ARGS__); Serial.println("");  \
                clientDebug.printf(str, ##__VA_ARGS__); clientDebug.println(""); \
            }

            #define DBGNOLN(str, ...)   { \
                Serial.printf(str, ##__VA_ARGS__);  \
                clientDebug.printf(str, ##__VA_ARGS__); \
            }

        #else

            #define DBG(str, ...)   {  clientDebug.printf(str, ##__VA_ARGS__); clientDebug.println("");  }
            #define DBGNOLN(str, ...)   {  clientDebug.printf(str, ##__VA_ARGS__); }

        #endif // DBG_SERIAL_ENABLED

    #else

        #define DBG(str, ...)   { Serial.printf(str, ##__VA_ARGS__); Serial.println(""); }
        #define DBGNOLN(str, ...)   {  Serial.printf(str, ##__VA_ARGS__);   }

    #endif // DBG_TCP_ENABLED

#else 

    #define DBG(str, ...)
    #define DBGNOLN(str, ...)

#endif


#endif // DEBUG_H
