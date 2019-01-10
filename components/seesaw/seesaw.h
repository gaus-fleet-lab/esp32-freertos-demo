#ifndef ESP32_TFT_LIBRARY_SEESAW_H
#define ESP32_TFT_LIBRARY_SEESAW_H

#include "esp_err.h"
#include <esp_types.h>


#define I2C_SEESAW_NUM                     I2C_NUM_1        /*!< I2C port number for master dev */
#define I2C_EXAMPLE_MASTER_SCL_IO          22               /*!< gpio number for I2C master clock */
#define I2C_EXAMPLE_MASTER_SDA_IO          23               /*!< gpio number for I2C master data  */
#define I2C_EXAMPLE_MASTER_FREQ_HZ         100000           /*!< I2C master clock frequency */
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE  0                /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE  0                /*!< I2C master do not need buffer */

#define ACK_CHECK_EN                       0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                      0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                            0x0              /*!< I2C ack value */
#define NACK_VAL                           0x1              /*!< I2C nack value */

#define WRITE_BIT                          I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT                           I2C_MASTER_READ  /*!< I2C master read */

/*=========================================================================
    I2C ADDRESS/BITS
    -----------------------------------------------------------------------*/
#define SEESAW_ADDRESS                (0x5E) ///< Default TFTWING_ADDR
/*=========================================================================*/

/*=========================================================================
    REGISTERS
    -----------------------------------------------------------------------*/

/** Module Base Addreses
 *  The module base addresses for different seesaw modules.
 */
enum
{
  SEESAW_STATUS_BASE = 0x00,
  SEESAW_GPIO_BASE = 0x01,
  SEESAW_SERCOM0_BASE = 0x02,

  SEESAW_TIMER_BASE = 0x08,
  SEESAW_ADC_BASE = 0x09,
  SEESAW_DAC_BASE = 0x0A,
  SEESAW_INTERRUPT_BASE = 0x0B,
  SEESAW_DAP_BASE = 0x0C,
  SEESAW_EEPROM_BASE = 0x0D,
  SEESAW_NEOPIXEL_BASE = 0x0E,
  SEESAW_TOUCH_BASE = 0x0F,
  SEESAW_KEYPAD_BASE = 0x10,
  SEESAW_ENCODER_BASE = 0x11,
};

/** GPIO module function addres registers
 */
enum
{
  SEESAW_GPIO_DIRSET_BULK = 0x02,
  SEESAW_GPIO_DIRCLR_BULK = 0x03,
  SEESAW_GPIO_BULK = 0x04,
  SEESAW_GPIO_BULK_SET = 0x05,
  SEESAW_GPIO_BULK_CLR = 0x06,
  SEESAW_GPIO_BULK_TOGGLE = 0x07,
  SEESAW_GPIO_INTENSET = 0x08,
  SEESAW_GPIO_INTENCLR = 0x09,
  SEESAW_GPIO_INTFLAG = 0x0A,
  SEESAW_GPIO_PULLENSET = 0x0B,
  SEESAW_GPIO_PULLENCLR = 0x0C,
};

/** status module function addres registers
 */
enum
{
  SEESAW_STATUS_HW_ID = 0x01,
  SEESAW_STATUS_VERSION = 0x02,
  SEESAW_STATUS_OPTIONS = 0x03,
  SEESAW_STATUS_TEMP = 0x04,
  SEESAW_STATUS_SWRST = 0x7F,
};

/** timer module function addres registers
 */
enum
{
  SEESAW_TIMER_STATUS = 0x00,
  SEESAW_TIMER_PWM = 0x01,
  SEESAW_TIMER_FREQ = 0x02,
};

/** ADC module function addres registers
 */
enum
{
  SEESAW_ADC_STATUS = 0x00,
  SEESAW_ADC_INTEN = 0x02,
  SEESAW_ADC_INTENCLR = 0x03,
  SEESAW_ADC_WINMODE = 0x04,
  SEESAW_ADC_WINTHRESH = 0x05,
  SEESAW_ADC_CHANNEL_OFFSET = 0x07,
};

/** Sercom module function addres registers
 */
enum
{
  SEESAW_SERCOM_STATUS = 0x00,
  SEESAW_SERCOM_INTEN = 0x02,
  SEESAW_SERCOM_INTENCLR = 0x03,
  SEESAW_SERCOM_BAUD = 0x04,
  SEESAW_SERCOM_DATA = 0x05,
};

/** neopixel module function addres registers
 */
enum
{
  SEESAW_NEOPIXEL_STATUS = 0x00,
  SEESAW_NEOPIXEL_PIN = 0x01,
  SEESAW_NEOPIXEL_SPEED = 0x02,
  SEESAW_NEOPIXEL_BUF_LENGTH = 0x03,
  SEESAW_NEOPIXEL_BUF = 0x04,
  SEESAW_NEOPIXEL_SHOW = 0x05,
};

/** touch module function addres registers
 */
enum
{
  SEESAW_TOUCH_CHANNEL_OFFSET = 0x10,
};

/** keypad module function addres registers
 */
enum
{
  SEESAW_KEYPAD_STATUS = 0x00,
  SEESAW_KEYPAD_EVENT = 0x01,
  SEESAW_KEYPAD_INTENSET = 0x02,
  SEESAW_KEYPAD_INTENCLR = 0x03,
  SEESAW_KEYPAD_COUNT = 0x04,
  SEESAW_KEYPAD_FIFO = 0x10,
};

/** keypad module edge definitions
 */
enum
{
  SEESAW_KEYPAD_EDGE_HIGH = 0,
  SEESAW_KEYPAD_EDGE_LOW,
  SEESAW_KEYPAD_EDGE_FALLING,
  SEESAW_KEYPAD_EDGE_RISING,
};

/** encoder module edge definitions
 */
enum
{
  SEESAW_ENCODER_STATUS = 0x00,
  SEESAW_ENCODER_INTENSET = 0x02,
  SEESAW_ENCODER_INTENCLR = 0x03,
  SEESAW_ENCODER_POSITION = 0x04,
  SEESAW_ENCODER_DELTA = 0x05,
};

#define ADC_INPUT_0_PIN 2 ///< default ADC input pin
#define ADC_INPUT_1_PIN 3 ///< default ADC input pin
#define ADC_INPUT_2_PIN 4 ///< default ADC input pin
#define ADC_INPUT_3_PIN 5 ///< default ADC input pin

#define PWM_0_PIN 4 ///< default PWM output pin
#define PWM_1_PIN 5 ///< default PWM output pin
#define PWM_2_PIN 6 ///< default PWM output pin
#define PWM_3_PIN 7 ///< default PWM output pin


#define SEESAW_HW_ID_CODE			0x55 ///< seesaw HW ID code
#define SEESAW_EEPROM_I2C_ADDR 0x3F ///< EEPROM address of i2c address to start up with (for devices that support this feature)

/*=========================================================================*/

/*=========================================================================
    GPIO FUNCTIONS

    Normally these are found in Arduino.h but for now we'll keep them here.
    -----------------------------------------------------------------------*/

//Fixme: @stelau cleanup unused types.
#define INPUT             0x00
#define INPUT_PULLUP      0x02
#define INPUT_PULLDOWN_16 0x04 // PULLDOWN only possible for pin16
#define OUTPUT            0x01
#define OUTPUT_OPEN_DRAIN 0x03
#define WAKEUP_PULLUP     0x05
#define WAKEUP_PULLDOWN   0x07
#define SPECIAL           0xF8 //defaults to the usable BUSes uart0rx/tx uart1tx and hspi
#define FUNCTION_0        0x08
#define FUNCTION_1        0x18
#define FUNCTION_2        0x28
#define FUNCTION_3        0x38
#define FUNCTION_4        0x48
/*=========================================================================*/

//Fixme: @stelau This comes from seesaw lib... but appears to conflict with original definitions in arduino.h not that we use open drain...
//Likely can remove the open drain when removing unused later...
#ifndef INPUT_PULLDOWN
#define INPUT_PULLDOWN 0x03 ///< for compatibility with platforms that do not already define INPUT_PULLDOWN
#endif

#define SEESAW_DEFAULT_DELAY 125

void Adafruit_seesaw_init();
bool Adafruit_seesaw_begin();

uint32_t Adafruit_seesaw_getVersion();
void Adafruit_seesaw_SWReset();

uint8_t Adafruit_seesaw_read8(uint8_t regHigh, uint8_t regLow, uint16_t delay);
esp_err_t  Adafruit_seesaw_read(uint8_t regHigh, uint8_t regLow, uint8_t *buf, uint8_t num, uint16_t delay);
void Adafruit_seesaw_write8(uint8_t regHigh, uint8_t regLow, uint8_t value);
esp_err_t Adafruit_seesaw_write(uint8_t regHigh, uint8_t regLow, uint8_t *buf, uint8_t num);

void Adafruit_seesaw_pinMode(uint8_t pin, uint8_t mode);
void Adafruit_seesaw_digitalWrite(uint8_t pin, uint8_t value);
bool Adafruit_seesaw_digitalRead(uint8_t pin);
uint32_t Adafruit_seesaw_digitalReadBulk(uint32_t pins);
uint32_t Adafruit_seesaw_digitalReadBulkB(uint32_t pins);

void Adafruit_seesaw_pinModeBulk(uint32_t pins, uint8_t mode);
void Adafruit_seesaw_pinModeBulkAB(uint32_t pinsa, uint32_t pinsb, uint8_t mode);

void Adafruit_seesaw_digitalWriteBulk(uint32_t pins, uint8_t value);
void Adafruit_seesaw_digitalWriteBulkAB(uint32_t pinsa, uint32_t pinsb, uint8_t value);


#endif //ESP32_TFT_LIBRARY_SEESAW_H
