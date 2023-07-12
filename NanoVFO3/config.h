#ifndef CONFIG_H
#define CONFIG_H

// hardware configurations
#include "config_hw.h"

// frequency settings
#include "config_sw.h"

// enabled internal CW key
#define ENABLE_INTERNAL_CWKEY

// enable SPLIT on different bands. disabled by default
//#define ENABLE_SPLIT_MULTIBAND

#define LSB 0
#define USB 1

extern const struct _Bands {
  uint8_t   mc;
  long  start, startSSB, end;
  uint8_t sideband;
} Bands[];

// count of bands. must be equal to band count in DEFINED_BANDS definition
#define BAND_COUNT  5
#define DEFINED_BANDS \
  {80,   3500000L,  3600000L,  3800000L, LSB}, \
  {40,   7000000L,  7045000L,  7200000L, LSB}, \
  {20,  14000000L, 14100000L, 14350000L, USB}, \
  {15,  21000000L, 21150000L, 21450000L, USB}, \
  {10,  28000000L, 28200000L, 29700000L, USB}

/* extended band list
#define BAND_COUNT  18
#define DEFINED_BANDS \
  {160,  1810000L,  1840000L,  2000000L, LSB}, \
  {80,   3500000L,  3600000L,  3800000L, LSB}, \
  {40,   7000000L,  7045000L,  7200000L, LSB}, \
  {30,  10100000L, 10150000L, 10150000L, USB}, \
  {20,  14000000L, 14100000L, 14350000L, USB}, \
  {17,  18068000L, 18110000L, 18168000L, USB}, \
  {15,  21000000L, 21150000L, 21450000L, USB}, \
  {12,  24890000L, 24930000L, 25140000L, USB}, \
  {10,  28000000L, 28200000L, 29700000L, USB}, \
  {100,  2000000L,  2000000L,  3500000L, USB}, \
  {60,   3800000L,  3800000L,  5000000L, USB}, \
  {49,   5000000L,  5000000L,  7000000L, USB}, \
  {41,   7200000L,  7200000L, 10100000L, USB}, \
  {25,  10150000L, 10150000L, 14000000L, USB}, \
  {19,  14350000L, 14350000L, 18068000L, USB}, \
  {15,  18168000L, 18168000L, 21000000L, USB}, \
  {13,  21450000L, 21450000L, 24890000L, USB}, \
  {11,  25140000L, 25140000L, 28000000L, USB}
*/

struct _Settings {
  int def_value;
  int min_value;
  int max_value;
  int step;
  char title[16];
};

#define SETTINGS_COUNT  39

#define SETTINGS_DATA \
  {1,   0,   1,   1, "ANALOG GAUGE"},       /* show analog gauge s-meter */\
  {2,   1,   2,   1, "LED HIGH"},           /* only for SuperLED board 0-off, 1-low, 2-full */\
  {1,   0,   2,   1, "LED LOW"},            /* only for SuperLED board 0-off, 1-low, 2-full */\
  {8,   0,   60,  1, "PWR DWN DELAY"},      /* after how much time to switch to the power saving mode, sec. 0 - always on */\
  {7,   1,   15,  1, "BRIGHT HIGH"},        /* LCD brightness - 0..15 in normal mode and powerdown (0 - off) */\
  {1,   0,   15,  1, "BRIGHT LOW"},         \
  \
  {0, 0, 0, 0, "VCC SETUP"},                /* ID_VCC */\
  {0, 0, 1, 1, "TEMP"},                     /* enable temp sensor LM35 */\
  {60,  10,  100, 1, "TEMP WARN"},          /* blink temp on display */\
  {85,  10,  120, 1, "TEMP STOP"},          /* disable TX */\
  {0, 0, 0, 0, ""},                         /* hidden. store ID_VCC_VAL */\
  \
  {1,  0,   1, 1, "ENABLE"},                /* allows the built-in key. if 0 then the foreign key connected to the dit or dah input is used. active level is low. */ \
  {20,  5,  40, 1, "SPEED"},                /* current key speed in WPM */ \
  {700, 300, 2000, 10, "TONE"},             /* self-monitoring frequency and frequency offset on CLK2 for CW signal shaping */ \
  {1, 0, 1, 1, "SEND DECODER"},             /* Decode morse on send */ \
  {800, 100, 2000, 10, "BREAK IN DELAY"},   /* break-in hang time (mS) in CW VOX mode. turn off TX if keyer not pressed more than BREAK_IN_DELAY time */ \
  {0,  0,   1, 1, "IAMBIC"},                /* switch between iambic and auto mode of cw keyer */ \
  {30, 20,  40, 1, "DASH-DOT RATIO"},       /* length of dash (in dots) *10 */ \
  {30, 20,  60, 1, "LETTER SPACE"},         /* length of space between letters (in dots) *10 */ \
  {70, 30, 120, 1, "WORD SPACE"},           /* length of space between words (in dots) *10 */ \
  \
  {0, -10000, 10000, 10, "LSB SHIFT"},      /* additional shift of the second local oscillator relative to the constants BFO_LSB / BFO_USB */ \
  {0, -10000, 10000, 10, "USB SHIFT"}, \
  {0, -20000, 20000, 5, "SI5351 XTAL"}, \
  \
  {0, 0, 0, 0, "CONFIRM RESET"}, \
  {0, 0, 0, 0, "CANCEL RESET"}, \
  \
  {0, 0, 0, 0, "SWR 1.5"}, \
  {0, 0, 0, 0, "SWR 2.0"}, \
  {0, 0, 0, 0, "SWR 3.0"}, \
  {0, 0, 0, 0, "POWER"}, \
  {100, 1, 500, 1, "TX MAX POWER"}, \
  {0, 0, 0, 0, ""},                        /* ID_POWER_VAL storage */\
  {0, 0, 0, 0, "S1"}, \
  {0, 0, 0, 0, "S3"}, \
  {0, 0, 0, 0, "S5"}, \
  {0, 0, 0, 0, "S7"}, \
  {0, 0, 0, 0, "S9"}, \
  {0, 0, 0, 0, "+20"}, \
  {0, 0, 0, 0, "+40"}, \
  {0, 0, 0, 0, "+60"}

// increase for reset stored to EEPROM settings values to default
#define SETTINGS_VERSION    0x12

// id for fast settings access
enum {
  ID_DISPLAY_GAUGE=0,
  ID_DISPLAY_LED_HIGH,
  ID_DISPLAY_LED_LOW,
  ID_POWER_DOWN_DELAY,
  ID_DISPLAY_BRIGHT_HIGH,
  ID_DISPLAY_BRIGHT_LOW,
  ID_VCC,
  ID_TEMP_ENABLED,
  ID_TEMP_WARN,
  ID_TEMP_STOP,
  ID_VCC_VAL,
  ID_KEY_ENABLE,
  ID_KEY_SPEED,
  ID_KEY_TONE_HZ,
  ID_CW_DECODER,
  ID_CW_BREAK_IN_DELAY,
  ID_KEY_IAMBIC,
  ID_KEY_DASH_LEN,
  ID_KEY_LETTER_SPACE,
  ID_KEY_WORD_SPACE,
  ID_LSB_SHIFT,
  ID_USB_SHIFT,
  ID_SI5351_XTAL,
  ID_FULL_RESET_CONFIRM,
  ID_FULL_RESET_CANCEL,
  ID_SWR_15,
  ID_SWR_20,
  ID_SWR_30,
  ID_POWER,
  ID_TX_MAX_POWER,
  ID_POWER_VAL,
  ID_SMETER,
  ID_CLOCK = 200,
  ID_SPLIT,
  ID_QRP,
  ID_TUNE,
  ID_MEMO
};

// hardware config
#include "config_hw.h"

// frequency config
#include "config_sw.h"

#endif
