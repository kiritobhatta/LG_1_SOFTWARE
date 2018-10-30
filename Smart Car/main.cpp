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

int main() {
    // Initialize Everything Here
    rcc_init();
    ticks_init();
    
    uint32_t lastticks=get_ticks();
    while(1){
        if(lastticks!=get_ticks()){
            lastticks=get_ticks();
            if (!(lastticks%250)){
                //code here will run every 250 ms
                
            }
        }
    }
}


