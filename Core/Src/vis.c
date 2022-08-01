/*
 * vis.c
 *
 *  Code for visualizing the FFT results on the WS2812 strip.
 */

#include "main.h"
#include "vis.h"
#include "math.h"

// Code kept for now, will likely be deleted later on.
#if SAMEWL == 1

const uint16_t ledProp = ((uint16_t)(NUM_LEDS/(OCT_N*4))%2) ? (uint16_t)(NUM_LEDS/(OCT_N*4)+1) : (uint16_t)(NUM_LEDS/(OCT_N*4));

void visRoutine1(uint16_t oct[], ledstruc leds[])
{
	static uint16_t oct_d[OCT_N] = {0};

	for(uint16_t i = 0; i < OCT_N; i++) {
		oct_d[i] = (oct_d[i] < DECAY_RATE) ? 0 : (oct_d[i] - DECAY_RATE);
		oct_d[i] = (oct[i+1] > oct_d[i]) ? oct[i+1] : oct_d[i];
	}

	uint8_t widthArr[ledProp];
	mapWidthx2(widthArr, oct_d[0]);
	for(uint16_t i = NUM_LEDS/2-1; i > (NUM_LEDS/2-1)-ledProp; i--)
		leds[i].abslt = widthArr[ (NUM_LEDS/2-1)-i ];
	for(uint8_t i = 1; i < OCT_N; i++) {
		mapWidthx2(widthArr, oct_d[i]);
		for(uint16_t j = 0; j < ledProp; j++) {
			leds[ (NUM_LEDS/2-1)-i*ledProp*2-j ].abslt = widthArr[j];
			leds[ (NUM_LEDS/2)-i*ledProp*2+j ].abslt = leds[ (NUM_LEDS/2-1)-i*ledProp*2-j ].abslt;
		}
	}
	for(uint16_t i = 0; i < NUM_LEDS/2; i++)
		leds[NUM_LEDS-1-i].abslt = leds[i].abslt;
	abslToRGB(leds);
}

void mapWidthx2(uint8_t widthArr[], uint16_t input)
{
	input = (input > CLNG) ? CLNG : input;
	float a = (float)(255*ledProp) / (CLNG*CLNG);
	uint32_t width = a*(input*input);

	for(uint16_t i = 0; i < ledProp; i++) {
		volatile float prop = (ledProp-(float)i)/ledProp;
		widthArr[i] = ( width*prop > 255 ) ? 255 : width*prop*prop;
	}
}

#else

// Where the amount of LEDs for each frequency is kept.
uint16_t ledProps[OCT_N];
// Colors for each frequency for visRoutine2
ledstruc colDist[OCT_N];
// Color coefficients for visRoutine3
coeffstruc coeffDist[OCT_N];

/*
 * The first visualization routine. This basically shows the
 * lowest frequency amplitude in the middle, with LEDs spreading
 * from center to sides. Same applies to the sides, but for
 * increasing frequencies.
 */
void visRoutine1(uint16_t oct[], ledstruc leds[], uint8_t col, uint8_t colCoeff)
{
	// Array to keep track of decay.
	static uint16_t oct_d[OCT_N] = {0}, state = 0;

	for(uint16_t i = 0; i < OCT_N; i++) {
		// Make sure decay does not overflow.
		oct_d[i] = (oct_d[i] < decayFun(oct_d[i])) ? 0 : (oct_d[i] - decayFun(oct_d[i]));
		// Change decay array value only if current FFT value is larger
		oct_d[i] = (oct[i] > oct_d[i]) ? oct[i] : oct_d[i];
	}

	uint8_t widthArr[ledProps[0]];
	// Map the magnitude of frequency to amoount of LEDs to turn on and their color.
	mapWidthx2(widthArr, ledProps[0], oct_d[0]);
	// Values stored in abslt, which is then converted to RGB.
	for(uint16_t i = NUM_LEDS/2-1; i > (NUM_LEDS/2-1)-ledProps[0]; i--)
		leds[i].abslt = widthArr[ (NUM_LEDS/2-1)-i ];
	// Do the same for the rest of the strip.
	int16_t pos = (NUM_LEDS/2-1) - ledProps[0];
	for(uint8_t i = 1; i < OCT_N; i++) {
		mapWidthx2(widthArr, ledProps[i], oct_d[i]);
		for(uint16_t j = 0; j < ledProps[i]; j++) {
			if( (pos-ledProps[i]+1+j)>= 0 )
				leds[ pos-ledProps[i]+1+j ].abslt = widthArr[j];
			if(pos-ledProps[i]-j >= 0)
				leds[ pos-ledProps[i]-j ].abslt = widthArr[j];
		}
		// Keep track of position where LEDs are turned on.
		pos = ( (pos - ledProps[i]*2)>= 0 ) ? pos - ledProps[i]*2 : 0;
	}

	if(col == 'M') state = !state;
	if(state)
		reverseOrder(leds);

	// Symmetrically show the FFT and convert abslt to RGB.
	for(uint16_t i = 0; i < NUM_LEDS/2; i++)
		leds[NUM_LEDS-1-i].abslt = leds[i].abslt;
	abslToRGB(leds, col, colCoeff);
	//abslToRGBrnbw(leds);
}

void visRoutine2(uint16_t oct[], ledstruc leds[])
{
	// Array to keep track of decay.
	static uint16_t oct_d[OCT_N] = {0};

	for(uint16_t i = 0; i < OCT_N; i++) {
		// Make sure decay does not overflow.
		oct_d[i] = (oct_d[i] < decayFun(oct_d[i])) ? 0 : (oct_d[i] - decayFun(oct_d[i]));
		// Change decay array value only if current FFT value is larger
		oct_d[i] = (oct[i] > oct_d[i]) ? oct[i] : oct_d[i];
	}

	uint8_t widthArr[NUM_LEDS/2];

	for(int16_t i = OCT_N-1; i >= 0; i--) {
		if(!i || i == 1) {
			for(uint16_t j = 0; j < NUM_LEDS/2; j++) widthArr[j] = 0;
			mapWidthx2(widthArr, NUM_LEDS/4, oct_d[i]);
		}
		else mapWidthx2(widthArr, NUM_LEDS/2, oct_d[i]);
		for(uint16_t j = NUM_LEDS/2; j < NUM_LEDS; j++) {
			if(i == OCT_N-1 || (i < OCT_N-1 && widthArr[j-NUM_LEDS/2] > 1)) {
				leds[j].r = mapLin(widthArr[j-NUM_LEDS/2], colDist[i].r);
				leds[j].g = mapLin(widthArr[j-NUM_LEDS/2], colDist[i].g);
				leds[j].b = mapLin(widthArr[j-NUM_LEDS/2], colDist[i].b);
				leds[NUM_LEDS-j-1] = leds[j];
			}
			else break;
		}
	}
}

void mapWidthx2(uint8_t widthArr[], uint16_t widthLen, uint16_t input)
{
	input = (input > CLNG) ? CLNG : input;
	// Map magnitude to a 2^x function.
	float a = logf(255*widthLen)/ (CLNG*logf(2));
	uint32_t width = powf(2, a*input) - 1;

	for(uint16_t i = 0; i < widthLen; i++) {
		volatile float prop = (widthLen-(float)i)/widthLen;
		// Set the width array. prop^2 done because WS2812 brightness is weird.
		widthArr[i] = ( width*prop > 255 ) ? 255 : width*prop*prop;
	}
}

#endif


void abslToRGB(ledstruc leds[], uint8_t col, uint8_t colCoeff)
{
	static float RCOEFF = 0, GCOEFF = 0.99, BCOEFF = 0.9;

	// Set the color coefficients from Bluetooth received values
	switch(col) {
	case 'B':
		RCOEFF = 0;
		GCOEFF = mapLogCol(colCoeff);
		BCOEFF = 1.7 - GCOEFF;
		break;
	case 'C':
		RCOEFF = mapLogCol(colCoeff);
		GCOEFF = 0;
		BCOEFF = 1.7 - RCOEFF;
		break;
	case 'D':
		RCOEFF = mapLogCol(colCoeff);
		GCOEFF = 1.7 - RCOEFF;
		BCOEFF = 0;
		break;
	default:
		break;
	}

	// Make sure main color turns on first and only then other colors follow.
	for(uint16_t i = 0; i < NUM_LEDS; i++) {
		leds[i].r = (leds[i].abslt > 255*RCOEFF) ? leds[i].abslt-255*RCOEFF : 0;
		leds[i].g = (leds[i].abslt > 255*GCOEFF) ? leds[i].abslt-255*GCOEFF : 0;
		leds[i].b = (leds[i].abslt > 255*BCOEFF) ? leds[i].abslt-255*BCOEFF : 0;
	}
}

//void abslToRGBrnbw(ledstruc leds[])
//{
//	for(uint16_t i = NUM_LEDS/2-1; i > (NUM_LEDS/2-1)-ledProps[0]; i--) {
//		leds[i].r = (leds[i].abslt > 255*coeffDist[0].RCOEFF) ? leds[i].abslt-255*coeffDist[0].RCOEFF : 0;
//		leds[i].g = (leds[i].abslt > 255*coeffDist[0].GCOEFF) ? leds[i].abslt-255*coeffDist[0].GCOEFF : 0;
//		leds[i].b = (leds[i].abslt > 255*coeffDist[0].BCOEFF) ? leds[i].abslt-255*coeffDist[0].BCOEFF : 0;
//	}
//	int16_t pos = (NUM_LEDS/2-1) - ledProps[0];
//	for(uint8_t i = 1; i < OCT_N; i++) {
//		for(uint16_t j = 0; j < ledProps[i]; j++) {
//			if( (pos-ledProps[i]+1+j)>= 0 ) {
//				leds[pos-ledProps[i]+1+j].r = (leds[pos-ledProps[i]+1+j].abslt > 255*coeffDist[i].RCOEFF) ?
//						leds[pos-ledProps[i]+1+j].abslt-255*coeffDist[i].RCOEFF : 0;
//				leds[pos-ledProps[i]+1+j].g = (leds[pos-ledProps[i]+1+j].abslt > 255*coeffDist[i].GCOEFF) ?
//						leds[pos-ledProps[i]+1+j].abslt-255*coeffDist[i].GCOEFF : 0;
//				leds[pos-ledProps[i]+1+j].b = (leds[pos-ledProps[i]+1+j].abslt > 255*coeffDist[i].BCOEFF) ?
//						leds[pos-ledProps[i]+1+j].abslt-255*coeffDist[i].BCOEFF : 0;
//			}
//			if(pos-ledProps[i]-j >= 0) {
//				leds[pos-ledProps[i]-j].r = (leds[pos-ledProps[i]-j].abslt > 255*coeffDist[i].RCOEFF) ?
//						leds[pos-ledProps[i]-j].abslt-255*coeffDist[i].RCOEFF : 0;
//				leds[pos-ledProps[i]-j].g = (leds[pos-ledProps[i]-j].abslt > 255*coeffDist[i].GCOEFF) ?
//						leds[pos-ledProps[i]-j].abslt-255*coeffDist[i].GCOEFF : 0;
//				leds[pos-ledProps[i]-j].b = (leds[pos-ledProps[i]-j].abslt > 255*coeffDist[i].BCOEFF) ?
//						leds[pos-ledProps[i]-j].abslt-255*coeffDist[i].BCOEFF : 0;
//			}
//		}
//		pos = ( (pos - ledProps[i]*2)>= 0 ) ? pos - ledProps[i]*2 : 0;
//	}
//	for(uint16_t i = 0; i < NUM_LEDS/2; i++)
//		leds[NUM_LEDS-1-i] = leds[i];
//}

void reverseOrder(ledstruc leds[])
{
	ledstruc swpbuf;
	for(uint16_t i = 0; i < NUM_LEDS*((float)1/4); i++) {
		volatile uint16_t ind = NUM_LEDS/2-i-1;
		swpbuf = leds[ind];
		leds[ind] = leds[i];
		leds[i] = swpbuf;
	}
}

float mapLogCol(uint8_t colCoeff)
{
	const float a = 0.3 / logf(255);
	return a*logf(colCoeff+1)+0.7;
}

uint16_t mapLin(uint16_t x, uint16_t out_max) {
  return x * out_max / 255;
}

uint16_t decayFun(uint8_t val)
{
	// Decay mapped to a 2^x function. Makes decay look smooth.
	static const float a = logf(DECAY_RATE_MAX)/(260*logf(2));
	return powf(2, a*val) + DECAY_RATE_MIN - 1;
}

// Helper sum function.
uint16_t sumP(uint16_t arr[])
{
    uint16_t sum = 0;
    for(uint16_t i = 0; i < OCT_N; i++) {
        if(!i)
            sum += arr[i];
        else sum += arr[i] * 2;
    }

    return sum;
}

/*
 * This function takes the amount of LEDs and distributes them.
 * Lower frequencies are giver larger weight.
 */
void initLED()
{
    uint16_t sum = 0;

    // Distribute LEDs equally.
    for(uint16_t i = 0; i < OCT_N; i++)
        ledProps[i] = NUM_LEDS/(4*OCT_N);
    sum = sumP(ledProps);

    /*
    * LEDs can't always be equally distributed, so redistribute
    * first and last elements, to ensure distribution has the same
    * amount of LEDs as defined.
    */
    if(sum > NUM_LEDS/2) {
        if((NUM_LEDS/2)%2 ^ ledProps[0]%2) {
            ledProps[OCT_N-1] -= (sum-NUM_LEDS/2)/2;
            ledProps[0] -= 1;
        }
        else ledProps[OCT_N-1] -= (sum-NUM_LEDS/2)/2;
    }
    else if(sum < NUM_LEDS/2) {
        ledProps[0] += (NUM_LEDS/2-sum);
    }

    // Distribute LEDs again, giving largest significance to low frequencies.
    for(uint16_t i = 0; i < OCT_N-1; i++) {
        uint16_t intm = (OCT_N/2-1 + i*2 > OCT_N-1) ? OCT_N : OCT_N/2-1 + i*2;
        for(uint16_t j = i+1; j < intm; j++) {
            if(!i && ledProps[j] > 1) {
                ledProps[i] += 2;
                ledProps[j] -= 1;
            }
            else if(ledProps[j] > 1) {
                ledProps[i] += 1;
                ledProps[j] -= 1;
            }
        }
    }

    // Initialize the color distributions needed for visRoutine2
    for(uint16_t i = 0; i < OCT_N/2; i++) {
        colDist[i].b = i*(255/(OCT_N/2-1));
        colDist[i].r = 255 - i*(255/(OCT_N/2-1));
        colDist[i].g = 0;
    }
    for(uint16_t i = OCT_N/2; i < OCT_N; i++) {
        colDist[i].b = 255 - (i-OCT_N/2+1)*(255/(OCT_N/2));
        colDist[i].r = 0;
        colDist[i].g = (i-OCT_N/2+1)*(255/(OCT_N/2));
    }

    // Initialize the color coefficient distributions needed for visRoutine3
    for(uint16_t i = 0; i < OCT_N/2; i++) {
        coeffDist[i].BCOEFF = COL_COEFF_MIN + i*((1-COL_COEFF_MIN)/(OCT_N/2-1));
        coeffDist[i].RCOEFF = i*((1-COL_COEFF_MIN)/(float)(OCT_N/2-1));
        coeffDist[i].GCOEFF = 1 - i*((1-COL_COEFF_MIN)/(float)(OCT_N/2-1));
    }
    for(uint16_t i = OCT_N/2; i < OCT_N; i++) {
        coeffDist[i].BCOEFF = 1 - (i-OCT_N/2+1)*(1/(float)(OCT_N/2));
        coeffDist[i].RCOEFF = (i-OCT_N/2+1)*(1/(float)(OCT_N/2));
        coeffDist[i].GCOEFF = 1 - (i-OCT_N/2+1)*(COL_COEFF_MIN/(OCT_N/2));
    }
}
