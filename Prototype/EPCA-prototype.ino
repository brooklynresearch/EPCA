// libraries
#include <Bounce.h>

// button pins
#define RESET_PIN 10
#define P1_LIMIT_LEFT 11
#define P1_LIMIT_RIGHT 12

// motor control pins
#define P1_MOTOR_ENABLE 8
#define P1_MOTOR_DIR 9

/** pins for player inputs from top to bottom, left to right:
          100
        50   50
      25  25  25
*/
#define P1_100 2
#define P1_50_LEFT 3
#define P1_50_RIGHT 4
#define P1_25_LEFT 5
#define P1_25_MIDDLE 6
#define P1_25_RIGHT 7

// setup game variables
#define numPlayers 1
#define numSensors (numPlayers * 6)
#define gameStarted false
#define playerWon false

const uint8_t motorSpeed = 100;
byte resetTimer = false;
float multiplierVals[6] = {0.6, 0.4, 0.4, 0.2, 0.2, 0.2};

// setup arrays for inputs
uint16_t playerInputs[numSensors] = {
  P1_100,
  P1_50_LEFT,
  P1_50_RIGHT,
  P1_25_LEFT,
  P1_25_MIDDLE,
  P1_25_RIGHT
};

uint8_t limitSwitchesLeft[numPlayers] = {
  P1_LIMIT_LEFT
};

uint8_t limitSwitchesRight[numPlayers] = {
  P1_LIMIT_RIGHT
};

uint16_t motorEnable[numPlayers] = {
  P1_MOTOR_ENABLE
};

uint16_t motorDirection[numPlayers] = {
  P1_MOTOR_DIR
};

uint8_t motorPosition[numPlayers] = {
  0
};


// skee ball sensor value arrays
uint16_t currentVal[numSensors];
uint16_t previousVal[numSensors];

// de-bounced button objects
Bounce resetButton = Bounce(RESET_PIN, 10);
Bounce p1LimitLeft = Bounce(P1_LIMIT_LEFT, 10);
Bounce p1LimitRight = Bounce(P1_LIMIT_RIGHT, 10);

Bounce leftLimits[numPlayers] = {
  p1LimitLeft
};

Bounce rightLimits[numPlayers] = {
  p1LimitRight
};


void initGame() {   // runs once on setup
  pinMode(RESET_PIN, INPUT_PULLUP);

  for (int i = 0; i < numSensors; i++) {
    pinMode(playerInputs[i], INPUT);

    if (i < numPlayers) {
      pinMode(limitSwitchesLeft[i], INPUT_PULLUP);
      pinMode(limitSwitchesRight[i], INPUT_PULLUP);
      pinMode(motorEnable[i], OUTPUT);
      pinMode(motorDirection[i], OUTPUT);
    }

  }

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
  float playerMultiplers[numPlayers] = {0};
  for (int i = 0; i < numSensors; i++) {
    currentVal[i] = 0;
    previousVal[i] = 0;
  }

  // rest player positions to home
  resetPlayers();

  Serial.println("---------------------------");
  Serial.println("Game reset");
  Serial.println("---------------------------");
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  initGame();
}

void loop() {
  if(resetCheck()) resetGame();

  getSensorVals();
  getWinner();
  getHome();
}

void getSensorVals() {    // read input sensors on skee ball lanes
  for (uint8_t i = 0; i < numSensors; i++) {
    previousVal[i] = currentVal[i];
    currentVal[i] = digitalRead(playerInputs[i]);

    if (!currentVal[i] && previousVal[i] ) { // sensor has been triggered

      uint8_t playerNum = floor(i / 6) + 1;  // first 6 vals are p1, next 6 are p2, ... p6
      float multiplier =  multiplierVals[i % 6];

      Serial.print("Player ");
      Serial.print(playerNum);
      Serial.print(" sensor #");
      Serial.print(playerInputs[i]);
      Serial.print(" was triggered.");
      Serial.println();

      movePlayer(playerNum, multiplier);
    }
    delay(1);
  }
}

void movePlayer(uint8_t playerInput, float multiplier) {  // calculate time period to move motor, move it
  unsigned long period = multiplier * 1000;
  uint8_t playerNum = playerInput - 1;  // array starts at 0

  static unsigned char motorRunning = false;
  if (timer(period)) { // time is up
    motorPosition[playerNum] += period;
    stopPlayer(playerNum);
    resetTimer = true;
  }
  if (!motorRunning) {  // first time
    analogWrite(motorDirection[playerNum], HIGH); // move player forward
    analogWrite(motorEnable[playerNum], motorSpeed);
  }
}

void stopPlayer(int playerNum) {
  analogWrite(motorEnable[playerNum], 0);
}

void resetPlayers() {
  // move backwards
  for (int i=0; i < numPlayers; i++) {
    analogWrite(motorDirection[i], HIGH);
    analogWrite(motorEnable[i], motorSpeed);
  }

  byte allHome = false;

  // wait until all motor positions are home (0)
  while(!allHome) {
    uint16_t total = 0;

    for(int i=0; i < numPlayers; i++){
      total += motorPosition[i];  
    }

    if(total == 0) allHome = true;
  }   
}


unsigned char timer(unsigned long duration) {
  if (resetTimer) {

  }

  static unsigned long timeInit = millis(); // initial time
  return ((millis() - timeInit) >= duration) ? 1 : 0;
}


void getWinner() {  // check if right side limit switch is triggered
  for (int i = 0; i < numPlayers; i++) {
    if (rightLimits[i].update()) {
      if (rightLimits[i].fallingEdge()) {
        analogWrite(motorEnable[i], 0);
        
        Serial.println();
        Serial.println("Winner! Player ");
        Serial.print(i);
        Serial.println();

        // call audio file

        //delay?
        //delay(1000);

        Serial.println("---------------------------");
        Serial.println("Game End");
        Serial.println("---------------------------");
        Serial.println();

        resetGame();  //reset game
      }
    }
  }
}

void getHome() {  // check if left side limit switch is triggered
  for (int i = 0; i < numPlayers; i++) {
    if (leftLimits[i].update()) {
      if (leftLimits[i].fallingEdge()) {
        analogWrite(motorEnable[i], 0);   // stop the motor
        motorPosition[i] = 0; // reset postion val
      }
    }
  }
}

byte resetCheck() {  // check if master reset button was pressed
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
