#ifndef MOTORDRIVER_H
#define MOTORDRIVER_H

#include <Arduino.h>

#define MAX_MOTORS 4
#define PWM 0
#define DIR 1


class MotorDriver {
private:
  int motors[MAX_MOTORS][2],        // motor > index > (pwm | dir | brk)
      lagVolt[MAX_MOTORS][2],                   // Lag applied at left(0) and front(1) motors
      front, right, back, left,     // Direction indices
      arr_dir[2],                   // Direction status of the left-right and front-back motors, respectively
      lastMove;                     // Variable stores the dir variable passed to move() function

  void revDir(int);                 // Reverse the direction of motor; Parameter - motor index
  void setDir();                    // Re-initialized arr_dir; Writes the new direction to the motor

public:
    MotorDriver(int [][2], int [][2]); // Constructor; Parameters - motor pins and lag voltage
    void move(char, int, bool = false);     // Moves the bot; Parameters - direction, voltage and adjust value
    void stop();                    // Stop bot's movement
    void stop(int, int);
    void turn(char);                // Turn bot; Parameter - direction
    int applyLag(int);              // Returns lag to be applied to the pin
};

/**
 * Contructor
 * Initializes motors[][3] and lagVolt[] arrays
 * Also sets the initial direction and lastMove
 * @param int[][3] m   Motor pins
 * @param int[]    lag Lag voltage for left and front motors
 */
MotorDriver::MotorDriver(int m[][2], int lag[][2]) {
    // Assiging pins to motors[][3] array
    for (int i = 0; i < MAX_MOTORS; i++)
        for (int j = 0; j < 2; j++) {
            motors[i][j] = m[i][j];
            pinMode(motors[i][j], OUTPUT);

            if (j == 1) {
                // Setting direction
                digitalWrite(motors[i][j], LOW);
            }
        }
    // Setting lag
    for (int i = 0; i < MAX_MOTORS; i++)
        for(int j = 0; j < 2; j++)
            lagVolt[i][j] = lag[i][j];

    // Initializing direction variables
    front = 0;
    right = 1;
    back = 2;
    left = 3;

    // Iinitializing directions[] array
    for (int i = 0; i < 2; i++)
        arr_dir[i] = LOW;
    
    // Initial move
    lastMove = 'f';
}

/**
 * Reverse the direction of motors with the given index
 * @param int index_m Index of motor
 */
void MotorDriver::revDir(int index_m) {
    int direction;  // New direction - Opposite of the current value

    if(index_m == front || index_m == back) {
        arr_dir[0] = direction = !arr_dir[0]; // Updating direction
        digitalWrite(motors[front][DIR], arr_dir[0]);
        digitalWrite(motors[back][DIR], arr_dir[0]);
    } 
    else { // For left or right
        arr_dir[1] = direction = !arr_dir[1]; // Updating direction
        digitalWrite(motors[left][DIR], arr_dir[1]);
        digitalWrite(motors[right][DIR], arr_dir[1]);
    }
}

/**
 * Re-initializes arr_dir[] array based on the value of front index
 * Also, writes these values to the motors
 */
void MotorDriver::setDir() {
    if (front == 0) {                   // Front facing (everything is set to default)
        for(int i = 0; i < 2; i++)
            arr_dir[i] = LOW;           // Reset
    }

    else if (front == 1) {              // Right facing (forward <----> backward)
        arr_dir[0] = LOW;               // For front and back motors
        arr_dir[1] = HIGH;              // For left and right motors
    }

    else if (front == 2) {              // Back facing (forward <----> backward and left <----> right)
        for (int i = 0; i < 2; i++)
            arr_dir[i] = HIGH;          // All reversed
    }

    else {                              // Left facing (left <----> right)
        arr_dir[0] = HIGH;              // For front and back motors
        arr_dir[1] = LOW;              // For left and right motors
    }

    for (int i = 0; i < MAX_MOTORS; i++) {
        if(i == front || i == back)
            digitalWrite(motors[i][DIR], arr_dir[0]);
        else    // For left and right motors
            digitalWrite(motors[i][DIR], arr_dir[1]);
    }
}

/**
 * Moves the bot in required direction
 * @param char dir    The direction in which the bot is supposed to move
 * @param int  volt   The voltage to be written
 * @param bool adjust Adjusting status; If enabled, adjacent motors won't stop
 */
void MotorDriver::move(char dir, int volt, bool adjust) {
    if (lastMove != dir) // Only if the last direction and current direction isn't the same
        switch(lastMove) {
            // Reset the directions
            case 'b':
                revDir(left);
                break;
            case 'r':
                revDir(front);
                break;
        }

    switch(dir) {
        case 'f':
            stop(front, back); // Stop all other motors
            Serial.println(motors[left][PWM]);            
            analogWrite(motors[left][PWM], volt);   // Write signal to left motor
            analogWrite(motors[right][PWM], volt);  // Write signal to right motor

            //digitalWrite(motors[left][BRK], LOW);   // Unlock left motor
            // digitalWrite(motors[right][BRK], LOW);  // Unlock right motor UNCOMMENT
            
            break;
        
        case 'b':
            stop(front, back); // Stop all other motors

            // No need to reverse direction if both are same
            if (lastMove != dir) {
                // Reverse left and right motors
                revDir(left);
            }
            Serial.println(motors[left][PWM]);            
            analogWrite(motors[left][PWM], volt);  // Write signal to left motor
            analogWrite(motors[right][PWM], volt); // Write signal to right motor

            //digitalWrite(motors[left][BRK], LOW);  // Unlock left motor
            // digitalWrite(motors[right][BRK], LOW); // Unlock right motor UNCOMMENT

            break;
        
        case 'l':
            if(!adjust) // Don't adjust
                stop(left, right); // Stop all other motors

            Serial.println(motors[front][PWM]);
            analogWrite(motors[front][PWM], volt);  // Write signal to front motor
            analogWrite(motors[back][PWM], volt); // Write signal to back motor

            //digitalWrite(motors[front][BRK], LOW);  // Unlock front motor
            // digitalWrite(motors[back][BRK], LOW); // Unlock back motor UNCOMMENT

            break;
        
        case 'r':
            if (!adjust) // Don't adjust
                stop(left, right); // Stop all other motors

            // No need to reverse direction if both are same
            if (lastMove != dir) {
                // Reverse left and right motors
                revDir(front);
            }
            analogWrite(motors[front][PWM], volt);  // Write signal to front motor
            analogWrite(motors[back][PWM], volt); // Write signal to back motor

            //digitalWrite(motors[front][BRK], LOW);  // Unlock front motor
            // digitalWrite(motors[back][BRK], LOW); // Unlock back motor UNCOMMENT

            break;
    }

    lastMove = dir;
}

void MotorDriver::turn(char dir) {
    int temp;
    
    switch (dir) {
        case 'f':      // Reset
            front = 0; // Motor0 will face forward
            right = 1; // Motor1 will face right
            back = 2;  // Motor2 will face back
            left = 3;  // Motor3 will face left
            break;
        case 'b':
            // Swap front and back
            temp = front;
            front = back;
            back = temp;
            // Swap left and right
            temp = left;
            left = right;
            right = temp;
            break;
        case 'l':
            temp = front;
            front = left; // Current left will be front
            left = back;  // Current front will be right
            back = right; // Current right will be back
            right = temp; // Current back will be left
            break;
        case 'r':
            temp = front;
            front = right; // Current right will be front
            right = back;  // Current front will be left
            back = left;   // Current left will be back
            left = temp;   // Current back will be right
            break;
    }

    setDir();

    lastMove = 'f'; // Prevent unwanted direction reversal in move()
}

/**
 * Stop all motors
 */
void MotorDriver::stop() {
    for(int i = 0; i < MAX_MOTORS; i++)
        digitalWrite(motors[i][PWM], LOW);
}

/**
 * Function to stop specified motors
 * @param int[] x   Indices of the motors to be stopped
 */
void MotorDriver::stop(int m1, int m2) {
    /*Serial.print("Stoping motors: ");
    Serial.print(motors[m1][PWM]);
    Serial.print(" ");    
    Serial.println(motors[m2][PWM]);*/
    
    digitalWrite(motors[m1][PWM], LOW);
    digitalWrite(motors[m2][PWM], LOW);
}

int MotorDriver::applyLag(int pin) {
    for(int i = 0; i < MAX_MOTORS; i++) {
        if(lagVolt[i][0] == pin)
            return lagVolt[i][1];
    }
    return 0;
}

#undef MAX_MOTORS
#undef PWM
#undef DIR

#endif
