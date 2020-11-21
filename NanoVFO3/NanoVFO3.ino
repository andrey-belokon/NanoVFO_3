//
// UR5FFR Si5351 NanoVFO
// v3.0 from 19.11.2020
// Copyright (c) Andrey Belokon, 2017-2020 Odessa
// https://github.com/andrey-belokon/
// http://www.dspview.com
// GNU GPL license
// Special thanks for
// vk3hn CW keyer https://github.com/prt459/vk3hn_CW_keyer/blob/master/Basic_CW_Keyer.ino
//

#include <avr/power.h>
#include <avr/eeprom.h> 

// !!! all user setting defined in config.h, config_hw.h and config_sw.h files !!!
#include "config.h"

#include "pins.h"
#include "utils.h"
#include <i2c.h>
#include "TRX.h"
#include "Encoder.h"

#ifdef VFO_SI5351
  #include <si5351a.h>
#endif
#ifdef VFO_SI570  
  #include <Si570.h>
#endif

#include "disp_OLED128x64.h"

#ifdef RTC_ENABLE
  #include "RTC.h"
#endif

#ifdef VFO_SI5351
  Si5351 vfo5351;
#endif
#ifdef VFO_SI570  
  Si570 vfo570;
#endif

Encoder encoder;
TRX trx;
Display_OLED128x64 disp;

InputPullUpPin inPTT(PIN_IN_PTT);
InputAnalogPin inSMeter(PIN_SMETER);

OutputBinPin outCW(PIN_OUT_CW, !OUT_CW_ACTIVE_LEVEL, OUT_CW_ACTIVE_LEVEL);
OutputBinPin outTX(PIN_OUT_TX, !OUT_TX_ACTIVE_LEVEL, OUT_TX_ACTIVE_LEVEL);
OutputBinPin outKEY(PIN_OUT_KEY, !OUT_KEY_ACTIVE_LEVEL, OUT_KEY_ACTIVE_LEVEL);
OutputBinPin outATT(PIN_OUT_ATT, !OUT_ATT_ACTIVE_LEVEL, OUT_ATT_ACTIVE_LEVEL);
OutputBinPin outPRE(PIN_OUT_PRE, !OUT_PRE_ACTIVE_LEVEL, OUT_PRE_ACTIVE_LEVEL);

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

void setup()
{
  Serial.begin(CAT_BAUND_RATE);
  i2c_init();
  readSettings();
  outCW.setup();
  outTX.setup();
  outKEY.setup();
  outATT.setup();
  outPRE.setup();
  inPTT.setup();
  inSMeter.setup();
  keypad.setup();
  pinMode(PIN_OUT_ATT, OUTPUT);
  pinMode(PIN_OUT_PRE, OUTPUT);
  pinMode(PIN_OUT_TONE, OUTPUT);
  pinMode(PIN_OUT_BAND0, OUTPUT);
  pinMode(PIN_OUT_BAND1, OUTPUT);
  pinMode(PIN_OUT_BAND2, OUTPUT);
  pinMode(PIN_OUT_BAND3, OUTPUT);
  pinMode(PIN_OUT_BAND4, OUTPUT);
  pinMode(PIN_IN_DIT, INPUT_PULLUP);
  pinMode(PIN_IN_DAH, INPUT_PULLUP);
#ifdef VFO_SI5351
  vfo5351.VCOFreq_Max = 800000000; // для использования "кривых" SI-шек с нестабильной генерацией
  // change for required output level
  vfo5351.setup(
    SI5351_CLK0_DRIVE,
    SI5351_CLK1_DRIVE,
    SI5351_CLK2_DRIVE
  );
  vfo5351.set_xtal_freq((SI5351_CALIBRATION/10000)*10000+Settings[ID_SI5351_XTAL]);
#endif  
#ifdef VFO_SI570  
  vfo570.setup(SI570_CALIBRATION);
#endif  
  encoder.Setup();
  trx.StateLoad();
  trx.setCWSpeed(Settings[ID_KEY_SPEED], Settings[ID_KEY_DASH_LEN]);
  disp.setup();
}

#include "freq_calc.h"

void UpdateBandCtrl()
{
#ifdef BAND_ACTIVE_LEVEL_LOW
  if (BAND_COUNT <= 5) {
    digitalWrite(PIN_OUT_BAND0, trx.BandIndex != 0);
    digitalWrite(PIN_OUT_BAND1, trx.BandIndex != 1);
    digitalWrite(PIN_OUT_BAND2, trx.BandIndex != 2);
    digitalWrite(PIN_OUT_BAND3, trx.BandIndex != 3);
    digitalWrite(PIN_OUT_BAND4, trx.BandIndex != 4);
  } else {
    digitalWrite(PIN_OUT_BAND0, !(trx.BandIndex & 0x1));
    digitalWrite(PIN_OUT_BAND1, !(trx.BandIndex & 0x2));
    digitalWrite(PIN_OUT_BAND2, !(trx.BandIndex & 0x4));
    digitalWrite(PIN_OUT_BAND3, !(trx.BandIndex & 0x8));
    digitalWrite(PIN_OUT_BAND4, !(trx.BandIndex & 0x10));
  }
#else
  if (BAND_COUNT <= 5) {
    digitalWrite(PIN_OUT_BAND0, trx.BandIndex == 0);
    digitalWrite(PIN_OUT_BAND1, trx.BandIndex == 1);
    digitalWrite(PIN_OUT_BAND2, trx.BandIndex == 2);
    digitalWrite(PIN_OUT_BAND3, trx.BandIndex == 3);
    digitalWrite(PIN_OUT_BAND4, trx.BandIndex == 4);
  } else {
    digitalWrite(PIN_OUT_BAND0, trx.BandIndex & 0x1);
    digitalWrite(PIN_OUT_BAND1, trx.BandIndex & 0x2);
    digitalWrite(PIN_OUT_BAND2, trx.BandIndex & 0x4);
    digitalWrite(PIN_OUT_BAND3, trx.BandIndex & 0x8);
    digitalWrite(PIN_OUT_BAND4, trx.BandIndex & 0x10);
  }
#endif
  outCW.Write(trx.CW);
  outATT.Write(trx.AttPre == 1 && !trx.TX);
  outPRE.Write(trx.AttPre == 2 && !trx.TX);
}

long last_event = 0;
long last_pool = 0;
uint8_t in_power_save = 0;
uint8_t last_key = 0;
long last_key_press = 0;
long last_cw = 0;

uint8_t cw_bits = 0;
uint8_t cw_bit_idx = 0;

enum {imIDLE,imDIT,imDAH};
uint8_t im_state = imIDLE;

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
      in_power_save = 1;
    }
  } else {
    if (in_power_save) {
      disp.setBright(Settings[ID_DISPLAY_BRIGHT_HIGH]);
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

void recognizeMorse();

void sendDit()
{
  recognizeMorse();
  outKEY.Write(1);
  OutputTone(PIN_OUT_TONE,Settings[ID_KEY_TONE_HZ]);
  delay(trx.dit_time);
  outKEY.Write(0);
  OutputTone(PIN_OUT_TONE,0);
  last_cw = millis();
  delay(trx.dit_time);
  cw_bit_idx++;
}

void sendDah()
{
  recognizeMorse();
  outKEY.Write(1);
  OutputTone(PIN_OUT_TONE,Settings[ID_KEY_TONE_HZ]);
  delay(trx.dah_time);
  outKEY.Write(0);
  OutputTone(PIN_OUT_TONE,0);
  last_cw = millis();
  delay(trx.dit_time);
  cw_bits |= 1 << cw_bit_idx;
  cw_bit_idx++;
}

void cwTXOn()
{
  trx.CWTX = 1;
  if (!trx.TX) {
    trx.TX = 1;
    trx.CWClear();
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
      outKEY.Write(1);
      OutputTone(PIN_OUT_TONE, Settings[ID_KEY_TONE_HZ]);
    } else {
      outKEY.Write(0);
      OutputTone(PIN_OUT_TONE,0);
    }
    last_on = on;
  }
  if (on) last_cw = millis();
}

#include "morse.h"
#include "menu.h"
#include "cat.h"

void recognizeMorse()
{
  if (!Settings[ID_CW_DECODER]) return;
  if (millis()-last_cw >= 2*trx.dit_time) {
    char ch = 0;
    if (cw_bit_idx > 0) {
      ch = decodeMorse(cw_bits,cw_bit_idx);
      if (ch) trx.PutCWChar(ch);
    }
    if (millis()-last_cw > 5*trx.dit_time) {
      ch = ' ';
      trx.PutCWChar(ch);
    }
    if (ch) disp.Draw(trx);
    cw_bits = cw_bit_idx = 0;
  }
}

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

  if (millis()-last_pool > POOL_INTERVAL) {
    trx.TX = trx.CWTX || inPTT.Read();
    outTX.Write(trx.TX);

    PoolKeyboard();
    if (keyb_key != 0) power_save(0);
    switch (keyb_key)
    {
      case 1:
        if (keyb_long) trx.SaveFreqToMemo();
        else trx.SwitchFreqToMemo();
        break;
      case 2:
        if (!trx.TX) {
          if (keyb_long) {
            if (trx.CW) {
              if (trx.sideband == USB) trx.sideband = LSB;
              else trx.CW = 0;
            } else {
              if (trx.sideband == USB) trx.CW = 1;
              else trx.sideband = USB;
            }
          } else {
            trx.SwitchAttPre();
          }
        }
        break;
      case 3:
        if (trx.CW) {
          cwTXOn();
          if (keyb_long) playMessage(PSTR(MEMO2));
          else playMessage(PSTR(MEMO1));
          power_save(0);
        }
        break;
      case 4:
        if (!trx.TX) {
          if (keyb_long) trx.Lock = !trx.Lock;
          else trx.NextBand();
        }
        break;
      case 5:
        if (keyb_long) trx.Freq = (trx.Freq/1000)*1000;
        else {
          show_menu();
          keypad.waitUnpress();
          disp.clear();
          power_save(0);
          trx.setCWSpeed(Settings[ID_KEY_SPEED], Settings[ID_KEY_DASH_LEN]);
        }
        break;
    }
  
    // read and convert smeter
    if (trx.TX) {
      trx.SMeter =  0;
    } else {
      int val = inSMeter.Read();
      bool rev_order = Settings[ID_SMETER+1] > Settings[ID_SMETER+4];
      for (int8_t i=14; i >= 0; i--) {
        int8_t ii = i>>1;
        int treshold;
        if (i&1) treshold = (Settings[ID_SMETER+ii]+Settings[ID_SMETER+ii+1]) >> 1;
        else treshold = Settings[ID_SMETER+ii];
        if ((!rev_order && val >= treshold) || (rev_order && val <= treshold)) {
          trx.SMeter = i+1;
          break;
        }
      }
    }

    UpdateFreq();
    UpdateBandCtrl();
    disp.Draw(trx);
    
    if (Settings[ID_POWER_DOWN_DELAY] > 0 && millis()-last_event > Settings[ID_POWER_DOWN_DELAY]*1000)
      power_save(1);
    last_pool = millis();
  }

  if (trx.TX || Settings[ID_CW_VOX]) {
    if (!Settings[ID_KEY_ENABLE]) {
      sendCW(readDit() || readDah());
    } else if (Settings[ID_KEY_IAMBIC]) {
      if (im_state == imIDLE) {
        if (readDit()) im_state = imDIT;
        else if (readDah()) im_state = imDAH;
      }
      if (im_state == imDIT) {
        cwTXOn();
        sendDit();
        //now, if dah is pressed go there, else check for dit
        if (readDah()) im_state = imDAH;
        else {
          if (readDit()) im_state = imDIT;
          else {
            //delay(trx.dit_time*Settings[ID_KEY_LETTER_SPACE]/10-trx.dit_time);
            im_state = imIDLE;
          }
        }     
      } else if (im_state == imDAH) {
        cwTXOn();
        sendDah();
        //now, if dit is pressed go there, else check for dah
        if (readDit()) im_state = imDIT;
        else {
          if (readDah()) im_state = imDAH;
          else {
            //delay(trx.dit_time*Settings[ID_KEY_LETTER_SPACE]/10-trx.dit_time);
            im_state = imIDLE;
          }
        }
      }
    } else {
      if (readDit()) {
        cwTXOn();
        sendDit();
      } else if (readDah()) {
        cwTXOn();
        sendDah();
      }
    }
  }

  recognizeMorse();

  if (trx.CWTX && millis()-last_cw > Settings[ID_CW_BREAK_IN_DELAY]) {
    trx.CWTX = 0;
    trx.TX = inPTT.Read();
    outTX.Write(trx.TX);
    if (!trx.TX)
      UpdateFreq();
  }
  
  static long state_poll_tm = 0;
  if (millis()-state_poll_tm > 500) {
    static uint16_t state_hash = 0;
    static uint8_t state_changed = false;
    static long state_changed_tm = 0;
    uint16_t new_state_hash = trx.StateHash();
    if (new_state_hash != state_hash) {
      state_hash = new_state_hash;
      state_changed = true;
      state_changed_tm = millis();
    } else if (state_changed && (millis()-state_changed_tm > 5000)) {
      // save state
      trx.StateSave();
      state_changed = false;
    }
    state_poll_tm = millis();
  }
}
