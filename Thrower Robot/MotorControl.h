#include "pwm.h"

#define LMotor MOTOR1
#define RMotor MOTOR2

void LForward(u16 a){motor_control(LMotor, a, 1);} // Left motor turing forward
void RForward(u16 a){motor_control(RMotor, a, 1);} // Right motor turing forward
void LBackward(u16 a){motor_control(LMotor, a, 0);} // Left motor turing backward
void RBackward(u16 a){motor_control(RMotor, a, 0);} // Right motor turing backward

void Forward (u16 speed){LForward(speed);RForward(speed);} //Car move forward
void Backward (u16 speed){LBackward(speed);RBackward(speed);} //Car move backward
void TurnLeft (u16 speed){LBackward(speed);RForward(speed);} //Car turn left
void TurnRight (u16 speed){LForward(speed);RBackward(speed);} //Car turn right
void Stop(){motor_control(LMotor, 0, 1);motor_control(RMotor, 0, 1);} //Stop the car