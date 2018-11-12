#ifndef __SONAR_H
#define __SONAR_H

#include "stm32f10x_tim.h"

/** 
 * @brief set system clock 
 */
void SetSysClockTo72(void);
	
/**
 * @brief initialize ultrasonic sensor
 */	
void sonar_init(void);

/**
 * @brief start the sonar by triggering signal to the Trig pin
 */
void sonar_start(void);

/** 
 * @brief return ultrasonic sensor value
 */
u32 sonar_get(void);

#endif
