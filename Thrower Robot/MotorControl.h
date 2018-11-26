#include "pwm.h"

#define LMotor MOTOR1
#define RMotor MOTOR3

void LForward(u16 a);
void RForward(u16 a);
void LBackward(u16 a);
void RBackward(u16 a);

void Forward (u16 speed);
void Backward (u16 speed);
void TurnLeft (u16 speed);
void TurnRight (u16 speed);
void Stop();

void MForward (u16 speed);
void MBackward (u16 speed);
void MTurnLeft (u16 speed);
void MTurnRight (u16 speed);
void MStop();
