#ifndef PROFILES_H
#define PROFILES_H

#include "SSD1306_Wire.h"

enum ProfileType {
    PROFILE_BOOTING,
    PROFILE_IDLE,
    PROFILE_INFO,
    PROFILE_SUCCESS,
    PROFILE_ERROR,
    // ROS 2 Communication Profiles
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

    ProfileType getCurrentProfile() const { return currentProfile; }

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
                    animStep = (animStep + 1) % 6; // Flashing eye state
                }
                break;

            case PROFILE_NAVIGATED:
                if (now - lastAnimTime >= 150) {
                    lastAnimTime = now;
                    renderNavigated();
                    animStep = (animStep + 1) % 4; // Eye panning / arrow
                }
                break;

            case PROFILE_GOAL_REACHED:
                if (now - lastAnimTime >= 200) {
                    lastAnimTime = now;
                    renderGoalReached();
                    animStep = (animStep + 1) % 4; // Bouncing face
                }
                break;

            case PROFILE_NAVIGATION_ERROR:
                if (now - lastAnimTime >= 250) {
                    lastAnimTime = now;
                    renderNavigationError();
                    animStep = (animStep + 1) % 2; // Screen flash
                }
                break;

            case PROFILE_IS_SPEAKING:
                if (now - lastAnimTime >= 80) {
                    lastAnimTime = now;
                    renderIsSpeaking();
                    animStep = (animStep + 1) % 8; // Equalizer bar animation
                }
                break;

            case PROFILE_IS_NOT_SPEAKING:
            case PROFILE_IDLE:
                // Passive eye-blinking every 3 seconds
                if (now - lastBlinkTime >= 3000) {
                    eyeOpen = !eyeOpen;
                    if (eyeOpen) lastBlinkTime = now;
                    else lastBlinkTime = now - 2800; // Blink duration 200ms
                }
                renderIsNotSpeaking();
                break;

            default:
                break;
        }
    }

private:
    void drawHeader(const char* title) {
        oled.fillRect(0, 0, 128, 12, 1);
        oled.drawCenteredString(2, title, 0, 1);
    }

    void drawFooter(const char* subtitle) {
        oled.drawFastHLine(0, 52, 128, 1);
        oled.drawCenteredString(54, subtitle, 1, 1);
    }

    // BOOTING PROFILE
    void renderBootingProfile() {
        oled.fillScreen();
        oled.drawCenteredString(14, "BOOTING", 0, 2);

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

    // ROS 2 PROFILE 1: GOAL RECEIVED
    void renderGoalReceived() {
        oled.clear();
        drawHeader("STATUS: TARGET");

        if (animStep % 2 == 0) {
            oled.drawCenteredString(24, " (O_O) ", 1, 2);
        } else {
            oled.drawCenteredString(24, " [o_o] ", 1, 2);
        }

        drawFooter("Target Set");
        oled.display();
    }

    // ROS 2 PROFILE 2: NAVIGATED
    void renderNavigated() {
        oled.clear();
        drawHeader("NAVIGATING");

        if (animStep == 0) {
            oled.drawCenteredString(24, " ( ._.)> ", 1, 2);
        } else if (animStep == 1) {
            oled.drawCenteredString(24, " ( •_•)> ", 1, 2);
        } else if (animStep == 2) {
            oled.drawCenteredString(24, " ( •_•)>=", 1, 2);
        } else {
            oled.drawCenteredString(24, " (•_• )> ", 1, 2);
        }

        drawFooter("Moving to target...");
        oled.display();
    }

    // ROS 2 PROFILE 3: GOAL REACHED
    void renderGoalReached() {
        oled.clear();
        drawHeader("SUCCESS");

        if (animStep % 2 == 0) {
            oled.drawCenteredString(22, " (^ _ ^) ", 1, 2);
        } else {
            oled.drawCenteredString(26, " (^ - ^) ", 1, 2);
        }

        drawFooter("Arrived safely");
        oled.display();
    }

    // ROS 2 PROFILE 4: NAVIGATION ERROR
    void renderNavigationError() {
        oled.clear();
        
        if (animStep == 0) {
            oled.setInvertDisplay(true);
            drawHeader("ALERT / ERROR");
            oled.drawCenteredString(24, " (>_<) ! ", 1, 2);
            drawFooter("Path Blocked");
        } else {
            oled.setInvertDisplay(false);
            oled.drawRect(0, 0, 128, 64, 1);
            drawHeader("ALERT / ERROR");
            oled.drawCenteredString(24, " (X_X) ! ", 1, 2);
            drawFooter("Canceled");
        }

        oled.display();
    }

    // ROS 2 PROFILE 5: IS SPEAKING
    void renderIsSpeaking() {
        oled.clear();
        drawHeader("AUDIO OUTPUT");

        // Dynamic 5-bar equalizer audio spectrum animation
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

        drawFooter("Voice Active");
        oled.display();
    }

    // ROS 2 PROFILE 6: IS NOT SPEAKING
    void renderIsNotSpeaking() {
        oled.clear();
        drawHeader("STANDBY");

        if (eyeOpen) {
            oled.drawCenteredString(24, " ( -_- ) ", 1, 2);
        } else {
            oled.drawCenteredString(24, " ( ___ ) ", 1, 2);
        }

        drawFooter("Listening...");
        oled.display();
    }
};

#endif // PROFILES_H