#include "config.h"
#include <Preferences.h> // <-- Move this to the top of the file, outside any function
#include <math.h> // Include math library for calculations

// --- ICON BITMAPS ---
// Add these at the top of your file, before any function that uses them

static const unsigned char PROGMEM image_Battery_Icon_bits[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x7f,0xff,0xf0,0x80,0x00,0x08,0x80,0x00,0x08,0x80,0x00,0x0e,0x80,0x00,0x01,0x80,0x00,0x01,0x80,0x00,0x01,0x80,0x00,0x01,0x80,0x00,0x0e,0x80,0x00,0x08,0x80,0x00,0x08,0x7f,0xff,0xf0,0x00,0x00,0x00};
static const unsigned char PROGMEM image_BLE_Icon_bits[] = {0x07,0xc0,0x18,0x30,0x21,0x08,0x41,0x84,0x49,0x44,0x85,0x22,0x83,0x42,0x81,0x82,0x83,0x42,0x85,0x22,0x49,0x44,0x41,0x84,0x21,0x08,0x18,0x30,0x07,0xc0};
static const unsigned char PROGMEM image_Home_Icon_bits[] = {0x01,0x00,0x12,0x80,0x1c,0x40,0x19,0x20,0x12,0x90,0x24,0x48,0x48,0x24,0x90,0x12,0x66,0x0c,0x26,0x08,0x20,0xe8,0x20,0xa8,0x20,0xe8,0x20,0xa8,0x3f,0xf8};
static const unsigned char PROGMEM image_NO_FLY_ZONE_Icon_bits[] = {0x0f,0xe0,0x10,0x10,0x27,0xc8,0x48,0x24,0x90,0x12,0xa4,0x4a,0xa2,0x8a,0xa1,0x0a,0xa2,0x8a,0xa4,0x4a,0x90,0x12,0x48,0x24,0x27,0xc8,0x10,0x10,0x0f,0xe0,0x00,0x00};
static const unsigned char PROGMEM image_Parked_car_icon_bits[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xf0,0x00,0x0c,0x88,0x00,0x18,0x84,0x00,0x7f,0xff,0xc0,0xf7,0xfd,0xa0,0xeb,0xfa,0xe0,0x55,0xf5,0x40,0x08,0x02,0x00};
static const unsigned char PROGMEM image_plane_for_center_of_screen_bits[] = {0xe0,0x00,0x90,0x0e,0x88,0x71,0x45,0x83,0x22,0x0c,0x10,0x30,0x08,0xc0,0x10,0x80,0x13,0x40,0x22,0xa0,0x24,0x5e,0x24,0x31,0x48,0x26,0x48,0x28,0x50,0x28,0x30,0x10};
static const unsigned char PROGMEM image_POI_Icon__bits[] = {0x0f,0x80,0x30,0x60,0x40,0x10,0x47,0x10,0x88,0x88,0x90,0x48,0x90,0x48,0x50,0x50,0x48,0x90,0x27,0x20,0x20,0x20,0x10,0x40,0x08,0x80,0x05,0x00,0x07,0x00,0x02,0x00};
static const unsigned char PROGMEM image_Sat_Connection_Icon_bits[] = {0x07,0xc0,0x18,0x30,0x27,0xc8,0x48,0x24,0x93,0x92,0xa4,0x4a,0xa9,0x2a,0xa3,0x8a,0x06,0xc0,0x03,0x80,0x01,0x00,0x03,0x80,0x02,0x80,0x06,0xc0,0x04,0x40,0x00,0x00};
static const unsigned char PROGMEM image_Sat_Count_Icon_bits[] = {0x00,0x0e,0x00,0x0a,0x00,0x0a,0x00,0x0a,0x00,0xea,0x00,0xaa,0x00,0xaa,0x00,0xaa,0x0e,0xaa,0x0e,0xaa,0x0e,0xaa,0x0e,0xaa,0xee,0xaa,0xee,0xaa,0xee,0xee,0x00,0x00};
static const unsigned char PROGMEM image_Save_Icon_bits[] = {0x7f,0xfc,0x90,0xaa,0x90,0xa9,0x90,0xe9,0x90,0x09,0x8f,0xf1,0x80,0x01,0x80,0x01,0x80,0x01,0x9f,0xf9,0x90,0x09,0x97,0xe9,0x90,0x09,0xd7,0xeb,0x90,0x09,0x7f,0xfe};
static const unsigned char PROGMEM image_Take_off_location_icon_bits[] = {0x00,0x00,0x80,0x00,0xcf,0x00,0xb6,0xf0,0xab,0x68,0xab,0x68,0x97,0xf0,0x9c,0x00,0xa0,0x00,0xc0,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x00,0x00};

// --- Color definitions ---
#define TURQUOISE 0x30FF // RGB565 for turquoise (can adjust as needed)

// Touch panel mode enum
enum {
    TOUCH_ACTIVITY_MODE,
    TOUCH_MONITOR_MODE,
    TOUCH_SLEEP_MODE,
};

TTGOClass *ttgo = nullptr;
TFT_eSPI *tft = nullptr;
TinyGPSPlus *gps = nullptr;
AXP20X_Class *power = nullptr;  // Add power management instance
BMA *sensor = nullptr;  // BMA423 accelerometer instance
PCF8563_Class *rtc = nullptr;  // RTC instance

Preferences prefs; // <-- Declare globally, not inside the function

// Touch and interrupt related variables
int16_t touchX = 0, touchY = 0;
bool pmuIrq = false;
bool sleepIrq = false;  // For deep sleep interrupt
uint8_t touchStatus = TOUCH_ACTIVITY_MODE;

int16_t touchStartX = 0;
int16_t touchStartY = 0;
bool isClockScreen = true;  // Track which screen is currently shown
bool isExtraScreen = false; // Track if extra screen is currently shown
bool isFlightInfoScreen = false; // New screen state for Flight Information

// --- Globals for Clock Display Optimization ---
bool forceClockRedraw = true;  // Flag to force a full redraw of the clock screen
char prevTimeStrClock[9] = ""; // Stores the last time string actually drawn (HH:MM:SS)

// --- Globals for Saved Car Location ---
float savedCarLat = 0.0;
float savedCarLon = 0.0;
bool isCarLocationSaved = false;
const int CAR_ICON_WIDTH = 24; 
const int CAR_ICON_HEIGHT = 20;
static int16_t g_prevCarIconDrawX = -1, g_prevCarIconDrawY = -1; // Stores top-left of previously drawn car icon

// --- Globals for Saved Home Location ---
float savedHomeLat = 0.0;
float savedHomeLon = 0.0;
bool isHomeLocationSaved = false;
const int HOME_ICON_WIDTH = 20; 
const int HOME_ICON_HEIGHT = 15;

// --- Globals for Saved Take-off Location ---
float savedTakeOffLat = 0.0;
float savedTakeOffLon = 0.0;
bool isTakeOffLocationSaved = false;
const int TAKEOFF_ICON_WIDTH = 20;
const int TAKEOFF_ICON_HEIGHT = 16;

// --- Variables for home circle color feedback ---
unsigned long homeSavedTimestamp = 0;
const unsigned long homeCircleGreenDuration = 5000; // 5 seconds
bool showHomeSavedMessage = false;
unsigned long homeSavedMessageTimestamp = 0;

// Structure and storage for previous compass letter positions
struct Rect { int16_t x, y, w, h; };
static Rect prev_compass_rects[4] = {{-1,0,0,0}, {-1,0,0,0}, {-1,0,0,0}, {-1,0,0,0}};

TFT_eSprite *eSpLoaction = nullptr;
TFT_eSprite *eSpDate = nullptr;
TFT_eSprite *eSpTime = nullptr;
TFT_eSprite *eSpSpeed = nullptr;
TFT_eSprite *eSpSatellites = nullptr;
TFT_eSprite *eSpPower = nullptr;  // New sprite for power info
TFT_eSprite *eSpRTC = nullptr;  // New sprite for RTC display
HardwareSerial *GNSS = NULL;

uint32_t last = 0;
uint32_t updateTimeout = 0;
uint32_t lastVibration = 0;  // Track last vibration time
uint32_t lastPowerCheck = 0;  // Track last power check time

int16_t prevTimeX = 0;
int16_t prevTimeY = 0;
int16_t prevTimeWidth = 0;
int16_t prevTimeHeight = 0;

struct Ball {
    float x;
    float y;
    float vx;
    float vy;
    int radius;
};

struct Hole {
    int x;
    int y;
    int radius;
};

// Update GameState struct to include initial hole size
struct GameState {
    Ball ball;
    Hole hole;
    uint32_t startTime;
    uint32_t bestTime;
    int level;
    bool isPlaying;
    bool levelComplete;
    int initialHoleSize;  // Added to track initial hole size
} gameState;

// Add a buffer for scrolling GPS NMEA data
#define GPS_SCROLL_BUF_LINES 4
#define GPS_SCROLL_BUF_LINE_LEN 76
char gpsScrollBuf[GPS_SCROLL_BUF_LINES][GPS_SCROLL_BUF_LINE_LEN] = {0};
int gpsScrollBufHead = 0;

// The T-Watch 2020 V2 (and similar TTGO boards) does NOT have a built-in magnetometer (compass sensor).
// Heading/course is derived from GPS movement (gps->course), not from a hardware compass.
// If you need true compass heading when stationary, you must add an external magnetometer (e.g., HMC5883L, QMC5883L, or LSM303).

static char prevLatStr_nav[20] = "";
static char prevLonStr_nav[20] = "";
static uint8_t prevBattPercentage_nav = 255;
static bool prevCharging_nav = false;
static char prevAltStr_nav[25] = "";
static char prevSpeedStr_nav[25] = "";
static float prevHeading_nav = -1000.0; // Use an unlikely initial value
static bool prevCarLocSaved_nav = false;
static float prevCarDistance_nav = -1.0;

// --- Globals for Flight Info Screen ---
static bool flightScreenInitialized = false;
static bool forceAllFlightDynamicElementsRedraw = true;
static uint8_t prevFlightBattPercentage = 255; // To store previous battery percentage for flight screen
static bool prevFlightCharging = false;         // To store previous charging status for flight screen
static int prevFlightSatellites = -1;           // To store previous satellite count for flight screen
static bool prevFlightGpsFix = false;           // To store previous GPS fix status for flight screen
static bool prevFlightHomeSaved = false;        // To track if home icon needs redraw
static bool prevFlightCarSaved = false;         // To track if car icon needs redraw

// --- Globals for Nav Screen ---
static bool navScreenInitialized = false;
static bool forceAllNavDynamicElementsRedraw = true;

// Forward declarations for Nav Screen functions
void initNavScreenBackground();
void updateNavLatLon(bool forceRedraw);
void updateNavBattery(bool forceRedraw);
void updateNavAltitude(bool forceRedraw);
void updateNavSpeed(bool forceRedraw);
void updateNavCompass(bool forceRedraw);
void updateNavCarIcon(bool forceRedraw);

// Helper: Calculate distance (meters) and bearing (degrees) between two lat/lon points
float calcDistanceToHome(float lat1, float lon1, float lat2, float lon2) {
    // Haversine formula
    const float R = 6371000.0; // Earth radius in meters
    float dLat = radians(lat2 - lat1);
    float dLon = radians(lon2 - lon1);
    float a = sin(dLat/2)*sin(dLat/2) + cos(radians(lat1))*cos(radians(lat2))*sin(dLon/2)*sin(dLon/2);
    float c = 2 * atan2(sqrt(a), sqrt(1-a));
    return R * c;
}
float calcBearingToHome(float lat1, float lon1, float lat2, float lon2) {
    float dLon = radians(lon2 - lon1);
    float y = sin(dLon) * cos(radians(lat2));
    float x = cos(radians(lat1))*sin(radians(lat2)) - sin(radians(lat1))*cos(radians(lat2))*cos(dLon);
    float brng = atan2(y, x);
    brng = degrees(brng);
    return fmod((brng + 360.0), 360.0);
}

bool Quectel_L76X_Probe()
{
    bool result = false;
    GNSS->write("$PCAS03,0,0,0,0,0,0,0,0,0,0,,,0,0*02\r\n");
    delay(5);
    // Get version information
    GNSS->write("$PCAS06,0*1B\r\n");
    uint32_t startTimeout = millis() + 500;
    while (millis() < startTimeout) {
        if (GNSS->available()) {
            String ver = GNSS->readStringUntil('\r');
            // Get module info , If the correct header is returned,
            // it can be determined that it is the MTK chip
            int index = ver.indexOf("$");
            if (index != -1) {
                ver = ver.substring(index);
                if (ver.startsWith("$GPTXT,01,01,02")) {
                    Serial.println("L76K GNSS init succeeded, using L76K GNSS Module");
                    result = true;
                }
            }
        }
    }
    // Initialize the L76K Chip, use GPS + GLONASS
    GNSS->write("$PCAS04,5*1C\r\n");
    delay(250);
    // only ask for RMC and GGA
    GNSS->write("$PCAS03,1,0,0,0,1,0,0,0,0,0,,,0,0*02\r\n");
    delay(250);
    // Switch to Vehicle Mode, since SoftRF enables Aviation < 2g
    GNSS->write("$PCAS11,3*1E\r\n");
    return result;
}

void setFlag(void)
{
    pmuIrq = true;
}

void setSleepFlag(void)
{
    sleepIrq = true;
}

void goToSleep()
{
    // Show countdown
    for (int i = 5; i > 0; i--) {
        tft->fillScreen(TFT_BLACK);
        tft->setCursor(0, 0);
        tft->print("Going to sleep in ");
        tft->print(i);
        tft->println(" seconds");
        delay(1000);
    }
    tft->println("Sleep now ...");
    delay(1000);

    // Set screen and touch to sleep mode
    ttgo->displaySleep();

    // Power management for T-Watch 2020 V2
    #ifdef LILYGO_WATCH_2020_V2
        // Keep essential functions, disable others
        power->setPowerOutPut(AXP202_LDO3, false);
        power->setPowerOutPut(AXP202_LDO4, false);
        power->setPowerOutPut(AXP202_LDO2, false);
        power->setPowerOutPut(AXP202_EXTEN, false);
        power->setPowerOutPut(AXP202_DCDC2, false);
    #endif

    // Use ext1 for external wakeup
    esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);

    esp_deep_sleep_start();
}

void initBMA423()
{
    // Get BMA423 sensor instance
    sensor = ttgo->bma;
    
    // Accel parameter structure
    Acfg cfg;
    // Output data rate in Hz
    cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
    // G-range
    cfg.range = BMA4_ACCEL_RANGE_2G;
    // Bandwidth parameter
    cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
    // Filter performance mode
    cfg.perf_mode = BMA4_CONTINUOUS_MODE;

    // Configure the BMA423 accelerometer
    sensor->accelConfig(cfg);
    // Enable BMA423 accelerometer
    sensor->enableAccel();
}

void initRTC()
{
    rtc = ttgo->rtc;
    
    // Set RTC time (uncomment and adjust if you need to set the time)
    // RTC_Date datetime = {
    //     .year = 2025,
    //     .month = 5,
    //     .day = 11,
    //     .hour = 12,
    //     .minute = 0,
    //     .second = 0
    // };
    // rtc->setDateTime(datetime);
    
    // Enable RTC alarm interrupt
    pinMode(RTC_INT_PIN, INPUT_PULLUP);
    attachInterrupt(RTC_INT_PIN, [] {
        // RTC interrupt handler
    }, FALLING);
}

void displayClock() {
    if (!ttgo || !ttgo->rtc || !ttgo->tft) {
        Serial.println("Error: Display components not initialized for clock");
        return;
    }

    RTC_Date now = ttgo->rtc->getDateTime();
    char currentTimeStr[9];
    sprintf(currentTimeStr, "%02d:%02d:%02d", now.hour, now.minute, now.second);

    if (forceClockRedraw) {
        Serial.println("Current Screen: Clock (Full Redraw)");
        ttgo->tft->fillScreen(TFT_BLACK);
        ttgo->tft->setTextSize(1); // Ensure consistent text size
        ttgo->tft->setTextFont(6); // Ensure consistent font
        ttgo->tft->setTextColor(TFT_CYAN);

        int16_t x = (ttgo->tft->width() - ttgo->tft->textWidth(currentTimeStr, 6)) / 2;
        int16_t y = (ttgo->tft->height() - ttgo->tft->fontHeight(6)) / 2;
        
        ttgo->tft->drawString(currentTimeStr, x, y, 6);

        strcpy(prevTimeStrClock, currentTimeStr);
        prevTimeX = x;
        prevTimeY = y;
        prevTimeWidth = ttgo->tft->textWidth(currentTimeStr, 6);
        prevTimeHeight = ttgo->tft->fontHeight(6);
        forceClockRedraw = false;
        return; 
    }

    if (strcmp(currentTimeStr, prevTimeStrClock) != 0) {
        Serial.println("Current Screen: Clock (Time Update)");
        // Clear previous time area
        if (prevTimeWidth > 0) { // Ensure there was a previous time to clear
            ttgo->tft->fillRect(prevTimeX, prevTimeY, prevTimeWidth, prevTimeHeight, TFT_BLACK);
        }

        // Draw new time
        ttgo->tft->setTextSize(1); // Ensure consistent text size
        ttgo->tft->setTextFont(6); // Ensure consistent font
        ttgo->tft->setTextColor(TFT_CYAN);

        int16_t x = (ttgo->tft->width() - ttgo->tft->textWidth(currentTimeStr, 6)) / 2;
        int16_t y = (ttgo->tft->height() - ttgo->tft->fontHeight(6)) / 2;

        ttgo->tft->drawString(currentTimeStr, x, y, 6);

        strcpy(prevTimeStrClock, currentTimeStr);
        prevTimeX = x;
        prevTimeY = y;
        prevTimeWidth = ttgo->tft->textWidth(currentTimeStr, 6);
        prevTimeHeight = ttgo->tft->fontHeight(6);
    }
}

bool isLastSundayOfMonth(int year, int month, int day) {
    // Get the last day of the month
    int daysInMonth = 31; // For March and October
    if (month == 3 || month == 10) {
        // Check if this day is a Sunday and there are no more Sundays in this month
        RTC_Date tempDate(year, month, day, 0, 0, 0);
        // Get day of week (0 = Sunday, 1 = Monday, etc.)
        int dayOfWeek = (tempDate.day + 2 * tempDate.month + 3 * (tempDate.month + 1) / 5 +
                        tempDate.year + tempDate.year / 4 - tempDate.year / 100 +
                        tempDate.year / 400) % 7;
        
        return (dayOfWeek == 0) && (day + 7 > daysInMonth);
    }
    return false;
}

bool isBST(const RTC_Date &date) {
    // BST starts at 1 AM UTC on the last Sunday of March
    // and ends at 1 AM UTC on the last Sunday of October
    
    if (date.month < 3 || date.month > 10) {
        return false;  // Definitely not BST
    }
    
    if (date.month > 3 && date.month < 10) {
        return true;  // Definitely BST
    }
    
    // Handle March and October specially
    if (date.month == 3) {
        // Before last Sunday = not BST
        // After last Sunday = BST
        for (int day = 31; day >= date.day; day--) {
            if (isLastSundayOfMonth(date.year, 3, day)) {
                // Found last Sunday
                if (date.day < day) return false;
                if (date.day > day) return true;
                // On the day, compare time (switch happens at 1AM UTC)
                return date.hour >= 1;
            }
        }
    }
    
    if (date.month == 10) {
        // Before last Sunday = BST
        // After last Sunday = not BST
        for (int day = 31; day >= date.day; day--) {
            if (isLastSundayOfMonth(date.year, 10, day)) {
                // Found last Sunday
                if (date.day < day) return true;
                if (date.day > day) return false;
                // On the day, compare time (switch happens at 1AM UTC)
                return date.hour < 1;
            }
        }
    }
    
    return false;  // Default case
}

void syncTimeFromGPS() {
    if (gps && gps->time.isValid() && gps->date.isValid()) {
        // Get GPS time (UTC)
        RTC_Date datetime(
            gps->date.year(),
            gps->date.month(),
            gps->date.day(),
            gps->time.hour(),
            gps->time.minute(),
            gps->time.second()
        );
        
        // Check if we should apply BST
        bool shouldApplyBST = isBST(datetime);
        
        // Apply BST if needed
        if (shouldApplyBST) {
            datetime.hour = (datetime.hour + 1) % 24;
        }
        
        // Update RTC with adjusted time
        ttgo->rtc->setDateTime(datetime);
        Serial.print("RTC time synchronized with GPS (");
        Serial.print(shouldApplyBST ? "BST" : "GMT");
        Serial.println(" applied)");
    }
}

void switchScreen(bool toClockScreen) {
    // Vibrate for 1 second when changing screen
    if (ttgo && ttgo->drv) {
        ttgo->drv->go();
        delay(1000);
        ttgo->drv->stop();
    }
    isClockScreen = toClockScreen;
    isExtraScreen = false;
    if (isClockScreen) {
        displayClock();
    } else {
        // Clear previous screen
        ttgo->tft->fillScreen(TFT_BLACK);
        // Show GPS info screen - enable sprite visibility
        if (eSpLoaction) eSpLoaction->pushSprite(0, 0);
        if (eSpDate) eSpDate->pushSprite(0, 49);
        if (eSpTime) eSpTime->pushSprite(0, 49 * 2);
        if (eSpSatellites) eSpSatellites->pushSprite(0, 49 * 3);
        if (eSpPower) eSpPower->pushSprite(0, 49 * 4);
    }
}

void initGame() {
    gameState.ball = {120, 120, 0, 0, 5};  // Start in center
    gameState.initialHoleSize = 50;  // Start with 100px diameter hole (50px radius)
    // Calculate current level's hole size
    int currentHoleSize = max(8, gameState.initialHoleSize - ((gameState.level - 1) * 5));
    gameState.hole = {
        random(currentHoleSize, 240-currentHoleSize),  // Keep hole fully on screen
        random(currentHoleSize, 240-currentHoleSize),
        currentHoleSize
    };
    gameState.startTime = millis();
    gameState.level = 1;
    gameState.isPlaying = true;
    gameState.levelComplete = false;
}

void drawGame() {
    ttgo->tft->fillScreen(TFT_BLACK);
    
    // Draw hole
    ttgo->tft->fillCircle(gameState.hole.x, gameState.hole.y, gameState.hole.radius, TFT_RED);
    
    // Draw ball
    ttgo->tft->fillCircle(gameState.ball.x, gameState.ball.y, gameState.ball.radius, TFT_WHITE);
    
    // Draw level and time
    ttgo->tft->setTextSize(1);
    ttgo->tft->setTextFont(2);
    ttgo->tft->setTextColor(TFT_GREEN);
    
    char buf[32];
    sprintf(buf, "Level: %d", gameState.level);
    ttgo->tft->drawString(buf, 5, 5, 2);
    
    uint32_t elapsed = (millis() - gameState.startTime) / 1000;
    sprintf(buf, "Time: %ds", elapsed);
    ttgo->tft->drawString(buf, 5, 25, 2);
}

void updateBall() {
    Accel acc;
    if (sensor->getAccel(acc)) {
        // Invert axis and reduce sensitivity
        gameState.ball.vx -= acc.y * 0.2;  // Inverted Y axis
        gameState.ball.vy += acc.x * 0.2;  // Inverted X axis
        
        // Increased friction from 0.95 to 0.92 to slow ball down more quickly
        gameState.ball.vx *= 0.92;
        gameState.ball.vy *= 0.92;
        
        // Update position
        gameState.ball.x += gameState.ball.vx;
        gameState.ball.y += gameState.ball.vy;
        
        // Screen boundaries
        if (gameState.ball.x < gameState.ball.radius) {
            gameState.ball.x = gameState.ball.radius;
            gameState.ball.vx = 0;
        }
        if (gameState.ball.x > 240 - gameState.ball.radius) {
            gameState.ball.x = 240 - gameState.ball.radius;
            gameState.ball.vx = 0;
        }
        if (gameState.ball.y < gameState.ball.radius) {
            gameState.ball.y = gameState.ball.radius;
            gameState.ball.vy = 0;
        }
        if (gameState.ball.y > 240 - gameState.ball.radius) {
            gameState.ball.y = 240 - gameState.ball.radius;
            gameState.ball.vy = 0;
        }
        
        // Check if ball is in hole
        float dx = gameState.ball.x - gameState.hole.x;
        float dy = gameState.ball.y - gameState.hole.y;
        float distance = sqrt(dx * dx + dy * dy);
        
        if (distance < gameState.hole.radius) {
            gameState.levelComplete = true;
            uint32_t levelTime = (millis() - gameState.startTime) / 1000;
            
            // Show completion message
            ttgo->tft->fillScreen(TFT_BLACK);
            ttgo->tft->setTextSize(1);
            ttgo->tft->setTextFont(4);
            ttgo->tft->setTextColor(TFT_GREEN);
            
            char buf[64];
            sprintf(buf, "Level %d Complete!", gameState.level);
            ttgo->tft->drawString(buf, 20, 60, 4);
            
            sprintf(buf, "Time: %ds", levelTime);
            ttgo->tft->drawString(buf, 20, 100, 4);
            
            sprintf(buf, "Next hole: %dpx", max(8, gameState.initialHoleSize - (gameState.level * 5)));
            ttgo->tft->drawString(buf, 20, 140, 4);
            
            // Vibrate to indicate success
            ttgo->drv->go();
            
            delay(2000);  // Show completion message
            
            // Start next level
            gameState.level++;
            // Reset ball position
            gameState.ball = {120, 120, 0, 0, 5};
            // Calculate new hole size for next level
            int newHoleSize = max(8, gameState.initialHoleSize - ((gameState.level - 1) * 5));
            // Place new hole with new size
            gameState.hole = {
                random(newHoleSize, 240-newHoleSize),
                random(newHoleSize, 240-newHoleSize),
                newHoleSize
            };
            gameState.startTime = millis();
            gameState.levelComplete = false;
        }
    }
}

void startBallGame() {
    initGame();
    while (gameState.isPlaying) {
        // Handle touch to exit
        int16_t x, y;
        if (ttgo->getTouch(x, y)) {
            gameState.isPlaying = false;
            break;
        }
        
        updateBall();
        drawGame();
        delay(16);  // ~60 FPS
    }
    // Return to clock screen
    forceClockRedraw = true; // Ensure clock screen does a full redraw
    navScreenInitialized = false; 
    flightScreenInitialized = false;
    isClockScreen = true; // Explicitly set screen state before calling switchScreen
    isExtraScreen = false;
    isFlightInfoScreen = false;
    switchScreen(true);
}

void saveHomeLocationToEEPROM(float lat, float lon) {
    prefs.begin("homeloc", false); // Use "homeloc" namespace
    prefs.putFloat("homeLat", lat);
    prefs.putFloat("homeLon", lon);
    prefs.end();
    
    savedHomeLat = lat;
    savedHomeLon = lon;
    isHomeLocationSaved = true;
    Serial.println("Home location saved.");
    // Force dynamic elements redraw to update icon color if on flight screen
    forceAllFlightDynamicElementsRedraw = true; 
}

void initFlightInfoScreen() {
    Serial.println("Current Screen: Flight Info (Initializing Background)");
    tft->fillScreen(TFT_BLACK); // Set background to black
    // Move top row icons to start at the far left
    tft->drawBitmap(0, 2, image_Battery_Icon_bits, 24, 16, TFT_WHITE);
    tft->drawBitmap(31, 3, image_BLE_Icon_bits, 15, 15, TFT_WHITE);
    tft->drawBitmap(55, 2, image_Sat_Count_Icon_bits, 15, 16, TFT_GREEN);
    tft->drawBitmap(78, 2, image_Sat_Connection_Icon_bits, 15, 16, TFT_GREEN);
    // Remove the white TX RX Data box
    // Center plane ring and plane (keep as before)
    tft->drawBitmap(112, 112, image_plane_for_center_of_screen_bits, 16, 16, TFT_WHITE);
}

void updateFlightInfoScreenDynamicElements(bool forceRedraw) {
    if (!tft || !power || !gps) return;

    // Update Battery Icon
    uint8_t currentBattPercentage = power->getBattPercentage();
    bool currentCharging = power->isChargeing();
    uint16_t batteryColor = TFT_GREEN;
    if (currentBattPercentage < 20) batteryColor = TFT_RED;
    else if (currentBattPercentage < 50) batteryColor = TFT_YELLOW;
    else batteryColor = TFT_GREEN;
    if (forceRedraw || currentBattPercentage != prevFlightBattPercentage || currentCharging != prevFlightCharging) {
        tft->drawBitmap(0, 2, image_Battery_Icon_bits, 24, 16, batteryColor);
        int fillWidth = map(currentBattPercentage, 0, 100, 0, 16);
        tft->fillRect(4, 6, fillWidth, 6, batteryColor);
        prevFlightBattPercentage = currentBattPercentage;
        prevFlightCharging = currentCharging;
    }

    // Update Satellite Count Icon & Connection Icon
    int currentSatellites = gps->satellites.value();
    bool currentGpsFix = gps->location.isValid() && gps->location.age() < 5000; // Consider a reasonable age for "fix"

    uint16_t gpsColor = TFT_RED;
    if (currentSatellites > 3) gpsColor = TFT_BLUE;
    else if (currentSatellites == 3) gpsColor = TFT_GREEN;
    else gpsColor = TFT_RED;
    if (forceRedraw || currentSatellites != prevFlightSatellites) {
        tft->drawBitmap(55, 2, image_Sat_Count_Icon_bits, 15, 16, gpsColor);
        prevFlightSatellites = currentSatellites;
    }

    if (forceRedraw || currentGpsFix != prevFlightGpsFix) {
        tft->drawBitmap(78, 2, image_Sat_Connection_Icon_bits, 15, 16, currentGpsFix ? TFT_GREEN : TFT_RED);
        prevFlightGpsFix = currentGpsFix;
    }

    // Update Home Icon if its saved state changed
    if (forceRedraw || isHomeLocationSaved != prevFlightHomeSaved) {
        prevFlightHomeSaved = isHomeLocationSaved;
    }

    // Update Car Icon if its saved state changed
    if (forceRedraw || isCarLocationSaved != prevFlightCarSaved) {
        prevFlightCarSaved = isCarLocationSaved;
    }

    // --- GPS Data Serial Output for Flight Info Screen ---
    static uint32_t lastFlightSerial = 0;
    if (millis() - lastFlightSerial > 1000) {
        lastFlightSerial = millis();
        if (gps->location.isValid()) {
            Serial.print("[FLIGHT] Lat: "); Serial.print(gps->location.lat(), 6);
            Serial.print(" Lon: "); Serial.print(gps->location.lng(), 6);
        } else {
            Serial.print("[FLIGHT] Lat/Lon: INVALID");
        }
        Serial.print("  Alt: ");
        if (gps->altitude.isValid()) {
            Serial.print(gps->altitude.meters(), 1);
            Serial.print(" m");
        } else {
            Serial.print("INVALID");
        }
        Serial.print("  Speed: ");
        if (gps->speed.isValid()) {
            Serial.print(gps->speed.kmph(), 1);
            Serial.print(" km/h");
        } else {
            Serial.print("INVALID");
        }
        Serial.print("  Sats: ");
        Serial.println(gps->satellites.value());
    }

    // --- Draw Altitude and Speed in bottom boxes ---
    int boxY = 200; // Y position for both boxes
    int boxHeight = 30;
    // Move both boxes 20px to the left
    int altBoxX = 3;   // 23 - 20
    int spdBoxX = 104; // 124 - 20
    // Left box: Altitude
    tft->fillRect(altBoxX, boxY, 98, boxHeight, TFT_BLACK);
    tft->setTextFont(4);
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setCursor(altBoxX + 5, boxY + 6);
    if (gps->altitude.isValid()) {
        char altBuf[24];
        sprintf(altBuf, "Alt: %.1f m", gps->altitude.meters());
        tft->print(altBuf);
    } else {
        tft->print("Alt: ---");
    }
    // Right box: Speed
    tft->fillRect(spdBoxX, boxY, 98, boxHeight, TFT_BLACK);
    tft->setTextFont(4);
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setCursor(spdBoxX + 5, boxY + 6);
    if (gps->speed.isValid()) {
        char spdBuf[24];
        sprintf(spdBuf, "Speed: %.1f km/h", gps->speed.kmph());
        tft->print(spdBuf);
    } else {
        tft->print("Speed: ---");
    }

    // --- Draw distance to home value at top right corner ---
    if (isFlightInfoScreen && isHomeLocationSaved && gps && gps->location.isValid()) {
        float dist = calcDistanceToHome(gps->location.lat(), gps->location.lng(), savedHomeLat, savedHomeLon);
        // Draw value at top right (no label)
        String distStr;
        if (dist < 500) {
            distStr = String((int)dist) + " m";
        } else {
            distStr = String(dist / 1000.0, 1) + " km";
        }
        int distWidth = tft->textWidth(distStr.c_str());
        int distHeight = tft->fontHeight();
        int distX = 240 - distWidth - 4; // 4px padding from right edge
        int distY = 4; // 4px from top
        tft->setTextColor(TFT_WHITE, TFT_BLACK);
        tft->drawString(distStr.c_str(), distX, distY);
        // --- Draw direction icon and dotted line ---
        float maxRadius = 100.0f; // Outer edge of screen (approx 15km)
        float r = (dist >= 15000) ? maxRadius : (dist/15000.0f)*maxRadius;
        float angleRad = radians(calcBearingToHome(gps->location.lat(), gps->location.lng(), savedHomeLat, savedHomeLon) - 90); // 0 deg = up
        int iconX = 120 + (int)(r * cos(angleRad));
        int iconY = 120 + (int)(r * sin(angleRad));
        // Dotted line color by distance
        uint16_t lineColor = TFT_GREEN;
        if (dist > 10000) lineColor = TFT_RED;
        else if (dist > 5000) lineColor = TFT_BLUE;
        // Draw dotted line from center to icon
        for (float frac = 0.0; frac < 1.0; frac += 0.05) {
            if (((int)(frac*20))%2==0) {
                int dotX = 120 + (int)(frac*r * cos(angleRad));
                int dotY = 120 + (int)(frac*r * sin(angleRad));
                tft->drawPixel(dotX, dotY, lineColor);
            }
        }
        // Draw direction icon (take-off icon) at calculated position
        static bool flash = false;
        if (dist >= 15000) flash = !flash;
        if (dist < 15000 || flash) {
            tft->drawBitmap(iconX-8, iconY-8, image_Take_off_location_icon_bits, 16, 16, TFT_WHITE);
        }
    }

    // --- Drawing the turquoise blue ring and plane icon ---
    int planeCenterX = 120; // Center of 240x240
    int planeCenterY = 120;
    int ringRadius = 20; // 40px diameter
    uint16_t turquoiseBlue = tft->color565(64, 224, 208); // Turquoise blue RGB(64,224,208)
    // Determine ring color based on satellite count
    uint16_t ringColor = turquoiseBlue;
    if (gps->satellites.value() < 3) {
        ringColor = tft->color565(255, 0, 0); // Red
    }
    tft->fillCircle(planeCenterX, planeCenterY, ringRadius, ringColor);
    // Draw the plane icon centered in the circle
    int planeIconW = 16, planeIconH = 16;
    tft->drawBitmap(planeCenterX - planeIconW/2, planeCenterY - planeIconH/2, image_plane_for_center_of_screen_bits, planeIconW, planeIconH, TFT_WHITE);

    // --- Show 'Home Saved' message for 2 seconds after saving ---
    if (showHomeSavedMessage && millis() - homeSavedMessageTimestamp < 2000) {
        tft->setTextColor(TFT_GREEN, TFT_BLACK);
        tft->setTextFont(4);
        tft->setCursor(planeCenterX - tft->textWidth("Home Saved")/2, planeCenterY - ringRadius - 30);
        tft->print("Home Saved");
    } else {
        showHomeSavedMessage = false;
    }

    // --- Draw Take-off Icon and ring if home is set ---
    if (isHomeLocationSaved) {
        int takeoffX = 30; // Example position, adjust as needed
        int takeoffY = 46;
        tft->drawBitmap(takeoffX, takeoffY, image_Take_off_location_icon_bits, TAKEOFF_ICON_WIDTH, TAKEOFF_ICON_HEIGHT, TFT_WHITE);
        // Draw a ring around the take-off icon (same size as center ring)
        int takeoffRingRadius = 20; // 40px diameter
        int takeoffCenterX = takeoffX + TAKEOFF_ICON_WIDTH/2;
        int takeoffCenterY = takeoffY + TAKEOFF_ICON_HEIGHT/2;
        tft->drawCircle(takeoffCenterX, takeoffCenterY, takeoffRingRadius, ringColor);
    }
}

void saveCarLocationToEEPROM(float lat, float lon) {
    prefs.begin("carpark", false); // Use "carpark" namespace
    prefs.putFloat("carLat", lat);
    prefs.putFloat("carLon", lon);
    prefs.end();
    
    savedCarLat = lat;
    savedCarLon = lon;
    isCarLocationSaved = true;
    Serial.println("Car location saved.");

    // Reset previous icon position so it gets cleared and redrawn correctly
    if (g_prevCarIconDrawX != -1 && tft) {
         tft->fillRect(g_prevCarIconDrawX, g_prevCarIconDrawY, CAR_ICON_WIDTH, CAR_ICON_HEIGHT, TFT_WHITE);
    }
    g_prevCarIconDrawX = -1; 
    g_prevCarIconDrawY = -1;
    forceAllFlightDynamicElementsRedraw = true; // To update icon color
}

// --- Nav Screen Function Definitions ---
void initNavScreenBackground() {
    // TODO: Implement Nav Screen background initialization
    Serial.println("NavScreen: initNavScreenBackground() called");
    if (tft) {
        // Example: Draw a simple background
        // tft->fillScreen(TFT_DARKCYAN);
        // tft->setTextColor(TFT_WHITE);
        // tft->setTextSize(2);
        // tft->setCursor(10, 10);
        // tft->println("NAV SCREEN");
    }
}

void updateNavLatLon(bool forceRedraw) {
    // TODO: Implement Nav Screen Lat/Lon update
    if (forceRedraw && gps && tft && gps->location.isValid()) {
        Serial.println("NavScreen: updateNavLatLon() called with forceRedraw");
        char buf[40]; // Increased buffer size
        tft->setTextFont(2); // Ensure font is set
        tft->setTextSize(1); // Ensure text size is set
        tft->setTextColor(TFT_BLACK, TFT_WHITE); // Set text color with white background to overwrite previous

        // Clear previous latitude display area (example coordinates, adjust as needed)
        // tft->fillRect(10, 50, 220, 18, TFT_WHITE); 
        sprintf(buf, "Lat: %.6f", gps->location.lat());
        // tft->drawString(buf, 10, 50); // Using default font if not specified by drawString overload

        // Clear previous longitude display area (example coordinates, adjust as needed)
        // tft->fillRect(10, 70, 220, 18, TFT_WHITE);
        sprintf(buf, "Lon: %.6f", gps->location.lng());
        // tft->drawString(buf, 10, 70);

        // For now, let's just print to Serial to confirm data access
        Serial.print("Nav Lat: "); Serial.println(gps->location.lat(), 6);
        Serial.print("Nav Lon: "); Serial.println(gps->location.lng(), 6);

        // Placeholder for actual drawing on TFT, e.g. using sprites or direct drawing
        // Example:
        // TFT_eSprite latLonSprite = TFT_eSprite(tft);
        // latLonSprite.createSprite(240, 40);
        // latLonSprite.fillSprite(TFT_WHITE);
        // latLonSprite.setTextColor(TFT_BLACK);
        // latLonSprite.setTextFont(2);
        // latLonSprite.setCursor(5,0);
        // latLonSprite.printf("Lat: %.6f", gps->location.lat());
        // latLonSprite.setCursor(5,20);
        // latLonSprite.printf("Lon: %.6f", gps->location.lng());
        // latLonSprite.pushSprite(0, 20); // Adjust Y position as needed
        // latLonSprite.deleteSprite();

        // strcpy(prevLatStr_nav, buf); // Update previous strings if implementing differential update
    } else if (forceRedraw && tft) {
        // tft->drawString("Lat: N/A", 10, 50, 2);
        // tft->drawString("Lon: N/A", 10, 70, 2);
        Serial.println("NavScreen: updateNavLatLon() - GPS data N/A");
    }
}

void updateNavBattery(bool forceRedraw) {
    // Implementation for updating the navigation screen battery icon
    // For now, we'll just print to Serial
    Serial.print("updateNavBattery called with forceRedraw: ");
    Serial.println(forceRedraw);
    // TODO: Add actual battery level reading and display logic
}

void updateNavAltitude(bool forceRedraw) {
    // TODO: Implement Nav Screen Altitude update
    if (forceRedraw && gps && tft && gps->altitude.isValid()) {
        Serial.println("NavScreen: updateNavAltitude() called with forceRedraw");
        char buf[25];
        tft->setTextFont(2);
        tft->setTextSize(1);
        tft->setTextColor(TFT_BLACK, TFT_WHITE);
        // tft->fillRect(10, 110, 220, 18, TFT_WHITE); 
        sprintf(buf, "Alt: %.1fm", gps->altitude.meters());
        // tft->drawString(buf, 10, 110);
        Serial.print("Nav Alt: "); Serial.println(gps->altitude.meters(), 1);
        // strcpy(prevAltStr_nav, buf);
    } else if (forceRedraw && tft) {
        // tft->drawString("Alt: N/A", 10, 110, 2);
        Serial.println("NavScreen: updateNavAltitude() - GPS data N/A");
    }
}

void updateNavSpeed(bool forceRedraw) {
    // TODO: Implement Nav Screen Speed update
    if (forceRedraw && gps && tft && gps->speed.isValid()) {
        Serial.println("NavScreen: updateNavSpeed() called with forceRedraw");
        char buf[25];
        tft->setTextFont(2);
        tft->setTextSize(1);
        tft->setTextColor(TFT_BLACK, TFT_WHITE);
        // tft->fillRect(10, 130, 220, 18, TFT_WHITE);
        sprintf(buf, "Speed: %.1fkm/h", gps->speed.kmph());
        // tft->drawString(buf, 10, 130);
        Serial.print("Nav Speed: "); Serial.println(gps->speed.kmph(), 1);
        // strcpy(prevSpeedStr_nav, buf);
    } else if (forceRedraw && tft) {
        // tft->drawString("Speed: N/A", 10, 130, 2);
        Serial.println("NavScreen: updateNavSpeed() - GPS data N/A");
    }
}

void updateNavCompass(bool forceRedraw) {
    // TODO: Implement Nav Screen Compass update
    if (forceRedraw && gps && tft && gps->course.isValid()) {
        Serial.println("NavScreen: updateNavCompass() called with forceRedraw");
        char buf[25];
        tft->setTextFont(2);
        tft->setTextSize(1);
        tft->setTextColor(TFT_BLACK, TFT_WHITE);
        // tft->fillRect(10, 150, 220, 18, TFT_WHITE);
        sprintf(buf, "Course: %.1f deg", gps->course.deg());
        // tft->drawString(buf, 10, 150);
        Serial.print("Nav Course: "); Serial.println(gps->course.deg(), 1);
        // prevHeading_nav = gps->course.deg();
    } else if (forceRedraw && tft) {
        // tft->drawString("Course: N/A", 10, 150, 2);
        Serial.println("NavScreen: updateNavCompass() - GPS data N/A");
    }
}

void updateNavCarIcon(bool forceRedraw) {
    // TODO: Implement Nav Screen Car Icon update
    if (forceRedraw && tft) {
        Serial.println("NavScreen: updateNavCarIcon() called with forceRedraw");
        // Example: Draw car icon if location is saved
        // if (isCarLocationSaved) {
        //     tft->drawBitmap(200, 10, image_Parked_car_icon_bits, CAR_ICON_WIDTH, CAR_ICON_HEIGHT, TFT_GREEN);
        // } else {
        //     tft->drawBitmap(200, 10, image_Parked_car_icon_bits, CAR_ICON_WIDTH, CAR_ICON_HEIGHT, TFT_RED);
        // }
    }
}

void setup(void)
{
    Serial.begin(115200);

    // Initialize watch
    ttgo = TTGOClass::getWatch();
    ttgo->begin();
    ttgo->openBL(); // Turn on the backlight
    
    // Get direct pointer to display
    tft = ttgo->tft;
    
    // Show clock first
    Serial.println("Displaying clock...");
    displayClock();
    delay(3000);  // Show for 3 seconds
    Serial.println("Clock display done");

    // Clear screen before continuing
    tft->fillScreen(TFT_BLACK);
    tft->setTextFont(2);
    tft->println("Begin GPS Module...");

    // Initialize DRV2605 haptic driver
    ttgo->enableDrv2650();
    ttgo->drv->selectLibrary(1);
    ttgo->drv->setMode(DRV2605_MODE_INTTRIG);

    // Initialize power management
    power = ttgo->power;

    // Enable ADC for power monitoring
    power->adc1Enable(
        AXP202_VBUS_VOL_ADC1 |
        AXP202_VBUS_CUR_ADC1 |
        AXP202_BATT_CUR_ADC1 |
        AXP202_BATT_VOL_ADC1,
        true);

    //Open gps power
    ttgo->trunOnGPS();

    ttgo->gps_begin();

    GNSS = ttgo->hwSerial;
    gps = ttgo->gps;

    if (!Quectel_L76X_Probe()) {
        tft->setCursor(0, 0);
        tft->setTextColor(TFT_RED);
        tft->println("GNSS Probe failed!"); while (1);
    }

    // Display on the screen, latitude and longitude, number of satellites, and date and time
    eSpLoaction   = new TFT_eSprite(tft); // Sprite object for eSpLoaction
    eSpDate   = new TFT_eSprite(tft); // Sprite object for eSpDate
    eSpTime   = new TFT_eSprite(tft); // Sprite object for eSpTime
    eSpSatellites   = new TFT_eSprite(tft); // Sprite object for eSpSatellites
    eSpPower   = new TFT_eSprite(tft); // Sprite object for eSpPower
    eSpRTC   = new TFT_eSprite(tft); // Sprite object for eSpRTC

    eSpLoaction->createSprite(240, 48);
    eSpLoaction->setTextFont(2);

    eSpDate->createSprite(240, 48);
    eSpDate->setTextFont(2);

    eSpTime->createSprite(240, 48);
    eSpTime->setTextFont(2);

    eSpSatellites->createSprite(240, 48);
    eSpSatellites->setTextFont(2);

    eSpPower->createSprite(240, 48);
    eSpPower->setTextFont(2);

    eSpRTC->createSprite(240, 48);
    eSpRTC->setTextFont(2);

    last = millis();

    // Initialize touch panel control
    ttgo->powerAttachInterrupt(setFlag);
    // Enable PEK button interrupt
    ttgo->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ, true);
    //Clear IRQ unprocessed first
    ttgo->power->clearIRQ();
    // Use touch panel interrupt as digital input
    pinMode(TOUCH_INT, INPUT);

    // Set up PEK button interrupt for sleep
    pinMode(AXP202_INT, INPUT_PULLUP);
    attachInterrupt(AXP202_INT, setSleepFlag, FALLING);

    // Initialize BMA423 accelerometer
    initBMA423();

    // Initialize RTC
    initRTC();

    // Load saved car location
    prefs.begin("carpark", true); // Read-only to check for existing values
    if (prefs.isKey("carLat") && prefs.isKey("carLon")) {
        savedCarLat = prefs.getFloat("carLat", 0.0);
        savedCarLon = prefs.getFloat("carLon", 0.0);
        if (savedCarLat != 0.0 || savedCarLon != 0.0) { // Check if not default values
            isCarLocationSaved = true;
            Serial.println("Loaded saved car location from EEPROM.");
        } else {
            Serial.println("Found carLat/carLon keys but they were zero, not loading.");
        }
    } else {
        Serial.println("No saved car location found in EEPROM.");
    }
    prefs.end();

    // Load saved Home location
    prefs.begin("homeloc", true);
    if (prefs.isKey("homeLat") && prefs.isKey("homeLon")) {
        savedHomeLat = prefs.getFloat("homeLat", 0.0);
        savedHomeLon = prefs.getFloat("homeLon", 0.0);
        if (savedHomeLat != 0.0 || savedHomeLon != 0.0) {
            isHomeLocationSaved = true;
            Serial.println("Loaded saved home location.");
        }
    } else { // Placeholder if not found
        Serial.println("No saved home location, using default/none.");
    }
    prefs.end();

    // Load saved Take-off location
    prefs.begin("takeoffloc", true);
    if (prefs.isKey("toLat") && prefs.isKey("toLon")) {
        savedTakeOffLat = prefs.getFloat("toLat", 0.0);
        savedTakeOffLon = prefs.getFloat("toLon", 0.0);
        if (savedTakeOffLat != 0.0 || savedTakeOffLon != 0.0) {
            isTakeOffLocationSaved = true;
            Serial.println("Loaded saved take-off location.");
        }
    } else { // Placeholder if not found
        Serial.println("No saved take-off location, using default/none.");
    }
    prefs.end();
}

void updatePowerInfo()
{
    if (millis() - lastPowerCheck >= 1000) {  // Update every second
        lastPowerCheck = millis();
        
        eSpPower->fillSprite(TFT_BLACK);
        eSpPower->setTextColor(TFT_YELLOW, TFT_BLACK);
        
        // Battery status
        if (power->isBatteryConnect()) {
            eSpPower->setCursor(0, 0);
            eSpPower->print("BAT: ");
            eSpPower->print(power->getBattVoltage());
            eSpPower->print("mV ");
            eSpPower->print(power->getBattPercentage());
            eSpPower->print("%");
            
            // Charging status
            eSpPower->setCursor(0, 16);
            if (power->isChargeing()) {
                eSpPower->setTextColor(TFT_GREEN, TFT_BLACK);
                eSpPower->print("CHG: ");
                eSpPower->print(power->getBattChargeCurrent());
                eSpPower->print("mA");
            } else {
                eSpPower->setTextColor(TFT_RED, TFT_BLACK);
                eSpPower->print("DISCHG: ");
                eSpPower->print(power->getBattDischargeCurrent());
                eSpPower->print("mA");
            }
        } else {
            eSpPower->setTextColor(TFT_RED, TFT_BLACK);
            eSpPower->print("NO BATTERY");
        }

        // USB status
        eSpPower->setCursor(0, 32);
        eSpPower->setTextColor(TFT_YELLOW, TFT_BLACK);
        if (power->isVBUSPlug()) {
            eSpPower->print("USB: ");
            eSpPower->print(power->getVbusVoltage());
            eSpPower->print("mV ");
            eSpPower->print(power->getVbusCurrent());
            eSpPower->print("mA");
        } else {
            eSpPower->print("USB: DISCONNECTED");
        }
        
        eSpPower->pushSprite(0, 49 * 4);  // Display below other sprites
    }
}

void updateRTCDisplay()
{
    RTC_Date now = rtc->getDateTime();
    
    eSpRTC->fillSprite(TFT_BLACK);
    eSpRTC->setTextColor(TFT_CYAN, TFT_BLACK);
    eSpRTC->setCursor(0, 0);
    
    // Format date and time strings
    char timeStr[20];
    char dateStr[20];
    sprintf(timeStr, "%02d:%02d:%02d", now.hour, now.minute, now.second);
    sprintf(dateStr, "%04d-%02d-%02d", now.year, now.month, now.day);
    
    eSpRTC->print("RTC Time: ");
    eSpRTC->println(timeStr);
    eSpRTC->print("Date: ");
    eSpRTC->println(dateStr);
    
    eSpRTC->pushSprite(0, 49 * 5);  // Display below power info
}

void handleTouchModeChange()
{
    if (pmuIrq) {
        pmuIrq = false;
        ttgo->power->readIRQ();

        // Press the PEK button to switch between different modes
        if (ttgo->power->isPEKShortPressIRQ()) {
            tft->fillScreen(TFT_BLACK);
            tft->setCursor(0, 0);
            switch (touchStatus) {
            case TOUCH_ACTIVITY_MODE:
                tft->println("Touch: Monitor Mode");
                ttgo->touchToMonitor();
                break;
            case TOUCH_MONITOR_MODE:
                tft->println("Touch: Sleep Mode");
                ttgo->touchToSleep();
                break;
            case TOUCH_SLEEP_MODE:
                tft->println("Touch: Active Mode");
                ttgo->touchWakup();
                break;
            default:
                break;
            }
            touchStatus++;
            touchStatus %= 3;

            // Redraw all sprites after mode change
            updatePowerInfo();
            delay(1000);  // Show mode change message briefly
        }
        ttgo->power->clearIRQ();
    }
}

void handleSleep()
{
    if (sleepIrq) {
        sleepIrq = false;
        power->clearIRQ();
        
        // Long press detection (optional)
        if (power->isPEKLongPressIRQ()) {
            goToSleep();
        }
        
        power->clearIRQ();
    }
}

void getAccelData()
{
    Accel acc;
    if (sensor->getAccel(acc)) {
        // Data available for use:
        // acc.x - X axis acceleration
        // acc.y - Y axis acceleration
        // acc.z - Z axis acceleration
    }
}

void loop(void) {
    // Always process GPS data, regardless of screen
    static char nmeaLine[GPS_SCROLL_BUF_LINE_LEN] = {0};
    static int nmeaLinePos = 0;
    bool nmeaLineAdded = false;
    while (GNSS->available()) {
        int r = GNSS->read();
        gps->encode(r);
        // --- Collect NMEA data for scrolling display ---
        if (r == '\n' || r == '\r') {
            if (nmeaLinePos > 0) {
                nmeaLine[nmeaLinePos] = 0;
                strncpy(gpsScrollBuf[gpsScrollBufHead], nmeaLine, GPS_SCROLL_BUF_LINE_LEN - 1);
                gpsScrollBuf[gpsScrollBufHead][GPS_SCROLL_BUF_LINE_LEN - 1] = 0;
                gpsScrollBufHead = (gpsScrollBufHead + 1) % GPS_SCROLL_BUF_LINES;
                // Print the full NMEA sentence to Serial for debugging
                if (isFlightInfoScreen) {
                    Serial.print("[NMEA] ");
                    Serial.print(nmeaLine);
                    Serial.print("  [GPS TIME] ");
                    if (gps->time.isValid()) {
                        Serial.printf("%02d:%02d:%02d", gps->time.hour(), gps->time.minute(), gps->time.second());
                    } else {
                        Serial.print("INVALID");
                    }
                    Serial.println();
                }
                nmeaLinePos = 0;
                nmeaLineAdded = true;
            }
        } else if (nmeaLinePos < GPS_SCROLL_BUF_LINE_LEN - 1) {
            nmeaLine[nmeaLinePos++] = (char)r;
        }
    }

    // Handle sleep mode and touch panel
    handleSleep();
    handleTouchModeChange();

    // Store initial touch coordinates for brightness control
    static uint8_t currentBrightness = 128; // Start with middle brightness

    // Handle touch gestures
    if (ttgo->getTouch(touchX, touchY)) {
        if (touchStartX == 0 && touchStartY == 0) {
            // Start of touch
            touchStartX = touchX;
            touchStartY = touchY;
        }
    } else { // Touch ended
        if (touchStartX != 0 || touchStartY != 0) { // Was a touch or swipe
            Serial.printf("Touch event: X=%d Y=%d\n", touchStartX, touchStartY);
            int16_t swipeX = touchX - touchStartX; // Final position used for swipe calculation
            int16_t swipeY = touchY - touchStartY; // Final position used for swipe calculation

            bool eventHandled = false; // Flag to indicate if a tap or swipe was processed

            // --- Tap detection (minimal movement) ---
            if (abs(swipeX) < 15 && abs(swipeY) < 15) { // Threshold for tap vs swipe
                // --- Tap in bottom left corner to return to clock ---
                if (touchStartX >= 0 && touchStartX <= 40 && touchStartY >= 200 && touchStartY <= 240 && !isClockScreen) {
                    if (ttgo && ttgo->drv) { ttgo->drv->go(); delay(1000); ttgo->drv->stop(); }
                    isClockScreen = true; isExtraScreen = false; isFlightInfoScreen = false;
                    forceClockRedraw = true;
                    navScreenInitialized = false; flightScreenInitialized = false;
                    eventHandled = true;
                } else if (touchStartX >= 200 && touchStartY >= 0 && touchStartY <= 40) {
                    // --- Tap in top right corner to increase brightness ---
                    currentBrightness = constrain(currentBrightness + 30, 10, 255);
                    ttgo->setBrightness(currentBrightness);
                    Serial.println("Brightness increased");
                    eventHandled = true;
                } else if (touchStartX >= 0 && touchStartX <= 40 && touchStartY >= 0 && touchStartY <= 40) {
                    // --- Tap in top left corner to decrease brightness ---
                    currentBrightness = constrain(currentBrightness - 30, 10, 255);
                    ttgo->setBrightness(currentBrightness);
                    Serial.println("Brightness decreased");
                    eventHandled = true;
                }
                // --- Tap in center plane circle to set home point ---
                if (isFlightInfoScreen) {
                    int planeCenterX = 120;
                    int planeCenterY = 120;
                    int ringRadius = 20; // 40px diameter
                    float dx = touchStartX - planeCenterX;
                    float dy = touchStartY - planeCenterY;
                    if ((dx*dx + dy*dy) <= (ringRadius*ringRadius)) {
                        Serial.println("Turquoise circle tapped: Setting home point");
                        if (gps && gps->location.isValid()) {
                            saveHomeLocationToEEPROM(gps->location.lat(), gps->location.lng());
                            homeSavedTimestamp = millis();
                            showHomeSavedMessage = true;
                            homeSavedMessageTimestamp = millis();
                            Serial.println("Home point saved!");
                        }
                        // 2 second vibration feedback
                        if (ttgo && ttgo->drv) {
                            for (int i = 0; i < 20; ++i) {
                                ttgo->drv->go();
                                delay(100);
                            }
                        }
                        eventHandled = true;
                    }
                }
            }

            // --- Swipe detection ---
            if (!eventHandled) { 
                // Vertical swipe
                if (abs(swipeY) > abs(swipeX) && abs(swipeY) > 20) {
                    if (isClockScreen && swipeY > 0) { // Swipe down from clock -> Nav
                        isClockScreen = false; isExtraScreen = true; isFlightInfoScreen = false;
                        navScreenInitialized = false; flightScreenInitialized = false; forceClockRedraw = false;
                        g_prevCarIconDrawX = -1; g_prevCarIconDrawY = -1; for (int k=0; k<4; ++k) prev_compass_rects[k].x = -1;
                    } else if (isClockScreen && swipeY < 0) { // Swipe UP from clock -> Flight
                        isClockScreen = false; isExtraScreen = false; isFlightInfoScreen = true;
                        flightScreenInitialized = false; navScreenInitialized = false; forceClockRedraw = false;
                    } else if (isExtraScreen && swipeY < 0) { // Swipe UP from Nav -> Flight
                        isClockScreen = false; isExtraScreen = false; isFlightInfoScreen = true;
                        flightScreenInitialized = false; navScreenInitialized = false; forceClockRedraw = false;
                    } else if (isFlightInfoScreen && swipeY < 0) { // Swipe UP from Flight -> Nav
                        isClockScreen = false; isExtraScreen = true; isFlightInfoScreen = false;
                        navScreenInitialized = false; flightScreenInitialized = false; forceClockRedraw = false;
                        g_prevCarIconDrawX = -1; g_prevCarIconDrawY = -1; for (int k=0; k<4; ++k) prev_compass_rects[k].x = -1;
                    }
                    eventHandled = true;
                }
                // Horizontal swipe
                else if (abs(swipeX) > 50) {
                    if (isClockScreen) {
                        if (swipeX < 0) { // Swipe left from clock -> Old GPS
                            isClockScreen = false; isExtraScreen = false; isFlightInfoScreen = false;
                            navScreenInitialized = false; flightScreenInitialized = false; forceClockRedraw = false;
                            switchScreen(false); // Clears & sets up Old GPS screen
                        } else if (swipeX > 0) { // Swipe right from clock -> start game
                            navScreenInitialized = false; flightScreenInitialized = false; forceClockRedraw = false;
                            startBallGame(); // This function will set clock state for return
                        }
                    } else if (isExtraScreen) { // Swipes on Nav screen
                        if (swipeX > 0) { // Swipe right from Nav -> Clock
                            isClockScreen = true; isExtraScreen = false; isFlightInfoScreen = false;
                            forceClockRedraw = true; navScreenInitialized = false; flightScreenInitialized = false;
                        }
                    } else if (!isClockScreen && !isExtraScreen && !isFlightInfoScreen) { // On "Old GPS" screen
                        if (swipeX > 0) { // Swipe right from Old GPS info to go to Clock
                            isClockScreen = true; isExtraScreen = false; isFlightInfoScreen = false;
                            forceClockRedraw = true; navScreenInitialized = false; flightScreenInitialized = false;
                        }
                    }
                    eventHandled = true;
                }
            }
            // Reset touch tracking variables after swipe/tap processing
            touchStartX = 0; touchStartY = 0;
        } // End if (touchStartX != 0 || touchStartY != 0) - i.e. if a touch/swipe was processed
    } // End touch ended

    // Screen drawing logic based on current state
    if (isFlightInfoScreen) {
        if (!flightScreenInitialized) {
            initFlightInfoScreen(); // Clears screen to BLACK
            flightScreenInitialized = true;
            forceAllFlightDynamicElementsRedraw = true;
        }
        updateFlightInfoScreenDynamicElements(forceAllFlightDynamicElementsRedraw);
        if (forceAllFlightDynamicElementsRedraw) { forceAllFlightDynamicElementsRedraw = false; }
        return; // IMPORTANT: Return after handling this screen
    } else if (isExtraScreen) { // Nav Screen
        if (!navScreenInitialized) {
            initNavScreenBackground();
            navScreenInitialized = true;
            forceAllNavDynamicElementsRedraw = true; // Force redraw of all dynamic parts on first entry
        }
        updateNavLatLon(forceAllNavDynamicElementsRedraw);
        updateNavBattery(forceAllNavDynamicElementsRedraw);
        updateNavAltitude(forceAllNavDynamicElementsRedraw);
        updateNavSpeed(forceAllNavDynamicElementsRedraw);
        updateNavCompass(forceAllNavDynamicElementsRedraw);
        updateNavCarIcon(forceAllNavDynamicElementsRedraw);

        if (forceAllNavDynamicElementsRedraw) { // After the first full draw of dynamic elements
            forceAllNavDynamicElementsRedraw = false;
        }
        return; // IMPORTANT: Return after handling this screen
    } else if (isClockScreen) {
        displayClock(); // Clears screen to BLACK if forceClockRedraw is true
        return; // IMPORTANT: Return after handling this screen
    } else { 
        // Fallback to default GPS info (Old GPS screen)
        // This screen is entered via switchScreen(false), which handles initial clear and sprite draw.
        // Subsequent updates for sprites are handled by the GPS data update logic below.
        tft->setTextFont(2); // Ensure font is set for this screen if it was changed by other screens
        tft->setTextSize(1);
        updatePowerInfo(); // This pushes eSpPower
        // updateRTCDisplay(); // If this is part of the "Old GPS" screen, uncomment
    }

    if (gps->time.isUpdated() && gps->date.isUpdated()) {
        syncTimeFromGPS();
    }

    // Add GPS time sync when time is updated
    if (gps->time.isUpdated() && gps->date.isUpdated()) {
        syncTimeFromGPS();
    }

    // Update GPS display elements
    if (gps->location.isUpdated()) {
        updateTimeout = millis();

        eSpLoaction->fillSprite(TFT_BLACK);
        eSpLoaction->setTextColor(TFT_GREEN, TFT_BLACK);
        eSpLoaction->setCursor(0, 0);
        eSpLoaction->print("LOCATION ");
        eSpLoaction->print("Fix Age=");
        eSpLoaction->println(gps->location.age());
        eSpLoaction->print("Lat= ");
        eSpLoaction->print(gps->location.lat(), 6);
        eSpLoaction->print(" Long= ");
        eSpLoaction->print(gps->location.lng(), 6);
        eSpLoaction->pushSprite(0, 0);

        Serial.print(F("LOCATION   Fix Age="));
        Serial.print(gps->location.age());
        Serial.print(F("ms Raw Lat="));
        Serial.print(gps->location.rawLat().negative ? "-" : "+");
        Serial.print(gps->location.rawLat().deg);
        Serial.print("[+");
        Serial.print(gps->location.rawLat().billionths);
        Serial.print(F(" billionths],  Raw Long="));
        Serial.print(gps->location.rawLng().negative ? "-" : "+");
        Serial.print(gps->location.rawLng().deg);
        Serial.print("[+");
        Serial.print(gps->location.rawLng().billionths);
        Serial.print(F(" billionths],  Lat="));
        Serial.print(gps->location.lat(), 6);
        Serial.print(F(" Long="));
        Serial.println(gps->location.lng(), 6);
    } else {
        if (millis() - updateTimeout > 3000) {
            updateTimeout = millis();

            eSpLoaction->fillSprite(TFT_BLACK);
            eSpLoaction->setTextColor(TFT_GREEN);
            eSpLoaction->setCursor(0, 0);
            eSpLoaction->print("LOCATION ");
            eSpLoaction->setTextColor(TFT_RED);
            eSpLoaction->print("INVAILD");
            eSpLoaction->pushSprite(0, 0);
        }
    }

    if (gps->date.isUpdated()) {
        eSpDate->setTextColor(TFT_GREEN, TFT_BLACK);
        eSpDate->fillSprite(TFT_BLACK);
        eSpDate->setCursor(0, 0);
        eSpDate->print("DATE Fix Age= ");
        eSpDate->println(gps->date.age());
        eSpDate->print("Year=");
        eSpDate->print(gps->date.year());
        eSpDate->print(" Month=");
        eSpDate->print(gps->date.month());
        eSpDate->print(" Day=");
        eSpDate->println(gps->date.day());
        eSpDate->pushSprite(0, 49);

        Serial.print(F("DATE Fix Age="));
        Serial.print(gps->date.age());
        Serial.print(F("ms Raw="));
        Serial.print(gps->date.value());
        Serial.print(F(" Year="));
        Serial.print(gps->date.year());
        Serial.print(F(" Month="));
        Serial.print(gps->date.month());
        Serial.print(F(" Day="));
        Serial.println(gps->date.day());
    }

    if (gps->time.isUpdated()) {
        eSpTime->setTextColor(TFT_GREEN, TFT_BLACK);
        eSpTime->fillSprite(TFT_BLACK);
        eSpTime->setCursor(0, 0);
        eSpTime->print("TIME Fix Age= ");
        eSpTime->println(gps->time.age());
        eSpTime->print("Hour=");
        eSpTime->print(gps->time.hour());
        eSpTime->print(" Minute=");
        eSpTime->print(gps->time.minute());
        eSpTime->print(" Second=");
        eSpTime->println(gps->time.second());
        eSpTime->pushSprite(0, 49 * 2);

        Serial.print(F("TIME Fix Age="));
        Serial.print(gps->time.age());
        Serial.print(F("ms Raw="));
        Serial.print(gps->time.value());
        Serial.print(F(" Hour="));
        Serial.print(gps->time.hour());
        Serial.print(F(" Minute="));
        Serial.print(gps->time.minute());
        Serial.print(F(" Second="));
        Serial.print(gps->time.second());
        Serial.print(F(" Hundredths="));
        Serial.println(gps->time.centisecond());
    }

    if (gps->satellites.isUpdated()) {
        eSpSatellites->setTextColor(TFT_GREEN, TFT_BLACK);
        eSpSatellites->fillSprite(TFT_BLACK);
        eSpSatellites->setCursor(0, 0);
        eSpSatellites->print("SATELLITES Fix Age= ");
        eSpSatellites->print(gps->satellites.age());
        eSpSatellites->print("ms Value=");
        eSpSatellites->println(gps->satellites.value());

        Serial.print(F("SATELLITES Fix Age="));
        Serial.print(gps->satellites.age());
        Serial.print(F("ms Value="));
        Serial.println(gps->satellites.value());

        eSpSatellites->pushSprite(0, 49 * 3);
    }
}
