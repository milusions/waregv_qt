#include <Wire.h>

#define OLED_ADDR 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

// 128x64 frame buffer (1024 bytes)
uint8_t frameBuffer[OLED_WIDTH * OLED_HEIGHT / 8];

// Send single command byte to SSD1306
void sendCommand(uint8_t cmd) {
  Wire.beginTransmission(OLED_ADDR);
  Wire.write(0x00); // Command stream header
  Wire.write(cmd);
  Wire.endTransmission();
}

// Corrected SSD1306 OLED Initialization
void initOLED() {
  Wire.begin();
  Wire.setClock(400000L); // Fast 400kHz I2C clock
  delay(50);
  
  sendCommand(0xAE); // Display OFF
  sendCommand(0xD5); sendCommand(0x80); // Set display clock divide ratio
  sendCommand(0xA8); sendCommand(0x3F); // Set multiplex ratio (64MUX)
  sendCommand(0xD3); sendCommand(0x00); // Set display offset (no offset)
  sendCommand(0x40);                     // Set start line #0
  sendCommand(0x8D); sendCommand(0x14); // Enable charge pump
  
  // FIX 1: Set Memory Addressing Mode to Horizontal Addressing (0x20, 0x00)
  sendCommand(0x20); sendCommand(0x00); 
  
  sendCommand(0xA1); // Column remap (SEG0 to 127)
  sendCommand(0xC8); // COM scan direction remapped
  sendCommand(0xDA); sendCommand(0x12); // Set COM pins hardware config
  sendCommand(0x81); sendCommand(0xCF); // Set contrast control
  sendCommand(0xD9); sendCommand(0xF1); // Set pre-charge period
  sendCommand(0xDB); sendCommand(0x40); // Set VCOMH deselect level
  sendCommand(0xA4);                     // Entire display ON (resume to RAM)
  sendCommand(0xA6);                     // Normal display mode
  sendCommand(0xAF); // Display ON

  clearBuffer();
  renderBuffer();
}

// Clear local RAM buffer
void clearBuffer() {
  memset(frameBuffer, 0, sizeof(frameBuffer));
}

// Corrected Buffer Renderer using 0x21 (Column) & 0x22 (Page) addressing
void renderBuffer() {
  // Set Column Address Range: 0 to 127
  sendCommand(0x21);
  sendCommand(0);
  sendCommand(127);

  // Set Page Address Range: 0 to 7
  sendCommand(0x22);
  sendCommand(0);
  sendCommand(7);

  // Stream full 1024-byte framebuffer in 16-byte Wire transmissions
  for (uint16_t i = 0; i < sizeof(frameBuffer); i += 16) {
    Wire.beginTransmission(OLED_ADDR);
    Wire.write(0x40); // Data stream header
    for (uint8_t j = 0; j < 16; j++) {
      Wire.write(frameBuffer[i + j]);
    }
    Wire.endTransmission();
  }
}

// Safe Pixel Draw Function
void drawPixel(int x, int y, bool color) {
  if (x < 0 || x >= OLED_WIDTH || y < 0 || y >= OLED_HEIGHT) return;
  uint16_t index = x + (y / 8) * OLED_WIDTH;
  if (color) {
    frameBuffer[index] |= (1 << (y % 8));
  } else {
    frameBuffer[index] &= ~(1 << (y % 8));
  }
}