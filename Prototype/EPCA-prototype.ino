// libraries
#include <Bounce.h>
#include <Timer.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <FastLED.h>
#include "definitions.h"

// game
bool gameStarted = false;

// motor movement
const uint8_t motorSpeed = 255;
const uint8_t multiplierVals[6] = {6, 6, 6, 4, 4, 2};

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

  // init all IO pins
  for (int i = 0; i < numSensors; i++) {
    pinMode(playerInputs[i], INPUT);
    digitalWrite(playerInputs[i], HIGH); // need initial pullup for beam break sensors

    if (i < numPlayers) {
      pinMode(limitSwitchesLeft[i], INPUT_PULLUP);
      pinMode(limitSwitchesRight[i], INPUT_PULLUP);
      pinMode(motorEnable[i], OUTPUT);
      pinMode(motorDirectionA[i], OUTPUT);
      pinMode(motorDirectionB[i], OUTPUT);
    }

  }

  // setup LEDs
  FastLED.addLeds<WS2813, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(128);

  Serial.println();
  Serial.println();
  Serial.println("---------------------------");
  Serial.println("Game Online");
  Serial.println("---------------------------");
  Serial.println();

  resetGame();
}


void resetGame() { // runs at start up or master reset

  // reset input arrays
  for (uint8_t i = 0; i < numSensors; i++) {
    currentVal[i] = 0;
    previousVal[i] = 0;
  }

  // move players to home
  resetPlayers();

  Serial.println("---------------------------");
  Serial.println("Game reset");
  Serial.println("---------------------------");
  Serial.println();

  gameStarted = true;
}

void setup() {
  Serial.begin(115200);
  AudioMemory(8);
  initGame();

  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("unable to read from SD card");
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

      // call audio file
      playSdWav1.play("ball.wav");
      delay(10); // wait for library to parse WAV info

      uint8_t playerNum = floor(i / 6);  // first 6 vals are p1, next 6 are p2, ... p6
      uint8_t multiplier =  multiplierVals[i % 6];

      Serial.print("Player ");
      Serial.print(playerNum + 1);
      Serial.print(" sensor on pin ");
      Serial.print(playerInputs[i]);
      Serial.print(" was triggered.");
      Serial.println();

      movePlayer(playerNum, multiplier);
    }

    previousVal[i] = currentVal[i];
  }
}

void movePlayer(uint8_t playerNum, uint8_t multiplier) {  // calculate time period to move motor, move it
  unsigned long period = multiplier * 1000;
  motorPosition[playerNum] += period;
  playerMovementPeriod[playerNum] += period;

  Serial.print("Moving player ");
  Serial.print(playerNum + 1);
  Serial.print(" forward for ");
  Serial.print(period / 1000);
  Serial.println(" seconds");
  Serial.println();

  Serial.print("Total period time: ");
  Serial.print(playerMovementPeriod[playerNum] / 1000);
  Serial.println(" seconds");
  Serial.println();

  digitalWrite(motorDirectionA[playerNum], HIGH); // move player forward
  digitalWrite(motorDirectionB[playerNum], LOW);
  analogWrite(motorEnable[playerNum], motorSpeed);

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
  analogWrite(motorEnable[playerNum], 0);
  playerMovementPeriod[playerNum] = 0;
  motorMoving[playerNum] = false;
}

void resetPlayers() {
  for (int i = 0; i < numPlayers; i++) {
    Serial.print("Moving player ");
    Serial.print(i + 1);
    Serial.println(" to home");

    digitalWrite(motorDirectionA[i], LOW);   // move player backward
    digitalWrite(motorDirectionB[i], HIGH);
    analogWrite(motorEnable[i], motorSpeed);
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
        analogWrite(motorEnable[i], 0);

        Serial.println();
        Serial.print("Winner! Player ");
        Serial.print(i + 1);
        Serial.println();
        Serial.println();

        playWinner();

        gameStarted = false;

        Serial.println("---------------------------");
        Serial.println("Game End");
        Serial.println("---------------------------");
        Serial.println();

        resetGame();  //reset game
      }
    }
  }
}

void playWinner() {
  if (soundNum >= 3) soundNum = 0;
  const char* winSounds[3] = {
    "win1.wav",
    "win2.wav",
    "win3.wav"
  };

  // call audio file
  playSdWav1.play(winSounds[soundNum]);
  delay(10); // wait for library to parse WAV info

  // lights
  fill_solid(leds, NUM_LEDS, CRGB::White);
  FastLED.show();

  // wait until sound effect finishes playing
  while (playSdWav1.isPlaying()) {
  }

  FastLED.clear();

  soundNum++;
}

void getHome() {  // check if left side limit switch is triggered
  for (int i = 0; i < numPlayers; i++) {
    if (leftLimits[i].update()) {
      if (leftLimits[i].fallingEdge()) {
        Serial.println();
        Serial.print("Player ");
        Serial.print(i + 1);
        Serial.print(" home switch hit");
        Serial.println();

        analogWrite(motorEnable[i], 0);   // stop the motor
        motorPosition[i] = 0; // reset postion val
      }
    }
  }
}

bool resetCheck() {  // check if master reset button was pressed
  if (resetButton.update()) {
    if (resetButton.fallingEdge()) {
      Serial.println();
      Serial.println("Reset switch was pressed, resetting game...");
      Serial.println();

      return true;
    }
  }
  return false;
}
