#ifdef RTC_ENABLE

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
    if (d <= -3 && val > 0) {
      val--;
      disp.DrawItemValue(val);
    } else if (d >= 3 && val < maxval) {
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
  buf[0] = PSTR("Hour");
  buf[1] = PSTR("Minutes");
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
      delay(300);
      encoder.GetDelta();
    }
  }
}

#endif

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
    if (millis()-tm > 200) {
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
    if (d <= -3 && mi+sel > idx) {
      if (sel > 0) {
        sel--;
        disp.DrawSelected(sel);
      } else {
        mi--;
        for (byte i=0; i < 4 && mi+i < idx+len; i++) {
          buf[i] = SettingsDef[mi+i].title;
          vals[i] = Settings[mi+i];
        }
        disp.DrawSMeterItems(buf,vals,sel);
      }
      delay(300);
      encoder.GetDelta();
    } else if (d >= 3 && mi+sel < idx+len-1) {
      if (sel < 3) {
        sel++;
        disp.DrawSelected(sel);
      } else {
        mi++;
        for (byte i=0; i < 4 && mi+i < idx+len; i++) {
          buf[i] = SettingsDef[mi+i].title;
          vals[i] = Settings[mi+i];
        }
        disp.DrawSMeterItems(buf,vals,sel);
      }
      delay(300);
      encoder.GetDelta();
    }
  }
}

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
            if (mi == ID_SI5351_XTAL) vfo5351.set_xtal_freq((SI5351_CALIBRATION/10000)*10000+Settings[ID_SI5351_XTAL]);
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
    if (d <= -3) {
      val -= step;
      if (val < minval) val = minval;
      disp.DrawItemValue(val);
    } else if (d >= 3) {
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
    PoolKeyboard();
    switch (keyb_key) {
      case 3:
      case 5:
        if (mi+sel == ID_FULL_RESET_CONFIRM) {
          disp.clear();
          resetSettings();
          writeSettings();
          delay(300);
          return;
        } if (mi+sel == ID_FULL_RESET_CANCEL) {
          return;
        } else {
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
    if (d <= -3 && mi+sel > idx) {
      if (sel > 0) {
        sel--;
        disp.DrawSelected(sel);
      } else {
        mi--;
        for (byte i=0; i < 4 && mi+i < idx+len; i++) buf[i] = SettingsDef[mi+i].title;
        disp.DrawItems(buf,sel);
      }
      delay(300);
      encoder.GetDelta();
    } else if (d >= 3 && mi+sel < idx+len-1) {
      if (sel < 3) {
        sel++;
        disp.DrawSelected(sel);
      } else {
        mi++;
        for (byte i=0; i < 4 && mi+i < idx+len; i++) buf[i] = SettingsDef[mi+i].title;
        disp.DrawItems(buf,sel);
      }
      delay(300);
      encoder.GetDelta();
    }
  }
}

#define MAINMENU_COUNT   8

const struct {
  //const char * title;
  char title[12];
  byte idx;
  byte len;
} MainMenu[MAINMENU_COUNT] PROGMEM = {
  {"KEY", ID_KEY_ENABLE, 7},
  {"CW", ID_CW_VOX, 4},
  {"SPLIT", ID_SPLIT, 0},
  {"POWER", ID_POWER_DOWN_DELAY, 3},
  {"FREQ", ID_LSB_SHIFT, 3},
  {"CLOCK", ID_CLOCK, 0},
  {"S-METER", ID_SMETER, 0},
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
            case ID_SPLIT:
              trx.split = !trx.split;
              return;
            case ID_CLOCK: // edit clock
              #ifdef RTC_ENABLE
                show_clockmenu();
                keypad.waitUnpress();
              #endif
              break;
            case ID_SMETER: // edit clock
              show_smetermenu(idx,8);
              keypad.waitUnpress();
              break;
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
    if (d <= -3 && mi+sel > 0) {
      if (sel > 0) {
        sel--;
        disp.DrawSelected(sel);
      } else {
        mi--;
        for (byte i=0; i < 4 && mi+i < MAINMENU_COUNT; i++) buf[i] = MainMenu[mi+i].title;
        disp.DrawItems(buf,sel);
      }
      delay(300);
      encoder.GetDelta();
    } else if (d >= 3 && mi+sel < MAINMENU_COUNT-1) {
      if (sel < 3) {
        sel++;
        disp.DrawSelected(sel);
      } else {
        mi++;
        for (byte i=0; i < 4 && mi+i < MAINMENU_COUNT; i++) buf[i] = MainMenu[mi+i].title;
        disp.DrawItems(buf,sel);
      }
      delay(300);
      encoder.GetDelta();
    }
  }
}
