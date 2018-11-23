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
  leds_init();
	led_on(LED1);
	
  tft_init(PIN_ON_BOTTOM, BLACK, WHITE, RED, YELLOW);
	tft_clear();
	//tft_prints(0,0,"Value: 0");
	tft_update();
	
  leds_init();
  buttons_init();
	
	//-------------------Initialization for solenoid Valve
	gpio_init(GPIO1,GPIO_Mode_Out_PP);//Shoot
	static u32 shoot_recieved;
	bool shoot=false;
	
	gpio_init(GPIO2,GPIO_Mode_Out_PP);//Grab

	
	gpio_init(GPIO3,GPIO_Mode_Out_PP);//Lift

	
	//-------------------Initialization for bluetooth module
  uart_init(COM3,9600);
	//-------------------Initialization for Line Sensors
	gpio_init(GPIO5,GPIO_Mode_IPU);//left line sensor
	gpio_init(GPIO6,GPIO_Mode_IPU);//right line sensor
	gpio_init(GPIO7,GPIO_Mode_IPU);//counter sensor
	
	int left_sensor_reading;
	int right_sensor_reading;
	//-------------------Initialization for Motors
  motor_init(MOTOR1, 1, 7200, 7000, 0);//Initialized to Stop ---- Wheel Rotation Direction (0-Forward, 1- Backward)
  motor_init(MOTOR2, 1, 7200, 7000, 0);//Initialized to Stop
	
	//Maxium Speed = 0
	const int forwardSpeed=0;
	const int turningSpeed=5000;
	const int adjustingSpeed=6000;
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
						tft_prints(0,0,"Value: %d",value_received);
						tft_prints(0,1,"Stopped");
						tft_update();
          }else if (value_received <= 50){
            Forward(7000-(value_received*140));
						tft_prints(0,0,"Value: %d",value_received);
						tft_prints(0,1,"Moving Forward");
						tft_update();
          }else if (value_received <= 100){
            TurnLeft(7000-((value_received - 50)*140));
						tft_prints(0,0,"Value: %d",value_received);
						tft_prints(0,1,"Turning Left");
						tft_update();
          }else if (value_received <= 150){
            Backward(7000-((value_received - 100)*140));
						tft_prints(0,0,"Value: %d",value_received);
						tft_prints(0,1,"Turning Right");
						tft_update();
          }else if (value_received <= 200){
            TurnRight(7000-((value_received - 150)*140));
						tft_prints(0,0,"Value: %d",value_received);
						tft_update();
          }else if (value_received == 210){//Shoot
						tft_prints(0,0,"Value: %d",value_received);
						tft_update();
            gpio_set(GPIO1);//Set gpio as high
          }else if (value_received == 220){//To Grab
						tft_prints(0,0,"Value: %d",value_received);
						tft_update();
            gpio_set(GPIO2);
						tft_prints(0,1,"Shooting has started");
          }else if (value_received == 230){//To Lift
						tft_prints(0,0,"Value: %d",value_received);
						tft_update();
            gpio_set(GPIO3);
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
					tft_clear();
					tft_prints(0,2,"Counter: %d",counter);
				}
				//Everytime there is a change in the input value compared to the value in the previous tick, counter increases
				//-------------------------------------------------------------------------------
				if(gpio_read(GPIO7)){previous_checker=true;}
				else{previous_checker=false;}
				
				
				switch(stages%7){
				//------------------------------------Stage1---Moving Straight Forward
					case 0:{
						tft_prints(0,3,"Stage 1");
						
						if(counter<5){//Moving until the first stage has been completed
							if(left_sensor_reading==1 && right_sensor_reading==1){
								Forward(forwardSpeed);
								tft_prints(0,4,"Moving Forward");
							}
							else{
								Stop();
								tft_prints(0,4,"Stopped");
							}
						}
						if(counter==5){
							Stop();
							stages++;
							counter=0;
						}
					}
			//-------------------------------------Stage2---Moving Backward 
					case 1:{
						tft_prints(0,3,"Stage 2");
						if(counter<3){
							if(left_sensor_reading==1 && right_sensor_reading==1){//while they are in parallel
								Backward(forwardSpeed);
								tft_prints(0,4,"Moving Backward");
							}
							else{
								Stop();
								tft_prints(0,4,"Stopped");
							}
						}
						if(counter==3){
							Stop();
							stages++;
							counter=0;
						}	
					}
			//-----------------------------------Stage3---Turning Right
					case 2:{//turning -- will loop every ms while in automode and in this
						tft_prints(0,3,"Stage 3");
						if(right_sensor_reading!=1){//make adjustment and move accordingly
							Forward(turningSpeed);
							tft_prints(0,4,"Adjusting");
						}
						if(left_sensor_reading!=1 && right_sensor_reading==1){//Until they come in parallel
							LForward(turningSpeed);
							RForward(0);
							tft_prints(0,4,"Turning");
						}	
						if(left_sensor_reading==1 && right_sensor_reading==1){
							stages++;
							counter=0;
						}
					}
		 //-------------------------------------Stage4---Move Forward
					case 3:{
						tft_prints(0,3,"Stage 4");
						if(counter<5){
							if(left_sensor_reading==1 && right_sensor_reading==1){
								Forward(forwardSpeed);
								tft_prints(0,4,"Moving forward");
							}
							else{
								Stop();
								tft_prints(0,4,"Stopped");
							}
						}
						if(counter==5){
							Stop();
							stages++;
							counter=0;
						}
					}
			  //-------------------------------------Stage5---Turning
					case 4:{
						tft_prints(0,3,"Stage 5");
						if(left_sensor_reading!=1 && right_sensor_reading==1){//Until they come in parallel
							LForward(turningSpeed);
							RForward(0);
							tft_prints(0,4,"Turning");
						}
						if(left_sensor_reading==1 && right_sensor_reading==1){
							stages++;
							counter=0;
						}
					}
			 //-------------------------------------Stage6
					case 5:{
						tft_prints(0,3,"Stage 6");
						if(counter<2){
							if(left_sensor_reading==1 && right_sensor_reading==1){
								Forward(forwardSpeed);
								tft_prints(0,4,"Moving forward");
							}
							else{
								Stop();
								tft_prints(0,4,"Stopped");
							}
						}
						if(counter==2){
							stages++;
							counter=0;
						}
					}
		  //---------------------------------------Stage7
					case 6:{
						tft_prints(0,3,"Stage 7");
						stop_time++;
						if(stop_time<=5000){//stops for 5 secs in stopping zone
							Stop();
							tft_prints(0,4,"Stopped");
						}
						else{
							stages++;
							counter=0;
						}
					}
				}
				//tft_update();
				//------------------------------------------------From here on is for the AUTO MODE - ULTRASONIC SENSOR
        sonar_start();
				
				
        //tft_clear();
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
          tft_prints(0,6, "Bluetooth: %d", value_received);
          tft_update();
        }
      }
    }
  }        
}