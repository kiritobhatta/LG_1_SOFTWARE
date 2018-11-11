#include "main.h"
#include "rcc.h"
#include "ticks.h"
#include "gpio.h"
#include "uart.h"
#include "lcd_main.h"
#include "oled.h"
#include "camera.h"
#include "pwm.h"
#include "adc.h"
#include "leds.h"
#include "buttons.h"
#include "buzzer.h"
#include "MotorControl.h"
//#include "LineSensor.h"
#include "Sonar.h"
#include <stdbool.h>

typedef enum{AUTO, MANUAL}Operation_Mode;
typedef enum{STRAIGHT_L,PUSH_FROM_LEFT}Left_Motor_Motion;
typedef enum{STRAIGHT_R,PUSH_FROM_RIGHT}Right_Motor_Motion;

u32 value_received; //value from Bluetooth connection
void UARTOnReceiveHandler(const u8 received){
  value_received = received;
}

int main() {
    // Initialize Everything Here
		
    rcc_init();
    ticks_init();
   
    leds_init();
    buttons_init();
		buzzer_init();
		tft_init(PIN_ON_LEFT, BLACK, WHITE, WHITE, WHITE);
   // uart_init(COM3,115200);
		motor_init(MOTOR1, 1, 11, 1200, 1);
		motor_init(MOTOR2, 1, 11, 1200, 1);
    
		//Initialising the gpio pin for the line sensor
		gpio_init(GPIO5, GPIO_Mode_IPU);//input pin5
		gpio_init(GPIO6, GPIO_Mode_IPU);//input pin6 for Right line sensor
		gpio_init(GPIO7, GPIO_Mode_IPU);
	
		int left_line_sensor=0;
		int right_line_sensor=0;
		
		bool previous_checker,current_checker;
		int counter=0;
		
		const int forwardSpeed=200;
		const int differenceSpeed=50;
		const int turningSpeed=80;
		const int stoppingSpeed=1200;
		
		Operation_Mode robot_mode;
		robot_mode=AUTO;//INITIALISE ROBOT MODE AS AUTO
		
    while(1){
			static u32 this_ticks=0;
      while(get_ticks()==this_ticks);
			this_ticks=get_ticks();
			
			
			left_line_sensor=gpio_read(GPIO5);
			right_line_sensor=gpio_read(GPIO6);
			
			tft_clear();
			tft_prints(0,0,"Left Sensor: %d",left_line_sensor);
			tft_prints(0,1,"Right Line Sensor: %d",right_line_sensor);
			tft_prints(0,2,"Left Motor Speed:");
			tft_prints(0,4,"Right Motor Speed:");
			tft_prints(0,6,"Counter Value:");
			tft_update();
			
			static u32 last_ticks=0;
			if((this_ticks-last_ticks)>=200){//Code from here and below will only work once every 200 ms
				
				
				/*
				 *The following code is to use the line sensor for counting grid function
				 *
				*/
				if(gpio_read(GPIO5)){current_checker=true;}
				else{current_checker=false;}	
//-------------------------------------------------------------------------------				
				if(current_checker!=previous_checker){
					counter++;
					tft_prints(0,7,"%d",counter);
				}
//Everytime there is a change in the input value compared to the value in the previous tick, counter increases
//-------------------------------------------------------------------------------
				if(gpio_read(GPIO5)){previous_checker=true;}
				else{previous_checker=false;}
				
				
				tft_prints(0,1,"%d",counter);
				tft_update();
				
				
				/*
				 *The code from here and below is for the side sensors to bounce the thrower robot away from the lines
				 *
				 */
				
				//if(robot_mode==AUTO){
					buzzer_on();
					if(left_line_sensor==0 && right_line_sensor==0){//If side sensor doesn't sense the line
						Forward(forwardSpeed);
					
						tft_prints(0,3,"%d",forwardSpeed);
						tft_prints(0,5,"%d",forwardSpeed);
						tft_update();
					}
					else if(left_line_sensor==1 && right_line_sensor==0){
						LForward(forwardSpeed);//Left motor remains constant
						RForward(forwardSpeed-differenceSpeed);//Right motor slows down
					
						tft_prints(0,3,"%d",forwardSpeed);
						tft_prints(0,5,"%d",forwardSpeed-differenceSpeed);
						tft_update();
					}
					else if(left_line_sensor==0 && right_line_sensor==1){
						LForward(forwardSpeed-differenceSpeed);
						RForward(forwardSpeed);
					
						tft_prints(0,3,"%d",forwardSpeed-differenceSpeed);
						tft_prints(0,5,"%d",forwardSpeed);
						tft_update();
					}
					else if(left_line_sensor==1 && right_line_sensor==1){
						Stop();
					
						tft_prints(0,3,"7200");
						tft_prints(0,5,"7200");
						tft_update();
					}
				
				
			}
			
			
//----------------------------------------------------------------------------------------------------------------------						
							if(robot_mode==MANUAL){
								if (value_received == 0){
									Stop();
								}else if (value_received <= 50){
									Forward(value_received * 24);
								}else if (value_received <= 100){
									TurnLeft((value_received - 50) * 24);
								}else if (value_received <= 150){
									Backward((value_received - 100) * 24);
								}else if (value_received <= 200){
									TurnRight((value_received - 150) * 24);
								}
							}
			
			}
		}