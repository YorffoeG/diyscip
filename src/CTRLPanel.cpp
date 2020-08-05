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

#define FRAME_DISPLAY_OFF         0x0000
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
#define FRAME_DISPLAY_DIGITH      (FRAME_DISPLAY_FRAGMENT_B|FRAME_DISPLAY_FRAGMENT_C|FRAME_DISPLAY_FRAGMENT_E|FRAME_DISPLAY_FRAGMENT_F|FRAME_DISPLAY_FRAGMENT_G)

#define DIGITOFF_VALUE            0xB
#define DIGITH_VALUE              0xD

#define FRAME_LED                 0x4000
#define FRAME_LED_POWER           0x0001
#define FRAME_LED_FILTER          0x1000
#define FRAME_LED_HEATER          0x0080
#define FRAME_LED_HEATREACHED     0x0200

#define SSP_FRAME_LED_BUBBLE      0x0400
#define SJB_FRAME_LED_BUBBLE      0x0002

#define FRAME_LED_JET             0x0400
#define FRAME_LED_SANITIZER       0x2000


#define FRAME_BEEP_BIT            0x0100

#define DISPLAY_DROP_FRAME_DELAY  100

#define DISPLAY_DIGIT1            0x0008
#define DISPLAY_DIGIT2            0x0004
#define DISPLAY_DIGIT3            0x0002
#define DISPLAY_DIGIT4            0x0001
#define DISPLAY_ALLDIGITS         DISPLAY_DIGIT1 | DISPLAY_DIGIT2 | DISPLAY_DIGIT3 | DISPLAY_DIGIT4

#define DISPLAY_OFF               0xBBBB
#define DISPLAY_UNIT_F            0x000F
#define DISPLAY_UNIT_C            0x000C


#define DIGITS2UINT(v)            ((((v >> 12) & 0x000F)*100) + (((v >> 8) & 0x000F)*10) + ((v >> 4) & 0x000F))
#define DISPLAY_UNIT(v)           (v & 0x000F)
#define NO_ERROR_ON_DISPLAY(v)    ((v & 0xF000) != 0xE000)
#define DISPLAY2ERROR(v)          ((v >> 4) & 0x0FFF)
#define TEMP_ON_DISPLAY(v)        ((DISPLAY_UNIT(v) == DISPLAY_UNIT_C) || (DISPLAY_UNIT(v) == DISPLAY_UNIT_F))
#define TIMING_ON_DISPLAY(v)      (DISPLAY_UNIT(v) == DIGITH_VALUE)


#define UINT8_TRUE                0x01
#define UINT8_FALSE               0x00

#define UNSET_VALUE               0xFFFF
#define UNSET_VALUEUINT8          0xFF

#define INIT_STABLE_VALUE_COUNTER   10
#define INIT_STABLE_WATER_COUNTER   500 

#define COMMAND_ADDRESS_S0        0x00
#define COMMAND_ADDRESS_S1        0x01
#define COMMAND_ADDRESS_S2        0x02
#define COMMAND_ADDRESS_S3        0x03
#define COMMAND_ADDRESS_S4        0x04
#define COMMAND_ADDRESS_S5        0x05
#define COMMAND_ADDRESS_S6        0x06
#define COMMAND_ADDRESS_S7        0x07

#ifdef PCB_DESIGN_2
  #define U4                      0x40
  #define U5                      0x80

  #define BUTTON_POWER            U5 | COMMAND_ADDRESS_S2
  #define BUTTON_TEMPUP           U5 | COMMAND_ADDRESS_S4

  #define BUTTON_HEATER           U5 | COMMAND_ADDRESS_S7
  #define BUTTON_SANITIZER        U4 | COMMAND_ADDRESS_S0

  #define SSP_BUTTON_TEMPDOWN     U4 | COMMAND_ADDRESS_S7
  #define SSP_BUTTON_FILTER       U4 | COMMAND_ADDRESS_S1

  #define SJB_BUTTON_TEMPDOWN     U5 | COMMAND_ADDRESS_S1
  #define SJB_BUTTON_FILTER       U4 | COMMAND_ADDRESS_S7

#else
  #define BUTTON_POWER              COMMAND_ADDRESS_S0
  #define BUTTON_TEMPUP             COMMAND_ADDRESS_S1
  #define SSP_BUTTON_TEMPDOWN       COMMAND_ADDRESS_S2
  #define SSP_BUTTON_FILTER         COMMAND_ADDRESS_S3
  #define BUTTON_HEATER             COMMAND_ADDRESS_S4

  #define SJB_BUTTON_TEMPDOWN       SSP_BUTTON_TEMPDOWN
  #define SJB_BUTTON_FILTER         SSP_BUTTON_FILTER
#endif

#define BUTTON_HOLD_PRESSED_MS    300
#define BUTTON_INTERVAL_MS        500

#define PUSH_COUNTER_MAX          25

#define MIN_SET_DESIRED_TEMPERATURE   20
#define MAX_SET_DESIRED_TEMPERATURE   40

#define BLINK_DESIRED_FRAME_MAX       600
#define BLINK_RESET_FRAME_MIN         1300

#define UNITCHANGE_FRAME_COUNTER_MAX  2500
#define UNITCHANGE_MIN                5

#define RST_ERROR_FRAME_COUNTER       10000
#define MIN_SANITIZER_FRAME_COUNTER   1500


CTRLPanel  *CTRLPanel::getInstance(bool isSJB) {
  if (instance == NULL) {
    instance = new CTRLPanel(isSJB);
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

uint16_t CTRLPanel::getError() {
  if ((errorValue != 0) && 
      ((frameCounter - lastErrorChangeFrameCounter) > RST_ERROR_FRAME_COUNTER)) {
      // no error displayed since RST_ERROR_FRAME_COUNTER, so error has disappeared
      errorValue = 0;
  }

  return errorValue;
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
  return (ledStatus != UNSET_VALUE) ? ((ledStatus & (isSJBModel ? SJB_FRAME_LED_BUBBLE : SSP_FRAME_LED_BUBBLE)) ? UINT8_TRUE : UINT8_FALSE) : UNSET_VALUEUINT8;
}

uint8_t CTRLPanel::isHeaterOn() {
  if (isSJBModel) {
    return (ledStatus != UNSET_VALUE) ? (!(ledStatus & FRAME_LED_SANITIZER) && ((ledStatus & (FRAME_LED_HEATER | FRAME_LED_HEATREACHED))) ? UINT8_TRUE : UINT8_FALSE) : UNSET_VALUEUINT8;
  } else {
    return (ledStatus != UNSET_VALUE) ? ((ledStatus & (FRAME_LED_HEATER | FRAME_LED_HEATREACHED)) ? UINT8_TRUE : UINT8_FALSE) : UNSET_VALUEUINT8;
  }
}

uint8_t CTRLPanel::isHeatReached() {
  if (isSJBModel) {
    return (ledStatus != UNSET_VALUE) ? (!(ledStatus & FRAME_LED_SANITIZER) && ((ledStatus & FRAME_LED_HEATREACHED)) ? UINT8_TRUE : UINT8_FALSE) : UNSET_VALUEUINT8;
  } else {
    return (ledStatus != UNSET_VALUE) ? ((ledStatus & FRAME_LED_HEATREACHED) ? UINT8_TRUE : UINT8_FALSE) : UNSET_VALUEUINT8;
  }
}

uint16_t CTRLPanel::getSanitizerTime() {
  if ((ledStatus != UNSET_VALUE) && (sanitizerTime != UNSET_VALUE) && 
      ((frameCounter - lastSanitizerFrameCounter) > MIN_SANITIZER_FRAME_COUNTER)) {
    
    // sanitizer must have been set for at least MIN_SANITIZER_FRAME_COUNTER 
    // because ledStatus may not have been set yet

    if (ledStatus & FRAME_LED_SANITIZER) {
        uint16_t time = DIGITS2UINT(sanitizerTime);
        // May happen sanitize time has fake value due to values multiplexing
        return ((time <= 8) && (time > 0)) ? time : UNSET_VALUE;

    } else {

      return 0;
    }

  } else {

    return UNSET_VALUE;
  }
}

boolean CTRLPanel::setSanitizerTime(uint16_t time) {

#ifdef PCB_DESIGN_2
  if ((ledStatus != UNSET_VALUE) && ((ledStatus & FRAME_LED_POWER) != 0) && (errorValue == 0)) {
    uint16_t pushCounter = 0;

    if (time == 0) {

      if ((ledStatus != UNSET_VALUE) && (ledStatus & FRAME_LED_SANITIZER)) {
        do {
          
          pushButton(BUTTON_SANITIZER);
          delay(BUTTON_INTERVAL_MS);

          pushCounter ++;

          // push button till 8H            
        } while ((DIGITS2UINT(sanitizerTime) != 8) && (pushCounter < PUSH_COUNTER_MAX));
        // then push a last time to cancel
        pushButton(BUTTON_SANITIZER);

      } // else already off

    } else if ((time == 3) || (time == 5) || (time == 8)) {
      do {

          pushButton(BUTTON_SANITIZER);
          delay(BUTTON_INTERVAL_MS);

          pushCounter ++;

      } while ((pushCounter < PUSH_COUNTER_MAX) && (DIGITS2UINT(sanitizerTime) != time));

    } else { // Invalid value
      return false;
    }

  }
#endif

  return true;
}

uint8_t CTRLPanel::isJetOn() {
  return (ledStatus != UNSET_VALUE) ? ((ledStatus & FRAME_LED_JET) ? UINT8_TRUE : UINT8_FALSE) : UNSET_VALUEUINT8;
} 


boolean CTRLPanel::setDesiredTemperatureCelsius(uint16_t temp) {
  if ((temp >= MIN_SET_DESIRED_TEMPERATURE) && (temp <= MAX_SET_DESIRED_TEMPERATURE)) {
    if ((ledStatus != UNSET_VALUE) && ((ledStatus & FRAME_LED_POWER) != 0) && (errorValue == 0)) {
      uint16_t  uint16DesiredTemp, pushCounter = 0;

      while ((getDesiredTemperatureCelsius() == UNSET_VALUE) && (pushCounter < PUSH_COUNTER_MAX)) {
        pushButton(BUTTON_TEMPUP);
        delay(BUTTON_INTERVAL_MS);

        pushCounter++;
      }

      while ((pushCounter < PUSH_COUNTER_MAX) && ((uint16DesiredTemp = getDesiredTemperatureCelsius()) != temp)) {
        if (uint16DesiredTemp > temp) {
          pushButton(isSJBModel ? SJB_BUTTON_TEMPDOWN : SSP_BUTTON_TEMPDOWN);
        } else {
          pushButton(BUTTON_TEMPUP);
        }
        delay(BUTTON_INTERVAL_MS);

        pushCounter++;
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
    pushButton(isSJBModel ? SJB_BUTTON_FILTER : SSP_BUTTON_FILTER);
  }
  return true;
}

boolean CTRLPanel::setHeaterOn(bool v) {
  if (v ^ (isHeaterOn() == UINT8_TRUE)) {
    pushButton(BUTTON_HEATER);
  }
  return true;
}

boolean CTRLPanel::isSetupModeTriggered() {
  return (counterTempUnitChanged >= UNITCHANGE_MIN) &&
         (isPowerOn() == UINT8_FALSE);
}

/***** PRIVATE *******************************************************************************/
CTRLPanel* CTRLPanel::instance   = NULL;
    
volatile uint16_t CTRLPanel::frameValue     = 0;
volatile uint16_t CTRLPanel::frameShift     = 0;
volatile uint32_t CTRLPanel::frameCounter   = 0;
volatile uint32_t CTRLPanel::frameDropped   = 0;
    
volatile uint16_t CTRLPanel::ledStatus      = UNSET_VALUE;
volatile uint16_t CTRLPanel::displayValue   = UNSET_VALUE;
volatile uint16_t CTRLPanel::errorValue     = 0;
volatile uint16_t CTRLPanel::waterTemp      = UNSET_VALUE;
volatile uint16_t CTRLPanel::desiredTemp    = UNSET_VALUE;
volatile uint16_t CTRLPanel::sanitizerTime  = UNSET_VALUE;

volatile uint16_t CTRLPanel::unsetDigits   = DISPLAY_ALLDIGITS;

volatile uint32_t CTRLPanel::lastSanitizerFrameCounter          = 0;

volatile uint32_t CTRLPanel::lastBlackDisplayFrameCounter       = 0;
volatile bool     CTRLPanel::isDisplayBlink                     = false;

volatile uint32_t CTRLPanel::lastErrorChangeFrameCounter        = 0;

volatile uint16_t CTRLPanel::latestLedStatus                    = UNSET_VALUE;
volatile uint16_t CTRLPanel::stableLedStatusCounter             = INIT_STABLE_VALUE_COUNTER;

volatile uint16_t CTRLPanel::latestDisplayValue                 = UNSET_VALUE;
volatile uint16_t CTRLPanel::stableDisplayValueCounter          = INIT_STABLE_VALUE_COUNTER;

volatile uint16_t CTRLPanel::latestDesiredTemp                  = UNSET_VALUE;

volatile uint16_t CTRLPanel::latestWaterTemp                    = UNSET_VALUE;
volatile uint16_t CTRLPanel::stableWaterTempCounter             = INIT_STABLE_WATER_COUNTER;

volatile uint8_t  CTRLPanel::lastTempUnit                       = 0;
volatile uint32_t CTRLPanel::lastTempUnitChangeFrameCounter     = 0;
volatile uint16_t CTRLPanel::counterTempUnitChanged             = 0;


CTRLPanel::CTRLPanel(bool isSJB) {
  isSJBModel = isSJB;

  pinMode(DATA_PIN, INPUT);
  pinMode(CLCK_PIN, INPUT);
  pinMode(HOLD_PIN, INPUT);

  attachInterrupt(digitalPinToInterrupt(CLCK_PIN), CTRLPanel::clckRisingInterrupt, RISING); 
  attachInterrupt(digitalPinToInterrupt(HOLD_PIN), CTRLPanel::holdRisingInterrupt, RISING);

#ifdef PCB_DESIGN_2

  pinMode(U4_E_, OUTPUT);
  digitalWrite(U4_E_, HIGH);

  pinMode(U5_E_, OUTPUT);
  digitalWrite(U5_E_, HIGH);

#else

  pinMode(E_, OUTPUT);
  digitalWrite(E_, HIGH);

#endif

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
        byte  digit;

        switch (frameValue & FRAME_DISPLAY_DIGIT_MASK) {
          case FRAME_DISPLAY_OFF:
            digit         = DIGITOFF_VALUE;
            unsetDigits   = 0;
            displayValue  = DISPLAY_OFF;
            break;
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
          case FRAME_DISPLAY_DIGITH:
            digit = DIGITH_VALUE; // happens on display4 when sanitizer time.
            break;
            
          default:
            return ;
        }

        if (frameValue & FRAME_DISPLAY_1) {
          displayValue = (displayValue & 0x0FFF) + (digit << 12);
          unsetDigits &= ~DISPLAY_DIGIT1;

          if (digit == 0xE) { // Display error, digit4 is not set
            displayValue = (displayValue & 0xFFF0);
            unsetDigits &= ~DISPLAY_DIGIT4;
          }
          
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

        if (unsetDigits == 0) {
          unsetDigits = DISPLAY_ALLDIGITS;

          if (displayValue == latestDisplayValue) {
            stableDisplayValueCounter--;
            if (stableDisplayValueCounter == 0) {
              stableDisplayValueCounter = INIT_STABLE_VALUE_COUNTER;

              if (displayValue == DISPLAY_OFF) {
                lastBlackDisplayFrameCounter  = frameCounter;
                isDisplayBlink = true;

                if (latestDesiredTemp != UNSET_VALUE) {
                    desiredTemp = latestDesiredTemp;
                }

              } else {

                if ((frameCounter - lastBlackDisplayFrameCounter) > BLINK_RESET_FRAME_MIN) { // blinking is over
                  isDisplayBlink    = false;
                  latestDesiredTemp = UNSET_VALUE;
                }                

                if (NO_ERROR_ON_DISPLAY(displayValue)) {

                  if (TIMING_ON_DISPLAY(displayValue)) { // sanitizer time
                    sanitizerTime             = displayValue;
                    lastSanitizerFrameCounter = frameCounter;

                  } else if (TEMP_ON_DISPLAY(displayValue)) {

                    if (isDisplayBlink && (errorValue == 0)) { // blinking but not in error !

                      // when blink finished, it displays water temp that should not be confused
                      // with desired temp !
                      // So desired temp is read just after a black screen and set at next black screen

                      if ((frameCounter - lastBlackDisplayFrameCounter) < BLINK_DESIRED_FRAME_MAX) { 
                        latestDesiredTemp = displayValue;
                      }

                    } else { // not blinking

                      if (displayValue == latestWaterTemp) {
                        stableWaterTempCounter--;
                        if (stableWaterTempCounter == 0) {
                          waterTemp = displayValue;
                          stableWaterTempCounter = INIT_STABLE_WATER_COUNTER;
                        }

                      } else {
                        latestWaterTemp = displayValue;
                        stableWaterTempCounter = INIT_STABLE_WATER_COUNTER;
                      }

                      if (DISPLAY_UNIT(displayValue) != lastTempUnit) {
                        if ((frameCounter - lastTempUnitChangeFrameCounter) < UNITCHANGE_FRAME_COUNTER_MAX) {
                          counterTempUnitChanged++;
                        } else {
                          counterTempUnitChanged = 0;
                        }

                        lastTempUnitChangeFrameCounter = frameCounter;
                        lastTempUnit = DISPLAY_UNIT(displayValue);
                      }
                    }
                  }

                } else { // error on display

                  errorValue = DISPLAY2ERROR(displayValue);
                  lastErrorChangeFrameCounter = frameCounter;
                }
              } 
            }

          } else { // displayValue not stable

            // While error, there is a black screen after error display
            // not visible by eye but must not break the stable counter

            if (NO_ERROR_ON_DISPLAY(latestDisplayValue) || (displayValue != DISPLAY_OFF)) {
              latestDisplayValue = displayValue;
              stableDisplayValueCounter = INIT_STABLE_VALUE_COUNTER;
            }
          }

        } // else all digits not yet set

      } else if (frameValue & FRAME_LED) {

        if (frameValue == latestLedStatus) {
            stableLedStatusCounter--;
            if (stableLedStatusCounter == 0) {
              ledStatus = frameValue;
              stableLedStatusCounter = INIT_STABLE_VALUE_COUNTER;
            }
        } else {
          latestLedStatus = frameValue;
          stableLedStatusCounter = INIT_STABLE_VALUE_COUNTER;
        }

      }

    } // else cue frame
    
  } else { // esp8266 misses some bits in frame (performance issue !?)
    frameDropped ++;
    frameShift = 0;
  }
}


void  CTRLPanel::pushButton(int8_t button) {
  digitalWrite(S0, (button & 0x01) ? HIGH : LOW);
  digitalWrite(S1, (button & 0x02) ? HIGH : LOW);
  digitalWrite(S2, (button & 0x04) ? HIGH : LOW);

  #ifdef PCB_DESIGN_2

    if (button & U4) {
      digitalWrite(U4_E_, LOW);
      delay(BUTTON_HOLD_PRESSED_MS);
      digitalWrite(U4_E_, HIGH);

    } else if (button & U5) {
      digitalWrite(U5_E_, LOW);
      delay(BUTTON_HOLD_PRESSED_MS);
      digitalWrite(U5_E_, HIGH);
    }

  #else

    digitalWrite(E_, LOW);
    delay(BUTTON_HOLD_PRESSED_MS);
    digitalWrite(E_, HIGH);

  #endif
}


uint16_t CTRLPanel::convertDisplayToCelsius(uint16_t value) {
 
  uint16_t celsiusValue = DIGITS2UINT(value);

  if (DISPLAY_UNIT(value) == DISPLAY_UNIT_F) { // convert °F to °C
    double fValue = (double)celsiusValue;

    celsiusValue = (uint16_t)round(((fValue - 32) * 5) / 9);

  } else if (DISPLAY_UNIT(value) != DISPLAY_UNIT_C) {

    return UNSET_VALUE;
  }

  return (celsiusValue >= 0) && (celsiusValue <= 60) ? celsiusValue : UNSET_VALUE;
}
