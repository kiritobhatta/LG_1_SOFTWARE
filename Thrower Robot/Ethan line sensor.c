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
	static u8 grab_counter = 0;
	static u8 grab_pressed = 0;
	
	gpio_init(GPIO3,GPIO_Mode_Out_PP);//Lift
  static u8 lift_counter = 0;
	static u8 lift_pressed = 0;
	
	//-------------------Initialization for bluetooth module
  uart_init(COM3,9600);
	//-------------------Initialization for Line Sensors
	gpio_init(GPIO5,GPIO_Mode_IPU);//right line sensor
	gpio_init(GPIO8,GPIO_Mode_IPU);//middle line sensor
	gpio_init(GPIO7,GPIO_Mode_IPU);//right line sensor
	
	//-------------------Initialization for Motors
  motor_init(MOTOR1, 6, 1200, 1200, 0);//Initialized to Stop ---- Wheel Rotation Direction (0-Forward, 1- Backward)
  motor_init(MOTOR3, 6, 1200, 1200, 0);//Initialized to Stop---- Wheel Rotation Direction (0-Forward, 1-Backward)
	
	//Maxium Speed = 0
	const u16 forwardSpeed = 600;
	const u16 turningSpeed = 200;
	//-----Initialization for ultrasonic Sensor
	SetSysClockTo72();
	sonar_init();
  uart_rx_init(COM3,&UARTOnReceiveHandler);  
  uint32_t lastticks=get_ticks();
  static u32 sonar_distance = 0;
	//-----------------------Line Sensor Counter initialization
	u8 disable;
	u8 counter = 0;
	//-----------------------Operation Mode Initialization
  OperationMode robot_mode;
	int mode=0;
	robot_mode=MANUAL;
  //-----------------------Auto Mode initialization for linesensors
	int stages=0;
	
	
	
	static u32 stop_time=0,last_ticks,this_ticks;
  while(1){
    if(lastticks!=get_ticks()){
      lastticks=get_ticks();
			
			this_ticks=get_ticks();
			
			//----------------------------------Button 1 can be used to interchange between Manual and Auto Mode
			/*if (this_ticks - last_ticks >= 50) {
				static u8 debounce;
				if (!button_pressed(BUTTON1) && debounce) {debounce = 0;}
			// set debounce if button is initially pressed
				if (button_pressed(BUTTON1) && !debounce) {debounce = 1;mode++;}
				else if (button_pressed(BUTTON1) && debounce) {}
			}
			if(mode%2==0){robot_mode=AUTO;}
			else if(mode%2==1){robot_mode=MANUAL;}
			*/
      if (!(lastticks%50)){
        //code here will run every 50
        if (robot_mode == MANUAL){
					//tft_clear();
          if (value_received == 0){
						led_on(LED1);
            Stop();
						/*
						tft_prints(0,0,"Value: %d",value_received);
						tft_prints(0,1,"Stopped");
						tft_update();*/
          }else if (value_received <= 50){
            Forward(value_received*24);
						led_on(LED1);
						/*
						tft_prints(0,0,"Value: %d",value_received);
						tft_prints(0,1,"Moving Forward");
						tft_update();*/
          }else if (value_received <= 100){
            TurnLeft((value_received - 50)*24);
						led_on(LED1);
						/*
						tft_prints(0,0,"Value: %d",value_received);
						tft_prints(0,1,"Turning Left");
						tft_update();*/
          }else if (value_received <= 150){
            Backward((value_received - 100)*24);
						led_on(LED1);
						/*
						tft_prints(0,0,"Value: %d",value_received);
						tft_prints(0,1,"Turning Right");
						tft_update();*/
          }else if (value_received <= 200){
						led_on(LED1);
            TurnRight((value_received - 150)*24);
						/*
						tft_prints(0,0,"Value: %d",value_received);
						tft_update();*/
          }else if (value_received == 210){//Shoot
						/*
						tft_prints(0,0,"Value: %d",value_received);
						tft_update();*/
            gpio_set(GPIO1);//Set gpio as high
						
						//led_on(LED1);
          }else{
					  led_off(LED1);
					}
					static u8 debounce2=0,debounce3=0;
					//-----------------Debouncing for grab and lift actions
					if (value_received == 220 && !debounce2){//When initially pressed for grab
						grab_counter++;
						debounce2=1;
					}
					else if(value_received!=220 && debounce2){
						debounce2=0;
					}
					
          if (value_received == 230 && !debounce3){//When initially pressed for lift
							lift_counter++;
							debounce3=1;
					}
					else if(value_received!=230 && debounce3){
							debounce3=0;
					}
					
					if((grab_counter%2)==1){
						gpio_set(GPIO2);
					}
					else if((grab_counter%2)==0){
						gpio_reset(GPIO2);
					}
					
					//WILL GRAB IF SIGNAL SENT AS HIGH
					if((lift_counter%2)==1){
						gpio_set(GPIO3);
					}
					else if((lift_counter%2)==0){//wILL RELEASE IF SIGNAL SENT AS LOW
						gpio_reset(GPIO3);
					}
					
					//----------------------------------Solenoid Valve communication
					if(value_received!=210){//While button isn't pressed, shooting command doesn't work
						gpio_reset(GPIO1);
					}
        }
      }
      
      if (robot_mode == AUTO){
				//------------------------------------------------From here on, it is for the AUTO MODE - LINE SENSOR PATHING
				u8 LReading = gpio_read(GPIO7);
				u8 MReading = gpio_read(GPIO8);
				u8 RReading = gpio_read(GPIO5);
				static u8 stage = 0;
				//0 is black!!!
				/*
				if(LReading == 1){
				  led_on(LED1);
					led_off(LED2);
				}else if (LReading == 0){
				  led_on(LED2);
					led_off(LED1);
				}
				*/
				if (!MReading){
				  disable = 1;
					counter += 1;
				}else if (disable == 1){
				  if (counter < 20){
					  counter += 1;
					}else {
					  counter = 0;
						disable = 0;
					}
				}
				
				if (sonar_distance > 80){
				  
					if (LReading && RReading){ //---forward OOO OXO (4)
				    Forward(forwardSpeed);
				  }else if (!LReading && RReading && !disable){ //---turn right XOO!d XXO!d
				    LForward(forwardSpeed + turningSpeed);
					  RForward(forwardSpeed);
				  }else if (LReading && !RReading && !disable){ //---turn left OOX!d OXX!d
				    LForward(forwardSpeed);
					  RForward(forwardSpeed + turningSpeed);
				  }else if (!LReading && !RReading && disable){ //--- XXXd XOXd
				    Forward(forwardSpeed);
				  }else {
					  Stop();
					}
				}else {
				  //grab
				}
				
				/*
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
        }*/
      }
      
			
      //print info to tft if button 1 is pressed
      /*if (!(lastticks%250)){
        if (button_pressed(BUTTON1)){
          //get the distance from the object to the ultrasonic sensor in mm
          //output the distance on tft in mm
          tft_clear();
          tft_prints(0, 0, "Sonar: %d", sonar_distance);
          tft_prints(0,6, "Bluetooth: %d", value_received);
          tft_update();
        }
      }*/
    }
  }        
}
