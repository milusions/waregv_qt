/*
 * Arduino Nano - SSD1306 OLED Profile Controller
 * Libraries used: Standard Wire.h ONLY
 */

#include <Wire.h>
#include "SSD1306_Wire.h"
#include "Profiles.h"

SSD1306Wire oled;
ProfileManager profiles(oled);

char serialBuffer[64];
uint8_t bufferIdx = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial) { ; }

    Serial.println(F("--- OLED Profile Controller Online ---"));

    // Initialize OLED hardware
    oled.begin();

    // Default profile state
    profiles.setProfile(PROFILE_BOOTING);
}

void loop() {
    // 1. Listen for incoming JSON serial commands
    checkSerialInput();

    // 2. Render and animate current profile
    profiles.update();
}

void checkSerialInput() {
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