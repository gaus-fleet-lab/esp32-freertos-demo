#include "miniTFTWing.h"
#include "seesaw.h"

/**
 *****************************************************************************************
 *  @brief      Equivilant of constructor
 *
 *  Used to initialize the seesaw's i2c connection
 *
 ****************************************************************************************/
void Adafruit_miniTFTWing_init() {
  Adafruit_seesaw_init();
}

/**************************************************************************/
/*!
    @brief  set up the miniTFTWing

    @returns true on success, false on error
*/
/**************************************************************************/

bool Adafruit_miniTFTWing_begin() {
  if (! Adafruit_seesaw_begin()){
    return false;
  }
  Adafruit_seesaw_pinMode(TFTWING_RESET_PIN, OUTPUT);
  Adafruit_seesaw_pinModeBulk(TFTWING_BUTTON_ALL, INPUT_PULLUP);
  return true;
}

/**************************************************************************/
/*!
    @brief  set the value of the backlight
    @param  value the backlight value to set NOTE: 0xFFFF is all the way on 0x0000 is off.
*/
/**************************************************************************/
void Adafruit_miniTFTWing_setBacklight(uint16_t value) {
  uint8_t cmd[] = {0x00, (uint8_t)(value >> 8), (uint8_t)value};
  Adafruit_seesaw_write(SEESAW_TIMER_BASE, SEESAW_TIMER_PWM, cmd, 3);
}

/**************************************************************************/
/*!
    @brief  set the PWM frequency for the backlight
    @param  freq the frequency to set the backlight to
*/
/**************************************************************************/
void Adafruit_miniTFTWing_setBacklightFreq(uint16_t freq){
  uint8_t cmd[] = {0x0, (uint8_t)(freq >> 8), (uint8_t)freq};
  Adafruit_seesaw_write(SEESAW_TIMER_BASE, SEESAW_TIMER_FREQ, cmd, 3);
}

/**************************************************************************/
/*!
    @brief  reset the TFT screen by setting the value of the reset pin
    @param  rst the value to set the reset pin to
*/
/**************************************************************************/
void Adafruit_miniTFTWing_tftReset(bool rst){
  Adafruit_seesaw_digitalWrite(TFTWING_RESET_PIN, rst);
}

/**************************************************************************/
/*!
    @brief  read all buttons on the wing and return as a 32 bit integer
    @returns the value of the buttons
*/
/**************************************************************************/
uint32_t Adafruit_miniTFTWing_readButtons(){
  return Adafruit_seesaw_digitalReadBulk(TFTWING_BUTTON_ALL);
}
