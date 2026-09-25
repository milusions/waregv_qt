/*
 * ============================================================================
 * HELIO / EMO-LIGHT — AUTONOMOUS OLED PROFILE CONTROLLER
 * ============================================================================
 *
 * Target:
 *   Arduino Nano / ATmega328P
 *   SSD1306 128x64 I2C OLED
 *
 * Libraries:
 *   Wire.h
 *   avr/pgmspace.h
 *
 * I2C:
 *   SDA = A4
 *   SCL = A5
 *   Default address = 0x3C
 *
 * New Serial commands:
 *   {"profile":"idle"}
 *   {"profile":"listening"}
 *   {"profile":"speaking"}
 * 
 * Kinematic Outputs (sent from Arduino to Host):
 *   {"cmd_vel":{"linear":0.0,"angular":0.0}}
 *
 * ============================================================================
 */

#include <Wire.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <stdlib.h>


// ============================================================================
// DISPLAY CONFIGURATION
// ============================================================================

#define OLED_I2C_ADDR        0x3C
#define OLED_WIDTH           128
#define OLED_HEIGHT          64
#define OLED_PAGES           8
#define OLED_BUFFER_SIZE     1024

#define OLED_I2C_CHUNK       16
#define I2C_RETRIES          3
#define SERIAL_BUFFER_SIZE   64


// ============================================================================
// HELIO FONT (5x7)
// ============================================================================
const uint8_t PROGMEM FONT5X7[][5] = {
  {0x00,0x00,0x00,0x00,0x00}, // 32 SPACE
  {0x00,0x00,0x5F,0x00,0x00}, // !
  {0x00,0x07,0x00,0x07,0x00}, // "
  {0x14,0x7F,0x14,0x7F,0x14}, // #
  {0x24,0x2A,0x7F,0x2A,0x12}, // $
  {0x23,0x13,0x08,0x64,0x62}, // %
  {0x36,0x49,0x55,0x22,0x50}, // &
  {0x00,0x05,0x03,0x00,0x00}, // '
  {0x00,0x1C,0x22,0x41,0x00}, // (
  {0x00,0x41,0x22,0x1C,0x00}, // )
  {0x14,0x08,0x3E,0x08,0x14}, // *
  {0x08,0x08,0x3E,0x08,0x08}, // +
  {0x00,0x50,0x30,0x00,0x00}, // ,
  {0x08,0x08,0x08,0x08,0x08}, // -
  {0x00,0x60,0x60,0x00,0x00}, // .
  {0x20,0x10,0x08,0x04,0x02}, // /
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
  {0x00,0x36,0x36,0x00,0x00}, // :
  {0x00,0x56,0x36,0x00,0x00}, // ;
  {0x08,0x14,0x22,0x41,0x00}, // <
  {0x14,0x14,0x14,0x14,0x14}, // =
  {0x00,0x41,0x22,0x14,0x08}, // >
  {0x02,0x01,0x51,0x09,0x06}, // ?
  {0x32,0x49,0x79,0x41,0x3E}, // @
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
  {0x00,0x7F,0x41,0x41,0x00}, // [
  {0x02,0x04,0x08,0x10,0x20}, // \ 
  {0x00,0x41,0x41,0x7F,0x00}, // ]
  {0x04,0x02,0x01,0x02,0x04}, // ^
  {0x40,0x40,0x40,0x40,0x40}  // _
};

// ============================================================================
// LOW-LEVEL SSD1306 DRIVER
// ============================================================================
class SSD1306Nano {
private:
  uint8_t framebuffer[OLED_BUFFER_SIZE];
  bool connected;
  bool inverted;

  bool command(uint8_t c) {
    for (uint8_t attempt = 0; attempt < I2C_RETRIES; ++attempt) {
      Wire.beginTransmission(OLED_I2C_ADDR);
      Wire.write(0x00);
      Wire.write(c);
      if (Wire.endTransmission(true) == 0) return true;
      delayMicroseconds(250);
    }
    connected = false;
    return false;
  }

  bool command2(uint8_t c, uint8_t v) {
    for (uint8_t attempt = 0; attempt < I2C_RETRIES; ++attempt) {
      Wire.beginTransmission(OLED_I2C_ADDR);
      Wire.write(0x00);
      Wire.write(c);
      Wire.write(v);
      if (Wire.endTransmission(true) == 0) return true;
      delayMicroseconds(250);
    }
    connected = false;
    return false;
  }

  bool writeChunk(uint8_t page, uint8_t column, const uint8_t *data, uint8_t count) {
    if (page >= OLED_PAGES) return false;
    if (column >= OLED_WIDTH) return false;
    if (count == 0) return true;
    if ((uint16_t)column + count > OLED_WIDTH) count = OLED_WIDTH - column;

    for (uint8_t attempt = 0; attempt < I2C_RETRIES; ++attempt) {
      Wire.beginTransmission(OLED_I2C_ADDR);
      Wire.write(0x40);
      for (uint8_t i = 0; i < count; ++i) Wire.write(data[i]);
      if (Wire.endTransmission(true) == 0) return true;
      delayMicroseconds(250);
    }
    connected = false;
    return false;
  }

public:
  SSD1306Nano() : connected(false), inverted(false) { clear(); }

  bool isConnected() const { return connected; }

  void begin() {
    connected = false;
    Wire.begin();
    Wire.setClock(400000UL);
    delay(100);
    Wire.beginTransmission(OLED_I2C_ADDR);
    if (Wire.endTransmission(true) != 0) return;
    connected = true;

    command(0xAE); command2(0xD5, 0x80); command2(0xA8, 0x3F);
    command2(0xD3, 0x00); command(0x40); command2(0x8D, 0x14);
    command2(0x20, 0x00); command(0xA1); command(0xC8);
    command2(0xDA, 0x12); command2(0x81, 0x8F); command2(0xD9, 0xF1);
    command2(0xDB, 0x40); command(0xA4); command(0xA6);
    command(0x2E); command(0xAF);

    inverted = false;
    clear();
    display();
  }

  void clear() { memset(framebuffer, 0, sizeof(framebuffer)); }
  void fillScreen(bool on) { memset(framebuffer, on ? 0xFF : 0x00, sizeof(framebuffer)); }
  void invert(bool state) { inverted = state; if (connected) command(state ? 0xA7 : 0xA6); }

  void pixel(int16_t x, int16_t y, bool on = true) {
    if (x < 0 || x >= OLED_WIDTH || y < 0 || y >= OLED_HEIGHT) return;
    uint16_t index = (uint16_t)x + (uint16_t)(y >> 3) * OLED_WIDTH;
    uint8_t mask = (uint8_t)(1U << (y & 7));
    if (on) framebuffer[index] |= mask;
    else framebuffer[index] &= (uint8_t)~mask;
  }

  void hline(int16_t x, int16_t y, int16_t w, bool on = true) {
    for (int16_t i = 0; i < w; ++i) pixel(x + i, y, on);
  }

  void vline(int16_t x, int16_t y, int16_t h, bool on = true) {
    for (int16_t i = 0; i < h; ++i) pixel(x, y + i, on);
  }

  void line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool on = true) {
    int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int16_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int16_t err = dx + dy;
    while (true) {
      pixel(x0, y0, on);
      if (x0 == x1 && y0 == y1) break;
      int16_t e2 = err * 2;
      if (e2 >= dy) { err += dy; x0 += sx; }
      if (e2 <= dx) { err += dx; y0 += sy; }
    }
  }

  void rect(int16_t x, int16_t y, int16_t w, int16_t h, bool on = true) {
    hline(x, y, w, on); hline(x, y + h - 1, w, on);
    vline(x, y, h, on); vline(x + w - 1, y, h, on);
  }

  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, bool on = true) {
    for (int16_t xx = 0; xx < w; ++xx) vline(x + xx, y, h, on);
  }

  void fillCircle(int16_t cx, int16_t cy, int16_t r, bool on = true) {
    for (int16_t y = -r; y <= r; ++y) {
      int16_t dx = (int16_t)sqrt((int32_t)r * r - (int32_t)y * y);
      hline(cx - dx, cy + y, dx * 2 + 1, on);
    }
  }

  void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, bool on = true) {
    if (r == 0) { fillRect(x, y, w, h, on); return; }
    fillRect(x + r, y, w - 2 * r, h, on);
    for (int16_t yy = 0; yy < r; ++yy) {
      int16_t dy = r - yy - 1;
      int16_t dx = (int16_t)sqrt((int32_t)r * r - (int32_t)dy * dy);
      int16_t left = r - dx;
      hline(x + left, y + yy, w - 2 * left, on);
      hline(x + left, y + h - 1 - yy, w - 2 * left, on);
    }
    fillRect(x, y + r, r, h - 2 * r, on);
    fillRect(x + w - r, y + r, r, h - 2 * r, on);
  }

  void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool on = true) {
    if (y0 > y1) { int16_t t=y0; y0=y1; y1=t; t=x0; x0=x1; x1=t; }
    if (y1 > y2) { int16_t t=y1; y1=y2; y2=t; t=x1; x1=x2; x2=t; }
    if (y0 > y1) { int16_t t=y0; y0=y1; y1=t; t=x0; x0=x1; x1=t; }
    if (y0 == y2) { hline(min(x0, min(x1, x2)), y0, max(x0, max(x1, x2)) - min(x0, min(x1, x2)) + 1, on); return; }
    for (int16_t y = y0; y <= y1; ++y) {
      int32_t xa = x0 + (int32_t)(x1 - x0) * (y - y0) / (y1 - y0);
      int32_t xb = x0 + (int32_t)(x2 - x0) * (y - y0) / (y2 - y0);
      if (xa > xb) { int32_t t = xa; xa = xb; xb = t; }
      hline(xa, y, xb - xa + 1, on);
    }
    for (int16_t y = y1 + 1; y <= y2; ++y) {
      int32_t xa = x1 + (int32_t)(x2 - x1) * (y - y1) / (y2 - y1);
      int32_t xb = x0 + (int32_t)(x2 - x0) * (y - y0) / (y2 - y0);
      if (xa > xb) { int32_t t = xa; xa = xb; xb = t; }
      hline(xa, y, xb - xa + 1, on);
    }
  }

  void char5x7(int16_t x, int16_t y, char c, bool on = true, uint8_t scale = 1) {
    if (c >= 'a' && c <= 'z') c -= ('a' - 'A');
    if (c < 32 || c > 95) c = ' ';
    uint8_t index = (uint8_t)c - 32;
    for (uint8_t col = 0; col < 5; ++col) {
      uint8_t bits = pgm_read_byte(&FONT5X7[index][col]);
      for (uint8_t row = 0; row < 7; ++row) {
        if (bits & (1U << row)) {
          if (scale == 1) pixel(x + col, y + row, on);
          else fillRect(x + col * scale, y + row * scale, scale, scale, on);
        }
      }
    }
  }

  void text(int16_t x, int16_t y, const char *str, bool on = true, uint8_t scale = 1) {
    while (*str) { char5x7(x, y, *str, on, scale); x += 6 * scale; ++str; }
  }

  bool display() {
    if (!connected) return false;
    for (uint8_t page = 0; page < OLED_PAGES; ++page) {
      if (!command(0xB0 | page)) return false;
      if (!command(0x00)) return false;
      if (!command(0x10)) return false;
      uint16_t base = (uint16_t)page * OLED_WIDTH;
      for (uint8_t column = 0; column < OLED_WIDTH; column += OLED_I2C_CHUNK) {
        if (!writeChunk(page, column, &framebuffer[base + column], OLED_I2C_CHUNK)) return false;
      }
    }
    return true;
  }
};


// ============================================================================
// PROFILE & STATE ENUMERATION
// ============================================================================
enum Profile {
  PROFILE_BOOTING = 0,
  PROFILE_IDLE,
  PROFILE_LISTENING,
  PROFILE_SPEAKING,
  PROFILE_INFO,
  PROFILE_SUCCESS,
  PROFILE_ERROR,
  PROFILE_GOAL_RECEIVED,
  PROFILE_NAVIGATED
};

enum IdleSubState {
  IDLE_NORMAL = 0,
  IDLE_THINK,
  IDLE_READ,
  IDLE_PAINT,
  IDLE_SHOOT
};

// ============================================================================
// PROFILE MANAGER & ANIMATION ENGINE
// ============================================================================
class HelioUI {
private:
  SSD1306Nano &oled;
  Profile profile;
  IdleSubState idleState;
  uint8_t frame;
  unsigned long lastFrame;

  // Sends kinematic velocity commands to the host (e.g. ROS / rover base)
  void sendVel(float lin, float ang) {
    
    // Clamp linear velocity to +/- 0.07
    if (lin > 0.07) lin = 0.07;
    if (lin < -0.07) lin = -0.07;
    
    // Clamp angular velocity to +/- 0.2
    if (ang > 0.2) ang = 0.2;
    if (ang < -0.2) ang = -0.2;

    Serial.print(F("{\"cmd_vel\":{\"linear\":"));
    Serial.print(lin, 2);
    Serial.print(F(",\"angular\":"));
    Serial.print(ang, 2);
    Serial.println(F("}}"));
  }

  // Draw expressive rounded eyes (Emo style)
  void drawEmoEyes(int16_t xOffset, int16_t yOffset, uint8_t style = 0) {
    int16_t lx = 28 + xOffset, rx = 76 + xOffset;
    int16_t y = 16 + yOffset;
    int16_t w = 24, h = 32;

    if (style == 1) { // Happy (Bottom cut)
      oled.fillRoundRect(lx, y, w, h, 8, true);
      oled.fillRoundRect(rx, y, w, h, 8, true);
      oled.fillRect(lx, y + h - 10, w, 10, false);
      oled.fillRect(rx, y + h - 10, w, 10, false);
    } else if (style == 2) { // Squint / Shoot
      oled.fillRoundRect(lx, y + 10, w, h - 10, 4, true);
      oled.fillRoundRect(rx, y + 10, w, h - 10, 4, true);
      oled.fillTriangle(lx, y+10, lx+10, y+10, lx, y+20, false); // Inner slant
      oled.fillTriangle(rx+w, y+10, rx+w-10, y+10, rx+w, y+20, false);
    } else if (style == 3) { // Blinking
      oled.fillRect(lx, y + 14, w, 4, true);
      oled.fillRect(rx, y + 14, w, 4, true);
    } else { // Normal
      oled.fillRoundRect(lx, y, w, h, 10, true);
      oled.fillRoundRect(rx, y, w, h, 10, true);
    }
  }

  // Animated Hand
  void drawHand(int16_t x, int16_t y, bool wave) {
    int16_t yo = wave ? (frame % 2 == 0 ? -3 : 3) : 0;
    oled.fillRoundRect(x, y + yo, 18, 16, 4, true);
    oled.fillRect(x + 2, y - 4 + yo, 4, 6, true);
    oled.fillRect(x + 8, y - 5 + yo, 4, 7, true);
    oled.fillRect(x + 14, y - 3 + yo, 3, 5, true);
  }

  // --------------------------------------------------------------------------
  // PROFILE RENDERERS
  // --------------------------------------------------------------------------
  void renderBooting() {
    oled.clear();
    oled.text(42, 28, "HELIO", true, 2);
    oled.fillRect(14, 48, (frame * 10) % 100, 4, true);
    oled.display();
  }

  void renderListening() {
    oled.clear();
    // Wide eyes, slight breathing scale
    int16_t yOffset = (frame % 4 < 2) ? 1 : 0;
    drawEmoEyes(0, yOffset, 0);
    // Listening indicator dots
    uint8_t active = frame % 3;
    for (uint8_t i = 0; i < 3; ++i) oled.fillCircle(56 + i * 8, 56, (i == active) ? 3 : 1, true);
    oled.display();
  }

  void renderSpeaking() {
    oled.clear();
    // Eyes shifting vertically simulating speech excitement
    int16_t yOffset = (frame % 2 == 0) ? -2 : 2;
    drawEmoEyes(0, yOffset, (frame % 8 == 0) ? 3 : 0);
    // Render waving hand
    drawHand(105, 40, true);
    oled.display();
  }

  void renderIdle() {
    oled.clear();
    
    // Manage Idle Sub-States
    if (frame == 0) {
      idleState = (IdleSubState)(random(0, 5));
    }

    int16_t xOff = 0, yOff = 0;
    uint8_t style = (frame % 20 == 19) ? 3 : 0; // Occasional blink

    switch(idleState) {
      case IDLE_NORMAL:
        xOff = (frame > 10) ? 6 : -6;
        drawEmoEyes(xOff, 0, style);
        break;

      case IDLE_THINK:
        yOff = -6; xOff = 6;
        drawEmoEyes(xOff, yOff, 0);
        // Chat Bubble
        oled.fillRoundRect(8, 4, 30, 20, 4, true);
        oled.fillTriangle(28, 24, 38, 24, 34, 30, true);
        oled.fillCircle(14, 14, 1, false);
        oled.fillCircle(23, 14, 1, false);
        oled.fillCircle(32, 14, 1, false);
        break;

      case IDLE_READ:
        yOff = 6;
        drawEmoEyes(0, yOff, style);
        // Book
        oled.fillRoundRect(48, 50, 15, 10, 2, true);
        oled.fillRoundRect(65, 50, 15, 10, 2, true);
        oled.fillRect(50, 52, 11, 1, false);
        oled.fillRect(50, 55, 11, 1, false);
        oled.fillRect(67, 52, 11, 1, false);
        oled.fillRect(67, 55, 11, 1, false);
        break;

      case IDLE_PAINT:
        // Eye tracking
        xOff = (frame % 8 < 4) ? -4 : 4;
        drawEmoEyes(xOff, 0, style);
        // Canvas & Brush
        oled.drawRect(8, 30, 20, 26, true);
        oled.fillRect(10, 32, 16, 22, true);
        oled.drawLine(28, 56, 38, 40, true);
        oled.fillCircle(38, 40, 2, true); // brush tip
        // Move Base (Clamped in sendVel)
        if (frame == 3) sendVel(0.0, 0.2);
        else if (frame == 10) sendVel(0.0, -0.2);
        else if (frame == 17) sendVel(0.0, 0.0);
        break;

      case IDLE_SHOOT:
        drawEmoEyes(0, 0, 2); // Squint style
        // Simple Gun
        oled.fillRect(12, 38, 20, 6, true);
        oled.fillRect(12, 44, 6, 10, true);
        if (frame % 4 == 0) {
          oled.fillCircle(36, 41, 4, true); // Flash
          sendVel(-0.07, 0.0); // Recoil back (Clamped in sendVel)
        } else if (frame % 4 == 2) {
          sendVel(0.0, 0.0);
        }
        break;
    }
    oled.display();
  }

public:
  HelioUI(SSD1306Nano &display) : oled(display), profile(PROFILE_BOOTING), frame(0), lastFrame(0) {
    randomSeed(analogRead(0));
  }

  void setProfile(Profile p) {
    profile = p;
    frame = 0;
    lastFrame = millis();
    oled.invert(false);
    render();
  }

  void render() {
    switch (profile) {
      case PROFILE_BOOTING: renderBooting(); break;
      case PROFILE_IDLE: renderIdle(); break;
      case PROFILE_LISTENING: renderListening(); break;
      case PROFILE_SPEAKING: renderSpeaking(); break;
      default: 
        oled.clear();
        oled.text(10, 28, "SYSTEM OK", true, 2);
        oled.display();
        break;
    }
  }

  void update() {
    unsigned long now = millis();
    uint16_t interval = 300; // Base interval for animations

    if (profile == PROFILE_BOOTING) interval = 100;
    else if (profile == PROFILE_SPEAKING) interval = 150;
    else if (profile == PROFILE_IDLE) interval = 350;

    if (now - lastFrame >= interval) {
      lastFrame = now;
      ++frame;
      if (profile == PROFILE_IDLE && frame >= 24) frame = 0;
      render();
    }
  }
};


// ============================================================================
// GLOBALS & SERIAL PARSER
// ============================================================================
SSD1306Nano oled;
HelioUI helio(oled);

char serialBuffer[SERIAL_BUFFER_SIZE];
uint8_t serialIndex = 0;

void parseCommand(const char *json) {
  if (!json || !json[0]) return;

  const char *key = strstr(json, "\"profile\"");
  if (!key) key = strstr(json, "profile");
  if (!key) return;

  const char *colon = strchr(key, ':');
  if (!colon) return;

  const char *value = colon + 1;
  while (*value == ' ' || *value == '\t' || *value == '"' || *value == '\'') ++value;

  char profileName[32];
  uint8_t i = 0;
  while (value[i] && value[i] != '"' && value[i] != '\'' && value[i] != ',' && value[i] != '}' && i < 31) {
    profileName[i] = value[i];
    ++i;
  }
  profileName[i] = '\0';

  if (strcmp(profileName, "booting") == 0) helio.setProfile(PROFILE_BOOTING);
  else if (strcmp(profileName, "idle") == 0) helio.setProfile(PROFILE_IDLE);
  else if (strcmp(profileName, "listening") == 0) helio.setProfile(PROFILE_LISTENING);
  else if (strcmp(profileName, "speaking") == 0 || strcmp(profileName, "is_speaking") == 0) helio.setProfile(PROFILE_SPEAKING);
  else if (strcmp(profileName, "info") == 0) helio.setProfile(PROFILE_INFO);
  else if (strcmp(profileName, "success") == 0) helio.setProfile(PROFILE_SUCCESS);
  else if (strcmp(profileName, "error") == 0) helio.setProfile(PROFILE_ERROR);
  else helio.setProfile(PROFILE_SUCCESS); // Default fallback
}

// ============================================================================
// SETUP & LOOP
// ============================================================================
void setup() {
  Serial.begin(115200);
  delay(100);
  
  oled.begin();
  helio.setProfile(PROFILE_BOOTING);
}

void loop() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (serialIndex > 0) {
        serialBuffer[serialIndex] = '\0';
        parseCommand(serialBuffer);
        serialIndex = 0;
      }
    } else {
      if (serialIndex < SERIAL_BUFFER_SIZE - 1) {
        serialBuffer[serialIndex++] = c;
      } else {
        serialIndex = 0; // Overflow, reset
      }
    }
  }
  helio.update();
}