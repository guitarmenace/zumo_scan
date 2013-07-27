#include <ZumoBuzzer.h>
#include <ZumoMotors.h>
#include <Pushbutton.h>

/*
   Obstacle-avoiding robot code.
 */

#define LED 13
#define SPEED 200.0
#define SPIN_FUDGE 800.0
#define TURN_FUDGE 1500.0
#define WINDOW 45.0
#define STEP 5.0
#define THRESHOLD 300
#define DEVIATION 1.2
#define UNDEVIATION (1.0 / DEVIATION)
#define LOOK_ARRAY_SIZE 10
#define STANDBY 0
#define GO 1

ZumoBuzzer buzzer; 
ZumoMotors motors; 
Pushbutton button(ZUMO_BUTTON); // pushbutton on pin 12

boolean state = STANDBY;
int look_count = LOOK_ARRAY_SIZE / 2;
int look_array[LOOK_ARRAY_SIZE];
float deviation = DEVIATION;

void setup()
{
  Serial.begin(9600); 
  pinMode(LED, OUTPUT);
  
  // uncomment one or both of the following lines if your motors' directions need to be flipped
  //motors.flipLeftMotor(true);
  //motors.flipRightMotor(true);
  
  randomSeed(analogRead(0));
}


void loop() {
  delay(100);
  
  int sensorValue = analogRead(A4);
  // print out the value you read:
  //Serial.println(sensorValue);
    
  if (state == STANDBY) {  
     waitForButtonAndCountDown();
     state = GO;
     delay(500);
  }
  
  // if we're blocked
  if (isBlocked()) {
    // stop
    go(0);
    
    // randomly
    if (random(0, 2) > 0) {
        // spin right until we see a gap
        if (spin_and_look_right(360, WINDOW)) {
            // then correct back half the window 
            spin(-WINDOW/2);
        } else {
          // we're stuck
          alarm();
          halt();
        }
    } else {
        // spin left until we see a gap
        if (spin_and_look_left(360, WINDOW)) {
            // then correct back half the window
            spin(WINDOW/2);
        } else {
          // we're stuck
          alarm();
          halt();
        }
    }
    
  // otherwise, move forward while scanning for best path
  } else {  
    curve_and_look(SPEED);
  }
  
  if (button.isPressed()) {
    halt();
  }
}

void alarm() {
    for (int i = 0; i < 8; i++)
    {
        buzzer.playNote(NOTE_G(5), 200, 15);
        delay(200);
    }
}

void halt() {
    go(0);
    state = STANDBY;
    delay(500);
}


boolean isBlocked() {
  return analogRead(A4) > THRESHOLD;
}


// spin until we see a wide enough window to go through
boolean spin_and_look_right(int degrees, int window) {
  int width = 0;
  
  for (int i=0; i < degrees; i += STEP) {
    spin(STEP);
    
    if (!isBlocked()) {
      width += STEP;
    } else {
      width = 0;
    }
    
    if (width >= window) return true;
  }
  
  return false;
}


// spin until we see a wide enough window to go through
boolean spin_and_look_left(int degrees, int window) {
  int width = 0;
  
  for (int i=0; i < degrees; i += STEP) {
    spin(-STEP);
    
    if (!isBlocked()) {
      width += STEP;
    } else {
      width = 0;
    }
    
    if (width >= window) return true;
  }
  
  return false;
}

void curve_and_look(int speed) {
  if (++look_count > LOOK_ARRAY_SIZE) {
    deviation = deviation > 1 ? UNDEVIATION : DEVIATION;
    look_count = 0;
  }
  
  // center robot on best look value out of the last 10
  int look_value = analogRead(A4);
  look_array[look_count] = look_value;
  if (look_value == smallest(look_array, LOOK_ARRAY_SIZE)) {
    look_count = 5;
  }
  
  motors.setLeftSpeed(speed * deviation);
  motors.setRightSpeed(speed / deviation);
}


void go(int speed) {
  motors.setLeftSpeed(speed);
  motors.setRightSpeed(speed);
}


void spin(int degrees) {
  if (degrees < 0) {
    motors.setLeftSpeed(-SPEED);
    motors.setRightSpeed(SPEED);
    delay(-degrees * SPIN_FUDGE / SPEED);
    motors.setLeftSpeed(0);
    motors.setRightSpeed(0);
  } else {
    motors.setLeftSpeed(SPEED);
    motors.setRightSpeed(-SPEED);
    delay(degrees * SPIN_FUDGE / SPEED);
    motors.setLeftSpeed(0);
    motors.setRightSpeed(0);
  }
}


void turn(int degrees) {
  if (degrees < 0) {
    motors.setLeftSpeed(-SPEED);
    delay(-degrees * TURN_FUDGE / SPEED);
    motors.setLeftSpeed(0);
  } else {
    motors.setRightSpeed(-SPEED);
    delay(degrees * TURN_FUDGE / SPEED);
    motors.setRightSpeed(0);
  }
}


void waitForButtonAndCountDown()
{
    digitalWrite(LED, HIGH);
    button.waitForButton();
    digitalWrite(LED, LOW);
    
    for (int i = 0; i < 3; i++)
    {
        buzzer.playNote(NOTE_G(3), 200, 15);
        delay(1000);
    }
    
    buzzer.playNote(NOTE_G(4), 500, 15);
    delay(1000);
}


int smallest(int array[], int array_size) {
   int lowest = -1;
   
   for (int i = 1; i < array_size; ++i) {
       if (array[i] < lowest || lowest < 0) {
         lowest = array[i];
       }
   }
   return lowest;
}
