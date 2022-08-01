/*
 * sampling.c
 *
 *  Sampling done by DMA + Timer. This is set so that the timer triggers ADC
 *  read at 40 kHz.
 */

#include "main.h"
#include "sampling.h"
#include "output.h"

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim3;

uint16_t samples[FFT_SIZE];
uint16_t AD_RES = 0;
uint8_t read_flag = 0;
uint8_t init_flag = 0;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    // Buffer filled with samples callback, stop DMA and timer.
	read_flag = 0;
	HAL_ADC_Stop_DMA(&hadc1);
	HAL_TIM_Base_Stop(&htim3);
}

uint8_t adc_read(uint16_t buf[])
{
	// State machine like code to ensure continous program operation.
	switch(0 ^ init_flag ^ (read_flag<<1)) {
	case 0:
		// Initialize ADC with DMA and timer.
		init_flag = 1;
		read_flag = 1;
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*)samples, FFT_SIZE);
		HAL_TIM_Base_Start(&htim3);
		return 0;
		break;
	case 3:
		// Do nothing, samples still reading.
		return 0;
		break;
	case 1:
		// Sample reading done (callback called).
		init_flag = 0;
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
		for(uint16_t i = 0; i < FFT_SIZE; i++) {
			// Samples need a shift to the left in hex so by 4 (to q15 format)
			buf[i] = ((samples[i] + 40) << 4) ^ 0x8000;
			// Show if the signal is clipping
			if(samples[i] < LCLIP) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
		}
		return 1;
		break;
	case 2:
		// Error state, should not reach this
		//output("Something went wrong with sample reading\r\n");
		read_flag = 0;
		return 0;
		break;
	default:
		return 0;
	}
}
