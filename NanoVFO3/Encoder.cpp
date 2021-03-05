#include <Arduino.h>
#include "Encoder.h"
#include "config_hw.h"

/*
  подключение энкодера PIN 2,3
  AS5600 - I2C
*/

#ifdef ENCODER_AS5600
#include <i2c.h>

#define AS5600_ADDR 0x36
#define RAWANGLEAddressMSB 0x0C
#define RAWANGLEAddressLSB 0x0D
#define STATUSAddress 0x0B

int last_angle = 0;
long Encoder_Value = 0;
long enc_last_tm = 0;
long enc_sum = 0;
uint8_t hi_mode = 0;

void Encoder::SetValue(long Value)
{
  Encoder_Value = Value;
}

long Encoder::GetDelta()
{
  int val;

  i2c_begin_write(AS5600_ADDR);
  i2c_write(RAWANGLEAddressMSB);
  i2c_begin_read(AS5600_ADDR);
  val = (int)(i2c_read() & 0xF) << 8; // 12 bits
  i2c_end();

  i2c_begin_write(AS5600_ADDR);
  i2c_write(RAWANGLEAddressLSB);
  i2c_begin_read(AS5600_ADDR);
  val |= (int)i2c_read();
  i2c_end();

  val >>= 4; // 4096 --> 256 "clicks" per turn

  //Serial.println(val); // debug
  int d1, d2;
  d1 = val - last_angle;
  if (val > last_angle) d2 = -(last_angle + 256 - val);
  else d2 = 256 - last_angle + val;
  long delta = (abs(d1) < abs(d2) ? d1 : d2);
  delta = delta * (hi_mode ? ENCODER_FREQ_HI_STEP : ENCODER_FREQ_LO_STEP) / 256;
  Encoder_Value += delta;
  delta = Encoder_Value;
  Encoder_Value = 0;
  last_angle = val;

  enc_sum += delta;
  if (millis()-enc_last_tm > 250) {
    if (enc_sum < 0) enc_sum = -enc_sum;
    hi_mode = enc_sum > ENCODER_FREQ_HI_LO_TRASH/4; // measured in 250msec
    enc_sum = 0;
    enc_last_tm = millis();
  }
  return delta;
}

void Encoder::Setup()
{
}

#else

#ifdef ENCODER_MULT_2
  #define ENC_MUL   2
#else
  #ifdef ENCODER_MULT_4
    #define ENC_MUL   4
  #else
    #define ENC_MUL   1
  #endif
#endif

#define ENC_LO_STEP           (ENCODER_FREQ_LO_STEP/ENCODER_PULSE_PER_TURN/ENC_MUL)
#define ENC_HI_STEP           (ENCODER_FREQ_HI_STEP/ENCODER_PULSE_PER_TURN/ENC_MUL)

long Encoder_Value = 0;
uint8_t enc_last = 0;
long enc_last_tm = 0;
long enc_sum = 0;
uint8_t hi_mode = 0;

void Encoder::SetValue(long Value) 
{
  Encoder_Value = Value;
}

long Encoder::GetDelta() 
{
  // обрабатываем все возможные состояния для увеличения кол-ва импульсов на оборот до 4х
  byte state = (PIND & 0xC) | enc_last;
  enc_last = state >> 2;
  switch(state){
    case 0b0100:
#ifdef ENCODER_MULT_2
    case 0b1011:
#endif
#ifdef ENCODER_MULT_4
    case 0b1101:
    case 0b0010:
#endif
      Encoder_Value -= (hi_mode ? ENC_HI_STEP : ENC_LO_STEP);
      break;
    case 0b1000:
#ifdef ENCODER_MULT_2
    case 0b0111:
#endif
#ifdef ENCODER_MULT_4
    case 0b1110:
    case 0b0001:
#endif
      Encoder_Value += (hi_mode ? ENC_HI_STEP : ENC_LO_STEP);
      break;
  }
  long val;
  val = Encoder_Value;
  Encoder_Value = 0;
  enc_sum += val;
  if (millis()-enc_last_tm > 250) {
    if (enc_sum < 0) enc_sum = -enc_sum;
    hi_mode = enc_sum > ENCODER_FREQ_HI_LO_TRASH/4; // measured in 250msec
    enc_sum = 0;
    enc_last_tm = millis();
  }
  return val;
}

void Encoder::Setup() {
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
}

#endif
