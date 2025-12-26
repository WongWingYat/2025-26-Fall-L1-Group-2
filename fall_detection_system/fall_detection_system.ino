#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SparkFun_MMA8452Q.h>
#include <SoftwareSerial.h>

SoftwareSerial bluetooth(4, 5);
LiquidCrystal_I2C lcd(0x27, 16, 2);
MMA8452Q accel;

#define PIR_PIN 2
#define BUTTON_PIN 3
#define BUZZER_PIN 8
#define RED_PIN 9
#define GREEN_PIN 10
#define BLUE_PIN 11

byte systemStatus = 1;
byte alertType = 0;
byte bluetoothConnected = 0;

unsigned long lastActivityTime = 0;
unsigned long alertStartTime = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastPIRCheck = 0;
unsigned long buttonPressTime = 0;
unsigned long lastAccelCheck = 0;

byte buttonPressed = 0;

const unsigned long INACTIVITY_WARNING = 60000;
const unsigned long NOTIFICATION_DELAY = 30000;
const unsigned long PIR_CHECK_INTERVAL = 200;
const unsigned long ACCEL_CHECK_INTERVAL = 50;
const unsigned long DISPLAY_UPDATE = 2000;

const float FALL_THRESHOLD = 2.5;

void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);
  pinMode(PIR_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  setRGB(0, 0, 255);
  lcd.setCursor(0, 0);
  lcd.print(F("Fall Detect"));
  lcd.setCursor(0, 1);
  lcd.print(F("Booting..."));
  if (accel.begin() == false) {
    Serial.println(F("Accel Fail!"));
    lcd.clear();
    lcd.print(F("Accel Error"));
  } else {
    accel.setScale(SCALE_2G);
    accel.setDataRate(ODR_100);
    Serial.println(F("Accel OK (1-Trg)"));
  }
  
  delay(2000);
  
  lastActivityTime = millis();
  setRGB(255, 255, 0);
  
  lcd.clear();
  lcd.print(F("Wait BT"));
  lcd.setCursor(0, 1);
  lcd.print(F("Pair HC-05"));
  
  Serial.println(F("Sys Ready (1-Trg)"));
  Serial.print(F("Threshold: ")); Serial.println(FALL_THRESHOLD);
  Serial.println(F("---"));
}

void loop() {
  checkBluetoothData();
  checkPIRActivity();
  checkAccelerometer();
  checkButton();
  checkInactivity();
  updateAlertStatus();
  updateDisplay();
  delay(50);
}

void checkAccelerometer() {
  if (millis() - lastAccelCheck < ACCEL_CHECK_INTERVAL) return;
  lastAccelCheck = millis();
  if (accel.available()) {
    float x = accel.getCalculatedX();
    float y = accel.getCalculatedY(); 
    float z = accel.getCalculatedZ();
    float totalAccel = sqrt(x*x + y*y + z*z);
    
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 1000) {
      lastDebug = millis();
      Serial.print(F("Accel: "));
      Serial.print(totalAccel, 2);
      Serial.print(F("g"));
      if (totalAccel > FALL_THRESHOLD) {
        Serial.print(F(" !HIGH!"));
      }
      Serial.println();
    }
    
    if (systemStatus == 1) {
      if (totalAccel > FALL_THRESHOLD) {
        Serial.println(F("FALL! Alarm ON"));
        triggerFallAlert();
        delay(1000);
      }
    }
  }
}

void checkBluetoothData() {
  if (bluetooth.available()) {
    String data = bluetooth.readString();
    data.trim();
    
    if (!bluetoothConnected) {
      bluetoothConnected = 1;
      onBluetoothConnected();
    }
    
    processBluetoothCommand(data);
  }
}

void onBluetoothConnected() {
  Serial.println(F("BT Connected!"));
  setRGB(0, 255, 0);
  
  lcd.clear();
  lcd.print(F("BT Connected"));
  lcd.setCursor(0, 1);
  lcd.print(F("System Ready"));
  
  bluetooth.println(F("Sys Ready (1-Trg)"));
  delay(1000);
  bluetooth.println(F("Cmds: STATUS,TEST,SOS,RESET,SIMFALL"));
}

void processBluetoothCommand(String cmd) {
  Serial.print(F("RCV Cmd: "));
  Serial.println(cmd);
  
  if (cmd == "STATUS") {
    sendSystemStatus();
  }
  else if (cmd == "TEST") {
    sendTestNotification();
  }
  else if (cmd == "HELP" || cmd == "SOS") {
    if (systemStatus != 2) triggerManualAlert();
  }
  else if (cmd == "RESET") {
    if (systemStatus == 2) resetAlert();
  }
  else if (cmd == "SIMFALL") {
    if (systemStatus != 2) triggerFallAlert();
  }
}

void checkPIRActivity() {
  if (millis() - lastPIRCheck < PIR_CHECK_INTERVAL) return;
  lastPIRCheck = millis();
  
  if (digitalRead(PIR_PIN) == HIGH) {
    lastActivityTime = millis();
    
    if (systemStatus == 2 && alertType == 2) {
      resetAlert();
      sendNotification(F("Motion Back"));
    }
  }
}

void checkButton() {
  if (digitalRead(BUTTON_PIN) == LOW && !buttonPressed) {
    buttonPressTime = millis();
    buttonPressed = 1;
  }
  
  if (digitalRead(BUTTON_PIN) == HIGH && buttonPressed) {
    unsigned long pressDuration = millis() - buttonPressTime;
    buttonPressed = 0;
    
    if (pressDuration > 1500) {
      if (systemStatus != 2) triggerManualAlert();
    } else if (pressDuration > 50) {
      if (systemStatus == 2) resetAlert();
    }
  }
}

void checkInactivity() {
  if (systemStatus == 1) {
    unsigned long inactiveTime = millis() - lastActivityTime;
    if (inactiveTime > INACTIVITY_WARNING) {
      triggerInactivityAlert();
    }
  }
}

void triggerFallAlert() {
  systemStatus = 2;
  alertType = 1;
  alertStartTime = millis();
  
  Serial.println(F("Alert: FALL"));
  activateAlarm();
  sendNotification(F("ALERT: FALL"));
  
  lcd.clear();
  lcd.print(F("FALL DETECTED"));
  lcd.setCursor(0, 1);
  lcd.print(F("30s -> Alert"));
}

void triggerInactivityAlert() {
  systemStatus = 2;
  alertType = 2;
  alertStartTime = millis();
  
  Serial.println(F("Alert: NO MOVE"));
  activateAlarm();
  sendNotification(F("ALERT: NO MOVE"));
  
  lcd.clear();
  lcd.print(F("NO ACTIVITY"));
  lcd.setCursor(0, 1);
  lcd.print(F("30s -> Alert"));
}

void triggerManualAlert() {
  systemStatus = 2;
  alertType = 3;
  alertStartTime = millis();
  
  Serial.println(F("Alert: MANUAL SOS"));
  activateAlarm();
  sendNotification(F("ALERT: MANUAL SOS"));
  
  lcd.clear();
  lcd.print(F("MANUAL SOS"));
  lcd.setCursor(0, 1);
  lcd.print(F("30s -> Alert"));
}

void activateAlarm() {
  setRGB(255, 0, 0);
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(300);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
  }
  digitalWrite(BUZZER_PIN, HIGH);
}

void updateAlertStatus() {
  if (systemStatus == 2) {
    unsigned long alertTime = millis() - alertStartTime;
    
    if (millis() % 1000 < 100) {
      int remaining = (NOTIFICATION_DELAY - alertTime) / 1000;
      if (remaining >= 0) {
        lcd.setCursor(0, 1);
        lcd.print(F("Alert in: "));
        lcd.print(remaining);
        lcd.print(F("s  "));
      }
    }
    
    if (alertTime > NOTIFICATION_DELAY) {
      sendEmergencyNotification();
    }
  }
}

void resetAlert() {
  systemStatus = 1;
  alertType = 0;
  digitalWrite(BUZZER_PIN, LOW);
  lastActivityTime = millis();
  
  if (bluetoothConnected) {
    setRGB(0, 255, 0);
  } else {
    setRGB(255, 255, 0);
  }
  
  sendNotification(F("Alert OFF"));
  Serial.println(F("Alert Reset"));
  lcd.clear();
  lcd.print(F("Monitoring..."));
}
void sendNotification(const __FlashStringHelper* message) {
  Serial.println(message);
  if (bluetoothConnected) {
    bluetooth.println(message);
  }
}

void sendNotification(const char* message) {
  Serial.println(message);
  if (bluetoothConnected) {
    bluetooth.println(message);
  }
}

void sendEmergencyNotification() {
  Serial.println(F("EMERGENCY SENT"));
  
  if (alertType == 1) {
    sendNotification(F("EMERGENCY: FALL"));
  }
  else if (alertType == 2) {
    sendNotification(F("EMERGENCY: NO MOVE"));
  }
  else if (alertType == 3) {
    sendNotification(F("EMERGENCY: SOS"));
  }
  
  delay(2000);
  resetAlert();
}

void sendTestNotification() {
  sendNotification(F("TEST: OK"));
  if (accel.available()) {
    sendNotification(F("Accel: OK"));
  }
}
void sendSystemStatus() {
  if (systemStatus == 2) {
    if (alertType == 1) sendNotification(F("ST:Alert-Fall"));
    else if (alertType == 2) sendNotification(F("ST:Alert-NoMove"));
    else if (alertType == 3) sendNotification(F("ST:Alert-SOS"));
  } else {
    unsigned long inactiveSec = (millis() - lastActivityTime) / 1000;
    String status = "ST:Mon | Inact:";
    status += inactiveSec;
    status += "s";
    sendNotification(status.c_str());
  }
}

void updateDisplay() {
  if (millis() - lastDisplayUpdate < DISPLAY_UPDATE) return;
  lastDisplayUpdate = millis();
  
  if (systemStatus != 2) {
    lcd.clear();
    if (!bluetoothConnected) {
      lcd.print(F("Wait BT"));
      lcd.setCursor(0, 1);
      lcd.print(F("Pair HC-05"));
    } else {
      lcd.print(F("Monitor..."));
      lcd.setCursor(0, 1);
      unsigned long inactiveSec = (millis() - lastActivityTime) / 1000;
      lcd.print(F("Inact:"));
      lcd.print(inactiveSec);
      lcd.print(F("s"));
    }
  }
}

void setRGB(byte r, byte g, byte b) {
  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN, b);
}
