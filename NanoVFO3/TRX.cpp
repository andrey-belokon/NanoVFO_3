#include "TRX.h"
#include "config.h"

const struct _Bands Bands[BAND_COUNT] = {
  DEFINED_BANDS
};

TRX::TRX() {
  for (byte i=0; i < BAND_COUNT; i++) {
	  if (Bands[i].startSSB != 0)
	    BandData[i] = Bands[i].startSSB;
	  else
	    BandData[i] = Bands[i].start;
  }
  TX=CWTX=Lock=split= 0;
  CWClear();
  SwitchToBand(0);
  #ifdef HARDWARE_3_1
  VCC = 0;
  #endif
}

void TRX::SwitchToBand(int band) {
  BandIndex = band;
  Freq = BandData[BandIndex];
  FreqMemo = 0;
  Lock=split=0;
  sideband = Bands[BandIndex].sideband;
  CW = inCW();
}

void TRX::SaveFreqToMemo()
{
  FreqMemo = Freq;
}

void TRX::SwitchFreqToMemo()
{
  if (!TX) {
    // проверяем выход за пределы диапазона
    if (FreqMemo < Bands[BandIndex].start)
      FreqMemo = Freq;
    else if (FreqMemo > Bands[BandIndex].end)
      FreqMemo = Freq;
    uint8_t old_cw = inCW();
    long tmp = Freq;
    Freq = FreqMemo;
    FreqMemo = tmp;
    uint8_t new_cw = inCW();
    if (old_cw != new_cw) CW = new_cw;
  }
}

void TRX::ChangeFreq(long freq_delta)
{
  if (!TX) {
    uint8_t old_cw = inCW();
    Freq += freq_delta;
    // проверяем выход за пределы диапазона
    if (Freq < Bands[BandIndex].start)
      Freq = Bands[BandIndex].start;
    else if (Freq > Bands[BandIndex].end)
      Freq = Bands[BandIndex].end;
    uint8_t new_cw = inCW();
    if (old_cw != new_cw) {
      CW = new_cw;
      sideband = Bands[BandIndex].sideband;
    }
  }
}

void TRX::SetFreqBand(long freq)
{
  for (byte i=0; i < BAND_COUNT; i++) {
    if (freq >= Bands[i].start && freq <= Bands[i].end) {
      if (i != BandIndex) SwitchToBand(i);
      Freq = freq;
      CW = inCW();
      return;
    }
  }
}

void TRX::NextBand()
{
  if (!TX) {
    BandData[BandIndex] = Freq;
    if (++BandIndex >= BAND_COUNT)
      BandIndex = 0;
    Freq = BandData[BandIndex];
    FreqMemo = 0;
    Lock=split=0;
    sideband = Bands[BandIndex].sideband;
    CW = inCW();
  }
}

void TRX::SwitchAttPre()
{
  AttPre++;
  if (AttPre > 2) AttPre = 0;
}

uint8_t TRX::inCW() {
  return 
    BandIndex >= 0 && Bands[BandIndex].startSSB > 0 &&
    Freq < Bands[BandIndex].startSSB &&
    Freq >= Bands[BandIndex].start;
}

uint8_t TRX::setCWSpeed(uint8_t speed, int dash_ratio)
{
  if (speed != cw_speed) {
    cw_speed = speed;
    dit_time = 1200/cw_speed;
    dah_time = dit_time*dash_ratio/10;
    return 1;
  }
  return 0;
}

void TRX::CWNewLine()
{
  if (cw_buf_idx == 0) return;
  for (byte k=0,i=sizeof(cw_buf)/2; i < sizeof(cw_buf); i++,k++) {
    cw_buf[k] = cw_buf[i];
    cw_buf[i] = ' ';
  }
  cw_buf_idx = sizeof(cw_buf)/2;
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

struct TRXState {
  long BandData[BAND_COUNT]; 
  int BandIndex;
  long Freq;
};

uint16_t hash_data(uint16_t hval, uint8_t* data, int sz)
{
  while (sz--) {
    hval ^= *data++;
    hval = (hval << 11) | (hval >> 5);
  }
  return hval;
}  

uint16_t TRX::StateHash()
{
  uint16_t hash = 0x5AC3;
  hash = hash_data(hash,(uint8_t*)BandData,sizeof(BandData));
  hash = hash_data(hash,(uint8_t*)&BandIndex,sizeof(BandIndex));
  hash = hash_data(hash,(uint8_t*)&Freq,sizeof(Freq));
  return hash;
}

struct TRXState EEMEM eeState;
uint16_t EEMEM eeStateVer;
#define STATE_VER     (0x5A2C ^ (BAND_COUNT))

void TRX::StateSave()
{
  struct TRXState st;
  for (byte i=0; i < BAND_COUNT; i++)
    st.BandData[i] = BandData[i];
  st.BandIndex = BandIndex;
  st.Freq = Freq;
  eeprom_write_block(&st, &eeState, sizeof(st));
  eeprom_write_word(&eeStateVer, STATE_VER);
}

void TRX::StateLoad()
{
  struct TRXState st;
  uint16_t ver;
  ver = eeprom_read_word(&eeStateVer);
  if (ver == STATE_VER) {
    eeprom_read_block(&st, &eeState, sizeof(st));
    for (byte i=0; i < BAND_COUNT; i++)
      BandData[i] = st.BandData[i];
    SwitchToBand(st.BandIndex);
    Freq = st.Freq;
  }
}
