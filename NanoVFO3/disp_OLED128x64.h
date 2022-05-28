#ifndef DISP_OLED12864_H
#define DISP_OLED12864_H

#include <Arduino.h>
#include "TRX.h"

class Display_OLED128x64 {
  public:
	  void setup();
    // 1..15
    void setBright(uint8_t brightness);
	  void Draw(TRX& trx);
    void DrawItemEdit(PGM_P text, int value);
    void DrawItemValue(int value);
    void DrawItems(PGM_P* text, uint8_t selected);
    void DrawSelected(uint8_t selected);
    void DrawFreqItems(TRX& trx, uint8_t idx, uint8_t selected);
    void DrawSMeterItems(PGM_P* text, const int* vals, uint8_t selected);
    void DrawSWRMeasureInit();
    void DrawSWRMeasure(int fval, int rval, int pwr = 0);
    void DrawSWRItemRaw(int fval, int rval);
    void DrawVCC(int rawdata, int vcc);
    void clear();
};

#endif
