#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <math.h>
#include <unistd.h>

#include "dht11.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_sleep.h"

#define LOW 0
#define HIGH 1

#define MIN_INTERVAL 2000
#define TIMEOUT -1

// Define types of sensors.
#define DHT11 11
#define DHT12 12
#define DHT22 22
#define DHT21 21
#define AM2301 21


static const char *TAG = "DHT11";

//Replica arduino functions:
static unsigned long millis();
void delay(int ms);
void delayMicroseconds(int us);

static float convertCtoF(float c);
static bool dht11_read(bool force);
static uint32_t dht11_expectPulse(bool level);

//Fixme: Currently only handles one sensor, if want to handle multiple put these in a struct and pass pointer to each call
uint32_t _lastreadtime, _max_wait;
int _type;
bool _lastresult;
int _pinnum;
uint8_t _data[5];

void dht11_init(int pin_num) {
  _max_wait = 1000; // 1 millisecond timeout for
                    // reading pulses from DHT sensor.
                    // Note that count is now ignored as the DHT reading algorithm adjusts itself
                    // based on the speed of the processor.
  _type = DHT11;
  _pinnum = pin_num;
  // set up the pins!
  gpio_pad_select_gpio(_pinnum);
  gpio_set_pull_mode(_pinnum, GPIO_PULLUP_ONLY);
  // Using this value makes sure that millis() - lastreadtime will be
  // >= MIN_INTERVAL right away. Note that this assignment wraps around,
  // but so will the subtraction.
  _lastreadtime = millis() - MIN_INTERVAL;
}

//boolean S == Scale.  True == Fahrenheit; False == Celcius
//boolean force: force reading of temperature.
float dht11_readTemperature(bool S, bool force) {
  float f = NAN;

  if (dht11_read(force)) {
    switch (_type) {
      case DHT11:
      case DHT12:
        f = _data[2];
        f += (_data[3] & 0x0f) * 0.1;
        if (_data[2] & 0x80) {
          f *= -1;
        }
        if(S) {
          f = convertCtoF(f);
        }
        break;
      case DHT22:
      case DHT21:
        f = ((uint8_t)(_data[2] & 0x7F)) << 8 | _data[3];
        f *= 0.1;
        if (_data[2] & 0x80) {
          f *= -1;
        }
        if(S) {
          f = convertCtoF(f);
        }
        break;
    }
  }
  return f;
}

//Params: boolean force (force read)
float dht11_readHumidity(bool force) {
  float f = NAN;
  if (dht11_read(force)) {
    switch (_type) {
      case DHT11:
      case DHT12:
        f = _data[0] + _data[1] * 0.1;
        break;
      case DHT22:
      case DHT21:
        f = ((uint8_t)_data[0]) << 8 | _data[1];
        f *= 0.1;
        break;
    }
  }
  return f;
}


static bool dht11_read(bool force) {
  // Check if sensor was read less than two seconds ago and return early
  // to use last reading.
  uint32_t currenttime = millis();
  if (!force && ((currenttime - _lastreadtime) < MIN_INTERVAL)) {
    return _lastresult; // return last correct measurement
  }
  ESP_LOGI(TAG, "Last read time: %dmS current time %dmS", _lastreadtime, currenttime);
  _lastreadtime = currenttime;

  // Reset 40 bits of received data to zero.
  _data[0] = _data[1] = _data[2] = _data[3] = _data[4] = 0;

  // Send start signal.  See DHT datasheet for full signal diagram:
  //   http://www.adafruit.com/datasheets/Digital%20humidity%20and%20temperature%20sensor%20AM2302.pdf

  // Go into high impedence state to let pull-up raise data line level and
  // start the reading process.
  gpio_set_direction(_pinnum, GPIO_MODE_INPUT);
  gpio_set_pull_mode(_pinnum, GPIO_PULLUP_ONLY);
  delay(1);

  // First set data line low for a period according to sensor type
  gpio_set_direction(_pinnum, GPIO_MODE_OUTPUT);
  gpio_set_level(_pinnum, LOW);
  switch(_type) {
    case DHT22:
    case DHT21:
      delayMicroseconds(1100); // data sheet says "at least 1ms"
      break;
    case DHT11:
    default:
      delay(20); //data sheet says at least 18ms, 20ms just to be safe
      break;
  }

  uint32_t cycles[80];
  {
    // End the start signal by setting data line high for 40 microseconds.
    gpio_set_direction(_pinnum, GPIO_MODE_INPUT);
    gpio_set_pull_mode(_pinnum, GPIO_PULLUP_ONLY);

    // Now start reading the data line to get the value from the DHT sensor.
    delayMicroseconds(60);  // Delay a bit to let sensor pull data line low.

    // Turn off interrupts temporarily because the next sections
    // are timing critical and we don't want any interruptions.
    taskDISABLE_INTERRUPTS();

    // First expect a low signal for ~80 microseconds followed by a high signal
    // for ~80 microseconds again.
    if (dht11_expectPulse(LOW) == TIMEOUT) {
      ESP_LOGW(TAG, "DHT timeout waiting for start signal low pulse.");
      _lastresult = false;
      taskENABLE_INTERRUPTS(); // Timing critical code is now complete.
      return _lastresult;
    }
    if (dht11_expectPulse(HIGH) == TIMEOUT) {
      ESP_LOGW(TAG, "DHT timeout waiting for start signal high pulse.");
      _lastresult = false;
      taskENABLE_INTERRUPTS(); // Timing critical code is now complete.
      return _lastresult;
    }

    // Now read the 40 bits sent by the sensor.  Each bit is sent as a 50
    // microsecond low pulse followed by a variable length high pulse.  If the
    // high pulse is ~28 microseconds then it's a 0 and if it's ~70 microseconds
    // then it's a 1.  We measure the cycle count of the initial 50us low pulse
    // and use that to compare to the cycle count of the high pulse to determine
    // if the bit is a 0 (high state cycle count < low state cycle count), or a
    // 1 (high state cycle count > low state cycle count). Note that for speed all
    // the pulses are read into a array and then examined in a later step.
    for (int i=0; i<80; i+=2) {
      cycles[i]   = dht11_expectPulse(LOW);
      cycles[i+1] = dht11_expectPulse(HIGH);
    }
    taskENABLE_INTERRUPTS(); // Timing critical code is now complete.
  }

  // Inspect pulses and determine which ones are 0 (high state cycle count < low
  // state cycle count), or 1 (high state cycle count > low state cycle count).
  for (int i=0; i<40; ++i) {
    uint32_t lowCycles  = cycles[2*i];
    uint32_t highCycles = cycles[2*i+1];
    if ((lowCycles == TIMEOUT) || (highCycles == TIMEOUT)) {
      ESP_LOGW(TAG, "DHT timeout waiting for pulse.");
      _lastresult = false;
      return _lastresult;
    }
    _data[i/8] <<= 1;
    // Now compare the low and high cycle times to see if the bit is a 0 or 1.
    if (highCycles > lowCycles) {
      // High cycles are greater than 50us low cycle count, must be a 1.
      _data[i/8] |= 1;
    }
    // Else high cycles are less than (or equal to, a weird case) the 50us low
    // cycle count so this must be a zero.  Nothing needs to be changed in the
    // stored data.
  }

  ESP_LOGI(TAG, "Received from DHT:");
  ESP_LOGI(TAG, "0x%x, 0x%x, 0x%x, 0x%x, 0x%x", _data[0], _data[1], _data[2], _data[3], _data[4]);
  ESP_LOGI(TAG, "Checksum = 0x%x", (_data[0] + _data[1] + _data[2] + _data[3]) & 0xFF);

  // Check we read 40 bits and that the checksum matches.
  if (_data[4] == ((_data[0] + _data[1] + _data[2] + _data[3]) & 0xFF)) {
    _lastresult = true;
    return _lastresult;
  }
    ESP_LOGE(TAG, "DHT checksum failure!");
    _lastresult = false;
    return _lastresult;

}

// Expect the signal line to be at the specified level for a period of time and
// return a count of loop cycles spent at that level (this cycle count can be
// used to compare the relative time of two pulses).  If more than a millisecond
// ellapses without the level changing then the call fails with a 0 response.
// This is adapted from Arduino's pulseInLong function (which is only available
// in the very latest IDE versions):
//   https://github.com/arduino/Arduino/blob/master/hardware/arduino/avr/cores/arduino/wiring_pulse.c
static uint32_t dht11_expectPulse(bool level) {
  uint32_t count = 0;
  uint64_t before = esp_timer_get_time();

  while (gpio_get_level(_pinnum) == level) {
    count++;
    if (esp_timer_get_time() - before  >= _max_wait) {
      uint64_t after = esp_timer_get_time();
      ESP_LOGI(TAG, "expectPulse timed out after %llduS before: %lld after: %lld", after - before, before, after);
      return TIMEOUT; // Exceeded timeout, fail.
    }
  }
  return count;
}

void delay(int ms) {
  usleep(ms * 1000);
}

void delayMicroseconds(int us) {
  usleep(us);
}

static float convertCtoF(float c) {
  return c * 1.8 + 32;
}


static unsigned long millis() {
  return (xTaskGetTickCount()  * portTICK_RATE_MS);
}
