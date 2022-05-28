#include <i2c.h>
#include "pins.h"

uint8_t InputPullUpPin::Read() {
  if (pin == PIN_NC) return false;
  if (millis()-last_tm < 50) return last;
  uint8_t val = digitalRead(pin) == LOW;
  if (val != last) {
	  last = val;
	  last_tm = millis();
  }
  return val;
}

void InputPullUpPin::setup() {
  if (pin != PIN_NC) 
    pinMode(pin, INPUT_PULLUP); 
}

void InputAnalogKeypad::setup()
{
  if (pin != PIN_NC) 
    pinMode(pin, INPUT); 
}

uint8_t InputAnalogKeypad::Read()
{
  if (pin == PIN_NC) return 0;
  uint16_t aval = 0;
  analogReference(DEFAULT);
  #ifdef __LGT8F__
    ADCSRD = 0x00; // bug in LGT8F analogReference
    //delay(20);
  #endif
  delay(1); analogRead(pin);
  for (byte i=4; i > 0; i--) {
    uint16_t v = analogRead(pin);
    if (v > aval) aval = v;
  }
  #ifdef __LGT8F__
    aval >>= 2; // LGT8F return 12 bit adc result
  #endif
  uint8_t val=0;
  while (val < btn_cnt && aval > levels[val]) val++;
  if (val >= btn_cnt) val = btn_cnt-1;
  if (val != last) {
    last = val;
    last_tm = millis();
  }
  return val;
}

void InputAnalogKeypad::waitUnpress()
{
  while (Read() != 0) delay(1);
  delay(50);
}

void InputAnalogPin::setup() {
  if (pin != PIN_NC) 
    pinMode(pin, INPUT); 
}

// VCC measurement compatible with LGT8F from https://github.com/LaZsolt/Arduino_Vcc

#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  #define ADMUX_VCCWRT1V1 (_BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1))
  #define _IVREF 1100
  #define _ADCMAXRES 1024
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
  #define ADMUX_VCCWRT1V1 (_BV(MUX5) | _BV(MUX0))
  #define _IVREF 1100
  #define _ADCMAXRES 1024
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  #define ADMUX_VCCWRT1V1 (_BV(MUX3) | _BV(MUX2))
  #define _IVREF 1100
  #define _ADCMAXRES 1024
#elif defined(__LGT8FX8P__)
  #define ADMUX_VCCWRT1V1 (_BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX0))
  #define _IVREF 1024
  #define _ADCMAXRES 4096
#elif defined(__LGT8FX8E__)
  #define ADMUX_VCCWRT1V1 (_BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1))
  #define _IVREF 1250
  #define _ADCMAXRES 4096
#else // defined(__AVR_ATmega328P__)
  #define ADMUX_VCCWRT1V1 (_BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1))
  #define _IVREF 1100
  #define _ADCMAXRES 1024
#endif  

uint16_t adcRead_(){
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA, ADSC));
  return ADC;
}

int ReadVCC()
{
  analogReference(DEFAULT);    // Set AD reference to VCC
#if defined(__LGT8FX8P__)
  ADCSRD |= _BV(BGEN);         // IVSEL enable
#endif
  // Read 1.1V/1.024V/1.25V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  if (ADMUX != ADMUX_VCCWRT1V1)
  {
    ADMUX = ADMUX_VCCWRT1V1;
    // Wait for Vref to settle. Bandgap reference start-up time: max 70us
    delayMicroseconds(350); 
  }

  uint16_t pVal;

#if defined(__LGT8FX8P__)
  uint16_t nVal;
  ADCSRC |=  _BV(SPN);
  nVal = adcRead_();
  ADCSRC &= ~_BV(SPN);
#endif
  
  pVal = adcRead_();

#if defined(__LGT8FX8P__)
  pVal = (pVal + nVal) >> 1;
#endif

// Logicgreen gain-error correction
#if defined(__LGT8FX8E__)
  pVal -= (pVal >> 5);
#elif defined(__LGT8FX8P__)
  pVal -= (pVal >> 7);
#endif
  
  // Calculate Vcc (in mV)
  float vcc = (long)_IVREF * _ADCMAXRES / pVal;

  return vcc;
} // end Read_Volts

int InputAnalogPin::Read()
{
  int vcc = ReadVCC(); 
  int new_value = analogRead(pin);
  new_value += analogRead(pin);
  new_value += analogRead(pin);
  new_value = (long)new_value*vcc/_ADCMAXRES/3;
  if (abs(new_value-value) > rfac)
    value=new_value;
  return value;
}

int InputAnalogPin::ReadRaw()
{
  int new_value = 0;
  analogReference(DEFAULT);
  delay(1); analogRead(pin);
  new_value += analogRead(pin);
  new_value += analogRead(pin);
  new_value += analogRead(pin);
  new_value = new_value/3;
  if (abs(new_value-value) > rfac)
    value=new_value;
  return value;
}

void OutputBinPin::setup() 
{
  if (pin != PIN_NC) {
    pinMode(pin, OUTPUT);
    Write(def_value);
  }
}

void OutputBinPin::Write(uint8_t value) 
{
  if (pin != PIN_NC && state != value) {
	  digitalWrite(pin,value?(active_level == HIGH?HIGH:LOW):(active_level == HIGH?LOW:HIGH));
	  state = value;
  }
}

void OutputPCF8574::setup() 
{
  pcf8574_write(value);
}

void OutputPCF8574::Set(uint8_t pin, uint8_t state) 
{
  if (state) value |= (1 << (pin & 0x7));
  else value &= ~(1 << (pin & 0x7));
}

void OutputPCF8574::Write() 
{
  if (value != old_value) {
	  pcf8574_write(value);
	  old_value = value;
  }
}

void OutputPCF8574::pcf8574_write(uint8_t data) 
{
  i2c_begin_write(i2c_addr);
	i2c_write(data);
  i2c_end();
}

//////////////////////////////////////////////////////////////////////////////////

uint8_t OutputTone_state=0;
uint8_t OutputTone_pin=0;
volatile uint8_t OutputTone_toggle=0;

void OutputTone(uint8_t pin, int value) 
{ 
  if (value) {
    if (!OutputTone_state) {
      int prescalers[] = {1, 8, 32, 64, 128, 256, 1024}; // timer2
      uint8_t ipresc=5,mreg;
      for (uint8_t i=0; i <= 6; i++) {
        long t=F_CPU/((long)prescalers[i]*value*2)-1;
        if (t <= 0xFF) {
          ipresc=i;
          mreg=t;
          break;
        }
      }
      if (ipresc > 6) return;
      ipresc++;
      OutputTone_pin = pin;
      // init timer2 2kHz interrup
      cli();
      TCCR2A = 0;// set entire TCCR2A register to 0
      TCCR2B = 0;// same for TCCR2B
      TCNT2  = 0;//initialize counter value to 0
      OCR2A = mreg;
      // turn on CTC mode
      TCCR2A |= (1 << WGM21);
      TCCR2B |= ipresc;
      // enable timer compare interrupt
      TIMSK2 |= (1 << OCIE2A);
      sei();
      OutputTone_state = 1;
    }
  } else {
    if (OutputTone_state) {
      // disable interrup
      cli();
      TIMSK2 = 0;
      sei();
      // set pin to zero
      OutputTone_state = 0;
    }
  }
}

ISR(TIMER2_COMPA_vect)
{
  digitalWrite(OutputTone_pin, OutputTone_toggle++ & 1);
}
