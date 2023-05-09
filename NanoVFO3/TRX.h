#ifndef TRX_H
#define TRX_H

#include <Arduino.h>
#include "config.h"

class TRX {
  public:
    long BandData[BAND_COUNT]; 
    int BandIndex;
    long Freq;
    long CurrentFreq; // rounded
    long FreqMemo;
    uint8_t Lock;
    uint8_t AttPre;    // 0-nothing; 1-ATT; 2-Preamp
    uint8_t sideband;
    uint8_t CW;
    uint8_t TX;
    uint8_t CWTX;
    uint8_t CATTX;
    uint8_t split;
    uint8_t qrp;
    uint8_t tune;
  #ifdef ENABLE_INTERNAL_CWKEY
    uint8_t cw_speed;
    uint16_t dit_time;
    uint16_t dah_time;
    char cw_buf[21*4];
    uint8_t cw_buf_idx;
  #endif
    uint8_t SMeter; // 0..15 
    #ifdef HARDWARE_SUPERLED
      uint8_t SMeterFrac; // 0..99 fraction part for analog gauge s-meter
    #endif
    #ifdef HARDWARE_3_1
      float temp;
      float VCC;
    #endif
    #ifdef HARDWARE_SUPERLED
      byte led_power; // 0=off/1=mid/2=full
    #endif
    // in tx mode
    float pwr; // 0..1
    float swr;
    uint8_t changed;

    TRX();
    void SetTX(uint8_t new_tx);
    void SwitchToBand(int band);
    void NextBand();
    void SelectBand(int band);
    void ChangeFreq(long freq_delta);
    void SaveFreqToMemo();
    void SwitchFreqToMemo();
    void SwitchAttPre();
    void NextMode();
  #ifdef ENABLE_INTERNAL_CWKEY
    void PutCWChar(char ch);
    void CWNewLine();
    void CWClear();
    void setCWSpeed(uint8_t speed, int dash_ratio);
  #endif
    void SetFreqBand(long freq);
    void SetFreqMemoBand(long freq);
    void UpdateMode();
    const struct _Bands& GetBandInfo(uint8_t idx);

    static float CalculateSWR(int fval, int rval);
    static int CalculatePower(int fval);

    void StateSave();
    void StateLoad();
};

#endif
