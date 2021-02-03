#include "disp_oled128x64.h"
// SSD1306Ascii library https://github.com/greiman/SSD1306Ascii
#include "SSD1306AsciiAvrI2c.h"
#include "fonts\lcdnums14x24mod.h"
#include "fonts\quad7x8.h"
#include "utils.h"
#ifdef RTC_ENABLE
  #include "RTC.h"
#endif

#define I2C_ADD_DISPLAY_OLED 0x3C

SSD1306AsciiAvrI2c oled64;

long last_freq;
uint8_t last_tx;
int last_BandIndex;
uint8_t last_wpm;
uint8_t last_mode;
uint8_t last_split;
uint8_t last_mem;
uint8_t last_cwmode;
uint8_t last_lock;
uint8_t last_brightness;
uint8_t last_attpre;
uint8_t init_smetr;
uint8_t last_sm[15];
long last_tmtm;
long last_cw_tm;
uint16_t last_VCC = 0xFFFF;

void Display_OLED128x64::setBright(uint8_t brightness)
{
  if (brightness < 0) brightness = 0;
  if (brightness > 15) brightness = 15;
  if (brightness == 0) 
    oled64.ssd1306WriteCmd(SSD1306_DISPLAYOFF);
  else {
    if (last_brightness == 0)
      oled64.ssd1306WriteCmd(SSD1306_DISPLAYON);
    oled64.setContrast(brightness << 4);
  }
  last_brightness = brightness; 
}

void Display_OLED128x64::setup() 
{
#ifdef DISPLAY_OLED128x64
  oled64.begin(&Adafruit128x64, I2C_ADD_DISPLAY_OLED);
#endif
#ifdef DISPLAY_OLED_SH1106_128x64
  oled64.begin(&SH1106_128x64, I2C_ADD_DISPLAY_OLED);
#endif
  clear();
  last_brightness=0;
}

extern int Settings[];

void Display_OLED128x64::Draw(TRX& trx) 
{
  long freq;
  
  if (trx.split && trx.TX && trx.FreqMemo > 0) freq = trx.FreqMemo;
  else freq = trx.Freq;

  if (freq != last_freq) {
    char buf[10];
    long f = freq/10;
    buf[8] = '0'+f%10; f/=10;
    buf[7] = '0'+f%10; f/=10;
    buf[6] = '.';
    buf[5] = '0'+f%10; f/=10;
    buf[4] = '0'+f%10; f/=10;
    buf[3] = '0'+f%10; f/=10;
    buf[2] = '.';
    buf[1] = '0'+f%10; f/=10;
    if (f > 0) buf[0] = '0'+f;
    else buf[0] = ':';
    buf[9] = 0;
    oled64.setFont(lcdnums14x24mod);
    oled64.setCursor(1, 2);
    oled64.print(buf);
    last_freq = freq;
  }

  oled64.setFont(X11fixed7x14);
  
  if (trx.TX != last_tx) {
    oled64.setCursor(0,0);
    if ((last_tx=trx.TX) != 0) oled64.print("TX");
    else oled64.print("  ");
  }

/*  if (trx.Lock != cur64_lock) {
    oled64.setCursor(25,0);
    char bb[2];
    bb[0]=0x24; bb[1]=0;
    if ((cur64_lock=trx.Lock) != 0) oled64.print(bb); // dollar
    else oled64.print(" ");
  } */
  
  if (trx.CW != last_cwmode) {
    oled64.setCursor(64,0);
    if ((last_cwmode = trx.CW) != 0) oled64.print("CW");
    else oled64.print("  ");
  }

  if (trx.sideband != last_mode) {
    oled64.setCursor(28,0);
    if ((last_mode = trx.sideband) == LSB) oled64.print("LSB");
    else oled64.print("USB");
  }

#ifdef RTC_ENABLE
  if (millis()-last_tmtm > 200) {
    RTCData d;
    char buf[7],*pb;
    last_tmtm=millis();
    RTC_Read(&d);   
    //sprintf(buf,"%2x:%02x",d.hour,d.min);
    pb=cwr_hex2sp(buf,d.hour);
    if (millis()/1000 & 1) *pb++=':';
    else *pb++=' ';
    pb=cwr_hex2(pb,d.min);
    oled64.setCursor(128-35,0);
    oled64.print(buf);
  }
#endif

  if (trx.TX && trx.cw_buf_idx > 0) {
    oled64.setFont(System5x7);
    if (init_smetr) {
      // clear s-meter
      oled64.setCursor(0,5);
      for (byte i=0; i < 22; i++) oled64.print(' ');
    }
    oled64.setCursor(0,6);
    for (byte i=0; i < sizeof(trx.cw_buf); i++) {
      if (i == 21) oled64.setCursor(0,7);
      oled64.print(trx.cw_buf[i]);
    }
    init_smetr = 0;
    last_attpre=last_split=last_wpm=last_mem=0xFF;
    last_cw_tm = millis();
  } else if (millis()-last_cw_tm > Settings[ID_CW_DECODER_HIDE_DELAY]*1000) {
    if (!init_smetr) {
      // clear lines
      oled64.setFont(System5x7);
      for (byte k=5; k <= 7; k++) {
        oled64.setCursor(0,k);
        for (byte i=0; i < 22; i++) oled64.print(' ');
      }
    }

    oled64.setFont(quad7x8);

    if (!init_smetr) {
      init_smetr=1;
      for (byte j=0; j < 15; j++) {
        byte x = 4+j*8;
        if (j < 9 || !(j&1)) {
          oled64.setCursor(x,5);
          oled64.write(0x23);
          oled64.setCursor(x,6);
          oled64.write(0x24);
        }
      }
    }
  
    for (byte j=0; j < 15; j++) {
      byte x = 4+j*8;
      if (j < trx.SMeter)
      {
        if (!last_sm[j])
        {
          oled64.setCursor(x, 5);
          oled64.write(0x21);
          oled64.setCursor(x, 6);
          oled64.write(0x22);
          last_sm[j] = 1;
        }
      }
      else
      {
        if (last_sm[j])
        {
          if (j < 9 || !(j & 1))
          {
            oled64.setCursor(x, 5);
            oled64.write(0x23);
            oled64.setCursor(x, 6);
            oled64.write(0x24);
          }
          else
          {
            oled64.setCursor(x, 5);
            oled64.write(0x20);
            oled64.setCursor(x, 6);
            oled64.write(0x20);
          }
          last_sm[j] = 0;
        }
      }
    }

    oled64.setFont(System5x7);

    if (trx.AttPre != last_attpre)
    {
      oled64.setCursor(4, 7);
      switch (last_attpre = trx.AttPre)
      {
      case 0:
        oled64.print("   ");
        break;
      case 1:
        oled64.print("ATT");
        break;
      case 2:
        oled64.print("PRE");
        break;
      }
    }

    if (trx.Freq == trx.FreqMemo)
    {
      if (last_mem != 1)
      {
        oled64.setCursor(33, 7);
        oled64.print("MEM");
        last_mem = 1;
      }
    }
    else
    {
      if (last_mem != 0)
      {
        oled64.setCursor(33, 7);
        oled64.print("   ");
        last_mem = 0;
      }
    }

    if (trx.split != last_split)
    {
      oled64.setCursor(62, 7);
      if ((last_split = trx.split) != 0)
        oled64.print("SPL  ");
      else {
        #ifdef HARDWARE_3_1
          last_VCC = 0xFFFF;
        #else
          oled64.print("    ");
        #endif
      }
    }

    #ifdef HARDWARE_3_1
    if (!last_split && trx.VCC != last_VCC) {
      last_VCC = trx.VCC;
      oled64.setCursor(62, 7);
      if (last_VCC > 1000) {
        oled64.print(last_VCC/1000);
        oled64.print('.');
        oled64.print(last_VCC/100 % 10);
        oled64.print('v');
      } else
        oled64.print("    ");
    }
    #endif

    if (trx.Lock != last_lock)
    {
      oled64.setCursor(128-4*7, 7);
      if ((last_lock = trx.Lock) != 0)
        oled64.print("LOCK");
      else
        oled64.print("    ");
    }
    
    /*if (trx.cw_speed != last_wpm)
    {
      oled64.setCursor(128 - 6 * 7, 7);
      if (trx.cw_speed < 10)
        oled64.print(' ');
      oled64.print(trx.cw_speed);
      oled64.print(" wpm");
      last_wpm = trx.cw_speed;
    }*/
  }
}

void Display_OLED128x64::DrawItemEdit(PGM_P text, int value)
{
  oled64.clear();
  oled64.setFont(X11fixed7x14);
  oled64.setCursor(10,1);
  oled64.print((const __FlashStringHelper*)text);
  oled64.setCursor(0,4);
  oled64.print("EDIT:");
  oled64.setCursor(50,4);
  oled64.print(value);
  oled64.print("     ");
}

void Display_OLED128x64::DrawItemValue(int value)
{
  oled64.setFont(X11fixed7x14);
  oled64.setCursor(50,4);
  oled64.print(value);
  oled64.print("     ");
}

void Display_OLED128x64::DrawItems(PGM_P* text, uint8_t selected)
{
  oled64.clear();
  oled64.setFont(X11fixed7x14);
  for (byte i=0; i < 4 && text[i]; i++) {
    oled64.setCursor(2,i<<1);
    if (i == selected) oled64.print('>');
    oled64.setCursor(12,i<<1);
    oled64.print((const __FlashStringHelper*)(text[i]));
  }
}

void Display_OLED128x64::DrawSelected(uint8_t selected)
{
  oled64.setFont(X11fixed7x14);
  for (byte i=0; i < 4; i++) {
    oled64.setCursor(2,i<<1);
    if (i == selected) oled64.print('>');
    else oled64.print(' ');
  }
}

void Display_OLED128x64::DrawSMeterItems(PGM_P* text, const int* vals, uint8_t selected)
{
  oled64.clear();
  oled64.setFont(X11fixed7x14);
  for (byte i=0; i < 4 && text[i]; i++) {
    oled64.setCursor(2,i<<1);
    if (i == selected) oled64.print('>');
    oled64.setCursor(12,i<<1);
    oled64.print((const __FlashStringHelper*)(text[i]));
    oled64.setCursor(50,i<<1);
    oled64.print(vals[i]);
  }
}

void Display_OLED128x64::clear()
{
  oled64.clear();
  last_freq=0;
  last_lock=last_mem=last_split=last_attpre=last_cwmode=last_mode=last_tx=last_BandIndex=0xFF;
  last_wpm=0;
  init_smetr=0;
  for (uint8_t i=0; i < 15; i++) last_sm[i]=0;
  last_cw_tm=last_tmtm=0;
}

