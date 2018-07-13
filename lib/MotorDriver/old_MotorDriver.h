#ifndef MOTORDRIVER_H
#define MOTORDRIVER_H

/*
    Library to control motor movement of the bot.
    The bot is square shaped, with one motor on each side.
    Omni/Mecanum wheels are attached to each motor.
    
    Initial motor orientation:
                    --------------------------------------
                    |          Motor 1 (Right)           |
                    |         -------------------        |
                    |         |       |         |        |
    Line            |         |       | Throwing|        |
    ================| Motor 0 |       | Arm     | Motor 2|   Starting zone
                    | (Front) |       |         | (Back) |
                    |         |       V         |        |
                    |         -------------------        |
                    |          Motor 3 (Left)            |
                    --------------------------------------
*/

#include <Arduino.h>

// Defining macros
#define MAX_MOTORS 4
#define posPin 0
#define negPin 1

class MotorDriver {

private:
    int motors[MAX_MOTORS][2],          // 4 motors; each motor containing 2 terminals
        orgPins[MAX_MOTORS][2],         // Copy of initial motors[][2] array; used in MotorDriver::setTerminal()
        lagVolt[2],                     // (optional) lag in motor movement; there are 2 variables since the bot moves in two directions
        front, right, back, left;       // Variables to store motor orientation

    void setTerminal();

public:
    MotorDriver(int motorPins[MAX_MOTORS][2], int lag[2]) {
        //Contructor
        for(int i = 0; i < MAX_MOTORS; i++)
            for(int j = 0; j < 2; j++) {
                orgPins[i][j] = motors[i][j] = motorPins[i][j]; // Assigning pins to the `motor` variable
                pinMode(motors[i][j], OUTPUT);
            }
       
        for(int i = 0; i < 2; i++)
            lagVolt[i] = lag[i]; // Assigning lag
        
        // Initializing directions
        front = 0;
        right = 1;
        back = 2;
        left = 3;
    }

    // Function declarations
    void move(char, int, bool adjust = false); // Moves the motor in desired directions at given voltage; additional boolean character for adjustment
    void stop();                // Stops all the motors
    void stop(int[]);      // Stops specified motors
    void turn(char);   // Changes bot's orientation; accepts the direction which is to be treated as front
};

// Function definitions

/**
 * Sets the motor-terminal orientation
 */
void MotorDriver::setTerminal() {
    if (front == 0) {
        // Reset
        for (int i = 0; i < MAX_MOTORS; i++)
            for (int j = 0; j < 2; j++)
                motors[i][j] = orgPins[i][j];
    }
    else if (front == 1) {
        // Forward and backward movement is reversed
        // Switch positive and negative terminals of left and right pins
        for (int i = 0; i < MAX_MOTORS; i++)
            for (int j = 0; j < 2; j++)
                if (i == front || i == back)
                    motors[i][j] = orgPins[i][j];
                else
                    motors[i][j] = orgPins[i][!j];
    }
    else if (front == 2) {
        // Forward and backward movement is reversed
        // Left and right movement is reversed
        // Switch all positive and negative terminals
        for (int i = 0; i < MAX_MOTORS; i++)
            for (int j = 0; j < 2; j++)
                motors[i][j] = orgPins[i][!j];
    }
    else if (front == 3) {
        // Left and right movement is reversed
        // Switch positive and negative terminals of front and back pins
        for (int i = 0; i < MAX_MOTORS; i++)
            for (int j = 0; j < 2; j++)
                if (i == left || i == right)
                    motors[i][j] = orgPins[i][j];
                else
                    motors[i][j] = orgPins[i][!j];
    }
}

/**
 * Function to rotate motors such that the bot moves in the given direction
 * @param char dir      The direction in which the bot is supposed to move - 'f', 'b', 'l', 'r'
 * @param int volt      The voltage to be applied (lag is added to FRONT and LEFT motor)
 * @param bool adjust   If set true, the bot will slide in the given direction without stopping the original movement. Only works if direction is either 'l' or 'r'
 */
void MotorDriver::move(char dir, int volt, bool adjust) {
    //int stoppingMotors[2];
    switch(dir) {
        case 'f':
            // Move forward
            
            //stoppingMotors[0] = front; stoppingMotors[1] = back;
            this->stop(); // Stopping adjecent motors
            
            // Move LEFT and RIGHT motor forward
            analogWrite(motors[left][posPin], volt + lagVolt[0]);
            analogWrite(motors[left][negPin], 0);
            //analogWrite(motors[right][posPin], volt + lagVolt[0]);
            //analogWrite(motors[right][negPin], 0);
            Serial.println("Move forward");

            Serial.print("Left Pin(+ve): ");
            Serial.print(motors[left][posPin]);
            Serial.print(" Volt: ");
            Serial.println(volt + lagVolt[1]);
            Serial.print("Left Pin(-ve): ");
            Serial.print(motors[left][negPin]);
            Serial.print(" Volt: ");
            Serial.println(0);

            Serial.print("Right Pin(+ve): ");
            Serial.print(motors[right][posPin]);
            Serial.print(" Volt: ");
            Serial.println(volt);
            Serial.print("Right Pin(-ve): ");
            Serial.print(motors[right][negPin]);
            Serial.print(" Volt: ");
            Serial.println(0);
            Serial.println();

            break;
        
        case 'b':
            // Move back
            
            //stoppingMotors[0] = front; stoppingMotors[1] = back;
            this->stop(); // Stopping adjecent motors 
            
            // Move LEFT and RIGHT motor backward
            analogWrite(motors[left][posPin], 0);
            analogWrite(motors[left][negPin], volt + lagVolt[0]);
            //analogWrite(motors[right][posPin], 0);
            //analogWrite(motors[right][negPin], volt);
            Serial.println("Move backward");

            Serial.print("Left Pin(+ve): ");
            Serial.print(motors[left][posPin]);
            Serial.print(" Volt: ");
            Serial.println(0);
            Serial.print("Left Pin(-ve): ");
            Serial.print(motors[left][negPin]);
            Serial.print(" Volt: ");
            Serial.println(volt + lagVolt[1]);

            Serial.print("Right Pin(+ve): ");
            Serial.print(motors[right][posPin]);
            Serial.print(" Volt: ");
            Serial.println(0);
            Serial.print("Right Pin(-ve): ");
            Serial.print(motors[right][negPin]);
            Serial.print(" Volt: ");
            Serial.println(volt);
            Serial.println();

            break;
        case 'l':
            // Move left

            if(!adjust)
                this->stop(); // Stopping adjecent motors
            else
                Serial.println("Adjusting\n");
            // Move FRONT and BACK motor to left
            analogWrite(motors[front][posPin], volt + lagVolt[1]);
            analogWrite(motors[front][negPin], 0);
            // analogWrite(motors[back][posPin], volt);
            // analogWrite(motors[back][negPin], 0);
            Serial.println("Move left");

            Serial.print("Front Pin(+ve): ");
            Serial.print(motors[front][posPin]);
            Serial.print(" Volt: ");
            Serial.println(volt + lagVolt[1]);
            Serial.print("Front Pin(-ve): ");
            Serial.print(motors[front][negPin]);
            Serial.print(" Volt: ");
            Serial.println(0);

            Serial.print("Back Pin(+ve): ");
            Serial.print(motors[back][posPin]);
            Serial.print(" Volt: ");
            Serial.println(volt);
            Serial.print("Back Pin(-ve): ");
            Serial.print(motors[back][negPin]);
            Serial.print(" Volt: ");
            Serial.println(0);
            Serial.println();

            break;
        case 'r':
            // Move right
            
            if(!adjust)
                this->stop(); // Stopping adjecent motors
            else
                Serial.println("Adjusting\n");

            // Move FRONT and BACK motor to right
            analogWrite(motors[front][posPin], 0);
            analogWrite(motors[front][negPin], volt + lagVolt[1]);
            // analogWrite(motors[back][posPin], 0);
            // analogWrite(motors[back][negPin], volt);
            Serial.println("Move right");

            Serial.print("Front Pin(+ve): ");
            Serial.print(motors[front][posPin]);
            Serial.print(" Volt: ");
            Serial.println(0);
            Serial.print("Front Pin(-ve): ");
            Serial.print(motors[front][negPin]);
            Serial.print(" Volt: ");
            Serial.println(volt + lagVolt[1]);

            Serial.print("Back Pin(+ve): ");
            Serial.print(motors[back][posPin]);
            Serial.print(" Volt: ");
            Serial.println(0);
            Serial.print("Back Pin(-ve): ");
            Serial.print(motors[back][negPin]);
            Serial.print(" Volt: ");
            Serial.println(volt);
            Serial.println();

            break;
    }
}

/**
 * Function to stop ALL the motors
 */
void MotorDriver::stop() {
    Serial.println("Stop bot movement");
    for(int i = 0; i < MAX_MOTORS; i++)
        for(int j = 0; j < 2; j++)
            digitalWrite(motors[i][j], LOW);
}

/**
 * Function to stop specified motors
 * @param int[] x   Indices of the motors to be stopped
 */
void MotorDriver::stop(int x[]) {
    int len_x = sizeof(x)/sizeof(*x); // Length of array
    for (int i = 0; i < len_x; i++) {
        int index = x[i]; // Index of motor
        for(int j = 0; j < 2; j++)
            digitalWrite(motors[index][j], LOW);
    }
}

/**
 * Swaps the orientation of the motors
 * @param char dir  Direction of turning
 */
void MotorDriver::turn(char dir) {
    int temp;
    switch(dir) {
        case 'f': // Reset
            front = 0;// Motor0 will face forward
            right = 1;// Motor1 will face right
            back = 2; // Motor2 will face back
            left = 3; // Motor3 will face left
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
    
    setTerminal();

    Serial.print("Turn: ");
    Serial.println(dir);

    Serial.print("Front: ");
    Serial.print(front);
    Serial.print(" +ve: ");
    Serial.print(motors[front][posPin]);
    Serial.print(" -ve: ");
    Serial.println(motors[front][negPin]);

    Serial.print("Right: ");
    Serial.print(right);
    Serial.print(" +ve: ");
    Serial.print(motors[right][posPin]);
    Serial.print(" -ve: ");
    Serial.println(motors[right][negPin]);
    
    Serial.print("Back: ");
    Serial.print(back);
    Serial.print(" +ve: ");
    Serial.print(motors[back][posPin]);
    Serial.print(" -ve: ");
    Serial.println(motors[back][negPin]);
    
    Serial.print("Left: ");
    Serial.print(left);
    Serial.print(" +ve: ");
    Serial.print(motors[left][posPin]);
    Serial.print(" -ve: ");
    Serial.println(motors[left][negPin]);
    
    Serial.println("\n");
}

// Undefining macros
#undef MAX_MOTORS
#undef posPin
#undef negPin

#endif
