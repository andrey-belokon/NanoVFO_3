#include "TRX.h"
#include "config.h"

#define FREQ_MASK     0xFFFFFFF
#define ATT_SHIFT     28

const struct _Bands Bands[BAND_COUNT] = {
  DEFINED_BANDS
};

extern int Settings[];

TRX::TRX() 
{
  for (byte i=0; i < BAND_COUNT; i++) {
	  if (Bands[i].startSSB != 0)
	    BandData[i] = Bands[i].startSSB;
	  else
	    BandData[i] = Bands[i].start;
  }
  changed=AttPre=TX=CWTX=CATTX=Lock=split=tune=qrp=0;
  FreqMemo=0;
#ifdef ENABLE_INTERNAL_CWKEY
  CWClear();
#endif
  SwitchToBand(0);
  #ifdef HARDWARE_3_1
    VCC = temp = 0;
  #endif
}

void TRX::SetTX(uint8_t new_tx)
{
  if (new_tx != TX) {
#ifdef ENABLE_SPLIT_MULTIBAND
    if (split && FreqMemo > 0) {
      SwitchFreqToMemo();
    }
#else
    if (split) {
      if (FreqMemo >= Bands[BandIndex].start && FreqMemo <= Bands[BandIndex].end) {
        SwitchFreqToMemo();
      } else {
        split = 0;
      }
    }
#endif
    TX = new_tx;
  }
}

void TRX::SwitchToBand(int band) 
{
  BandIndex = band;
  Freq = BandData[BandIndex] & FREQ_MASK;
  AttPre = (BandData[BandIndex] >> ATT_SHIFT) & 0x7;
  Lock = split = 0;
  UpdateMode();
  changed++;
}

void TRX::SaveFreqToMemo()
{
  FreqMemo = CurrentFreq;
}

void TRX::SwitchFreqToMemo()
{
  if (FreqMemo > 0 && !TX) {
    long old_freq = Freq;
    Freq = FreqMemo;
    FreqMemo = old_freq;
    SetFreqBand(Freq);
    changed++;
  }
}

void TRX::ChangeFreq(long freq_delta)
{
  if (!TX) {
    bool std_mode;
    if (Freq >= Bands[BandIndex].startSSB) {
      std_mode = sideband == Bands[BandIndex].sideband && !CW;
     } else {
      std_mode = sideband == USB && CW;
     }
    Freq += freq_delta;
    // проверяем выход за пределы диапазона
    if (Freq < Bands[BandIndex].start)
      Freq = Bands[BandIndex].start;
    else if (Freq > Bands[BandIndex].end)
      Freq = Bands[BandIndex].end;
    if (std_mode) {
      // only if default mode
      UpdateMode();
    }
    changed++;
  }
}

void TRX::SetFreqBand(long freq)
{
  for (byte i=0; i < BAND_COUNT; i++) {
    if (freq >= Bands[i].start && freq <= Bands[i].end) {
      if (i != BandIndex) SwitchToBand(i);
      Freq = freq;
      UpdateMode();
      changed++;
      return;
    }
  }
}

void TRX::SetFreqMemoBand(long freq)
{
  for (byte i=0; i < BAND_COUNT; i++) {
    if (freq >= Bands[i].start && freq <= Bands[i].end) {
      if (i != BandIndex) SwitchToBand(i);
      FreqMemo = CurrentFreq;
      return;
    }
  }
}

void TRX::NextBand()
{
  if (!TX) {
    BandData[BandIndex] = CurrentFreq | (long(AttPre) << ATT_SHIFT);
    if (++BandIndex >= BAND_COUNT)
      BandIndex = 0;
    Freq = BandData[BandIndex] & FREQ_MASK;
    AttPre = (BandData[BandIndex] >> ATT_SHIFT) & 0x7;
    Lock=0;
    UpdateMode();
    changed++;
  }
}

void TRX::SwitchAttPre()
{
  AttPre++;
  if (AttPre > 2) AttPre = 0;
  changed++;
}

void TRX::UpdateMode() 
{
  if (Freq < Bands[BandIndex].startSSB) {
    CW = 1;
    sideband = USB;
  } else {
    CW = 0;
    sideband = Bands[BandIndex].sideband;
  }
  changed++;
}

void TRX::NextMode()
{
  if (CW) {
    if (sideband == USB) sideband = LSB;
    else CW = 0;
  } else {
    if (sideband == USB) CW = 1;
    else sideband = USB;
  }
  changed++;
}

#ifdef ENABLE_INTERNAL_CWKEY

void TRX::setCWSpeed(uint8_t speed, int dash_ratio)
{
  cw_speed = speed;
  dit_time = 1200/cw_speed;
  dah_time = dit_time*dash_ratio/10;
}

void TRX::CWNewLine()
{
  if (cw_buf_idx == 0) return;
  for (byte k=0,i=21; i < sizeof(cw_buf); i++,k++) {
    cw_buf[k] = cw_buf[i];
    cw_buf[i] = ' ';
  }
  cw_buf_idx = 63;
}

void TRX::CWClear()
{
  cw_buf_idx = 0;
  for (byte i=0; i < sizeof(cw_buf); i++) cw_buf[i] = ' ';
}

void TRX::PutCWChar(char ch)
{
  if (ch == ' ' && (cw_buf_idx == 0 || cw_buf[cw_buf_idx-1] == ' ')) return;
  if (cw_buf_idx == sizeof(cw_buf)) CWNewLine();
  cw_buf[cw_buf_idx] = ch;
  cw_buf_idx++;
}

#endif

const struct _Bands& TRX::GetBandInfo(uint8_t idx)
{
  return Bands[idx];
}

void TRX::SelectBand(int band)
{
  BandData[BandIndex] = CurrentFreq | (long(AttPre) << ATT_SHIFT);
  SwitchToBand(band);
}

float TRX::CalculateSWR(int fval, int rval)
{
  float swr;
  if (fval <= rval) swr=10.0;
  else swr = (fval+rval)*1.0 / (fval-rval);
  if (Settings[ID_SWR_15] != 0 && Settings[ID_SWR_20] != 0 && Settings[ID_SWR_30] != 0) {
    // interpolation
    float t15 = Settings[ID_SWR_15]/100.0;
    float t20 = Settings[ID_SWR_20]/100.0;
    float t30 = Settings[ID_SWR_30]/100.0;
    if (swr <= t15) {
      swr = 1.0+0.5*(swr-1)/(t15-1);
    } else if (swr <= t20) {
      swr = 1.5+0.5*(swr-t15)/(t20-t15);
    } else {
      swr = 2.0+1.0*(swr-t20)/(t30-t20);
    }
  }
  if (swr > 10) swr = 10.0;
  return swr;
}

int TRX::CalculatePower(int fval)
{
  if (Settings[ID_POWER_VAL] > 0 && Settings[ID_POWER] > 0) 
    return (int)(((float)Settings[ID_POWER]*fval*fval)/((float)Settings[ID_POWER_VAL]*Settings[ID_POWER_VAL]));
  else
    return 0;
}

long EEMEM eeBandData[BAND_COUNT]; 
uint16_t EEMEM eeBandIndex;
uint16_t EEMEM eeStateVer;
#define STATE_VER     (0x5A2D ^ (BAND_COUNT))

void TRX::StateSave()
{
  if (changed) {
    BandData[BandIndex] = CurrentFreq | ((long(AttPre) << ATT_SHIFT));
    eeprom_write_block(BandData, &eeBandData, sizeof(BandData));
    eeprom_write_word(&eeBandIndex, BandIndex);
    eeprom_write_word(&eeStateVer, STATE_VER);
    changed = 0;
  }
}

void TRX::StateLoad()
{
  if (eeprom_read_word(&eeStateVer) == STATE_VER) {
    eeprom_read_block(BandData, &eeBandData, sizeof(BandData));
    SwitchToBand(eeprom_read_word(&eeBandIndex));
  }
  changed = 0;
}
