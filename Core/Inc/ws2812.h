/*
 * ws2812.h
 *
 *  Created on: Jun 3, 2022
 *      Author: php21mc
 */

#ifndef INC_WS2812_H_
#define INC_WS2812_H_

#define NUM_LEDS 166
#define HT 65
#define LT 25

typedef struct ledstruc{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t abslt;
} ledstruc;

uint8_t ws2812_show();

#endif /* INC_WS2812_H_ */
