#ifndef CTRLPANEL_H
#define CTRLPANEL_H

#include "Arduino.h"

class CTRLPanel {
  
  public:
    static CTRLPanel* getInstance();
    
    uint32_t  getFrameCounter();
    uint32_t  getFrameDropped();

    uint16_t  getWaterTemperatureCelsius();
    uint16_t  getDesiredTemperatureCelsius();

    uint16_t  getRawStatus();
    
    uint8_t   isPowerOn();
    uint8_t   isFilterOn();
    uint8_t   isBubbleOn();
    uint8_t   isHeaterOn();
    uint8_t   isHeatReached();

    bool      hasError();
    uint16_t  getError();

    bool      setDesiredTemperatureCelsius(uint16_t temp);
    bool      setPowerOn(bool v);
    bool      setFilterOn(bool v);
    bool      setHeaterOn(bool v);
    

  private:
    static CTRLPanel*   instance;
    
    static volatile uint16_t  frameValue;
    static volatile uint16_t  frameShift;
    static volatile uint32_t  frameCounter;
    static volatile uint32_t  frameDropped;
    
    static volatile uint16_t  ledStatus;
    static volatile uint16_t  displayValue;
    static volatile uint16_t  waterTemp;
    static volatile uint16_t  desiredTemp;

    static volatile uint16_t  lastWaterTemp;

    static volatile uint32_t  lastBlackDisplayFrameCounter;
    static volatile bool      isDisplayBlink;

    static volatile uint32_t  lastWaterTempChangeFrameCounter;
    static volatile uint32_t  lastDesiredTempChangeFrameCounter;
    static volatile uint32_t  lastLedStatusChangeFrameCounter;
    
    static volatile uint16_t  unsetDigits;

    static volatile uint16_t  blinkDisplay;
    static volatile uint16_t  blinkDelay;


    CTRLPanel();
    static ICACHE_RAM_ATTR void clckRisingInterrupt();
    static ICACHE_RAM_ATTR void holdRisingInterrupt();

    void  pushButton(int8_t button);

    uint16_t  convertDisplayToCelsius(uint16_t displayValue);
};



#endif // CTRLPANEL_H
