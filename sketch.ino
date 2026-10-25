#include <Wire.h>
#include <U8g2lib.h>

// =====================
// Pin define
// =====================
#define OLED_SDA      21
#define OLED_SCL      20

#define ENCODER_CLK   39
#define ENCODER_DT    38
#define ENCODER_SW    37

// =====================
// OLED
// =====================
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// =====================
// Menu state
// =====================
int menuIndex = 0;
const char* menuItems[] = {"Menu 1", "Menu 2", "Menu 3", "Menu 4"};
const int menuCount = 4;

// =====================
// Rotary encoder state
// =====================
volatile long encPos = 0;
uint8_t encPrevState = 0;
int lastEnc = 0;
bool encBtnPrev = HIGH;

// =====================
// Drawing
// =====================
void drawMenu() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);

  u8g2.drawStr(0, 10, "DEBUG MENU");

  for (int i = 0; i < menuCount; i++) {
    int y = 24 + i * 14;

    if (i == menuIndex) {
      u8g2.drawBox(0, y - 10, 128, 12);
      u8g2.setDrawColor(0);
      u8g2.drawStr(6, y, menuItems[i]);
      u8g2.setDrawColor(1);
    } else {
      u8g2.drawStr(6, y, menuItems[i]);
    }
  }

  char posText[32];
  snprintf(posText, sizeof(posText), "enc=%ld", encPos);
  u8g2.drawStr(0, 60, posText);

  u8g2.sendBuffer();
}

// =====================
// Encoder ISR
// =====================
void IRAM_ATTR encISR() {
  uint8_t a = digitalRead(ENCODER_CLK);
  uint8_t b = digitalRead(ENCODER_DT);
  uint8_t state = (a << 1) | b;

  if (state != encPrevState) {
    if ((encPrevState == 0 && state == 1) ||
        (encPrevState == 1 && state == 3) ||
        (encPrevState == 3 && state == 2) ||
        (encPrevState == 2 && state == 0)) {
      encPos++;
    } else if ((encPrevState == 0 && state == 2) ||
               (encPrevState == 2 && state == 3) ||
               (encPrevState == 3 && state == 1) ||
               (encPrevState == 1 && state == 0)) {
      encPos--;
    }
    encPrevState = state;
  }
}

// =====================
// Setup
// =====================
void setup() {
  Serial.begin(115200);
  delay(50);

  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), encISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_DT), encISR, CHANGE);

  encBtnPrev = digitalRead(ENCODER_SW);

  Wire.begin(OLED_SDA, OLED_SCL);
  u8g2.begin();

  drawMenu();
  Serial.println("Debug OLED+Encoder ready");
}

// =====================
// Loop
// =====================
void loop() {
  int cur = (int)encPos;
  if (cur != lastEnc) {
    int delta = cur - lastEnc;
    lastEnc = cur;

    if (delta > 0) {
      menuIndex++;
      if (menuIndex >= menuCount) menuIndex = 0;
    } else if (delta < 0) {
      menuIndex--;
      if (menuIndex < 0) menuIndex = menuCount - 1;
    }

    drawMenu();
    Serial.print("Menu index: ");
    Serial.println(menuIndex);
  }

  int btn = digitalRead(ENCODER_SW);
  if (btn == LOW && encBtnPrev == HIGH) {
    Serial.println("Encoder button pressed");
  }
  encBtnPrev = btn;

  delay(10);
}