#include "config.h"

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

// Touch and interrupt related variables
int16_t touchX = 0, touchY = 0;
bool pmuIrq = false;
bool sleepIrq = false;  // For deep sleep interrupt
uint8_t touchStatus = TOUCH_ACTIVITY_MODE;

int16_t touchStartX = 0;
int16_t touchStartY = 0;
bool isClockScreen = true;  // Track which screen is currently shown

TFT_eSprite *eSpLoaction = nullptr;
TFT_eSprite *eSpDate = nullptr;
TFT_eSprite *eSpTime = nullptr;
TFT_eSprite *eSpSpeed = nullptr;
TFT_eSprite *eSpSatellites = nullptr;
TFT_eSprite *eSpPower = nullptr;  // New sprite for power info
TFT_eSprite *eSpRTC = nullptr;  // New sprite for RTC display
TFT_eSprite *gameSprite = nullptr;  // Sprite for game rendering
HardwareSerial *GNSS = NULL;

uint32_t last = 0;
uint32_t updateTimeout = 0;
uint32_t lastVibration = 0;  // Track last vibration time
uint32_t lastPowerCheck = 0;  // Track last power check time

int16_t prevTimeX = 0;
int16_t prevTimeY = 0;
int16_t prevTimeWidth = 0;
int16_t prevTimeHeight = 0;
bool clockInitialized = false;

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
    // Make sure we have valid pointers
    if (!ttgo || !ttgo->rtc || !ttgo->tft) {
        Serial.println("Error: Display components not initialized");
        return;
    }
    
    RTC_Date now = ttgo->rtc->getDateTime();
    
    // Only clear entire screen on first run
    if (!clockInitialized) {
        ttgo->tft->fillScreen(TFT_BLACK);
        clockInitialized = true;
    }
    
    ttgo->tft->setTextSize(1);
    ttgo->tft->setTextFont(6);
    
    // Format time as HH:MM:SS
    char timeStr[9];
    sprintf(timeStr, "%02d:%02d:%02d", now.hour, now.minute, now.second);
    
    // Calculate new position
    int16_t x = (ttgo->tft->width() - ttgo->tft->textWidth(timeStr, 6)) / 2;
    int16_t y = (ttgo->tft->height() - ttgo->tft->fontHeight(6)) / 2;
    int16_t width = ttgo->tft->textWidth(timeStr, 6);
    int16_t height = ttgo->tft->fontHeight(6);
    
    // Clear previous time area only if this isn't the first run
    if (prevTimeWidth > 0) {
        ttgo->tft->fillRect(prevTimeX, prevTimeY, prevTimeWidth, prevTimeHeight, TFT_BLACK);
    }
    
    // Draw new time
    ttgo->tft->setTextColor(TFT_CYAN);
    ttgo->tft->drawString(timeStr, x, y, 6);
    
    // Store current position and size for next update
    prevTimeX = x;
    prevTimeY = y;
    prevTimeWidth = width;
    prevTimeHeight = height;
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
    isClockScreen = toClockScreen;
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

    // Initialize game sprite if not already done
    if (!gameSprite) {
        gameSprite = new TFT_eSprite(ttgo->tft);
        gameSprite->createSprite(240, 240);
    }
}

void drawGame() {
    // Draw to sprite instead of directly to screen
    gameSprite->fillSprite(TFT_BLACK);
    
    // Draw hole
    gameSprite->fillCircle(gameState.hole.x, gameState.hole.y, gameState.hole.radius, TFT_RED);
    
    // Draw ball
    gameSprite->fillCircle(gameState.ball.x, gameState.ball.y, gameState.ball.radius, TFT_WHITE);
    
    // Draw level and time
    gameSprite->setTextSize(1);
    gameSprite->setTextFont(2);
    gameSprite->setTextColor(TFT_GREEN);
    
    char buf[32];
    sprintf(buf, "Level: %d", gameState.level);
    gameSprite->drawString(buf, 5, 5, 2);
    
    uint32_t elapsed = (millis() - gameState.startTime) / 1000;
    sprintf(buf, "Time: %ds", elapsed);
    gameSprite->drawString(buf, 5, 25, 2);
    
    // Push sprite to screen
    gameSprite->pushSprite(0, 0);
}

void updateBall() {
    Accel acc;
    if (sensor->getAccel(acc)) {
        // Reduce sensitivity even further and make movement smoother
        float sensitivity = 0.1;  // Reduced from 0.2
        float friction = 0.95;    // Increased from 0.92 for smoother movement
        
        // Apply smooth acceleration
        gameState.ball.vx = (gameState.ball.vx * friction) - (acc.y * sensitivity);
        gameState.ball.vy = (gameState.ball.vy * friction) + (acc.x * sensitivity);
        
        // Limit maximum velocity
        float maxSpeed = 4.0;
        gameState.ball.vx = constrain(gameState.ball.vx, -maxSpeed, maxSpeed);
        gameState.ball.vy = constrain(gameState.ball.vy, -maxSpeed, maxSpeed);
        
        // Update position
        gameState.ball.x += gameState.ball.vx;
        gameState.ball.y += gameState.ball.vy;
        
        // Screen boundaries with bounce effect
        if (gameState.ball.x < gameState.ball.radius) {
            gameState.ball.x = gameState.ball.radius;
            gameState.ball.vx = -gameState.ball.vx * 0.5;  // Bounce with energy loss
        }
        if (gameState.ball.x > 240 - gameState.ball.radius) {
            gameState.ball.x = 240 - gameState.ball.radius;
            gameState.ball.vx = -gameState.ball.vx * 0.5;
        }
        if (gameState.ball.y < gameState.ball.radius) {
            gameState.ball.y = gameState.ball.radius;
            gameState.ball.vy = -gameState.ball.vy * 0.5;
        }
        if (gameState.ball.y > 240 - gameState.ball.radius) {
            gameState.ball.y = 240 - gameState.ball.radius;
            gameState.ball.vy = -gameState.ball.vy * 0.5;
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
            
            delay(2000);
            
            // Start next level
            gameState.level++;
            gameState.ball = {120, 120, 0, 0, 5};
            int newHoleSize = max(8, gameState.initialHoleSize - ((gameState.level - 1) * 5));
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
        int16_t x, y;
        if (ttgo->getTouch(x, y)) {
            if (gameSprite) {
                delete gameSprite;
                gameSprite = nullptr;
            }
            gameState.isPlaying = false;
            break;
        }
        
        updateBall();
        drawGame();
        delay(16);  // ~60 FPS
    }
    // Return to clock screen
    switchScreen(true);
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
    // Handle sleep mode and touch panel
    handleSleep();
    handleTouchModeChange();

    // Handle touch gestures
    if (ttgo->getTouch(touchX, touchY)) {
        if (touchStartX == 0 && touchStartY == 0) {
            // Start of touch
            touchStartX = touchX;
            touchStartY = touchY;
        }
    } else if (touchStartX != 0 || touchStartY != 0) {
        // End of touch - check for swipe
        int16_t swipeX = touchX - touchStartX;
        
        // Reset touch start positions
        touchStartX = 0;
        touchStartY = 0;
        
        // Detect horizontal swipe (threshold of 50 pixels)
        if (abs(swipeX) > 50) {
            if (isClockScreen) {
                if (swipeX < 0) {
                    // Swipe left from clock - go to GPS
                    switchScreen(false);
                } else if (swipeX > 0) {
                    // Swipe right from clock - start game
                    startBallGame();
                }
            } else {
                if (swipeX > 0) {
                    // Swipe right from any other screen - return to clock
                    switchScreen(true);
                }
            }
        }
    }

    // Update current screen based on mode
    if (isClockScreen) {
        // Update clock when on clock screen
        static uint32_t lastClockUpdate = 0;
        if (millis() - lastClockUpdate >= 1000) {  // Update every second
            displayClock();
            lastClockUpdate = millis();
        }
    } else {
        // GPS screen updates
        updatePowerInfo();
        
        // GPS data handling
        while (GNSS->available()) {
            int r = GNSS->read();
            Serial.write(r);
            gps->encode(r);
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
}
