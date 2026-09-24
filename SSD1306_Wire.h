#ifndef SSD1306_WIRE_H
#define SSD1306_WIRE_H

#include <Arduino.h>
#include <Wire.h>
#include "Font.h"

#define OLED_I2C_ADDR 0x3C
#define OLED_WIDTH    128
#define OLED_HEIGHT   64

class SSD1306Wire {
private:
    uint8_t buffer[OLED_WIDTH * OLED_HEIGHT / 8];
    bool invertedMode;

    void sendCommand(uint8_t command) {
        Wire.beginTransmission(OLED_I2C_ADDR);
        Wire.write(0x00); // Command byte
        Wire.write(command);
        Wire.endTransmission();
    }

public:
    SSD1306Wire() : invertedMode(false) {}

    void begin() {
        Wire.begin();
        Wire.setClock(400000L); // 400kHz Fast I2C
        delay(50);

        // Hardware initialization sequence
        sendCommand(0xAE); // Display OFF
        sendCommand(0xD5); sendCommand(0x80); // Display clock divide
        sendCommand(0xA8); sendCommand(0x3F); // Multiplex ratio (64MUX)
        sendCommand(0xD3); sendCommand(0x00); // Display offset
        sendCommand(0x40);                     // Start line #0
        sendCommand(0x8D); sendCommand(0x14); // Enable charge pump
        sendCommand(0x20); sendCommand(0x00); // Horizontal Addressing Mode
        sendCommand(0xA1); // Column remap
        sendCommand(0xC8); // COM scan direction
        sendCommand(0xDA); sendCommand(0x12); // COM pins hardware config
        sendCommand(0x81); sendCommand(0xCF); // Contrast control
        sendCommand(0xD9); sendCommand(0xF1); // Pre-charge period
        sendCommand(0xDB); sendCommand(0x40); // VCOMH deselect level
        sendCommand(0xA4);                     // Display ON RAM
        sendCommand(0xA6);                     // Normal mode
        sendCommand(0xAF); // Display ON

        clear();
        display();
    }

    void clear() {
        memset(buffer, 0x00, sizeof(buffer));
    }

    void fillScreen() {
        memset(buffer, 0xFF, sizeof(buffer));
    }

    void setInvertDisplay(bool invert) {
        invertedMode = invert;
        sendCommand(invert ? 0xA7 : 0xA6);
    }

    void setPixel(int16_t x, int16_t y, uint8_t color = 1) {
        if (x < 0 || x >= OLED_WIDTH || y < 0 || y >= OLED_HEIGHT) return;
        uint16_t index = x + (y / 8) * OLED_WIDTH;
        if (color) {
            buffer[index] |= (1 << (y % 8));
        } else {
            buffer[index] &= ~(1 << (y % 8));
        }
    }

    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint8_t color = 1) {
        for (int16_t i = 0; i < w; i++) setPixel(x + i, y, color);
    }

    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint8_t color = 1) {
        for (int16_t i = 0; i < h; i++) setPixel(x, y + i, color);
    }

    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color = 1) {
        drawFastHLine(x, y, w, color);
        drawFastHLine(x, y + h - 1, w, color);
        drawFastVLine(x, y, h, color);
        drawFastVLine(x + w - 1, y, h, color);
    }

    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color = 1) {
        for (int16_t i = 0; i < w; i++) {
            for (int16_t j = 0; j < h; j++) setPixel(x + i, y + j, color);
        }
    }

    // Missing helper: drawLine (Bresenham's line algorithm)
    void drawLine(int x0, int y0, int x1, int y1, uint8_t color = 1) {
        int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2;
        while (true) {
            setPixel(x0, y0, color);
            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }

    // Missing helper: fillRoundRect (used for biological eyes & chat bubbles)
    void fillRoundRect(int x, int y, int w, int h, int r, uint8_t color = 1) {
        for (int i = x; i < x + w; i++) {
            for (int j = y; j < y + h; j++) {
                if ((i < x + r && j < y + r && (x + r - i)*(x + r - i) + (y + r - j)*(y + r - j) > r*r) ||
                    (i >= x + w - r && j < y + r && (i - (x + w - r - 1))*(i - (x + w - r - 1)) + (y + r - j)*(y + r - j) > r*r) ||
                    (i < x + r && j >= y + h - r && (x + r - i)*(x + r - i) + (j - (y + h - r - 1))*(j - (y + h - r - 1)) > r*r) ||
                    (i >= x + w - r && j >= y + h - r && (i - (x + w - r - 1))*(i - (x + w - r - 1)) + (j - (y + h - r - 1))*(j - (y + h - r - 1)) > r*r)) {
                    continue;
                }
                setPixel(i, j, color);
            }
        }
    }

    // Missing helper: fillTriangle (used for speech bubble tails & thinking indicators)
    void fillTriangle(int x0, int0, int x1, int y1, int x2, int y2, uint8_t color = 1) {
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

    void drawChar(int16_t x, int16_t y, char c, uint8_t color = 1, uint8_t scale = 1) {
        if (c < 32 || c > 95) {
            if (c >= 'a' && c <= 'z') c -= 32;
            else c = ' ';
        }
        uint8_t charIndex = c - 32;

        for (uint8_t col = 0; col < 8; col++) {
            uint8_t line = pgm_read_byte(&(font8x8[charIndex][col]));
            for (uint8_t row = 0; row < 8; row++) {
                if (line & (1 << row)) {
                    if (scale == 1) {
                        setPixel(x + col, y + row, color);
                    } else {
                        for (uint8_t sx = 0; sx < scale; sx++) {
                            for (uint8_t sy = 0; sy < scale; sy++) {
                                setPixel(x + (col * scale) + sx, y + (row * scale) + sy, color);
                            }
                        }
                    }
                }
            }
        }
    }

    void drawString(int16_t x, int16_t y, const char *str, uint8_t color = 1, uint8_t scale = 1) {
        int16_t curX = x;
        while (*str) {
            drawChar(curX, y, *str, color, scale);
            curX += (8 * scale);
            str++;
        }
    }

    // Missing helper: drawString5x7 (compatibility wrapper for Profiles.h)
    void drawString5x7(int x, int y, const char* text, uint8_t color = 1) {
        drawString(x, y, text, color, 1);
    }

    // PROGMEM F() string overload
    void drawString(int16_t x, int16_t y, const __FlashStringHelper *ifsh, uint8_t color = 1, uint8_t scale = 1) {
        PGM_P p = reinterpret_cast<PGM_P>(ifsh);
        int16_t curX = x;
        while (1) {
            unsigned char c = pgm_read_byte(p++);
            if (!c) break;
            drawChar(curX, y, c, color, scale);
            curX += (8 * scale);
        }
    }

    void drawCenteredString(int16_t y, const char *str, uint8_t color = 1, uint8_t scale = 1) {
        uint8_t len = strlen(str);
        int16_t totalWidth = len * 8 * scale;
        int16_t x = (OLED_WIDTH - totalWidth) / 2;
        if (x < 0) x = 0;
        drawString(x, y, str, color, scale);
    }

    // PROGMEM F() string overload
    void drawCenteredString(int16_t y, const __FlashStringHelper *ifsh, uint8_t color = 1, uint8_t scale = 1) {
        PGM_P p = reinterpret_cast<PGM_P>(ifsh);
        uint8_t len = strlen_P(p);
        int16_t totalWidth = len * 8 * scale;
        int16_t x = (OLED_WIDTH - totalWidth) / 2;
        if (x < 0) x = 0;
        drawString(x, y, ifsh, color, scale);
    }

    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint8_t color = 1) {
        for (int16_t y = -r; y <= r; y++) {
            for (int16_t x = -r; x <= r; x++) {
                if (x * x + y * y <= r * r) {
                    setPixel(x0 + x, y0 + y, color);
                }
            }
        }
    }

    void display() {
        sendCommand(0x21); sendCommand(0); sendCommand(127);
        sendCommand(0x22); sendCommand(0); sendCommand(7);

        for (uint16_t i = 0; i < sizeof(buffer); i += 16) {
            Wire.beginTransmission(OLED_I2C_ADDR);
            Wire.write(0x40);
            for (uint8_t j = 0; j < 16; j++) {
                Wire.write(buffer[i + j]);
            }
            Wire.endTransmission();
        }
    }
};

#endif // SSD1306_WIRE_H