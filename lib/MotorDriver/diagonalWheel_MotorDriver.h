#ifndef MOTORDRIVER_H
#define MOTORDRIVER_H


#include <Arduino.h>


#define MAX_MOTORS 4
#define PWM 0
#define DIR 1
#define f_l 0
#define f_r 1
#define b_r 2
#define b_l 3

class MotorDriver {
private:
  int motors[MAX_MOTORS][2],			// motor > index > (pwm | dir)
	  lagVolt[2][2],					// Lag applied at left(0) and front(1) motors
	  front_l, front_r, back_l, back_r, // Direction indices
	  arr_dir[4],						// Direction status of the front-left, front-right, back-right, and back-left motors respectively
	  lastMove;							// Variable stores the dir variable passed to move() function

  void revDir(int);  // Reverse the direction of motor
  void setDir();	 // Re-initialized arr_dir; Writes the new direction to the motor
  int applyLag(int); // Returns the lag to be applied at given pin

public:
	MotorDriver(int[][2], int[][2]);	// Constructor
	void move(char, int, bool = false); // Moves the bot
	void stop();						// Stop bot's movement
	void turn(char); 					// Re-orients the motor roles
};

/**
 * Reverse the direction of motors with the given index
 * @param int index_m Index of motor
 */
void MotorDriver::revDir(int index_m) {
	if (index_m == front_l) {
		arr_dir[f_l] = !arr_dir[f_l];
		digitalWrite(motors[front_l][DIR], arr_dir[f_l]);
	}
	else if (index_m == front_r) {
		arr_dir[f_r] = !arr_dir[f_r];
		digitalWrite(motors[front_r][DIR], arr_dir[f_r]);
	}
	else if (index_m == back_l) {
		arr_dir[b_l] = !arr_dir[b_l];
		digitalWrite(motors[back_l][DIR], arr_dir[b_l]);
	}
	else {
		arr_dir[b_r] = !arr_dir[b_r];
		digitalWrite(motors[back_r][DIR], arr_dir[b_r]);
	}
}

/**
 * Re-initializes arr_dir[] array based on the value of front index
 * Also, writes these values to the motors
 */
void MotorDriver::setDir() {
	if (front_l == 0 || front_l == 1)
	{
		arr_dir[f_l] = LOW;
		arr_dir[f_r] = HIGH;
		arr_dir[b_r] = HIGH;
		arr_dir[b_l] = LOW;
	}
	else if (front_l == 2)
	{
		arr_dir[f_l] = LOW;
		arr_dir[f_r] = HIGH;
		arr_dir[b_l] = LOW;
		arr_dir[b_r] = HIGH;
	}
	else if (front_l == 3)
	{
		arr_dir[f_l] = HIGH;
		arr_dir[f_r] = LOW;
		arr_dir[b_l] = HIGH;
		arr_dir[b_r] = LOW;		
	}

	digitalWrite(motors[front_l][DIR], arr_dir[f_l]);
	digitalWrite(motors[front_r][DIR], arr_dir[f_r]);
	digitalWrite(motors[back_l][DIR], arr_dir[b_l]);
	digitalWrite(motors[back_r][DIR], arr_dir[b_r]);	
}

int MotorDriver::applyLag(int pin) {
	for (int i = 0; i < 2; i++)
		if(lagVolt[i][0] == pin)	// Lag is to be applied
			return lagVolt[i][1]; 	// Return the lag
	return 0;						// No lag is applied
}

/**
 * Contructor
 * Initializes motors[4][2] and lagVolt[2][2] arrays
 * Also sets the initial direction and lastMove
 * @param int[][3] m   Motor pins
 * @param int[][2]    lag Lag voltage for left and front motors
 */
MotorDriver::MotorDriver(int m[][2], int lag[][2]) {
	// Assiging pins to motors[][3] array
	for (int i = 0; i < MAX_MOTORS; i++)
		for (int j = 0; j < 2; j++) {
			motors[i][j] = m[i][j];
			pinMode(motors[i][j], OUTPUT);
		}
	
	// Setting lag
	for (int i = 0; i < 2; i++)
		for(int j = 0; j < 2; j++)
			lagVolt[i][j] = lag[i][j];

	// Initializing direction variables
	front_l = 0;
	front_r = 1;
	back_r = 2;
	back_l = 3;

	// Iinitializing directions[] array for forward movement
	arr_dir[f_l] = LOW;
	arr_dir[f_r] = HIGH;
	arr_dir[b_r] = HIGH;
	arr_dir[b_l] = LOW;

	for(int i = 0; i < MAX_MOTORS; i++)
		digitalWrite(motors[i][DIR], arr_dir[i]);

	// Initial move
	lastMove = 'f';
}

/**
 * Moves the bot in required direction
 * @param char dir    The direction in which the bot is supposed to move
 * @param int  volt   The voltage to be written
 * @param bool adjust Adjusting status; If enabled, adjacent motors won't stop
 */
void MotorDriver::move(char dir, int volt, bool adjust) {

	if (lastMove != dir) // Only if the last direction and current direction aren't the same
		switch (lastMove) {
			// Reset the directions
			case 'l':
				// Reset direction of front-left and back-right motors
				revDir(front_l);
				revDir(back_r);
				break;
			case 'r':
				revDir(front_r);
				revDir(back_l);
				break;
		}

	// Initialize m[] and set direction based on dir
	switch(dir) {
		case 'f':
			for(int i = 0; i < MAX_MOTORS; i++)
				analogWrite(motors[i][PWM], volt);

			break;
			
		case 'l':
			if (lastMove != dir) {
				revDir(front_l);
				revDir(back_r);
			}

			// Move only front-right and back-left motors
			analogWrite(motors[front_r][PWM], volt);
			analogWrite(motors[back_l][PWM], volt);

			if(adjust) {
				// Stop unwanted motors
				stop(front_l, back_r);
			}
			else {
				analogWrite(motors[front_l][PWM], volt);
				analogWrite(motors[back_r][PWM], volt);				
			}

			break;

		case 'r':
			if (lastMove != dir) {
				revDir(front_r);
				revDir(back_l);
			}

			// Move only front-left and back-right motors
			analogWrite(motors[front_l][PWM], volt);
			analogWrite(motors[back_r][PWM], volt);

			if (adjust) {
				// Stop unwanted motors
				stop(front_r, back_l);
			}
			else {
				analogWrite(motors[front_r][PWM], volt);
				analogWrite(motors[back_l][PWM], volt);				
			}

			break;
	}

	/*int lag;
	// Moving motors
	for(int i = 0; i < len_m; i++) {
		lag = applyLag(m[i]);
		analogWrite(m[i], volt + lag);
	}*/
	
	// Storing dir
	lastMove = dir;
}

/**
 * Stop all motors
 */
void MotorDriver::stop() {
	for (int i = 0; i < MAX_MOTORS; i++)
		digitalWrite(motors[i][PWM], LOW);
}

/**
 * Changes the indices values based on direction of turning
 * @param char dir	Direction of turning
 */
void MotorDriver::turn(char dir) {
	int temp;

	switch(dir) {
		case 'l':
			temp = front_l;
			front_l = back_l;
			back_l = back_r;
			back_r = front_r;
			front_r = temp;
			
			break;
		
		case 'r':
			temp = front_r;
			front_r = back_r;
			back_r = back_l;
			back_l = front_l;
			front_l = temp;

			break;
		
		case 'b':
			temp = front_l;
			front_l = back_r;
			back_r = temp;

			temp = front_r;
			front_r = back_l;
			back_l = temp;

			break;
	}

	setDir();

	lastMove = 'f';
}

#undef MAX_MOTORS
#undef PWM
#undef DIR
#undef f_l
#undef f_r
#undef b_r
#undef b_l

#endif
