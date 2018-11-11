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


#define LMotor MOTOR1
#define RMotor MOTOR2

void LForward(u16 a){motor_control(LMotor, a, 1);} // Left motor turing forward
void RForward(u16 a){motor_control(RMotor, a, 1);} // Right motor turing forward
void LBackward(u16 a){motor_control(LMotor, a, 0);} // Left motor turing backward
void RBackward(u16 a){motor_control(RMotor, a, 0);} // Right motor turing backward

void Forward (u16 speed){LForward(speed);RForward(speed);} //Car move forward
void Backward (u16 speed){LBackward(speed);RBackward(speed);} //Car move backward
void TurnLeft (u16 speed){LBackward(speed);RForward(speed);} //Car turn left
void TurnRight (u16 speed){LForward(speed);RBackward(speed);} //Car turn right
void Stop(){motor_control(LMotor, 0, 1);motor_control(RMotor, 0, 1);} //Stop the car

u32 value_received; //value from Bluetooth connection
void UARTOnReceiveHandler(const u8 received){
  value_received = received;
}

int main() {
    // Initialize Everything Here
    SetSysClockTo72();

    tft_init(PIN_ON_TOP, WHITE, BLACK, RED, YELLOW);
    sonar_init();
	
    rcc_init();
    ticks_init();
    
    leds_init();
    buttons_init();	
    uart_init(COM3,38400);
    motor_init(MOTOR1, 11, 1200, 0, 1);
    motor_init(MOTOR2, 11, 1200, 0, 1);
    
    uart_rx_init(COM3,&UARTOnReceiveHandler);  
    uint32_t lastticks=get_ticks();
    while(1){
        if(lastticks!=get_ticks()){
            lastticks=get_ticks();
            if (!(lastticks%50)){
              //code here will run every 50 ms
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
              }

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
}

