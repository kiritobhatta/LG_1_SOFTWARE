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
#include "sonar.h"
#include "MotorControl.h"
#include "LineSensor.h"

typedef enum{AUTO, MANUAL}Operation_Mode;

u32 value_received; //value from Bluetooth connection
void UARTOnReceiveHandler(const u8 received){
  value_received = received;
}

int main() {
    // Initialize Everything Here
    char buffer[80] = {'\0'};

    SetSysClockTo72();

    // TIMER4 Двічи за секунду викликає sonar_start(); і встановлює FLAG_ECHO = 1;
    TIM_TimeBaseInitTypeDef TIMER_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    TIM_TimeBaseStructInit(&TIMER_InitStructure);
    TIMER_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIMER_InitStructure.TIM_Prescaler = 7200;
    TIMER_InitStructure.TIM_Period = 5000;
    TIM_TimeBaseInit(TIM4, &TIMER_InitStructure);
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM4, ENABLE);

    /* NVIC Configuration */
    /* Enable the TIM4_IRQn Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    usart_init();
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
		int sonar_distance;
		
		Operation_Mode robot_mode;
		
    while(1){
        if(lastticks!=get_ticks()){
            lastticks=get_ticks();
            if (!(lastticks%50)){
              //code here will run every 50 ms
              
              //output the distance from the object to the ultrasonic sensor on tft in mm
              if (FLAG_ECHO == 1){
                tft_clear();
                sonar_distance = sonar_get();
                tft_prints(0,0,"%d",sonar_distance);
                USARTSend(buffer);
                FLAG_ECHO = 0;
                tft_update();
              }
              
              if (robot_mode == AUTO){
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