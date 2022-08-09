/*
 * bt.c
 *
 *  Created on: Jul 1, 2022
 *      Author: marius
 */

#include "main.h"
#include "bt.h"
#include "stdlib.h"

extern UART_HandleTypeDef huart1;

uint8_t rbuf[1] = {0}, valbuf = 0;
uint16_t dflag = 0;
uint16_t numbuf = 0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	static uint16_t i = 0;
	if(i == 0) {
		valbuf = rbuf[0];
		i++;
	}
	else if(i == 1) {
		numbuf = rbuf[0];
		i++;
	}
	else if(i >= 2 && rbuf[0] == '\n') {
		i = 0;
		dflag = 1;
	}
}

uint8_t getBT(uint8_t *val)
{
	static uint16_t iflag = 0;
	if(!iflag) {
		HAL_UART_Receive_DMA(&huart1, rbuf, 1);
		iflag = 1;
	}
	if(dflag) {
		*val = valbuf;
		dflag = 0;
		uint16_t ret = numbuf;
		return ret;
	}
	else {
		*val = 0;
		return 0;
	}
}
