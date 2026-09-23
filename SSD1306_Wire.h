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
        Wire.write(0x00);
        Wire.write(command);
        Wire.endTransmission();
    }

public:
    SSD1306Wire() : invertedMode(false) {}

    void begin() {
        Wire.begin();
        Wire.setClock(400000L); // Fast 400kHz I2C

        sendCommand(0xAE);
        sendCommand(0xD5); sendCommand(0x80);
        sendCommand(0xA8); sendCommand(0x3F);
        sendCommand(0xD3); sendCommand(0x00);
        sendCommand(0x40);
        sendCommand(0x8D); sendCommand(0x14);
        sendCommand(0x20); sendCommand(0x00);
        sendCommand(0xA1);
        sendCommand(0xC8);
        sendCommand(0xDA); sendCommand(0x12);
        sendCommand(0x81); sendCommand(0xCF);
        sendCommand(0xD9); sendCommand(0xF1);
        sendCommand(0xDB); sendCommand(0x40);
        sendCommand(0xA4);
        sendCommand(0xA6);
        sendCommand(0xAF);

        clear();
        display();
    }

    void clear() { memset(buffer, 0x00, sizeof(buffer)); }
    void fillScreen() { memset(buffer, 0xFF, sizeof(buffer)); }

    void setInvertDisplay(bool invert) {
        invertedMode = invert;
        sendCommand(invert ? 0xA7 : 0xA6);
    }

    void setPixel(int16_t x, int16_t y, uint8_t color = 1) {
        if (x < 0 || x >= OLED_WIDTH || y < 0 || y >= OLED_HEIGHT) return;
        uint16_t index = x + (y / 8) * OLED_WIDTH;
        if (color) buffer[index] |= (1 << (y % 8));
        else buffer[index] &= ~(1 << (y % 8));
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
                    for (uint8_t sx = 0; sx < scale; sx++) {
                        for (uint8_t sy = 0; sy < scale; sy++) {
                            setPixel(x + (col * scale) + sx, y + (row * scale) + sy, color);
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

    void drawCenteredString(int16_t y, const char *str, uint8_t color = 1, uint8_t scale = 1) {
        int16_t totalWidth = strlen(str) * 8 * scale;
        int16_t x = (OLED_WIDTH - totalWidth) / 2;
        if (x < 0) x = 0;
        drawString(x, y, str, color, scale);
    }

    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint8_t color = 1) {
        for (int16_t y = -r; y <= r; y++) {
            for (int16_t x = -r; x <= r; x++) {
                if (x * x + y * y <= r * r) setPixel(x0 + x, y0 + y, color);
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