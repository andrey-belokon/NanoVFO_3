#include <Arduino.h>
#include "Encoder.h"
#include "config_hw.h"

/*
  подключение энкодера PIN 2,3
*/

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
