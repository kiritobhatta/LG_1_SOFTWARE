#include "main.h"
#include "rcc.h"
#include "ticks.h"
#include "gpio.h"
#include "uart.h"
#include "lcd_main.h"
#include "oled.h"
#include "pwm.h"
#include "leds.h"
#include "buttons.h"
#include "buzzer.h"
#include "sonar.h"
#include "MotorControl.h"
//#include "LineSensor.h"
#include <stdbool.h>

typedef enum{AUTO, MANUAL}OperationMode;

u8 value_received; //value from Bluetooth connection
void UARTOnReceiveHandler(const u8 received){
  value_received = received;
}

int main() {
    // Initialize Everything Here
  rcc_init();
  ticks_init();
  
  tft_init(PIN_ON_TOP, BLACK, WHITE, RED, YELLOW);
	
  leds_init();
  buttons_init();
	
	//-------------------Initialization for bluetooth module
  uart_init(COM3,9600);
	//-------------------Initialization for Line Sensors
	gpio_init(GPIO5,GPIO_Mode_IPU);//left line sensor
	gpio_init(GPIO6,GPIO_Mode_IPU);//right line sensor
	gpio_init(GPIO7,GPIO_Mode_IPU);//counter sensor
	
	int left_sensor_reading;
	int right_sensor_reading;
	//-------------------Initialization for Motors
  motor_init(MOTOR1, 11, 1200, 0, 1);
  motor_init(MOTOR2, 11, 1200, 0, 1);
	
	const int forwardSpeed=200;
	const int turningSpeed=50;
	const int adjustingSpeed=10;
	//-----Initialization for ultrasonic Sensor
	SetSysClockTo72();
	sonar_init();
  uart_rx_init(COM3,&UARTOnReceiveHandler);  
  uint32_t lastticks=get_ticks();
  static u32 sonar_distance = 0;
	//-----------------------Line Sensor Counter initialization
	bool current_checker;
	bool previous_checker;
	int counter=0;
	//-----------------------Operation Mode Initialization
  OperationMode robot_mode;
	robot_mode=MANUAL;
  //-----------------------Auto Mode initialization for linesensors
	int stages=0;
	static u32 stop_time=0;
  while(1){
    if(lastticks!=get_ticks()){
      lastticks=get_ticks();
      if (!(lastticks%50)){
        //code here will run every 50
        if (robot_mode == MANUAL){
					tft_clear();
          if (value_received == 0){
            Stop();
						tft_prints(0,0,"Value_received: %d",value_received);
          }else if (value_received <= 50){
            Forward(value_received * 24);
						tft_prints(0,0,"Value_received: %d",value_received);
          }else if (value_received <= 100){
            TurnLeft((value_received - 50) * 24);
						tft_prints(0,0,"Value_received: %d",value_received);
          }else if (value_received <= 150){
            Backward((value_received - 100) * 24);
						tft_prints(0,0,"Value_received: %d",value_received);
          }else if (value_received <= 200){
            TurnRight((value_received - 150) * 24);
						tft_prints(0,0,"Value_received: %d",value_received);
          }else if (value_received == 210){
            //SHOOT
          }else if (value_received == 220){
            //GRAB
          }else if (value_received == 220){
            //RELEASE
          }
        }
      }
      
      if (robot_mode == AUTO){
				//------------------------------------------------From here on, it is for the AUTO MODE - LINE SENSOR PATHING
				left_sensor_reading=gpio_read(GPIO5);
				right_sensor_reading=gpio_read(GPIO6);
				
				if(gpio_read(GPIO7)){current_checker=true;}
				else{current_checker=false;}	
				//-------------------------------------------------------------------------------				
				if(current_checker!=previous_checker){
					counter++;
				}
				//Everytime there is a change in the input value compared to the value in the previous tick, counter increases
				//-------------------------------------------------------------------------------
				if(gpio_read(GPIO7)){previous_checker=true;}
				else{previous_checker=false;}
				
				if(counter==5){
					Stop();
				}
				
				switch(stages%7){
				//------------------------------------Stage1
					case 0:{//Moving Straight forward
						while(counter<5){
							if(left_sensor_reading==1 && right_sensor_reading==1){
								Forward(forwardSpeed);
							}
							else{
								Stop();
							}
						}
						if(counter==5){
							Stop();
							stages++;
							counter=0;
						}
					}
			//-------------------------------------Stage2
					case 1:{
						while(counter<3){
							if(left_sensor_reading==1 && right_sensor_reading==1){//while they are in parallel
								Backward(forwardSpeed);
							}
							else{
								Stop();
							}
						}
						if(counter==3){
							Stop();
							stages++;
							counter=0;
						}	
					}
			//-----------------------------------Stage3
					case 2:{
						while(left_sensor_reading!=1 && right_sensor_reading==1){//Until they come in parallel
							LForward(turningSpeed);
						}
						if(left_sensor_reading==1 && right_sensor_reading==1){
							stages++;
							counter=0;
						}
					}
		 //-------------------------------------Stage4
					case 3:{
						while(counter<5){
							if(left_sensor_reading==1 && right_sensor_reading==1){
								Forward(forwardSpeed);
							}
							else{
								Stop();
							}
						}
						if(counter==5){
							Stop();
							stages++;
							counter=0;
						}
					}
			  //-------------------------------------Stage5
					case 4:{
						while(left_sensor_reading!=1 && right_sensor_reading==1){//Until they come in parallel
							LForward(turningSpeed);
						}
						if(left_sensor_reading==1 && right_sensor_reading==1){
							stages++;
							counter=0;
						}
					}
					
			 //-------------------------------------Stage6
					case 5:{
						while(counter<2){
							if(left_sensor_reading==1 && right_sensor_reading==1){
								Forward(forwardSpeed);
							}
							else{
								Stop();
							}
						}
						if(counter==2){
							stages++;
							counter=0;
						}
					}
		  //---------------------------------------Stage7
					case 6:{
						stop_time++;
						if(stop_time<=5000){//stops for 5 secs in stopping zone
							Stop();
						}
						else{
							stages++;
							counter=0;
						}
					}
				}
				//------------------------------------------------From here on is for the AUTO MODE - ULTRASONIC SENSOR
        sonar_start();
				
				
        tft_clear();
        sonar_distance = sonar_get();
        tft_prints(0, 0, "%d", sonar_distance);
        tft_update();
                  
        if ((sonar_distance >= 100) && (sonar_distance <= 250)){
          //grab
        }else if (sonar_distance > 1000){
          //Forward(300);
        }else if (sonar_distance > 250){
          //Forward(300);
        }else if (sonar_distance < 100){
          //Backward(300);
        }else {
          Stop();
        }
        
        if (value_received == 255){
          robot_mode = MANUAL;
        }
      }
      
      //print info to tft if button 1 is pressed
      if (!(lastticks%250)){
        if (button_pressed(BUTTON1)){
          //get the distance from the object to the ultrasonic sensor in mm
          //output the distance on tft in mm
          tft_clear();
          tft_prints(0, 0, "Sonar: %d", sonar_distance);
          tft_prints(10,0, "Bluetooth: %d", value_received);
          tft_update();
        }
      }
    }
  }        
}