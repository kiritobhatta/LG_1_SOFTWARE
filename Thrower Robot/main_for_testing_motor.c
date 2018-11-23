// Include Library Headers Here
#include "rcc.h"
#include "ticks.h"
#include "gpio.h"
#include "leds.h"
#include "buttons.h"
#include "buzzer.h"
#include "uart.h"
#include "lcd_main.h"
#include "oled.h"
#include "pwm.h"

int main() {
 // Initialize Everything Here
  static u16 motor = 0;
	
	//change the initial direction here
	static u8 direction_one = 0;
	static u8 direction_two = 1;
	rcc_init();
  ticks_init();
	leds_init();
  buttons_init();
  gpio_init(GPIO1, GPIO_Mode_Out_PP);
	gpio_init(GPIO2, GPIO_Mode_Out_PP);
	gpio_init(GPIO3, GPIO_Mode_Out_PP);
	motor_init(MOTOR1, 6, 1200, motor, direction_one);
	motor_init(MOTOR3, 6, 1200, motor, direction_two);//Hard code to adjust speed
	tft_init(PIN_ON_TOP, BLACK, WHITE, RED, YELLOW);
	
  while (1) {
   static u32 this_ticks = 0;
		while (get_ticks() == this_ticks);
		this_ticks = get_ticks();

		// Everything from here will run every 1ms

		static u32 last_led_ticks=0;
		if ((this_ticks - last_led_ticks) >= 50) {
			last_led_ticks = this_ticks;
			//Code in here will run every 50ms
			static u8 button_check_1 = 0;
			static u8 button_check_2 = 0;
			static u8 button_check_3 = 0;
			
      
      //button one increase the auto reload value, decrease the speed
      if ((button_pressed(BUTTON1))&&(button_check_1 == 0)){
			  button_check_1 = 1;
			}else if (!(button_pressed(BUTTON1)) && (button_check_1 == 1)){
				button_check_1 = 0;
			  motor += 100;
				motor_control(MOTOR1, motor, direction_one);
				motor_control(MOTOR3, motor, direction_two);
			}
			
      
      //button two decrease the auto reload value, increase the speed
			if ((button_pressed(BUTTON2))&&(button_check_2 == 0)){
			  button_check_2 = 1;
			}else if (!(button_pressed(BUTTON2)) && (button_check_2 == 1)){
				button_check_2 = 0;
			  motor -= 100;
				motor_control(MOTOR1, motor, direction_one);
				motor_control(MOTOR3, motor, direction_two);
			}
			
      
      //button three change direction
			if ((button_pressed(BUTTON3))&&(button_check_3 == 0)){
			  button_check_3 = 1;
			}else if (!(button_pressed(BUTTON3)) && (button_check_3 == 1)){
				button_check_3 = 0;
			  if (direction_one == 0){
					direction_one = 1;
					direction_two = 0;
					motor_control(MOTOR1, motor, direction_one);
				  motor_control(MOTOR3, motor, direction_two);
				}else if (direction_one == 1){
					direction_one = 0;
					direction_two = 1;
				  motor_control(MOTOR1, motor, direction_one);
				  motor_control(MOTOR3, motor, direction_two);
				}
			}
			
      
      //print to tft
			tft_clear();
      tft_prints(0, 0, "M 1: %d Dir: %d", motor, direction_one);
      tft_prints(0, 3, "M 3: %d Dir: %d", motor, direction_two);
      tft_update();
			
      
      //testing valve below
			/*
			
			
			
			if (button_pressed(BUTTON1)){
			  gpio_set(GPIO1);
			}else {
			  gpio_reset(GPIO1);
			}
			
			if (button_pressed(BUTTON2)){
			  gpio_set(GPIO2);
			}else {
			  gpio_reset(GPIO2);
			}
			
			if (button_pressed(BUTTON3)){
			  gpio_set(GPIO3);
			}else {
			  gpio_reset(GPIO3);
			}
			*/
		}
 }
}
