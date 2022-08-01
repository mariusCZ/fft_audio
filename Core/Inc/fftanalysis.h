/*
 * fftanalysis.h
 *
 *  Created on: May 8, 2022
 *      Author: marius
 */

#ifndef INC_FFTANALYSIS_H_
#define INC_FFTANALYSIS_H_

#include "arm_math.h"

#define Q15_SBIT		0x8000
#define Q15_MBITS		0x7FFF
#define Q15_MULT		0x8000
#define Q15_PMAX		0x7FFF
#define Q15_NMAX		0x8000

#define FREQ_5KHZ_IND 51

int16_t f_to_q15 (float f);
void window_hamming (void);
void conv_oct(uint16_t input[], uint16_t output[]);
void perform_fft(q15_t *samples, q15_t outputfft[]);

#endif /* INC_FFTANALYSIS_H_ */
