/**
 * @file sketch.ino
 * @brief Lab1: Prekidi u ugradbenim sustavima - ESP32
 * Demonstracija višestrukih prekida: tipke, timer, senzor udaljenosti.
 */

#include <Ticker.h>

// Pinovi
const int btnHigh = 18;
const int btnMid = 19;
const int btnLow = 21;
const int ledPin = 2;
const int trigPin = 5;
const int echoPin = 4;

// Flagovi
volatile bool btnHighPressed = false;
volatile bool btnMidPressed = false;
volatile bool btnLowPressed = false;
volatile bool timerFired = false;

// Timer
Ticker timer1s;

// ISR za tipke
void IRAM_ATTR handleHighPriorityBtn() { btnHighPressed = true; }
void IRAM_ATTR handleMidPriorityBtn()  { btnMidPressed  = true; }
void IRAM_ATTR handleLowPriorityBtn()  { btnLowPressed  = true; }

// ISR za timer
void onTimerTick() { timerFired = true; }

void setup() {
  Serial.begin(115200);

  pinMode(btnHigh, INPUT_PULLUP);
  pinMode(btnMid, INPUT_PULLUP);
  pinMode(btnLow, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  attachInterrupt(digitalPinToInterrupt(btnHigh), handleHighPriorityBtn, FALLING);
  attachInterrupt(digitalPinToInterrupt(btnMid), handleMidPriorityBtn, FALLING);
  attachInterrupt(digitalPinToInterrupt(btnLow), handleLowPriorityBtn, FALLING);

  timer1s.attach(1.0, onTimerTick);
}

void loop() {
  if (btnHighPressed) {
    btnHighPressed = false;
    Serial.println("Tipkalo visokog prioriteta pritisnuto");
  } else if (btnMidPressed) {
    btnMidPressed = false;
    Serial.println("Tipkalo srednjeg prioriteta pritisnuto");
  } else if (btnLowPressed) {
    btnLowPressed = false;
    Serial.println("Tipkalo niskog prioriteta pritisnuto");
  }

  if (timerFired) {
    timerFired = false;

    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    unsigned long duration = pulseIn(echoPin, HIGH, 30000);
    if (duration == 0) {
      Serial.println("Nema reflektiranog signala.");
      return;
    }

    float distance = duration * 0.0343 / 2;

    Serial.print("Udaljenost: ");
    Serial.print(distance);
    Serial.println(" cm");

    if (distance < 20.0) {
      digitalWrite(ledPin, HIGH);
    } else {
      digitalWrite(ledPin, LOW);
    }
  }
}
