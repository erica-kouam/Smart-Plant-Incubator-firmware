#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <SoftwareSerial.h>

// ---------- Hardware Subsystem Interfaces ----------
SoftwareSerial espSerial(10, 11); // RX (Pin 10), TX (Pin 11) to ESP8266 Bus [2]

#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE); // [2]

LiquidCrystal_I2C lcd(0x27, 16, 2); // [2]

// ---------- Concurrency Timing State Registers ----------
unsigned long lastScreenSwitch  = 0;
unsigned long lastWaterReminder = 0;
unsigned long lastStatusSend    = 0;

const unsigned long SCREEN_INTERVAL = 5000;    // 5-second display alternate cycle [2]
const unsigned long WATER_INTERVAL  = 300000;  // 5-minute hydration tracking window [2]
const unsigned long STATUS_INTERVAL = 60000;   // 60-second structural telemetry broadcast [2]

bool showMessageScreen = true; // [2]

// ---------- Operational Environmental Thresholds ----------
const float TEMP_HIGH = 28.0; // [2]
const float TEMP_LOW  = 20.0; // [2]
const float HUM_LOW   = 55.0; // [2]
const float HUM_HIGH  = 85.0; // [2]

// --- Edge-Trigger Flags (Prevents Serial Buffer Overflows / Flooding) ---
bool tempHighAlertActive = false;
bool tempLowAlertActive  = false;
bool humHighAlertActive  = false;
bool humLowAlertActive   = false;

void setup() {
  Serial.begin(9600);      // Local USB Hardware Debugging Console [2]
  espSerial.begin(9600);   // Transmit Link to ESP8266 Network Layer [2]

  Wire.begin(); // [2]
  dht.begin(); // [2]

  lcd.init(); // [2]
  lcd.backlight(); // [2]

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Incubator Boot");
  lcd.setCursor(0,1);
  lcd.print("Please wait");
  delay(2000); // Standard hardware settlement delay [2]
  lcd.clear();
}

void loop() {
  unsigned long now = millis(); // Master reference system clock cycle [2]

  float hum  = dht.readHumidity(); // [2]
  float temp = dht.readTemperature(); // [2]

  // ---------- LCD REFRESH CLOCK SWITCH (Non-Blocking) ----------
  if (now - lastScreenSwitch >= SCREEN_INTERVAL) {
    lastScreenSwitch = now;
    showMessageScreen = !showMessageScreen;
    lcd.clear();
  }

  // ---------- Data Validation & Defensive Interception Pipeline ----------
  if (isnan(hum) || isnan(temp)) {
    // Non-blocking error handling allows screen to refresh safely without freezing the chip execution loop
    lcd.setCursor(0,0);
    lcd.print("DHT SENSOR ERROR");
    lcd.setCursor(0,1);
    lcd.print("Bus Fault / NaN ");
    return; // Safe cycle escape [2]
  }

  // ---------- Local HMI Display State Machine ----------
  if (showMessageScreen) {
    lcd.setCursor(0,0);
    lcd.print("Hello, there is"); // [2]
    lcd.setCursor(0,1);
    lcd.print("a plant growing"); // [2]
  } else {
    lcd.setCursor(0,0);
    lcd.print("Hum: ");
    lcd.print(hum, 1);
    lcd.print("%   ");

    lcd.setCursor(0,1);
    lcd.print("Temp: ");
    lcd.print(temp, 1);
    lcd.print(" C  ");
  }

  // ---------- Telemetry Sync Loop (Every 60 Seconds) ----------
  if (now - lastStatusSend >= STATUS_INTERVAL) {
    lastStatusSend = now;
    espSerial.print("STATUS:TEMP=");
    espSerial.print(temp, 1);
    espSerial.print(",HUM=");
    espSerial.println(hum, 1); // [2]
  }

  // ---------- Edge-Triggered Thermal System Safeguards ----------
  if (temp > TEMP_HIGH) {
    if (!tempHighAlertActive) {
      espSerial.println("ALERT:TEMP_HIGH");
      tempHighAlertActive = true; // Lock register to prevent communication bus jamming
    }
  } else if (temp < TEMP_LOW) {
    if (!tempLowAlertActive) {
      espSerial.println("ALERT:TEMP_LOW");
      tempLowAlertActive = true;
    }
  } else {
    // Automatic system flag clearing once limits clear back to nominal thresholds
    tempHighAlertActive = false;
    tempLowAlertActive  = false;
  }

  // ---------- Edge-Triggered Hydration System Safeguards ----------
  if (hum < HUM_LOW) {
    if (!humLowAlertActive) {
      espSerial.println("ALERT:HUM_LOW");
      humLowAlertActive = true;
    }
  } else if (hum > HUM_HIGH) {
    if (!humHighAlertActive) {
      espSerial.println("ALERT:HUM_HIGH");
      humHighAlertActive = true;
    }
  } else {
    humLowAlertActive  = false;
    humHighAlertActive = false;
  }

  // ---------- Utility Operations & Maintenance Reminders ----------
  if (now - lastWaterReminder >= WATER_INTERVAL) {
    lastWaterReminder = now;
    espSerial.println("REMINDER:WATER"); // [2]
  }
}
