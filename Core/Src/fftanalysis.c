/*
 * fftanalysis.c
 *
 *  Code for performing FFT on the read samples.
 */

#include "main.h"
#include "fftanalysis.h"
#include "arm_math.h"
#include "math.h"

q15_t rWindow[FFT_SIZE];
uint8_t wflag = 0;

void perform_fft(q15_t *samples, q15_t outputfft[])
{
	static arm_rfft_instance_q15 fft_instance;
	q15_t fftbuf[FFT_SIZE*2] = {0};

	// Create the Hamming window array and initialize FFT.
	if(!wflag) {
		wflag = 1;
		window_hamming();
		arm_rfft_init_q15(&fft_instance, FFT_SIZE/*bin count*/, 0/*forward FFT*/, 1/*output bit order is normal*/);
	}

	// Multiply samples by the Hamming window.
	arm_mult_q15(samples, rWindow, samples, FFT_SIZE);
	// Perform FFT
	arm_rfft_q15(&fft_instance, samples, fftbuf);
	// Get real magnitude squared of the FFT
	arm_cmplx_mag_squared_q15(fftbuf, outputfft, FFT_SIZE/2+8);
	// Fix artifacts caused by above function.
	outputfft[0] = outputfft[0]/2;
	outputfft[FFT_SIZE-1] = outputfft[FFT_SIZE-1]/2;
}

// Hamming window creation function
void window_hamming (void)
{
	float alpha = 25.0/46;
	float factor = 2.0 * M_PI / FFT_SIZE;

	for (int i = 0; i < FFT_SIZE; i++) {
		float f  = alpha - (1.0 - alpha) * arm_cos_f32 (factor * i);
		rWindow[i] = f_to_q15 (f);
	}
}

// Convert float to Q15.1
int16_t f_to_q15 (float f)
{
	if (f >= 1.0) return Q15_PMAX;
	if (f <= -1.0) return Q15_NMAX;

	if (f < 0) {
		return - (Q15_MBITS & (int16_t) (-f * Q15_MULT));
	} else {
		return Q15_MBITS & (int16_t) (f * Q15_MULT);
	}
}

// Function to take the only most important frequencies
// and make the magnitudes logarithmic.
void conv_oct(uint16_t input[], uint16_t output[])
{
//	output[0] = 32 * log2f(input[0]);
//	output[1] = 32 * log2f(input[1]);
//
//	volatile uint32_t intm = 0;
//	for(int i = 0; i <= OCT_N-2; i++) {
//		for(volatile int j = (2 << i) ; j < (2 << (i+1)); j++) {
//			intm += (input[j]) * (input[j]);
//		}
//		output[i + 2] = 32 * log2f(sqrt(intm));
//		intm = 0;
//	}
//	output[OCT_N-2] = output[OCT_N-2] * 3;
//	output[OCT_N-1] = output[OCT_N-1] * 3;
//	output[OCT_N] = output[OCT_N] * 3;

	uint32_t intm = 0;
	// Frequencies displayed individually
    for(uint16_t i = 0; i < OCT_N-2; i++)
    	output[i] = 32*log2f(input[i+1]);
    // Combined frequencies.
    for(uint16_t i = OCT_N-2; i < FREQ_5KHZ_IND/2; i++)
		intm += (input[i]) * (input[i]);
	output[OCT_N-2] = 32 * log2f(sqrt(intm));
	intm = 0;
    for(uint16_t i = FREQ_5KHZ_IND/2; i < FREQ_5KHZ_IND; i++)
		intm += (input[i]) * (input[i]);
	output[OCT_N-1] = 32 * log2f(sqrt(intm));

	/*
	 * Yes, this is bad practice, hopefully only a placeholder.
	 * This is needed because the higher frequencies tend to
	 * have a weaker response. My personal guess is this is
	 * because of the anti-aliasing filter, but it should be
	 * set to around 15-20 kHz, so no idea why it would impact
	 * frequencies significantly lower than the filter cut-off.
	 */
	for(uint8_t i = OCT_N-1; i > 3; i--)
		output[i] = output[i] * (float)1.5;
}
