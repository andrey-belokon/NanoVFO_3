#define CAT_BUF_SIZE  40
char CAT_buf[CAT_BUF_SIZE];
uint8_t CAT_buf_idx = 0;

void ExecCAT()
{
  int b;
  while ((b = Serial.read()) >= 0) {
    if (b <= ' ') continue;
    if (CAT_buf_idx >= CAT_BUF_SIZE) CAT_buf_idx = 0;
    CAT_buf[CAT_buf_idx++] = (uint8_t)b;
    if (b == ';') {
      /*Serial.println();
      Serial.write(CAT_buf);
      Serial.println();*/
      // parse command
      if (CAT_buf[0] == 'I' && CAT_buf[1] == 'F') {
        ltoazp(CAT_buf + 2, trx.Freq, 11);
        memset(CAT_buf + 13, ' ', 5);
        memset(CAT_buf + 18, '0', 6);
        memset(CAT_buf + 24, '0', 4);
        CAT_buf[28] = (trx.TX ? '1' : '0');
        CAT_buf[29] = (trx.CW ? '3' : (trx.sideband == LSB ? '1' : '2'));
        CAT_buf[30] = '0';// + trx.state.VFO_Index;
        CAT_buf[31] = '0';
        CAT_buf[32] = (trx.split ? '1' : '0');
        memset(CAT_buf + 33, '0', 3);
        CAT_buf[36] = ' ';
        CAT_buf[37] = ';';
        CAT_buf[38] = 0;
        Serial.write(CAT_buf);
      } else if (CAT_buf[0] == 'I' && CAT_buf[1] == 'D') {
       CAT_buf[2] = '0';
       CAT_buf[3] = '2';
       CAT_buf[4] = '0';
       CAT_buf[5] = ';';
       CAT_buf[6] = 0;
       Serial.write(CAT_buf);
      } else if (CAT_buf[0] == 'F' && (CAT_buf[1] == 'A' || CAT_buf[1] == 'B')) {
        if (CAT_buf[2] == ';') {
          ltoazp(CAT_buf + 2, trx.Freq, 11);
          CAT_buf[13] = ';';
          CAT_buf[14] = 0;
          Serial.write(CAT_buf);
        } else {
          trx.SetFreqBand(atoln(CAT_buf + 2, 11));
        }
      } else if (CAT_buf[0] == 'M' && CAT_buf[1] == 'D') {
        if (CAT_buf[2] == ';') {
          CAT_buf[2] = (trx.CW ? '3' : (trx.sideband == LSB ? '1' : '2'));
          CAT_buf[3] = ';';
          CAT_buf[4] = 0;
          Serial.write(CAT_buf);
        } else {
          switch (CAT_buf[2]) {
            case '1':  
              trx.sideband = LSB;
              trx.CW = 0;
              break;
            case '2':
              trx.sideband = USB;
              trx.CW = 0;
              break;
            case '3':
              trx.CW = 1;
              trx.sideband = Bands[trx.BandIndex].sideband;
              break;
          }
        }
      } else if (CAT_buf[0] == 'B' && CAT_buf[1] == 'U') {
        trx.NextBand();
/*      } else if (CAT_buf[0] == 'T' && CAT_buf[1] == 'X') {
       digitalWrite(PIN_OUT_TX,HIGH);
      } else if (CAT_buf[0] == 'R' && CAT_buf[1] == 'X') {
       digitalWrite(PIN_OUT_TX,LOW); */
      } else {
        Serial.write("?;");
      }
      CAT_buf_idx = 0;
    }
  }
}
