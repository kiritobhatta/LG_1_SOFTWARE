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
#include "LineSensor.h"

typedef enum{AUTO, MANUAL}OperationMode;

u8 value_received; //value from Bluetooth connection
void UARTOnReceiveHandler(const u8 received){
  value_received = received;
}

int main() {
    // Initialize Everything Here
  rcc_init();
  ticks_init();
  SetSysClockTo72();
  
  tft_init(PIN_ON_TOP, BLACK, WHITE, RED, YELLOW);
  sonar_init();
  
  leds_init();
  buttons_init();
  uart_init(COM3,9600);
  motor_init(MOTOR1, 11, 1200, 0, 1);
  motor_init(MOTOR2, 11, 1200, 0, 1);
  
  uart_rx_init(COM3,&UARTOnReceiveHandler);  
  uint32_t lastticks=get_ticks();
  static u32 sonar_distance = 0;
  
  OperationMode robot_mode;
  
  while(1){
    if(lastticks!=get_ticks()){
      lastticks=get_ticks();
      if (!(lastticks%50)){
        //code here will run every 50
        if (robot_mode == MANUAL){
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
      if (!(lasticks%250)){
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
