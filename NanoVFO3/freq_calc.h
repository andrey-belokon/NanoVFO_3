#ifndef FREQ_GRANULATION
#define FREQ_GRANULATION    10
#endif

void vfo_set_freq(long f1, long f2, long f3)
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

void vfo_set_freq(long f1, long f2)
{
#ifdef VFO_SI570  
  #ifdef VFO_SI5351
    vfo570.set_freq(f1);
    vfo5351.set_freq(f2);
  #else
    // single Si570
    vfo570.set_freq(f1);
  #endif
#else
  #ifdef VFO_SI5351
    vfo5351.set_freq(f1,f2);
  #endif
#endif
}

void UpdateFreq() 
{
  long Freq;

  if (trx.split && trx.TX && trx.FreqMemo > 0) Freq = trx.FreqMemo;
  else Freq = trx.Freq;
  Freq = ((Freq+FREQ_GRANULATION/2)/FREQ_GRANULATION)*FREQ_GRANULATION;

#ifdef MODE_DC
  uint8_t cwtx = trx.TX && trx.CWTX;
  vfo_set_freq(
    cwtx? 0: Freq,
    cwtx? Freq+(trx.sideband == LSB ? -Settings[ID_KEY_TONE_HZ]: Settings[ID_KEY_TONE_HZ]): 0
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
  CWIF = (trx.sideband == LSB ? ((BFO_USB)+Settings[ID_USB_SHIFT])+Settings[ID_KEY_TONE_HZ] : ((BFO_LSB)+Settings[ID_LSB_SHIFT])-Settings[ID_KEY_TONE_HZ]);
  vfo_set_freq(
    trx.TX ? (trx.CWTX ? (CLK0_TX_CW) : (CLK0_TX_SSB)) : (CLK0_RX),
    trx.TX ? (trx.CWTX ? (CLK1_TX_CW) : (CLK1_TX_SSB)) : (CLK1_RX),
    trx.TX ? (trx.CWTX ? (CLK2_TX_CW) : (CLK2_TX_SSB)) : (CLK2_RX)
  );
#endif
}