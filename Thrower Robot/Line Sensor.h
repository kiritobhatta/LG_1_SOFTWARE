/*
	Library for Line Sensor
*/

typedef enum{LINE, NO_LINE}sideSensor_state;//If leaves line, it should speed up the motor on that side
typedef enum{Left, Right}sideSensor_value;//Which sensor is it referring to
typedef enum{MoveLeft, MoveRight, Straight}maneuverDirection;
typedef enum{Forward, Backward}movementDirection;

#define GPIO1 &PA0//First pin for center sensor
#define GPIO2 &PA1//second pin for the left sensor
#define GPIO3 &PA2//third pin for the right sensor

const int turning_speed=80;
const int moving_forward_speed=100;
const int difference_speed=20;

//SIDE sensors will entirely be used for movement only throughout the operation
/*
*Function: SIDESensorInput
*To return the input received by the respective pins for the respective side sensors
*returns an integer value 0 or 1
*0 for low and 1 for high
* Passed Parameters:
* 1. left_or_right: to determine which sensor's value to read from
*/


int SIDESensorInput(sideSensor_value left_or_right){
	int high_low;
	
	switch(left_or_right){
		case Left:
			high_low=gpio_read(GPIO2);
		
			if(high_low==0){
				return 1;
				break;
			}
			else{
				return 0;
				break;
			}
			
		case Right:
			high_low=gpio_read(GPIO3);
		
			if(high_low==0){
				return 1;
				break;
			}
			else{
				return 0;
				break;
			}
	}
}

/*
*Function:throwerRobotAutonomous
*To control the entire movement of the thrower robot within this function according to the line sensor inputs
*No return value (Mainly to direct control the circuit boards)
*Parameters passed:
* 1. left_sensor: the output for left sensor( High or Low )
* 2. right_sensor: the output for right_sensor( High or Low )
* 3. previous_speed: the array which will store the speed 250ms ago
* 4. turning_mode: whether turning mode is on or not
* 5. movementDirection: Forward or Backward
*/
void throwerRobotAutonomous(int left_sensor, int right_sensor, int previous_speed,bool turning_mode,movementDirection forward_backward
maneuverDirection left_right_straight){

//previous_speed will be a 2 element array which will store the pwm signal send to the two motors previously
//this array element values will be updated every 250ms
	if(turning_mode==true){
		bool completion=false;
		do{
			turnright(turning_speed);
			if(left_sensor==1){//If the left_sensor senses the line
				completion=true;//this stops the turning
			}
		}while(completion!=true);
		if(completion==true){turning_mode==false;}//this changes the mode of movement
	}
	else{
		if(forward_backward==Forward){
			if(left_sensor==1 && right_sensor==1){//This means that the thrower robot base is parallel to the two lines
				Forward(LForward(moving_forward_speed),RForward(moving_forward_speed));
			}
			if(left_sensor==0 && right_sensor==1){//right sensor in line while left sensor out of line 
				Forward(LForward(moving_forward_speed
			}
		}
		else if(forward_backward==Backward){
		
		}
	}
	
}