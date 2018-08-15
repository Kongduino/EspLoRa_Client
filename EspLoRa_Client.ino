#include <TFT.h>
#include <Esplora.h>
#include <SoftwareSerial.h>
#include <RH_RF95.h>

// Singleton instance of the radio driver
SoftwareSerial ss(11, 3);
RH_RF95 rf95(ss);
int baseX = 76, baseY = 48;

void drawButton(int x, int y, int steps, int clr) {
  EsploraTFT.fillCircle(x, y, 12, clr);
  EsploraTFT.drawLine(x, y - steps, x - steps, y, ST7735_YELLOW);
  EsploraTFT.drawLine(x - steps, y, x, y + steps, ST7735_YELLOW);
  EsploraTFT.drawLine(x, y + steps, x + steps, y, ST7735_YELLOW);
  EsploraTFT.drawLine(x + steps, y, x, y - steps, ST7735_YELLOW);
  EsploraTFT.drawCircle(x, y, 12, ST7735_YELLOW);
}

uint8_t nSFNC = 5, nFQNC = 10, sfNumericChoices[15] = {
  0x72, 0x74, 0x00, // default AGC off
  0x92, 0x74, 0x04, // fast rate AGC off
  0x48, 0x94, 0x04, // SF 9 BW 31.25KHz AGC off
  0x78, 0xc4, 0x0c, // SF 12 BW 125KHz AGC on
  0x08, 0xc4, 0x0c  // SF 12 BW 7.8KHz AGC on
};

float fqChoices[10] = {
  400.0, 410.0, 420.0, 433.0, 434.0, 435.0, 436.0, 448.0, 480.0, 500.0
};
uint8_t fq = 2, nChoice = 3;
// 420.0 MHz, SF 12 BW 125KHz AGC on
float freq = fqChoices[fq];

void switchBWSFAGC() {
  nChoice += 1;
  if (nChoice == nSFNC) nChoice = 0;
  drawSplash();
}

void plusFreq() {
  fq += 1;
  if (fq == nFQNC) fq = 0;
  freq = fqChoices[fq];
  drawSplash();
}

void minusFreq() {
  if (fq == 0) fq = nFQNC - 1;
  else fq -= 1;
  freq = fqChoices[fq];
  drawSplash();
}

void drawFreq() {
  EsploraTFT.setCursor(2, 120);
  EsploraTFT.setTextColor(ST7735_CYAN, ST7735_BLACK);
  EsploraTFT.print(freq); EsploraTFT.print(" MHz");
  EsploraTFT.setTextColor(ST7735_YELLOW, ST7735_BLACK);
}

void drawSFBW() {
  EsploraTFT.setCursor(70, 120);
  EsploraTFT.setTextColor(ST7735_CYAN, ST7735_BLACK);
  uint8_t offset = nChoice * 3;
  String s = String(sfNumericChoices[offset], HEX);
  if (s.length() == 1) s = "0" + s;
  EsploraTFT.print("0x" + s);
  s = String(sfNumericChoices[offset + 1], HEX);
  if (s.length() == 1) s = "0" + s;
  EsploraTFT.print(" 0x" + s);
  s = String(sfNumericChoices[offset + 2], HEX);
  if (s.length() == 1) s = "0" + s;
  EsploraTFT.print(" 0x" + s);
  EsploraTFT.setTextColor(ST7735_YELLOW, ST7735_BLACK);
}

void drawLaunchSplash() {
  EsploraTFT.background(0, 0, 0);
  // Draw splash screen text
  EsploraTFT.setTextSize(2);
  EsploraTFT.setCursor(20, 2);
  EsploraTFT.setTextColor(ST7735_YELLOW, ST7735_BLACK);
  EsploraTFT.print("LoRa Test");
  // Draw the menu
  EsploraTFT.setTextSize(1);
  int x = baseX, y = baseY, steps = 22;
  drawButton(x, y, steps, ST7735_YELLOW);
  EsploraTFT.setCursor(x + steps * .8, y - steps * .66);
  EsploraTFT.print("Stop");
  drawFreq();
  drawSFBW();
}

void drawSplash() {
  EsploraTFT.background(0, 0, 0);
  // Draw splash screen text
  EsploraTFT.setTextSize(2);
  EsploraTFT.setCursor(20, 2);
  EsploraTFT.setTextColor(ST7735_YELLOW, ST7735_BLACK);
  EsploraTFT.print("LoRa Test");
  // Draw the menu
  EsploraTFT.setTextSize(1);
  int x = baseX, y = baseY, steps = 22;
  drawButton(x, y, steps, ST7735_YELLOW);
  EsploraTFT.setCursor(x + steps * .8, y - steps * .66);
  EsploraTFT.print("Launch");

  y = baseY + (steps + 2) * 2;
  drawButton(x, y, steps, ST7735_YELLOW);
  EsploraTFT.setCursor(x + steps, y + steps / 2);
  EsploraTFT.print("BWSF");

  y = baseY + (steps + 22) / 2 + 2; x = baseX + steps + 2;
  drawButton(x, y, steps, ST7735_YELLOW);
  EsploraTFT.setCursor(x + steps * 1.2, y - 4);
  EsploraTFT.print("+freq");

  x = baseX - steps - 2;
  drawButton(x, y, steps, ST7735_YELLOW);
  EsploraTFT.setCursor(0, y - 4);
  EsploraTFT.print("-freq");
  drawSFBW();
  drawFreq();
}

void setup() {
  EsploraTFT.begin();
  drawSplash();

  if (!rf95.init()) {
    EsploraTFT.setTextSize(2);
    EsploraTFT.setCursor(10, 32);
    EsploraTFT.print("INIT FAILED!");
    while (1);
  }
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setFrequency(433.0);
  rf95.write(0x1d, 0x78);
  rf95.write(0x1e, 0xc4);
  rf95.write(0x26, 0x0c);
  rf95.setTxPower(23);
}

uint16_t counter = 0;
char pktBuf[256];


void loop() {
  int x = baseX, y = baseY, steps = 22;
  if (Esplora.readButton(SWITCH_LEFT) == LOW) {
    y = baseY + (steps + 22) / 2 + 2; x = baseX - steps - 2;
    drawButton(x, y, steps, ST7735_MAGENTA);
    delay(500);
    drawButton(x, y, steps, ST7735_YELLOW);
    // #2
    minusFreq();
  } else if (Esplora.readButton(SWITCH_RIGHT) == LOW) {
    y = baseY + (steps + 22) / 2 + 2; x = baseX + steps + 2;
    drawButton(x, y, steps, ST7735_MAGENTA);
    delay(500);
    drawButton(x, y, steps, ST7735_YELLOW);
    // #4
    plusFreq();
  } else if (Esplora.readButton(SWITCH_UP) == LOW) {
    drawButton(x, y, steps, ST7735_MAGENTA);
    delay(500);
    drawButton(x, y, steps, ST7735_YELLOW);
    // #3
    ping();
  } else if (Esplora.readButton(SWITCH_DOWN) == LOW) {
    y = baseY + (steps + 2) * 2;
    drawButton(x, y, steps, ST7735_MAGENTA);
    delay(500);
    drawButton(x, y, steps, ST7735_YELLOW);
    switchBWSFAGC();
  }
}

void ping() {
  // setup with latest settings
  counter = 0;
  rf95.setFrequency(freq);
  uint8_t offset = nChoice * 3;
  rf95.write(0x1d, sfNumericChoices[offset]);
  rf95.write(0x1e, sfNumericChoices[offset + 1]);
  rf95.write(0x26, sfNumericChoices[offset + 2]);
  drawLaunchSplash();
  int x = baseX, y = baseY, steps = 22;
  double t0 = millis(), t1 = t0 + 13000;
  bool gottaStop = false;
  while (!gottaStop) {
    if (Esplora.readButton(SWITCH_UP) == LOW) {
      drawButton(x, y, steps, ST7735_MAGENTA);
      delay(500);
      drawButton(x, y, steps, ST7735_YELLOW);
      gottaStop = true;
    } else {
      if (t1 - t0 > 13000) {
        t0 = t1;
        // Send a message to rf95_server
        if (counter > 0) {
          EsploraTFT.setCursor(2, 88);
          EsploraTFT.setTextColor(ST7735_BLACK, ST7735_BLACK);
          EsploraTFT.print("Send Count: " + String(counter - 1));
          EsploraTFT.setTextColor(ST7735_YELLOW, ST7735_BLACK);
        }
        EsploraTFT.setCursor(2, 88);
        EsploraTFT.print("Send Count: " + String(counter));
        String pkt = "{\"from\": \"Esplora\", \"to\": \"*\", \"sendCount\": " + String(counter++) + ", \"msg\": \"PING\"}";
        uint8_t len = pkt.length() + 1;

        pkt.toCharArray(pktBuf, len);
        rf95.send(pktBuf, len);
        rf95.waitPacketSent();
      }
    }
    t1 = millis();
  }
  drawSplash();
}
