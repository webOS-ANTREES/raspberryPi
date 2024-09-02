#include <Stepper.h>

// Define Constants
const int STEPS_PER_REV = 200;
const int SPEED_CONTROL = A0;

// Create an instance of the Stepper class
Stepper stepper_NEMA17(STEPS_PER_REV, 1, 2, 3, 4);

void setup() {
  // nothing to do inside the void setup
}

void loop() {
  // read the sensor value:
  int sensorReading = analogRead(SPEED_CONTROL);
  // map it to a range from 0 to 100:
  int motorSpeed = map(sensorReading, 0, 1023, 0, 100);
  
  // set the motor speed:
  if (motorSpeed > 0) {
    stepper_NEMA17.setSpeed(motorSpeed);
    
    // Rotate 60 degrees to the left (counterclockwise)
    stepper_NEMA17.step(33);  // 60도 회전에 필요한 스텝 수: 33 스텝
    delay(1000);  // Wait for 1 second

    // Rotate 60 degrees to the right (clockwise)
    stepper_NEMA17.step(-33);  // 원래 위치로 돌아가기 위해 반대 방향으로 33 스텝
    delay(1000);  // Wait for 1 second
  }
}
