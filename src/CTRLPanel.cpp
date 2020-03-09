
#include "CTRLPanel.h"
#include "Debug.h"
#include "config.h"


/*
 * Frame constant / Serial frame on 16bits.
 * 
 * 
 * 
 *           |-------------------------------------------------------------------------------|
 * BIT       | 15 | 14 | 13 | 12 | 11 | 10 |  9 |  8 |  7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
 * 
 * DISPLAY   | DP |  0 |  A |  B | S3 |  D |  C |  0 |  E | S1 | S2 |  G |  F | S4 |  0 |  0 |
 * 
 * LED       |  X |  1 |  x |  x |  x |  x |  x |  x |  x |  x |  x |  x |  x |  x |  x |  x |
 * 
 * 
 * S1 | S2 | S3 | S4  =>  7-segment display     A
 *                                            -----
 *                                           |     |  
 *                                         F |     | B
 *                                           |     |
 *                                            --G--
 *                                           |     |
 *                                         E |     | C
 *                                           |     |
 *                                            -----    .DP
 *                                              D
 *                                              
 *                                              
 * 
 */

#define FRAME_BITS_SIZE           16

#define FRAME_CUE                 0x0100
#define FRAME_BP                  0xB58A
 
#define FRAME_DISPLAY_1           0x0040
#define FRAME_DISPLAY_2           0x0020
#define FRAME_DISPLAY_3           0x0800
#define FRAME_DISPLAY_4           0x0004
#define FRAME_DISPLAY             (FRAME_DISPLAY_1 | FRAME_DISPLAY_2 | FRAME_DISPLAY_3 | FRAME_DISPLAY_4)

#define FRAME_DISPLAY_FRAGMENT_A  0x2000
#define FRAME_DISPLAY_FRAGMENT_B  0x1000
#define FRAME_DISPLAY_FRAGMENT_C  0x0200
#define FRAME_DISPLAY_FRAGMENT_D  0x0400
#define FRAME_DISPLAY_FRAGMENT_E  0x0080
#define FRAME_DISPLAY_FRAGMENT_F  0x0008
#define FRAME_DISPLAY_FRAGMENT_G  0x0010
#define FRAME_DISPLAY_FRAGMENT_DP 0x8000

#define FRAME_DISPLAY_DIGIT_MASK  (FRAME_DISPLAY_FRAGMENT_A|FRAME_DISPLAY_FRAGMENT_B|FRAME_DISPLAY_FRAGMENT_C|FRAME_DISPLAY_FRAGMENT_D|FRAME_DISPLAY_FRAGMENT_E|FRAME_DISPLAY_FRAGMENT_F|FRAME_DISPLAY_FRAGMENT_G)

#define FRAME_DISPLAY_DIGIT0      (FRAME_DISPLAY_FRAGMENT_A | FRAME_DISPLAY_FRAGMENT_B | FRAME_DISPLAY_FRAGMENT_C | FRAME_DISPLAY_FRAGMENT_D | FRAME_DISPLAY_FRAGMENT_E | FRAME_DISPLAY_FRAGMENT_F)
#define FRAME_DISPLAY_DIGIT1      (FRAME_DISPLAY_FRAGMENT_B | FRAME_DISPLAY_FRAGMENT_C)
#define FRAME_DISPLAY_DIGIT2      (FRAME_DISPLAY_FRAGMENT_A|FRAME_DISPLAY_FRAGMENT_B|FRAME_DISPLAY_FRAGMENT_G|FRAME_DISPLAY_FRAGMENT_E|FRAME_DISPLAY_FRAGMENT_D)
#define FRAME_DISPLAY_DIGIT3      (FRAME_DISPLAY_FRAGMENT_A|FRAME_DISPLAY_FRAGMENT_B|FRAME_DISPLAY_FRAGMENT_C|FRAME_DISPLAY_FRAGMENT_D|FRAME_DISPLAY_FRAGMENT_G)
#define FRAME_DISPLAY_DIGIT4      (FRAME_DISPLAY_FRAGMENT_F|FRAME_DISPLAY_FRAGMENT_G|FRAME_DISPLAY_FRAGMENT_B|FRAME_DISPLAY_FRAGMENT_C)
#define FRAME_DISPLAY_DIGIT5      (FRAME_DISPLAY_FRAGMENT_A|FRAME_DISPLAY_FRAGMENT_F|FRAME_DISPLAY_FRAGMENT_G|FRAME_DISPLAY_FRAGMENT_C|FRAME_DISPLAY_FRAGMENT_D)
#define FRAME_DISPLAY_DIGIT6      (FRAME_DISPLAY_FRAGMENT_A|FRAME_DISPLAY_FRAGMENT_F|FRAME_DISPLAY_FRAGMENT_E|FRAME_DISPLAY_FRAGMENT_D|FRAME_DISPLAY_FRAGMENT_C|FRAME_DISPLAY_FRAGMENT_G)
#define FRAME_DISPLAY_DIGIT7      (FRAME_DISPLAY_FRAGMENT_A|FRAME_DISPLAY_FRAGMENT_B|FRAME_DISPLAY_FRAGMENT_C)
#define FRAME_DISPLAY_DIGIT8      (FRAME_DISPLAY_FRAGMENT_A|FRAME_DISPLAY_FRAGMENT_B|FRAME_DISPLAY_FRAGMENT_C|FRAME_DISPLAY_FRAGMENT_D|FRAME_DISPLAY_FRAGMENT_E|FRAME_DISPLAY_FRAGMENT_F|FRAME_DISPLAY_FRAGMENT_G)
#define FRAME_DISPLAY_DIGIT9      (FRAME_DISPLAY_FRAGMENT_A|FRAME_DISPLAY_FRAGMENT_B|FRAME_DISPLAY_FRAGMENT_C|FRAME_DISPLAY_FRAGMENT_D|FRAME_DISPLAY_FRAGMENT_F|FRAME_DISPLAY_FRAGMENT_G)
#define FRAME_DISPLAY_DIGITA      (FRAME_DISPLAY_FRAGMENT_E|FRAME_DISPLAY_FRAGMENT_F|FRAME_DISPLAY_FRAGMENT_A|FRAME_DISPLAY_FRAGMENT_B|FRAME_DISPLAY_FRAGMENT_C|FRAME_DISPLAY_FRAGMENT_G)
#define FRAME_DISPLAY_DIGITC      (FRAME_DISPLAY_FRAGMENT_A|FRAME_DISPLAY_FRAGMENT_F|FRAME_DISPLAY_FRAGMENT_E|FRAME_DISPLAY_FRAGMENT_D)
#define FRAME_DISPLAY_DIGITE      (FRAME_DISPLAY_FRAGMENT_A|FRAME_DISPLAY_FRAGMENT_F|FRAME_DISPLAY_FRAGMENT_E|FRAME_DISPLAY_FRAGMENT_D|FRAME_DISPLAY_FRAGMENT_G)
#define FRAME_DISPLAY_DIGITF      (FRAME_DISPLAY_FRAGMENT_E|FRAME_DISPLAY_FRAGMENT_F|FRAME_DISPLAY_FRAGMENT_A|FRAME_DISPLAY_FRAGMENT_G)

#define FRAME_LED                 0x4000
#define FRAME_LED_POWER           0x0001
#define FRAME_LED_BUBBLE          0x0400
#define FRAME_LED_FILTER          0x1000
#define FRAME_LED_HEATER          0x0000
#define FRAME_LED_HEATREACHED     0x0200

#define FRAME_BEEP_BIT            0x0100

#define DISPLAY_DROP_FRAME_DELAY  100

#define DISPLAY_DIGIT1            0x0008
#define DISPLAY_DIGIT2            0x0004
#define DISPLAY_DIGIT3            0x0002
#define DISPLAY_DIGIT4            0x0001
#define DISPLAY_ALLDIGITS         DISPLAY_DIGIT1 | DISPLAY_DIGIT2 | DISPLAY_DIGIT3 | DISPLAY_DIGIT4

#define DIGITS2UINT(v)            ((((v >> 12) & 0x000F)*100) + (((v >> 8) & 0x000F)*10) + ((v >> 4) & 0x000F))
#define NO_ERROR_ON_DISPLAY(v)    ((v & 0xF000) != 0xE000)

#define UINT8_TRUE                0x01
#define UINT8_FALSE               0x00

#define UNSET_VALUE               0xFFFF
#define UNSET_VALUEUINT8          0xFF


#define RST_GLITCH_COUNTER        2000
#define MIN_GLITCH_COUNTER        1000
#define PREVENT_GLITCH_VALUE(src, dest, cnt)  if (dest != src) { if ((frameCounter - cnt) > RST_GLITCH_COUNTER) { cnt = frameCounter; } else if ((frameCounter - cnt) > MIN_GLITCH_COUNTER) { dest = src; }}


#define COMMAND_ADDRESS_S0        0x00
#define COMMAND_ADDRESS_S1        0x01
#define COMMAND_ADDRESS_S2        0x02
#define COMMAND_ADDRESS_S3        0x03
#define COMMAND_ADDRESS_S4        0x04

#define BUTTON_POWER              COMMAND_ADDRESS_S0
#define BUTTON_TEMPUP             COMMAND_ADDRESS_S1
#define BUTTON_TEMPDOWN           COMMAND_ADDRESS_S2
#define BUTTON_FILTER             COMMAND_ADDRESS_S3
#define BUTTON_HEATER             COMMAND_ADDRESS_S4

#define BUTTON_HOLD_PRESSED_MS    300
#define BUTTON_INTERVAL_MS        500

#define MIN_SET_DESIRED_TEMPERATURE   20
#define MAX_SET_DESIRED_TEMPERATURE   40

CTRLPanel  *CTRLPanel::getInstance() {
  if (instance == NULL) {
    instance = new CTRLPanel();
  }

  return instance;
}

unsigned int CTRLPanel::getFrameCounter() {
  return frameCounter;
}

unsigned int CTRLPanel::getFrameDropped() {
  return frameDropped;  
}
    
uint16_t CTRLPanel::getWaterTemperatureCelsius() {
  return (waterTemp != UNSET_VALUE) ?
    convertDisplayToCelsius(waterTemp) : UNSET_VALUE;
}

uint16_t CTRLPanel::getDesiredTemperatureCelsius() {
  return (desiredTemp != UNSET_VALUE) ?
    convertDisplayToCelsius(desiredTemp) : UNSET_VALUE;
}

boolean CTRLPanel::hasError() {
  return !NO_ERROR_ON_DISPLAY(displayValue);
}

uint16_t CTRLPanel::getError() {
  return hasError() ?
    DIGITS2UINT(displayValue) : 0;
}

uint16_t  CTRLPanel::getRawStatus() {
  return (ledStatus != UNSET_VALUE) ? ledStatus & ~FRAME_BEEP_BIT : UNSET_VALUE;  
}

uint8_t CTRLPanel::isPowerOn() {
  return (ledStatus != UNSET_VALUE) ? ((ledStatus & FRAME_LED_POWER) ? UINT8_TRUE : UINT8_FALSE) : UNSET_VALUEUINT8;
}

uint8_t CTRLPanel::isFilterOn() {
  return (ledStatus != UNSET_VALUE) ? ((ledStatus & FRAME_LED_FILTER) ? UINT8_TRUE : UINT8_FALSE) : UNSET_VALUEUINT8;
}

uint8_t CTRLPanel::isBubbleOn() {
  return (ledStatus != UNSET_VALUE) ? ((ledStatus & FRAME_LED_BUBBLE) ? UINT8_TRUE : UINT8_FALSE) : UNSET_VALUEUINT8;
}

uint8_t CTRLPanel::isHeaterOn() {
  return (ledStatus != UNSET_VALUE) ? ((ledStatus & FRAME_LED_HEATER) ? UINT8_TRUE : UINT8_FALSE) : UNSET_VALUEUINT8;
}

uint8_t CTRLPanel::isHeatReached() {
  return (ledStatus != UNSET_VALUE) ? ((ledStatus & FRAME_LED_HEATREACHED) ? UINT8_TRUE : UINT8_FALSE) : UNSET_VALUEUINT8;
}



boolean CTRLPanel::setDesiredTemperatureCelsius(uint16_t temp) {
  if ((temp >= MIN_SET_DESIRED_TEMPERATURE) && (temp <= MAX_SET_DESIRED_TEMPERATURE)) {
    if (isPowerOn()) {
      uint16_t  uint16DesiredTemp;

      while (getDesiredTemperatureCelsius() == UNSET_VALUE) {
        DBG("CTRL: current desired temperature unknown");
        pushButton(BUTTON_TEMPDOWN);
        delay(BUTTON_INTERVAL_MS);
      }

      while ((uint16DesiredTemp = getDesiredTemperatureCelsius()) != temp) {
        DBG("CTRL: current desired temperature= %d", uint16DesiredTemp);

        if (uint16DesiredTemp > temp) {
          pushButton(BUTTON_TEMPDOWN);
        } else {
          pushButton(BUTTON_TEMPUP);
        }
        delay(BUTTON_INTERVAL_MS);
      }
    }
    return true;
  }
  return false;
}

boolean CTRLPanel::setPowerOn(bool v) {
  if (v  ^  (isPowerOn() == UINT8_TRUE)) {
    pushButton(BUTTON_POWER);
  } 
  return true;
}

boolean CTRLPanel::setFilterOn(bool v) {
  if (v ^ (isFilterOn() == UINT8_TRUE)) {
    pushButton(BUTTON_FILTER);
  }
  return true;
}

boolean CTRLPanel::setHeaterOn(bool v) {
  if (v ^ (isHeaterOn() == UINT8_TRUE)) {
    pushButton(BUTTON_HEATER);
  }
  return true;
}

/***** PRIVATE *******************************************************************************/
CTRLPanel* CTRLPanel::instance   = NULL;
    
volatile uint16_t CTRLPanel::frameValue = 0;
volatile uint16_t CTRLPanel::frameShift = 0;
volatile uint32_t CTRLPanel::frameCounter = 0;
volatile uint32_t CTRLPanel::frameDropped  = 0;
    
volatile uint16_t CTRLPanel::ledStatus     = UNSET_VALUE;
volatile uint16_t CTRLPanel::displayValue  = UNSET_VALUE;
volatile uint16_t CTRLPanel::waterTemp   = UNSET_VALUE;
volatile uint16_t CTRLPanel::desiredTemp   = UNSET_VALUE;

volatile uint16_t CTRLPanel::lastWaterTemp = UNSET_VALUE;

volatile uint16_t CTRLPanel::unsetDigits   = DISPLAY_ALLDIGITS;

volatile uint32_t CTRLPanel::lastBlackDisplayFrameCounter = 0;
volatile bool     CTRLPanel::isDisplayBlink = false;

volatile uint32_t CTRLPanel::lastWaterTempChangeFrameCounter;
volatile uint32_t CTRLPanel::lastDesiredTempChangeFrameCounter;
volatile uint32_t CTRLPanel::lastLedStatusChangeFrameCounter;


CTRLPanel::CTRLPanel() {
  pinMode(DATA_PIN, INPUT);
  pinMode(CLCK_PIN, INPUT);
  pinMode(HOLD_PIN, INPUT);

  attachInterrupt(digitalPinToInterrupt(CLCK_PIN), CTRLPanel::clckRisingInterrupt, RISING); 
  attachInterrupt(digitalPinToInterrupt(HOLD_PIN), CTRLPanel::holdRisingInterrupt, RISING);

  pinMode(E_, OUTPUT);
  digitalWrite(E_, HIGH);

  pinMode(S0, OUTPUT);
  digitalWrite(S0, LOW);
  pinMode(S1, OUTPUT);
  digitalWrite(S1, LOW);
  pinMode(S2, OUTPUT);
  digitalWrite(S2, LOW);
}
   
void CTRLPanel::clckRisingInterrupt() {
  frameValue = (frameValue << 1) + ! digitalRead(DATA_PIN);
  frameShift ++;
}
    
void CTRLPanel::holdRisingInterrupt() { 
  frameCounter ++;

  if (frameShift == FRAME_BITS_SIZE) {
    frameShift = 0;
    
    if (frameValue != FRAME_CUE) {
     if (frameValue & FRAME_DISPLAY) {
        byte digit;

        if ((frameValue & FRAME_DISPLAY_DIGIT_MASK) == 0) { // black display
            if (((frameCounter - lastBlackDisplayFrameCounter) > 900)
                && ((frameCounter - lastBlackDisplayFrameCounter) < 1000)) {

              isDisplayBlink = true;      
            }
            lastBlackDisplayFrameCounter  = frameCounter;
            unsetDigits                   = DISPLAY_ALLDIGITS;
            return ;
        }

        switch (frameValue & FRAME_DISPLAY_DIGIT_MASK) {
          case FRAME_DISPLAY_DIGIT0:
            digit = 0x0;
            break;
          case FRAME_DISPLAY_DIGIT1:
            digit = 0x1;
            break;
          case FRAME_DISPLAY_DIGIT2:
            digit = 0x2;
            break;
          case FRAME_DISPLAY_DIGIT3:
            digit = 0x3;
            break;
          case FRAME_DISPLAY_DIGIT4:
            digit = 0x4;
            break;
          case FRAME_DISPLAY_DIGIT5:
            digit = 0x5;
            break;
          case FRAME_DISPLAY_DIGIT6:
            digit = 0x6;
            break;
          case FRAME_DISPLAY_DIGIT7:
            digit = 0x7;
            break;
          case FRAME_DISPLAY_DIGIT8:
            digit = 0x8;
            break;
          case FRAME_DISPLAY_DIGIT9:
            digit = 0x9;
            break;
          case FRAME_DISPLAY_DIGITA:
            digit = 0xA;
            break;
          case FRAME_DISPLAY_DIGITC:
            digit = 0xC;
            break;
          case FRAME_DISPLAY_DIGITE:
            digit = 0xE;
            break;
          case FRAME_DISPLAY_DIGITF:
            digit = 0xF;
            break;
            
          default:
            return ;
        }

        if (frameValue & FRAME_DISPLAY_1) {
          displayValue = (displayValue & 0x0FFF) + (digit << 12);
          unsetDigits &= ~DISPLAY_DIGIT1;
          
        } else if (frameValue & FRAME_DISPLAY_2) {
          displayValue = (displayValue & 0xF0FF) + (digit << 8);
          unsetDigits &= ~DISPLAY_DIGIT2;
          
        } else if (frameValue & FRAME_DISPLAY_3) {
          displayValue = (displayValue & 0xFF0F) + (digit << 4);
          unsetDigits &= ~DISPLAY_DIGIT3;

        } else if (frameValue & FRAME_DISPLAY_4) {
          displayValue = (displayValue & 0xFFF0) + digit;
          unsetDigits &= ~DISPLAY_DIGIT4;
        }

        if ((unsetDigits == 0) && NO_ERROR_ON_DISPLAY(displayValue)) {
          if (isDisplayBlink) { // display is blinking, set desired temperature
            
            if (frameCounter - lastBlackDisplayFrameCounter > 960) { // no blinking since too long
              isDisplayBlink = false;
              unsetDigits = DISPLAY_ALLDIGITS;
              
            } else {
              
              PREVENT_GLITCH_VALUE(displayValue, desiredTemp, lastDesiredTempChangeFrameCounter);
            }
          } else {

            if (displayValue != waterTemp) {
              if ((frameCounter - lastWaterTempChangeFrameCounter) > 5000) {

                lastWaterTempChangeFrameCounter = frameCounter;
                lastWaterTemp = waterTemp;

              } else if ((frameCounter - lastWaterTempChangeFrameCounter) > 3000) {
                if (lastWaterTemp != displayValue) {
                  lastWaterTemp = displayValue;
                  lastWaterTempChangeFrameCounter = frameCounter;
                } else {
                  waterTemp = lastWaterTemp;
                }
              } 
            }
          }
          
        }

      } else if (frameValue & FRAME_LED) {
        PREVENT_GLITCH_VALUE(frameValue, ledStatus, lastLedStatusChangeFrameCounter);
      } 
    }
    // else cue frame
    
  } else { // esp32 misses some bits in frame (performance issue !?)
    frameDropped ++;
    frameShift = 0;
  }
}


void  CTRLPanel::pushButton(int8_t button) {
  digitalWrite(S0, (button & 0x01) ? HIGH : LOW);
  digitalWrite(S1, (button & 0x02) ? HIGH : LOW);
  digitalWrite(S2, (button & 0x04) ? HIGH : LOW);

  digitalWrite(E_, LOW);
  delay(BUTTON_HOLD_PRESSED_MS);
  digitalWrite(E_, HIGH);
}


uint16_t CTRLPanel::convertDisplayToCelsius(uint16_t displayValue) {
/*  
  unsigned int celsiusValue = DIGITS2UINT(displayValue);

  if ((displayValue & 0x000F) == 0x000F) { // convert °F to °C
    celsiusValue = ((celsiusValue - 32) * 5) / 9;
  } 

  return celsiusValue;
*/
  return DIGITS2UINT(displayValue);
}
