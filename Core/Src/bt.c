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

uint8_t buf[BUF_SIZE] = {0};
uint8_t rbuf[1] = {0}, valbuf = 0;
uint16_t dflag = 0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	static uint16_t i = 0;
	if(rbuf[0] >= '0' && rbuf[0] <= '9'){
		buf[i] = rbuf[0];
		i++;
	}
	else if(rbuf[0] != '\n' && rbuf[0] != '\r' && rbuf[0] != '\\' && rbuf[0] != 'n')
		valbuf = rbuf[0];
	else if(rbuf[0] == '\n' || rbuf[0] == '\\') {
		i = 0;
		dflag = 1;
	}
	else if(rbuf[0] == 'n') ;
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
		uint16_t ret = atoi(buf);
		for(uint16_t i = 0; i < BUF_SIZE; i++) buf[i] = 0;
		return ret;
	}
	else {
		*val = 0;
		return 0;
	}
}
