// libraries
#include <Bounce.h>
#include <Timer.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <FastLED.h>
#include "definitions.h"

// comment out to turn of debug serial printout
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)

// game
bool gameStarted = false;
bool systemRestart = true;

// motor movement
const float playerAdvance = 1; // client wants 4" movement, .75 sec @ 255 on 168RPM ~= 4"
const float multiplierVals[3] = {
  playerAdvance * 3,
  playerAdvance * 2,
  playerAdvance
};

const float playerAdjustment[6] = {
  1, //PLAYER ONE
  1, //PLAYER TWO
  1, //PLAYER THREE
  1, //PLAYER FOUR
  1, //PLAYER FIVE
  1, //PLAYER SIX
};

long bounceTimingsForIR[36] = {
  0,0,0,  //PLAYER ONE
  0,0,0,  //PLAYER TWO
  0,0,0,  //PLAYER THREE
  0,0,0,  //PLAYER FOUR
  0,0,0,  //PLAYER FIVE
  0,0,0,  //PLAYER SIX
};

uint8_t irThreshold[36] = {
  10, 10, 10, 
  10, 10, 10,
  10, 10, 10,
  10, 10, 10,
  10, 10, 10,
  10, 10, 10,
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

  pinMode(LOAD_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(DATA_PIN, INPUT);

  // init all IO pins
  for (int i = 0; i < numSensors; i++) {

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

  digitalWrite(CLOCK_PIN, LOW);
  digitalWrite(LOAD_PIN, HIGH);

  SENSOR_VALUES = read_shift_regs();
  print_values();
  OLD_SENSOR_VALUES = SENSOR_VALUES;

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
  getHome();

  if (gameStarted) {
    SENSOR_VALUES = read_shift_regs();
    if(SENSOR_VALUES != OLD_SENSOR_VALUES){
      print_values();
      checkMovement();
      OLD_SENSOR_VALUES = SENSOR_VALUES;
    }
    playerCheck();
    getWinner();
  }
}

unsigned long read_shift_regs()
{
    long bitVal;
    unsigned long bytesVal = 0;

    digitalWrite(ENABLE_PIN, HIGH);
    digitalWrite(LOAD_PIN, LOW);
    delayMicroseconds(5);
    digitalWrite(LOAD_PIN, HIGH);
    digitalWrite(ENABLE_PIN, LOW);

    for(int i = 0; i < DATA_WIDTH; i++)
    {
        bitVal = digitalRead(DATA_PIN);
        motorTriggerValue[i] = bitVal;
        bytesVal |= (bitVal << ((DATA_WIDTH-1) - i));

        digitalWrite(CLOCK_PIN, HIGH);
        delayMicroseconds(5);
        digitalWrite(CLOCK_PIN, LOW);
    }

    return(bytesVal);
}

void print_values() { 
  byte i; 

  Serial.println("*Shift Register Values:*\r\n");

  for(byte i=0; i<=DATA_WIDTH-1; i++) 
  { 
    Serial.print("S");
    Serial.print(i);
    Serial.print(" "); 
  }
  Serial.println();
  for(byte i=0; i<=DATA_WIDTH-1; i++) 
  { 
    Serial.print(SENSOR_VALUES >> i & 1, BIN); 
    
    if(i>8){Serial.print(" ");}
    Serial.print("  "); 
    
  } 
  
  Serial.print("\n"); 
  Serial.println();Serial.println();

}

void checkMovement(){
  for(int i=0; i< DATA_WIDTH; i++){
    if(!motorTriggerValue[i]){
      Serial.print("Player ");
      Serial.print(motorLookUp[i]);
      Serial.print(" with speed x=");
      Serial.print(multiplierLookUp[i]);
      Serial.println(" moved");
      movePlayer(motorLookUp[i], multiplierLookUp[i]);
    }
  }
}

void movePlayer(uint8_t playerNum, float multiplier) {  // calculate time period to move motor, move it
  float period = multiplier * 2000;
  motorPosition[playerNum] += period * playerAdjustment[playerNum];
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
  if(playerNum == 2){
    analogWrite(motorDirectionB[playerNum], 60);
  } else {
    analogWrite(motorDirectionB[playerNum], 0);
  }
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

    // quick forward and back to fix issue where left limit switches didn't get pressed when player was already against the left side
    digitalWrite(motorDirectionA[i], HIGH); // move player forward: A = HIGH, B = 0-254 (0 is fastest)
    analogWrite(motorDirectionB[i], 0);
    delay(75);
    digitalWrite(motorDirectionA[i], LOW);   // move player backward: A = LOW, B = 255-1 (255 is fastest)
    analogWrite(motorDirectionB[i], 255);
    delay(25);
    digitalWrite(motorDirectionA[i], HIGH); // move player forward: A = HIGH, B = 0-254 (0 is fastest)
    analogWrite(motorDirectionB[i], 0);
    delay(75);
    digitalWrite(motorDirectionA[i], LOW);   // move player backward: A = LOW, B = 255-1 (255 is fastest)
    analogWrite(motorDirectionB[i], 255);
    delay(25);

    //getHome();

    if (motorPosition[i] != 0) {
      digitalWrite(motorDirectionA[i], LOW);   // move player backward: A = LOW, B = 255-1 (255 is fastest)
       analogWrite(motorDirectionB[i], 255);
    }
  }

  bool allHome = false;

  // wait until all motor positions are home (0)
  while (!allHome) {
    getHome();

    uint8_t total = 0;
    for (int i = 0; i < numPlayers; i++) {
      total += motorPosition[i];
      //Serial.println(total);
    }

    if (total == 0) {
      Serial.println("All home!");

      allHome = true;
      gameStarted = true;

      DEBUG_PRINTLN();
      DEBUG_PRINTLN("---------------------------");
      DEBUG_PRINTLN("Game reset");
      DEBUG_PRINTLN("---------------------------");
      DEBUG_PRINTLN();
    }

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
  unsigned long lightPeriod = 17; // light timing

  // call audio file
  playSdWav1.play(winSounds[soundNum]);
  delay(10); // wait for library to parse WAV info

  // lights
  unsigned long checkMillis = millis();
  unsigned long previousMillis = 0;
  unsigned long currentMillis = 0;

  // for pulse
  bool lightOn = false;

  // for up and down
  uint8_t dot = 0;
  bool forward = true;

  while (millis() - checkMillis <= period) {
    currentMillis = millis();

        //pulse lights at lightPeriod ms
//        if (currentMillis - previousMillis >= lightPeriod) {
//          if (lightOn) {
//            fill_solid(leds, NUM_LEDS, CRGB::Black);
//            FastLED.show();
//          } else {
//            fill_solid(leds, NUM_LEDS, CRGB::White);
//            FastLED.show();
//          }
//    
//          lightOn = !lightOn;
//          previousMillis = currentMillis;
//        }

    // light up and down strip, pixel movement at lightPeriod ms;
    if (currentMillis - previousMillis >= lightPeriod) {
      leds[dot] = CRGB::White;
      FastLED.show();

      if (forward) {
        if (dot != 0) leds[dot - 1] = CRGB::Black;
        dot++;
      } else {
        leds[dot + 1] = CRGB::Black;
        dot--;
      }

      FastLED.show();

      if (dot >= 20) forward = false;
      if (dot <= 0) forward = true;
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
