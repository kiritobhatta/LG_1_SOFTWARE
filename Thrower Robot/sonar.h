#ifndef __SONAR_H
#define __SONAR_H

#include "stm32f10x_tim.h"

extern volatile u8 FLAG_ECHO;

/** 
 * @brief set system clock 
 */
void SetSysClockTo72(void);
	
/**
 * @brief initialize ultrasonic sensor
 */	
void sonar_init(void);

void sonar_start(void);

/** 
 * @brief return ultrasonic sensor value
 */
u32 sonar_get(void);

void usart_init(void);

void USARTSend(char *pucBuffer);

#endif
