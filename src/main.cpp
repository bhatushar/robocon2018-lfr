/*
    Author: Tushar Bhatt
    Date: December 19, 2017
    Description: Line following system for the Robocon project.
*/

#include <Arduino.h>
#include <LineDetector.h>
#include <MotorDriver.h>
#include <PIDController.h>


#define MAX_TZ3 5   // Maximum throws allowed through TZ3


int lfrPins[] = {40, 41, 42, 43, 44, 45, 46, 47}, // IR array pins
    servoPin = 31,                                // IR servo pin
    motorPins[4][2] = {
        // Motor pins
        {5, 28}, // Front
        {2, 22}, // Right
        {3, 24}, // Back
        {4, 26}  // Left
},
    lagVolt[2][2] = {{0, 0}, {0, 0}}, // Lag in motors as {{Pin, Lag}, {Pin, Lag}}
    throwShuttle = 0,       // Pin to send signal to the main board for throwing the shuttle
    tz = 1,        // Throwing zone to move to
    tz3Throws = 0; // Total throws through TZ3

MotorDriver motor(motorPins, lagVolt);
LineDetector lfr(lfrPins);
PIDController pid(13, 0, 5);


// Function declarations
void moveForward(int = 80); // Moves the bot in forward direction
void moveToTZ();    // Moves the bot to/from throwing zone


/**
 * Function responsible for bot's movement upto Loading zone 1.
 * Start form ARS.
 * Move forward until first right turn.
 * Turn and continue moving forward until a cross-section is detected.
 * On reaching cross-section turn away from throwing zone.
 */
void setup() {
    // Starting from ARS zone
    
    lfr.initServo(servoPin);

    // Move ahead of starting cross-section
    motor.move('f', 100);
    delay(500);

    moveForward(); // Move forward until first turn
    motor.turn('r'); // First turn is right
    lfr.rotate('r'); // Rotate the IR array
    moveForward(); // Continue

    // First loading point reached
    motor.turn('r'); // TZ1 on left; Face away
    lfr.rotate('r');
}

/**
 * Function responsible for movement for remaining of the game.
 * Rotate back to face the throwing zone.
 * Move to the throwing zone, throw shuttle.
 * Move back to loading point.
 * Update throwing zone information.
 * Repeat.
 */
void loop() {
    // Bot at loading cross-section

    // After recieving shuttle
    motor.turn('b'); // Face towards the throwing zone
    lfr.rotate('b'); // Also rotate the IR array

    moveToTZ();      // Bot moves to throwing zone

    // Reached TZ
    // Throw shuttle
    digitalWrite(throwShuttle, HIGH);
    delay(1000); // Wait for signal to be read
    digitalWrite(throwShuttle, LOW);

    // After completing thorw
    // Return to cross-section for loading
    motor.turn('b'); // Face towards loading zone
    lfr.rotate('b');
    moveToTZ();      // Bot goes back to loading cross-section

    // Throwing zone specific conditions
    if (tz == 1)
    {
        // Bot is at loading zone 1
        // And has cleared the first throwing zone
        // Move it to second loading zone
        motor.turn('l'); // Face towards the next loading cross-section
        lfr.rotate('l');
        moveForward();   // Move until loading cross-section is reached
        motor.turn('r'); // Face away from the throwing zone
        lfr.rotate('r');

        tz = 2; // Next throw from TZ2
    }
    else if (tz == 2 && tz3Throws != MAX_TZ3)
        // TZ2 complete
        // TZ3 throws available
        tz = 3;
    else if (tz == 3)
    {
        tz3Throws++;
        if (tz3Throws == MAX_TZ3)
            // No more TZ3 throws available
            tz = 2; // Continue throwing from second TZ
    }
}

/**
 * Function moves the bot in a straight line until a turn of cross-section is detected
 * @param int stdVolt   The standard voltage which is applied to move straight
 */
void moveForward(int stdVolt) {
    int error, volt;

    // Loop until a cross-section or turn is detected
    do {
        error = lfr.calcDeviation(); // Calculate the deviation
        volt = pid.calcVolt(error);  // Calculate the voltage requierd to fix error

        if (error < 0) {
            // Adjust to right
            motor.move('r', volt, true);
        }
        else if (error > 0) {
            // Adjust to left
            motor.move('l', volt, true);
        }
        else {
            // Move straight
            motor.move('f', stdVolt);
        }
    } while (!lfr.isCrossSection());

    motor.stop(); // Stop bot movement
}


/**
 * Throwing zone is detected by a cross-section.
 * Each throwing zone is surrounded by one additional cross-section on each side.
 * This cross-sections must be skipped.
 * Check the arena configuration to know how the number of skips are calculated.
 */
void moveToTZ() {
    int skips;
    if (tz == 1 || tz == 2)
        skips = 2;
    else if (tz == 3)
        skips = 5;

    while (skips) {
        moveForward();
        skips--;
        motor.move('f', 255);
        delay(500);
    }
}
