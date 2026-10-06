#include <Wire.h>
#include <avr/pgmspace.h>
#include <string.h>

#define OLED_ADDR       0x3C
#define OLED_WIDTH      128
#define OLED_HEIGHT     64
#define OLED_PAGES      8
#define OLED_OFFSET     2 // Set to 2 for SH1106, 0 for SSD1306

// 5x7 Font
const uint8_t PROGMEM FONT[][5] = {
  {0x00,0x00,0x00,0x00,0x00},{0x00,0x00,0x5F,0x00,0x00},{0x00,0x07,0x00,0x07,0x00},{0x14,0x7F,0x14,0x7F,0x14},
  {0x24,0x2A,0x7F,0x2A,0x12},{0x23,0x13,0x08,0x64,0x62},{0x36,0x49,0x55,0x22,0x50},{0x00,0x05,0x03,0x00,0x00},
  {0x00,0x1C,0x22,0x41,0x00},{0x00,0x41,0x22,0x1C,0x00},{0x14,0x08,0x3E,0x08,0x14},{0x08,0x08,0x3E,0x08,0x08},
  {0x00,0x50,0x30,0x00,0x00},{0x08,0x08,0x08,0x08,0x08},{0x00,0x60,0x60,0x00,0x00},{0x20,0x10,0x08,0x04,0x02},
  {0x3E,0x51,0x49,0x45,0x3E},{0x00,0x42,0x7F,0x40,0x00},{0x42,0x61,0x51,0x49,0x46},{0x21,0x41,0x45,0x4B,0x31},
  {0x18,0x14,0x12,0x7F,0x10},{0x27,0x45,0x45,0x45,0x39},{0x3C,0x4A,0x49,0x49,0x30},{0x01,0x71,0x09,0x05,0x03},
  {0x36,0x49,0x49,0x49,0x36},{0x06,0x49,0x49,0x29,0x1E},{0x00,0x36,0x36,0x00,0x00},{0x00,0x56,0x36,0x00,0x00},
  {0x08,0x14,0x22,0x41,0x00},{0x14,0x14,0x14,0x14,0x14},{0x00,0x41,0x22,0x14,0x08},{0x02,0x01,0x51,0x09,0x06},
  {0x32,0x49,0x79,0x41,0x3E},{0x7E,0x11,0x11,0x11,0x7E},{0x7F,0x49,0x49,0x49,0x36},{0x3E,0x41,0x41,0x41,0x22},
  {0x7F,0x41,0x41,0x22,0x1C},{0x7F,0x49,0x49,0x49,0x41},{0x7F,0x09,0x09,0x09,0x01},{0x3E,0x41,0x49,0x49,0x7A},
  {0x7F,0x08,0x08,0x08,0x7F},{0x00,0x41,0x7F,0x41,0x00},{0x20,0x40,0x41,0x3F,0x01},{0x7F,0x08,0x14,0x22,0x41},
  {0x7F,0x40,0x40,0x40,0x40},{0x7F,0x02,0x0C,0x02,0x7F},{0x7F,0x04,0x08,0x10,0x7F},{0x3E,0x41,0x41,0x41,0x3E},
  {0x7F,0x09,0x09,0x09,0x06},{0x3E,0x41,0x51,0x21,0x5E},{0x7F,0x09,0x19,0x29,0x46},{0x46,0x49,0x49,0x49,0x31},
  {0x01,0x01,0x7F,0x01,0x01},{0x3F,0x40,0x40,0x40,0x3F},{0x1F,0x20,0x40,0x20,0x1F},{0x3F,0x40,0x38,0x40,0x3F},
  {0x63,0x14,0x08,0x14,0x63},{0x07,0x08,0x70,0x08,0x07},{0x61,0x51,0x49,0x45,0x43},{0x00,0x7F,0x41,0x41,0x00},
  {0x02,0x04,0x08,0x10,0x20},{0x00,0x41,0x41,0x7F,0x00},{0x04,0x02,0x01,0x02,0x04},{0x40,0x40,0x40,0x40,0x40}
};

enum ActionType { ACTION_NONE, ACTION_LOADER, ACTION_SPINNER };

// ---------------- LEDs ----------------
#define PIN_WARN_LIGHT  11
#define PIN_HEADLIGHT   12

enum LedMode : uint8_t {
  LED_OFF, LED_ON, LED_BLINK_2HZ, LED_BLINK_5HZ, LED_PULSE_3, LED_PULSE_5
};

#define PULSE_ON_MS     100   // one flash: on time
#define PULSE_OFF_MS    100   // one flash: off time
#define PULSE_PAUSE_MS  1000  // pause between bursts

struct Led {
  uint8_t pin;
  LedMode mode;
  unsigned long modeStart;

  void begin(uint8_t p) {
    pin = p;
    pinMode(pin, OUTPUT);
    setMode(LED_OFF);
  }

  void setMode(LedMode m) {
    mode = m;
    modeStart = millis();   // restart pattern phase on every change
  }

  void update() {
    unsigned long t = millis() - modeStart;
    bool on = false;
    switch (mode) {
      case LED_OFF:       on = false; break;
      case LED_ON:        on = true;  break;
      case LED_BLINK_2HZ: on = (t % 500) < 250; break;   // 500 ms period
      case LED_BLINK_5HZ: on = (t % 200) < 100; break;   // 200 ms period
      case LED_PULSE_3:
      case LED_PULSE_5: {
        uint8_t n = (mode == LED_PULSE_3) ? 3 : 5;
        const unsigned long flash = PULSE_ON_MS + PULSE_OFF_MS;
        unsigned long period = n * flash + PULSE_PAUSE_MS;
        unsigned long pt = t % period;
        on = (pt < n * flash) && ((pt % flash) < PULSE_ON_MS);
        break;
      }
    }
    digitalWrite(pin, on ? HIGH : LOW);
  }
};

Led warnLight, headlight;

class Display {
private:
  uint8_t buffer[1024];

  void cmd(uint8_t c) {
    Wire.beginTransmission(OLED_ADDR);
    Wire.write(0x00);
    Wire.write(c);
    Wire.endTransmission();
  }

  void cmd2(uint8_t c, uint8_t v) {
    Wire.beginTransmission(OLED_ADDR);
    Wire.write(0x00); Wire.write(c); Wire.write(v);
    Wire.endTransmission();
  }

public:
  void begin() {
    Wire.begin();
    Wire.setClock(400000UL);
    cmd(0xAE); cmd2(0xD5, 0x80); cmd2(0xA8, 0x3F); cmd2(0xD3, 0x00);
    cmd(0x40); cmd2(0x8D, 0x14); cmd2(0x20, 0x02); cmd(0xA1);
    cmd(0xC8); cmd2(0xDA, 0x12); cmd2(0x81, 0x8F); cmd(0xAF);
    clear();
    display();
  }

  void clear() { memset(buffer, 0, 1024); }

  void pixel(int16_t x, int16_t y) {
    if (x >= 0 && x < OLED_WIDTH && y >= 0 && y < OLED_HEIGHT)
      buffer[x + (y >> 3) * OLED_WIDTH] |= (1 << (y & 7));
  }

  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h) {
    for (int16_t i = 0; i < w; i++)
      for (int16_t j = 0; j < h; j++) pixel(x + i, y + j);
  }

  void drawText(int16_t x, int16_t y, const char *str, uint8_t scale) {
    while (*str) {
      char c = *str;
      if (c >= 'a' && c <= 'z') c -= 32;
      if (c >= 32 && c <= 95) {
        uint8_t idx = c - 32;
        for (uint8_t col = 0; col < 5; col++) {
          uint8_t b = pgm_read_byte(&FONT[idx][col]);
          for (uint8_t row = 0; row < 7; row++) {
            if (b & (1 << row)) {
              if (scale == 1) pixel(x + col, y + row);
              else fillRect(x + col * scale, y + row * scale, scale, scale);
            }
          }
        }
      }
      x += 6 * scale;
      str++;
    }
  }

  void display() {
    for (uint8_t p = 0; p < OLED_PAGES; p++) {
      cmd(0xB0 | p);
      cmd(0x00 | (OLED_OFFSET & 0x0F));
      cmd(0x10 | (OLED_OFFSET >> 4));
      uint16_t idx = p * OLED_WIDTH;
      for (uint8_t c = 0; c < OLED_WIDTH; c += 16) {
        Wire.beginTransmission(OLED_ADDR);
        Wire.write(0x40);
        for (uint8_t i = 0; i < 16; i++) Wire.write(buffer[idx + c + i]);
        Wire.endTransmission();
      }
    }
  }
};

// Global UI state
Display oled;
char title[24] = "";
char subtitle[24] = "";
ActionType action = ACTION_NONE;
uint8_t animStep = 0;
unsigned long lastAnim = 0;
char rxBuffer[100];
uint8_t rxIdx = 0;

// Finds "key" (with quotes, so "title" can't match inside "subtitle").
// Returns true if the key was present.
bool extractJsonVal(const char *json, const char *key, char *out, uint8_t maxLen) {
  char pat[24];
  snprintf(pat, sizeof(pat), "\"%s\"", key);
  out[0] = '\0';
  const char *p = strstr(json, pat);
  if (!p) return false;
  p = strchr(p + strlen(pat), ':');
  if (!p) return false;
  p++;
  while (*p == ' ' || *p == '"') p++;
  uint8_t i = 0;
  while (*p && *p != '"' && *p != ',' && *p != '}' && i < maxLen - 1) {
    out[i++] = *p++;
  }
  out[i] = '\0';
  return true;
}

// Accepts "OFF","ON","BLINK_2HZ","BLINK_5HZ","PULSE_3","PULSE_5"
// (case-insensitive) or the numbers 0..5. Returns false if unknown.
bool parseLedMode(const char *v, LedMode &m) {
  char u[12];
  uint8_t i = 0;
  for (; v[i] && i < sizeof(u) - 1; i++) u[i] = toupper(v[i]);
  u[i] = '\0';

  if      (!strcmp(u, "OFF")       || !strcmp(u, "0")) m = LED_OFF;
  else if (!strcmp(u, "ON")        || !strcmp(u, "1")) m = LED_ON;
  else if (!strcmp(u, "BLINK_2HZ") || !strcmp(u, "2")) m = LED_BLINK_2HZ;
  else if (!strcmp(u, "BLINK_5HZ") || !strcmp(u, "3")) m = LED_BLINK_5HZ;
  else if (!strcmp(u, "PULSE_3")   || !strcmp(u, "4")) m = LED_PULSE_3;
  else if (!strcmp(u, "PULSE_5")   || !strcmp(u, "5")) m = LED_PULSE_5;
  else return false;
  return true;
}

// Every key is optional: only keys present in the message are changed.
void parseJson(const char *json) {
  extractJsonVal(json, "title", title, sizeof(title));
  extractJsonVal(json, "subtitle", subtitle, sizeof(subtitle));

  char val[16];
  if (extractJsonVal(json, "action", val, sizeof(val))) {
    if (strcmp(val, "loader") == 0 || strcmp(val, "bar") == 0) action = ACTION_LOADER;
    else if (strcmp(val, "spinner") == 0) action = ACTION_SPINNER;
    else action = ACTION_NONE;
  }

  LedMode m;
  if (extractJsonVal(json, "warn_light_mode", val, sizeof(val)) && parseLedMode(val, m))
    warnLight.setMode(m);
  if (extractJsonVal(json, "headlight_mode", val, sizeof(val)) && parseLedMode(val, m))
    headlight.setMode(m);
}

void renderUI() {
  oled.clear();

  bool hasTitle = title[0] != '\0';
  bool hasSub = subtitle[0] != '\0';
  bool hasAction = (action != ACTION_NONE);

  // Height definitions: Title (14px), Subtitle (7px), Action (8px)
  uint8_t hTitle = hasTitle ? 14 : 0;
  uint8_t hSub = hasSub ? 7 : 0;
  uint8_t hAct = hasAction ? 8 : 0;

  uint8_t count = (hasTitle ? 1 : 0) + (hasSub ? 1 : 0) + (hasAction ? 1 : 0);
  uint8_t spacing = (count > 1) ? 6 : 0;
  
  uint8_t totalHeight = hTitle + hSub + hAct + ((count > 1) ? (count - 1) * spacing : 0);
  int16_t currentY = (OLED_HEIGHT - totalHeight) / 2;

  // Render Title (Scale 2)
  if (hasTitle) {
    int16_t x = (OLED_WIDTH - (strlen(title) * 12 - 2)) / 2;
    oled.drawText(x < 0 ? 0 : x, currentY, title, 2);
    currentY += hTitle + spacing;
  }

  // Render Subtitle (Scale 1)
  if (hasSub) {
    int16_t x = (OLED_WIDTH - (strlen(subtitle) * 6 - 1)) / 2;
    oled.drawText(x < 0 ? 0 : x, currentY, subtitle, 1);
    currentY += hSub + spacing;
  }

  // Render Action Graphic
  if (hasAction) {
    if (action == ACTION_LOADER) {
      int16_t barW = 60, barH = 6;
      int16_t barX = (OLED_WIDTH - barW) / 2;
      for (int i = 0; i < barW; i++) {
        oled.pixel(barX + i, currentY);
        oled.pixel(barX + i, currentY + barH - 1);
      }
      for (int i = 0; i < barH; i++) {
        oled.pixel(barX, currentY + i);
        oled.pixel(barX + barW - 1, currentY + i);
      }
      uint8_t fillW = (animStep % 10) * (barW - 4) / 9;
      oled.fillRect(barX + 2, currentY + 2, fillW, barH - 4);
    } 
    else if (action == ACTION_SPINNER) {
      int16_t cx = OLED_WIDTH / 2;
      int16_t cy = currentY + 4;
      const int8_t dots[4][2] = {{0, -3}, {3, 0}, {0, 3}, {-3, 0}};
      for (uint8_t i = 0; i < 4; i++) {
        if (i == (animStep % 4)) {
          oled.fillRect(cx + dots[i][0] - 1, cy + dots[i][1] - 1, 3, 3);
        } else {
          oled.pixel(cx + dots[i][0], cy + dots[i][1]);
        }
      }
    }
  }

  oled.display();
}

void setup() {
  Serial.begin(115200);
  oled.begin();
  warnLight.begin(PIN_WARN_LIGHT);
  headlight.begin(PIN_HEADLIGHT);

  // Startup state: both lights blink at 5 Hz until a JSON command changes them
  warnLight.setMode(LED_BLINK_5HZ);
  headlight.setMode(LED_BLINK_5HZ);

  // Default centered view
  strcpy(title, "WareGV");
  action = ACTION_SPINNER;
}

void loop() {
  // Read Serial JSON Input
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (rxIdx > 0) {
        rxBuffer[rxIdx] = '\0';
        parseJson(rxBuffer);
        rxIdx = 0;
      }
    } else if (rxIdx < sizeof(rxBuffer) - 1) {
      rxBuffer[rxIdx++] = c;
    }
  }

  // Non-blocking LED patterns
  warnLight.update();
  headlight.update();

  // Animate at ~10 FPS
  if (millis() - lastAnim > 100) {
    lastAnim = millis();
    animStep++;
    renderUI();
  }
}