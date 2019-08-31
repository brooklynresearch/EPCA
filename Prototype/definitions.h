// players
#define numPlayers 1
#define numSensors (numPlayers * 6)

// switch pins
#define RESET_PIN 11

#define P1_LIMIT_LEFT 12
#define P1_LIMIT_RIGHT 14
//#define P2_LIMIT_LEFT 12
//#define P2_LIMIT_RIGHT 13
//#define P3_LIMIT_LEFT 12
//#define P3_LIMIT_RIGHT 13
//#define P4_LIMIT_LEFT 12
//#define P4_LIMIT_RIGHT 13
//#define P5_LIMIT_LEFT 12
//#define P5_LIMIT_RIGHT 13
//#define P6_LIMIT_LEFT 12
//#define P6_LIMIT_RIGHT 13

// motor control pins
#define P1_MOTOR_ENABLE 8
#define P1_MOTOR_DIR_A 9
#define P1_MOTOR_DIR_B 10
//#define P2_MOTOR_ENABLE 8
//#define P2_MOTOR_DIR_A 9
//#define P2_MOTOR_DIR_B 10
//#define P3_MOTOR_ENABLE 8
//#define P3_MOTOR_DIR_A 9
//#define P3_MOTOR_DIR_B 10
//#define P4_MOTOR_ENABLE 8
//#define P4_MOTOR_DIR_A 9
//#define P4_MOTOR_DIR_B 10
//#define P5_MOTOR_ENABLE 8
//#define P5_MOTOR_DIR_A 9
//#define P5_MOTOR_DIR_B 10
//#define P6_MOTOR_ENABLE 8
//#define P6_MOTOR_DIR_A 9
//#define P6_MOTOR_DIR_B 10

/** player input pins: top to bottom, left to right:
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
//#define P2_100 2
//#define P2_50_LEFT 3
//#define P2_50_RIGHT 4
//#define P2_25_LEFT 5
//#define P2_25_MIDDLE 6
//#define P2_25_RIGHT 7
//#define P3_100 2
//#define P3_50_LEFT 3
//#define P3_50_RIGHT 4
//#define P3_25_LEFT 5
//#define P3_25_MIDDLE 6
//#define P3_25_RIGHT 7
//#define P4_100 2
//#define P4_50_LEFT 3
//#define P4_50_RIGHT 4
//#define P4_25_LEFT 5
//#define P4_25_MIDDLE 6
//#define P4_25_RIGHT 7
//#define P5_100 2
//#define P5_50_LEFT 3
//#define P5_50_RIGHT 4
//#define P5_25_LEFT 5
//#define P5_25_MIDDLE 6
//#define P5_25_RIGHT 7
//#define P6_100 2
//#define P6_50_LEFT 3
//#define P6_50_RIGHT 4
//#define P6_25_LEFT 5
//#define P6_25_MIDDLE 6
//#define P6_25_RIGHT 7

// input sensor arrays
uint8_t playerInputs[numSensors] = {
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


// motor arrays
uint8_t motorEnable[numPlayers] = {
  P1_MOTOR_ENABLE
};

byte motorDirectionA[numPlayers] = {
  P1_MOTOR_DIR_A
};

byte motorDirectionB[numPlayers] = {
  P1_MOTOR_DIR_B
};

uint8_t motorPosition[numPlayers] = {
  0
};

bool motorMoving[numPlayers] = {
  false
};

unsigned long startMillis[numPlayers] = {
  0
};

unsigned long playerMovementPeriod[numPlayers] = {
  0
};

// skee ball sensor value arrays
uint8_t currentVal[numSensors];
uint8_t previousVal[numSensors];

// de-bounced button objects
Bounce resetButton = Bounce(RESET_PIN, 10);

Bounce p1LimitLeft = Bounce(P1_LIMIT_LEFT, 10);
//Bounce p2LimitLeft = Bounce(P2_LIMIT_LEFT, 10);
//Bounce p3LimitLeft = Bounce(P3_LIMIT_LEFT, 10);
//Bounce p4LimitLeft = Bounce(P4_LIMIT_LEFT, 10);
//Bounce p5LimitLeft = Bounce(P5_LIMIT_LEFT, 10);
//Bounce p6LimitLeft = Bounce(P6_LIMIT_LEFT, 10);

Bounce p1LimitRight = Bounce(P1_LIMIT_RIGHT, 10);
//Bounce p2LimitRight = Bounce(P1_LIMIT_RIGHT, 10);
//Bounce p3LimitRight = Bounce(P1_LIMIT_RIGHT, 10);
//Bounce p4LimitRight = Bounce(P1_LIMIT_RIGHT, 10);
//Bounce p5LimitRight = Bounce(P1_LIMIT_RIGHT, 10);
//Bounce p6LimitRight = Bounce(P1_LIMIT_RIGHT, 10);

// arays for buttons
Bounce leftLimits[numPlayers] = {
  p1LimitLeft
//  p2LimitLeft,
//  p3LimitLeft,
//  p4LimitLeft,
//  p5LimitLeft,
//  p6LimitLeft
};

Bounce rightLimits[numPlayers] = {
  p1LimitRight
//  p2LimitRight,
//  p3LimitRight,
//  p4LimitRight,
//  p5LimitRight,
//  p6LimitRight
};
