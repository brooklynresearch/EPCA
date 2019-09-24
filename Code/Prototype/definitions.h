// players
const uint8_t numPlayers = 1;
const uint8_t numSensors = numPlayers * 3;

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

// LED pins
#define LED_PIN 15
#define NUM_LEDS 20

/*  motor pins
 *   
 *  Driving DC motors with L293N, using 2 pins instead of 3
 *    Enable pin: always +5v 
 *    InA: digital, HIGH = forward / LOW = backward  
 *    InB: analog, 0-255 / 255-0 depending on InA is HIGH / LOW
 * 
 */
#define P1_MOTOR_DIR_A 9
#define P1_MOTOR_DIR_B 10
//#define P2_MOTOR_DIR_A 9
//#define P2_MOTOR_DIR_B 10
//#define P3_MOTOR_DIR_A 9
//#define P3_MOTOR_DIR_B 10
//#define P4_MOTOR_DIR_A 9
//#define P4_MOTOR_DIR_B 10
//#define P5_MOTOR_DIR_A 9
//#define P5_MOTOR_DIR_B 10
//#define P6_MOTOR_DIR_A 9
//#define P6_MOTOR_DIR_B 10

/*  player input pins, top to bottom:
 *   
 *      100  100  100
 *         50   50
 *           25
 *           
*/
#define P1_TOP 2
#define P1_MIDDLE 3
#define P1_BOTTOM 4
//#define P2_100 2
//#define P2_50_LEFT 3
//#define P2_50_RIGHT 4
//#define P3_100 2
//#define P3_50_LEFT 3
//#define P3_50_RIGHT 4
//#define P4_100 2
//#define P4_50_LEFT 3
//#define P4_50_RIGHT 4
//#define P5_100 2
//#define P5_50_LEFT 3
//#define P5_50_RIGHT 4
//#define P6_100 2
//#define P6_50_LEFT 3
//#define P6_50_RIGHT 4

// input sensor arrays
uint8_t playerInputs[numSensors] = {
  P1_TOP,
  P1_MIDDLE,
  P1_BOTTOM
  //  P2_100,
  //  P2_50_LEFT,
  //  P2_50_RIGHT,
  //  P3_100,
  //  P3_50_LEFT,
  //  P3_50_RIGHT,
  //  P4_100,
  //  P4_50_LEFT,
  //  P4_50_RIGHT,
  //  P5_100,
  //  P5_50_LEFT,
  //  P5_50_RIGHT,
  //  P6_100,
  //  P6_50_LEFT,
  //  P6_50_RIGHT
};

uint8_t limitSwitchesLeft[numPlayers] = {
  P1_LIMIT_LEFT
  //  P2_LIMIT_LEFT,
  //  P3_LIMIT_LEFT,
  //  P4_LIMIT_LEFT,
  //  P5_LIMIT_LEFT,
  //  P6_LIMIT_LEFT
};

uint8_t limitSwitchesRight[numPlayers] = {
  P1_LIMIT_RIGHT
  //  P2_LIMIT_RIGHT,
  //  P3_LIMIT_RIGHT,
  //  P4_LIMIT_RIGHT,
  //  P5_LIMIT_RIGHT,
  //  P6_LIMIT_RIGHT
};


// motor arrays
byte motorDirectionA[numPlayers] = {
  P1_MOTOR_DIR_A
  //  P2_MOTOR_DIR_A,
  //  P3_MOTOR_DIR_A,
  //  P4_MOTOR_DIR_A,
  //  P5_MOTOR_DIR_A,
  //  P6_MOTOR_DIR_A
};

byte motorDirectionB[numPlayers] = {
  P1_MOTOR_DIR_B
  //  P2_MOTOR_DIR_B,
  //  P3_MOTOR_DIR_B,
  //  P4_MOTOR_DIR_B,
  //  P5_MOTOR_DIR_B,
  //  P6_MOTOR_DIR_B
};

uint8_t motorPosition[numPlayers] = {
  0
  //0, 0, 0, 0, 0
};

bool motorMoving[numPlayers] = {
  false
  // false, false, false, false, false
};

unsigned long startMillis[numPlayers] = {
  0
  //0, 0, 0, 0, 0
};

unsigned long playerMovementPeriod[numPlayers] = {
  0
  //0, 0, 0, 0, 0
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
