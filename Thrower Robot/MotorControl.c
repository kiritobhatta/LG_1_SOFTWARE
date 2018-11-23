//MotorControl.c

#include "pwm.h"

#define LMotor MOTOR1 //Assume this is Kyle's Hbridge---- LMotor-> Kyle's Hbridge
#define RMotor MOTOR3	//Assume this is Jonathan's Hbridge---- RMotor->Jonathan's Hbridge

void LForward(u16 a){motor_control(LMotor, 1200-a, 0);} // Left motor turing forward-----Kyle---Done
void LBackward(u16 a){motor_control(LMotor, 1200-a, 1);} // Left motor turing backward----Kyle---Done


void RForward(u16 a){motor_control(RMotor, 1200-a, 0);} // Right motor turing forward----Jonathan(dir remains constant)
void RBackward(u16 a){motor_control(RMotor, 1200-a, 1);} // Right motor turing backward----Jonathan

void Forward (u16 speed){LForward(speed);RForward(speed);} //Car move forward
void Backward (u16 speed){LBackward(speed);RBackward(speed);} //Car move backward
void TurnLeft (u16 speed){LBackward(speed);RForward(speed);} //Car turn left
void TurnRight (u16 speed){LForward(speed);RBackward(speed);} //Car turn right
void Stop(){motor_control(LMotor, 1200, 1);motor_control(RMotor, 1200, 1);} //Stop the car