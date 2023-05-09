// required SSD1306Ascii library https://github.com/greiman/SSD1306Ascii

#include "disp_oled128x64.h"
#include "i2c.h"
#include <SSD1306Ascii.h>
#include "fonts\X11fixed7x14mod.h"
#include "fonts\System5x7mod.h"
#include "fonts\lcdnums14x24mod.h"
#include "fonts\quad7x8.h"
#include "utils.h"
#include "RTC.h"

class OledI2C : public SSD1306Ascii {
  protected:
    uint8_t m_i2cAddr;
    uint8_t m_nData;
  public:
    void begin(const DevType* dev, uint8_t i2cAddr) {
      m_nData = 0;
      m_i2cAddr = i2cAddr;
      init(dev);
    }
    void writeByteXY(uint8_t x, uint8_t y, uint8_t data);
  protected:
    void writeDisplay(uint8_t b, uint8_t mode);
};

#define CTRL_LAST_CMD 0x00
#define CTRL_NEXT_CMD 0x80
#define CTRL_LAST_RAM 0x40
#define CTRL_NEXT_RAM 0xC0

void OledI2C::writeByteXY(uint8_t x, uint8_t y, uint8_t data)
{
  if (m_nData) {
    i2c_end();
    m_nData = 0;
  }
  i2c_begin_write(m_i2cAddr);
  x += m_colOffset;
  i2c_write(CTRL_NEXT_CMD);
  i2c_write(SSD1306_SETLOWCOLUMN | (x & 0xF));
  i2c_write(CTRL_NEXT_CMD);
  i2c_write(SSD1306_SETHIGHCOLUMN | (x >> 4));
  i2c_write(CTRL_NEXT_CMD);
  i2c_write(SSD1306_SETSTARTPAGE | y);
  i2c_write(CTRL_LAST_RAM);
  i2c_write(data);
  i2c_end();
}

void OledI2C::writeDisplay(uint8_t b, uint8_t mode) 
{
  if (m_nData && mode == SSD1306_MODE_CMD) {
    i2c_end();
    m_nData = 0;
  }
  if (m_nData == 0) {
    i2c_begin_write(m_i2cAddr);
    i2c_write(mode == SSD1306_MODE_CMD ? CTRL_LAST_CMD : CTRL_LAST_RAM);
  }
  i2c_write(b);
  if (mode == SSD1306_MODE_RAM_BUF) {
    m_nData = 1;
  } else {
    i2c_end();
    m_nData = 0;
  }
}

#define I2C_ADR_DISPLAY_OLED 0x3C

OledI2C oled64;

long last_freq;
uint8_t last_tx;
int last_BandIndex;
uint8_t last_mode;
uint8_t last_split;
uint8_t last_qrp;
uint8_t last_mem;
uint8_t last_cwmode;
uint8_t last_lock;
uint8_t last_brightness;
uint8_t last_attpre;
uint8_t init_smetr;
uint8_t last_temp;
uint8_t last_sm[15];
uint8_t last_smval = 0xFF;
uint8_t dec_visible = 0;
uint8_t last_cw_idx = 0xFF;
long last_tmtm;
uint16_t last_VCC = 0xFFFF;
float last_swr = 0;
int last_swrx = -1;

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
  oled64.begin(&SH1106_128x64, I2C_ADR_DISPLAY_OLED);
#endif
  clear();
  last_brightness=0;
}

extern int Settings[];

// Image size 128x16 pix
// Row count 2

const byte smeter[] PROGMEM={
0x80, 0x02,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x82, 0x92, 
  0x92, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x4E, 0x8A, 0x8A, 0x8A, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0xE2, 0x12, 0x0E, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x92, 0x92, 
  0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x38, 0x10, 0x00, 0x84, 
  0xC2, 0xA2, 0x92, 0x8C, 0x00, 0x7C, 0x82, 0x82, 0x82, 0x7C, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x10, 0x38, 0x10, 0x00, 0x30, 0x28, 0x24, 0xFE, 0x20, 0x00, 0x7C, 0x82, 
  0x82, 0x82, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00,  // row 0
  0x00, 0x3E, 0x3E, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x38, 0x3C, 
  0x38, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 
  0x30, 0x30, 0x38, 0x3C, 0x38, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 
  0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x38, 0x3C, 0x38, 0x30, 0x30, 0x30, 0x30, 0x30, 
  0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x38, 0x3C, 
  0x38, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 
  0x30, 0x30, 0x30, 0x3C, 0x3C, 0x3C, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 
  0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3C, 0x3C, 0x3C, 0x30, 
  0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3E, 
  0x3E, 0x00,  // row 1
};


void drawBitmap(byte x, byte y, const byte* pbm) 
{
  byte w, h;
  w = pgm_read_byte(pbm++);
  h = pgm_read_byte(pbm++);
  for (byte j = 0; j < h; j++, y++) {
    oled64.setCursor(x, y);
    for (byte i=0; i < w; i++)
      oled64.ssd1306WriteRamBuf(pgm_read_byte(pbm++)); // seq write
  }
  oled64.setRow(0); // end seq write
}

static byte last_pix = 0;
static int last_x = -1;
static int last_y = -1;

void drawPixel(byte x, byte y, byte color)
{
  byte yy = y >> 3;
  if (x != last_x || yy != last_y) {
    if (last_x >= 0) oled64.writeByteXY(last_x, last_y, last_pix);
    last_x=x;
    last_y=yy;
    last_pix=0;
  }
  if (color) last_pix |= 1 << (y & 0x7);
  else last_pix = 0; //&= ~(1 << (y & 0x7));
}

#define _swap_int16_t(a, b)                                                    \
  {                                                                            \
    int16_t t = a;                                                             \
    a = b;                                                                     \
    b = t;                                                                     \
  }

void drawLine(int x0, int y0, int x1, int y1, byte ymax, byte color) 
{
  last_x = last_y = -1;
  byte steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }
  if (x0 > x1) {
    _swap_int16_t(x0, x1);
    _swap_int16_t(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) ystep = 1;
  else ystep = -1;

  for (; x0 <= x1; x0++) {
    if (steep) {
      if (x0 < ymax) drawPixel(y0, x0, color);
    } else {
      if (y0 < ymax) drawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
  drawPixel(-1,-1,0); // flush
}

void Display_OLED128x64::Draw(TRX& trx) 
{
#ifdef HARDWARE_SUPERLED
  byte gauge = Settings[ID_DISPLAY_GAUGE];
#else
  const byte gauge = 0;
#endif

  oled64.setFont(System5x7mod);

#ifdef ENABLE_INTERNAL_CWKEY
  if (trx.TX && trx.cw_buf_idx > 0) {
    if (!dec_visible) {
      dec_visible = 1;
      oled64.clear(0,127,4,7);
      last_cw_idx = 0xFF;
      init_smetr = 0;
      last_temp=last_qrp=last_lock=last_mem=last_split=last_attpre=last_cwmode=
        last_cw_idx=last_mode=0xFF;
      last_VCC = 0xFFFF;
    }
    if (trx.cw_buf_idx != last_cw_idx) {
      DrawCWBuf(trx);
      last_cw_idx = trx.cw_buf_idx;
    }
  } else if (!trx.TX && dec_visible) {
    // clear lines
    oled64.clear(0,127,4,7);
    trx.CWClear();
    dec_visible = 0;
    last_freq = 0; // refresh freq
  }
#endif

  if (!gauge) {
    long freq;
    
    freq = trx.CurrentFreq;
  
    if (freq != last_freq) {
      char buf[10];
      long f = freq/10;
      buf[8] = '0'+f%10; f/=10;
      buf[7] = '0'+f%10; f/=10;
      buf[6] = '/';
      buf[5] = '0'+f%10; f/=10;
      buf[4] = '0'+f%10; f/=10;
      buf[3] = '0'+f%10; f/=10;
      buf[2] = '/';
      buf[1] = '0'+f%10; f/=10;
      if (f > 0) buf[0] = '0'+f;
      else buf[0] = ':';
      buf[9] = 0;
      oled64.setFont(lcdnums14x24mod);
      oled64.setCursor(1, 2);
      oled64.print(buf);
      last_freq = freq;
    }
  }

  byte top_line_y = (gauge ? 5 : 0);

  if (!gauge || !dec_visible) {
    oled64.setFont(X11fixed7x14mod);
    
    if (trx.TX != last_tx || trx.qrp != last_qrp) {
      oled64.setCursor(0,top_line_y);
      if ((last_tx=trx.TX) != 0) oled64.print("TX ");
      else if ((last_qrp=trx.qrp) != 0) oled64.print("QRP");
      else oled64.print("   ");
    }
   
    oled64.setCursor(28,top_line_y);
  #ifdef ENABLE_SWR_SENSOR
    if (trx.TX) {
      int swr = trx.swr*10;
      if (swr > 0 && swr != last_swrx) {
        oled64.print(swr/10);
        oled64.print(".");
        oled64.print(swr%10);
        last_swrx = swr;
        last_cwmode = last_mode = 0xFF;
      }
    } else 
  #endif
    if (trx.CW != last_cwmode || trx.sideband != last_mode) {
      if ((last_cwmode = trx.CW) != 0) {
        if ((last_mode = trx.sideband) == LSB) oled64.print("CWL");
        else oled64.print("CWU");
      } else {
        if ((last_mode = trx.sideband) == LSB) oled64.print("LSB");
        else oled64.print("USB");
      }
      last_swrx = -1;
    }
  
    if (RTC_found() && millis()-last_tmtm > 200) {
      RTCData d;
      char buf[7],*pb;
      last_tmtm=millis();
      RTC_Read(&d);   
      //sprintf(buf,"%2x:%02x",d.hour,d.min);
      pb=cwr_hex2sp(buf,d.hour);
      if (millis()/1000 & 1) *pb++=':';
      else *pb++=' ';
      pb=cwr_hex2(pb,d.min);
      oled64.setCursor(128-35,top_line_y);
      oled64.print(buf);
    }
  
    #ifdef HARDWARE_3_1
      if (Settings[ID_TEMP_ENABLED]) {
        oled64.setCursor(60,top_line_y);
        uint8_t new_temp = int16_t((trx.temp+5)/10) % 100;
        if (new_temp >= Settings[ID_TEMP_WARN] || new_temp >= Settings[ID_TEMP_STOP]) {
          static long lasttm = 0;
          static uint8_t tmst = 0;
          if (millis()-lasttm > (new_temp >= Settings[ID_TEMP_STOP] ? 200 : 500)) {
            tmst ^= 1;
            lasttm = millis();
            if (tmst == 1) {
              oled64.print(new_temp);
              oled64.print('C');
            } else {
              oled64.print("   ");
            }
          }
        } else if (new_temp != last_temp) {
          oled64.print(new_temp);
          oled64.print('C');
        }
        last_temp = new_temp;
      }
    #endif
  }

#ifdef HARDWARE_SUPERLED
  if (gauge) {
    const byte smeter_x[] = {0,4,8,13,21,31,40,49,59,69,78,88,98,109,118,127};
    if (!init_smetr) {
      drawBitmap(0,0,smeter);
      init_smetr=1;
    }
    static long last_draw = 0;
#ifdef ENABLE_SWR_SENSOR
    byte new_val = smeter_x[trx.TX ? int(trx.pwr*15) : trx.SMeter];
#else
    byte new_val = smeter_x[trx.TX ? 0 : trx.SMeter];
#endif
    if (millis()-last_draw > 10) {
      if (new_val < 127) new_val += (int(smeter_x[trx.SMeter+1])-int(smeter_x[trx.SMeter])) * trx.SMeterFrac / 100;
      if (new_val > 127) new_val = 127;
      if (new_val != last_smval) {
        drawLine(64,80,last_smval,16,40,0);
        last_smval = new_val;
        drawLine(64,80,last_smval,16,40,1);
      }
      last_draw = millis();
    }
  } else 
#endif
  if (!dec_visible) {
    if (!init_smetr) {
      // clear lines
      oled64.setFont(System5x7mod);
      for (byte k=5; k <= 7; k++) {
        oled64.setCursor(0,k);
        for (byte i=0; i < 22; i++) oled64.print(' ');
      }
      init_smetr = 1;
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

    uint8_t meter_value;
    if (trx.TX) {
      meter_value = 15*trx.pwr;
    } else {
      meter_value = trx.SMeter;
    }
  
    for (byte j=0; j < 15; j++) {
      byte x = 4+j*8;
      if (j < meter_value) {
        if (!last_sm[j]) {
          oled64.setCursor(x, 5);
          oled64.write(0x21);
          oled64.setCursor(x, 6);
          oled64.write(0x22);
          last_sm[j] = 1;
        }
      } else {
        if (last_sm[j]) {
          if (j < 9 || !(j & 1)) {
            oled64.setCursor(x, 5);
            oled64.write(0x23);
            oled64.setCursor(x, 6);
            oled64.write(0x24);
          } else {
            oled64.setCursor(x, 5);
            oled64.write(0x20);
            oled64.setCursor(x, 6);
            oled64.write(0x20);
          }
          last_sm[j] = 0;
        }
      }
    }
  }

  if (!dec_visible) {
    oled64.setFont(System5x7mod);
  
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
  
    if (abs(trx.Freq-trx.FreqMemo) < FREQ_GRANULATION) {
      if (last_mem != 1) {
        oled64.setCursor(33, 7);
        oled64.print("MEM");
        last_mem = 1;
      }
    } else {
      if (last_mem != 0) {
        oled64.setCursor(33, 7);
        oled64.print("   ");
        last_mem = 0;
      }
    }
  
    if (trx.split != last_split) {
      oled64.setCursor(62, 7);
      if ((last_split = trx.split) != 0) {
        oled64.print("SPL  ");
      } else {
        #ifdef HARDWARE_3_1
          last_VCC = 0xFFFF;
        #else
          oled64.print("    ");
        #endif
      }
    }
  
    #ifdef HARDWARE_3_1
      {
        int new_vcc = trx.VCC*10;
        if (!trx.split && new_vcc != last_VCC) {
          oled64.setCursor(62, 7);
          if (new_vcc > 10) {
            oled64.print(new_vcc/10);
            oled64.print('.');
            oled64.print(new_vcc % 10);
            oled64.print('v');
          } else {
            oled64.print("    ");
          }
          last_VCC = new_vcc;
        }
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
  }
}

void Display_OLED128x64::DrawItemEdit(PGM_P text, int value)
{
  oled64.clear();
  oled64.setFont(X11fixed7x14mod);
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
  oled64.setFont(X11fixed7x14mod);
  oled64.setCursor(50,4);
  oled64.print(value);
  oled64.print("     ");
}

void Display_OLED128x64::DrawItemsValue(byte idx, int value, PGM_P sfx)
{
  oled64.setFont(X11fixed7x14mod);
  oled64.setCursor(75,idx<<1);
  oled64.print(value);
  oled64.print((const __FlashStringHelper*)sfx);
  oled64.print("   ");
}

void Display_OLED128x64::DrawItems(PGM_P* text, uint8_t selected)
{
  oled64.clear();
  oled64.setFont(X11fixed7x14mod);
  for (byte i=0; i < 4 && text[i]; i++) {
    oled64.setCursor(2,i<<1);
    if (i == selected) oled64.print('>');
    oled64.setCursor(12,i<<1);
    oled64.print((const __FlashStringHelper*)(text[i]));
  }
}

void Display_OLED128x64::DrawSelected(uint8_t selected)
{
  oled64.setFont(X11fixed7x14mod);
  for (byte i=0; i < 4; i++) {
    oled64.setCursor(2,i<<1);
    if (i == selected) oled64.print('>');
    else oled64.print(' ');
  }
}

void Display_OLED128x64::DrawSMeterItems(PGM_P* text, const int* vals, uint8_t selected)
{
  oled64.clear();
  oled64.setFont(X11fixed7x14mod);
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
  last_temp=last_qrp=last_lock=last_mem=last_split=last_attpre=last_cwmode=
    last_mode=last_tx=last_BandIndex=0xFF;
  dec_visible=init_smetr=0;
  for (uint8_t i=0; i < 15; i++) last_sm[i]=0;
  last_smval=0xFF;
  last_tmtm=0;
  last_VCC=0xFFFF;
}

void Display_OLED128x64::DrawFreqItems(TRX& trx, uint8_t idx, uint8_t selected)
{
  oled64.clear();
  oled64.setFont(X11fixed7x14mod);
  for (byte i=0; i < 4 && idx+i < BAND_COUNT; i++) {
    oled64.setCursor(2,i<<1);
    if (i == selected) oled64.print('>');
    oled64.setCursor(12,i<<1);
    const struct _Bands& info = trx.GetBandInfo(idx+i);
    oled64.print(info.mc);
    oled64.setCursor(64,i<<1);
    int f = info.start / 1000000L;
    if (f > 9) oled64.print(f);
    else {
      oled64.print(' ');
      oled64.print(f);
    }
    oled64.print('.');
    f = (info.start / 1000) % 1000;
    if (f <= 9) oled64.print("00");
    else if (f <= 99) oled64.print('0');
    oled64.print(f);
  }
}

#ifdef HARDWARE_3_1

void Display_OLED128x64::DrawVCC(int rawdata, int vcc)
{
  oled64.setFont(X11fixed7x14mod);
  oled64.setCursor(0,2);
  oled64.print("ADC ");
  oled64.print(rawdata);
  oled64.clearToEOL();
  oled64.setCursor(0,4);
  oled64.print("VCC ");
  oled64.print(vcc / 10);
  oled64.print(".");
  oled64.print(vcc % 10);
  oled64.clearToEOL();
}

#endif

void Display_OLED128x64::DrawText(bool visible, PGM_P text, uint8_t y)
{
  oled64.clear();
  if (visible) {
    oled64.setFont(X11fixed7x14mod);
    oled64.setCursor((128-strlen_P(text)*7)/2,y);
    oled64.print((const __FlashStringHelper*)text);
  }
}

void Display_OLED128x64::BlinkText(PGM_P text)
{
  for (uint8_t i=1; i < 8; i++) {
    DrawText(i&1, text, 3);
    delay(250);
  }
}

#ifdef ENABLE_INTERNAL_CWKEY

void Display_OLED128x64::DrawCWBuf(TRX& trx)
{
  oled64.setFont(System5x7mod);
  oled64.setCursor(0,4);
  for (byte i=0; i < sizeof(trx.cw_buf); i++) {
    if (i == 21 || i == 42 || i == 63) oled64.setCursor(0,4+i/21);
    oled64.print(trx.cw_buf[i]);
  }
}

#endif

#ifdef ENABLE_SWR_SENSOR

// Image size 128x16 pix
// Row count 2
const byte swrmeter[] PROGMEM={
0x80, 0x02,
  0x00, 0x00, 0x22, 0x3F, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x22, 0x3F, 0x20, 0x00, 0x20, 0x00, 0x13, 0x25, 0x25, 0x19, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x31, 0x29, 0x26, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x21, 0x25, 0x1A, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x24, 0x24, 0x24, 0x18, 0x18, 0x24, 0x24, 0x24, 
  0x18, 0x00,  // row 0
  0x1F, 0x1F, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
  0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
  0x18, 0x18, 0x18, 0x1C, 0x1F, 0x1C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
  0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
  0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1C, 0x1F, 0x1C, 0x18, 0x18, 0x18, 0x18, 
  0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
  0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1C, 0x1F, 0x1C, 
  0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
  0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
  0x1F, 0x1F,  // row 1
};

void Display_OLED128x64::DrawSWRMeasureInit()
{
  oled64.clear();
  drawBitmap(0,0,swrmeter);
  oled64.setFont(X11fixed7x14mod);
  oled64.setCursor(0,6);
  oled64.print("SWR");
  oled64.setCursor(70,6);
  oled64.print("PWR");
  last_swr = 0;
  last_swrx = -1;
}

void Display_OLED128x64::DrawSWRItemRaw(int fval, int rval)
{
  oled64.setFont(X11fixed7x14mod);
  oled64.setCursor(0,2);
  oled64.print("FWD ");
  oled64.print(fval);
  oled64.clearToEOL();
  oled64.setCursor(0,4);
  oled64.print("RET ");
  oled64.print(rval);
  oled64.clearToEOL();
}

void Display_OLED128x64::DrawSWRMeasure(int fval, int rval, int pwr)
{
  float swr = TRX::CalculateSWR(fval, rval);
  // float average
  if (swr > last_swr) last_swr = swr;
  else last_swr = 0.9*last_swr+0.1*swr;
  // interpolation to scale
  int x;
  if (last_swr <= 1.5) {
    x = 32*(last_swr-1)/0.5;
  } else if (last_swr <= 2.0) {
    x = 32+32*(last_swr-1.5)/0.5;
  } else {
    x = 64+32*(last_swr-2.0)/1.0;
  }
  if (x > 127) x = 127;

  static long last_draw = 0;
  if (millis()-last_draw > 50) {
    if (x != last_swrx) {
      if (last_swrx >= 0) drawLine(64,80,last_swrx,16,40,0);
      last_swrx = x;
      drawLine(64,80,last_swrx,16,40,1);
    }
    last_draw = millis();
    oled64.setFont(X11fixed7x14mod);
    oled64.setCursor(30,6);
    oled64.print((int)last_swr);
    oled64.print(".");
    oled64.print((int)(last_swr*10) % 10);

    if (pwr == 0) {
      pwr = TRX::CalculatePower(fval);
    }

    if (pwr > 0) {
      oled64.setCursor(95,6);
      oled64.print(pwr / 10);
      oled64.print(".");
      oled64.print(pwr % 10);
    }
    oled64.clearToEOL();
  }
}

#endif