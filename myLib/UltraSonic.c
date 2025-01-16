/*
 * UltraSonic.c
 *
 *  Created on: Jan 16, 2025
 *      Author: user
 */

#include "main.h"
#include "C:\Users\user\STM32Cube\Repository\STM32Cube_FW_F4_V1.28.1\Drivers\STM32F4xx_HAL_Driver\Inc\stm32f4xx_hal_tim.h"
// TIM is not default device > can't use "extern" in general-purpose
// for later use of "TIM_HandleTypeDef"

// extern TIM_HandleTypeDef *htim;
TIM_HandleTypeDef *htim = NULL;
// If TIM is used, *htim will be re-defined in "main.c".
// Otherwise, *htim will be remained as NULL.


// int TIM_init(TIM_HandleTypeDef *p)
void TIM_init(void *p)
{
	htim = p;
}

// In main.c, call like this : TIM_init(&htim2) for Usage Timer2
// If TIM is not used, this function does nothing.
// if(htim == NULL) return;

void Micro_Delay(int us)// us : micro-sec (10^-6)
{
	// Assume Internal CLK Source : 84 MHz
	// Set Timer Parameter : PSC = (84 - 1) & ARR = Max. (To avoid CNT Register Overflow)
	// Then, Timer counts per 1 us.

	// Consider Overflow Case
	htim->Instance->CNT = 0;
	while( htim->Instance->CNT < us );
}

/*
void Trigger()
{
	// Assume that there is "Trig" GPIO Pin

	// *** Trigger Pulse Generation *** //
	// HAL_GPIO_WritePin(Pin Group, Pin#, Pin State(H/L));
	HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, 0); // Initial Reset
	Micro_Delay(10); // 10us Delay
	// void Micro_Delay(int us)

	HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, 1); // Trigger Pulse High
	Micro_Delay(10); // 10us Delay

	HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, 0); // Reset
	// *** Trigger Pulse Generation *** //
}
*/
