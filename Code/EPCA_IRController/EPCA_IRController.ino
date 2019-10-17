#include <Bounce2.h>

//#define DEBUG 1

int DETECTOR_ONE_PIN = A0;
int DETECTOR_TWO_PIN = A1;
int DETECTOR_THREE_PIN = A2;
int DETECTOR_FOUR_PIN = A3;
int DETECTOR_FIVE_PIN = A4;
int DETECTOR_SIX_PIN = A5;

#define ON_DELAY  25


#define OUTPUT_ONE_PIN  10
#define OUTPUT_TWO_PIN  9
#define OUTPUT_THREE_PIN 11
#define OUTPUT_FOUR_PIN 12

Bounce detectorOne = Bounce(DETECTOR_ONE_PIN, 20);
Bounce detectorTwo = Bounce(DETECTOR_TWO_PIN, 20);
Bounce detectorThree = Bounce(DETECTOR_THREE_PIN, 20);
Bounce detectorFour = Bounce(DETECTOR_FOUR_PIN, 20);
Bounce detectorFive = Bounce(DETECTOR_FIVE_PIN, 20);
Bounce detectorSix = Bounce(DETECTOR_SIX_PIN, 20);

Bounce IRDetector[6] = {detectorOne, detectorTwo, detectorThree, detectorFour, detectorFive, detectorSix};

// skee ball sensor value arrays
uint8_t currentVal[6];
uint8_t previousVal[6];

void setup() {
  //Serial Port begin
  Serial.begin (9600);
  //Define inputs and outputs
  pinMode(OUTPUT_ONE_PIN, OUTPUT);
  pinMode(OUTPUT_TWO_PIN, OUTPUT);
  pinMode(OUTPUT_THREE_PIN, OUTPUT);
  pinMode(OUTPUT_FOUR_PIN, OUTPUT);
}

void loop() {
  for (int i = 0; i < 6; i++) {
    IRDetector[i].update();
    if (IRDetector[i].fell()) {
      Serial.print("IR Detector #");
      Serial.print(i);
      Serial.print(" triggered");
      Serial.println();

      if (i == 0) {
        digitalWrite(OUTPUT_ONE_PIN, LOW);
        Serial.println("Short move");
        delay(ON_DELAY);
      } else if (i <= 2) {
        digitalWrite(OUTPUT_TWO_PIN, LOW);
        Serial.println("Medium move");
        delay(ON_DELAY);
      } else {
        digitalWrite(OUTPUT_THREE_PIN, LOW);
        Serial.println("Long move");
        delay(ON_DELAY);
      }
    }
  }

  resetOutputs();
}

void resetOutputs(){
  digitalWrite(OUTPUT_ONE_PIN, HIGH);
  digitalWrite(OUTPUT_TWO_PIN, HIGH);
  digitalWrite(OUTPUT_THREE_PIN, HIGH);
  digitalWrite(OUTPUT_FOUR_PIN, HIGH);
}

