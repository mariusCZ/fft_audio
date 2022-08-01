/*
 * ws2812.c
 *
 *  Created on: Jun 3, 2022
 *      Author: php21mc
 */

#include "main.h"
#include "ws2812.h"

extern TIM_HandleTypeDef htim1;
extern DMA_HandleTypeDef hdma_tim1_ch1;
extern ledstruc leds[NUM_LEDS];

volatile uint8_t indrst = 0;
volatile uint16_t indby = 1;
uint32_t bcolors = 0;
uint32_t circbuffer[24] = {0};

volatile uint8_t rd_flag = 0;
volatile uint8_t in_flag = 0;

void HAL_TIM_PWM_PulseFinishedHalfCpltCallback(TIM_HandleTypeDef *htim)
{
	if(indby < NUM_LEDS) {
		bcolors = (leds[indby].g << 16) | (leds[indby].r << 8) | leds[indby].b;
		for(uint8_t i = 23; i > 11; i--) {
			if(bcolors & (1<<i)) circbuffer[23-i] = HT;
			else circbuffer[23-i] = LT;
		}
	}
	else {
		for(uint8_t i = 23; i > 11; i--)
			circbuffer[23-i] = 0;
	}
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	if(indby < NUM_LEDS) {
		for(uint8_t i = 11; i > 0; i--) {
			if(bcolors & (1<<i)) circbuffer[23-i] = HT;
			else circbuffer[23-i] = LT;
		}
		if(bcolors & (1<<0)) circbuffer[23] = HT;
		else circbuffer[23] = LT;
		indby++;
	}
	else {
		if(!indrst) {
			for(uint8_t i = 11; i > 0; i--)
				circbuffer[23-i] = 0;
			circbuffer[23] = 0;
		}
		if(indrst >= 2) {
			HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_1);
			rd_flag = 0;
			indby = 1;
			indrst = 0;
		}
		else
			indrst++;
	}
}

uint8_t ws2812_show()
{
	switch(0 ^ in_flag ^ (rd_flag<<1)) {
	case 0:
		in_flag = 1;
		rd_flag = 1;
		bcolors = (leds[0].g << 16) | (leds[0].r << 8) | leds[0].b;
		for(uint8_t i = 23; i > 0; i--) {
			if(bcolors & (1<<i)) circbuffer[23-i] = HT;
			else circbuffer[23-i] = LT;
		}
		if(bcolors & (1<<0)) circbuffer[23] = HT;
		else circbuffer[23] = LT;
		HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, circbuffer, 24);
		return 0;
		break;
	case 3:
		return 0;
		break;
	case 1:
		in_flag = 0;
		return 1;
		break;
	case 2:
		rd_flag = 0;
		return 0;
		break;
	default:
		return 0;
	}
}
