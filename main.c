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

typedef enum{AUTO, MANUAL, STOP}OperationMode;

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
	//tft_clear();
	//tft_prints(0,0,"Value: 0");
	//tft_update();
	
  leds_init();
  buttons_init();
	int mode=0;
	//-------------------Initialization for solenoid Valve
	gpio_init(GPIO1,GPIO_Mode_Out_PP);//Shoot
	static u32 shoot_recieved;
	bool shoot=false;
	
	gpio_init(GPIO2,GPIO_Mode_Out_PP);//Grab
	int grab_counter=1;
	int grab_checker=0;
	static u32 grab_recieved;
	
	gpio_init(GPIO3,GPIO_Mode_Out_PP);//Lift
	u8 lift_counter=1;
	u8 lift_checker=0;
	static u32 lift_received;
	
	//-------------------Initialization for bluetooth module
  uart_init(COM3,9600);
	//-------------------Initialization for Line Sensors
	gpio_init(GPIO5,GPIO_Mode_IPU);//left line sensor
	gpio_init(GPIO8,GPIO_Mode_IPU);//middle line sensor
	gpio_init(GPIO7,GPIO_Mode_IPU);//right line sensor
	
	u8 left_sensor_reading;
  u8 right_sensor_reading;
	//-------------------Initialization for Motors
  motor_init(MOTOR1, 6, 1200, 1200, 1);//Initialized to Stop ---- Wheel Rotation Direction (1-Forward, 0- Backward)
  motor_init(MOTOR3, 6, 1200, 1200, 1);//Initialized to Stop---- Wheel Rotation Direction (1-Forward, 0-Backward)
	
	//Maximum Speed = 0
	const u16 forwardSpeed=400;
	const u16 adjustingSpeed=200;
	//-----Initialization for ultrasonic Sensor
	SetSysClockTo72();
	sonar_init();
  uart_rx_init(COM3,&UARTOnReceiveHandler);  
  uint32_t lastticks=get_ticks();
  static u32 sonar_distance = 0;
	
	u8 UltrasonicStop=0;
	u8 EverythingDone=0;
	//-----------------------Line Sensor Counter initialization
	bool current_checker;
	bool previous_checker;
	u8 counter=0;
	//-----------------------Operation Mode Initialization
  OperationMode robot_mode;
	robot_mode=AUTO;
  //-----------------------Auto Mode initialization for linesensors
	int auto_mode_grabbing=0;
	static u32 auto_grab_time;
	int auto_grabbing_done=0;
	static u32 mid_sensor_sense=0;
	
	int ultrasonic_stop=0;
	static u32 ultrasonic_stopping_time=0;
	
	int stop_grab=0;
	static u32 stop_grab_time=0;
	
	static u32 stop_back_time=0;
	int stop_back=0;
	
	//-------------------------------For shaking of the motor
	static u32 shaking_time=0;
	int shaking_counter=0;
	
	static u32 stop_time=0,last_ticks,this_ticks;
  while(1){
    if(lastticks!=get_ticks()){
      lastticks=get_ticks();
			
			this_ticks=get_ticks();
			
			
			
			//----------------------------------Button 1 can be used to interchange between Manual and Auto Mode
		
/*			if (this_ticks - last_ticks >= 50) {
				static u8 debounce;
				if (!button_pressed(BUTTON1) && debounce) {debounce = 0;}
			// set debounce if button is initially pressed
				if (button_pressed(BUTTON1) && !debounce) {debounce = 1;mode++;}
				else if (button_pressed(BUTTON1) && debounce) {}
			}
			if(mode%2==0){robot_mode=AUTO;
			//led_on(LED1);led_off(LED2);
			}
			else if(mode%2==1){robot_mode=MANUAL;
			//led_on(LED2);led_off(LED1);
			}*/
			//--------------------------------The codes above are to use button 1 to change the mode
			
			
			
    if (!(lastticks%50)){//Code from here runs every 50ms
				
				 //---------------------------------AUTO CODE MODE STARTS FROM HERE
			/*
			static u8 b1 = 0;
			if (button_pressed(BUTTON1) && (b1 == 0)){
			  b1 = 1;
			}else if (!(button_pressed(BUTTON1)) && (b1 == 1)){
			  b1 = 0;
				robot_mode = AUTO;
			}
			*/
			//sonar_start();
			//sonar_distance = sonar_get();
	//		tft_clear();
		//	tft_prints(0,0,"sonar: ", sonar_distance);
		//	tft_update();
			
      if (robot_mode == AUTO){
				led_on(LED1);
				led_off(LED2);
				//------------------------------------------------From here on, it is for the AUTO MODE - LINE SENSOR PATHING
				left_sensor_reading=gpio_read(GPIO7);//Reading 1 is black, Reading 0 is non-black
				right_sensor_reading=gpio_read(GPIO5);//right line sensor
				static u8 mid_sensor_debounce;
				
				
				//---------------------The code here is for counting
				if(gpio_read(GPIO8)){current_checker=true;}//Middle Sensors
				else{current_checker=false;}	
				//-------------------------------------------------------------------------------				
				if(current_checker!=previous_checker){
					counter++;
					//tft_clear();
					//tft_prints(0,2,"Counter: %d",counter);
				}
				//Everytime there is a change in the input value compared to the value in the previous tick, counter increases
				//--------------------The code here is for counting lines
				
				
				if(gpio_read(GPIO8)){previous_checker=true;}
				else{previous_checker=false;}
				
				if(gpio_read(GPIO8) && !mid_sensor_debounce){//When it first senses the line
					mid_sensor_sense=get_ticks();//gets the time when it first senses the black line 
					mid_sensor_debounce=1;
				}
				else if(!gpio_read(GPIO8) && mid_sensor_debounce){//After cannot detect anymore, reset the debounce
					mid_sensor_debounce=0;
				}
				
				if(UltrasonicStop==0){//First Stage where the counter is less than 5
					//----------------------------------Normal Movement
					
					if((get_ticks()-mid_sensor_sense)>=100){//The left sensors and rightsensors effect will be delayed
						if(left_sensor_reading && right_sensor_reading){
							//tft_clear();
							//tft_prints(0,1,"moving Forward");
							//tft_update();
							Forward(forwardSpeed);
						}
						else if(!left_sensor_reading && right_sensor_reading){//LEFT sensor touches line
							LForward(forwardSpeed + adjustingSpeed);
							RForward(forwardSpeed);
							//tft_clear();
							//tft_prints(0,1,"moving RIGHT");
							//tft_update();
						}
						else if(left_sensor_reading && !right_sensor_reading){//RIGHT sensor touches line
							LForward(forwardSpeed);
							RForward(forwardSpeed + adjustingSpeed);
							//ft_clear();
							//tft_prints(0,1,"moving LEFT");
							//tft_update();
						}
					}
					else if((get_ticks()-mid_sensor_sense)<80){//In between will move forward
						//Forward(forwardSpeed);
						LForward(forwardSpeed+adjustingSpeed+128);
						RForward(forwardSpeed-adjustingSpeed);
						//tft_clear();
						//tft_prints(0,1,"Fwd,L/R delay");
						//tft_update();
					}
				}
				else if(UltrasonicStop==1){
					Stop();
					EverythingDone=1;
				}
				
				//tft_update();
				//------------------------------------------------From here on is for the AUTO MODE - ULTRASONIC SENSOR
        sonar_start();
				
				
        //tft_clear();
				sonar_distance = sonar_get();
				
        //tft_prints(0, 0, "%d", sonar_distance);
        //tft_update();
         
			//---------------------------------The code here is to shake the thrower robot till the ultrasonic senses
			/*
				if(this_ticks>=14000 && sonar_distance==20){//crossed 5th line and the ultrasonic sensor is not sernsing
					led_on(LED1);
					led_on(LED2);
					
					if((get_ticks()-shaking_time)>=50){//Alternating every 50ms
						if(shaking_counter%2==0){
							motor_control(MOTOR1,800,1);
							motor_control(MOTOR3,800,0);
							shaking_counter++;
						}
						else if(shaking_counter%2==1){
							motor_control(MOTOR1,800,0);
							motor_control(MOTOR3,800,1);
						}
					}
				}*/
				//------------------------------------------------------------
        if ((sonar_distance < 80) && (sonar_distance>20) && auto_grabbing_done==0 && EverythingDone==0){//While grabbing and lifting hasn't been done
					if(ultrasonic_stop==0){
						ultrasonic_stopping_time=get_ticks();
						ultrasonic_stop=1;
					}
					if((get_ticks()-ultrasonic_stopping_time)>300){//Will stop 300ms after thrower robot within 80cm
						Stop();
						
						if(stop_grab==0){
							stop_grab_time=get_ticks();
							stop_grab=1;
						}
						
						if((get_ticks()-stop_grab_time)>=600){//take 1 sec before grabbing is done
							if(auto_mode_grabbing==0){
								gpio_set(GPIO2);//Grabbing has been done
								auto_mode_grabbing=1;
								auto_grab_time=get_ticks();
							}
							if((get_ticks()-auto_grab_time)>=2000){//2 seconds after the grabbing has been done
								gpio_set(GPIO3);//Lifting will be done
								auto_grabbing_done=1;
								UltrasonicStop=1;
							}
						}
					}
        }
				else if (sonar_distance > 1000){
          //Forward(300);
        }else if (sonar_distance > 250){
          //Forward(300);
        }
				//else if (sonar_distance < 100){
          //Backward(300);
       // }
				
				if(EverythingDone==1){
					if(stop_back==0){
						stop_back_time=get_ticks();
						stop_back=1;
					}
					
					if((get_ticks()-stop_back_time)>=2000 && (get_ticks()-stop_back_time)<4000){
						//adjust = 45;
						LBackward(800);
						RBackward(600);
					}
					else if((get_ticks()-stop_back_time)>=4000){
						Stop();
					}
				}
        
        if (value_received == 255){
          robot_mode = MANUAL;
        }
      }//--------AUTO MODE Code done here
			
	

			
//------------------------------MANUAL MODE CODE STARTS HERE

			
			
			
				static u8 debounce2=0,debounce3=0;
        if (robot_mode == MANUAL){
					led_off(LED1);
					led_on(LED2);
					//tft_clear();
					//tft_prints(0,4,"Value: %d",value_received);	
					//tft_update();
          if (value_received == 0){
            MStop();
          }else if (value_received <= 50){//Forward Moving
            MForward(value_received*24);
          }else if (value_received <= 100){//Turning Left
            MTurnLeft((value_received - 50)*24);
          }else if (value_received <= 150){//Backward Moving
            MBackward((value_received - 100)*24);
          }else if (value_received <= 200){//Turning Right
            MTurnRight((value_received - 150)*24);
          }else if (value_received == 210){//Shoot
            gpio_set(GPIO1);//Set gpio as high
          }
					
					//----------------------------------Solenoid Valve communication
					
					
					if(value_received!=210){//While button isn't pressed, shooting command doesn't work
						gpio_reset(GPIO1);
					}
					
					
					//-----------------Debouncing for grab and lift actions
		
					//----------------------------For Grabbing
					
					
					if (value_received == 220 && !debounce2){//When initially pressed for grab
						grab_counter++;
						debounce2=1;
					}
					else if(value_received!=220 && debounce2){
						debounce2=0;
					}
					
					
					//----------------------------For lifting
         

					if (value_received == 230 && !debounce3){//When initially pressed for lift
							lift_counter++;
							debounce3=1;
					}
					else if(value_received!=230 && debounce3){
							debounce3=0;
					}
					
					
					//--------Counter System to interchange between on and off
					//Will grab if the signal sent as HIGH
					
					
					if((grab_counter%2)==1){
						gpio_set(GPIO2);
					}
					else if((grab_counter%2)==0){
						gpio_reset(GPIO2);
					}
					
					
					//will lift IF SIGNAL SENT AS HIGH
					if((lift_counter%2)==1){
						gpio_set(GPIO3);
					}
					else if((lift_counter%2)==0){//wILL RELEASE IF SIGNAL SENT AS LOW
						gpio_reset(GPIO3);
					}
        }//---------------MANUAL MODE Code done here
      }//---------------Every 50ms code ends here
    }
  }//While loop ends here        
}