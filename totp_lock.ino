#include <SD.h>
#include <DS3231.h>
#include <Wire.h>
#include <TOTP.h>
#include <sha1.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
DS3231 rtc(SDA, SCL);

const int chipSelect = 10;
File myFile;
const char filename[] = "logs.txt";

long lastClearTime = 0;
int cursorpos = 0;

// Master password HMAC keys (Loaded from SD)
uint8_t hmacKeys[5][10];
TOTP *totp[5];  // Dynamic allocation for TOTP objects

// Master passwords generated from TOTP
char codes[5][7] = {0};
char Data[7] = {0};
const int lockOutput = 13;
byte data_count = 0;

// Keypad voltage thresholds and characters stored in PROGMEM
const int threshold[16] PROGMEM = {70, 50, 27, 6, 185, 171, 131, 116, 244, 231, 200, 186, 332, 322, 282, 263};
const char keypad[16] PROGMEM = {'1', '2', '3', 'A', '4', '5', '6', 'B', '7', '8', '9', 'C', '*', '0', '#', 'D'};

void defaults(){
  lcd.setCursor(0,0);
  lcd.print(F("Enter Password:"));
  lcd.setCursor(0,1);
}

void readHMACKeysFromSD() {
    lcd.setCursor(0, 0);
    myFile = SD.open("hmacKeys.bin");
    if (myFile) {
        myFile.read((uint8_t*)hmacKeys, sizeof(hmacKeys));
        myFile.close();
        Serial.println(F("HMAC keys loaded from SD."));
        lcd.print(F("HMAC keys loaded"));
        delay(900);
        lcd.clear();
    } else {
        Serial.println(F("Error reading HMAC keys."));
        lcd.print(F("Error reading keys"));
        delay(900);
        lcd.clear();
    }
}

void initializeTOTP() {
    for (int i = 0; i < 5; i++) {
        totp[i] = new TOTP(hmacKeys[i], 10, 86400);
    }
}

void freeTOTP() {
    for (int i = 0; i < 5; i++) {
        delete totp[i];
    }
}

bool isMasterPasswordCorrect() {
    for (int i = 0; i < 5; i++) {
        if (!strcmp(Data, codes[i])) {
            createLogs(i);
            return true;
        }
    }
    return false;
}

void createLogs(int i) {
    clearLogsIfNeeded();
    delay(100);

    myFile = SD.open(filename, FILE_WRITE);
    if (!myFile) {
        Serial.println(F("Error: Failed to open logs.txt for writing"));
        return;
    }

    myFile.print(F("USER"));
    myFile.print(i + 1);
    myFile.print(" ");
    myFile.print(rtc.getDateStr());
    myFile.print(" -- ");
    myFile.println(rtc.getTimeStr());
    
    myFile.flush();
    myFile.close();
    
    Serial.println(F("Log successfully written!"));
}

void clearLogsIfNeeded() {
    long currentTime = rtc.getUnixTime(rtc.getTime());  // Get current time

    if (currentTime - lastClearTime > 120) {  // Change this to 86400 for daily reset
        Serial.println(F("Clearing logs..."));

        // Check if the file exists before deleting it
        if (SD.exists(filename)) {
            if (SD.remove(filename)) {
                Serial.println(F("logs.txt deleted successfully."));
            } else {
                Serial.println(F("Error: Failed to delete logs.txt."));
                return;
            }
        }

        delay(100); // Give the SD card some time to process deletion

        // Attempt to create a new file
        myFile = SD.open(filename, FILE_WRITE);
        if (!myFile) {
            Serial.println(F("Error: Failed to recreate logs.txt"));
            return;
        }

        // Write a small header to ensure the file is created properly
        myFile.println(F("Log Start"));
        myFile.flush();
        myFile.close();

        Serial.println(F("logs.txt recreated successfully."));
        lastClearTime = currentTime;  // Update last clear time
    }
}

void clearData() {
    memset(Data, 0, 7);
    data_count = 0;
}

void updateTOTPCodes() {
    long GMT = rtc.getUnixTime(rtc.getTime());
    char newCode[7];
    
    for (int i = 0; i < 5; i++) {
        strncpy(newCode, totp[i]->getCode(GMT), 6);
        newCode[6] = '\0';
        if (strcmp(codes[i], newCode) != 0) {
            strcpy(codes[i], newCode);
        }
    }
}

char getKeypadInput() {
    int value = analogRead(A0);
    for (int i = 0; i < 16; i++) {
        if (abs(value - pgm_read_word(&threshold[i])) < 5) {
            while (analogRead(A0) < 1000) { delay(100); }
            return pgm_read_byte(&keypad[i]);
        }
    }
    return 0;
}

void handleKeyInput(char key) {
    if (key == 'A') {
        if (data_count > 0) {
            data_count--;
            cursorpos--;
            Data[data_count] = '\0';
            lcd.setCursor(cursorpos, 1); // Move cursor back
            lcd.print(" ");  // Overwrite with space
            lcd.setCursor(cursorpos, 1); // Move cursor back again
            Serial.println(F("Backspace"));
        }
    } else if (data_count < 6) {
        Data[data_count] = key;
        data_count++;
        cursorpos++;
    }
}

void setup() {
    Serial.begin(9600);
    rtc.begin();
    lcd.begin();
    lcd.backlight();
    pinMode(4,OUTPUT); //yellow
    pinMode(5,OUTPUT);
    
    if (!SD.begin(chipSelect)) {
        Serial.println(F("SD initialization failed!"));
        return;
    }
    Serial.println(F("SD initialized."));
    digitalWrite(4,HIGH);
    readHMACKeysFromSD();
    defaults();
    initializeTOTP();
}

void loop() {

    updateTOTPCodes();
    char key = getKeypadInput();
    if (key) {
      if(key!='A'){
        Serial.print(key);
        lcd.print(key);
      }
        
        handleKeyInput(key);
    }
    
    if (data_count == 6) {
      lcd.clear();
        lcd.setCursor(0, 0);
        cursorpos = 0;
        if (isMasterPasswordCorrect()) {
            lcd.print(F("Correct"));
            Serial.println(F("Correct"));
            digitalWrite(5,HIGH);
            digitalWrite(4,LOW);
        } else {
            lcd.print(F("Incorrect"));
            Serial.println(F("Incorrect"));
        }

        delay(5000);
        digitalWrite(5,LOW);
        digitalWrite(4,HIGH);
        lcd.clear();
        defaults();
        clearData();
    }
}