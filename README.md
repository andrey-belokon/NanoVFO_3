<h2>Nano VFO 3 - simple and powerfull digital VFO</h2>

Last event:
Version 3.4 from 12.07.2023<br>
- editable memory banks for CW messages<br>
- support for double superheterodyne architecture<br>
- temperature sensor LM35<br>
- measurement of SWR and power<br>
- TUNE mode<br>
- new SuperLED boards<br>
- updated documentation<br>

CPU: Atmega328P<br>
VFO: Si5351 and/or Si570<br>
Display: OLED 1.3" 128/132x64, OLED 0.96" 128x64<br>
Encoder: mechanic rotary encoder, AS5600 magnetic:<br>
Keypad: 4+1 buttons:<br>
Support different TRX architecture:<br>
- Single/double IF superheterodyne.
- Direct conversion with 2x or 4x output.
- Direct conversion with quadrature output.

Builtin CW key: auto/iambic mode, 3 phrase memory, morse decoder<br>
Major features: VFO A/B, SPLIT, calibrated S-meter, CAT protocol<br>
Control attenuator, LNA, 5 band BPF without decoder and 16 band with external decoder<br>

Project homepage http://www.ur5ffr.com/viewtopic.php?t=277<br>
Links to PCB:<br>
Version 3.1 https://oshwlab.com/ban.relayer/nano-vfo-3-1<br>
Version 3.2 https://oshwlab.com/ban.relayer/nano-vfo-3-2<br>
"Square" version https://oshwlab.com/ban.relayer/nano-vfo-3-1_copy<br>
SuperLED version will be available later after the victory in the war. Glory to Ukraine!<br>

Required libraries:<br>
 1. UR5FFR_Si5351 https://github.com/andrey-belokon/UR5FFR_Si5351
 2. SSD1306Ascii - install from Arduino IDE

<img src="doc/img/nanovfo3_square.jpg"></img>

<img src="doc/img/nanovfo3_front.jpg"></img>

<img src="doc/img/nanovfo3_cw_send.jpg"></img>

<img src="TRX DoubleFox.jpg"></img>

<img src="doc/nanovfo_superled.jpg"></img>

<img src="doc/Schematic_Nano VFO 3.2.png"></img>

Copyright (c) 2016-2023, Andrii Bilokon, UR5FFR<br>
http://www.ur5ffr.com<br>
License GNU GPL, see license.txt for more information