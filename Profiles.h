#ifndef PROFILES_H
#define PROFILES_H

#include "SSD1306_Wire.h"

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
                renderIsNotSpeaking();
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

    void renderIsNotSpeaking() {
        oled.clear();
        drawHeader(F("STANDBY"));

        if (eyeOpen) {
            oled.drawCenteredString(24, F(" ( -_- ) "), 1, 2);
        } else {
            oled.drawCenteredString(24, F(" ( ___ ) "), 1, 2);
        }

        drawFooter(F("Listening..."));
        oled.display();
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

#endif // PROFILES_H