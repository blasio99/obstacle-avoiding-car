/*
---------------------------------------------------
           Benedek Balazs - Group 30434
                    23/12/2020
---------------------------------------------------

       Technical University of Cluj-Napoca         
    Faculty of Automation and Computer Science     
          Design with Microprocessors              

     _     _            _        ____   ____
    | |__ | | ___  ___ |_| ___  /    \ /    \ 
    |    \| |/__ \| __|| |/   \ \__'  |\__'  |
    |  .  | |  .  |__ || |  .  | __|  | __|  |
    \____/|_|\___/|___||_|\___/ |____/ |____/

    ------------------------------------------
                github.com/blasio99


---------------------------------------------------
*/

#include <AFMotor.h>  
#include <NewPing.h>
#include <Servo.h> 

#define TRIG_PIN  A0 // Pin A0 on the Motor Drive Shield connected to the ultrasonic sensor
#define ECHO_PIN  A1 // Pin A1 on the Motor Drive Shield connected to the ultrasonic sensor
#define BACK_LED  A3 // Pin A3 on the Motor Drive Shield connected to the red backlight led
#define RIGHT_LED A4 // Pin A3 on the Motor Drive Shield connected to the yellow right light led
#define LEFT_LED  A5 // Pin A3 on the Motor Drive Shield connected to the yellow left  light led
#define LEFT  1   // for index led
#define RIGHT 2   // for index led
#define MAX_DISTANCE 300    // sets maximum useable sensor measuring distance to 300cm
                            // maximum sensor distance is rated at 400-500cm
#define MAX_SPEED 170       // sets speed of DC  motors
#define MAX_SPEED_OFFSET 20 // this sets offset to allow for differences between the two DC traction motors

// it sets up sensor library to use the correct pins to measure distance.
NewPing sonar(TRIG_PIN, 
              ECHO_PIN, 
              MAX_DISTANCE); 

AF_DCMotor motor1(3, MOTOR34_1KHZ); // create motor #1, using M3 output, set to 1kHz PWM frequency
AF_DCMotor motor2(4, MOTOR34_1KHZ); // create motor #2, using M4 output, set to 1kHz PWM frequency

Servo myServo; // it creates myServo object to control the servo motor

boolean goesForward = false; // if the car goes forward or backward
int distance = 100;          // current distance, with initial value of 100



// --------------------------------------------------- setup ------------------------------------------------------
void setup() {
  pinMode( LEFT_LED, OUTPUT);
  pinMode(RIGHT_LED, OUTPUT);
  pinMode( BACK_LED, OUTPUT);
  
  myServo.attach(10); // attaches the servo on pin 10
  myServo.write(100); // ultrasonic sensor facing forward
  
  digitalWrite(BACK_LED, HIGH); // brake (stop) light
  delay(1500);
  digitalWrite(BACK_LED, LOW);

  // reading the distance more times
  for(int i = 0; i < 4; ++i){
    distance = readPing();
    delay(100);
  }
}

// ------------------------------------------------ main loop ----------------------------------------------------
void loop() {
  delay(30);

  // if there is an obstacle in 20 cm, we must change the path
  if (distance <= 20) 
       changePath();
  else moveForward(); // else it moves forward
  
  distance = readPing();
}

// ----------------------------------------------- change path ----------------------------------------------------
// What does it mean? if there is an obstacle in 2ö cm-s the robot car does the following steps:
//       - It stops (the brake lamp is ON)
//       - It moves backward a bit
//       - It stops
//       - It looks right saving the distance data 
//       - Then, it looks left, saving the distance data here too
//       - It turns to the direction where the distance is much more to an obstacle
//       - Finally, it stops, and it is ready to move forward

void changePath(){
  int leftDistance  = 0;
  int rightDistance = 0;
  
  moveStop();
  digitalWrite(BACK_LED, HIGH);
  delay(100);
  digitalWrite(BACK_LED, LOW);
  moveBackward();
  digitalWrite(BACK_LED, HIGH);
  delay(700);
  moveStop();
  delay(100);
  rightDistance = lookRight();
  delay(100);
  leftDistance  = lookLeft();
  delay(100);
  digitalWrite(BACK_LED, LOW);
  
       if(rightDistance > leftDistance) turnRight();
  else if(rightDistance < leftDistance) turnLeft ();
  else turnMore();
    
  
  digitalWrite(BACK_LED, HIGH);
  moveStop();
  delay(20);
  digitalWrite(BACK_LED, LOW);
}

// -------------------------------------- look left (servo motor turns) -----------------------------------------

int lookLeft()
{
    myServo.write(155); // it turns to the angle 155
    delay(300);
    int distance = readPing(); // it detects the distance
    delay(75);
    myServo.write(100); //then back to the center, to angle 100
    return distance;
    
}

// ------------------------------------ look right (servo motor turns) ------------------------------------------
int lookRight()
{
    myServo.write(25); // it turns to the angle 25
    delay(300);
    int distance = readPing(); // it detects the distance
    delay(75);
    myServo.write(100); //then back to the center, to angle 100
    return distance;
}

// --------------------------------------------- read ping -------------------------------------------------------
int readPing() { 
  delay(70);
  return sonar.ping_cm() ? sonar.ping_cm() : 250; // if it is 0, it becames 250
}

// ------------------------------------------------ stop ---------------------------------------------------------
void moveStop() {
  motor1.run(RELEASE); 
  motor2.run(RELEASE);
} 

// --------------------------------------------- goes forward ----------------------------------------------------
void moveForward() {
  
  if(!goesForward)
  {
    goesForward = true;
    motor1.run(FORWARD);      
    motor2.run(FORWARD); 

    // just easily with the gas pedal, the batteries should tackle with it
    for (int speedSet = 0; speedSet < MAX_SPEED; speedSet +=2)
    {
      motor1.setSpeed(speedSet);
      motor2.setSpeed(speedSet);
      delay(5);
    }
  }
}

// ---------------------------------------------- goes backward --------------------------------------------------
void moveBackward() {
  goesForward = false;
  motor1.run(BACKWARD);      
  motor2.run(BACKWARD); 
  
  // just easily with the gas pedal, the batteries should tackle with it
  for (int speedSet = 0; speedSet < MAX_SPEED; speedSet +=2) 
  {
    motor1.setSpeed(speedSet);
    motor2.setSpeed(speedSet);
    delay(5);
  }
}  

// ---------------------------------------------- led as index ---------------------------------------------------
// this is a function for handling the indexes, such as on cars
void ledIndex (int direction, int timer)
{
  //in all delay(timer * 100)
  for(int led = 0; led < timer; ++led)
  {
    delay(50);
    digitalWrite(direction == LEFT ? LEFT_LED : RIGHT_LED, HIGH);
    
    delay(50);
    digitalWrite(direction == LEFT ? LEFT_LED : RIGHT_LED, LOW);     
  }
}

// ------------------------------------------------ turn right ---------------------------------------------------
void turnRight() {
  motor1.run(FORWARD);
  motor2.run(BACKWARD);
  
  ledIndex(RIGHT, 7); // turn right in 0.7 sec
  
  motor1.run(FORWARD);      
  motor2.run(FORWARD);  
} 

// ------------------------------------------------ turn left -----------------------------------------------------
void turnLeft() {
  motor1.run(BACKWARD);
  motor2.run(FORWARD);   
  
  ledIndex(LEFT, 7); // turn left in 0.7 sec
    
  motor1.run(FORWARD);     
  motor2.run(FORWARD);
}  

// --------------------------------------------- turn right more --------------------------------------------------
void turnMore() { // in case the the obstacles from the right and the left are at the same distance
  motor1.run(FORWARD);
  motor2.run(BACKWARD);
  
  ledIndex(RIGHT, 10); // turn right in 1 sec
  
  motor1.run(FORWARD);      
  motor2.run(FORWARD);  
} 

// --------------------------------------------- THAT'S ALL FOLKS -------------------------------------------------


// ----------------------------------------------------------------------------------------------------------------
