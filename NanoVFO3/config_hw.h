#ifndef CONFIG_HW_H
#define CONFIG_HW_H

// версия "железа" 3.1 с 74РС595
#define HARDWARE_3_1

// раскоментировать используемый дисплей (только один!). 
#define DISPLAY_OLED_SH1106_128x64     // OLED 1.3" 128x64 (132x64)
//#define DISPLAY_OLED128x64     // OLED 0.96" 128x64

// keypad pool interval in ms
#define POOL_INTERVAL       50
// продолжительность для длинного нажатия
#define LONG_PRESS_DELAY    1000

// раскоментировать установленные чипы
#define VFO_SI5351
//#define VFO_SI570

// выбрать в меню калибровку и прописать измеренные частоты на выходах синтезаторов
#define SI5351_CALIBRATION       26000000
#define SI570_CALIBRATION        56319832

// уровень сигнала на выходе Si5351. 0=2mA, 1=4mA, 2=6mA, 3=8mA
#define SI5351_CLK0_DRIVE   3
#define SI5351_CLK1_DRIVE   3
#define SI5351_CLK2_DRIVE   3

// раскоментировать при использовании AS5600
//#define ENCODER_AS5600

// количество импульсов на оборот механического энкодера
#define ENCODER_PULSE_PER_TURN    20
// умножение количества импульсов за счет обработки дополнительных состояний. раскоментарьте требуемый коэффициент
// учетверение ENCODER_MULT_4 может работать нестабильно на некоторых экземлярах валкодеров
#define ENCODER_MULT_2
//#define ENCODER_MULT_4
// изменение частоты в Гц на один оборот в обычном режиме
#define ENCODER_FREQ_LO_STEP      3000
// изменение частоты в Гц на один оборот в ускоренном режиме
#define ENCODER_FREQ_HI_STEP      15000
// порог переключения в ускоренный режим. если частота изменится более
// чем на ENCODER_FREQ_HI_LO_TRASH Гц за секунду то переходим в ускоренный режим
#define ENCODER_FREQ_HI_LO_TRASH  2000

// установлен realtime clock DS3231
#define RTC_ENABLE

// скорость обмена порта для CAT (протокол кенвуд)
#define CAT_BAUND_RATE    9600

// Pin active levels (HIGH / LOW)

#define OUT_CW_ACTIVE_LEVEL  HIGH
#define OUT_KEY_ACTIVE_LEVEL  HIGH
#define OUT_TX_ACTIVE_LEVEL  HIGH
#define OUT_ATT_ACTIVE_LEVEL  HIGH
#define OUT_PRE_ACTIVE_LEVEL  HIGH
// по умолчанию для  управления ДПФ активный уровень высокий. раскоментарьте следующую строку чтобы активным уровнем был низкий
//#define BAND_ACTIVE_LEVEL_LOW

// Pin mapping and active levels

#ifdef HARDWARE_3_1

#define PIN_OUT_TX          11
#define PIN_OUT_USR         12
#define PIN_OUT_KEY         13

#define PIN_OUT_TONE        5

#define PIN_IN_PTT          4
#define PIN_IN_DIT          6
#define PIN_IN_DAH          7

#define PIN_CWDEC           A0
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
#define PIN_OUT_BAND2       17
#define PIN_OUT_BAND3       17
#define PIN_OUT_BAND4       13

#endif

#endif
