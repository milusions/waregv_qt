#include <Wire.h>

#define OLED_ADDR 0x3C 

enum Mode { IDLE, THINK, ERROR, BULLET };
Mode currentMode = IDLE;

// --- Organic Animation & Kinematic Variables ---
float curX = 0, curY = 0;
float curW = 32, curH = 32;

float tarX = 0, tarY = 0;
float tarW = 32, tarH = 32;

// Biological timing
unsigned long lastGazeTime = 0;
unsigned long nextGazeInterval = 1000;
unsigned long blinkStartTime = 0;
unsigned long nextBlinkTime = 2000;
bool isBlinking = false;
int idleMood = 0; 

// --- Idle Chat Bubble Variables ---
bool chatVisible = false;
bool chatPondering = false;
unsigned long chatStartTime = 0;
unsigned long nextChatTime = 0;
unsigned long chatDuration = 0;
uint8_t chatIndex = 0;

const char* idleMessages[] = {
  "HI HUMAN", "ANY PLANS?", "WHERE TO?", "I AM BORED",
  "BEEP BOOP", "WATCHING...", "GO LEFT?", "GO RIGHT?",
  "HMM...", "I WONDER...", "READY?", "LETS ROAM",
  "MAP LOOKS GOOD", "NEED A HAND?", "STILL HERE", "CURIOUS..."
};
const uint8_t idleMessageCount = sizeof(idleMessages) / sizeof(idleMessages[0]);

// Tiny 5x7 font. Each byte is one column, LSB is the top pixel.
const uint8_t font5x7[][5] PROGMEM = {
  {0x00,0x00,0x00,0x00,0x00}, // space
  {0x3E,0x51,0x49,0x45,0x3E}, // 0
  {0x00,0x42,0x7F,0x40,0x00}, // 1
  {0x42,0x61,0x51,0x49,0x46}, // 2
  {0x21,0x41,0x45,0x4B,0x31}, // 3
  {0x18,0x14,0x12,0x7F,0x10}, // 4
  {0x27,0x45,0x45,0x45,0x39}, // 5
  {0x3C,0x4A,0x49,0x49,0x30}, // 6
  {0x01,0x71,0x09,0x05,0x03}, // 7
  {0x36,0x49,0x49,0x49,0x36}, // 8
  {0x06,0x49,0x49,0x29,0x1E}, // 9
  {0x7E,0x11,0x11,0x11,0x7E}, // A
  {0x7F,0x49,0x49,0x49,0x36}, // B
  {0x3E,0x41,0x41,0x41,0x22}, // C
  {0x7F,0x41,0x41,0x22,0x1C}, // D
  {0x7F,0x49,0x49,0x49,0x41}, // E
  {0x7F,0x09,0x09,0x09,0x01}, // F
  {0x3E,0x41,0x49,0x49,0x7A}, // G
  {0x7F,0x08,0x08,0x08,0x7F}, // H
  {0x00,0x41,0x7F,0x41,0x00}, // I
  {0x20,0x40,0x41,0x3F,0x01}, // J
  {0x7F,0x08,0x14,0x22,0x41}, // K
  {0x7F,0x40,0x40,0x40,0x40}, // L
  {0x7F,0x02,0x0C,0x02,0x7F}, // M
  {0x7F,0x04,0x08,0x10,0x7F}, // N
  {0x3E,0x41,0x41,0x41,0x3E}, // O
  {0x7F,0x09,0x09,0x09,0x06}, // P
  {0x3E,0x41,0x51,0x21,0x5E}, // Q
  {0x7F,0x09,0x19,0x29,0x46}, // R
  {0x46,0x49,0x49,0x49,0x31}, // S
  {0x01,0x01,0x7F,0x01,0x01}, // T
  {0x3F,0x40,0x40,0x40,0x3F}, // U
  {0x1F,0x20,0x40,0x20,0x1F}, // V
  {0x3F,0x40,0x38,0x40,0x3F}, // W
  {0x63,0x14,0x08,0x14,0x63}, // X
  {0x07,0x08,0x70,0x08,0x07}, // Y
  {0x61,0x51,0x49,0x45,0x43}, // Z
  {0x00,0x36,0x36,0x00,0x00}, // .
  {0x00,0x60,0x60,0x00,0x00}, // :
  {0x00,0x08,0x08,0x08,0x00}, // -
  {0x00,0x08,0x00,0x00,0x00}  // fallback ?
};

// 128x64 display buffer (Takes 1024 bytes of the Nano's 2048 bytes RAM)
uint8_t frameBuffer[1024];

void sendCommand(uint8_t cmd) {
  Wire.beginTransmission(OLED_ADDR);
  Wire.write(0x00);
  Wire.write(cmd);
  Wire.endTransmission();
}

void initOLED() {
  Wire.begin();
  Wire.setClock(400000); // Fast I2C for liquid framerates
  delay(50);
  
  sendCommand(0xAE); 
  sendCommand(0xD5); sendCommand(0x80); 
  sendCommand(0xA8); sendCommand(0x3F); 
  sendCommand(0xD3); sendCommand(0x00); 
  sendCommand(0x40);                     
  sendCommand(0x8D); sendCommand(0x14); 
  sendCommand(0xA1);                     
  sendCommand(0xC8);                     
  sendCommand(0xDA); sendCommand(0x12); 
  sendCommand(0x81); sendCommand(0xCF); 
  sendCommand(0xD9); sendCommand(0xF1); 
  sendCommand(0xDB); sendCommand(0x40); 
  sendCommand(0xA4);                     
  sendCommand(0xA6);                     
  
  // Clear RAM Memory
  for (uint8_t page = 0; page < 8; page++) {
    sendCommand(0xB0 + page);
    sendCommand(0x02);
    sendCommand(0x10);
    Wire.beginTransmission(OLED_ADDR);
    Wire.write(0x40);
    for (uint8_t col = 0; col < 128; col++) {
      Wire.write(0x00);
    }
    Wire.endTransmission();
  }
  sendCommand(0xAF); 
}

void clearBuffer() {
  memset(frameBuffer, 0, sizeof(frameBuffer));
}

void renderBuffer() {
  for (uint8_t page = 0; page < 8; page++) {
    sendCommand(0xB0 + page); 
    sendCommand(0x02); // SH1106 offset (+2 column offset)
    sendCommand(0x10);        
    
    for (uint8_t chunk = 0; chunk < 128; chunk += 16) {
      Wire.beginTransmission(OLED_ADDR);
      Wire.write(0x40);
      for (uint8_t i = 0; i < 16; i++) {
        Wire.write(frameBuffer[page * 128 + chunk + i]);
      }
      Wire.endTransmission();
    }
  }
}

void drawPixel(int x, int y, bool color) {
  if (x < 0 || x >= 128 || y < 0 || y >= 64) return;
  uint16_t index = x + (y / 8) * 128;
  if (color) frameBuffer[index] |= (1 << (y % 8));
  else       frameBuffer[index] &= ~(1 << (y % 8));
}

void fillRoundRect(int x, int y, int w, int h, int r, bool color) {
  for (int i = x; i < x + w; i++) {
    for (int j = y; j < y + h; j++) {
      if ((i < x + r && j < y + r && (x + r - i)*(x + r - i) + (y + r - j)*(y + r - j) > r*r) ||
          (i >= x + w - r && j < y + r && (i - (x + w - r - 1))*(i - (x + w - r - 1)) + (y + r - j)*(y + r - j) > r*r) ||
          (i < x + r && j >= y + h - r && (x + r - i)*(x + r - i) + (j - (y + h - r - 1))*(j - (y + h - r - 1)) > r*r) ||
          (i >= x + w - r && j >= y + h - r && (i - (x + w - r - 1))*(i - (x + w - r - 1)) + (j - (y + h - r - 1))*(j - (y + h - r - 1)) > r*r)) {
        continue;
      }
      drawPixel(i, j, color);
    }
  }
}

void drawLine(int x0, int y0, int x1, int y1, bool color = true) {
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy, e2;
  while (true) {
    drawPixel(x0, y0, color);
    if (x0 == x1 && y0 == y1) break;
    e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; }
    if (e2 <= dx) { err += dx; y0 += sy; }
  }
}

void fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, bool color = true) {
  int a, b, y, last;
  if (y0 > y1) { int t=y0; y0=y1; y1=t; t=x0; x0=x1; x1=t; }
  if (y1 > y2) { int t=y1; y1=y2; y2=t; t=x1; x1=x2; x2=t; }
  if (y0 > y1) { int t=y0; y0=y1; y1=t; t=x0; x0=x1; x1=t; }

  if (y0 == y2) { a = b = x0; if (x1 < a) a = x1; else if (x1 > b) b = x1; if (x2 < a) a = x2; else if (x2 > b) b = x2; drawLine(a, y0, b, y0, color); return; }
  int dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0, dx12 = x2 - x1, dy12 = y2 - y1;
  int32_t sa = 0, sb = 0;

  if (y1 == y2) last = y1; else last = y1 - 1;
  for (y = y0; y <= last; y++) {
    a = x0 + sa / dy01; b = x0 + sb / dy02; sa += dx01; sb += dx02;
    if (a > b) { int t=a; a=b; b=t; } drawLine(a, y, b, y, color);
  }
  sa = (int32_t)dx12 * (y - y1); sb = (int32_t)dx02 * (y - y0);
  for (; y <= y2; y++) {
    a = x1 + sa / dy12; b = x0 + sb / dy02; sa += dx12; sb += dx02;
    if (a > b) { int t=a; a=b; b=t; } drawLine(a, y, b, y, color);
  }
}

int fontIndex(char c) {
  if (c == ' ') return 0;
  if (c >= '0' && c <= '9') return 1 + (c - '0');
  if (c >= 'A' && c <= 'Z') return 11 + (c - 'A');
  if (c == '.') return 37;
  if (c == ':') return 38;
  if (c == '-') return 39;
  return 40;
}

void drawChar5x7(int x, int y, char c, bool color = true) {
  int idx = fontIndex(c);
  for (int col = 0; col < 5; col++) {
    uint8_t bits = pgm_read_byte(&font5x7[idx][col]);
    for (int row = 0; row < 7; row++) {
      if (bits & (1 << row)) drawPixel(x + col, y + row, color);
    }
  }
}

int textWidth5x7(const char* text) {
  int w = 0;
  while (*text) { w += 6; text++; }
  return w > 0 ? w - 1 : 0;
}

void drawText5x7(int x, int y, const char* text) {
  while (*text) {
    drawChar5x7(x, y, *text++);
    x += 6;
  }
}

void updateIdleChat() {
  unsigned long t = millis();
  if (currentMode != IDLE) {
    chatVisible = false;
    chatPondering = false;
    return;
  }

  if (!chatVisible && t >= nextChatTime) {
    chatIndex = random(0, idleMessageCount);
    chatPondering = (random(0, 100) < 22);
    chatStartTime = t;
    chatDuration = random(2600, 4800);
    chatVisible = true;
  }

  if (chatVisible && t - chatStartTime >= chatDuration) {
    chatVisible = false;
    chatPondering = false;
    nextChatTime = t + random(4500, 11000);
  }
}

void drawIdleChat() {
  if (!chatVisible || currentMode != IDLE) return;

  const char* msg = idleMessages[chatIndex];
  const int maxTextW = 68;
  char line1[20];
  char line2[20];
  line1[0] = '\0';
  line2[0] = '\0';

  int len = strlen(msg);
  int split = -1;
  int currentW = 0;
  for (int i = 0; i < len; i++) {
    currentW += 6;
    if (msg[i] == ' ' && currentW <= maxTextW) split = i;
    if (currentW > maxTextW) break;
  }
  if (currentW <= maxTextW) {
    strncpy(line1, msg, sizeof(line1) - 1);
    line1[sizeof(line1) - 1] = '\0';
  } else if (split > 0) {
    int n1 = split;
    if (n1 >= (int)sizeof(line1)) n1 = sizeof(line1) - 1;
    strncpy(line1, msg, n1);
    line1[n1] = '\0';
    strncpy(line2, msg + split + 1, sizeof(line2) - 1);
    line2[sizeof(line2) - 1] = '\0';
  } else {
    strncpy(line1, msg, sizeof(line1) - 1);
    line1[sizeof(line1) - 1] = '\0';
  }

  int w1 = textWidth5x7(line1);
  int w2 = textWidth5x7(line2);
  int textW = (w1 > w2) ? w1 : w2;
  int bubbleW = textW + 12;
  if (bubbleW < 38) bubbleW = 38;
  if (bubbleW > 78) bubbleW = 78;
  bool twoLines = line2[0] != '\0';
  int bubbleH = twoLines ? 27 : 17;
  int bx = 128 - bubbleW - 2;
  int by = 2;

  unsigned long age = millis() - chatStartTime;
  unsigned long remaining = (chatDuration > age) ? (chatDuration - age) : 0;
  int inset = 0;
  if (age < 180) inset = 3 - (age / 60);
  else if (remaining < 180) inset = 1 + ((180 - remaining) / 60);
  if (inset < 0) inset = 0;

  fillRoundRect(bx + inset, by + inset, bubbleW - 2 * inset, bubbleH - 2 * inset, 4, true);
  fillRoundRect(bx + 2 + inset, by + 2 + inset,
                bubbleW - 4 - 2 * inset, bubbleH - 4 - 2 * inset, 2, false);

  fillTriangle(bx + 8, by + bubbleH - 1,
               bx + 16, by + bubbleH - 1,
               bx + 9, by + bubbleH + 5, true);
  drawLine(bx + 9, by + bubbleH - 2, bx + 9, by + bubbleH + 3, false);

  if (chatPondering) {
    int dots = (millis() / 220) % 4;
    for (int i = 0; i < dots; i++) {
      fillRoundRect(bx + 12 + i * 10, by + bubbleH / 2 - 2, 5, 5, 1, true);
    }
  } else {
    int tx1 = bx + (bubbleW - w1) / 2;
    int tx2 = bx + (bubbleW - w2) / 2;
    if (tx1 < bx + 4) tx1 = bx + 4;
    if (tx2 < bx + 4) tx2 = bx + 4;
    drawText5x7(tx1, by + 5, line1);
    if (twoLines) drawText5x7(tx2, by + 15, line2);
  }
}

void setup() {
  Serial.begin(9600);
  initOLED();
  clearBuffer();
  renderBuffer();
}

void loop() {
  handleSerialInput();
  
  updateBehaviors();
  updateIdleChat();
  applyKinematics();
  
  clearBuffer();
  drawScene();
  drawIdleChat();
  renderBuffer();
}

// Memory-friendly static char parsing for ATmega328P
void handleSerialInput() {
  static char buffer[16];
  static size_t idx = 0;

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      buffer[idx] = '\0'; // Null-terminate string
      
      if (strstr(buffer, "IDLE") != NULL) currentMode = IDLE;
      else if (strstr(buffer, "THINK") != NULL) currentMode = THINK;
      else if (strstr(buffer, "ERROR") != NULL) currentMode = ERROR;
      else if (strstr(buffer, "BULLET") != NULL) currentMode = BULLET;
      
      idx = 0; // Reset index for next command
    } else if (idx < sizeof(buffer) - 1) {
      buffer[idx++] = c;
    }
  }
}

void updateBehaviors() {
  unsigned long t = millis();
  
  // Natural Blinking Logic
  if (t > nextBlinkTime && !isBlinking && currentMode != BULLET) {
    isBlinking = true;
    blinkStartTime = t;
    
    if (random(0, 100) > 85) {
      nextBlinkTime = t + 250; 
    } else {
      nextBlinkTime = t + random(2000, 6000);
    }
  }
  
  if (isBlinking && (t - blinkStartTime > 120)) {
    isBlinking = false;
  }

  switch(currentMode) {
    case IDLE:
      if (t - lastGazeTime > nextGazeInterval) {
        tarX = random(-14, 15);
        tarY = random(-8, 9);
        
        idleMood = random(0, 10);
        if (idleMood > 8) { tarW = 36; tarH = 30; }      
        else if (idleMood > 6) { tarW = 28; tarH = 36; } 
        else { tarW = 32; tarH = 34; }                   

        lastGazeTime = t;
        nextGazeInterval = random(400, 2500); 
      }
      break;

    case THINK:
      tarW = 28; tarH = 18; 
      tarY = -8;            
      
      if (t - lastGazeTime > 300) {
        tarX = random(2, 14);
        lastGazeTime = t;
      }
      break;

    case ERROR:
      tarW = 30; tarH = 30;
      tarY = 6; 
      tarX = (t % 100 < 50) ? 1 : -1;
      break;

    case BULLET:
      int cycle = t % 1800;
      if (cycle < 1200) {
        float p = cycle / 1200.0; 
        tarX = -p * 38;             
        tarY = p * 16;              
        tarW = 32 - (p * 14);       
        tarH = 34 - (p * 14);
      } else {
        float recover = (cycle - 1200) / 600.0;
        tarX = -38 + (recover * 38);
        tarY = 16 - (recover * 16);
        tarW = 18 + (recover * 14);
        tarH = 20 + (recover * 14);
      }
      break;
  }
}

void applyKinematics() {
  float speedXY = (currentMode == BULLET) ? 0.6 : 0.4; 
  
  curX += (tarX - curX) * speedXY;
  curY += (tarY - curY) * speedXY;
  
  curW += (tarW - curW) * 0.35;
  
  float activeTarH = tarH;
  if (isBlinking) activeTarH = 2; 
  
  curH += (activeTarH - curH) * 0.7; 
}

void drawScene() {
  unsigned long t = millis();
  
  // Biological Breathing
  float breathFreq = (currentMode == IDLE) ? 800.0 : ((currentMode == ERROR) ? 300.0 : 500.0);
  float breathAmp = (currentMode == IDLE) ? 2.0 : 1.0;
  float breathY = sin(t / breathFreq) * breathAmp;

  // Micro-saccades
  float jitterX = 0;
  if (currentMode == IDLE && !isBlinking && curH > 15) {
    jitterX = cos(t / 150.0) * 0.5;
  }

  // Draw Eyes
  int eyeSpacing = (currentMode == BULLET) ? (28 - (32 - (int)curW)/2) : 28;
  
  int lX = 64 - eyeSpacing - (int)curW/2 + (int)curX + jitterX;
  int rX = 64 + eyeSpacing - (int)curW/2 + (int)curX + jitterX;
  int y = 32 - (int)curH/2 + (int)curY + breathY;
  
  fillRoundRect(lX, y, (int)curW, (int)curH, ((int)curW > 16) ? 8 : 4, true);
  fillRoundRect(rX, y, (int)curW, (int)curH, ((int)curW > 16) ? 8 : 4, true);

  if (curH > 10) {
    if (currentMode == IDLE && tarY < -3) {
      fillRoundRect(lX - 2, y + (int)curH - 6, (int)curW + 4, 10, 4, false);
      fillRoundRect(rX - 2, y + (int)curH - 6, (int)curW + 4, 10, 4, false);
    }
    else if (currentMode == ERROR) {
      for(int i = 0; i < 14; i++) {
        drawLine(lX + (int)curW - i, y, lX + (int)curW, y + i, false); 
        drawLine(rX, y, rX + i, y + i, false);            
      }
    }
  }

  if (currentMode == THINK) {
    fillRoundRect(94, 2, 32, 14, 3, true);
    fillRoundRect(96, 4, 28, 10, 2, false); 
    int dots = (t / 120) % 4; 
    if(dots > 0) fillRoundRect(98,  6, 5, 6, 1, true);
    if(dots > 1) fillRoundRect(107, 6, 5, 6, 1, true);
    if(dots > 2) fillRoundRect(116, 6, 5, 6, 1, true);
  }
  else if (currentMode == ERROR) {
    if ((t / 300) % 2 == 0) {
      fillTriangle(112, 1, 98, 19, 126, 19, true);  
      fillTriangle(112, 5, 102, 17, 122, 17, false); 
      fillRoundRect(111, 7, 3, 6, 1, true);
      fillRoundRect(111, 15, 3, 3, 1, true);
    }
  }
  else if (currentMode == BULLET) {
    int cycle = t % 1800;
    if (cycle < 1200) {
      float p = cycle / 1200.0; 
      int bulletSize = 2 + (int)(p * p * 34); 
      int bulletX = 64 + (int)(p * 10); 
      int bulletY = 32 - (int)(p * 4);
      
      if (bulletSize > 4) {
        fillRoundRect(bulletX - bulletSize/2 - 2, bulletY - bulletSize/2 - 2, bulletSize + 4, bulletSize + 4, bulletSize/2, false);
        fillRoundRect(bulletX - bulletSize/2, bulletY - bulletSize/2, bulletSize, bulletSize, bulletSize/2 - 1, true);
        if (bulletSize > 12) fillRoundRect(bulletX - bulletSize/4, bulletY - bulletSize/4, bulletSize/2, bulletSize/2, bulletSize/4, false);
      } else {
        drawPixel(bulletX, bulletY, true);
      }
    }
  }
}