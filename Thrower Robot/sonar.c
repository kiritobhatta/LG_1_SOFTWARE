#include "sonar.h"

#include "stm32f10x_tim.h"
#include "stdio.h"
#include "misc.h"

volatile uint16_t SonarValue;

void SetSysClockTo72(void)
{
	ErrorStatus HSEStartUpStatus;
    /* SYSCLK, HCLK, PCLK2 and PCLK1 configuration -----------------------------*/
    /* RCC system reset(for debug purpose) */
    RCC_DeInit();

    /* Enable HSE */
    RCC_HSEConfig( RCC_HSE_ON);

    /* Wait till HSE is ready */
    HSEStartUpStatus = RCC_WaitForHSEStartUp();

    if (HSEStartUpStatus == SUCCESS)
    {
        /* Enable Prefetch Buffer */
    	//FLASH_PrefetchBufferCmd( FLASH_PrefetchBuffer_Enable);

        /* Flash 2 wait state */
        //FLASH_SetLatency( FLASH_Latency_2);

        /* HCLK = SYSCLK */
        RCC_HCLKConfig( RCC_SYSCLK_Div1);

        /* PCLK2 = HCLK */
        RCC_PCLK2Config( RCC_HCLK_Div1);

        /* PCLK1 = HCLK/2 */
        RCC_PCLK1Config( RCC_HCLK_Div2);

        /* PLLCLK = 8MHz * 9 = 72 MHz */
        RCC_PLLConfig(0x00010000, RCC_PLLMul_9);

        /* Enable PLL */
        RCC_PLLCmd( ENABLE);

        /* Wait till PLL is ready */
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
        {
        }

        /* Select PLL as system clock source */
        RCC_SYSCLKConfig( RCC_SYSCLKSource_PLLCLK);

        /* Wait till PLL is used as system clock source */
        while (RCC_GetSYSCLKSource() != 0x08)
        {
        }
    }
    else
    { /* If HSE fails to start-up, the application will have wrong clock configuration.
     User can add here some code to deal with this error */

        /* Go to infinite loop */
        while (1)
        {
        }
    }
}

void sonar_init() {
	GPIO_InitTypeDef gpio_cfg;
	GPIO_StructInit(&gpio_cfg);

	/* Timer TIM2 enable clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	/* Timer TIM2 settings */
	TIM_TimeBaseInitTypeDef timer_base;
	TIM_TimeBaseStructInit(&timer_base);
	timer_base.TIM_CounterMode = TIM_CounterMode_Up;
	timer_base.TIM_Prescaler = 72;
	TIM_TimeBaseInit(TIM2, &timer_base);
	TIM_Cmd(TIM2, ENABLE);

	//Trigger Pin
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	gpio_cfg.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio_cfg.GPIO_Pin = GPIO_Pin_11; //connect Trig pin to servo 2 output pin
	GPIO_Init(GPIOB, &gpio_cfg);

	//EXTI

	/* Set variables used */
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;

	/* Enable clock for AFIO */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	/* Set pin as input */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10; //connect Echo pin to servo 3 output pin
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* Add IRQ vector to NVIC */
	/* PB0 is connected to EXTI_Line10, which has EXTI15_10_IRQn vector */
	NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
	/* Set priority */
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	/* Set sub priority */
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
	/* Enable interrupt */
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	/* Add to NVIC */
	NVIC_Init(&NVIC_InitStruct);

	/* Tell system that you will use PB10 for EXTI_Line10 */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource10);

	/* PB10 is connected to EXTI_Line10 */
	EXTI_InitStruct.EXTI_Line = EXTI_Line10;
	/* Enable interrupt */
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	/* Interrupt mode */
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	/* Triggers on rising and falling edge */
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	/* Add to EXTI */
	EXTI_Init(&EXTI_InitStruct);
}

/* Comment the EXTI15_10_IRQHandler(void) in camera.c */
void EXTI15_10_IRQHandler(void) {
	/* Make sure that interrupt flag is set */
	if (EXTI_GetITStatus(EXTI_Line10) != RESET) {
		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) != 0) {
			// Rising
			TIM_SetCounter(TIM2, 0);
		}
		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == 0) {
			// Falling
			SonarValue = TIM_GetCounter(TIM2);
		}

		/* Clear interrupt flag */
		EXTI_ClearITPendingBit(EXTI_Line10);
	}
}

void sonar_start() {
	int i;

	GPIO_SetBits(GPIOB, GPIO_Pin_11);
	//Delay 0x72000
	for(i=0;i<0x7200;i++);
	GPIO_ResetBits(GPIOB, GPIO_Pin_11);
}

u32 sonar_get() {
	u32 Sonar;
	// 354000 - Sound speed (mm/sec)
	// 72000000 - F_CPU
	// 16 - Timer Prescaler
	// Result = mm
	Sonar = (354/2) * (u32)SonarValue / (72000 / 72);
	if (Sonar > 4000) Sonar = 4000;
	if (Sonar < 20) Sonar = 20;

	return (u32)Sonar;
}
