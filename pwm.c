/*******************************************************************************
 * File Name          : pwmLab.c
 * Description        : The code runs 3 leds. 1. linear, 2. sinusoidal and 3. parabolic X times X = X ^ 2
 * 		     Uses 2 interrupt, TIM1 as pwm generator and TIM11 as timer interrupt
 * 
 * Author:              Group 1
 *			 Sinan KARACA
 *			 Mohammed Al Bunde
 * Date:                03.11.2021				 
 ******************************************************************************
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "common.h"
#include "main.h"
#include "math.h"

//Define PI value for sin function
#define PI 3.14159256

//Necessary function for PWM running
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

//Global variable declerations
int32_t channel = 0;
int32_t isChannelEntered = 0;
int32_t pwmValue, pwmValue1 = 0;
int32_t ispwmValueEntered = 0;
uint32_t turning, turning1 = 0;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef tim11;

// Initialise the two timers
// Initialise the GPIO for pwm output of 3 channel
ParserReturnVal_t glow(int action) {

	uint32_t speedPwm;
	uint32_t scalerTimer;
	fetch_uint32_arg(&speedPwm);
    
	
	//Change the speed by manipulating the TIM11 period
	if (speedPwm == 1) {
		scalerTimer = 25000;
	} else if (speedPwm == 2) {
		scalerTimer = 50000;
	} else if (speedPwm == 3) {
		scalerTimer = 100000;
	} else {
		printf("Please write down speed!");
		return CmdReturnBadParameter1;
	}

	__HAL_RCC_TIM1_CLK_ENABLE();
	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_OC_InitTypeDef sConfigOC = { 0 };
	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = { 0 };

	htim1.Instance = TIM1;
	htim1.Init.Prescaler = HAL_RCC_GetPCLK2Freq() / 1000000 - 1;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 1000;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

	__HAL_RCC_TIM11_CLK_ENABLE();
	tim11.Instance = TIM11;
	tim11.Init.Prescaler = HAL_RCC_GetPCLK2Freq() / scalerTimer - 1;
	tim11.Init.CounterMode = TIM_COUNTERMODE_UP;
	tim11.Init.Period = 999;
	tim11.Init.ClockDivision =
	TIM_CLOCKDIVISION_DIV1;
	tim11.Init.RepetitionCounter = 0;

	HAL_TIM_Base_Init(&tim11);

	HAL_NVIC_SetPriority(TIM1_TRG_COM_TIM11_IRQn, 10, 0U);
	HAL_NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);

	HAL_TIM_Base_Start_IT(&tim11);

	if (HAL_TIM_Base_Init(&htim1) != HAL_OK) {
		Error_Handler();
	}

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_Init(&htim1) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1)
			!= HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2)
			!= HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3)
			!= HAL_OK) {
		Error_Handler();
	}
	sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	sBreakDeadTimeConfig.DeadTime = 0;
	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig)
			!= HAL_OK) {
		Error_Handler();
	}

	HAL_TIM_MspPostInit(&htim1);
	
	//Enable 3 channels
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

	return CmdReturnOk;
}


// FUNCTION      : HAL_TIM_MspPostInit()
// DESCRIPTION   : Initialise the 3 output channel pins
// PARAMETERS    : TIM_HandleTypeDef -- Take the TIM1 as input
// RETURNS       : None

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	if (htim->Instance == TIM1) {

		__HAL_RCC_GPIOA_CLK_ENABLE();
		GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	}

}

ADD_CMD("pwmGlow",glow,"glow the leds");

// FUNCTION      : TIM1_TRG_COM_TIM11_IRQHandler()
// DESCRIPTION   : Interrupt handler function
// PARAMETERS    : None
// RETURNS       : None

void TIM1_TRG_COM_TIM11_IRQHandler (void) {

	HAL_TIM_IRQHandler(&tim11);

}

// FUNCTION      : breathingFunc()
// DESCRIPTION   : For interrupt callback function
//                 Manipulate pwmValue and pwmValue1 variables to change pwm duty cycle for 3 channels.
// PARAMETERS    : None
// RETURNS       : None

void breathingFunc(void) {

	if (pwmValue > 254) {
		turning = 1;

	} else if (pwmValue < 2) {
		turning = 0;
	}

	if (turning == 0) {
		pwmValue = pwmValue + 1;

	} else if (turning == 1) {
		pwmValue = pwmValue - 1;
	}

	if (pwmValue1 > 14) {
		turning1 = 1;

	} else if (pwmValue1 < 1) {
		turning1 = 0;

	}

	if (turning1 == 0) {
		pwmValue1 = pwmValue1 + 1;

	} else if (turning1 == 1) {
		pwmValue1 = pwmValue1 - 1;

	}

}

// FUNCTION      : HAL_TIM_PeriodElapsedCallback()
// DESCRIPTION   : It changes the duty cycle percentage for each interrupt cycle
// PARAMETERS    : TIM_HandleTypeDef ---- TIM11 
// RETURNS       : None

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

	float resultChn2;
	int resultChn3;

	if (htim == &tim11) {

		breathingFunc();

		resultChn2 = sin(pwmValue / 255.0 * 3.14159265) * 255;

		resultChn3 = pwmValue1 * pwmValue1;

		htim1.Instance->CCR1 = pwmValue;
		htim1.Instance->CCR2 = resultChn2;
		htim1.Instance->CCR3 = resultChn3;

	}
}
