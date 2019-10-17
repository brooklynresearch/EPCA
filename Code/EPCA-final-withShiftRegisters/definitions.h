// players
const uint8_t numPlayers = 6;
const uint8_t numSensors = numPlayers * 3;

// switch pins
#define  RESET_PIN  23

#define  P1_LIMIT_LEFT  0
#define  P1_LIMIT_RIGHT  1
#define  P2_LIMIT_LEFT  2
#define  P2_LIMIT_RIGHT  3
#define  P3_LIMIT_LEFT  4
#define  P3_LIMIT_RIGHT  5
#define  P4_LIMIT_LEFT  6
#define  P4_LIMIT_RIGHT  7
#define  P5_LIMIT_LEFT  8
#define  P5_LIMIT_RIGHT  9
#define  P6_LIMIT_LEFT  10
#define  P6_LIMIT_RIGHT  11

// LED pins
#define  LED_PIN  12
#define  NUM_LEDS  14

/*  motor pins

    Driving DC motors with L293N, using 2 pins instead of 3
      Enable pin: always +5v
      InA: digital, HIGH = forward / LOW = backward
      InB: analog, 0-255 / 255-0 depending on InA is HIGH / LOW

*/
#define P1_MOTOR_DIR_A 24
#define P1_MOTOR_DIR_B 25
#define P2_MOTOR_DIR_A 26
#define P2_MOTOR_DIR_B 27
#define P3_MOTOR_DIR_A 28
#define P3_MOTOR_DIR_B 29
#define P4_MOTOR_DIR_A 30
#define P4_MOTOR_DIR_B 31
#define P5_MOTOR_DIR_A 32
#define P5_MOTOR_DIR_B 35
#define P6_MOTOR_DIR_A 34
#define P6_MOTOR_DIR_B 33

/*  player input pins, top to bottom:

        100  100  100
           50   50
             25

*/

#define NUMBER_OF_SHIFT_CHIPS   3
#define DATA_WIDTH   NUMBER_OF_SHIFT_CHIPS * 8

#define LOAD_PIN    37
#define ENABLE_PIN  38
#define DATA_PIN    39
#define CLOCK_PIN   36

unsigned long SENSOR_VALUES;
unsigned long OLD_SENSOR_VALUES;

bool motorTriggerValue[24];
uint8_t motorLookUp[24] = {4,4,4,4,5,5,5,5,2,2,2,2,3,3,3,3,0,0,0,0,1,1,1,1};
uint8_t multiplierLookUp[24] = {0,2,3,1,0,2,3,1,0,2,3,1,0,2,3,1,0,2,3,1,0,2,3,1};

uint8_t limitSwitchesLeft[numPlayers] = {
  P1_LIMIT_LEFT,
  P2_LIMIT_LEFT,
  P3_LIMIT_LEFT,
  P4_LIMIT_LEFT,
  P5_LIMIT_LEFT,
  P6_LIMIT_LEFT
};

uint8_t limitSwitchesRight[numPlayers] = {
  P1_LIMIT_RIGHT,
  P2_LIMIT_RIGHT,
  P3_LIMIT_RIGHT,
  P4_LIMIT_RIGHT,
  P5_LIMIT_RIGHT,
  P6_LIMIT_RIGHT
};


// motor arrays
byte motorDirectionA[numPlayers] = {
  P1_MOTOR_DIR_A,
  P2_MOTOR_DIR_A,
  P3_MOTOR_DIR_A,
  P4_MOTOR_DIR_A,
  P5_MOTOR_DIR_A,
  P6_MOTOR_DIR_A
};

byte motorDirectionB[numPlayers] = {
  P1_MOTOR_DIR_B,
  P2_MOTOR_DIR_B,
  P3_MOTOR_DIR_B,
  P4_MOTOR_DIR_B,
  P5_MOTOR_DIR_B,
  P6_MOTOR_DIR_B
};

float motorPosition[numPlayers] = {
  0, 0, 0, 0, 0, 0
};

bool motorMoving[numPlayers] = {
  false, false, false, false, false, false
};

unsigned long startMillis[numPlayers] = {
  0, 0, 0, 0, 0, 0
};

float playerMovementPeriod[numPlayers] = {
  0, 0, 0, 0, 0, 0
};

// skee ball sensor value arrays
uint8_t currentVal[numSensors];
uint8_t previousVal[numSensors];

// de-bounced button objects

Bounce resetButton = Bounce(RESET_PIN, 50);

Bounce p1LimitLeft = Bounce(P1_LIMIT_LEFT, 50);
Bounce p2LimitLeft = Bounce(P2_LIMIT_LEFT, 50);
Bounce p3LimitLeft = Bounce(P3_LIMIT_LEFT, 50);
Bounce p4LimitLeft = Bounce(P4_LIMIT_LEFT, 50);
Bounce p5LimitLeft = Bounce(P5_LIMIT_LEFT, 50);
Bounce p6LimitLeft = Bounce(P6_LIMIT_LEFT, 50);

Bounce p1LimitRight = Bounce(P1_LIMIT_RIGHT, 50);
Bounce p2LimitRight = Bounce(P2_LIMIT_RIGHT, 50);
Bounce p3LimitRight = Bounce(P3_LIMIT_RIGHT, 50);
Bounce p4LimitRight = Bounce(P4_LIMIT_RIGHT, 50);
Bounce p5LimitRight = Bounce(P5_LIMIT_RIGHT, 50);
Bounce p6LimitRight = Bounce(P6_LIMIT_RIGHT, 50);

// arays for buttons
Bounce leftLimits[numPlayers] = {
  p1LimitLeft,
  p2LimitLeft,
  p3LimitLeft,
  p4LimitLeft,
  p5LimitLeft,
  p6LimitLeft
};

Bounce rightLimits[numPlayers] = {
  p1LimitRight,
  p2LimitRight,
  p3LimitRight,
  p4LimitRight,
  p5LimitRight,
  p6LimitRight
};
