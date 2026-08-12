#ifndef DEVICE_CONTROL_H
#define DEVICE_CONTROL_H

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>


extern  const uint16_t BRAND_GREEN;      
extern  const uint16_t BRAND_GREEN_DARK;  
extern  const uint16_t BRAND_MINT;      
extern  const uint16_t BRAND_BG;         
extern  const uint16_t BRAND_PANEL;      
extern  const uint16_t BRAND_PANEL_2;    
extern  const uint16_t BRAND_WHITE;      
extern  const uint16_t BRAND_MUTED;
extern  const uint16_t BRAND_WARNING;
extern  const uint16_t BRAND_ERROR;

extern const int16_t SCREEN_WIDTH;
extern const int16_t SCREEN_HEIGHT;
extern uint8_t wifiAnimationFrame;

extern bool dashboardVisible;
extern bool reconnectScreenVisible;
extern bool internetAvailable;
extern String message_display ;
extern unsigned long  status_duration ;
extern unsigned long status_millis;


struct SensorValues
{
    float temperature;
    float humidity;
    uint16_t co2;
    float waterLevel;
    float ec11;
    float ph11;
    float waterTemperature11;
    float ec12;
    float ph12;
    float waterTemperature12;
    float ec21;
    float ph21;
    float waterTemperature21;
    float ec22;
    float ph22;
    float waterTemperature22;
};

// Declaration only.
// The actual variable is created in SensorData.cpp.
extern SensorValues sensors;
void updateControllerMessage(const String& message);
/*
  INNOFarms.AI RGB565 logo configuration

  1. Convert the PNG/JPG logo into an RGB565 C array.
  2. Replace the placeholder array below with the generated array.
  3. Set INNOFARMS_LOGO_READY to 1.
  4. Set the correct width and height.

  Recommended logo size for the ESP8266:
      200 x 80 pixels

  If the displayed colours are incorrect, change
  INNOFARMS_LOGO_SWAP_BYTES from true to false.
*/

#define INNOFARMS_LOGO_READY 0
#define INNOFARMS_LOGO_WIDTH 1
#define INNOFARMS_LOGO_HEIGHT 1

// Set to 1 when the image itself already contains the text "INNOFarms.AI".
#define INNOFARMS_LOGO_CONTAINS_TEXT 0

// Change to false if the RGB565 colours appear swapped or incorrect.
#define INNOFARMS_LOGO_SWAP_BYTES true

/*
  Replace this placeholder with your converted RGB565 array, for example:

  #define INNOFARMS_LOGO_READY 1
  #define INNOFARMS_LOGO_WIDTH 200
  #define INNOFARMS_LOGO_HEIGHT 80

  const uint16_t innofarmsLogo[
      INNOFARMS_LOGO_WIDTH * INNOFARMS_LOGO_HEIGHT
  ] PROGMEM = {
      0x0000, 0x0000, 0x04D0, ...
  };
*/

const uint16_t innofarmsLogo[
    INNOFARMS_LOGO_WIDTH * INNOFARMS_LOGO_HEIGHT
] PROGMEM = {
    0x04D0
};

// Set all running conditions to false
void clearControllerStatus();

// =========================================================
// DISPLAY INITIALIZATION
// =========================================================

// Initialize SPI and TFT
void initDevices();

// Show startup screen
void showStartupLogo();

// =========================================================
// WI-FI DISPLAY FUNCTIONS
// =========================================================

// Show initial Wi-Fi or reconnection screen
void showWiFiConnectingScreen(bool reconnecting);

// Draw Wi-Fi connection animation
void drawWiFiAnimation(uint8_t frame);

// =========================================================
// MAIN DASHBOARD FUNCTIONS
// =========================================================

// Draw complete dashboard
void drawMainDashboard();

// Draw top dashboard header
void drawDashboardHeader();

// Draw Wi-Fi and MQTT status
void drawHeaderConnectivityIcons();

// =========================================================
// SENSOR DISPLAY FUNCTIONS
// =========================================================

// Draw all static sensor cards
void drawSensorLayout();

// Draw one small sensor card
void drawSmallSensorCard(
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    const char* label
);

// Update only sensor values
void updateSensorValuesOnly();

// =========================================================
// WATER-QUALITY DISPLAY FUNCTIONS
// =========================================================

// Draw complete water-quality panel
void drawWaterQualityPanel();

// Update active unit and zone text
void updateWaterQualityContext();

// Rotate water-quality values across Unit/Zone combinations
void rotateWaterQualityDisplay();

// Update active unit and zone variables
void updateActiveUnitZoneFromStatus();

// =========================================================
// CONTROLLER FOOTER
// =========================================================

// Draw controller-status message at bottom
void drawControllerStatusLine();

// =========================================================
// ICON-DRAWING FUNCTIONS
// =========================================================

// Draw connection check mark
void drawStatusTick(
    int16_t x,
    int16_t y,
    uint16_t color
);

// Draw connection cross mark
void drawStatusCross(
    int16_t x,
    int16_t y,
    uint16_t color
);

// Draw cloud icon
void drawCloudIcon(
    int16_t x,
    int16_t y,
    uint16_t color
);



#endif
