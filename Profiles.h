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

    // --- Organic Eye Kinematics & Chat Variables ---
    float curX = 0, curY = 0;
    float curW = 32, curH = 32;
    float tarX = 0, tarY = 0;
    float tarW = 32, tarH = 32;
    unsigned long lastGazeTime = 0;
    unsigned long nextGazeInterval = 1000;
    
    unsigned long blinkStartTime = 0;
    unsigned long nextBlinkTime = 2000;
    bool isBlinking = false;
    
    bool chatVisible = false;
    bool chatPondering = false;
    unsigned long chatStartTime = 0;
    unsigned long nextChatTime = 0;
    unsigned long chatDuration = 0;
    uint8_t chatIndex = 0;

public:
    ProfileManager(SSD1306Wire &display) 
        : oled(display), currentProfile(PROFILE_BOOTING), animStep(0), lastAnimTime(0) {}

    void setProfile(ProfileType profile) {
        currentProfile = profile;
        animStep = 0;
        
        // Reset IDLE state to prevent jumpy eyes on entry
        if (profile == PROFILE_IDLE || profile == PROFILE_IS_NOT_SPEAKING) {
            curW = 32; curH = 32;
            tarW = 32; tarH = 32;
            chatVisible = false;
            nextChatTime = millis() + 2000;
        }

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

            case PROFILE_IS_NOT_SPEAKING:
            case PROFILE_IDLE:
                // Runs organic eye physics on every loop cycle freely
                updateOrganicIdle(now);
                renderOrganicIdle(now);
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
                
            default:
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

    // -------------------------------------------------------------
    // PROFILE: BOOTING (Text in center, Spinner at bottom)
    // -------------------------------------------------------------
    void renderBootingProfile() {
        oled.fillScreen();
        oled.drawCenteredString(14, F("BOOTING"), 0, 2);

        int16_t centerX = 64, centerY = 48;
        const int8_t dx[8] = { 9,  6,  0, -6, -9, -6,  0,  6 };
        const int8_t dy[8] = { 0,  6,  9,  6,  0, -6, -9, -6 };

        for (uint8_t i = 0; i < 8; i++) {
            int16_t px = centerX + dx[i];
            int16_t py = centerY + dy[i];
            if (i == animStep) oled.fillCircle(px, py, 2, 0);
            else if (i == (animStep + 7) % 8) oled.fillCircle(px, py, 1, 0);
            else oled.setPixel(px, py, 0);
        }
        oled.display();
    }

    // -------------------------------------------------------------
    // PROFILE: ORGANIC IDLE (Eyes + Chat Bubbles)
    // -------------------------------------------------------------
    void updateOrganicIdle(unsigned long t) {
        // Natural Blinking
        if (t > nextBlinkTime && !isBlinking) {
            isBlinking = true;
            blinkStartTime = t;
            nextBlinkTime = t + (random(0, 100) > 85 ? 250 : random(2000, 6000));
        }
        if (isBlinking && (t - blinkStartTime > 120)) {
            isBlinking = false;
        }

        // Natural Gaze Targetting
        if (t - lastGazeTime > nextGazeInterval) {
            tarX = random(-14, 15);
            tarY = random(-8, 9);
            uint8_t mood = random(0, 10);
            if (mood > 8) { tarW = 36; tarH = 30; }      // Wide open
            else if (mood > 6) { tarW = 28; tarH = 36; } // Tall focus
            else { tarW = 32; tarH = 34; }               // Normal
            lastGazeTime = t;
            nextGazeInterval = random(400, 2500);
        }

        // Smooth Kinematics (Lerp)
        curX += (tarX - curX) * 0.4;
        curY += (tarY - curY) * 0.4;
        curW += (tarW - curW) * 0.35;
        float activeTarH = isBlinking ? 2.0 : tarH;
        curH += (activeTarH - curH) * 0.7;

        // Chat Bubble Trigger
        if (!chatVisible && t >= nextChatTime) {
            chatIndex = random(0, 16);
            chatPondering = (random(0, 100) < 22);
            chatStartTime = t;
            chatDuration = random(2600, 4800);
            chatVisible = true;
        }
        if (chatVisible && (t - chatStartTime >= chatDuration)) {
            chatVisible = false;
            chatPondering = false;
            nextChatTime = t + random(4500, 11000);
        }
    }

    void renderOrganicIdle(unsigned long t) {
        oled.clear();
        
        // Biological Breathing Sine Wave
        float breathY = sin(t / 800.0) * 2.0;

        // Micro-saccades (tiny eye shakes when focused)
        float jitterX = (!isBlinking && curH > 15) ? cos(t / 150.0) * 0.5 : 0;

        int lX = 64 - 28 - (int)curW/2 + (int)curX + jitterX;
        int rX = 64 + 28 - (int)curW/2 + (int)curX + jitterX;
        int y = 32 - (int)curH/2 + (int)curY + breathY;
        
        // Draw the Eyes
        oled.fillRoundRect(lX, y, (int)curW, (int)curH, ((int)curW > 16) ? 8 : 4, 1);
        oled.fillRoundRect(rX, y, (int)curW, (int)curH, ((int)curW > 16) ? 8 : 4, 1);
        
        // Draw eye-bags if looking up sharply
        if (curH > 10 && tarY < -3) {
            oled.fillRoundRect(lX - 2, y + (int)curH - 6, (int)curW + 4, 10, 4, 0);
            oled.fillRoundRect(rX - 2, y + (int)curH - 6, (int)curW + 4, 10, 4, 0);
        }

        // Render Random Chat Bubble
        if (chatVisible) drawIdleChatBubble(t);

        oled.display();
    }

    // Safe PROGMEM String lookup to save massive SRAM amounts
    const __FlashStringHelper* getIdleMessage(uint8_t idx) {
        switch(idx) {
            case 0: return F("HI HUMAN");
            case 1: return F("ANY PLANS?");
            case 2: return F("WHERE TO?");
            case 3: return F("I AM BORED");
            case 4: return F("BEEP BOOP");
            case 5: return F("WATCHING...");
            case 6: return F("GO LEFT?");
            case 7: return F("GO RIGHT?");
            case 8: return F("HMM...");
            case 9: return F("I WONDER...");
            case 10: return F("READY?");
            case 11: return F("LETS ROAM");
            case 12: return F("ALL GOOD");
            case 13: return F("NEED A HAND?");
            case 14: return F("STILL HERE");
            default: return F("CURIOUS...");
        }
    }

    void drawIdleChatBubble(unsigned long t) {
        const __FlashStringHelper* msg = getIdleMessage(chatIndex);
        
        int textW = 0;
        if (chatPondering) {
            textW = 25; // fixed width for 3 animated dots
        } else {
            PGM_P p = reinterpret_cast<PGM_P>(msg);
            while (pgm_read_byte(p++)) textW += 6;
            if (textW > 0) textW -= 1; 
        }
        
        int bubbleW = textW + 12;
        if (bubbleW < 38) bubbleW = 38;
        int bubbleH = 17;
        int bx = 128 - bubbleW - 2;
        int by = 2;
        
        // Pop-in / pop-out animation bounds
        unsigned long age = t - chatStartTime;
        unsigned long remaining = (chatDuration > age) ? (chatDuration - age) : 0;
        int inset = 0;
        if (age < 180) inset = 3 - (age / 60);
        else if (remaining < 180) inset = 1 + ((180 - remaining) / 60);
        if (inset < 0) inset = 0;
        
        // Draw Bubble shape
        oled.fillRoundRect(bx + inset, by + inset, bubbleW - 2*inset, bubbleH - 2*inset, 4, 1);
        oled.fillRoundRect(bx + 2 + inset, by + 2 + inset, bubbleW - 4 - 2*inset, bubbleH - 4 - 2*inset, 2, 0);
        
        // Draw Tail
        oled.fillTriangle(bx + 8, by + bubbleH - 1, bx + 16, by + bubbleH - 1, bx + 9, by + bubbleH + 5, 1);
        oled.drawLine(bx + 9, by + bubbleH - 2, bx + 9, by + bubbleH + 3, 0);
        
        // Draw text or loading dots inside bubble
        if (chatPondering) {
            int dots = (t / 220) % 4;
            for (int i = 0; i < dots; i++) {
                oled.fillRoundRect(bx + 12 + i * 8, by + bubbleH / 2 - 2, 4, 4, 1, 1);
            }
        } else {
            int tx = bx + (bubbleW - textW) / 2;
            oled.drawString5x7(tx, by + 5, msg, 1);
        }
    }

    // -------------------------------------------------------------
    // ROS 2 PROFILES
    // -------------------------------------------------------------
    void renderGoalReceived() {
        oled.clear();
        drawHeader(F("STATUS: TARGET"));
        if (animStep % 2 == 0) oled.drawCenteredString(24, F(" (O_O) "), 1, 2);
        else oled.drawCenteredString(24, F(" [o_o] "), 1, 2);
        drawFooter(F("Target Set"));
        oled.display();
    }

    void renderNavigated() {
        oled.clear();
        drawHeader(F("NAVIGATING"));
        if (animStep == 0) oled.drawCenteredString(24, F(" ( ._.)> "), 1, 2);
        else if (animStep == 1) oled.drawCenteredString(24, F(" ( •_•)> "), 1, 2);
        else if (animStep == 2) oled.drawCenteredString(24, F(" ( •_•)>="), 1, 2);
        else oled.drawCenteredString(24, F(" (•_• )> "), 1, 2);
        drawFooter(F("Moving to target..."));
        oled.display();
    }

    void renderGoalReached() {
        oled.clear();
        drawHeader(F("SUCCESS"));
        if (animStep % 2 == 0) oled.drawCenteredString(22, F(" (^ _ ^) "), 1, 2);
        else oled.drawCenteredString(26, F(" (^ - ^) "), 1, 2);
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
            { 4, 12, 20, 12,  4 }, { 8, 18, 12, 22,  8 }, { 16, 8, 24,  8, 16 }, { 22, 14, 6, 18, 20 },
            { 12, 22, 16, 8, 14 }, { 6, 16, 22, 14,  8 }, { 18, 6, 12, 20, 16 }, { 24, 12, 8, 16, 22 }
        };
        for (uint8_t i = 0; i < 5; i++) {
            uint8_t h = heights[animStep][i];
            oled.fillRect(36 + (i * 12), 44 - h, 8, h, 1);
        }
        drawFooter(F("Voice Active"));
        oled.display();
    }
};

#endif // PROFILES_H