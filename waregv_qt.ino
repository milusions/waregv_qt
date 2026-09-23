/*
 * Arduino Nano / Uno - SSD1306 OLED Controller for ROS 2 Serial Interface
 * Wire.h Driver Only (No Adafruit or external SSD1306 libraries required)
 *
 * Expected Serial Input @ 115200 Baud:
 *   {"profile":"goal_received"}
 *   {"profile":"navigated"}
 *   {"profile":"goal_reached"}
 *   {"profile":"navigation_error"}
 *   {"profile":"is_speaking"}
 *   {"profile":"is_not_speaking"}
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

    Serial.println(F("--- ROS 2 OLED Profile Controller Online ---"));
    Serial.println(F("Send JSON command, e.g.: {\"profile\":\"goal_received\"}"));

    // Initialize SSD1306 OLED (I2C address 0x3C)
    oled.begin();

    // Set default standby state
    profiles.setProfile(PROFILE_IS_NOT_SPEAKING);
}

void loop() {
    // 1. Process Serial JSON commands from ROS 2 Node
    checkSerialInput();

    // 2. Render and animate active profile screen
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
    Serial.print(F("ROS 2 Received: "));
    Serial.println(jsonStr);

    const char* keyPos = strstr(jsonStr, "\"profile\"");
    if (!keyPos) keyPos = strstr(jsonStr, "profile");

    if (keyPos) {
        const char* colonPos = strchr(keyPos, ':');
        if (colonPos) {
            // ROS 2 Navigation & Audio Profiles
            if (strstr(colonPos, "goal_received")) {
                profiles.setProfile(PROFILE_GOAL_RECEIVED);
                Serial.println(F("-> Profile: GOAL_RECEIVED"));
            } else if (strstr(colonPos, "navigated")) {
                profiles.setProfile(PROFILE_NAVIGATED);
                Serial.println(F("-> Profile: NAVIGATED"));
            } else if (strstr(colonPos, "goal_reached")) {
                profiles.setProfile(PROFILE_GOAL_REACHED);
                Serial.println(F("-> Profile: GOAL_REACHED"));
            } else if (strstr(colonPos, "navigation_error")) {
                profiles.setProfile(PROFILE_NAVIGATION_ERROR);
                Serial.println(F("-> Profile: NAVIGATION_ERROR"));
            } else if (strstr(colonPos, "is_speaking")) {
                profiles.setProfile(PROFILE_IS_SPEAKING);
                Serial.println(F("-> Profile: IS_SPEAKING"));
            } else if (strstr(colonPos, "is_not_speaking")) {
                profiles.setProfile(PROFILE_IS_NOT_SPEAKING);
                Serial.println(F("-> Profile: IS_NOT_SPEAKING"));
            } 
            // Preset Profiles
            else if (strstr(colonPos, "booting")) {
                profiles.setProfile(PROFILE_BOOTING);
                Serial.println(F("-> Profile: BOOTING"));
            } else if (strstr(colonPos, "idle")) {
                profiles.setProfile(PROFILE_IS_NOT_SPEAKING);
                Serial.println(F("-> Profile: IDLE"));
            } else {
                Serial.println(F("-> Unknown profile specified."));
            }
            return;
        }
    }
    
    Serial.println(F("-> Expected JSON syntax: {\"profile\":\"navigated\"}"));
}