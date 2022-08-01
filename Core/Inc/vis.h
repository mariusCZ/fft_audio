/*
 * vis.h
 *
 *  Created on: Jun 16, 2022
 *      Author: marius
 */

#ifndef INC_VIS_H_
#define INC_VIS_H_

#include "ws2812.h"

#define DECAY_RATE_MAX 8
#define DECAY_RATE_MIN 2
#define DECAY_RATE 4
#define NFLR 0
#define CLNG 245
#define COL_COEFF_MIN 0.7

#define SAMEWL 0
#if !SAMEWL

#if OCT_N == 8

#define OCTP_0 0.4
#define OCTP_1 0.2
#define OCTP_2 0.1
#define OCTP_3 0.125
#define OCTP_4 0.125
#define OCTP_5 0.1
#define OCTP_6 0.1
#define OCTP_7 0.1

#elif OCT_N == 6

#define OCTP_0 0.4
#define OCTP_1 0.3
#define OCTP_2 0.2
#define OCTP_3 0.2
#define OCTP_4 0.1
#define OCTP_5 0.1

#endif

void mapWidthx2(uint8_t widthArr[], uint16_t widthLen, uint16_t input);

#else

void mapWidthx2(uint8_t widthArr[], uint16_t input);

#endif

typedef struct coeffstruc{
	float RCOEFF;
	float GCOEFF;
	float BCOEFF;
} coeffstruc;

void visRoutine1(uint16_t oct[], ledstruc leds[], uint8_t col, uint8_t colCoeff);
void visRoutine2(uint16_t oct[], ledstruc leds[]);
void abslToRGB(ledstruc leds[], uint8_t col, uint8_t colCoeff);
void abslToRGBrnbw(ledstruc leds[]);
void reverseOrder(ledstruc leds[]);
uint16_t decayFun(uint8_t val);
uint16_t sumP(uint16_t arr[]);
float mapLogCol(uint8_t colCoeff);
uint16_t mapLin(uint16_t x, uint16_t out_max);
void initLED();

#endif /* INC_VIS_H_ */
