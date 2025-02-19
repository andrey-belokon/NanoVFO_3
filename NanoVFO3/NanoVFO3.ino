//
// UR5FFR Si5351 NanoVFO
// v3.4 from 12.07.2023
// Copyright (c) Andrey Belokon, 2017-2023 Odessa
// https://github.com/andrey-belokon/
// http://www.ur5ffr.com
// GNU GPL license
// Special thanks for
// vk3hn CW keyer https://github.com/prt459/vk3hn_CW_keyer/blob/master/Basic_CW_Keyer.ino
//

#include <avr/eeprom.h> 

// !!! all user setting defined in config.h, config_hw.h and config_sw.h files !!!
#include "config.h"

#include "pins.h"
#include "utils.h"
#include <i2c.h>
#include "TRX.h"
#include "Encoder.h"
#include "RTC.h"

#if defined(VFO_SI5351) || defined(VFO_SI5351_2)
  #include <si5351a.h>
#endif
#ifdef VFO_SI570  
  #include <Si570.h>
#endif

#include "disp_OLED128x64.h"

#ifdef VFO_SI5351
  Si5351 vfo5351;
#endif
#ifdef VFO_SI5351_2
  Si5351 vfo5351_2;
#endif
#ifdef VFO_SI5351_2
  #define   SELECT_SI5351(x)    digitalWrite(PIN_OUT_SISEL,(x))
#else
  #define   SELECT_SI5351(x)
#endif

#ifdef VFO_SI570  
  Si570 vfo570;
#endif

Encoder encoder;
TRX trx;
Display_OLED128x64 disp;

InputPullUpPin inPTT(PIN_IN_PTT);
InputAnalogPin inSMeter(PIN_SMETER);

#ifdef HARDWARE_3_1
  InputAnalogPin inPower(PIN_POWER);
  InputAnalogPin inSWRF(PIN_SWR_F);
  InputAnalogPin inSWRR(PIN_SWR_R);
  InputAnalogPin inTEMP(PIN_TEMP);
#endif

/*  button hardcode place
    [3]  [1]
                [5]
    [4]  [2]
*/
int16_t KeyLevels[] = {130, 320, 525, 745, 940};
InputAnalogKeypad keypad(PIN_ANALOG_KEYPAD, 6, KeyLevels);

const struct _Settings SettingsDef[SETTINGS_COUNT] PROGMEM = {
  SETTINGS_DATA
};

uint16_t EEMEM eeSettingsVersion = 0;
int EEMEM eeSettings[SETTINGS_COUNT] = {0};  

int Settings[SETTINGS_COUNT] = {0};  

void writeSettings()
{
  eeprom_write_block(Settings, eeSettings, sizeof(Settings));
}

void resetSettings()
{
  for (uint8_t j=0; j < SETTINGS_COUNT; j++)
    Settings[j] = (int)pgm_read_word(&SettingsDef[j].def_value);
}

void readSettings()
{
  uint16_t ver;
  ver = eeprom_read_word(&eeSettingsVersion);
  if (ver == ((SETTINGS_VERSION << 8) | SETTINGS_COUNT))
    eeprom_read_block(Settings, eeSettings, sizeof(Settings));
  else {
    // fill with defaults
    resetSettings();
    writeSettings();
    ver = (SETTINGS_VERSION << 8) | SETTINGS_COUNT;
    eeprom_write_word(&eeSettingsVersion, ver);
  }
}

#ifdef HARDWARE_3_1
  #ifdef HARDWARE_SUPERLED
    void sendBandData(byte data, long freq, byte led_power, byte extdata)
    {
      // map 7 segments to bits (from 0 to 7) * d e c g f b a
      static const byte digimap[10] = {
        B00010000, B10110110, B00101000, B00100100, B10000110, B01000100, B01000000, B00110110, B00000000, B00000100
      };
      digitalWrite(PIN_SR_LATCH, LOW);
      // send data in reverse order
      shiftOut(PIN_SR_DATA, PIN_SR_SHIFT, MSBFIRST, extdata);
      for (byte i=7; i > 0; i--) {
        byte bb;
        freq /= 10;
        if (freq) bb = digimap[freq % 10];
        else bb = B11111110;
        if (i == 7 && led_power == 0) bb |= 1; // off
        if (i == 6 && led_power == 1) bb |= 1; // low power
        shiftOut(PIN_SR_DATA, PIN_SR_SHIFT, MSBFIRST, bb);
      }
      shiftOut(PIN_SR_DATA, PIN_SR_SHIFT, MSBFIRST, data);
      digitalWrite(PIN_SR_LATCH, HIGH);
    }
  #else
    void sendBandData(byte data)
    {
      digitalWrite(PIN_SR_LATCH, LOW);
      shiftOut(PIN_SR_DATA, PIN_SR_SHIFT, MSBFIRST, data); 
      digitalWrite(PIN_SR_LATCH, HIGH);
    }
  #endif
#endif

void setup()
{
  Serial.begin(CAT_BAUND_RATE);
  i2c_init(800000);
  readSettings();
  inPTT.setup();
  inSMeter.setup();
  keypad.setup();
  pinMode(PIN_OUT_TONE, OUTPUT);
#ifdef PIN_OUT_TX
  pinMode(PIN_OUT_TX, OUTPUT);
  digitalWrite(PIN_OUT_TX, !OUT_TX_ACTIVE_LEVEL);
#endif
  pinMode(PIN_OUT_KEY, OUTPUT); 
  digitalWrite(PIN_OUT_KEY, !OUT_KEY_ACTIVE_LEVEL);
  pinMode(PIN_IN_DIT, INPUT_PULLUP);
  pinMode(PIN_IN_DAH, INPUT_PULLUP);
#ifdef HARDWARE_3_1
  #ifdef HARDWARE_SUPERLED
    pinMode(PIN_OUT_SISEL, OUTPUT);
  #else
    pinMode(PIN_OUT_USR, OUTPUT);
  #endif
  pinMode(PIN_SR_DATA, OUTPUT);
  pinMode(PIN_SR_SHIFT, OUTPUT);
  pinMode(PIN_SR_LATCH, OUTPUT);
  #ifdef HARDWARE_SUPERLED
    sendBandData(
      #ifdef BPF_ACTIVE_LEVEL_LOW
        B1111+
      #endif
      (!OUT_ATT_ACTIVE_LEVEL << 4) +
      (!OUT_CW_ACTIVE_LEVEL << 5) +
      (!OUT_PRE_ACTIVE_LEVEL << 6) +
      (!OUT_TX_ACTIVE_LEVEL << 7),
      0,0,
      #ifdef LPF_ACTIVE_LEVEL_LOW
        B11111+
      #endif
      (!OUT_TUNE_ACTIVE_LEVEL << 5)+
      (!OUT_QRP_ACTIVE_LEVEL << 6)
    );
  #else
    sendBandData(
      !OUT_CW_ACTIVE_LEVEL +
      #ifdef BPF_ACTIVE_LEVEL_LOW
        B111110 +
      #endif
      (!OUT_ATT_ACTIVE_LEVEL << 6) +
      (!OUT_PRE_ACTIVE_LEVEL << 7)
    );
  #endif
  inPower.setup();
  inSWRF.setup();
  inSWRR.setup();
  inTEMP.setup();
#else
  pinMode(PIN_OUT_CW, OUTPUT);
  digitalWrite(PIN_OUT_CW, !OUT_CW_ACTIVE_LEVEL);
  pinMode(PIN_OUT_ATT, OUTPUT);
  digitalWrite(PIN_OUT_ATT, !OUT_ATT_ACTIVE_LEVEL);
  pinMode(PIN_OUT_PRE, OUTPUT);
  digitalWrite(PIN_OUT_PRE, !OUT_PRE_ACTIVE_LEVEL);
  pinMode(PIN_OUT_BAND0, OUTPUT);
  pinMode(PIN_OUT_BAND1, OUTPUT);
  pinMode(PIN_OUT_BAND2, OUTPUT);
  pinMode(PIN_OUT_BAND3, OUTPUT);
  pinMode(PIN_OUT_BAND4, OUTPUT);
  #ifdef BPF_ACTIVE_LEVEL_LOW 
    digitalWrite(PIN_OUT_BAND0, HIGH);
    digitalWrite(PIN_OUT_BAND1, HIGH);
    digitalWrite(PIN_OUT_BAND2, HIGH);
    digitalWrite(PIN_OUT_BAND3, HIGH);
    digitalWrite(PIN_OUT_BAND4, HIGH);
  #else
    digitalWrite(PIN_OUT_BAND0, LOW);
    digitalWrite(PIN_OUT_BAND1, LOW);
    digitalWrite(PIN_OUT_BAND2, LOW);
    digitalWrite(PIN_OUT_BAND3, LOW);
    digitalWrite(PIN_OUT_BAND4, LOW);
  #endif
#endif
#if defined(VFO_SI5351) || defined(VFO_SI5351_2)
  // static field, assign once
  vfo5351.VCOFreq_Max = SI5351_VCOMAXFREQ; // to use defective SI5351 with unstable generation
#endif
#ifdef VFO_SI5351
  SELECT_SI5351(0);
  vfo5351.setup(
    SI5351_CLK0_DRIVE,
    SI5351_CLK1_DRIVE,
    SI5351_CLK2_DRIVE
  );
  vfo5351.set_xtal_freq((SI5351_CALIBRATION/10000)*10000+Settings[ID_SI5351_XTAL]);
#endif
#ifdef VFO_SI5351_2
  SELECT_SI5351(1);
  vfo5351_2.setup(
    SI5351_2_CLK0_DRIVE,
    SI5351_2_CLK1_DRIVE,
    SI5351_2_CLK2_DRIVE
  );
  vfo5351_2.set_xtal_freq((SI5351_CALIBRATION/10000)*10000+Settings[ID_SI5351_XTAL]);
#endif
#ifdef VFO_SI570  
  vfo570.setup(SI570_CALIBRATION);
#endif  
  encoder.Setup();
  trx.StateLoad();
  #ifdef ENABLE_INTERNAL_CWKEY
    trx.setCWSpeed(Settings[ID_KEY_SPEED], Settings[ID_KEY_DASH_LEN]);
  #endif
  disp.setup();
  #ifdef HARDWARE_SUPERLED
    trx.led_power = Settings[ID_DISPLAY_LED_HIGH];
  #endif
}

#include "freq_calc.h"

void UpdateBandCtrl()
{
#ifdef HARDWARE_3_1
  
  byte data;

  #ifdef HARDWARE_SUPERLED
    static byte last_data = 
      #ifdef BPF_ACTIVE_LEVEL_LOW
        B1111+
      #endif
      (!OUT_ATT_ACTIVE_LEVEL << 4) +
      (!OUT_CW_ACTIVE_LEVEL << 5) +
      (!OUT_PRE_ACTIVE_LEVEL << 6) +
      (!OUT_TX_ACTIVE_LEVEL << 7);

    static long last_freq = 0;
    static byte last_led_pwr = -1;
    static byte last_ext_data = 0;
    byte ext_data;

    data = 
      ((trx.AttPre == 1 && !trx.TX ? OUT_ATT_ACTIVE_LEVEL : !OUT_ATT_ACTIVE_LEVEL) << 4) +
      #ifdef DISABLE_CW_ON_CWTX
        ((trx.CW && !trx.TX ? OUT_CW_ACTIVE_LEVEL : !OUT_CW_ACTIVE_LEVEL) << 5) +
      #else
        ((trx.CW ? OUT_CW_ACTIVE_LEVEL : !OUT_CW_ACTIVE_LEVEL) << 5) +
      #endif      
      ((trx.AttPre == 2 && !trx.TX ? OUT_PRE_ACTIVE_LEVEL : !OUT_PRE_ACTIVE_LEVEL) << 6) +
      ((trx.TX ? OUT_TX_ACTIVE_LEVEL : !OUT_TX_ACTIVE_LEVEL) << 7);
  
    #if 1
      // binary code for band number
      #ifdef BPF_ACTIVE_LEVEL_LOW
        data = data + (!trx.BandIndex & 0xF);
      #else
        data = data + trx.BandIndex;
      #endif
    #else
      // BPF code mapping for NoTune BPF http://www.ur5ffr.com/viewtopic.php?t=363
      if (trx.CurrentFreq < 4600000) data = data + 0;
      else if (trx.CurrentFreq < 7500000) data = data + B100;
      else if (trx.CurrentFreq < 11800000) data = data + B010;
      else if (trx.CurrentFreq < 16000000) data = data + B001;
      else if (trx.CurrentFreq < 23000000) data = data + B101;
      else data = data + B011;
    #endif

    #ifdef LPF_ACTIVE_LEVEL_LOW
      if (trx.CurrentFreq <= 2500000)       ext_data = B11110;  // 160
      else if (trx.CurrentFreq <= 4000000)  ext_data = B11101;  // 80
      else if (trx.CurrentFreq <= 8000000)  ext_data = B11011;  // 40
      else if (trx.CurrentFreq <= 16000000) ext_data = B10111;  // 20
      else                                  ext_data = B01111;  // 10
    #else
      if (trx.CurrentFreq <= 2500000)       ext_data = B00001;  // 160
      else if (trx.CurrentFreq <= 4000000)  ext_data = B00010;  // 80
      else if (trx.CurrentFreq <= 8000000)  ext_data = B00100;  // 40
      else if (trx.CurrentFreq <= 16000000) ext_data = B01000;  // 20
      else                                  ext_data = B10000;  // 10
    #endif

    ext_data |= (trx.tune ? OUT_TUNE_ACTIVE_LEVEL : !OUT_TUNE_ACTIVE_LEVEL) << 5;
    ext_data |= (trx.qrp ? OUT_QRP_ACTIVE_LEVEL : !OUT_QRP_ACTIVE_LEVEL) << 6;

    if (data != last_data || trx.CurrentFreq != last_freq || trx.led_power != last_led_pwr || ext_data != last_ext_data) {
      sendBandData(data, trx.CurrentFreq, trx.led_power, ext_data);
      last_data = data;
      last_freq = trx.CurrentFreq;
      last_led_pwr = trx.led_power;
      last_ext_data = ext_data;
    }

  #else 

    static byte last_data = 
      !OUT_CW_ACTIVE_LEVEL +
      #ifdef BPF_ACTIVE_LEVEL_LOW
        B111110 +
      #endif
      (!OUT_ATT_ACTIVE_LEVEL << 6) +
      (!OUT_PRE_ACTIVE_LEVEL << 7);
  
    data = 
      #ifdef DISABLE_CW_ON_CWTX
        (trx.CW && !trx.TX ? OUT_CW_ACTIVE_LEVEL : !OUT_CW_ACTIVE_LEVEL) +
      #else
        (trx.CW ? OUT_CW_ACTIVE_LEVEL : !OUT_CW_ACTIVE_LEVEL) +
      #endif      
      ((trx.AttPre == 1 && !trx.TX ? OUT_ATT_ACTIVE_LEVEL : !OUT_ATT_ACTIVE_LEVEL) << 6) +
      ((trx.AttPre == 2 && !trx.TX ? OUT_PRE_ACTIVE_LEVEL : !OUT_PRE_ACTIVE_LEVEL) << 7);
  
    #ifdef BPF_ACTIVE_LEVEL_LOW
      #if BAND_COUNT <= 5
        data = data +
          (trx.BandIndex == 0 ? 0 : B000010) +
          (trx.BandIndex == 1 ? 0 : B000100) +
          (trx.BandIndex == 2 ? 0 : B001000) +
          (trx.BandIndex == 3 ? 0 : B010000) +
          (trx.BandIndex == 4 ? 0 : B100000);
      #else
        data = data +
          (trx.BandIndex & B00001 ? 0 : B000010) +
          (trx.BandIndex & B00010 ? 0 : B000100) +
          (trx.BandIndex & B00100 ? 0 : B001000) +
          (trx.BandIndex & B01000 ? 0 : B010000) +
          (trx.BandIndex & B10000 ? 0 : B100000);
      #endif
    #else
      #if BAND_COUNT <= 5
        data = data +
          (trx.BandIndex == 0 ? B000010 : 0) +
          (trx.BandIndex == 1 ? B000100 : 0) +
          (trx.BandIndex == 2 ? B001000 : 0) +
          (trx.BandIndex == 3 ? B010000 : 0) +
          (trx.BandIndex == 4 ? B100000 : 0);
      #else
        data = data +
          (trx.BandIndex & B00001 ? B000010 : 0) +
          (trx.BandIndex & B00010 ? B000100 : 0) +
          (trx.BandIndex & B00100 ? B001000 : 0) +
          (trx.BandIndex & B01000 ? B010000 : 0) +
          (trx.BandIndex & B10000 ? B100000 : 0);
      #endif
    #endif

    if (data != last_data) {
      sendBandData(data);
      last_data = data;
    }

    #ifdef PIN_OUT_TX
      digitalWrite(PIN_OUT_TX, (trx.TX ? OUT_TX_ACTIVE_LEVEL : !OUT_TX_ACTIVE_LEVEL));
    #endif

  #endif

#else

  #ifdef BPF_ACTIVE_LEVEL_LOW
    #if BAND_COUNT <= 5
      digitalWrite(PIN_OUT_BAND0, trx.BandIndex != 0);
      digitalWrite(PIN_OUT_BAND1, trx.BandIndex != 1);
      digitalWrite(PIN_OUT_BAND2, trx.BandIndex != 2);
      digitalWrite(PIN_OUT_BAND3, trx.BandIndex != 3);
      digitalWrite(PIN_OUT_BAND4, trx.BandIndex != 4);
    #else
      digitalWrite(PIN_OUT_BAND0, !(trx.BandIndex & 0x1));
      digitalWrite(PIN_OUT_BAND1, !(trx.BandIndex & 0x2));
      digitalWrite(PIN_OUT_BAND2, !(trx.BandIndex & 0x4));
      digitalWrite(PIN_OUT_BAND3, !(trx.BandIndex & 0x8));
      digitalWrite(PIN_OUT_BAND4, !(trx.BandIndex & 0x10));
    #endif
  #else
    #if BAND_COUNT <= 5
      digitalWrite(PIN_OUT_BAND0, trx.BandIndex == 0);
      digitalWrite(PIN_OUT_BAND1, trx.BandIndex == 1);
      digitalWrite(PIN_OUT_BAND2, trx.BandIndex == 2);
      digitalWrite(PIN_OUT_BAND3, trx.BandIndex == 3);
      digitalWrite(PIN_OUT_BAND4, trx.BandIndex == 4);
    #else
      digitalWrite(PIN_OUT_BAND0, trx.BandIndex & 0x1);
      digitalWrite(PIN_OUT_BAND1, trx.BandIndex & 0x2);
      digitalWrite(PIN_OUT_BAND2, trx.BandIndex & 0x4);
      digitalWrite(PIN_OUT_BAND3, trx.BandIndex & 0x8);
      digitalWrite(PIN_OUT_BAND4, trx.BandIndex & 0x10);
    #endif
  #endif
  
#ifdef DISABLE_CW_ON_CWTX
  digitalWrite(PIN_OUT_CW, (trx.CW && !trx.TX ? OUT_CW_ACTIVE_LEVEL : !OUT_CW_ACTIVE_LEVEL));
#else
  digitalWrite(PIN_OUT_CW, (trx.CW ? OUT_CW_ACTIVE_LEVEL : !OUT_CW_ACTIVE_LEVEL));
#endif
  digitalWrite(PIN_OUT_ATT, (trx.AttPre == 1 && !trx.TX ? OUT_ATT_ACTIVE_LEVEL : !OUT_ATT_ACTIVE_LEVEL));
  digitalWrite(PIN_OUT_PRE, (trx.AttPre == 2 && !trx.TX ? OUT_PRE_ACTIVE_LEVEL : !OUT_PRE_ACTIVE_LEVEL));
  #ifdef PIN_OUT_TX
    digitalWrite(PIN_OUT_TX, (trx.TX ? OUT_TX_ACTIVE_LEVEL : !OUT_TX_ACTIVE_LEVEL));
  #endif

#endif  
}

long last_event = 0;
long last_pool = 0;
uint8_t in_power_save = 0;
uint8_t last_key = 0;
long last_key_press = 0;
long last_cw = 0;

uint8_t keyb_key = 0;
uint8_t keyb_long = 0;

// fill keyb_key & keyb_long
void PoolKeyboard()
{
  uint8_t key = keypad.Read();
  keyb_key = 0;
  if (keyb_long && key) return; // wait unpress
  keyb_long = 0;
  if (!last_key && key) {
    // KEY DOWN
    last_key = key;
    last_key_press = millis();
  } else if (last_key && key) {
    // KEY 
    if (millis()-last_key_press > LONG_PRESS_DELAY) {
      keyb_key = last_key;
      keyb_long = 1;
      last_key = 0;
      last_key_press = 0;
    }
  } else if (last_key && !key) {
    // KEY UP
    keyb_key = last_key;
    keyb_long = 0;
    last_key = 0;
    last_key_press = 0;
  }
}

void power_save(uint8_t enable)
{
  if (enable) {
    if (!in_power_save) {
      disp.setBright(Settings[ID_DISPLAY_BRIGHT_LOW]);
      #ifdef HARDWARE_SUPERLED
        trx.led_power = Settings[ID_DISPLAY_LED_LOW];
      #endif
      in_power_save = 1;
    }
  } else {
    if (in_power_save) {
      disp.setBright(Settings[ID_DISPLAY_BRIGHT_HIGH]);
      #ifdef HARDWARE_SUPERLED
        trx.led_power = Settings[ID_DISPLAY_LED_HIGH];
      #endif
      in_power_save = 0;
    }
    last_event = millis();
  }
}

uint8_t readDit()
{
  if(digitalRead(PIN_IN_DIT)) return 0;
  else return 1;
}

uint8_t readDah()
{
  if(digitalRead(PIN_IN_DAH)) return 0;
  else return 1;
}

void cwTXOn()
{
  trx.CWTX = 1;
  if (!trx.TX) {
    trx.SetTX(true);
    #ifdef ENABLE_INTERNAL_CWKEY
      trx.CWClear();
    #endif
    UpdateBandCtrl();
    UpdateFreq();
    disp.Draw(trx);
  }
}

void sendCW(byte on)
{
  static byte last_on = 0;
  if (on != last_on) {
    if (on) {
      cwTXOn();
      digitalWrite(PIN_OUT_KEY, OUT_KEY_ACTIVE_LEVEL);
      OutputTone(PIN_OUT_TONE, Settings[ID_KEY_TONE_HZ]);
    } else {
      digitalWrite(PIN_OUT_KEY, !OUT_KEY_ACTIVE_LEVEL);
      OutputTone(PIN_OUT_TONE,0);
    }
    last_on = on;
  }
  if (on) last_cw = millis();
}

#include "cwkey.h"
#include "menu.h"
#include "cat.h"

void loop()
{
  if (!trx.TX && !trx.Lock) {
    long delta = encoder.GetDelta();
    if (delta) {
      trx.ChangeFreq(delta);
      power_save(0);
    }
  }

  if (Serial.available() > 0) 
    ExecCAT();

  if (trx.TX || millis()-last_pool > POOL_INTERVAL) {
    static byte last_ptt = 0;
    byte ptt = inPTT.Read();
    if (ptt != last_ptt) {
      trx.CATTX = 0;
      last_ptt = ptt;
    }
    uint8_t new_tx = trx.CWTX || trx.CATTX || ptt;
    if (Settings[ID_TEMP_ENABLED] && (int16_t(trx.temp)/10 > Settings[ID_TEMP_STOP])) {
      new_tx = 0;
    }
    trx.SetTX(new_tx);

    UpdateBandCtrl();

    if (!trx.TX) {
      PoolKeyboard();
      if (keyb_key != 0) power_save(0);
      switch (keyb_key)
      {
        case 1:
          // right-up
          if (keyb_long) trx.SaveFreqToMemo();
          else trx.SwitchFreqToMemo();
          break;
        case 2:
          // right-down
          if (keyb_long) trx.NextMode();
          else trx.SwitchAttPre();
          break;
        case 3:
          // left-up
#ifdef ENABLE_INTERNAL_CWKEY
          if (keyb_long) {
            show_menu();
            keypad.waitUnpress();
            disp.clear();
            power_save(0);
            trx.setCWSpeed(Settings[ID_KEY_SPEED], Settings[ID_KEY_DASH_LEN]);
          } else {
            const uint8_t idxmap[] = {0xFF, 1, 3, 0, 2, 0xFF};
            disp.DrawText(true, PSTR("CW MEMO"),3);
            while (true) {
              PoolKeyboard();
              uint8_t idx = idxmap[keyb_key];
              uint8_t len = 0;
              if (idx <= 3) {
                if (idx <= 2 && eeprom_read_word(cw_bank_full+idx) == BANK_IS_FULL && (len=eeprom_read_byte(cw_bank_len+idx)) > 0) {
                  cwTXOn();
                  disp.clear();
                  playMessage(idx,len);
                  power_save(0);
                  trx.CWTX = 0;
                  trx.TX = inPTT.Read();
                  if (!trx.TX) {
                    UpdateBandCtrl();
                    UpdateFreq();
                  }
                }
                break;
              }
            }
            keypad.waitUnpress();
            PoolKeyboard();
            disp.clear();
            power_save(0);
          }
#else
          if (keyb_long) {
            trx.Freq = ((trx.Freq+250)/500)*500;
            trx.changed++;
          } else {
            show_menu();
            keypad.waitUnpress();
            disp.clear();
            power_save(0);
          }
#endif
          break;
        case 4:
          // left-down
          if (keyb_long) trx.Lock = !trx.Lock;
          else {
            if (BAND_COUNT > 2) {
              select_band();
              keypad.waitUnpress();
              disp.clear();
              power_save(0);
            } else if (BAND_COUNT == 2) trx.NextBand();
          }
          break;
#ifndef ENCODER_AS5600
        case 5:
          // encoder btn
          trx.Freq = ((trx.Freq+250)/500)*500;
          trx.changed++;
          break;
#endif
      }
    }

    // read and convert swr/power/smeter
    static float last_sm = 0;
#ifdef ENABLE_SWR_SENSOR
    static float last_pwrlevel = 0;
    static float last_swr = 0;
    if (trx.TX) {
      uint16_t fval = inSWRF.Read();
      uint16_t rval = inSWRR.Read();
      int ipwr = trx.CalculatePower(fval);
      float pwr = ipwr*0.1/Settings[ID_TX_MAX_POWER];
      trx.pwr = 0;
      trx.swr = 0;
      if (ipwr >= 10) {
        // >= 1wt
        float new_swr = trx.CalculateSWR(fval,rval);
        if (new_swr > last_swr) last_swr = new_swr;
        else last_swr = 0.95*last_swr+0.05*new_swr;
        trx.swr = last_swr;
      }
      if (Settings[ID_TX_MAX_POWER] > 0) {
        if (pwr >= last_pwrlevel) trx.pwr = pwr;
        else trx.pwr = 0.995*last_pwrlevel + 0.005*pwr;
        if (trx.pwr > 1) trx.pwr = 1.0;
        last_pwrlevel = trx.pwr;
      }
      last_sm = 0;
    } else
#endif
    {
      int val = inSMeter.Read();
      if (val > last_sm) last_sm = val;
      else last_sm = 0.9*last_sm+0.1*val;
      val = last_sm;
      bool rev_order = Settings[ID_SMETER+1] > Settings[ID_SMETER+4];
      for (int8_t i=14; i >= 0; i--) {
        int8_t ii = i>>1;
        int treshold;
        if (i&1) treshold = (Settings[ID_SMETER+ii]+Settings[ID_SMETER+ii+1]) >> 1;
        else treshold = Settings[ID_SMETER+ii];
        if ((!rev_order && val >= treshold) || (rev_order && val <= treshold)) {
          trx.SMeter = i+1;
          #ifdef HARDWARE_SUPERLED
            trx.SMeterFrac = 0;
            if (i < 14) {
              val -= treshold;
              if (i&1) treshold = Settings[ID_SMETER+ii+1]-treshold;
              else treshold = ((Settings[ID_SMETER+ii]+Settings[ID_SMETER+ii+1]) >> 1) - treshold;
              trx.SMeterFrac = abs(int(long(val)*100 / treshold));
            }
          #endif
          break;
        }
      }
#ifdef ENABLE_SWR_SENSOR
      last_pwrlevel = last_swr = 0;
#endif
    }

    #ifdef HARDWARE_3_1
      if (Settings[ID_VCC] > 0 && Settings[ID_VCC_VAL] > 0) {
        float new_val = 0.1*Settings[ID_VCC]*inPower.Read()/Settings[ID_VCC_VAL];
        if (trx.VCC < 1) trx.VCC = new_val;
        else trx.VCC = trx.VCC*0.99+new_val*0.01;
      }
      if (Settings[ID_TEMP_ENABLED]) {
        if (trx.temp < 1) trx.temp = inTEMP.Read();
        else trx.temp = inTEMP.Read()*0.05 + trx.temp*0.95;
      }
    #endif

    UpdateFreq();
    UpdateBandCtrl();
    disp.Draw(trx);
    
    if (Settings[ID_POWER_DOWN_DELAY] > 0 && millis()-last_event > (long)1000*Settings[ID_POWER_DOWN_DELAY])
      power_save(1);
    last_pool = millis();
  }

#ifdef ENABLE_INTERNAL_CWKEY
  if (!Settings[ID_KEY_ENABLE]) {
    sendCW(readDit() || readDah());
  } else {
    if (recognizeMorse('*')) disp.Draw(trx);
    ReadCWKey(true);
  }
#else
  sendCW(readDit() || readDah());
#endif

#ifdef ENABLE_INTERNAL_CWKEY
  if (trx.CWTX && millis()-last_cw > Settings[ID_CW_BREAK_IN_DELAY]+trx.dit_time*Settings[ID_KEY_WORD_SPACE]/10) {
#else
  if (trx.CWTX && millis()-last_cw > Settings[ID_CW_BREAK_IN_DELAY]) {
#endif
    trx.CWTX = 0;
    trx.SetTX(inPTT.Read());
    UpdateBandCtrl();
    if (!trx.TX) UpdateFreq();
  }

  static long state_poll_tm = 0;
  if (millis()-state_poll_tm > 500) {
    static uint8_t last_changed = 0;
    static uint8_t state_changed = false;
    static long state_changed_tm = 0;
    if (trx.changed) {
      if (trx.changed != last_changed) {
        last_changed = trx.changed;
        state_changed_tm = millis();
        state_changed = true;
      } else if (state_changed && millis()-state_changed_tm > 5000) {
        // save state
        trx.StateSave();
        state_changed = false;
      }
    }
    state_poll_tm = millis();
  }
}
