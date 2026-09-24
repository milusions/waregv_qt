/*
 * =========================================================================
 * ALL-IN-ONE STANDALONE ARDUINO SKETCH FOR OLED PROFILE CONTROLLER
 * - NO EXTERNAL LIBRARIES (Uses Wire.h and avr/pgmspace.h ONLY)
 * - Self-contained Font, Driver, Profiles, and Serial Command Parser
 * =========================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <avr/pgmspace.h>

#define OLED_I2C_ADDR 0x3C
#define OLED_WIDTH    128
#define OLED_HEIGHT   64

// =========================================================================
// 1. STANDARD 8x8 ASCII FONT (Stored in Flash / PROGMEM)
// =========================================================================
const uint8_t PROGMEM font8x8[][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Space (32)
    {0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00}, // !
    {0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // "
    {0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00}, // #
    {0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00}, // $
    {0x00, 0x63, 0x66, 0x0C, 0x18, 0x33, 0x63, 0x00}, // %
    {0x1C, 0x36, 0x1C, 0x3E, 0x6B, 0x6C, 0x37, 0x00}, // &
    {0x06, 0x06, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00}, // '
    {0x0C, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0C, 0x00}, // (
    {0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x18, 0x30, 0x00}, // )
    {0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00}, // *
    {0x00, 0x0C, 0x0C, 0x3E, 0x0C, 0x0C, 0x00, 0x00}, // +
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x18}, // ,
    {0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x00}, // -
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00}, // .
    {0x00, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x00}, // /
    {0x3E, 0x63, 0x6B, 0x6F, 0x73, 0x63, 0x3E, 0x00}, // 0
    {0x0C, 0x1C, 0x0C, 0x0C, 0x0C, 0x0C, 0x3E, 0x00}, // 1
    {0x3E, 0x63, 0x06, 0x1C, 0x30, 0x60, 0x7F, 0x00}, // 2
    {0x3E, 0x63, 0x06, 0x1C, 0x06, 0x63, 0x3E, 0x00}, // 3
    {0x0E, 0x1E, 0x36, 0x66, 0x7F, 0x06, 0x06, 0x00}, // 4
    {0x7F, 0x60, 0x7C, 0x06, 0x06, 0x63, 0x3E, 0x00}, // 5
    {0x1C, 0x30, 0x60, 0x7C, 0x63, 0x63, 0x3E, 0x00}, // 6
    {0x7F, 0x63, 0x06, 0x0C, 0x18, 0x18, 0x18, 0x00}, // 7
    {0x3E, 0x63, 0x63, 0x3E, 0x63, 0x63, 0x3E, 0x00}, // 8
    {0x3E, 0x63, 0x63, 0x3E, 0x03, 0x06, 0x3C, 0x00}, // 9
    {0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00}, // :
    {0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x30, 0x00}, // ;
    {0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00}, // <
    {0x00, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00}, // =
    {0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60, 0x00}, // >
    {0x3E, 0x63, 0x06, 0x0C, 0x18, 0x00, 0x18, 0x00}, // ?
    {0x3C, 0x66, 0x6E, 0x6E, 0x60, 0x62, 0x3C, 0x00}, // @
    {0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00}, // A
    {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00}, // B
    {0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00}, // C
    {0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00}, // D
    {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x7E, 0x00}, // E
    {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x00}, // F
    {0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3B, 0x00}, // G
    {0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00}, // H
    {0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00}, // I
    {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x6C, 0x38, 0x00}, // J
    {0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66, 0x00}, // K
    {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00}, // L
    {0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x00}, // M
    {0x66, 0x76, 0x7E, 0x7E, 0x6E, 0x66, 0x66, 0x00}, // N
    {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00}, // O
    {0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00}, // P
    {0x3C, 0x66, 0x66, 0x66, 0x6A, 0x6C, 0x36, 0x00}, // Q
    {0x7C, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0x66, 0x00}, // R
    {0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00}, // S
    {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, // T
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00}, // U
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00}, // V
    {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00}, // W
    {0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00}, // X
    {0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00}, // Y
    {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E, 0x00}, // Z
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // [
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // \
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // ]
    {0x08, 0x1C, 0x36, 0x22, 0x00, 0x00, 0x00, 0x00}, // ^
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00}  // _
};

// =========================================================================
// 2. ROBUST STANDALONE OLED DRIVER (Universal Page Mode with Offset)
// =========================================================================
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
        Wire.setClock(400000L); // 400kHz Fast Mode
        delay(100);

        sendCommand(0xAE); // Display OFF
        sendCommand(0xD5); sendCommand(0x50); // Clock divide
        sendCommand(0xA8); sendCommand(0x3F); // Multiplex ratio (64)
        sendCommand(0xD3); sendCommand(0x00); // Display offset
        sendCommand(0x40);                     // Start line #0
        sendCommand(0xAD); sendCommand(0x8B); // Charge pump enable
        sendCommand(0xA1); // Column remap
        sendCommand(0xC8); // COM scan direction
        sendCommand(0xDA); sendCommand(0x12); // COM pins config
        sendCommand(0x81); sendCommand(0xCF); // Contrast control
        sendCommand(0xD9); sendCommand(0xF1); // Pre-charge period
        sendCommand(0xDB); sendCommand(0x40); // VCOMH deselect level
        sendCommand(0xA4);                     // Display ON RAM
        sendCommand(0xA6);                     // Normal display mode
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

    void fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint8_t color = 1) {
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

    void drawString5x7(int x, int y, const char* text, uint8_t color = 1) {
        drawString(x, y, text, color, 1);
    }

    void drawString5x7(int x, int y, const __FlashStringHelper* ifsh, uint8_t color = 1) {
        drawString(x, y, ifsh, color, 1);
    }

    void drawCenteredString(int16_t y, const char *str, uint8_t color = 1, uint8_t scale = 1) {
        uint8_t len = strlen(str);
        int16_t totalWidth = len * 8 * scale;
        int16_t x = (OLED_WIDTH - totalWidth) / 2;
        if (x < 0) x = 0;
        drawString(x, y, str, color, scale);
    }

    void drawCenteredString(int16_t y, const __FlashStringHelper *ifsh, uint8_t color = 1, uint8_t scale = 1) {
        uint8_t len = strlen_P(reinterpret_cast<PGM_P>(ifsh));
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

    // Page-by-page rendering with +2 column offset (eliminates garbage display data)
    void display() {
        for (uint8_t page = 0; page < 8; page++) {
            sendCommand(0xB0 + page); // Page address (0 to 7)
            sendCommand(0x02);        // Lower column address start (+2 offset)
            sendCommand(0x10);        // Higher column address start (0)

            for (uint8_t chunk = 0; chunk < 128; chunk += 16) {
                Wire.beginTransmission(OLED_I2C_ADDR);
                Wire.write(0x40); // Data stream header
                for (uint8_t j = 0; j < 16; j++) {
                    Wire.write(buffer[page * 128 + chunk + j]);
                }
                Wire.endTransmission();
            }
        }
    }
};

// =========================================================================
// 3. PROFILE MANAGER & ANIMATIONS
// =========================================================================
enum ProfileType {
    PROFILE_BOOTING,
    PROFILE_IDLE,
    PROFILE_INFO,
    PROFILE_SUCCESS,
    PROFILE_ERROR,
    PROFILE_GOAL_RECEIVED,
    PROFILE_NAVIGATED,
    PROFILE_GOAL_REACHED,
    PROFILE_NAVIGATION_ERROR,
    PROFILE_IS_SPEAKING,
    PROFILE_IS_NOT_SPEAKING
};

class ProfileManager {
private:
    SSD1306Wire &oled;
    ProfileType currentProfile;
    uint8_t animStep;
    unsigned long lastAnimTime;
    unsigned long lastBlinkTime;
    bool eyeOpen;

public:
    ProfileManager(SSD1306Wire &display) 
        : oled(display), currentProfile(PROFILE_BOOTING), animStep(0), lastAnimTime(0), lastBlinkTime(0), eyeOpen(true) {}

    void setProfile(ProfileType profile) {
        currentProfile = profile;
        animStep = 0;
        oled.setInvertDisplay(false);
        oled.clear();
        oled.display();
    }

    ProfileType getCurrentProfile() const {
        return currentProfile;
    }

    void update() {
        unsigned long now = millis();

        switch (currentProfile) {
            case PROFILE_BOOTING:
                if (now - lastAnimTime >= 90) {
                    lastAnimTime = now;
                    renderBootingProfile();
                    animStep = (animStep + 1) % 8;
                }
                break;

            case PROFILE_GOAL_RECEIVED:
                if (now - lastAnimTime >= 120) {
                    lastAnimTime = now;
                    renderGoalReceived();
                    animStep = (animStep + 1) % 6;
                }
                break;

            case PROFILE_NAVIGATED:
                if (now - lastAnimTime >= 150) {
                    lastAnimTime = now;
                    renderNavigated();
                    animStep = (animStep + 1) % 4;
                }
                break;

            case PROFILE_GOAL_REACHED:
                if (now - lastAnimTime >= 200) {
                    lastAnimTime = now;
                    renderGoalReached();
                    animStep = (animStep + 1) % 4;
                }
                break;

            case PROFILE_NAVIGATION_ERROR:
                if (now - lastAnimTime >= 250) {
                    lastAnimTime = now;
                    renderNavigationError();
                    animStep = (animStep + 1) % 2;
                }
                break;

            case PROFILE_IS_SPEAKING:
                if (now - lastAnimTime >= 80) {
                    lastAnimTime = now;
                    renderIsSpeaking();
                    animStep = (animStep + 1) % 8;
                }
                break;

            case PROFILE_IS_NOT_SPEAKING:
            case PROFILE_IDLE:
                if (now - lastBlinkTime >= 3000) {
                    eyeOpen = !eyeOpen;
                    if (eyeOpen) lastBlinkTime = now;
                    else lastBlinkTime = now - 2800;
                }
                renderOrganicIdle(now);
                break;

            case PROFILE_INFO:
                renderInfoProfile();
                break;

            case PROFILE_SUCCESS:
                renderSuccessProfile();
                break;

            case PROFILE_ERROR:
                renderErrorProfile();
                break;
        }
    }

private:
    void drawHeader(const __FlashStringHelper* title) {
        oled.fillRect(0, 0, 128, 12, 1);
        oled.drawCenteredString(2, title, 0, 1);
    }

    void drawFooter(const __FlashStringHelper* subtitle) {
        oled.drawFastHLine(0, 52, 128, 1);
        oled.drawCenteredString(54, subtitle, 1, 1);
    }

    void renderBootingProfile() {
        oled.fillScreen();
        oled.drawCenteredString(14, F("BOOTING"), 0, 2);

        int16_t centerX = 64, centerY = 48;
        const int8_t dx[8] = { 9,  6,  0, -6, -9, -6,  0,  6 };
        const int8_t dy[8] = { 0,  6,  9,  6,  0, -6, -9, -6 };

        for (uint8_t i = 0; i < 8; i++) {
            int16_t px = centerX + dx[i];
            int16_t py = centerY + dy[i];

            if (i == animStep) {
                oled.fillCircle(px, py, 2, 0);
            } else if (i == (animStep + 7) % 8) {
                oled.fillCircle(px, py, 1, 0);
            } else {
                oled.setPixel(px, py, 0);
            }
        }
        oled.display();
    }

    void renderGoalReceived() {
        oled.clear();
        drawHeader(F("STATUS: TARGET"));
        if (animStep % 2 == 0) {
            oled.drawCenteredString(24, F(" (O_O) "), 1, 2);
        } else {
            oled.drawCenteredString(24, F(" [o_o] "), 1, 2);
        }
        drawFooter(F("Target Set"));
        oled.display();
    }

    void renderNavigated() {
        oled.clear();
        drawHeader(F("NAVIGATING"));
        if (animStep == 0) {
            oled.drawCenteredString(24, F(" ( ._.)> "), 1, 2);
        } else if (animStep == 1) {
            oled.drawCenteredString(24, F(" ( •_•)> "), 1, 2);
        } else if (animStep == 2) {
            oled.drawCenteredString(24, F(" ( •_•)>="), 1, 2);
        } else {
            oled.drawCenteredString(24, F(" (•_• )> "), 1, 2);
        }
        drawFooter(F("Moving to target..."));
        oled.display();
    }

    void renderGoalReached() {
        oled.clear();
        drawHeader(F("SUCCESS"));
        if (animStep % 2 == 0) {
            oled.drawCenteredString(22, F(" (^ _ ^) "), 1, 2);
        } else {
            oled.drawCenteredString(26, F(" (^ - ^) "), 1, 2);
        }
        drawFooter(F("Arrived safely"));
        oled.display();
    }

    void renderNavigationError() {
        oled.clear();
        if (animStep == 0) {
            oled.setInvertDisplay(true);
            drawHeader(F("ALERT / ERROR"));
            oled.drawCenteredString(24, F(" (>_<) ! "), 1, 2);
            drawFooter(F("Path Blocked"));
        } else {
            oled.setInvertDisplay(false);
            oled.drawRect(0, 0, 128, 64, 1);
            drawHeader(F("ALERT / ERROR"));
            oled.drawCenteredString(24, F(" (X_X) ! "), 1, 2);
            drawFooter(F("Canceled"));
        }
        oled.display();
    }

    void renderIsSpeaking() {
        oled.clear();
        drawHeader(F("AUDIO OUTPUT"));
        uint8_t heights[8][5] = {
            { 4, 12, 20, 12,  4 },
            { 8, 18, 12, 22,  8 },
            { 16, 8, 24,  8, 16 },
            { 22, 14, 6, 18, 20 },
            { 12, 22, 16, 8, 14 },
            { 6, 16, 22, 14,  8 },
            { 18, 6, 12, 20, 16 },
            { 24, 12, 8, 16, 22 }
        };
        int16_t startX = 36;
        int16_t baseY = 44;
        for (uint8_t i = 0; i < 5; i++) {
            uint8_t h = heights[animStep][i];
            oled.fillRect(startX + (i * 12), baseY - h, 8, h, 1);
        }
        drawFooter(F("Voice Active"));
        oled.display();
    }

    void renderOrganicIdle(unsigned long now) {
        oled.clear();
        drawHeader(F("STANDBY"));

        int y = 24;
        float curW = 20;
        float curH = eyeOpen ? 16 : 4;
        int lX = 32;
        int rX = 76;

        oled.fillRoundRect(lX, y, (int)curW, (int)curH, ((int)curW > 16) ? 8 : 4, 1);
        oled.fillRoundRect(rX, y, (int)curW, (int)curH, ((int)curW > 16) ? 8 : 4, 1);

        if (!eyeOpen) {
            oled.fillRoundRect(lX - 2, y + (int)curH - 6, (int)curW + 4, 10, 4, 0);
            oled.fillRoundRect(rX - 2, y + (int)curH - 6, (int)curW + 4, 10, 4, 0);
        }

        drawIdleChatBubble(now);
        drawFooter(F("Listening..."));
        oled.display();
    }

    void drawIdleChatBubble(unsigned long now) {
        int bx = 84;
        int by = 16;
        int bubbleW = 38;
        int bubbleH = 16;
        int inset = 2;

        oled.fillRoundRect(bx + inset, by + inset, bubbleW - 2*inset, bubbleH - 2*inset, 4, 1);
        oled.fillRoundRect(bx + 2 + inset, by + 2 + inset, bubbleW - 4 - 2*inset, bubbleH - 4 - 2*inset, 2, 0);

        oled.fillTriangle(bx + 8, by + bubbleH - 1, bx + 16, by + bubbleH - 1, bx + 9, by + bubbleH + 5, 1);
        oled.drawLine(bx + 9, by + bubbleH - 2, bx + 9, by + bubbleH + 3, 0);

        int dotSeq = (now / 400) % 3;
        for (int i = 0; i <= dotSeq; i++) {
            oled.fillRoundRect(bx + 10 + i * 7, by + bubbleH / 2 - 2, 3, 3, 1, 1);
        }
    }

    void renderInfoProfile() {
        oled.clear();
        drawHeader(F("SYSTEM INFO"));
        oled.drawString(10, 20, F("CPU: 16MHz"), 1, 1);
        oled.drawString(10, 34, F("I2C: 400kHz"), 1, 1);
        drawFooter(F("ROS2 Connected"));
        oled.display();
    }

    void renderSuccessProfile() {
        oled.clear();
        drawHeader(F("SYSTEM OK"));
        oled.drawCenteredString(24, F(" READY "), 1, 2);
        drawFooter(F("Online"));
        oled.display();
    }

    void renderErrorProfile() {
        oled.setInvertDisplay(true);
        oled.clear();
        drawHeader(F("SYSTEM ERROR"));
        oled.drawCenteredString(24, F(" FAIL "), 1, 2);
        drawFooter(F("Check Wire"));
        oled.display();
    }
};

// =========================================================================
// 4. MAIN ARDUINO SETUP & LOOP
// =========================================================================
SSD1306Wire oled;
ProfileManager profiles(oled);

char serialBuffer[64];
uint8_t bufferIdx = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial) { ; }

    Serial.println(F("--- Standalone OLED Profile Controller Online ---"));

    // Initialize OLED display hardware
    oled.begin();

    // Start with booting profile
    profiles.setProfile(PROFILE_BOOTING);
}

void loop() {
    // 1. Check incoming serial commands
    while (Serial.available() > 0) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            if (bufferIdx > 0) {
                serialBuffer[bufferIdx] = '\0';
                parseJsonCommand(serialBuffer);
                bufferIdx = 0;
            }
        } else if (bufferIdx < sizeof(serialBuffer) - 1) {
            serialBuffer[bufferIdx++] = c;
        }
    }

    // 2. Update current profile animations
    profiles.update();
}

void parseJsonCommand(const char* jsonStr) {
    Serial.print(F("Received: "));
    Serial.println(jsonStr);

    const char* keyPos = strstr(jsonStr, "\"profile\"");
    if (!keyPos) keyPos = strstr(jsonStr, "profile");

    if (keyPos) {
        const char* colonPos = strchr(keyPos, ':');
        if (colonPos) {
            if (strstr(colonPos, "goal_received")) profiles.setProfile(PROFILE_GOAL_RECEIVED);
            else if (strstr(colonPos, "navigated")) profiles.setProfile(PROFILE_NAVIGATED);
            else if (strstr(colonPos, "goal_reached")) profiles.setProfile(PROFILE_GOAL_REACHED);
            else if (strstr(colonPos, "navigation_error")) profiles.setProfile(PROFILE_NAVIGATION_ERROR);
            else if (strstr(colonPos, "is_speaking")) profiles.setProfile(PROFILE_IS_SPEAKING);
            else if (strstr(colonPos, "is_not_speaking")) profiles.setProfile(PROFILE_IS_NOT_SPEAKING);
            else if (strstr(colonPos, "booting")) profiles.setProfile(PROFILE_BOOTING);
            else if (strstr(colonPos, "idle")) profiles.setProfile(PROFILE_IS_NOT_SPEAKING);
            else if (strstr(colonPos, "info")) profiles.setProfile(PROFILE_INFO);
            else if (strstr(colonPos, "success")) profiles.setProfile(PROFILE_SUCCESS);
            else if (strstr(colonPos, "error")) profiles.setProfile(PROFILE_ERROR);
        }
    }
}