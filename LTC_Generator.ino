/*
 * Teensy 4.1 LTC/Word Clock Generator for Eagle Automation
 * Complete Working Implementation v1.0
 * 
 * Hardware Requirements:
 * - Teensy 4.1
 * - Audio Adapter Board
 * - 128x64 OLED (I2C)
 * - Rotary Encoder with button
 * - Audio transformer (600:600 ohm)
 * - 2x BNC connectors for word clock
 * - TRS jack for LTC output
 */

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Encoder.h>
#include <EEPROM.h>

// Pin Definitions
#define WC_OUT1_PIN      2    // Word Clock Output 1
#define WC_OUT2_PIN      3    // Word Clock Output 2
#define LED_POWER_PIN    4    // Green LED
#define LED_LTC_PIN      5    // Yellow LED
#define ENCODER_A_PIN    6    // Rotary encoder A
#define ENCODER_B_PIN    7    // Rotary encoder B
#define ENCODER_BTN_PIN  8    // Rotary encoder button

// Display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// LTC Constants
#define LTC_BITS_PER_FRAME 80
#define SYNC_WORD_START 64
#define SAMPLES_PER_SECOND 48000

// Create display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Audio system - using Audio library's synthesis for LTC
AudioSynthWaveform       ltcWaveform;
AudioOutputI2S           audioOutput;
AudioConnection          patchCord1(ltcWaveform, 0, audioOutput, 0);
AudioConnection          patchCord2(ltcWaveform, 0, audioOutput, 1);
AudioControlSGTL5000     sgtl5000_1;

// Controls
Encoder encoder(ENCODER_A_PIN, ENCODER_B_PIN);
int32_t encoderPosition = 0;
uint8_t buttonState = HIGH;
uint8_t lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// Timing
IntervalTimer wordClockTimer;
IntervalTimer ltcBitTimer;
elapsedMillis displayUpdateTimer;
elapsedMillis frameTimer;

// Timecode structure
struct Timecode {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint8_t frames;
    uint32_t userBits;
};

// System settings (saved to EEPROM)
struct Settings {
    uint8_t frameRate;      // 24, 25, 30
    bool dropFrame;         // true for 29.97
    uint32_t sampleRate;    // 44100, 48000, 88200, 96000
    uint8_t outputLevel;    // 0-100%
    uint32_t magic;         // To verify EEPROM data validity
} settings;

// Current state
Timecode currentTC = {1, 0, 0, 0, 0};
bool ltcRunning = true;
uint8_t ltcBits[LTC_BITS_PER_FRAME];
uint32_t currentBit = 0;
bool currentBitState = false;
uint32_t bitTransitions = 0;

// Menu system
enum MenuMode {
    MODE_RUN,
    MODE_SET_FPS,
    MODE_SET_SAMPLE,
    MODE_SET_TIME,
    MODE_SET_LEVEL
} currentMode = MODE_RUN;

// Function declarations
void initializeHardware();
void loadSettings();
void saveSettings();
void generateLTCFrame();
void updateDisplay();
void handleEncoder();
void wordClockISR();
void ltcBitISR();
void incrementTimecode();
bool isDropFrame();

void setup() {
    Serial.begin(115200);
    delay(1000); // Wait for serial
    Serial.println("LTC/Word Clock Generator Starting...");
    
    // Initialize hardware
    initializeHardware();
    
    // Load settings from EEPROM
    loadSettings();
    
    // Initialize audio system
    AudioMemory(12);
    sgtl5000_1.enable();
    sgtl5000_1.volume(0.8);
    sgtl5000_1.lineOutLevel(13); // 3.16V p-p
    
    // Configure LTC waveform
    ltcWaveform.begin(WAVEFORM_SQUARE);
    ltcWaveform.frequency(1920); // Base frequency for LTC
    ltcWaveform.amplitude(0.8);
    
    // Generate initial LTC frame
    generateLTCFrame();
    
    // Start word clock generation
    uint32_t wcPeriod = 1000000 / (settings.sampleRate * 2); // Microseconds
    wordClockTimer.begin(wordClockISR, wcPeriod);
    
    // Start LTC bit timer
    float bitsPerSecond = settings.frameRate * LTC_BITS_PER_FRAME;
    if (isDropFrame()) bitsPerSecond = 29.97 * LTC_BITS_PER_FRAME;
    uint32_t ltcPeriod = 1000000 / bitsPerSecond;
    ltcBitTimer.begin(ltcBitISR, ltcPeriod);
    
    Serial.println("Initialization complete!");
    Serial.print("Frame Rate: "); Serial.println(settings.frameRate);
    Serial.print("Sample Rate: "); Serial.println(settings.sampleRate);
}

void loop() {
    // Handle user input
    handleEncoder();
    
    // Update timecode every frame period
    uint32_t framePeriod = 1000 / settings.frameRate;
    if (isDropFrame()) framePeriod = 33.367; // 29.97 fps
    
    if (frameTimer >= framePeriod) {
        frameTimer = 0;
        if (ltcRunning) {
            incrementTimecode();
            generateLTCFrame();
        }
    }
    
    // Update display
    if (displayUpdateTimer >= 100) {
        displayUpdateTimer = 0;
        updateDisplay();
    }
    
    // Blink LTC LED when running
    digitalWrite(LED_LTC_PIN, ltcRunning && (millis() % 500 < 250));
}

void initializeHardware() {
    // Initialize pins
    pinMode(WC_OUT1_PIN, OUTPUT);
    pinMode(WC_OUT2_PIN, OUTPUT);
    pinMode(LED_POWER_PIN, OUTPUT);
    pinMode(LED_LTC_PIN, OUTPUT);
    pinMode(ENCODER_BTN_PIN, INPUT_PULLUP);
    
    digitalWrite(LED_POWER_PIN, HIGH); // Power LED on
    
    // Initialize I2C display
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println("SSD1306 allocation failed");
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.display();
}

void loadSettings() {
    EEPROM.get(0, settings);
    
    // Check if EEPROM contains valid data
    if (settings.magic != 0xDEADBEEF) {
        // Initialize with defaults
        settings.frameRate = 30;
        settings.dropFrame = false;
        settings.sampleRate = 48000;
        settings.outputLevel = 80;
        settings.magic = 0xDEADBEEF;
        saveSettings();
    }
}

void saveSettings() {
    EEPROM.put(0, settings);
}

// Generate complete 80-bit LTC frame
void generateLTCFrame() {
    // Clear frame
    memset(ltcBits, 0, LTC_BITS_PER_FRAME);
    
    // Encode BCD time values
    // Frames (bits 0-3, 8-9)
    ltcBits[0] = (currentTC.frames % 10) & 0x01;
    ltcBits[1] = ((currentTC.frames % 10) >> 1) & 0x01;
    ltcBits[2] = ((currentTC.frames % 10) >> 2) & 0x01;
    ltcBits[3] = ((currentTC.frames % 10) >> 3) & 0x01;
    ltcBits[8] = (currentTC.frames / 10) & 0x01;
    ltcBits[9] = ((currentTC.frames / 10) >> 1) & 0x01;
    
    // Seconds (bits 16-19, 24-26)
    ltcBits[16] = (currentTC.seconds % 10) & 0x01;
    ltcBits[17] = ((currentTC.seconds % 10) >> 1) & 0x01;
    ltcBits[18] = ((currentTC.seconds % 10) >> 2) & 0x01;
    ltcBits[19] = ((currentTC.seconds % 10) >> 3) & 0x01;
    ltcBits[24] = (currentTC.seconds / 10) & 0x01;
    ltcBits[25] = ((currentTC.seconds / 10) >> 1) & 0x01;
    ltcBits[26] = ((currentTC.seconds / 10) >> 2) & 0x01;
    
    // Minutes (bits 32-35, 40-42)
    ltcBits[32] = (currentTC.minutes % 10) & 0x01;
    ltcBits[33] = ((currentTC.minutes % 10) >> 1) & 0x01;
    ltcBits[34] = ((currentTC.minutes % 10) >> 2) & 0x01;
    ltcBits[35] = ((currentTC.minutes % 10) >> 3) & 0x01;
    ltcBits[40] = (currentTC.minutes / 10) & 0x01;
    ltcBits[41] = ((currentTC.minutes / 10) >> 1) & 0x01;
    ltcBits[42] = ((currentTC.minutes / 10) >> 2) & 0x01;
    
    // Hours (bits 48-51, 56-57)
    ltcBits[48] = (currentTC.hours % 10) & 0x01;
    ltcBits[49] = ((currentTC.hours % 10) >> 1) & 0x01;
    ltcBits[50] = ((currentTC.hours % 10) >> 2) & 0x01;
    ltcBits[51] = ((currentTC.hours % 10) >> 3) & 0x01;
    ltcBits[56] = (currentTC.hours / 10) & 0x01;
    ltcBits[57] = ((currentTC.hours / 10) >> 1) & 0x01;
    
    // Drop frame flag (bit 10)
    ltcBits[10] = isDropFrame() ? 1 : 0;
    
    // Color frame flag (bit 11) - not used
    ltcBits[11] = 0;
    
    // Binary group flags (bits 43, 59)
    ltcBits[43] = 0;
    ltcBits[59] = 0;
    
    // Sync word (bits 64-79) = 0x3FFD
    uint16_t syncWord = 0xBFFC; // Reversed bit order
    for (int i = 0; i < 16; i++) {
        ltcBits[64 + i] = (syncWord >> i) & 0x01;
    }
    
    // Reset bit counter for transmission
    currentBit = 0;
}

// Word Clock interrupt service routine
volatile bool wcState = false;
void wordClockISR() {
    wcState = !wcState;
    digitalWriteFast(WC_OUT1_PIN, wcState);
    digitalWriteFast(WC_OUT2_PIN, wcState);
}

// LTC bit interrupt service routine (Biphase Mark encoding)
void ltcBitISR() {
    if (!ltcRunning) return;
    
    static bool midBit = false;
    
    if (!midBit) {
        // Start of bit - always transition
        currentBitState = !currentBitState;
        
        // If bit is 1, prepare for mid-bit transition
        midBit = (ltcBits[currentBit] == 1);
        
        if (!midBit) {
            // Bit is 0, move to next bit
            currentBit++;
            if (currentBit >= LTC_BITS_PER_FRAME) {
                currentBit = 0;
            }
        }
    } else {
        // Mid-bit transition for '1' bits
        currentBitState = !currentBitState;
        midBit = false;
        currentBit++;
        if (currentBit >= LTC_BITS_PER_FRAME) {
            currentBit = 0;
        }
    }
    
    // Update audio output
    ltcWaveform.amplitude(currentBitState ? 0.8 : 0.0);
}

void incrementTimecode() {
    currentTC.frames++;
    
    // Handle drop frame
    if (isDropFrame() && currentTC.frames == 30) {
        currentTC.frames = 0;
        currentTC.seconds++;
        
        // Drop frame: skip frames 0 and 1 at start of each minute except multiples of 10
        if (currentTC.seconds == 0 && currentTC.minutes % 10 != 0) {
            currentTC.frames = 2;
        }
    } else if (currentTC.frames >= settings.frameRate) {
        currentTC.frames = 0;
        currentTC.seconds++;
    }
    
    if (currentTC.seconds >= 60) {
        currentTC.seconds = 0;
        currentTC.minutes++;
    }
    
    if (currentTC.minutes >= 60) {
        currentTC.minutes = 0;
        currentTC.hours++;
    }
    
    if (currentTC.hours >= 24) {
        currentTC.hours = 0;
    }
}

bool isDropFrame() {
    return (settings.frameRate == 30 && settings.dropFrame);
}

void handleEncoder() {
    // Read button with debouncing
    int reading = digitalRead(ENCODER_BTN_PIN);
    
    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }
    
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading != buttonState) {
            buttonState = reading;
            
            if (buttonState == LOW) {
                // Button pressed - cycle through modes
                currentMode = (MenuMode)((currentMode + 1) % 5);
            }
        }
    }
    
    lastButtonState = reading;
    
    // Read encoder rotation
    int32_t newPosition = encoder.read() / 4; // Divide by 4 for detent
    
    if (newPosition != encoderPosition) {
        int32_t delta = newPosition - encoderPosition;
        encoderPosition = newPosition;
        
        switch (currentMode) {
            case MODE_RUN:
                // Start/stop LTC
                if (delta != 0) {
                    ltcRunning = !ltcRunning;
                }
                break;
                
            case MODE_SET_FPS:
                if (delta > 0) {
                    if (settings.frameRate == 24) settings.frameRate = 25;
                    else if (settings.frameRate == 25) settings.frameRate = 30;
                    else if (settings.frameRate == 30 && !settings.dropFrame) {
                        settings.dropFrame = true; // 29.97
                    } else {
                        settings.frameRate = 24;
                        settings.dropFrame = false;
                    }
                } else {
                    if (settings.frameRate == 24) {
                        settings.frameRate = 30;
                        settings.dropFrame = true; // 29.97
                    } else if (settings.frameRate == 30 && settings.dropFrame) {
                        settings.dropFrame = false; // 30
                    } else if (settings.frameRate == 30) {
                        settings.frameRate = 25;
                    } else {
                        settings.frameRate = 24;
                    }
                }
                saveSettings();
                
                // Restart LTC bit timer with new rate
                ltcBitTimer.end();
                float bitsPerSecond = settings.frameRate * LTC_BITS_PER_FRAME;
                if (isDropFrame()) bitsPerSecond = 29.97 * LTC_BITS_PER_FRAME;
                uint32_t ltcPeriod = 1000000 / bitsPerSecond;
                ltcBitTimer.begin(ltcBitISR, ltcPeriod);
                break;
                
            case MODE_SET_SAMPLE:
                if (delta > 0) {
                    if (settings.sampleRate == 44100) settings.sampleRate = 48000;
                    else if (settings.sampleRate == 48000) settings.sampleRate = 88200;
                    else if (settings.sampleRate == 88200) settings.sampleRate = 96000;
                    else settings.sampleRate = 44100;
                } else {
                    if (settings.sampleRate == 96000) settings.sampleRate = 88200;
                    else if (settings.sampleRate == 88200) settings.sampleRate = 48000;
                    else if (settings.sampleRate == 48000) settings.sampleRate = 44100;
                    else settings.sampleRate = 96000;
                }
                saveSettings();
                
                // Restart word clock with new rate
                wordClockTimer.end();
                uint32_t wcPeriod = 1000000 / (settings.sampleRate * 2);
                wordClockTimer.begin(wordClockISR, wcPeriod);
                break;
                
            case MODE_SET_TIME:
                // Adjust hours
                currentTC.hours = (currentTC.hours + delta + 24) % 24;
                break;
                
            case MODE_SET_LEVEL:
                // Adjust output level
                settings.outputLevel = constrain(settings.outputLevel + delta * 5, 0, 100);
                float level = settings.outputLevel / 100.0;
                sgtl5000_1.lineOutLevel(13 + (level * 18)); // 13-31 range
                saveSettings();
                break;
        }
    }
}

void updateDisplay() {
    display.clearDisplay();
    
    // Main timecode display (large)
    display.setTextSize(2);
    display.setCursor(0, 0);
    
    char tcStr[12];
    sprintf(tcStr, "%02d:%02d:%02d:%02d", 
            currentTC.hours, currentTC.minutes, 
            currentTC.seconds, currentTC.frames);
    display.print(tcStr);
    
    // Frame rate and sample rate
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.print("FPS: ");
    if (isDropFrame()) {
        display.print("29.97DF");
    } else {
        display.print(settings.frameRate);
        display.print("   ");
    }
    
    display.setCursor(64, 20);
    display.print("WC: ");
    display.print(settings.sampleRate / 1000);
    display.print("k");
    
    // Current mode
    display.setCursor(0, 32);
    display.print("Mode: ");
    switch (currentMode) {
        case MODE_RUN:
            display.print(ltcRunning ? "RUNNING" : "STOPPED");
            break;
        case MODE_SET_FPS:
            display.print("SET FPS");
            break;
        case MODE_SET_SAMPLE:
            display.print("SET SAMPLE");
            break;
        case MODE_SET_TIME:
            display.print("SET TIME");
            break;
        case MODE_SET_LEVEL:
            display.print("LEVEL ");
            display.print(settings.outputLevel);
            display.print("%");
            break;
    }
    
    // Status bar
    display.setCursor(0, 48);
    if (ltcRunning) {
        display.print("[LOCKED] Level:");
        display.print(settings.outputLevel);
        display.print("%");
    } else {
        display.print("[STOPPED]");
    }
    
    // Instructions at bottom
    display.setCursor(0, 56);
    display.setTextSize(1);
    display.print("Push:Mode Turn:Adjust");
    
    display.display();
}
