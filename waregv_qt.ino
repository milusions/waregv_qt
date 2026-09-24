/*
 * ============================================================================
 * HELIO — BEAUTIFIED STANDALONE OLED PROFILE CONTROLLER
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
 * No Adafruit / U8g2 / GFX libraries.
 *
 * I2C:
 *   SDA = A4
 *   SCL = A5
 *   Default address = 0x3C
 *
 * Profiles:
 *   01 BOOTING
 *   02 IDLE
 *   03 INFO
 *   04 SUCCESS
 *   05 ERROR
 *   06 GOAL RECEIVED
 *   07 NAVIGATING
 *   08 GOAL REACHED
 *   09 NAVIGATION ERROR
 *   10 SPEAKING
 *   11 NOT SPEAKING
 *
 * Serial command:
 *   {"profile":"booting"}
 *   {"profile":"idle"}
 *   {"profile":"info"}
 *   {"profile":"success"}
 *   {"profile":"error"}
 *   {"profile":"goal_received"}
 *   {"profile":"navigated"}
 *   {"profile":"goal_reached"}
 *   {"profile":"navigation_error"}
 *   {"profile":"is_speaking"}
 *   {"profile":"is_not_speaking"}
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

/*
 * AVR Wire has a 32-byte buffer.
 * 16 bytes of display payload + control byte stays comfortably below it.
 */
#define OLED_I2C_CHUNK       16
#define I2C_RETRIES          3

#define SERIAL_BUFFER_SIZE   64


// ============================================================================
// HELIO FONT
// ============================================================================
//
// Compact 5x7 bitmap font.
// Characters are stored in PROGMEM.
// Each character is 5 columns wide.
// Bit 0 is the top pixel.
//
// ASCII range: 32 (' ') through 90 ('Z')
// Additional punctuation required by the UI is included.
// ============================================================================

const uint8_t PROGMEM FONT5X7[][5] = {

  // 32 SPACE
  {0x00,0x00,0x00,0x00,0x00},

  // !
  {0x00,0x00,0x5F,0x00,0x00},

  // "
  {0x00,0x07,0x00,0x07,0x00},

  // #
  {0x14,0x7F,0x14,0x7F,0x14},

  // $
  {0x24,0x2A,0x7F,0x2A,0x12},

  // %
  {0x23,0x13,0x08,0x64,0x62},

  // &
  {0x36,0x49,0x55,0x22,0x50},

  // '
  {0x00,0x05,0x03,0x00,0x00},

  // (
  {0x00,0x1C,0x22,0x41,0x00},

  // )
  {0x00,0x41,0x22,0x1C,0x00},

  // *
  {0x14,0x08,0x3E,0x08,0x14},

  // +
  {0x08,0x08,0x3E,0x08,0x08},

  // ,
  {0x00,0x50,0x30,0x00,0x00},

  // -
  {0x08,0x08,0x08,0x08,0x08},

  // .
  {0x00,0x60,0x60,0x00,0x00},

  // /
  {0x20,0x10,0x08,0x04,0x02},

  // 0
  {0x3E,0x51,0x49,0x45,0x3E},

  // 1
  {0x00,0x42,0x7F,0x40,0x00},

  // 2
  {0x42,0x61,0x51,0x49,0x46},

  // 3
  {0x21,0x41,0x45,0x4B,0x31},

  // 4
  {0x18,0x14,0x12,0x7F,0x10},

  // 5
  {0x27,0x45,0x45,0x45,0x39},

  // 6
  {0x3C,0x4A,0x49,0x49,0x30},

  // 7
  {0x01,0x71,0x09,0x05,0x03},

  // 8
  {0x36,0x49,0x49,0x49,0x36},

  // 9
  {0x06,0x49,0x49,0x29,0x1E},

  // :
  {0x00,0x36,0x36,0x00,0x00},

  // ;
  {0x00,0x56,0x36,0x00,0x00},

  // <
  {0x08,0x14,0x22,0x41,0x00},

  // =
  {0x14,0x14,0x14,0x14,0x14},

  // >
  {0x00,0x41,0x22,0x14,0x08},

  // ?
  {0x02,0x01,0x51,0x09,0x06},

  // @
  {0x32,0x49,0x79,0x41,0x3E},

  // A
  {0x7E,0x11,0x11,0x11,0x7E},

  // B
  {0x7F,0x49,0x49,0x49,0x36},

  // C
  {0x3E,0x41,0x41,0x41,0x22},

  // D
  {0x7F,0x41,0x41,0x22,0x1C},

  // E
  {0x7F,0x49,0x49,0x49,0x41},

  // F
  {0x7F,0x09,0x09,0x09,0x01},

  // G
  {0x3E,0x41,0x49,0x49,0x7A},

  // H
  {0x7F,0x08,0x08,0x08,0x7F},

  // I
  {0x00,0x41,0x7F,0x41,0x00},

  // J
  {0x20,0x40,0x41,0x3F,0x01},

  // K
  {0x7F,0x08,0x14,0x22,0x41},

  // L
  {0x7F,0x40,0x40,0x40,0x40},

  // M
  {0x7F,0x02,0x0C,0x02,0x7F},

  // N
  {0x7F,0x04,0x08,0x10,0x7F},

  // O
  {0x3E,0x41,0x41,0x41,0x3E},

  // P
  {0x7F,0x09,0x09,0x09,0x06},

  // Q
  {0x3E,0x41,0x51,0x21,0x5E},

  // R
  {0x7F,0x09,0x19,0x29,0x46},

  // S
  {0x46,0x49,0x49,0x49,0x31},

  // T
  {0x01,0x01,0x7F,0x01,0x01},

  // U
  {0x3F,0x40,0x40,0x40,0x3F},

  // V
  {0x1F,0x20,0x40,0x20,0x1F},

  // W
  {0x3F,0x40,0x38,0x40,0x3F},

  // X
  {0x63,0x14,0x08,0x14,0x63},

  // Y
  {0x07,0x08,0x70,0x08,0x07},

  // Z
  {0x61,0x51,0x49,0x45,0x43},

  // [  (index 59)
  {0x00,0x7F,0x41,0x41,0x00},

  // \  (60)
  {0x02,0x04,0x08,0x10,0x20},

  // ]  (61)
  {0x00,0x41,0x41,0x7F,0x00},

  // ^  (62)
  {0x04,0x02,0x01,0x02,0x04},

  // _  (63)
  {0x40,0x40,0x40,0x40,0x40}
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

      if (Wire.endTransmission(true) == 0) {
        return true;
      }

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

      if (Wire.endTransmission(true) == 0) {
        return true;
      }

      delayMicroseconds(250);
    }

    connected = false;
    return false;
  }


  bool writeChunk(
      uint8_t page,
      uint8_t column,
      const uint8_t *data,
      uint8_t count) {

    if (page >= OLED_PAGES) return false;
    if (column >= OLED_WIDTH) return false;
    if (count == 0) return true;

    if ((uint16_t)column + count > OLED_WIDTH) {
      count = OLED_WIDTH - column;
    }

    for (uint8_t attempt = 0; attempt < I2C_RETRIES; ++attempt) {

      Wire.beginTransmission(OLED_I2C_ADDR);
      Wire.write(0x40);

      for (uint8_t i = 0; i < count; ++i) {
        Wire.write(data[i]);
      }

      if (Wire.endTransmission(true) == 0) {
        return true;
      }

      delayMicroseconds(250);
    }

    connected = false;
    return false;
  }


public:

  SSD1306Nano() :
    connected(false),
    inverted(false) {
    clear();
  }


  bool isConnected() const {
    return connected;
  }


  void begin() {

    connected = false;

    Wire.begin();
    Wire.setClock(400000UL);

    delay(100);

    Wire.beginTransmission(OLED_I2C_ADDR);

    if (Wire.endTransmission(true) != 0) {
      Serial.println(F("OLED I2C ERROR"));
      return;
    }

    connected = true;

    // SSD1306 128x64 initialization.
    command(0xAE);              // Display OFF
    command2(0xD5, 0x80);       // Clock
    command2(0xA8, 0x3F);       // Multiplex 64
    command2(0xD3, 0x00);       // Display offset
    command(0x40);              // Start line 0
    command2(0x8D, 0x14);       // Internal charge pump
    command2(0x20, 0x00);       // Horizontal addressing
    command(0xA1);              // Segment remap
    command(0xC8);              // COM scan direction
    command2(0xDA, 0x12);       // COM pins
    command2(0x81, 0x8F);       // Contrast
    command2(0xD9, 0xF1);       // Pre-charge
    command2(0xDB, 0x40);       // VCOMH
    command(0xA4);              // RAM display
    command(0xA6);              // Normal display
    command(0x2E);              // Stop scroll
    command(0xAF);              // Display ON

    inverted = false;

    clear();
    display();
  }


  void clear() {
    memset(framebuffer, 0, sizeof(framebuffer));
  }


  void fillScreen(bool on) {
    memset(framebuffer, on ? 0xFF : 0x00, sizeof(framebuffer));
  }


  void invert(bool state) {

    inverted = state;

    if (connected) {
      command(state ? 0xA7 : 0xA6);
    }
  }


  void pixel(int16_t x, int16_t y, bool on = true) {

    if (x < 0 || x >= OLED_WIDTH ||
        y < 0 || y >= OLED_HEIGHT) {
      return;
    }

    uint16_t index =
      (uint16_t)x +
      (uint16_t)(y >> 3) * OLED_WIDTH;

    uint8_t mask =
      (uint8_t)(1U << (y & 7));

    if (on) {
      framebuffer[index] |= mask;
    } else {
      framebuffer[index] &= (uint8_t)~mask;
    }
  }


  void hline(
      int16_t x,
      int16_t y,
      int16_t w,
      bool on = true) {

    if (w <= 0) return;

    for (int16_t i = 0; i < w; ++i) {
      pixel(x + i, y, on);
    }
  }


  void vline(
      int16_t x,
      int16_t y,
      int16_t h,
      bool on = true) {

    if (h <= 0) return;

    for (int16_t i = 0; i < h; ++i) {
      pixel(x, y + i, on);
    }
  }


  void line(
      int16_t x0,
      int16_t y0,
      int16_t x1,
      int16_t y1,
      bool on = true) {

    int16_t dx = abs(x1 - x0);
    int16_t sx = x0 < x1 ? 1 : -1;

    int16_t dy = -abs(y1 - y0);
    int16_t sy = y0 < y1 ? 1 : -1;

    int16_t err = dx + dy;

    while (true) {

      pixel(x0, y0, on);

      if (x0 == x1 && y0 == y1) {
        break;
      }

      int16_t e2 = err * 2;

      if (e2 >= dy) {
        err += dy;
        x0 += sx;
      }

      if (e2 <= dx) {
        err += dx;
        y0 += sy;
      }
    }
  }


  void rect(
      int16_t x,
      int16_t y,
      int16_t w,
      int16_t h,
      bool on = true) {

    if (w <= 0 || h <= 0) return;

    hline(x, y, w, on);
    hline(x, y + h - 1, w, on);
    vline(x, y, h, on);
    vline(x + w - 1, y, h, on);
  }


  void fillRect(
      int16_t x,
      int16_t y,
      int16_t w,
      int16_t h,
      bool on = true) {

    if (w <= 0 || h <= 0) return;

    for (int16_t xx = 0; xx < w; ++xx) {
      vline(x + xx, y, h, on);
    }
  }


  void circle(
      int16_t cx,
      int16_t cy,
      int16_t r,
      bool on = true) {

    if (r < 0) return;

    int16_t x = r;
    int16_t y = 0;
    int16_t err = 0;

    while (x >= y) {

      pixel(cx + x, cy + y, on);
      pixel(cx + y, cy + x, on);
      pixel(cx - y, cy + x, on);
      pixel(cx - x, cy + y, on);
      pixel(cx - x, cy - y, on);
      pixel(cx - y, cy - x, on);
      pixel(cx + y, cy - x, on);
      pixel(cx + x, cy - y, on);

      ++y;

      if (err <= 0) {
        err += 2 * y + 1;
      } else {
        --x;
        err += 2 * (y - x) + 1;
      }
    }
  }


  void fillCircle(
      int16_t cx,
      int16_t cy,
      int16_t r,
      bool on = true) {

    if (r <= 0) {
      pixel(cx, cy, on);
      return;
    }

    for (int16_t y = -r; y <= r; ++y) {

      int32_t yy = (int32_t)y * y;
      int16_t dx =
        (int16_t)sqrt(
          (double)((int32_t)r * r - yy)
        );

      hline(cx - dx, cy + y, dx * 2 + 1, on);
    }
  }


  void roundRect(
      int16_t x,
      int16_t y,
      int16_t w,
      int16_t h,
      int16_t r,
      bool on = true) {

    if (w <= 0 || h <= 0) return;

    if (r < 0) r = 0;

    int16_t maxR = w / 2;

    if (h / 2 < maxR) {
      maxR = h / 2;
    }

    if (r > maxR) {
      r = maxR;
    }

    if (r == 0) {
      rect(x, y, w, h, on);
      return;
    }

    hline(x + r, y, w - 2 * r, on);
    hline(x + r, y + h - 1, w - 2 * r, on);

    vline(x, y + r, h - 2 * r, on);
    vline(x + w - 1, y + r, h - 2 * r, on);

    int16_t px = r;
    int16_t py = 0;
    int16_t err = 0;

    while (px >= py) {

      pixel(x + r - px, y + r - py, on);
      pixel(x + r - py, y + r - px, on);

      pixel(x + w - 1 - r + px, y + r - py, on);
      pixel(x + w - 1 - r + py, y + r - px, on);

      pixel(x + r - px, y + h - 1 - r + py, on);
      pixel(x + r - py, y + h - 1 - r + px, on);

      pixel(x + w - 1 - r + px, y + h - 1 - r + py, on);
      pixel(x + w - 1 - r + py, y + h - 1 - r + px, on);

      ++py;

      if (err <= 0) {
        err += 2 * py + 1;
      } else {
        --px;
        err += 2 * (py - px) + 1;
      }
    }
  }


  void fillRoundRect(
      int16_t x,
      int16_t y,
      int16_t w,
      int16_t h,
      int16_t r,
      bool on = true) {

    if (w <= 0 || h <= 0) return;

    if (r < 0) r = 0;

    int16_t maxR = w / 2;

    if (h / 2 < maxR) {
      maxR = h / 2;
    }

    if (r > maxR) {
      r = maxR;
    }

    if (r == 0) {
      fillRect(x, y, w, h, on);
      return;
    }

    fillRect(x + r, y, w - 2 * r, h, on);

    for (int16_t yy = 0; yy < r; ++yy) {

      int16_t dy = r - yy - 1;

      int32_t inside =
        (int32_t)r * r -
        (int32_t)dy * dy;

      int16_t dx =
        (int16_t)sqrt((double)inside);

      int16_t left = r - dx;
      int16_t width =
        w - 2 * left;

      hline(x + left, y + yy, width, on);
      hline(x + left, y + h - 1 - yy, width, on);
    }

    fillRect(
      x,
      y + r,
      r,
      h - 2 * r,
      on
    );

    fillRect(
      x + w - r,
      y + r,
      r,
      h - 2 * r,
      on
    );
  }


  void triangle(
      int16_t x0,
      int16_t y0,
      int16_t x1,
      int16_t y1,
      int16_t x2,
      int16_t y2,
      bool on = true) {

    line(x0, y0, x1, y1, on);
    line(x1, y1, x2, y2, on);
    line(x2, y2, x0, y0, on);
  }


  /*
   * Scanline triangle fill.
   * Handles flat-top, flat-bottom and degenerate triangles.
   * No divide-by-zero paths.
   */
  void fillTriangle(
      int16_t x0,
      int16_t y0,
      int16_t x1,
      int16_t y1,
      int16_t x2,
      int16_t y2,
      bool on = true) {

    if (y0 > y1) {
      int16_t t = y0; y0 = y1; y1 = t;
      t = x0; x0 = x1; x1 = t;
    }

    if (y1 > y2) {
      int16_t t = y1; y1 = y2; y2 = t;
      t = x1; x1 = x2; x2 = t;
    }

    if (y0 > y1) {
      int16_t t = y0; y0 = y1; y1 = t;
      t = x0; x0 = x1; x1 = t;
    }

    if (y0 == y2) {

      int16_t minX = x0;
      int16_t maxX = x0;

      if (x1 < minX) minX = x1;
      if (x2 < minX) minX = x2;

      if (x1 > maxX) maxX = x1;
      if (x2 > maxX) maxX = x2;

      hline(minX, y0, maxX - minX + 1, on);
      return;
    }

    if (y1 != y0) {

      for (int16_t y = y0; y <= y1; ++y) {

        int32_t xa =
          x0 +
          (int32_t)(x1 - x0) *
          (y - y0) /
          (y1 - y0);

        int32_t xb =
          x0 +
          (int32_t)(x2 - x0) *
          (y - y0) /
          (y2 - y0);

        if (xa > xb) {
          int32_t t = xa; xa = xb; xb = t;
        }

        hline(
          (int16_t)xa,
          y,
          (int16_t)(xb - xa + 1),
          on
        );
      }
    }

    if (y2 != y1) {

      for (int16_t y = y1 + 1; y <= y2; ++y) {

        int32_t xa =
          x1 +
          (int32_t)(x2 - x1) *
          (y - y1) /
          (y2 - y1);

        int32_t xb =
          x0 +
          (int32_t)(x2 - x0) *
          (y - y0) /
          (y2 - y0);

        if (xa > xb) {
          int32_t t = xa; xa = xb; xb = t;
        }

        hline(
          (int16_t)xa,
          y,
          (int16_t)(xb - xa + 1),
          on
        );
      }
    }
  }


  // --------------------------------------------------------------------------
  // Text
  // --------------------------------------------------------------------------

  void char5x7(
      int16_t x,
      int16_t y,
      char c,
      bool on = true,
      uint8_t scale = 1) {

    if (scale == 0) scale = 1;

    if (c >= 'a' && c <= 'z') {
      c -= ('a' - 'A');
    }

    if (c < 32 || c > 95) {
      c = ' ';
    }

    uint8_t index = (uint8_t)c - 32;

    for (uint8_t col = 0; col < 5; ++col) {

      uint8_t bits =
        pgm_read_byte(&FONT5X7[index][col]);

      for (uint8_t row = 0; row < 7; ++row) {

        if (bits & (1U << row)) {

          if (scale == 1) {
            pixel(x + col, y + row, on);
          } else {
            fillRect(
              x + col * scale,
              y + row * scale,
              scale,
              scale,
              on
            );
          }
        }
      }
    }
  }


  uint16_t textWidth(
      const char *text,
      uint8_t scale = 1) const {

    if (!text) return 0;

    uint16_t len = strlen(text);

    if (len == 0) return 0;

    return
      (uint16_t)(len * 6UL * scale - scale);
  }


  void text(
      int16_t x,
      int16_t y,
      const char *str,
      bool on = true,
      uint8_t scale = 1) {

    if (!str) return;

    while (*str) {

      char5x7(
        x,
        y,
        *str,
        on,
        scale
      );

      x += 6 * scale;
      ++str;
    }
  }


  void centeredText(
      int16_t y,
      const char *str,
      bool on = true,
      uint8_t scale = 1) {

    if (!str) return;

    uint16_t width =
      textWidth(str, scale);

    int16_t x =
      (OLED_WIDTH - width) / 2;

    if (x < 0) x = 0;

    text(x, y, str, on, scale);
  }


  // --------------------------------------------------------------------------
  // Framebuffer upload
  // --------------------------------------------------------------------------

  bool display() {

    if (!connected) {
      return false;
    }

    for (uint8_t page = 0; page < OLED_PAGES; ++page) {

      if (!command(0xB0 | page)) {
        return false;
      }

      // Column = 0.
      if (!command(0x00)) {
        return false;
      }

      if (!command(0x10)) {
        return false;
      }

      uint16_t base =
        (uint16_t)page * OLED_WIDTH;

      for (
        uint8_t column = 0;
        column < OLED_WIDTH;
        column += OLED_I2C_CHUNK
      ) {

        if (!writeChunk(
              page,
              column,
              &framebuffer[base + column],
              OLED_I2C_CHUNK)) {

          return false;
        }
      }
    }

    return true;
  }
};


// ============================================================================
// HELIO PROFILE ENUMERATION
// ============================================================================

enum Profile {

  PROFILE_BOOTING = 0,
  PROFILE_IDLE,
  PROFILE_INFO,
  PROFILE_SUCCESS,
  PROFILE_ERROR,
  PROFILE_GOAL_RECEIVED,
  PROFILE_NAVIGATED,
  PROFILE_GOAL_REACHED,
  PROFILE_NAVIGATION_ERROR,
  PROFILE_SPEAKING,
  PROFILE_NOT_SPEAKING
};


// ============================================================================
// PROFILE MANAGER
// ============================================================================

class HelioUI {

private:

  SSD1306Nano &oled;

  Profile profile;

  uint8_t frame;
  uint8_t phase;

  unsigned long lastFrame;
  unsigned long enteredAt;

  bool dirty;


  void header(
      const char *title,
      uint8_t number) {

    // Tiny HELIO mark.
    oled.circle(7, 7, 4, true);
    oled.pixel(7, 7, false);

    oled.text(15, 3, "HELIO", true, 1);

    // Profile number at right.
    char num[4];

    num[0] = '0' + (number / 10);
    num[1] = '0' + (number % 10);
    num[2] = '\0';

    oled.text(111, 3, num, true, 1);

    // Header separator.
    oled.hline(0, 13, 128, true);

    // Small title below.
    oled.text(4, 17, title, true, 1);
  }


  void footer(const char *status) {

    oled.hline(0, 54, 128, true);

    oled.text(4, 57, status, true, 1);
  }


  void drawHelioEyes(
      int16_t leftX,
      int16_t rightX,
      int16_t y,
      uint8_t openness) {

    /*
     * openness:
     *   0 = closed
     *   1 = almost closed
     *   2 = normal
     *   3 = wide
     */

    int16_t w = 25;

    if (openness == 0) {

      oled.hline(leftX, y + 7, w, true);
      oled.hline(rightX, y + 7, w, true);

      return;
    }

    int16_t h;

    if (openness == 1) h = 5;
    else if (openness == 3) h = 17;
    else h = 13;

    oled.fillRoundRect(
      leftX,
      y,
      w,
      h,
      6,
      true
    );

    oled.fillRoundRect(
      rightX,
      y,
      w,
      h,
      6,
      true
    );

    // Eye highlights / pupils.
    if (h >= 13) {

      oled.fillCircle(
        leftX + 13,
        y + h / 2,
        3,
        false
      );

      oled.fillCircle(
        rightX + 13,
        y + h / 2,
        3,
        false
      );
    }
  }


  void drawListeningDot(
      uint8_t active) {

    for (uint8_t i = 0; i < 3; ++i) {

      uint8_t r =
        (i == active) ? 2 : 1;

      oled.fillCircle(
        103 + i * 7,
        48,
        r,
        true
      );
    }
  }


  void drawTargetIcon(
      int16_t cx,
      int16_t cy,
      uint8_t pulse) {

    uint8_t r1 = 7 + pulse;
    uint8_t r2 = 3 + pulse / 2;

    oled.circle(cx, cy, r1, true);
    oled.circle(cx, cy, r2, true);

    oled.hline(cx - r1 - 4, cy, 5, true);
    oled.hline(cx + r1, cy, 5, true);
    oled.vline(cx, cy - r1 - 4, 5, true);
    oled.vline(cx, cy + r1, 5, true);

    oled.fillCircle(cx, cy, 1, true);
  }


  void drawRover(
      int16_t cx,
      int16_t cy,
      bool moving) {

    // Body.
    oled.fillRoundRect(
      cx - 13,
      cy - 7,
      26,
      14,
      4,
      true
    );

    // Front sensor.
    oled.fillRoundRect(
      cx + 5,
      cy - 3,
      5,
      6,
      2,
      false
    );

    // Wheels.
    oled.fillCircle(cx - 9, cy + 7, 4, true);
    oled.fillCircle(cx + 9, cy + 7, 4, true);

    if (moving) {

      oled.hline(cx - 20, cy + 12, 5, true);
      oled.hline(cx + 15, cy + 12, 5, true);
    }
  }


  void drawPath(
      int16_t x0,
      int16_t y0,
      int16_t x1,
      int16_t y1,
      uint8_t progress) {

    int16_t px =
      x0 + ((x1 - x0) * progress) / 100;

    int16_t py =
      y0 + ((y1 - y0) * progress) / 100;

    oled.line(x0, y0, px, py, true);

    // Moving dot.
    oled.fillCircle(px, py, 2, true);
  }


  void renderBooting() {

    oled.clear();

    header("STARTING", 1);

    // Central Helio orbital mark.
    oled.circle(64, 35, 15, true);
    oled.circle(64, 35, 10, true);

    oled.fillCircle(64, 35, 3, true);

    // Orbiting dot.
    const int8_t ox[8] =
      {0, 8, 14, 8, 0, -8, -14, -8};

    const int8_t oy[8] =
      {-15, -8, 0, 8, 15, 8, 0, -8};

    oled.fillCircle(
      64 + ox[frame & 7],
      35 + oy[frame & 7],
      2,
      true
    );

    footer("INITIALIZING SYSTEM");

    oled.display();
  }


  void renderIdle() {

    oled.clear();

    header("STANDBY", 2);

    /*
     * Slow breathing animation.
     * The eye state remains calm instead of looking like a
     * terminal/ASCII face.
     */
    uint8_t openness;

    if (frame == 0) openness = 1;
    else if (frame == 1) openness = 0;
    else if (frame == 2) openness = 1;
    else openness = 2;

    drawHelioEyes(
      25,
      78,
      27,
      openness
    );

    // Listening indicator.
    drawListeningDot(
      (millis() / 450UL) % 3
    );

    footer("READY / LISTENING");

    oled.display();
  }


  void renderInfo() {

    oled.clear();

    header("SYSTEM", 3);

    // Three compact diagnostic cards.
    oled.roundRect(5, 27, 36, 21, 4, true);
    oled.roundRect(46, 27, 36, 21, 4, true);
    oled.roundRect(87, 27, 36, 21, 4, true);

    oled.centeredText(30, "I2C", true, 1);
    oled.centeredText(38, "400K", true, 1);

    oled.text(52, 30, "CPU", true, 1);
    oled.text(51, 38, "16M", true, 1);

    oled.text(93, 30, "RAM", true, 1);
    oled.text(93, 38, "1K", true, 1);

    footer("SSD1306 / NANO");

    oled.display();
  }


  void renderSuccess() {

    oled.clear();

    header("SYSTEM READY", 4);

    // Checkmark inside a clean circular confirmation.
    oled.circle(64, 35, 15, true);
    oled.circle(64, 35, 12, true);

    oled.line(56, 35, 62, 41, true);
    oled.line(62, 41, 74, 28, true);

    footer("ALL SYSTEMS NOMINAL");

    oled.display();
  }


  void renderError() {

    oled.clear();

    header("SYSTEM ERROR", 5);

    // Warning diamond.
    oled.line(64, 23, 82, 47, true);
    oled.line(82, 47, 46, 47, true);
    oled.line(46, 47, 64, 23, true);

    oled.vline(64, 30, 9, true);
    oled.fillCircle(64, 43, 1, true);

    footer("CHECK I2C / DISPLAY");

    oled.display();
  }


  void renderGoalReceived() {

    oled.clear();

    header("TARGET LOCK", 6);

    uint8_t pulse = frame & 3;

    drawTargetIcon(
      64,
      37,
      pulse
    );

    // Four small corner brackets.
    oled.hline(36, 25, 7, true);
    oled.vline(36, 25, 7, true);

    oled.hline(85, 25, 7, true);
    oled.vline(91, 25, 7, true);

    oled.hline(36, 49, 7, true);
    oled.vline(36, 43, 7, true);

    oled.hline(85, 49, 7, true);
    oled.vline(91, 43, 7, true);

    footer("TARGET RECEIVED");

    oled.display();
  }


  void renderNavigated() {

    oled.clear();

    header("NAVIGATION", 7);

    // Map-like path.
    oled.roundRect(
      6, 25,
      116, 26,
      4,
      true
    );

    // Clear inside of map.
    oled.fillRoundRect(
      8, 27,
      112, 22,
      3,
      false
    );

    // Dotted route.
    oled.hline(18, 39, 20, true);
    oled.hline(46, 39, 17, true);
    oled.hline(71, 39, 18, true);

    // Rover travels along route.
    int16_t roverX =
      20 + (frame * 14);

    if (roverX > 88) {
      roverX = 20;
    }

    drawRover(
      roverX,
      32,
      true
    );

    // Destination.
    drawTargetIcon(
      105,
      39,
      0
    );

    footer("MOVING TO TARGET");

    oled.display();
  }


  void renderGoalReached() {

    oled.clear();

    header("MISSION COMPLETE", 8);

    // Expanding ring.
    uint8_t radius =
      8 + (frame & 3) * 2;

    oled.circle(64, 36, radius, true);

    // Check mark.
    oled.line(53, 36, 61, 44, true);
    oled.line(61, 44, 77, 27, true);

    // Small satellite dots.
    oled.fillCircle(64, 21, 1, true);
    oled.fillCircle(45, 36, 1, true);
    oled.fillCircle(83, 36, 1, true);

    footer("TARGET REACHED");

    oled.display();
  }


  void renderNavigationError() {

    oled.clear();

    header("NAVIGATION", 9);

    // Pulsing warning frame.
    if (frame & 1) {
      oled.rect(2, 22, 124, 30, true);
    } else {
      oled.roundRect(2, 22, 124, 30, 5, true);
    }

    // Exclamation warning.
    oled.fillTriangle(
      64, 26,
      81, 47,
      47, 47,
      true
    );

    oled.vline(64, 33, 8, false);
    oled.fillCircle(64, 44, 1, false);

    footer("PATH BLOCKED / ABORTED");

    oled.display();
  }


  void renderSpeaking() {

    oled.clear();

    header("VOICE OUTPUT", 10);

    /*
     * Deterministic 7-column waveform.
     * Frame changes the waveform but never writes outside the display.
     */
    static const uint8_t waveform[8][7] PROGMEM = {

      { 4,  9, 15, 21, 15,  9,  4 },
      { 8, 15, 21, 11,  6, 14,  8 },
      { 14, 22, 10,  6, 17, 22, 11 },
      { 20, 11,  5, 17, 23,  9, 16 },
      { 10,  5, 18, 23, 11,  6, 20 },
      {  6, 17, 23, 10,  5, 18, 13 },
      { 18, 23,  9,  5, 17, 21,  7 },
      {  9,  6, 16, 22, 10,  5, 18 }
    };

    for (uint8_t i = 0; i < 7; ++i) {

      uint8_t h =
        pgm_read_byte(
          &waveform[frame & 7][i]
        );

      int16_t x =
        26 + i * 12;

      int16_t y =
        44 - h;

      oled.fillRoundRect(
        x,
        y,
        7,
        h,
        3,
        true
      );
    }

    footer("SPEAKING");

    oled.display();
  }


  void renderNotSpeaking() {

    oled.clear();

    header("VOICE", 11);

    // Calm listening eyes.
    drawHelioEyes(
      25,
      78,
      28,
      2
    );

    // Small microphone/listening symbol.
    oled.roundRect(
      58, 45,
      12, 7,
      3,
      true
    );

    oled.vline(64, 48, 7, true);
    oled.hline(59, 55, 11, true);

    footer("VOICE READY");

    oled.display();
  }


public:

  HelioUI(SSD1306Nano &display) :
    oled(display),
    profile(PROFILE_BOOTING),
    frame(0),
    phase(0),
    lastFrame(0),
    enteredAt(0),
    dirty(true) {}


  void setProfile(Profile p) {

    profile = p;
    frame = 0;
    phase = 0;

    enteredAt = millis();
    lastFrame = 0;
    dirty = true;

    oled.invert(false);

    /*
     * Render immediately when a new profile arrives.
     * This avoids the UI appearing one animation interval late.
     */
    render();
  }


  Profile currentProfile() const {
    return profile;
  }


  void render() {

    switch (profile) {

      case PROFILE_BOOTING:
        renderBooting();
        break;

      case PROFILE_IDLE:
        renderIdle();
        break;

      case PROFILE_INFO:
        renderInfo();
        break;

      case PROFILE_SUCCESS:
        renderSuccess();
        break;

      case PROFILE_ERROR:
        renderError();
        break;

      case PROFILE_GOAL_RECEIVED:
        renderGoalReceived();
        break;

      case PROFILE_NAVIGATED:
        renderNavigated();
        break;

      case PROFILE_GOAL_REACHED:
        renderGoalReached();
        break;

      case PROFILE_NAVIGATION_ERROR:
        renderNavigationError();
        break;

      case PROFILE_SPEAKING:
        renderSpeaking();
        break;

      case PROFILE_NOT_SPEAKING:
        renderNotSpeaking();
        break;
    }

    dirty = false;
  }


  void update() {

    unsigned long now = millis();

    uint16_t interval;

    switch (profile) {

      case PROFILE_BOOTING:
        interval = 90;
        break;

      case PROFILE_GOAL_RECEIVED:
        interval = 130;
        break;

      case PROFILE_NAVIGATED:
        interval = 180;
        break;

      case PROFILE_GOAL_REACHED:
        interval = 180;
        break;

      case PROFILE_NAVIGATION_ERROR:
        interval = 350;
        break;

      case PROFILE_SPEAKING:
        interval = 90;
        break;

      case PROFILE_IDLE:
        interval = 180;
        break;

      default:
        interval = 500;
        break;
    }

    if (now - lastFrame >= interval) {

      lastFrame = now;

      ++frame;

      render();
    }
  }
};


// ============================================================================
// GLOBALS
// ============================================================================

SSD1306Nano oled;
HelioUI helio(oled);

char serialBuffer[SERIAL_BUFFER_SIZE];
uint8_t serialIndex = 0;


// ============================================================================
// SERIAL COMMAND PARSER
// ============================================================================

bool containsProfile(
    const char *value,
    const char *profileName) {

  return strcmp(value, profileName) == 0;
}


void parseCommand(
    const char *json) {

  if (!json || !json[0]) {
    return;
  }

  Serial.print(F("RX: "));
  Serial.println(json);

  const char *key =
    strstr(json, "\"profile\"");

  if (!key) {
    key = strstr(json, "profile");
  }

  if (!key) {
    Serial.println(F("ERR: no profile"));
    return;
  }

  const char *colon =
    strchr(key, ':');

  if (!colon) {
    Serial.println(F("ERR: malformed profile"));
    return;
  }

  const char *value =
    colon + 1;

  while (*value == ' ' ||
         *value == '\t' ||
         *value == '"' ||
         *value == '\'') {
    ++value;
  }

  char profileName[32];
  uint8_t i = 0;

  while (
      value[i] &&
      value[i] != '"' &&
      value[i] != '\'' &&
      value[i] != ',' &&
      value[i] != '}' &&
      value[i] != ' ' &&
      value[i] != '\t' &&
      i < sizeof(profileName) - 1) {

    profileName[i] = value[i];
    ++i;
  }

  profileName[i] = '\0';


  if (containsProfile(profileName, "booting")) {

    helio.setProfile(PROFILE_BOOTING);

  } else if (containsProfile(profileName, "idle")) {

    helio.setProfile(PROFILE_IDLE);

  } else if (containsProfile(profileName, "info")) {

    helio.setProfile(PROFILE_INFO);

  } else if (containsProfile(profileName, "success")) {

    helio.setProfile(PROFILE_SUCCESS);

  } else if (containsProfile(profileName, "error")) {

    helio.setProfile(PROFILE_ERROR);

  } else if (containsProfile(profileName, "goal_received")) {

    helio.setProfile(PROFILE_GOAL_RECEIVED);

  } else if (containsProfile(profileName, "navigated")) {

    helio.setProfile(PROFILE_NAVIGATED);

  } else if (containsProfile(profileName, "goal_reached")) {

    helio.setProfile(PROFILE_GOAL_REACHED);

  } else if (containsProfile(profileName, "navigation_error")) {

    helio.setProfile(PROFILE_NAVIGATION_ERROR);

  } else if (containsProfile(profileName, "is_speaking")) {

    helio.setProfile(PROFILE_SPEAKING);

  } else if (containsProfile(profileName, "is_not_speaking")) {

    helio.setProfile(PROFILE_NOT_SPEAKING);

  } else {

    Serial.print(F("ERR: unknown profile: "));
    Serial.println(profileName);
  }
}


// ============================================================================
// SERIAL HELP
// ============================================================================

void printHelp() {

  Serial.println();
  Serial.println(F("HELIO OLED COMMANDS"));
  Serial.println(F("-------------------"));

  Serial.println(F("{\"profile\":\"booting\"}"));
  Serial.println(F("{\"profile\":\"idle\"}"));
  Serial.println(F("{\"profile\":\"info\"}"));
  Serial.println(F("{\"profile\":\"success\"}"));
  Serial.println(F("{\"profile\":\"error\"}"));
  Serial.println(F("{\"profile\":\"goal_received\"}"));
  Serial.println(F("{\"profile\":\"navigated\"}"));
  Serial.println(F("{\"profile\":\"goal_reached\"}"));
  Serial.println(F("{\"profile\":\"navigation_error\"}"));
  Serial.println(F("{\"profile\":\"is_speaking\"}"));
  Serial.println(F("{\"profile\":\"is_not_speaking\"}"));

  Serial.println();
}


// ============================================================================
// SETUP
// ============================================================================

void setup() {

  Serial.begin(115200);

  delay(100);

  Serial.println();
  Serial.println(F("================================"));
  Serial.println(F(" HELIO OLED CONTROLLER"));
  Serial.println(F(" Arduino Nano / ATmega328P"));
  Serial.println(F(" SSD1306 128x64 / I2C"));
  Serial.println(F(" No display libraries"));
  Serial.println(F("================================"));

  oled.begin();

  if (oled.isConnected()) {

    Serial.println(F("OLED: I2C OK"));

  } else {

    Serial.println(F("OLED: NOT DETECTED"));
    Serial.println(F("SDA=A4 / SCL=A5 / ADDR=0x3C"));
  }

  printHelp();

  helio.setProfile(PROFILE_BOOTING);
}


// ============================================================================
// LOOP
// ============================================================================

void loop() {

  while (Serial.available() > 0) {

    char c =
      (char)Serial.read();

    if (c == '\n' || c == '\r') {

      if (serialIndex > 0) {

        serialBuffer[serialIndex] = '\0';

        parseCommand(serialBuffer);

        serialIndex = 0;
      }

    } else {

      if (serialIndex <
          SERIAL_BUFFER_SIZE - 1) {

        serialBuffer[serialIndex++] = c;

      } else {

        /*
         * Never allow a malformed/oversized serial packet
         * to overwrite the buffer.
         */
        serialIndex = 0;

        Serial.println(
          F("ERR: command too long")
        );
      }
    }
  }

  helio.update();
}