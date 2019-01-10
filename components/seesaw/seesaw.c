#include "seesaw.h"
#include "driver/i2c.h"

/**
 *****************************************************************************************
 *  @brief      Equivilant of constructor
 *
 *  Used to initialize the seesaw's i2c connection
 *
 ****************************************************************************************/
void Adafruit_seesaw_init() {
  int i2c_master_port = I2C_SEESAW_NUM;
  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = I2C_EXAMPLE_MASTER_FREQ_HZ;
  i2c_param_config(i2c_master_port, &conf);
  i2c_driver_install(i2c_master_port, conf.mode,
                           I2C_EXAMPLE_MASTER_RX_BUF_DISABLE,
                           I2C_EXAMPLE_MASTER_TX_BUF_DISABLE, 0);
}
/**
 *****************************************************************************************
 *  @brief      read 1 byte from the specified seesaw register.
 *
 *  @param      regHigh the module address register (ex. SEESAW_STATUS_BASE)
 *	@param		regLow the function address register (ex. SEESAW_STATUS_VERSION)
 *	@param		delay a number of microseconds to delay before reading out the data.
 *				Different delay values may be necessary to ensure the seesaw chip has
 *				time to process the requested data. Defaults to 125.
 *
 *  @return     the value between 0 and 255 read from the passed register
 ****************************************************************************************/
uint8_t Adafruit_seesaw_read8(uint8_t regHigh, uint8_t regLow, uint16_t delay)
{
  uint8_t ret;
  Adafruit_seesaw_read(regHigh, regLow, &ret, 1, delay);

  return ret;
}

/**
 *****************************************************************************************
 *  @brief      Write 1 byte to the specified seesaw register.
 *
 *  @param      regHigh the module address register (ex. SEESAW_NEOPIXEL_BASE)
 *	@param		regLow the function address register (ex. SEESAW_NEOPIXEL_PIN)
 *	@param		value the value between 0 and 255 to write
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw_write8(uint8_t regHigh, uint8_t regLow, uint8_t value)
{
  Adafruit_seesaw_write(regHigh, regLow, &value, 1);
}


/**
 *****************************************************************************************
 *  @brief      Read a specified number of bytes into a buffer from the seesaw.
 *
 *  @param      regHigh the module address register (ex. SEESAW_STATUS_BASE)
 *	@param		regLow the function address register (ex. SEESAW_STATUS_VERSION)
 *	@param		buf the buffer to read the bytes into
 *	@param		num the number of bytes to read.
 *	@param		delay an optional delay in between setting the read register and reading
 *				out the data. This is required for some seesaw functions (ex. reading ADC data)
 *
 *  @return     none
 ****************************************************************************************/
esp_err_t  Adafruit_seesaw_read(uint8_t regHigh, uint8_t regLow, uint8_t *buf, uint8_t num, uint16_t delay)
{

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (SEESAW_ADDRESS << 1) | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, regHigh, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, regLow, ACK_CHECK_EN);
  i2c_master_stop(cmd);
  esp_err_t ret = i2c_master_cmd_begin(I2C_SEESAW_NUM, cmd, 1000 / portTICK_RATE_MS);
  if(ret!=ESP_OK) {
    printf("write i2c_master_cmd_begin 1 FAILED 0x%x!", ret);
  }
  i2c_cmd_link_delete(cmd);


  //Fixme: Todo: tune this
  vTaskDelay(delay / portTICK_RATE_MS);


  i2c_cmd_handle_t cmd2 = i2c_cmd_link_create();
  i2c_master_start(cmd2);
  i2c_master_write_byte(cmd2, (SEESAW_ADDRESS << 1) | READ_BIT, ACK_CHECK_EN);
  if (num > 1) {
    i2c_master_read(cmd2, buf, num - 1, ACK_VAL);
  }
  i2c_master_read_byte(cmd2, buf + num - 1, NACK_VAL);
  i2c_master_stop(cmd2);
  ret = i2c_master_cmd_begin(I2C_SEESAW_NUM, cmd2, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd2);
  if(ret!=ESP_OK) {
    printf("Adafruit_seesaw_read:: Read i2c_master_cmd_begin 2 FAILED 0x%x!", ret);
  }
  return ret;
}

/**
 *****************************************************************************************
 *  @brief      Write a specified number of bytes to the seesaw from the passed buffer.
 *
 *  @param      regHigh the module address register (ex. SEESAW_GPIO_BASE)
 *	@param		regLow the function address register (ex. SEESAW_GPIO_BULK_SET)
 *	@param		buf the buffer the the bytes from
 *	@param		num the number of bytes to write.
 *
 *  @return     none
 ****************************************************************************************/
esp_err_t Adafruit_seesaw_write(uint8_t regHigh, uint8_t regLow, uint8_t *buf, uint8_t num)
{

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (SEESAW_ADDRESS << 1) | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, regHigh, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, regLow, ACK_CHECK_EN);
  i2c_master_write(cmd, buf, num, ACK_CHECK_EN);
  i2c_master_stop(cmd);
  esp_err_t ret = i2c_master_cmd_begin(I2C_SEESAW_NUM, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  if(ret!=ESP_OK) {
    printf("Adafruit_seesaw_write:: i2c_master_cmd_begin FAILED 0x%x!", ret);
  }
  return ret;
}


/**
 *****************************************************************************************
 *  @brief      Start the seesaw
 *
 *				This should be called when your code is connecting to the seesaw
 *
 *  @param      addr the I2C address of the seesaw
 *  @param      flow the flow control pin to use
 *  @param		reset pass true to reset the seesaw on startup. Defaults to true.
 *
 *  @return     true if we could connect to the seesaw, false otherwise
 ****************************************************************************************/
bool Adafruit_seesaw_begin()
{
  Adafruit_seesaw_SWReset();

  uint8_t c = Adafruit_seesaw_read8(SEESAW_STATUS_BASE, SEESAW_STATUS_HW_ID, SEESAW_DEFAULT_DELAY);
  if(c != SEESAW_HW_ID_CODE) {
    return false;
  }
  return true;
}

/**
 *****************************************************************************************
 *  @brief      perform a software reset. This resets all seesaw registers to their default values.
 *
 *  			This is called automatically from Adafruit_seesaw.begin()
 *
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw_SWReset()
{
  Adafruit_seesaw_write8(SEESAW_STATUS_BASE, SEESAW_STATUS_SWRST, 0xFF);
}

/**
 *****************************************************************************************
 *  @brief      Returns the version of the seesaw
 *
 *  @return     The version code. Bits [31:16] will be a date code, [15:0] will be the product id.
 ****************************************************************************************/
uint32_t Adafruit_seesaw_getVersion()
{
  uint8_t buf[4];
  Adafruit_seesaw_read(SEESAW_STATUS_BASE, SEESAW_STATUS_VERSION, buf, 4, SEESAW_DEFAULT_DELAY);
  uint32_t ret = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) | (uint32_t)buf[3];
  return ret;
}

/**
 *****************************************************************************************
 *  @brief      Set the mode of a GPIO pin.
 *
 *  @param      pin the pin number. On the SAMD09 breakout, this corresponds to the number on the silkscreen.
 *  @param		mode the mode to set the pin. One of INPUT, OUTPUT, or INPUT_PULLUP.
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw_pinMode(uint8_t pin, uint8_t mode)
{
  if(pin >= 32)
    Adafruit_seesaw_pinModeBulkAB(0, 1ul << (pin-32), mode);
  else
    Adafruit_seesaw_pinModeBulk(1ul << pin, mode);
}

/**
 *****************************************************************************************
 *  @brief      Set the output of a GPIO pin
 *
 *  @param      pin the pin number. On the SAMD09 breakout, this corresponds to the number on the silkscreen.
 *	@param		value the value to write to the GPIO pin. This should be HIGH or LOW.
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw_digitalWrite(uint8_t pin, uint8_t value)
{
  if(pin >= 32)
    Adafruit_seesaw_digitalWriteBulkAB(0, 1ul << (pin-32), value);
  else
    Adafruit_seesaw_digitalWriteBulk(1ul << pin, value);
}


/**
 *****************************************************************************************
 *  @brief      Read the current status of a GPIO pin
 *
 *  @param      pin the pin number. On the SAMD09 breakout, this corresponds to the number on the silkscreen.
 *
 *  @return     the status of the pin. HIGH or LOW (1 or 0).
 ****************************************************************************************/
bool Adafruit_seesaw_digitalRead(uint8_t pin)
{
  if(pin >= 32)
    return Adafruit_seesaw_digitalReadBulkB((1ul << (pin-32))) != 0;
  else
    return Adafruit_seesaw_digitalReadBulk((1ul << pin)) != 0;

}

/**
 *****************************************************************************************
 *  @brief      read the status of multiple pins on port A.
 *
 *  @param      pins a bitmask of the pins to write. On the SAMD09 breakout, this corresponds to the number on the silkscreen.
 *				For example, passing 0b0110 will return the values of pins 2 and 3.
 *
 *  @return     the status of the passed pins. If 0b0110 was passed and pin 2 is high and pin 3 is low, 0b0010 (decimal number 2) will be returned.
 ****************************************************************************************/
uint32_t Adafruit_seesaw_digitalReadBulk(uint32_t pins)
{
  uint8_t buf[4];
  Adafruit_seesaw_read(SEESAW_GPIO_BASE, SEESAW_GPIO_BULK, buf, 4, SEESAW_DEFAULT_DELAY);
  uint32_t ret = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) | (uint32_t)buf[3];
  return ret & pins;
}

/**
 *****************************************************************************************
 *  @brief      read the status of multiple pins on port B.
 *
 *  @param      pins a bitmask of the pins to write.
 *
 *  @return     the status of the passed pins. If 0b0110 was passed and pin 2 is high and pin 3 is low, 0b0010 (decimal number 2) will be returned.
 ****************************************************************************************/
uint32_t Adafruit_seesaw_digitalReadBulkB(uint32_t pins)
{
  uint8_t buf[8];
  Adafruit_seesaw_read(SEESAW_GPIO_BASE, SEESAW_GPIO_BULK, buf, 8, SEESAW_DEFAULT_DELAY);
  uint32_t ret = ((uint32_t)buf[4] << 24) | ((uint32_t)buf[5] << 16) | ((uint32_t)buf[6] << 8) | (uint32_t)buf[7];
  return ret & pins;
}

/**
 *****************************************************************************************
 *  @brief      set the mode of multiple GPIO pins at once.
 *
 *  @param      pins a bitmask of the pins to write. On the SAMD09 breakout, this corresponds to the number on the silkscreen.
 *				For example, passing 0b0110 will set the mode of pins 2 and 3.
 *	@param		mode the mode to set the pins to. One of INPUT, OUTPUT, or INPUT_PULLUP.
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw_pinModeBulk(uint32_t pins, uint8_t mode)
{
  uint8_t cmd[] = { (uint8_t)(pins >> 24) , (uint8_t)(pins >> 16), (uint8_t)(pins >> 8), (uint8_t)pins };
  switch (mode){
    case OUTPUT:
      Adafruit_seesaw_write(SEESAW_GPIO_BASE, SEESAW_GPIO_DIRSET_BULK, cmd, 4);
      break;
    case INPUT:
      Adafruit_seesaw_write(SEESAW_GPIO_BASE, SEESAW_GPIO_DIRCLR_BULK, cmd, 4);
      break;
    case INPUT_PULLUP:
      Adafruit_seesaw_write(SEESAW_GPIO_BASE, SEESAW_GPIO_DIRCLR_BULK, cmd, 4);
      Adafruit_seesaw_write(SEESAW_GPIO_BASE, SEESAW_GPIO_PULLENSET, cmd, 4);
      Adafruit_seesaw_write(SEESAW_GPIO_BASE, SEESAW_GPIO_BULK_SET, cmd, 4);
      break;
    case INPUT_PULLDOWN:
      Adafruit_seesaw_write(SEESAW_GPIO_BASE, SEESAW_GPIO_DIRCLR_BULK, cmd, 4);
      Adafruit_seesaw_write(SEESAW_GPIO_BASE, SEESAW_GPIO_PULLENSET, cmd, 4);
      Adafruit_seesaw_write(SEESAW_GPIO_BASE, SEESAW_GPIO_BULK_CLR, cmd, 4);
      break;
  }

}

/**
 *****************************************************************************************
 *  @brief      set the mode of multiple GPIO pins at once. This supports both ports A and B.
 *
 *  @param      pinsa a bitmask of the pins to write on port A. On the SAMD09 breakout, this corresponds to the number on the silkscreen.
 *				For example, passing 0b0110 will set the mode of pins 2 and 3.
 *  @param      pinsb a bitmask of the pins to write on port B.
 *	@param		mode the mode to set the pins to. One of INPUT, OUTPUT, or INPUT_PULLUP.
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw_pinModeBulkAB(uint32_t pinsa, uint32_t pinsb, uint8_t mode)
{
  uint8_t cmd[] = { (uint8_t)(pinsa >> 24) , (uint8_t)(pinsa >> 16), (uint8_t)(pinsa >> 8), (uint8_t)pinsa,
                    (uint8_t)(pinsb >> 24) , (uint8_t)(pinsb >> 16), (uint8_t)(pinsb >> 8), (uint8_t)pinsb };
  switch (mode){
    case OUTPUT:
      Adafruit_seesaw_write(SEESAW_GPIO_BASE, SEESAW_GPIO_DIRSET_BULK, cmd, 8);
      break;
    case INPUT:
      Adafruit_seesaw_write(SEESAW_GPIO_BASE, SEESAW_GPIO_DIRCLR_BULK, cmd, 8);
      break;
    case INPUT_PULLUP:
      Adafruit_seesaw_write(SEESAW_GPIO_BASE, SEESAW_GPIO_DIRCLR_BULK, cmd, 8);
      Adafruit_seesaw_write(SEESAW_GPIO_BASE, SEESAW_GPIO_PULLENSET, cmd, 8);
      Adafruit_seesaw_write(SEESAW_GPIO_BASE, SEESAW_GPIO_BULK_SET, cmd, 8);
      break;
    case INPUT_PULLDOWN:
      Adafruit_seesaw_write(SEESAW_GPIO_BASE, SEESAW_GPIO_DIRCLR_BULK, cmd, 8);
      Adafruit_seesaw_write(SEESAW_GPIO_BASE, SEESAW_GPIO_PULLENSET, cmd, 8);
      Adafruit_seesaw_write(SEESAW_GPIO_BASE, SEESAW_GPIO_BULK_CLR, cmd, 8);
      break;
  }
}

/**
 *****************************************************************************************
 *  @brief      write a value to multiple GPIO pins at once.
 *
 *  @param      pins a bitmask of the pins to write. On the SAMD09 breakout, this corresponds to the number on the silkscreen.
 *				For example, passing 0b0110 will write the passed value to pins 2 and 3.
 *	@param		value pass HIGH to set the output on the passed pins to HIGH, low to set the output on the passed pins to LOW.
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw_digitalWriteBulk(uint32_t pins, uint8_t value)
{
  uint8_t cmd[] = { (uint8_t)(pins >> 24) , (uint8_t)(pins >> 16), (uint8_t)(pins >> 8), (uint8_t)pins };
  if(value)
    Adafruit_seesaw_write(SEESAW_GPIO_BASE, SEESAW_GPIO_BULK_SET, cmd, 4);
  else
    Adafruit_seesaw_write(SEESAW_GPIO_BASE, SEESAW_GPIO_BULK_CLR, cmd, 4);
}

/**
 *****************************************************************************************
 *  @brief      write a value to multiple GPIO pins at once. This supports both ports A and B
 *
 *  @param      pinsa a bitmask of the pins to write on port A. On the SAMD09 breakout, this corresponds to the number on the silkscreen.
 *				For example, passing 0b0110 will write the passed value to pins 2 and 3.
 *  @param      pinsb a bitmask of the pins to write on port B.
 *	@param		value pass HIGH to set the output on the passed pins to HIGH, low to set the output on the passed pins to LOW.
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw_digitalWriteBulkAB(uint32_t pinsa, uint32_t pinsb, uint8_t value)
{
  uint8_t cmd[] = { (uint8_t)(pinsa >> 24) , (uint8_t)(pinsa >> 16), (uint8_t)(pinsa >> 8), (uint8_t)pinsa,
                    (uint8_t)(pinsb >> 24) , (uint8_t)(pinsb >> 16), (uint8_t)(pinsb >> 8), (uint8_t)pinsb };
  if(value)
    Adafruit_seesaw_write(SEESAW_GPIO_BASE, SEESAW_GPIO_BULK_SET, cmd, 8);
  else
    Adafruit_seesaw_write(SEESAW_GPIO_BASE, SEESAW_GPIO_BULK_CLR, cmd, 8);
}
