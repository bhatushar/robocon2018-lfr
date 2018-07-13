#ifndef LINEDETECTOR_H
#define LINEDETECTOR_H


#include <Arduino.h>
#include <Servo.h>


#define MAX_SENSOR 8  // Total number of sensors in IR array


struct IRSensor {
  int pin,      // The pin to which the sensor is connected
      weight;   // The weight assigned to each pin
};


class LineDetector {

private:
  IRSensor sensor[MAX_SENSOR];
  Servo servo;
  int sensorOnLine;
  bool servoBackOdd;  // Shows if servo is rotated backwards odd number of times

public:
  LineDetector(int[]);   // Constructor
  int calcDeviation();   // Calculates the deviation by which the bot is off the line
  bool isTurn();         // Checks if the bot is on a turn
  bool isCrossSection(); // Checks if the bot is on a cross-section
  void rotate(char);     // Rotates the IR array
  void initServo(int servoPin) {
    servo.attach(servoPin);
    servo.write(90);
  }
};

/**
 * Constructor
 * Assigns sensor pin and weight
 * @param int[] pins  The pins to which the IR array is connected
 * @param int   servo Pin to which the IR servo is connected
 */
LineDetector::LineDetector(int pins[]) {
  sensorOnLine = 0;
  servoBackOdd = false;

  // Assigning pins to each sensor
  for (int i = 0; i < MAX_SENSOR; i++)
  {
    sensor[i].pin = pins[i];
    pinMode(sensor[i].pin, INPUT);
  }

  // Assigning weight to each sensor
  // -3, -2, -1, 0, 0, 1, 2, 3
  int even = !(MAX_SENSOR % 2);                                 // Checking if total sensors are even or odd
  int value = even ? -(MAX_SENSOR / 2 - 1) : -(MAX_SENSOR / 2); //Minimum weight
  for (int i = 0; i < MAX_SENSOR; i++, value++) {
    if (even && i == MAX_SENSOR / 2)
      // Additional condition for even sensors
      // Two sensors in middle should have weight = 0
      value--;
    sensor[i].weight = value; //Adding weight to each sensor
  }
}

/**
   * Calculates the off-line value for the IR sensor array
   * @return int err  The positive or negative deviation  
   */
int LineDetector::calcDeviation() {
  sensorOnLine = 0; // Reset number of on-line sensors
  int err = 0;
  for (int i = 0; i < MAX_SENSOR; i++) {
    byte readValue = digitalRead(sensor[i].pin);
    if (readValue == HIGH)
      // Current IR sensor is on the white line
      sensorOnLine++;
    else
      // Add weights off sensors which are off line
      err += sensor[i].weight;
  }
  return err;

}

/**
   * Checks if the bot is on a right-angled turn
   * @return bool result  Boolean status
   */
bool LineDetector::isTurn() {
  int contOnLine = 0; // Total continuous sensors which are on line
  for (int i = 0; i < MAX_SENSOR; i++) {
    if(digitalRead(sensor[i].pin) == LOW)
      contOnLine++;
    else
      contOnLine = 0;
  }
  if(contOnLine >= MAX_SENSOR/2 - 1 && contOnLine < MAX_SENSOR)
    // contOnLine >= MAX_SENSOR/2 - 1   Half or more sensors must be on line for it to be a turn
    // contOnLine < MAX_SENSOR          Required to check that it's not a cross section
    return true;
  else
    return false;
}

/**
   * Function checks if the bot is on a cross-section
   * If the number of sensors which are currently above the line is the same the number of total sensors
   * Then bot is at a cross-section
   */
bool LineDetector::isCrossSection() {
  if (sensorOnLine == MAX_SENSOR)
    // All sensors are on-line
    return true;
  else
    return false;
}


/**
 * Rotates the servo to which the IR array is attached.
 * Depending on how many times the rotate funciton is called (even/odd), the command for left or right direction is altered.
 * @param char dir  Direction of rotation
 */
void LineDetector::rotate(char dir) {
  switch (dir) {
    case 'l':
      if(servoBackOdd)
        servo.write(servo.read() - 90);
      else
        servo.write(servo.read() + 90);
      break;
    case 'r':
      if(servoBackOdd)
        servo.write(servo.read() + 90);
      else
        servo.write(servo.read() - 90);
      break;
    case 'b':
      // Instead of rotating the servo, reverse the order of assigned weight
      for (int i = 0; i <= MAX_SENSOR/2; i++) {
        int temp = sensor[i].weight;
        sensor[i].weight = sensor[MAX_SENSOR - i - 1].weight;
        sensor[MAX_SENSOR - i - 1].weight = temp;
      }
      servoBackOdd = !servoBackOdd; // Toggle value
      break;
  }

  delay(500); // Time required to adjust the Servo
}

#undef MAX_SENSOR

#endif
