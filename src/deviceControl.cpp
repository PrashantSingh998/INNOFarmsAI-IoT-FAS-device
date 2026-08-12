// src/deviceControl.cpp
#include "deviceControl.h"
#include "configControl.h"
#include "mqttControl.h"
#include "variables.h"
#include "networkControl.h"
#include <SPI.h>
#include <TFT_eSPI.h>

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <vector>


TFT_eSPI tft = TFT_eSPI();

const uint16_t BRAND_GREEN      = 0x04D0;
const uint16_t BRAND_GREEN_DARK = 0x01E5; // Deep INNOFarms-style green
const uint16_t BRAND_MINT       = 0xCFF7;
const uint16_t BRAND_BG         = 0x0841;
const uint16_t BRAND_PANEL      = 0x10E3;
const uint16_t BRAND_PANEL_2    = 0x1945;
const uint16_t BRAND_WHITE      = TFT_WHITE;
const uint16_t BRAND_MUTED      = 0xBDF7;
const uint16_t BRAND_WARNING    = 0xFD20;
const uint16_t BRAND_ERROR      = 0xF800;

const int16_t SCREEN_WIDTH  = 480;
const int16_t SCREEN_HEIGHT = 320;

unsigned long  status_duration =0;
unsigned long status_millis = 0;

uint8_t wifiAnimationFrame = 0;
bool dashboardVisible = false;
bool reconnectScreenVisible = false;
bool internetAvailable = false;

SensorValues sensors = {
    0,   // temperature
    0,   // humidity
    0,   // CO2
    0,   // water level
    0,   // EC 1/1
    0,   // pH 1/1
    0,   // water temperature 1/1
    0,   // EC 1/2
    0,   // pH 1/2
    0,   // water temperature 1/2
    0,   // EC 2/1
    0,   // pH 2/1
    0,   // water temperature 2/1
    0,   // EC 2/2
    0,   // pH 2/2
    0    // water temperature 2/2
};

// Change these based on the currently selected irrigation context.
uint8_t activeUnit = 1;
uint8_t activeZone = 1;

String message_display = "Controllers are idle";

void updateControllerMessage(const String& message)
{
    message_display = message;
    drawControllerStatusLine();
}

// ========== INITIALIZATION ==========
void initDevices() {
  SPI.begin();
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(BRAND_BG);
}


void showStartupLogo() {
  tft.fillScreen(TFT_BLACK);

  int16_t x = (SCREEN_WIDTH - LOGO_WIDTH) / 2;
  int16_t y = (SCREEN_HEIGHT - LOGO_HEIGHT) / 2;

  tft.setSwapBytes(true);

  tft.pushImage(
    x,
    y,
    LOGO_WIDTH,
    LOGO_HEIGHT,
    innofarms_logo
  );
  delay(5000);
}


void showWiFiConnectingScreen(bool reconnecting) {
  dashboardVisible = false;
  reconnectScreenVisible = true;
  wifiAnimationFrame = 0;

  tft.fillScreen(BRAND_BG);
  tft.fillRect(0, 0, SCREEN_WIDTH, 46, BRAND_GREEN_DARK);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(BRAND_WHITE, BRAND_GREEN_DARK);
  tft.drawString("INNOFarms.AI", SCREEN_WIDTH / 2, 23, 4);

  tft.setTextColor(BRAND_WHITE, BRAND_BG);
  tft.drawString(reconnecting ? "Wi-Fi disconnected" : "Connecting to Wi-Fi",
                  SCREEN_WIDTH / 2, 78, 4);

  tft.setTextColor(BRAND_GREEN, BRAND_BG);
  tft.drawString(WIFI_SSID, SCREEN_WIDTH / 2, 111, 2);

  tft.setTextColor(BRAND_MUTED, BRAND_BG);
  tft.drawString(reconnecting ? "Trying to reconnect..." : "Please wait...",
                  SCREEN_WIDTH / 2, 282, 2);

  drawWiFiAnimation(0);
}


void drawWiFiAnimation(uint8_t frame) {
    const int16_t cx = SCREEN_WIDTH / 2;
    const int16_t cy = 202;
    const uint8_t activeBars = (frame % 4) + 1;

    tft.fillRect(cx - 95, cy - 70, 190, 125, BRAND_BG);
    tft.fillCircle(cx, cy + 30, 7,
                   activeBars >= 1 ? BRAND_GREEN : BRAND_PANEL_2);

    for (uint8_t i = 1; i <= 3; i++) {
        const int16_t radius = 18 + (i * 17);
        const uint16_t color = activeBars > i ? BRAND_GREEN : BRAND_PANEL_2;
        tft.drawCircle(cx, cy + 30, radius, color);
        tft.drawCircle(cx, cy + 30, radius + 1, color);
        tft.drawCircle(cx, cy + 30, radius + 2, color);
        tft.fillRect(cx - radius - 5, cy + 30,
                     radius * 2 + 10, radius + 10, BRAND_BG);
        tft.fillRect(cx - radius - 5, cy - radius - 7,
                     radius * 2 + 10, radius / 2, BRAND_BG);
    }
}


void drawMainDashboard() {
    tft.fillScreen(BRAND_BG);
    drawDashboardHeader();
    drawSensorLayout();
    updateSensorValuesOnly();
    drawControllerStatusLine();
}
void updateActiveUnitZoneFromStatus() {
    // The water-quality panel rotates itself with rotateWaterQualityDisplay().
    // Keep this function as a compatibility hook without forcing the display
    // back to Unit 1 / Zone 1.
}

void rotateWaterQualityDisplay() {
    if (activeUnit == 1 && activeZone == 1) {
        activeUnit = 1;
        activeZone = 2;
    } else if (activeUnit == 1 && activeZone == 2) {
        activeUnit = 2;
        activeZone = 1;
    } else if (activeUnit == 2 && activeZone == 1) {
        activeUnit = 2;
        activeZone = 2;
    } else {
        activeUnit = 1;
        activeZone = 1;
    }

    updateWaterQualityContext();
    updateSensorValuesOnly();
}

void drawControllerStatusLine()
{
    const int16_t x = 8;
    const int16_t y = 244;
    const int16_t w = 464;
    const int16_t h = 68;

    // Redraw footer background
    tft.fillRoundRect(
        x,
        y,
        w,
        h,
        7,
        BRAND_GREEN_DARK
    );

    tft.drawRoundRect(
        x,
        y,
        w,
        h,
        7,
        BRAND_GREEN
    );

    // Status indicator dot
    tft.fillCircle(
        x + 17,
        y + h / 2,
        6,
        BRAND_GREEN
    );

    const int16_t textX = x + 30;
    const int16_t textY = y + h / 2 + 1;
    const int16_t availableWidth = w - 50;

    // Try large font first
    uint8_t footerFont = 4;

    // Use smaller font if the message does not fit
    if (tft.textWidth(message_display, footerFont) > availableWidth)
    {
        footerFont = 2;
    }

    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(
        BRAND_WHITE,
        BRAND_GREEN_DARK
    );

    // Draw message with bold effect
    tft.drawString(
        message_display,
        textX,
        textY,
        footerFont
    );

    tft.drawString(
        message_display,
        textX + 1,
        textY,
        footerFont
    );

    tft.drawString(
        message_display,
        textX,
        textY + 1,
        footerFont
    );

    tft.setTextDatum(TL_DATUM);
}

void updateSensorValuesOnly() {
    const int16_t y = 54;
    const int16_t w = 111;
    const int16_t gap = 7;
    const int16_t x0 = 8;
    const int16_t valueY = y + 47;

    const String topValues[4] = {
        String(sensors.temperature, 1) + " C",
        String(sensors.humidity, 1) + " %",
        String(sensors.co2) + " ppm",
        String(sensors.waterLevel, 1) + " L"
    };

    for (uint8_t i = 0; i < 4; i++) {
        int16_t x = x0 + (w + gap) * i;
        tft.fillRect(x + 5, y + 29, w - 10, 37, BRAND_PANEL);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(BRAND_GREEN, BRAND_PANEL);
        tft.drawString(topValues[i], x + w / 2, valueY, 2);
        tft.drawString(topValues[i], x + w / 2 + 1, valueY, 2);
    }

    const int16_t panelX = 8;
    const int16_t panelY = 134;
    const int16_t panelW = 464;
    const int16_t colW = panelW / 3;
    const int16_t waterValueY = panelY + 76;

    float activeEc = sensors.ec11;
    float activePh = sensors.ph11;
    float activeWaterTemperature = sensors.waterTemperature11;

    if (activeUnit == 1 && activeZone == 2) {
        activeEc = sensors.ec12;
        activePh = sensors.ph12;
        activeWaterTemperature = sensors.waterTemperature12;
    } else if (activeUnit == 2 && activeZone == 1) {
        activeEc = sensors.ec21;
        activePh = sensors.ph21;
        activeWaterTemperature = sensors.waterTemperature21;
    } else if (activeUnit == 2 && activeZone == 2) {
        activeEc = sensors.ec22;
        activePh = sensors.ph22;
        activeWaterTemperature = sensors.waterTemperature22;
    }

    const String waterValues[3] = {
        String(activeEc, 2) + " mS/cm",
        String(activePh, 2),
        String(activeWaterTemperature, 1) + " C"
    };

    for (uint8_t i = 0; i < 3; i++) {
        int16_t cx = panelX + colW * i + colW / 2;
        int16_t clearX = panelX + colW * i + 5;
        tft.fillRect(clearX, panelY + 58, colW - 10, 34, BRAND_PANEL);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(BRAND_GREEN, BRAND_PANEL);
        tft.drawString(waterValues[i], cx, waterValueY, 2);
        tft.drawString(waterValues[i], cx + 1, waterValueY, 2);
    }

    tft.setTextDatum(TL_DATUM);
}

void drawSensorLayout() {
    const int16_t y = 54;
    const int16_t w = 111;
    const int16_t h = 72;
    const int16_t gap = 7;
    const int16_t x0 = 8;

    drawSmallSensorCard(x0, y, w, h, "TEMPERATURE");
    drawSmallSensorCard(x0 + (w + gap), y, w, h, "HUMIDITY");
    drawSmallSensorCard(x0 + (w + gap) * 2, y, w, h, "CO2");
    drawSmallSensorCard(x0 + (w + gap) * 3, y, w, h, "WATER LEVEL");

    drawWaterQualityPanel();
}
void drawSmallSensorCard(int16_t x, int16_t y, int16_t w, int16_t h,
                         const char* label) {
    tft.fillRoundRect(x, y, w, h, 8, BRAND_PANEL);
    tft.drawRoundRect(x, y, w, h, 8, BRAND_GREEN);
    tft.fillRoundRect(x + 1, y + 1, w - 2, 24, 7, BRAND_GREEN_DARK);
    tft.fillRect(x + 1, y + 17, w - 2, 8, BRAND_GREEN_DARK);

    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(BRAND_WHITE, BRAND_GREEN_DARK);
    tft.drawString(label, x + w / 2, y + 12, 1);
    tft.drawString(label, x + w / 2 + 1, y + 12, 1);
    tft.setTextDatum(TL_DATUM);
}

void updateWaterQualityContext() {
    const int16_t x = 8;
    const int16_t y = 134;
    const int16_t w = 464;

    tft.fillRect(x + 245, y + 4, 205, 21, BRAND_GREEN_DARK);

    String context = "UNIT: " + String(activeUnit) + "   ZONE: " + String(activeZone);
    tft.setTextDatum(MR_DATUM);
    tft.setTextColor(BRAND_MINT, BRAND_GREEN_DARK);
    tft.drawString(context, x + w - 12, y + 14, 2);
    tft.setTextDatum(TL_DATUM);
}

void drawWaterQualityPanel() {
    const int16_t x = 8;
    const int16_t y = 134;
    const int16_t w = 464;
    const int16_t h = 102;

    tft.fillRoundRect(x, y, w, h, 9, BRAND_PANEL);
    tft.drawRoundRect(x, y, w, h, 9, BRAND_GREEN);

    tft.fillRoundRect(x + 1, y + 1, w - 2, 28, 8, BRAND_GREEN_DARK);
    tft.fillRect(x + 1, y + 20, w - 2, 9, BRAND_GREEN_DARK);

    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(BRAND_WHITE, BRAND_GREEN_DARK);
    tft.drawString("WATER QUALITY", x + 12, y + 14, 2);
    tft.drawString("WATER QUALITY", x + 13, y + 14, 2);

    updateWaterQualityContext();

    const int16_t colW = w / 3;
    for (uint8_t i = 1; i < 3; i++) {
        tft.drawFastVLine(x + i * colW, y + 35, h - 43, BRAND_PANEL_2);
    }

    const char* labels[3] = {"EC", "pH", "WATER TEMP."};
    for (uint8_t i = 0; i < 3; i++) {
        int16_t cx = x + colW * i + colW / 2;
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(BRAND_MUTED, BRAND_PANEL);
        tft.drawString(labels[i], cx, y + 45, 2);
        tft.drawString(labels[i], cx + 1, y + 45, 2);
    }

    tft.setTextDatum(TL_DATUM);
}

void drawDashboardHeader() {
    tft.fillRect(0, 0, SCREEN_WIDTH, 46, BRAND_GREEN_DARK);
    tft.fillRect(0, 43, SCREEN_WIDTH, 3, BRAND_GREEN);

    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(BRAND_WHITE, BRAND_GREEN_DARK);
    tft.drawString("INNOFarms.AI", 14, 22, 4);
    tft.drawString("INNOFarms.AI", 15, 22, 4); // bold effect

    drawHeaderConnectivityIcons();
    tft.setTextDatum(TL_DATUM);
}
void drawStatusTick(int16_t x, int16_t y, uint16_t color) {
    // Thick, compact check mark.
    tft.drawLine(x, y + 6, x + 5, y + 11, color);
    tft.drawLine(x + 5, y + 11, x + 13, y + 1, color);
    tft.drawLine(x, y + 7, x + 5, y + 12, color);
    tft.drawLine(x + 5, y + 12, x + 14, y + 1, color);
}

void drawHeaderConnectivityIcons() {
    const bool wifiConnected = checkWiFiConnection();
    const bool internetAndMqttConnected = isMqttConnected();
    
    // Clear only the icon area.
    tft.fillRect(305, 3, 170, 38, BRAND_GREEN_DARK);

    // Smaller Wi-Fi icon with a clearly separated status mark.
    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(BRAND_WHITE, BRAND_GREEN_DARK);

    // Move upward and make bolder
    tft.drawString("WI-FI", 314, 18, 2);
    tft.drawString("WI-FI", 315, 18, 2);
    tft.drawString("WI-FI", 314, 19, 2);

    if (wifiConnected) {
        drawStatusTick(361, 14, BRAND_MINT);
    } else {
        drawStatusCross(361, 14, BRAND_ERROR);
    }

    // Smaller internet/MQTT cloud icon with a proper tick or X.
    drawCloudIcon(420, 20, BRAND_WHITE);
    if (internetAndMqttConnected) {
        drawStatusTick(442, 14, BRAND_MINT);
    } else {
        drawStatusCross(442, 14, BRAND_ERROR);
    }
}


void drawStatusCross(int16_t x, int16_t y, uint16_t color) {
    // Proper thick X made from both diagonal strokes.
    const int16_t size = 12;
    tft.drawLine(x, y, x + size, y + size, color);
    tft.drawLine(x + 1, y, x + size + 1, y + size, color);
    tft.drawLine(x, y + size, x + size, y, color);
    tft.drawLine(x + 1, y + size, x + size + 1, y, color);
}

void drawCloudIcon(int16_t x, int16_t y, uint16_t color) {
    // Approximately 85% of the previous cloud size.
    tft.fillCircle(x - 7, y + 2, 6, color);
    tft.fillCircle(x, y - 2, 8, color);
    tft.fillCircle(x + 8, y + 2, 6, color);
    tft.fillRoundRect(x - 13, y, 27, 9, 4, color);
}
