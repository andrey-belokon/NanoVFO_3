struct morse_letter {
  char ch; 
  byte len;
  byte code; // 0 - dit, 1 - dah
};

const morse_letter MorseCode[] PROGMEM = {
  {'A', 2, B10},        // .-
  {'B', 4, B0001},      // -...
  {'C', 4, B0101},      // -.-.
  {'D', 3, B001},       // -..
  {'E', 1, B0},         // .
  {'F', 4, B0100},      // ..-.
  {'G', 3, B011},       // --.
  {'H', 4, B0000},      // ....
  {'I', 2, B00},        // ..
  {'J', 4, B1110},      // .---
  {'K', 3, B101},       // -.-
  {'L', 4, B0010},      // .-..
  {'M', 2, B11},        // --
  {'N', 2, B01},        // -.
  {'O', 3, B111},       // ---
  {'P', 4, B0110},      // .--.
  {'Q', 4, B1011},      // --.-
  {'R', 3, B010},       // .-.
  {'S', 3, B000},       // ...
  {'T', 1, B1},         // -
  {'U', 3, B100},       // ..-
  {'V', 4, B1000},      // ...-
  {'W', 3, B110},       // .--
  {'X', 4, B1001},      // -..-
  {'Y', 4, B1101},      // -.--
  {'Z', 4, B0011},      // --..
  {'0', 5, B11111},     // -----
  {'1', 5, B11110},     // .----
  {'2', 5, B11100},     // ..---
  {'3', 5, B11000},     // ...--
  {'4', 5, B10000},     // ....-
  {'5', 5, B00000},     // .....
  {'6', 5, B00001},     // -....
  {'7', 5, B00011},     // --...
  {'8', 5, B00111},     // ---..
  {'9', 5, B01111},     // ----.
  {'/', 5, B01001},     // -..-.
  {'?', 6, B001100},    // ..--..
  {'.', 6, B101010},    // .-.-.-
  {',', 6, B110011},    // --..--
  {0}
};

void playMessage(PGM_P msg)
{
  char ch,c;
  while ((ch = pgm_read_byte(msg++)) != 0) {
    if (ch == ' ')
      delay(trx.dit_time*Settings[ID_KEY_WORD_SPACE]/10);
    else {
      const morse_letter* pm = MorseCode;
      for(int i=0; (c=pgm_read_byte(pm)) != 0; i++, pm++)  {
        if (ch == c) {
          // play letter
          uint8_t code = pgm_read_byte((char*)pm+2);
          for (uint8_t j=pgm_read_byte((char*)pm+1); j > 0; j--) {
            if (code & 1) sendDah();
            else sendDit();
            code >>= 1;
          }
          delay(trx.dit_time*Settings[ID_KEY_LETTER_SPACE]/10);
          break;
        }
      }
    }
    if (readDit() || readDah()) return;
  }
}

char decodeMorse(byte bits, byte len)
{
  char c,codelen,code;
  const morse_letter* pm = MorseCode;
  for(int i=0; (c=pgm_read_byte(pm)) != 0; i++, pm++)  {
    codelen = pgm_read_byte((char*)pm+1);
    code = pgm_read_byte((char*)pm+2);
    if (code == bits && codelen == len) return c;
  }
  return '*';
}