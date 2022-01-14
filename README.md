<h2>Nano VFO 3 - simple and powerfull digital VFO</h2>

Last event:
17.09.2021<br>
- RTC clock autodetect<br>
- band select menu<br>
- frequency granulation<br>
- selectable function for MEMO button<br>
- documentation<br>
- support AS5600 encoder<br>
- minor bugfix<br>
- small "square" version of PCB

CPU: Atmega328P<br>
PLL: Si5351 and/or Si570<br>
Display: OLED 1.3" 128/132x64, OLED 0.96" 128x64<br>
Encoder: mechanic rotary encoder, AS5600 magnetic
Keypad: 4+1 buttons
Support different TRX architecture:<br>
 1. Single  IF superheterodyne.
 2. Direct conversion with 2x or 4x output.
 3. Direct conversion with quadrature output.

Builtin CW key: auto/iambic mode, 2 phrase memory, morse decoder<br>
VFOA/B, SPLIT, calibrated S-meter, CAT protocol<br>
Control attenuator, LNA, 5 band BPF without decoder and 16 band with external decoder<br>

Project homepage http://www.ur5ffr.com/viewtopic.php?t=277<br>
PCB available here: https://oshwlab.com/ban.relayer/nano-vfo-3-1<br>
PCB for "Square" version: https://oshwlab.com/ban.relayer/nano-vfo-3-1_copy<br>

Required libraries:<br>
 1. UR5FFR_Si5351 https://github.com/andrey-belokon/UR5FFR_Si5351
 2. SSD1306Ascii - install from Arduino IDE

<img src="doc/img/nanovfo3_square.jpg"></img>

<img src="doc/img/nanovfo3_front.jpg"></img>

<img src="doc/img/nanovfo3_cw_send.jpg"></img>

<img src="doc/Schematic_Nano-VFO-3.png"></img>

Copyright (c) 2016-2021, Andrii Bilokon, UR5FFR<br>
License GNU GPL, see license.txt for more information