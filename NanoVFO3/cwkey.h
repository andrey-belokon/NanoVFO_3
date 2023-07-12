#ifdef ENABLE_INTERNAL_CWKEY

uint8_t cw_bits = 0;
uint8_t cw_bit_idx = 0;

enum {imIDLE,imDIT,imDAH};
uint8_t im_state = imIDLE;

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

void recognizeMorse();

void sendDiDah(uint16_t tm, bool tx)
{
  if (tx) cwTXOn();
  if (tx) digitalWrite(PIN_OUT_KEY, OUT_KEY_ACTIVE_LEVEL);
  OutputTone(PIN_OUT_TONE,Settings[ID_KEY_TONE_HZ]);
  delay(tm);
  if (tx) digitalWrite(PIN_OUT_KEY, !OUT_KEY_ACTIVE_LEVEL);
  OutputTone(PIN_OUT_TONE,0);
  last_cw = millis();
  delay(trx.dit_time);
}

void sendDit(bool tx)
{
  sendDiDah(trx.dit_time,tx);
  cw_bit_idx++;
}

void sendDah(bool tx)
{
  sendDiDah(trx.dah_time,tx);
  cw_bits |= 1 << cw_bit_idx;
  cw_bit_idx++;
}

void playChar(char ch, bool tx)
{
  if (ch == ' ')
    delay(trx.dit_time*Settings[ID_KEY_WORD_SPACE]/10);
  else {
    const morse_letter* pm = MorseCode;
    char c;
    for(int i=0; (c=pgm_read_byte(pm)) != 0; i++, pm++)  {
      if (ch == c) {
        // play letter
        uint8_t code = pgm_read_byte((char*)pm+2);
        for (uint8_t j=pgm_read_byte((char*)pm+1); j > 0; j--) {
          if (code & 1) sendDah(tx);
          else sendDit(tx);
          code >>= 1;
          PoolKeyboard();
          if (last_key) return;
        }
        delay(trx.dit_time*Settings[ID_KEY_LETTER_SPACE]/10);
        break;
      }
    }
  }
  if (readDit() || readDah()) return;
}

char decodeMorse(byte bits, byte len, char unk_sym)
{
  char c,codelen,code;
  const morse_letter* pm = MorseCode;
  for(int i=0; (c=pgm_read_byte(pm)) != 0; i++, pm++)  {
    codelen = pgm_read_byte((char*)pm+1);
    code = pgm_read_byte((char*)pm+2);
    if (code == bits && codelen == len) return c;
  }
  return unk_sym;
}

void ReadCWKey(bool tx)
{
  if (Settings[ID_KEY_ENABLE]) {
    if (Settings[ID_KEY_IAMBIC]) {
      if (im_state == imIDLE) {
        if (readDit()) im_state = imDIT;
        else if (readDah()) im_state = imDAH;
      }
      if (im_state == imDIT) {
        sendDit(tx);
        //now, if dah is pressed go there, else check for dit
        if (readDah()) im_state = imDAH;
        else {
          if (readDit()) im_state = imDIT;
          else {
            //delay(trx.dit_time*Settings[ID_KEY_LETTER_SPACE]/10-trx.dit_time);
            im_state = imIDLE;
          }
        }     
      } else if (im_state == imDAH) {
        sendDah(tx);
        //now, if dit is pressed go there, else check for dah
        if (readDit()) im_state = imDIT;
        else {
          if (readDah()) im_state = imDAH;
          else {
            //delay(trx.dit_time*Settings[ID_KEY_LETTER_SPACE]/10-trx.dit_time);
            im_state = imIDLE;
          }
        }
      }
    } else {
      if (readDit()) {
        sendDit(tx);
      } else if (readDah()) {
        sendDah(tx);
      }
    }
  }
}

bool recognizeMorse(char unk_sym)
{
  if (Settings[ID_CW_DECODER] && millis()-last_cw >= 2*trx.dit_time) {
    char ch = 0;
    if (cw_bit_idx > 0) {
      ch = decodeMorse(cw_bits,cw_bit_idx,unk_sym);
      if (ch) trx.PutCWChar(ch);
    }
    if (millis()-last_cw > 5*trx.dit_time) {
      ch = ' ';
      trx.PutCWChar(ch);
    }
    cw_bits = cw_bit_idx = 0;
    return true;
  }
  return false;
}

#define CWBANK_SIZE   (21*4)
char EEMEM cw_bank[3*CWBANK_SIZE];
#define BANK_IS_FULL  0x5A7E
uint16_t EEMEM cw_bank_full[3];   // == BANK_IS_FULL if bank contains data
uint8_t EEMEM cw_bank_len[3];    // msg length

void playMessage(uint8_t idx, uint8_t len)
{
  const uint8_t *p = (const uint8_t *)(cw_bank+idx*CWBANK_SIZE);
  for (uint8_t i=len+1; i > 0; i--) {
    char ch = eeprom_read_byte(p++);
    trx.PutCWChar(ch);
    disp.Draw(trx);
    playChar(ch, true);
    PoolKeyboard();
    if (last_key || readDit() || readDah()) return;
  }
}

#endif
