#include <math.h>

// Testni način rada za simulaciju u Wokwi
#define FAST_TEST true

#if FAST_TEST
  #define TEMP_INTERVAL_MS 300  // Vrijeme između mjerenja temperature (0.3s)
  #define TIME_TO_SLEEP 5       // Vrijeme spavanja nakon ciklusa (5s)
#else
  #include <esp_sleep.h>
  #define TEMP_INTERVAL_MS 3000 // Vrijeme između mjerenja temperature (3s)
  #define TIME_TO_SLEEP 30      // Spavanje nakon ciklusa (30s)
  #define uS_TO_S_FACTOR 1000000ULL
#endif

// HC-SR04 pinovi
const int trigPin = 5;
const int echoPin = 18;

// Tipkalo za provjeru prisutnosti kave
const int buttonPin = 33;

// LED-ice za prikaz razine vode (plave)
const int blueLED1 = 13;
const int blueLED2 = 12;
const int blueLED3 = 14;

// LED-ice za upozorenja (crvene)
const int redLED_water = 27;
const int redLED_filter = 26;

// LED-ica za rad kuhala (zelena)
const int greenLED = 25;

// NTC termistor (analogni ulaz)
const int tempPin = 34;

// Funkcija za očitanje udaljenosti pomoću HC-SR04
float readDistanceCM() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);  // mjerenje trajanja impulsa
  return duration * 0.034 / 2;  // pretvorba u centimetre
}

// Funkcija za očitanje temperature preko NTC otpornika
float readTemperatureC() {
  int analogValue = analogRead(tempPin);
  float voltage = analogValue * 3.3 / 4095.0;
  float resistance = (3.3 - voltage) * 10000.0 / voltage;
  float temperature = 1.0 / (log(resistance / 10000.0) / 3950.0 + 1.0 / 298.15) - 273.15;
  return temperature;
}

// Prikaz razine vode pomoću plavih LED-ica
void showWaterLevelLEDs(int level) {
  digitalWrite(blueLED1, level >= 1);
  digitalWrite(blueLED2, level >= 2);
  digitalWrite(blueLED3, level >= 3);
}

// Gašenje svih LED-ica
void clearAllLEDs() {
  digitalWrite(blueLED1, LOW);
  digitalWrite(blueLED2, LOW);
  digitalWrite(blueLED3, LOW);
  digitalWrite(redLED_water, LOW);
  digitalWrite(redLED_filter, LOW);
  digitalWrite(greenLED, LOW);
}

// Glavna funkcija (pokreće se nakon svakog buđenja ili resetiranja)
void setup() {
  Serial.begin(115200);  // Uključivanje serijskog monitora

  // Postavljanje pinova
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
  // Ispis uzroka buđenja (samo u stvarnom režimu)
  esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();
  Serial.print("Wakeup reason: ");
  Serial.println(wakeupReason == ESP_SLEEP_WAKEUP_TIMER ? "Timer" : "GPIO");
#endif

  // Mjerenje razine vode
  float distance = readDistanceCM();
  Serial.print("Udaljenost (voda): ");
  Serial.print(distance);
  Serial.println(" cm");

  // Provjera ima li kave (tipkalo)
  bool hasCoffee = digitalRead(buttonPin) == LOW;
  Serial.print("Filter s kavom: ");
  Serial.println(hasCoffee ? "DA" : "NE");

  // Određivanje razine vode na temelju udaljenosti
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

  // Prikaz razine vode LED-icama
  showWaterLevelLEDs(waterLevel);

  // Upozorenje: nema vode
  if (waterLevel == 0) {
    digitalWrite(redLED_water, HIGH);
  }

  // Upozorenje: nema kave
  if (!hasCoffee) {
    digitalWrite(redLED_filter, HIGH);
  }

  // Ako su svi uvjeti ispunjeni – pokreće se grijanje
  if (waterLevel >= 2 && hasCoffee) {
    Serial.println("Pokrećem grijanje...");
    digitalWrite(greenLED, HIGH);  // Zelena LED – aktivno grijanje

    unsigned long lastCheck = millis();  // vrijeme posljednje provjere temperature

    while (true) {
      // Provjera svakih TEMP_INTERVAL_MS milisekundi
      if (millis() - lastCheck >= TEMP_INTERVAL_MS) {
        lastCheck = millis();

        float temp = readTemperatureC();
        Serial.print("Temperatura: ");
        Serial.print(temp);
        Serial.println(" °C");

        if (temp >= 80.0) {
          Serial.println("Temperatura postignuta!");
          break;  // grijanje završava
        }
      }

#if !FAST_TEST
      // Light sleep za uštedu energije između mjerenja
      esp_sleep_enable_timer_wakeup(100 * 1000); // 100ms
      esp_light_sleep_start();
#else
      delay(50);  // Kratka pauza u testnom režimu
#endif
    }

    digitalWrite(greenLED, LOW);  // Grijanje gotovo – isključi zelenu LED-icu
  }

  delay(50);  // Stabilizacija
  clearAllLEDs();  // Gašenje svih LED-ica

  Serial.println("Završeno. Ulazak u mirovanje.");

#if !FAST_TEST
  // Postavljanje uvjeta za buđenje i ulazak u deep sleep
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 1);  // buđenje tipkalom (HIGH)
  esp_deep_sleep_start();
#else
  // U testnom načinu, ostani aktivan radi pregleda rezultata
  while (true);
#endif
}

void loop() {
  // loop() se ne koristi jer se setup() izvršava iznova nakon buđenja iz sleep-a
}
