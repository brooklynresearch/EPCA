// libraries
#include <Bounce.h>
#include <Timer.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <FastLED.h>
#include "definitions.h"
#include "debug.h"

#define DEBUG // comment out to turn of debug serial printout

// game
bool gameStarted = false;

// motor movement
const float playerAdvance = .75; // client wants 4" movement, .75 sec @ 255 on 168RPM ~= 4"
const float multiplierVals[3] = {
  playerAdvance * 3,
  playerAdvance * 2,
  playerAdvance
};

// lights
CRGB leds[NUM_LEDS];

// audio
AudioPlaySdWav           playSdWav1;
AudioOutputAnalogStereo  dacs1;
AudioConnection          patchCord1(playSdWav1, 0, dacs1, 0);
AudioConnection          patchCord2(playSdWav1, 1, dacs1, 1);
uint8_t soundNum = 0;

void initGame() {   // runs once on setup
  pinMode(RESET_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  // init all IO pins
  for (int i = 0; i < numSensors; i++) {
    pinMode(playerInputs[i], INPUT);
    digitalWrite(playerInputs[i], HIGH); // need initial pullup for beam break sensors

    if (i < numPlayers) {
      pinMode(limitSwitchesLeft[i], INPUT_PULLUP);
      pinMode(limitSwitchesRight[i], INPUT_PULLUP);
      //pinMode(motorEnable[i], OUTPUT);
      pinMode(motorDirectionA[i], OUTPUT);
      pinMode(motorDirectionB[i], OUTPUT);
    }

  }

  // setup LEDs
  FastLED.addLeds<WS2813, LED_PIN, GRB>(leds, NUM_LEDS);
  //FastLED.setBrightness(128);

  DEBUG_PRINTLN();
  DEBUG_PRINTLN();
  DEBUG_PRINTLN("---------------------------");
  DEBUG_PRINTLN("Game Online");
  DEBUG_PRINTLN("---------------------------");
  DEBUG_PRINTLN();

  resetGame();
}


void resetGame() { // runs at start up or master reset

  // reset input arrays
  for (uint8_t i = 0; i < numSensors; i++) {
    currentVal[i] = 0;
    previousVal[i] = 0;

    if (i < numPlayers) {
      playerMovementPeriod[i] = 0;
      motorMoving[i] = false;
    }
  }

  // reset lights
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();

  // move players to home
  resetPlayers();


  DEBUG_PRINTLN();
  DEBUG_PRINTLN("---------------------------");
  DEBUG_PRINTLN("Game reset");
  DEBUG_PRINTLN("---------------------------");
  DEBUG_PRINTLN();

  gameStarted = true;
}

void setup() {
  Serial.begin(115200);
  //analogWriteFrequency(20, 20000);
  AudioMemory(8);
  initGame();

  if (!SD.begin(BUILTIN_SDCARD)) {
    DEBUG_PRINTLN("unable to read from SD card");
    return;
  }
}

void loop() {
  if (resetCheck()) resetGame();

  if (gameStarted) {
    getSensorVals();
    playerCheck();
    getWinner();
    getHome();
  }
}

void getSensorVals() {    // read input sensors on skee ball lanes
  for (uint8_t i = 0; i < numSensors; i++) {
    currentVal[i] = digitalRead(playerInputs[i]);

    if (!currentVal[i] && previousVal[i]) { // sensor has been triggered
      uint8_t playerNum = floor(i / numSensors);  // first 6 vals are p1, next 6 are p2, ... p6
      float multiplier =  multiplierVals[i % numSensors];

      DEBUG_PRINT("Player ");
      DEBUG_PRINT(playerNum + 1);
      DEBUG_PRINT(" sensor on pin ");
      DEBUG_PRINT(playerInputs[i]);
      DEBUG_PRINT(" was triggered.");
      DEBUG_PRINTLN();

      movePlayer(playerNum, multiplier);
    }

    previousVal[i] = currentVal[i];
  }
}

void movePlayer(uint8_t playerNum, float multiplier) {  // calculate time period to move motor, move it
  unsigned long period = multiplier * 1000;
  motorPosition[playerNum] += period;
  playerMovementPeriod[playerNum] += period;

  DEBUG_PRINT("Moving player ");
  DEBUG_PRINT(playerNum + 1);
  DEBUG_PRINT(" forward for ");
  DEBUG_PRINT(period / 1000);
  DEBUG_PRINTLN(" seconds");
  DEBUG_PRINTLN();

  DEBUG_PRINT("Total period time: ");
  DEBUG_PRINT(playerMovementPeriod[playerNum] / 1000);
  DEBUG_PRINTLN(" seconds");
  DEBUG_PRINTLN();

  digitalWrite(motorDirectionA[playerNum], HIGH); // move player forward: A = HIGH, B = 0-254 (0 is fastest)
  analogWrite(motorDirectionB[playerNum], 0);

  if (!motorMoving[playerNum]) startMillis[playerNum] = millis(); // start the timer the first time -- if motor is already moving, the player triggered a sensor before the original movement finished
  motorMoving[playerNum] = true;
}

void playerCheck() {
  for (int i = 0; i < numPlayers; i++) {
    if (motorMoving[i]) {
      if (millis() - startMillis[i] >= playerMovementPeriod[i]) {
        stopPlayer(i);
      }
    }
  }
}

void stopPlayer(uint8_t playerNum) {
  digitalWrite(motorDirectionA[playerNum], LOW); // stop movement: A = LOW, B = 0 OR A = HIGH, B = 255
  analogWrite(motorDirectionB[playerNum], 0);
  playerMovementPeriod[playerNum] = 0;
  motorMoving[playerNum] = false;
}

void resetPlayers() {
  for (int i = 0; i < numPlayers; i++) {
    DEBUG_PRINT("Moving player ");
    DEBUG_PRINT(i + 1);
    DEBUG_PRINTLN(" to home");

    digitalWrite(motorDirectionA[i], LOW);   // move player backward: A = LOW, B = 255-1 (255 is fastest)
    analogWrite(motorDirectionB[i], 255);
  }

  bool allHome = false;

  // wait until all motor positions are home (0)
  while (!allHome) {
    getHome();

    uint8_t total = 0;
    for (int i = 0; i < numPlayers; i++) {
      total += motorPosition[i];
    }

    if (total == 0) allHome = true;

  }
}

void getWinner() {  // check if right side limit switch is triggered
  for (int i = 0; i < numPlayers; i++) {
    if (rightLimits[i].update()) {
      if (rightLimits[i].fallingEdge()) {

        // stop all players
        for (uint8_t i = 0; i < numPlayers; i++) {
          digitalWrite(motorDirectionA[i], LOW);
          analogWrite(motorDirectionB[i], 0);
        }

        DEBUG_PRINTLN();
        DEBUG_PRINT("Winner! Player ");
        DEBUG_PRINT(i + 1);
        DEBUG_PRINTLN();
        DEBUG_PRINTLN();

        gameStarted = false;
        
        playWinner();



        DEBUG_PRINTLN();
        DEBUG_PRINTLN("---------------------------");
        DEBUG_PRINTLN("Game End");
        DEBUG_PRINTLN("---------------------------");
        DEBUG_PRINTLN();

        resetGame();  //reset game
      }
    }
  }
}

void playWinner() {
  if (soundNum >= 3) soundNum = 0;
  const char* winSounds[3] = {
    "WIN1.WAV",
    "WIN2.WAV",
    "WIN3.WAV"
  };

  unsigned long period = 4000; // total delay period to flash lights and play audio after game end (ms)
  unsigned long lightPeriod = 250; // time between light flashes (ms)

  // call audio file
  playSdWav1.play(winSounds[soundNum]);
  delay(10); // wait for library to parse WAV info

  // lights
  bool lightOn = false;
  unsigned long checkMillis = millis();
  unsigned long previousMillis = 0;
  unsigned long currentMillis = 0;

  while (millis() - checkMillis <= period) {
    currentMillis = millis();

    if (currentMillis - previousMillis >= lightPeriod) {
      if (lightOn) {
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        FastLED.show();
      } else {
        fill_solid(leds, NUM_LEDS, CRGB::White);
        FastLED.show();
      }

      lightOn = !lightOn;
      previousMillis = currentMillis;
    }
  }

  FastLED.clear();

  soundNum++;
}

void getHome() {  // check if left side limit switch is triggered
  for (int i = 0; i < numPlayers; i++) {
    if (leftLimits[i].update()) {
      if (leftLimits[i].fallingEdge()) {
        DEBUG_PRINTLN();
        DEBUG_PRINT("Player ");
        DEBUG_PRINT(i + 1);
        DEBUG_PRINT(" home switch hit");
        DEBUG_PRINTLN();

        //analogWrite(motorEnable[i], 0);   // stop the motor
        analogWrite(motorDirectionB[i], 0);
        motorPosition[i] = 0; // reset postion val
      }
    }
  }
}

bool resetCheck() {  // check if master reset button was pressed
  if (resetButton.update()) {
    if (resetButton.fallingEdge()) {
      DEBUG_PRINTLN();
      DEBUG_PRINTLN("Reset switch was pressed, resetting game...");
      DEBUG_PRINTLN();

      return true;
    }
  }
  return false;
}
