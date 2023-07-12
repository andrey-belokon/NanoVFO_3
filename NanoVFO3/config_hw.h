#ifndef CONFIG_HW_H
#define CONFIG_HW_H

// hardware version 3.1+ with 74HС595 extender
#define HARDWARE_3_1

// uncomment for SuperLED board
//#define HARDWARE_SUPERLED

#ifdef HARDWARE_SUPERLED
  #define HARDWARE_3_1
#endif

// select display type (only one!)
#define DISPLAY_OLED_SH1106_128x64     // OLED 1.3"  128x64 (132x64)
//#define DISPLAY_OLED128x64           // OLED 0.96" 128x64

// keypad pool interval in ms
#define POOL_INTERVAL       50
// delay in ms for detect long press of key
#define LONG_PRESS_DELAY    1000

// select used IC
#ifdef HARDWARE_SUPERLED
  #define VFO_SI5351
  #define VFO_SI5351_2
#else
  #define VFO_SI5351
#endif
//#define VFO_SI570

// select in synt menu calibration item and write here measured frequency
#define SI5351_CALIBRATION       26000000
#define SI570_CALIBRATION        56319832

// for "bad" si5351 with internal VCO limited to 800MHz max
#define SI5351_VCOMAXFREQ       800000000

// output level of Si5351. 0=2mA, 1=4mA, 2=6mA, 3=8mA
#define SI5351_CLK0_DRIVE   3
#define SI5351_CLK1_DRIVE   3
#define SI5351_CLK2_DRIVE   3
#ifdef VFO_SI5351_2
  #define SI5351_2_CLK0_DRIVE   3
  #define SI5351_2_CLK1_DRIVE   3
  #define SI5351_2_CLK2_DRIVE   3
#endif

// multiplicity of frequency setting in Hz. comment if not needed
#define FREQ_GRANULATION        50

// uncomment for usage AS5600 as encoder
//#define ENCODER_AS5600

// number of pulses per turn of a mechanical encoder
#define ENCODER_PULSE_PER_TURN    20
// multiplying the number of pulses by processing additional states. 
// uncomment required multiplier quadruple ENCODER_MULT_4 may be unstable on some valcoder instances
#define ENCODER_MULT_2
//#define ENCODER_MULT_4
// frequency change in Hz per turn in normal mode
#define ENCODER_FREQ_LO_STEP      2000
// frequency change in Hz per turn in fast mode
#define ENCODER_FREQ_HI_STEP      25000
// threshold for switching to fast mode. if the frequency changes by more than
// ENCODER_FREQ_HI_LO_TRASH Hz per second, then we switch to accelerated mode
#define ENCODER_FREQ_HI_LO_TRASH  1000

// select type of CAT protocol (only one!)
//#define CAT_PROTOCOL_KENWOOD_TS480
#define CAT_PROTOCOL_YAESU_FT817

// com-port speed
#define CAT_BAUND_RATE    9600

// Pin active levels (HIGH / LOW)

#define OUT_CW_ACTIVE_LEVEL  HIGH
#define OUT_KEY_ACTIVE_LEVEL  HIGH
#define OUT_TX_ACTIVE_LEVEL  HIGH
#define OUT_ATT_ACTIVE_LEVEL  HIGH
#define OUT_PRE_ACTIVE_LEVEL  HIGH
// default for BPF control is active high. uncomment the following line so the active level is low
//#define BPF_ACTIVE_LEVEL_LOW

#ifdef HARDWARE_SUPERLED
  #define OUT_QRP_ACTIVE_LEVEL    HIGH
  #define OUT_TUNE_ACTIVE_LEVEL   HIGH
  //#define LPF_ACTIVE_LEVEL_LOW
#endif 

// whether or not to output a tone when tuning (TUNE). if not, the tone is only used for self-control
//#define ENABLE_TONE_ON_TUNE

// active or not CW pin when transmitting CW and TUNE
//#define DISABLE_CW_ON_CWTX

// comment out the following line if the SWR sensor is not connected
#define ENABLE_SWR_SENSOR

// Pin mapping

#ifdef HARDWARE_3_1

#ifdef HARDWARE_SUPERLED
  #define PIN_OUT_SISEL       12
#else
  #define PIN_OUT_TX          11
  #define PIN_OUT_USR         12
#endif

#define PIN_OUT_KEY         13

#define PIN_OUT_TONE        5

#define PIN_IN_PTT          4
#define PIN_IN_DIT          6
#define PIN_IN_DAH          7

#define PIN_TEMP            A0
#define PIN_POWER           A1
#define PIN_SWR_F           A2
#define PIN_SWR_R           A3

#define PIN_ANALOG_KEYPAD   A6
#define PIN_SMETER          A7

#define PIN_SR_DATA         10
#define PIN_SR_SHIFT        9
#define PIN_SR_LATCH        8

#else

#define PIN_OUT_CW          10
#define PIN_OUT_KEY         11
#define PIN_OUT_TX          12

#define PIN_OUT_ATT         7
#define PIN_OUT_PRE         8

#define PIN_OUT_TONE        9

#define PIN_IN_PTT          4
#define PIN_IN_DIT          5
#define PIN_IN_DAH          6

#define PIN_ANALOG_KEYPAD   A6
#define PIN_SMETER          A7

#define PIN_OUT_BAND0       14
#define PIN_OUT_BAND1       15
#define PIN_OUT_BAND2       16
#define PIN_OUT_BAND3       17
#define PIN_OUT_BAND4       13

#endif

#endif
