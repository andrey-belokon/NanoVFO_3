#ifndef CONFIG_H
#define CONFIG_H

#define LSB 0
#define USB 1

extern const struct _Bands {
  uint8_t   mc;
  long  start, startSSB, end;
  uint8_t sideband;
} Bands[];

// число диапазонов. должно соответствовать количеству объявленному в DEFINED_BANDS
#define BAND_COUNT  5

#define DEFINED_BANDS \
  {80,   3500000L,  3600000L,  3800000L, LSB}, \
  {40,   7000000L,  7045000L,  7200000L, LSB}, \
  {20,  14000000L, 14100000L, 14350000L, USB}, \
  {15,  21000000L, 21150000L, 21450000L, USB}, \
  {10,  28000000L, 28200000L, 29700000L, USB}

/* описание стандартных любительских диапазонов
 *  скопировать требуемые в DEFINED_BANDS
 *  и изменить константу BAND_COUNT
  {160,  1810000L,  1840000L,  2000000L, LSB}, \
  {80,   3500000L,  3600000L,  3800000L, LSB}, \
  {40,   7000000L,  7045000L,  7200000L, LSB}, \
  {30,  10100000L,        0,  10150000L, USB}, \
  {20,  14000000L, 14100000L, 14350000L, USB}, \
  {17,  18068000L, 18110000L, 18168000L, USB}, \
  {15,  21000000L, 21150000L, 21450000L, USB}, \
  {12,  24890000L, 24930000L, 25140000L, USB}, \
  {10,  28000000L, 28200000L, 29700000L, USB}
*/

struct _Settings {
  int def_value;
  int min_value;
  int max_value;
  int step;
  char title[16];
};

#define SETTINGS_COUNT  35

#define SETTINGS_DATA \
  {8,   0,   60,  1, "PWR DWN DELAY"},      /* через сколько времени переходить в режим сохранения питания, сек. 0 - постоянно включен*/ \
  {7,   1,   15,  1, "BRIGHT HIGH"},        /* яркость LCD - 0..15 в обычнойм режиме и powerdown (0 - погашен) */ \
  {1,   0,   15,  1, "BRIGHT LOW"},         \
  \
  {1,  0,   1, 1, "ENABLE"},                /* разрешает встроенный ключ. если 0 то используется внешний ключ подключенный к входу dit или dah. активный уровень низкий. */ \
  {20,  5,  40, 1, "SPEED"},             /* текущая скорость ключа in WPM */ \
  {700, 300, 2000, 10, "TONE"},          /* частота самоконтроля и сдвиг частоты на CLK2 для формирования CW-сигнала */ \
  {0,  0,   1, 1, "IAMBIC"},                /* switch between iambic and auto mode of cw keyer */ \
  {30, 20,  40, 1, "DASH-DOT RATIO"},       /* length of dash (in dots) *10 */ \
  {30, 20,  60, 1, "LETTER SPACE"},         /* length of space between letters (in dots) *10 */ \
  {70, 30, 120, 1, "WORD SPACE"},           /* length of space between words (in dots) *10 */ \
  \
  {1, 0, 1, 1, "CW VOX"},                   /* auto turn into TX on keyer press (on if in CW mode). if undef you need external PTT */ \
  {800, 100, 2000, 10, "BREAK IN DELAY"},   /* break-in hang time (mS) in CW VOX mode. turn off TX if keyer not pressed more than BREAK_IN_DELAY time */ \
  {1, 0, 1, 1, "SEND DECODER"},             /* Decode morse on send */ \
  {2, 0, 5, 1, "DEC HIDE DELAY"},          /* Hide decoder after send delay in second */ \
  \
  {0, -10000, 10000, 10, "LSB SHIFT"},      /* доп.сдвиг второго гетеродина относительно констант BFO_LSB/BFO_USB */ \
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
  {0, 0, 0, 0, ""},                        /* ID_POWER_VAL storage */ \
  {0, 0, 0, 0, ""},                        /* ID_VCC */ \
  {0, 0, 0, 0, ""},                         /* ID_VCC_VAL */ \
  {0, 0, 0, 0, "S1"}, \
  {0, 0, 0, 0, "S3"}, \
  {0, 0, 0, 0, "S5"}, \
  {0, 0, 0, 0, "S7"}, \
  {0, 0, 0, 0, "S9"}, \
  {0, 0, 0, 0, "+20"}, \
  {0, 0, 0, 0, "+40"}, \
  {0, 0, 0, 0, "+60"}

// increase for reset stored to EEPROM settings values to default
#define SETTINGS_VERSION    0x06

// id for fast settings access
enum {
  ID_POWER_DOWN_DELAY = 0,
  ID_DISPLAY_BRIGHT_HIGH,
  ID_DISPLAY_BRIGHT_LOW,
  ID_KEY_ENABLE,
  ID_KEY_SPEED,
  ID_KEY_TONE_HZ,
  ID_KEY_IAMBIC,
  ID_KEY_DASH_LEN,
  ID_KEY_LETTER_SPACE,
  ID_KEY_WORD_SPACE,
  ID_CW_VOX,
  ID_CW_BREAK_IN_DELAY,
  ID_CW_DECODER,
  ID_CW_DECODER_HIDE_DELAY,
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
  ID_VCC,
  ID_VCC_VAL,
  ID_SMETER,
  ID_CLOCK = 200,
  ID_SPLIT,
  ID_TUNE
};

// конфиг "железа"
#include "config_hw.h"

// настройки генерируемых частот 
#include "config_sw.h"

#endif
