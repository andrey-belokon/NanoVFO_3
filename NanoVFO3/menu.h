#define MENU_DEBOUNCE   250
#define MENU_ENCDELTA   5

byte dec2bcd(byte val)
{
  return( (val/10*16) + (val%10));
}

byte bcd2dec(byte val)
{
  return( (val/16*10) + (val%16));
}

byte edit_clockitem(const char *title, byte val, byte maxval)
{
  disp.DrawItemEdit(title,val);
  keypad.waitUnpress();
  while (1) {
    PoolKeyboard();
    switch (keyb_key) {
      case 3:
      case 5: 
        // save value
        return val;
        break;
      case 1:
      case 2:
      case 4:
        // exit
        return 0xFF;
    }
    long d=encoder.GetDelta();
    if (d <= -MENU_ENCDELTA && val > 0) {
      val--;
      disp.DrawItemValue(val);
    } else if (d >= MENU_ENCDELTA && val < maxval) {
      val++;
      disp.DrawItemValue(val);
    }
  }
}

void show_clockmenu()
{
  PGM_P buf[4];
  uint8_t sel=0;
  RTCData dt;
  for (byte i=0; i < 4; i++) buf[i] = NULL;
  buf[0] = PSTR("HOUR");
  buf[1] = PSTR("MINUTES");
  disp.DrawItems(buf,sel);
  keypad.waitUnpress();
  while (1) {
    PoolKeyboard();
    switch (keyb_key) {
      case 3:
      case 5:
        if (sel == 0) {
          // hour
          RTC_Read(&dt);
          byte val = edit_clockitem(buf[0],bcd2dec(dt.hour),23);
          if (val != 0xFF) {
            dt.hour = dec2bcd(val);
            RTC_Write(&dt);
          }
        } else {
          // min
          RTC_Read(&dt);
          byte val = edit_clockitem(buf[1],bcd2dec(dt.min),59);
          if (val != 0xFF) {
            dt.min = dec2bcd(val);
            RTC_Write(&dt);
          }
        }
        disp.DrawItems(buf,sel);
        keypad.waitUnpress();
        break;
      case 1:
      case 2:
      case 4:
        // exit
        return;
    }
    long d=encoder.GetDelta();
    if ((d >> 1) != 0) {
      sel ^= 1;
      disp.DrawSelected(sel);
      delay(MENU_DEBOUNCE);
      encoder.GetDelta();
    }
  }
}

void edit_smeteritem(byte idx)
{
  int val = inSMeter.Read();
  long tm = millis();
  disp.DrawItemEdit(SettingsDef[idx].title,val);
  keypad.waitUnpress();
  while (1) {
    PoolKeyboard();
    switch (keyb_key) {
      case 3:
      case 5:
        // save value
        Settings[idx] = val;
        writeSettings();
        return;
      case 1:
      case 2:
      case 4:
        // exit
        return;
    }
    if (millis()-tm > 300) {
      val = inSMeter.Read();
      disp.DrawItemValue(val);
      tm = millis();
    }
  }
}

void show_smetermenu(byte idx, byte len)
{
  PGM_P buf[4];
  int vals[4];
  uint8_t mi=idx,sel=0;
  for (byte i=0; i < 4; i++) buf[i] = NULL;
  for (byte i=0; i < 4 && mi+i < idx+len; i++) {
    buf[i] = SettingsDef[mi+i].title;
    vals[i] = Settings[mi+i];
  }
  disp.DrawSMeterItems(buf,vals,sel);
  keypad.waitUnpress();
  while (1) {
    PoolKeyboard();
    switch (keyb_key) {
      case 3:
      case 5:
        //edit_item(mi+sel);
        edit_smeteritem(mi+sel);
        for (byte i=0; i < 4 && mi+i < idx+len; i++) {
          buf[i] = SettingsDef[mi+i].title;
          vals[i] = Settings[mi+i];
        }
        disp.DrawSMeterItems(buf,vals,sel);
        keypad.waitUnpress();
        break;
      case 1:
      case 2:
      case 4:
        // exit
        return;
    }
    long d=encoder.GetDelta();
    uint8_t dmi = 0;
    uint8_t deb = 0;
    if (d <= -MENU_ENCDELTA && mi+sel > idx) {
      if (sel > 0) {
        sel--;
        disp.DrawSelected(sel);
      } else {
        dmi = -1;
      }
      deb = 1;
    } else if (d >= MENU_ENCDELTA && mi+sel < idx+len-1) {
      if (sel < 3) {
        sel++;
        disp.DrawSelected(sel);
      } else {
        dmi = 1;
      }
      deb = 1;
    }
    if (dmi) {
      mi += dmi;
      for (byte i=0; i < 4 && mi+i < idx+len; i++) {
        buf[i] = SettingsDef[mi+i].title;
        vals[i] = Settings[mi+i];
      }
      disp.DrawSMeterItems(buf,vals,sel);
    }
    if (deb) {
      delay(MENU_DEBOUNCE);
      encoder.GetDelta();
    }

  }
}

#ifdef HARDWARE_3_1

void show_vcc_setup()
{
  keypad.waitUnpress();
  disp.clear();
  int rawdata = inPower.Read();
  int vcc = Settings[ID_VCC];
  if (vcc <= 50 || vcc > 300) vcc = 120;
  disp.DrawVCC(rawdata,vcc);
  long tm = millis();
  while (1) {
    PoolKeyboard();
    switch (keyb_key) {
      case 3:
      case 5:
        // save value
        Settings[ID_VCC] = vcc;
        Settings[ID_VCC_VAL] = rawdata;
        writeSettings();
      case 1:
      case 2:
      case 4:
        return;
    }
    long d=encoder.GetDelta();
    if (d <= -MENU_ENCDELTA) {
      vcc--;
      if (vcc < 50) vcc = 50;
      disp.DrawVCC(rawdata,vcc);
    } else if (d >= MENU_ENCDELTA) {
      vcc++;
      disp.DrawVCC(rawdata,vcc);
    }
    if (millis()-tm > 50) {
      rawdata = inPower.Read();
      disp.DrawVCC(rawdata,vcc);
      tm = millis();
    }
  }
}

#endif

#define TUNE_TONE_FREQ   2000

#ifdef CW_ON_CWTX
  uint8_t save_tune_cw;
#endif
uint8_t save_tune_qrp;

void TuneOn()
{
  #ifdef ENABLE_TONE_ON_TUNE
    OutputTone(PIN_OUT_TONE,TUNE_TONE_FREQ);
  #endif
  #ifdef CW_ON_CWTX
    save_tune_cw = trx.CW;
    trx.CW = 1;
  #endif
  save_tune_qrp = trx.qrp;
  trx.qrp = trx.tune = trx.CWTX = trx.TX = 1;
  UpdateFreq();
  UpdateBandCtrl();
  digitalWrite(PIN_OUT_KEY, OUT_KEY_ACTIVE_LEVEL);
  delay(200);
}

void TuneOff()
{
  #ifdef ENABLE_TONE_ON_TUNE
    OutputTone(PIN_OUT_TONE,0);
  #endif
  #ifdef CW_ON_CWTX
    trx.CW = save_tune_cw;
  #endif
  trx.tune = trx.CWTX = trx.TX = 0;
  trx.qrp = save_tune_qrp;
  digitalWrite(PIN_OUT_KEY, !OUT_KEY_ACTIVE_LEVEL);
  UpdateBandCtrl();
  UpdateFreq();
}

void show_swr_measure()
{
  TuneOn();
#ifdef ENABLE_SWR_SENSOR
  disp.DrawSWRMeasureInit();
  disp.DrawSWRMeasure(inSWRF.Read(),inSWRR.Read());
#else
  disp.DrawTuneMode(true,PSTR("TUNE  MODE"));
  long lasttm = millis();
  uint8_t blink = 1;
#endif
  keypad.waitUnpress();
  while (1) {
    PoolKeyboard();
    if (keyb_key) {
      TuneOff();
      return;
    }
#ifdef ENABLE_SWR_SENSOR
    disp.DrawSWRMeasure(inSWRF.Read(),inSWRR.Read());
#else
    if (millis()-lasttm > 500) {
      blink ^= 1;
      disp.DrawText(blink,"TUNE  MODE", 3);
      lasttm = millis();
    }
#endif
  }
}

#ifdef ENABLE_SWR_SENSOR

void show_swr_setupitem(uint8_t idx)
{
  TuneOn();
  keypad.waitUnpress();
  disp.clear();
  int fval = inSWRF.Read();
  int rval = inSWRR.Read();
  disp.DrawSWRItemRaw(fval,rval);
  long tm = millis();
  while (1) {
    PoolKeyboard();
    switch (keyb_key) {
      case 3:
      case 5:
        // save value
        if (fval == rval) Settings[idx] = 10000;
        else Settings[idx] = long(fval+rval)*100 / (fval-rval);
        writeSettings();
      case 1:
      case 2:
      case 4:
        TuneOff();
        return;
    }
    if (millis()-tm > 50) {
      fval = inSWRF.Read();
      rval = inSWRR.Read();
      disp.DrawSWRItemRaw(fval,rval);
      tm = millis();
    }
  }
}

void show_power_setupitem()
{
  TuneOn();
  keypad.waitUnpress();
  disp.clear();
  int fval = inSWRF.Read();
  int rval = inSWRR.Read();
  int pwr = Settings[ID_POWER];
  if (pwr < 10 || pwr > 500) pwr = 50;
  disp.DrawSWRMeasureInit();
  disp.DrawSWRMeasure(inSWRF.Read(),inSWRR.Read(),pwr);
  long tm = millis();
  while (1) {
    PoolKeyboard();
    switch (keyb_key) {
      case 3:
      case 5:
        // save value
        Settings[ID_POWER] = pwr;
        Settings[ID_POWER_VAL] = fval;
        writeSettings();
      case 1:
      case 2:
      case 4:
        TuneOff();
        return;
    }
    long d=encoder.GetDelta();
    if (d <= -MENU_ENCDELTA) {
      pwr--;
      if (pwr < 1) pwr = 1;
      disp.DrawSWRMeasure(fval,rval,pwr);
    } else if (d >= MENU_ENCDELTA) {
      pwr++;
      disp.DrawSWRMeasure(fval,rval,pwr);
    }
    if (millis()-tm > 50) {
      fval = inSWRF.Read();
      rval = inSWRR.Read();
      disp.DrawSWRMeasure(fval,rval,pwr);
      tm = millis();
    }
  }
}

#endif

#ifdef ENABLE_INTERNAL_CWKEY

void show_bank_edit(uint8_t idx)
{
  PGM_P buf[4];
  uint8_t sel=0;
  uint8_t msg_len=0;
  buf[0] = PSTR("PLAY");
  buf[1] = PSTR("RECORD");
  buf[2] = PSTR("CLEAR");
  buf[3] = PSTR("SAVE");
  disp.DrawItems(buf,sel);
  trx.CWClear();
  if (eeprom_read_word(cw_bank_full+idx) == BANK_IS_FULL) {
    eeprom_read_block(trx.cw_buf, cw_bank+idx*CWBANK_SIZE, sizeof(trx.cw_buf));
    msg_len = eeprom_read_byte(cw_bank_len+idx);
    if (msg_len > sizeof(trx.cw_buf)) msg_len = 0; // invalid length
  }
  keypad.waitUnpress();
  while (1) {
    PoolKeyboard();
    switch (keyb_key) {
      case 3:
      case 5:
        switch (sel) {
          case 0:
            // play
            disp.DrawText(true, buf[0],2);
            disp.DrawCWBuf(trx);
            keypad.waitUnpress();
            for (uint8_t i=0; i < msg_len; i++) {
              playChar(trx.cw_buf[i],false);
              PoolKeyboard();
              if (last_key || readDit() || readDah()) break;
            }
            keypad.waitUnpress();
            PoolKeyboard();
            disp.DrawItems(buf,sel);
            break;
          case 1:
            // record
            disp.DrawText(true, buf[1],2);
            trx.CWClear();
            disp.DrawCWBuf(trx);
            last_cw = millis();
            while (1) {
              ReadCWKey(false);
              if (recognizeMorse(' ')) disp.DrawCWBuf(trx);
              if (millis()-last_cw > 2*Settings[ID_CW_BREAK_IN_DELAY]+trx.dit_time*Settings[ID_KEY_WORD_SPACE]/10) break;
            }
            msg_len = trx.cw_buf_idx;
            if (msg_len > 0 && trx.cw_buf[msg_len-1] == ' ') msg_len--; // remove last space
            disp.DrawItems(buf,sel);
            break;
          case 2:
            // clear
            trx.CWClear();
            msg_len = 0;
            eeprom_write_word(cw_bank_full+idx, 0);
            eeprom_write_byte(cw_bank_len+idx, 0);
            disp.BlinkText(buf[2]);
            disp.DrawItems(buf,sel);
            break;
          case 3:
            // save
            eeprom_write_block(trx.cw_buf, cw_bank+idx*CWBANK_SIZE, sizeof(trx.cw_buf));
            eeprom_write_byte(cw_bank_len+idx, msg_len);
            eeprom_write_word(cw_bank_full+idx, BANK_IS_FULL);
            trx.CWClear();
            disp.BlinkText(buf[3]);
            return;
        }
        break;
      case 1:
      case 2:
      case 4:
        // exit
        trx.CWClear();
        return;
    }
    long d=encoder.GetDelta();
    uint8_t dsel=0;
    if (d <= -MENU_ENCDELTA && sel > 0) {
      dsel = -1;
    } else if (d >= MENU_ENCDELTA && sel < 3) {
      dsel = 1;
    }
    if (dsel) {
      sel += dsel;
      disp.DrawSelected(sel);
      delay(MENU_DEBOUNCE);
      encoder.GetDelta();
    }
  }
}

void show_cw_memo()
{
  PGM_P buf[4];
  uint8_t sel=0;
  buf[0] = PSTR("BANK A");
  buf[1] = PSTR("BANK B");
  buf[2] = PSTR("BANK C");
  buf[3] = NULL;
  disp.DrawItems(buf,sel);
  keypad.waitUnpress();
  while (1) {
    PoolKeyboard();
    switch (keyb_key) {
      case 3:
      case 5:
        show_bank_edit(sel);
        disp.DrawItems(buf,sel);
        keypad.waitUnpress();
        break;
      case 1:
      case 2:
      case 4:
        // exit
        return;
    }
    long d=encoder.GetDelta();
    uint8_t dsel=0;
    if (d <= -MENU_ENCDELTA && sel > 0) {
      dsel = -1;
    } else if (d >= MENU_ENCDELTA && sel < 2) {
      dsel = 1;
    }
    if (dsel) {
      sel += dsel;
      disp.DrawSelected(sel);
      delay(MENU_DEBOUNCE);
      encoder.GetDelta();
    }
  }
}

#endif

void edit_item(uint8_t mi)
{
  int val = Settings[mi];
  int minval = (int)pgm_read_word(&SettingsDef[mi].min_value);
  int maxval = (int)pgm_read_word(&SettingsDef[mi].max_value);
  int step = (int)pgm_read_word(&SettingsDef[mi].step);
  if (mi == ID_SI5351_XTAL) {
    #ifdef VFO_SI5351
      vfo5351.out_calibrate_freq();
    #endif
    #ifdef VFO_SI570
      vfo570.out_calibrate_freq();
    #endif
  }
  disp.DrawItemEdit(SettingsDef[mi].title,val);
  keypad.waitUnpress();
  while (1) {
    PoolKeyboard();
    switch (keyb_key) {
      case 3:
      case 5:
        if (keyb_long) {
          // reset to default value
          val = (int)pgm_read_word(&SettingsDef[mi].def_value);
          disp.DrawItemValue(val);
        } else {
          // save value
          Settings[mi] = val;
          writeSettings();
          #ifdef VFO_SI5351
            if (mi == ID_SI5351_XTAL) {
              vfo5351.set_xtal_freq((SI5351_CALIBRATION/10000)*10000+Settings[ID_SI5351_XTAL]);
            }
          #endif
          return;
        }
        break;
      case 1:
      case 2:
      case 4:
        // exit
        return;
    }
    long d=encoder.GetDelta();
    if (d <= -MENU_ENCDELTA) {
      val -= step;
      if (val < minval) val = minval;
      disp.DrawItemValue(val);
    } else if (d >= MENU_ENCDELTA) {
      val += step;
      if (val > maxval) val = maxval;
      disp.DrawItemValue(val);
    }
  }
}

void show_submenu(byte idx, byte len)
{
  PGM_P buf[4];
  uint8_t mi=idx,sel=0;
  for (byte i=0; i < 4; i++) buf[i] = NULL;
  for (byte i=0; i < 4 && mi+i < idx+len; i++) buf[i] = (PGM_P)&SettingsDef[mi+i].title;
  disp.DrawItems(buf,sel);
  keypad.waitUnpress();
  while (1) {
    #ifdef HARDWARE_3_1
      static long lasttm=0;
      if (millis()-lasttm > 1000) {
        lasttm = millis();
        for (byte i=0; i < 4 && mi+i < idx+len; i++) {
          if (mi+i == ID_TEMP_ENABLED) {
            disp.DrawItemsValue(i, (inTEMP.Read()+5)/10, PSTR("C"));
          }
        }
      }
    #endif
    PoolKeyboard();
    switch (keyb_key) {
      case 3:
      case 5:
        switch (mi+sel) {
          case ID_FULL_RESET_CONFIRM:
            resetSettings();
            writeSettings();
            disp.BlinkText(PSTR("RESET"));
            return;
          case ID_FULL_RESET_CANCEL:
            return;
#ifdef ENABLE_SWR_SENSOR
          case ID_SWR_15:
          case ID_SWR_20:
          case ID_SWR_30:
            show_swr_setupitem(mi+sel);
            disp.DrawItems(buf,sel);
            keypad.waitUnpress();
            break;
          case ID_POWER:
            show_power_setupitem();
            disp.DrawItems(buf,sel);
            keypad.waitUnpress();
            break;
#endif
#ifdef HARDWARE_3_1
          case ID_VCC:
            show_vcc_setup();
            disp.DrawItems(buf,sel);
            keypad.waitUnpress();
            break;
#endif
          default:
            edit_item(mi+sel);
            disp.DrawItems(buf,sel);
            keypad.waitUnpress();
        }
        break;
      case 1:
      case 2:
      case 4:
        // exit
        return;
    }
    long d=encoder.GetDelta();
    uint8_t dmi = 0;
    uint8_t deb = 0;
    if (d <= -MENU_ENCDELTA && mi+sel > idx) {
      if (sel > 0) {
        sel--;
        disp.DrawSelected(sel);
      } else {
        dmi = -1;
      }
      deb = 1;
    } else if (d >= MENU_ENCDELTA && mi+sel < idx+len-1) {
      if (sel < 3) {
        sel++;
        disp.DrawSelected(sel);
      } else {
        dmi = 1;
      }
      deb = 1;
    }
    if (dmi) {
      mi += dmi;
      for (byte i=0; i < 4 && mi+i < idx+len; i++) buf[i] = SettingsDef[mi+i].title;
      disp.DrawItems(buf,sel);
    }
    if (deb) {
      delay(MENU_DEBOUNCE);
      encoder.GetDelta();
    }
  }
}

#ifdef HARDWARE_SUPERLED
  #ifdef ENABLE_INTERNAL_CWKEY
    #ifdef ENABLE_SWR_SENSOR
      #define MAINMENU_COUNT   12
    #else
      #define MAINMENU_COUNT   11
    #endif
  #else
    #ifdef ENABLE_SWR_SENSOR
      #define MAINMENU_COUNT   11
    #else
      #define MAINMENU_COUNT   10
    #endif
  #endif
#else
  #ifdef ENABLE_INTERNAL_CWKEY
    #ifdef ENABLE_SWR_SENSOR
      #define MAINMENU_COUNT   11
    #else
      #define MAINMENU_COUNT   10
    #endif
  #else
    #ifdef ENABLE_SWR_SENSOR
      #define MAINMENU_COUNT   10
    #else
      #define MAINMENU_COUNT   9
    #endif
  #endif
#endif

const struct {
  //const char * title;
  char title[12];
  byte idx;
  byte len;
} MainMenu[MAINMENU_COUNT] PROGMEM = {
  {"TUNE", ID_TUNE, 0},
#ifdef HARDWARE_SUPERLED
  {"QRP", ID_QRP, 0},
#endif
  {"SPLIT", ID_SPLIT, 0},
#ifdef ENABLE_INTERNAL_CWKEY
  {"CW KEY", ID_KEY_ENABLE, 9},
  {"CW MEMO", ID_MEMO, 0},
#else
  {"CW VOX", ID_CW_BREAK_IN_DELAY, 1},
#endif
#ifdef HARDWARE_SUPERLED
  {"DISPLAY", ID_DISPLAY_GAUGE, 6},
#else
  {"DISPLAY", ID_POWER_DOWN_DELAY, 3},
#endif
#ifdef HARDWARE_3_1
  {"SENSOR", ID_VCC, 4},
#endif
  {"FREQ", ID_LSB_SHIFT, 3},
  {"CLOCK", ID_CLOCK, 0},
  {"S-METER", ID_SMETER, 0},
#ifdef ENABLE_SWR_SENSOR
  {"SWR", ID_SWR_15, 5},
#endif
  {"FULL RESET", ID_FULL_RESET_CONFIRM, 2}
};

void show_menu()
{
  PGM_P buf[4];
  uint8_t mi=0,sel=0,len,idx;
  for (byte i=0; i < 4; i++) buf[i] = NULL;
  for (byte i=0; i < 4 && mi+i < MAINMENU_COUNT; i++) buf[i] = MainMenu[mi+i].title;
  disp.DrawItems(buf,sel);
  while (1) {
    len = pgm_read_byte(&MainMenu[mi+sel].len);
    idx = pgm_read_byte(&MainMenu[mi+sel].idx);
    PoolKeyboard();
    switch (keyb_key) {
      case 3:
      case 5:
        if (len > 0)
          show_submenu(idx,len);
        else {
          switch (idx) {
            case ID_TUNE:
              show_swr_measure();
              keypad.waitUnpress();
              return;
            case ID_SPLIT:
              if (trx.FreqMemo > 0) trx.split = !trx.split;
              return;
            case ID_QRP:
              trx.qrp = !trx.qrp;
              return;
            case ID_CLOCK: // edit clock
              if (RTC_found()) {
                show_clockmenu();
                keypad.waitUnpress();
              }
              break;
            case ID_SMETER: // edit clock
              show_smetermenu(idx,8);
              keypad.waitUnpress();
              break;
#ifdef ENABLE_INTERNAL_CWKEY
            case ID_MEMO:
              show_cw_memo();
              keypad.waitUnpress();
              break;
#endif
          }
        }
        disp.DrawItems(buf,sel);
        break;
      case 1:
      case 2:
      case 4:
        // exit
        return;
        break;
    }
    long d=encoder.GetDelta();
    uint8_t dmi = 0;
    uint8_t deb = 0;
    if (d <= -MENU_ENCDELTA && mi+sel > 0) {
      if (sel > 0) {
        sel--;
        disp.DrawSelected(sel);
      } else {
        dmi = -1;
      }
      deb = 1;
    } else if (d >= MENU_ENCDELTA && mi+sel < MAINMENU_COUNT-1) {
      if (sel < 3) {
        sel++;
        disp.DrawSelected(sel);
      } else {
        dmi = 1;
      }
      deb = 1;
    }
    if (dmi) {
      mi += dmi;
      for (byte i=0; i < 4 && mi+i < MAINMENU_COUNT; i++) buf[i] = MainMenu[mi+i].title;
      disp.DrawItems(buf,sel);
    }
    if (deb) {
      delay(MENU_DEBOUNCE);
      encoder.GetDelta();
    }
  }
}

void select_band()
{
  uint8_t mi=0,sel=0;
  if (BAND_COUNT <= 4) mi = 0;
  else mi = trx.BandIndex;
  if (mi+4 >= BAND_COUNT && BAND_COUNT >= 4) {
    mi = BAND_COUNT-4;
    sel = trx.BandIndex-mi;
  }
  disp.DrawFreqItems(trx,mi,sel);
  while (1) {
    PoolKeyboard();
    switch (keyb_key) {
      case 4:
      case 5:
        trx.SelectBand(mi+sel);
        return;
      case 1:
      case 2:
      case 3:
        // exit
        return;
    }
    long d=encoder.GetDelta();
    if (d <= -MENU_ENCDELTA && mi+sel > 0) {
      if (sel > 0) {
        sel--;
        disp.DrawSelected(sel);
      } else {
        mi--;
        disp.DrawFreqItems(trx,mi,sel);
      }
      delay(MENU_DEBOUNCE);
      encoder.GetDelta(); // debounce
    } else if (d >= MENU_ENCDELTA && mi+sel < BAND_COUNT-1) {
      if (sel < 3) {
        sel++;
        disp.DrawSelected(sel);
      } else {
        mi++;
        disp.DrawFreqItems(trx,mi,sel);
      }
      delay(MENU_DEBOUNCE);
      encoder.GetDelta(); // debounce
    }
  }
}