#ifndef FREQ_GRANULATION
#define FREQ_GRANULATION    10
#endif

#ifdef VFO_SI5351_2

void vfo_set_freq(long f1, long f2, long f3, long f4)
{
#ifdef VFO_SI570  
  vfo570.set_freq(f1);
  SELECT_SI5351(0);
  vfo5351.set_freq(f2,f4);
  SELECT_SI5351(1);
  vfo5351_2.set_freq(f3);
#else
  SELECT_SI5351(0);
  vfo5351.set_freq(f1,f4);
  SELECT_SI5351(1);
  vfo5351_2.set_freq(f2,f3);
#endif
}

#else

void vfo_set_freq(long f1, long f2, long f3, long f4)
{
#ifdef VFO_SI570  
  #ifdef VFO_SI5351
    vfo570.set_freq(f1);
    vfo5351.set_freq(f2,f3);
  #else
    // single Si570
    vfo570.set_freq(f1);
  #endif
#else
  #ifdef VFO_SI5351
    vfo5351.set_freq(f1,f2,f3);
  #endif
#endif
}

#endif

void vfo_set_freq(long f1, long f2)
{
#ifdef VFO_SI570  
  #ifdef VFO_SI5351
    vfo570.set_freq(f1);
    SELECT_SI5351(0);
    vfo5351.set_freq(f2);
  #else
    // single Si570
    vfo570.set_freq(f1);
  #endif
#else
  #ifdef VFO_SI5351
    SELECT_SI5351(0);
    #ifdef VFO_SI5351_2
      vfo5351.set_freq(f1);
      SELECT_SI5351(1);
      vfo5351.set_freq(f2);
    #else
      vfo5351.set_freq(f1,f2);
    #endif
  #endif
#endif
}

void UpdateFreq()
{
  long Freq = ((trx.Freq+FREQ_GRANULATION/2)/FREQ_GRANULATION)*FREQ_GRANULATION;

  trx.CurrentFreq = Freq;

#ifdef MODE_DC
  uint8_t cwtx = trx.TX && trx.CWTX;
  vfo_set_freq(
    cwtx? 0: Freq,
    cwtx? Freq+(trx.sideband == LSB ? -Settings[ID_KEY_TONE_HZ]: Settings[ID_KEY_TONE_HZ]): 0,
    0, 0
  );
#endif

#ifdef MODE_DC_QUADRATURE
  uint8_t cwtx = trx.TX && trx.CWTX;
  vfo5351.set_freq_quadrature(
    cwtx? 0: Freq,
    cwtx? Freq+(trx.sideband == LSB ? -Settings[ID_KEY_TONE_HZ]: Settings[ID_KEY_TONE_HZ]): 0
  );
#endif

#ifdef MODE_SUPER
  // инверсия боковой - гетеродин сверху
  long VFO,BFO,CWTX,CWIF;
  VFO = Freq + (trx.sideband == LSB ? ((BFO_USB)+Settings[ID_USB_SHIFT]) : ((BFO_LSB)+Settings[ID_LSB_SHIFT]));
  BFO = (trx.sideband == LSB ? ((BFO_USB)+Settings[ID_USB_SHIFT]) : ((BFO_LSB)+Settings[ID_LSB_SHIFT]));
  CWTX = Freq + (trx.sideband == LSB ? -Settings[ID_KEY_TONE_HZ] : Settings[ID_KEY_TONE_HZ]);
  CWIF = BFO + (trx.sideband == LSB ? Settings[ID_KEY_TONE_HZ] : -Settings[ID_KEY_TONE_HZ]);
#ifdef BFO2
  long BFO2CW = BFO2 + (trx.sideband == LSB ? Settings[ID_KEY_TONE_HZ] : -Settings[ID_KEY_TONE_HZ]);
#endif
#ifdef BFO2_LSB
  BFO += (trx.sideband == LSB ? (BFO2) : -(BFO2));
#else
#ifdef BFO2_USB
  BFO += (trx.sideband == LSB ? -(BFO2) : (BFO2));
#endif
#endif
  if (trx.TX) {
    if (trx.CWTX) vfo_set_freq(CLK0_TX_CW,CLK1_TX_CW,CLK2_TX_CW,CLK3_TX_CW);
    else vfo_set_freq(CLK0_TX_SSB,CLK1_TX_SSB,CLK2_TX_SSB,CLK3_TX_SSB);
  } else {
    if (trx.CW) vfo_set_freq(CLK0_RX_CW,CLK1_RX_CW,CLK2_RX_CW,CLK3_RX_CW);
    else vfo_set_freq(CLK0_RX_SSB,CLK1_RX_SSB,CLK2_RX_SSB,CLK3_RX_SSB);
  }
#endif

}
