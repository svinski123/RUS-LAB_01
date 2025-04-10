#include <math.h>

#define FAST_TEST true

#if FAST_TEST
  #define TEMP_INTERVAL_MS 300  // 0.3s
  #define TIME_TO_SLEEP 5       // 5s
#else
  #include <esp_sleep.h>
  #define TEMP_INTERVAL_MS 3000 // 3s
  #define TIME_TO_SLEEP 30      // 30s
  #define uS_TO_S_FACTOR 1000000ULL
#endif

// HC-SR04
const int trigPin = 5;
const int echoPin = 18;

// Tipkalo
const int buttonPin = 33;

// RGB LED - plave (voda)
const int blueLED1 = 13;
const int blueLED2 = 12;
const int blueLED3 = 14;

// RGB LED - crvene (upozorenja)
const int redLED_water = 27;
const int redLED_filter = 26;

// RGB LED - zelena (grijanje)
const int greenLED = 25;

// Termistor
const int tempPin = 34;

float readDistanceCM() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  return duration * 0.034 / 2;
}

float readTemperatureC() {
  int analogValue = analogRead(tempPin);
  float voltage = analogValue * 3.3 / 4095.0;
  float resistance = (3.3 - voltage) * 10000.0 / voltage;
  float temperature = 1.0 / (log(resistance / 10000.0) / 3950.0 + 1.0 / 298.15) - 273.15;
  return temperature;
}

void showWaterLevelLEDs(int level) {
  digitalWrite(blueLED1, level >= 1);
  digitalWrite(blueLED2, level >= 2);
  digitalWrite(blueLED3, level >= 3);
}

void clearAllLEDs() {
  digitalWrite(blueLED1, LOW);
  digitalWrite(blueLED2, LOW);
  digitalWrite(blueLED3, LOW);
  digitalWrite(redLED_water, LOW);
  digitalWrite(redLED_filter, LOW);
  digitalWrite(greenLED, LOW);
}

void setup() {
  Serial.begin(115200);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  pinMode(blueLED1, OUTPUT);
  pinMode(blueLED2, OUTPUT);
  pinMode(blueLED3, OUTPUT);
  pinMode(redLED_water, OUTPUT);
  pinMode(redLED_filter, OUTPUT);
  pinMode(greenLED, OUTPUT);

#if !FAST_TEST
  esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();
  Serial.print("Wakeup reason: ");
  Serial.println(wakeupReason == ESP_SLEEP_WAKEUP_TIMER ? "Timer" : "GPIO");
#endif

  float distance = readDistanceCM();
  Serial.print("Udaljenost (voda): ");
  Serial.print(distance);
  Serial.println(" cm");

  bool hasCoffee = digitalRead(buttonPin) == LOW;
  Serial.print("Filter s kavom: ");
  Serial.println(hasCoffee ? "DA" : "NE");

int waterLevel = 1;
if (distance > 40) {
  waterLevel = 0; // prazno
} else if (distance > 25) {
  waterLevel = 1;
} else if (distance > 10) {
  waterLevel = 2;
} else {
  waterLevel = 3;
}


    showWaterLevelLEDs(waterLevel);

  if (waterLevel == 0) {
    digitalWrite(redLED_water, HIGH);
  }

  if (!hasCoffee) {
    digitalWrite(redLED_filter, HIGH);
  }

  if (waterLevel >= 2 && hasCoffee) {
    Serial.println("Pokrećem grijanje...");
    digitalWrite(greenLED, HIGH);

    unsigned long lastCheck = millis();

    while (true) {
      if (millis() - lastCheck >= TEMP_INTERVAL_MS) {
        lastCheck = millis();

        float temp = readTemperatureC();
        Serial.print("Temperatura: ");
        Serial.print(temp);
        Serial.println(" °C");

        if (temp >= 80.0) {
          Serial.println("Temperatura postignuta!");
          break;
        }
      }

#if !FAST_TEST
      // Light sleep
      esp_sleep_enable_timer_wakeup(100 * 1000); // 100ms
      esp_light_sleep_start();
#else
      delay(50);
#endif
    }

    digitalWrite(greenLED, LOW);
  }

  delay(50);
  clearAllLEDs();

  Serial.println("Završeno. Ulazak u mirovanje.");

#if !FAST_TEST
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 1);
  esp_deep_sleep_start();
#else
  // Ovdje samo pauziramo da vidiš kraj simulacije
  while (true);
#endif
}

void loop() {
  // Ne koristi se
}
