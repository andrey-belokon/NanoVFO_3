#ifndef TRX_H
#define TRX_H

#include <Arduino.h>
#include "config.h"

class TRX {
  public:
    long BandData[BAND_COUNT]; 
    int BandIndex;
    long Freq;
    long FreqMemo;
    uint8_t Lock;
    uint8_t AttPre;    // 0-nothing; 1-ATT; 2-Preamp
    uint8_t sideband;
    uint8_t CW;
    uint8_t TX;
    uint8_t CWTX;
    uint8_t CATTX;
    uint8_t split;
    uint8_t cw_speed;
    uint16_t dit_time;
    uint16_t dah_time;
    char cw_buf[42];
    uint8_t cw_buf_idx;
    uint8_t SMeter; // 0..15 
    #ifdef HARDWARE_3_1
      float VCC;
    #endif
    // in tx mode
    float pwr; // 0..1
    float swr;

    TRX();
    void SwitchToBand(int band);
    void NextBand();
    void SelectBand(int band);
    void ChangeFreq(long freq_delta);
    void SaveFreqToMemo();
    void SwitchFreqToMemo();
    void SwitchAttPre();
    void PutCWChar(char ch);
    void CWNewLine();
    void CWClear();
    void SetFreqBand(long freq);
    void SetFreqMemoBand(long freq);
    uint8_t inCW();
    uint8_t setCWSpeed(uint8_t speed, int dash_ratio);
    const struct _Bands& GetBandInfo(uint8_t idx);

    static float CalculateSWR(int fval, int rval);
    static int CalculatePower(int fval);

    uint16_t StateHash();
    void StateSave();
    void StateLoad();
};

#endif
