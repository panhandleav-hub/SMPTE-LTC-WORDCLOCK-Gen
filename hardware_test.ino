/*
 * Hardware Test for LTC/Word Clock Generator
 * Run this BEFORE the main code to verify all connections
 * 
 * Tests each component individually
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Encoder.h>

// Pin definitions (must match main code)
#define WC_OUT1_PIN      2
#define WC_OUT2_PIN      3
#define LED_POWER_PIN    4
#define LED_LTC_PIN      5
#define ENCODER_A_PIN    6
#define ENCODER_B_PIN    7
#define ENCODER_BTN_PIN  8

// Display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// Create objects
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Encoder myEnc(ENCODER_A_PIN, ENCODER_B_PIN);

// Test state
int testNumber = 0;
long oldPosition = -999;
bool testPassed[10];
String testNames[10] = {
  "Power LED",
  "LTC LED",
  "Word Clock 1",
  "Word Clock 2",
  "OLED Display",
  "Encoder Rotation",
  "Encoder Button",
  "Audio Board",
  "I2C Bus",
  "Complete"
};

void setup() {
  Serial.begin(115200);
  delay(2000); // Wait for serial monitor
  
  Serial.println("\n=================================");
  Serial.println("LTC/Word Clock Generator");
  Serial.println("Hardware Test Program v1.0");
  Serial.println("=================================\n");
  
  // Initialize pins
  pinMode(WC_OUT1_PIN, OUTPUT);
  pinMode(WC_OUT2_PIN, OUTPUT);
  pinMode(LED_POWER_PIN, OUTPUT);
  pinMode(LED_LTC_PIN, OUTPUT);
  pinMode(ENCODER_BTN_PIN, INPUT_PULLUP);
  
  // Initialize display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("❌ SSD1306 OLED NOT FOUND!");
    Serial.println("Check I2C connections:");
    Serial.println("  - SDA to pin 18");
    Serial.println("  - SCL to pin 19");
    Serial.println("  - VCC to 3.3V");
    Serial.println("  - GND to ground");
    testPassed[4] = false;
  } else {
    Serial.println("✓ OLED Display found");
    testPassed[4] = true;
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println("Hardware Test Mode");
    display.println("Check Serial Monitor");
    display.display();
  }
  
  // Test sequence
  Serial.println("\nStarting hardware tests...\n");
  delay(1000);
}

void loop() {
  switch(testNumber) {
    case 0:
      testPowerLED();
      break;
    case 1:
      testLTCLED();
      break;
    case 2:
      testWordClock1();
      break;
    case 3:
      testWordClock2();
      break;
    case 4:
      testOLED();
      break;
    case 5:
      testEncoder();
      break;
    case 6:
      testEncoderButton();
      break;
    case 7:
      testAudioBoard();
      break;
    case 8:
      testI2CBus();
      break;
    case 9:
      showResults();
      break;
  }
  
  delay(100);
}

void testPowerLED() {
  Serial.println("TEST 1: Power LED (Green)");
  Serial.println("The green LED should be blinking...");
  
  for(int i = 0; i < 10; i++) {
    digitalWrite(LED_POWER_PIN, HIGH);
    delay(200);
    digitalWrite(LED_POWER_PIN, LOW);
    delay(200);
  }
  
  digitalWrite(LED_POWER_PIN, HIGH); // Leave on
  
  Serial.println("Did the green LED blink? (y/n)");
  waitForResponse();
}

void testLTCLED() {
  Serial.println("\nTEST 2: LTC LED (Yellow)");
  Serial.println("The yellow LED should be blinking...");
  
  for(int i = 0; i < 10; i++) {
    digitalWrite(LED_LTC_PIN, HIGH);
    delay(200);
    digitalWrite(LED_LTC_PIN, LOW);
    delay(200);
  }
  
  Serial.println("Did the yellow LED blink? (y/n)");
  waitForResponse();
}

void testWordClock1() {
  Serial.println("\nTEST 3: Word Clock Output 1");
  Serial.println("Generating 1 Hz square wave on BNC 1");
  Serial.println("Use oscilloscope or frequency counter to verify");
  Serial.println("You should see a 1 Hz square wave");
  
  for(int i = 0; i < 20; i++) {
    digitalWrite(WC_OUT1_PIN, HIGH);
    delay(500);
    digitalWrite(WC_OUT1_PIN, LOW);
    delay(500);
  }
  
  Serial.println("Did you see 1 Hz on BNC 1? (y/n)");
  waitForResponse();
}

void testWordClock2() {
  Serial.println("\nTEST 4: Word Clock Output 2");
  Serial.println("Generating 1 Hz square wave on BNC 2");
  
  for(int i = 0; i < 20; i++) {
    digitalWrite(WC_OUT2_PIN, HIGH);
    delay(500);
    digitalWrite(WC_OUT2_PIN, LOW);
    delay(500);
  }
  
  Serial.println("Did you see 1 Hz on BNC 2? (y/n)");
  waitForResponse();
}

void testOLED() {
  Serial.println("\nTEST 5: OLED Display");
  
  if(!testPassed[4]) {
    Serial.println("OLED not detected - skipping");
    testNumber++;
    return;
  }
  
  Serial.println("Display should show test pattern...");
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,0);
  display.println("01:23:45:67");
  display.setTextSize(1);
  display.setCursor(0,20);
  display.println("Display Test OK!");
  display.setCursor(0,30);
  display.println("Check all pixels");
  display.setCursor(0,40);
  display.println("No dead areas?");
  
  // Draw border
  display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
  display.display();
  
  Serial.println("Is the display working correctly? (y/n)");
  waitForResponse();
}

void testEncoder() {
  Serial.println("\nTEST 6: Rotary Encoder");
  Serial.println("Turn the encoder knob clockwise and counter-clockwise");
  Serial.println("You should see the position value change");
  Serial.println("Turn it at least 5 clicks in each direction");
  Serial.println("Press 'c' when complete\n");
  
  long maxPos = 0;
  long minPos = 0;
  
  while(true) {
    long newPosition = myEnc.read() / 4;
    if (newPosition != oldPosition) {
      oldPosition = newPosition;
      Serial.print("Encoder position: ");
      Serial.println(newPosition);
      
      if(newPosition > maxPos) maxPos = newPosition;
      if(newPosition < minPos) minPos = newPosition;
      
      // Update display if working
      if(testPassed[4]) {
        display.fillRect(0, 50, 128, 14, SSD1306_BLACK);
        display.setCursor(0,50);
        display.print("Encoder: ");
        display.print(newPosition);
        display.display();
      }
    }
    
    if(Serial.available()) {
      char c = Serial.read();
      if(c == 'c' || c == 'C') {
        if((maxPos - minPos) >= 5) {
          testPassed[5] = true;
          Serial.println("✓ Encoder test passed");
        } else {
          testPassed[5] = false;
          Serial.println("❌ Insufficient rotation detected");
        }
        testNumber++;
        break;
      }
    }
    delay(10);
  }
}

void testEncoderButton() {
  Serial.println("\nTEST 7: Encoder Button");
  Serial.println("Press and release the encoder button 3 times");
  
  int pressCount = 0;
  bool lastState = HIGH;
  
  while(pressCount < 3) {
    bool buttonState = digitalRead(ENCODER_BTN_PIN);
    
    if(buttonState != lastState) {
      if(buttonState == LOW) {
        pressCount++;
        Serial.print("Button press detected: ");
        Serial.print(pressCount);
        Serial.println(" of 3");
        
        // Flash LED
        digitalWrite(LED_LTC_PIN, HIGH);
        delay(100);
        digitalWrite(LED_LTC_PIN, LOW);
      }
      lastState = buttonState;
      delay(50); // Debounce
    }
  }
  
  testPassed[6] = true;
  Serial.println("✓ Button test passed");
  testNumber++;
}

void testAudioBoard() {
  Serial.println("\nTEST 8: Audio Board");
  Serial.println("This test checks if the Audio Board is detected");
  
  // The audio library will initialize in the main sketch
  // Here we just check for I2C communication with SGTL5000
  Wire.beginTransmission(0x0A); // SGTL5000 I2C address
  byte error = Wire.endTransmission();
  
  if (error == 0) {
    Serial.println("✓ Audio codec (SGTL5000) detected at address 0x0A");
    testPassed[7] = true;
  } else {
    Serial.println("❌ Audio codec NOT detected!");
    Serial.println("Check Audio Board connections:");
    Serial.println("  - Board properly seated on Teensy");
    Serial.println("  - All pins making contact");
    testPassed[7] = false;
  }
  
  testNumber++;
}

void testI2CBus() {
  Serial.println("\nTEST 9: I2C Bus Scan");
  Serial.println("Scanning for I2C devices...\n");
  
  byte count = 0;
  
  for(byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      
      // Identify known devices
      if(address == 0x3C) Serial.print(" (OLED Display)");
      if(address == 0x0A) Serial.print(" (Audio Codec)");
      
      Serial.println();
      count++;
    }
  }
  
  Serial.print("\nTotal devices found: ");
  Serial.println(count);
  
  testPassed[8] = (count > 0);
  testNumber++;
  delay(2000);
}

void showResults() {
  Serial.println("\n=================================");
  Serial.println("TEST RESULTS SUMMARY");
  Serial.println("=================================\n");
  
  int passed = 0;
  int failed = 0;
  
  for(int i = 0; i < 9; i++) {
    Serial.print(testNames[i]);
    Serial.print(": ");
    if(i == 5 || i == 6) {
      // Special handling for tests that don't use waitForResponse
      if(testPassed[i]) {
        Serial.println("✓ PASSED");
        passed++;
      } else {
        Serial.println("❌ FAILED");
        failed++;
      }
    } else if(testPassed[i]) {
      Serial.println("✓ PASSED");
      passed++;
    } else {
      Serial.println("❌ FAILED");
      failed++;
    }
  }
  
  Serial.println("\n---------------------------------");
  Serial.print("Tests Passed: ");
  Serial.print(passed);
  Serial.print(" / ");
  Serial.println(passed + failed);
  
  if(failed == 0) {
    Serial.println("\n🎉 ALL TESTS PASSED!");
    Serial.println("Hardware is ready for main program");
    
    if(testPassed[4]) {
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(20,20);
      display.println("ALL PASS!");
      display.display();
    }
    
    // Victory blink
    for(int i = 0; i < 10; i++) {
      digitalWrite(LED_POWER_PIN, HIGH);
      digitalWrite(LED_LTC_PIN, HIGH);
      delay(100);
      digitalWrite(LED_POWER_PIN, LOW);
      digitalWrite(LED_LTC_PIN, LOW);
      delay(100);
    }
    digitalWrite(LED_POWER_PIN, HIGH);
    
  } else {
    Serial.println("\n⚠️ Some tests failed");
    Serial.println("Check connections and try again");
  }
  
  Serial.println("\nTest complete. Reset to run again.");
  testNumber++;
  
  while(1) {
    delay(1000);
  }
}

void waitForResponse() {
  while(true) {
    if(Serial.available()) {
      char response = Serial.read();
      if(response == 'y' || response == 'Y') {
        testPassed[testNumber] = true;
        Serial.println("✓ Test passed");
        testNumber++;
        break;
      } else if(response == 'n' || response == 'N') {
        testPassed[testNumber] = false;
        Serial.println("❌ Test failed");
        testNumber++;
        break;
      }
    }
    delay(10);
  }
}
