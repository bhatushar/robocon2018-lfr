#ifndef PIDCONTROLLER_H
#define PIDCONTROLLER_H

/*
  Library to calculate voltage using PID controller
  kP, kI, kD are propotionality, integral and derivative constants respectively
*/

class PIDController {

private:
  float kP, kI, kD;
  int P, I, D;
  int lastErr;

public:
  PIDController(float const_p, float const_i, float const_d) {
    kP = const_p;
    kI = const_i;
    kD = const_d;

    P = I = D = 0;

    lastErr = 0;
  }

  int calcVolt(int);

};

int PIDController::calcVolt(int err) {
  P = kP * err;
  I += err;
  D = kD * (err - lastErr);
  int result = P + (kI * I) + D;

  lastErr = err; // Storing error for future use

  return (result > 0) ? result : -result; // Returning absolute value of the result
}

#endif
