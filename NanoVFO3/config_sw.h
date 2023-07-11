//  default config for simple superhet with SSB XF 9000300-9003000Hz
//  Single IF=9MHz, VFO/BFO are not switched

#ifndef CONFIG_SW_H
#define CONFIG_SW_H

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//                  you need to uncomment the required mode (only one!)
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// direct conversion mode
//#define MODE_DC

// direct conversion mode with quadrature VFO
//#define MODE_DC_QUADRATURE

// superheterodyne with single/double frequency conversions
// VFO is always higher than the received frequency
#define MODE_SUPER

// 2nd local oscillator frequencies for MODE_SUPER
#define BFO_LSB   9003300L
#define BFO_USB   9000000L

// 3rd local oscillator frequency for MODE_SUPER with double frequency conversions
//#define BFO2        500000L

// Type of 2nd filter used with double frequency conversions- LSB or USB. Uncomment only one macro!
//#define BFO2_LSB
//#define BFO2_USB

// For the superheterodyne MODE_SUPER, it is necessary to determine by constants
// what will be at the output of the synthesizer in different operating modes
// You can use numbers, arithmetic operations and the following macros
// VFO/BFO/BFO2 - first, second and third local oscillators
// CWTX - CW signal at transmit frequency
// CWIF - CW signal at the frequency of the first IF (corresponds to the frequency of the received CW tone)
// BFO2CW is the frequency of the third local oscillator for CW mode when using a CW filter with a BFO2 center frequency
// you can use numbers, arithmetic operations and brackets
// set multipliers in normal notation. e.g. 2*VFO, 4*VFO, 2*BFO, 2*CWTX etc.
// use 0 for disabled output

#define CLK0_RX_SSB     VFO
#define CLK1_RX_SSB     BFO
#define CLK2_RX_SSB     0
#define CLK3_RX_SSB     0

#define CLK0_TX_SSB     VFO
#define CLK1_TX_SSB     BFO
#define CLK2_TX_SSB     0
#define CLK3_TX_SSB     0

#define CLK0_RX_CW      VFO
#define CLK1_RX_CW      BFO
#define CLK2_RX_CW      0
#define CLK3_RX_CW      0

#define CLK0_TX_CW      0
#define CLK1_TX_CW      0
#define CLK2_TX_CW      CWTX
#define CLK3_TX_CW      0

#endif
