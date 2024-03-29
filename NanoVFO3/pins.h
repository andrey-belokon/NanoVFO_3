#ifndef PINS_H
#define PINS_H

#include <Arduino.h>

#define PIN_NC  0xFF

// pin with bouncing, pull up and active low level
class InputPullUpPin {
  private:
	  uint8_t pin;
	  uint8_t last;
	  long last_tm;
  public:
    InputPullUpPin(uint8_t _pin):pin(_pin),last(false),last_tm(0) {}
    void setup();
    uint8_t Read();
};

class InputAnalogPin {
  private:
	  uint8_t pin;
	  int value,rfac;
  public:
	  InputAnalogPin(uint8_t _pin, int _rfac=0):
	  pin(_pin),value(0),rfac(_rfac) {}
    void setup();
    int Read();
    // return raw ADC data 0..1023
    int ReadRaw();
};

class InputAnalogKeypad {
  private:
    uint8_t pin;
    uint8_t btn_cnt;
    int16_t *levels;
    uint8_t last;
    long last_tm;
  public:
    InputAnalogKeypad(uint8_t _pin, uint8_t _btn_cnt, int16_t *_levels): pin(_pin), btn_cnt(_btn_cnt), levels(_levels), last_tm(-1000) {}
    void setup();
    uint8_t Read();
    void waitUnpress();
};

class OutputBinPin {
  private:
	  uint8_t pin,active_level,def_value,state;
  public:
	  OutputBinPin(uint8_t _pin, uint8_t _def_value, uint8_t _active_level):
	  pin(_pin),active_level(_active_level),def_value(_def_value),state(0xFF) {}
    void setup();
    void Write(uint8_t value);
};

class OutputPCF8574 {
  private:
	  uint8_t i2c_addr,value,old_value;
    void pcf8574_write(uint8_t data);
  public:
	  OutputPCF8574(uint8_t _i2c_addr, uint8_t init_state):i2c_addr(_i2c_addr),value(init_state),old_value(init_state) {}
    void setup();
	  void Set(uint8_t pin, uint8_t state);
	  void Write();
};

void OutputTone(uint8_t pin, int value);

// return VCC in milli-volts (*1000)
int ReadVCC();

#endif
