//MotorControl.c

#include "pwm.h"
#include "MotorControl.h"

u16 adjust = 55; //adjust = 70 for 100% duty cycle
u16 Lmotor = 0;
u16 Rmotor = 0;

void LForward(u16 a){
	Lmotor = 1200-a;
	if (a>=adjust){
	  Lmotor = 1200 - a + adjust;
	}else {
	  Lmotor = 1200;
	}
  motor_control(LMotor, Lmotor, 1);
} // Left motor turing forward-----Kyle---Done
void LBackward(u16 a){
  Lmotor = 1200-a;
	if (a>=adjust){
	  Lmotor = 1200 - a + adjust;
	}else {
	  Lmotor = 1200;
	}
	motor_control(LMotor, Lmotor, 0);
} // Left motor turing backward----Kyle---Done


void RForward(u16 a){
	Rmotor = 1200-a;
	if (a<=(1200-adjust)){
	  Rmotor = 1200 - a - adjust;
	}else {
	  Rmotor = 0;
	}
  motor_control(RMotor, Rmotor, 1);
} // Right motor turing forward----Jonathan(dir remains constant)
void RBackward(u16 a){
	Rmotor = 1200-a;
	if (a<=(1200-adjust)){
	  Rmotor = 1200 - a - adjust;
	}else {
	  Rmotor = 0;
	}
  motor_control(RMotor, Rmotor, 0);
} // Right motor turing backward----Jonathan

void Forward (u16 speed){LForward(speed);RForward(speed);} //Car move forward
void Backward (u16 speed){LBackward(speed);RBackward(speed);} //Car move backward
void TurnLeft (u16 speed){LBackward(speed);RForward(speed);} //Car turn left
void TurnRight (u16 speed){LForward(speed);RBackward(speed);} //Car turn right
void Stop(){motor_control(LMotor, 1200, 1);motor_control(RMotor, 1200, 1);} //Stop the car

void LMForward(u16 a){motor_control(LMotor,1200-a,1);}
void RMForward(u16 a){motor_control(RMotor,1200-a,1);}
void LMBackward(u16 a){motor_control(LMotor,1200-a,0);}
void RMBackward(u16 a){motor_control(RMotor,1200-a,0);}

void MForward(u16 speed){LMForward(speed);RMForward(speed);}
void MBackward(u16 speed){LMBackward(speed);RMBackward(speed);}
void MTurnLeft(u16 speed){LMBackward(speed);RMForward(speed);}
void MTurnRight(u16 speed){LMForward(speed);RMBackward(speed);}
void MStop(){motor_control(LMotor,1200,1);motor_control(RMotor,1200,1);}