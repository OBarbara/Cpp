#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <sys/time.h>	// for seed in srand()
#include <stdint.h>
void SSmulFFT(uint32_t* X, bool inverse, const int size, const int bpe);

/*! \name Timing functions */

//! Returns current time in seconds
inline double gettime()
   {
   struct timeval tv;
   struct timezone tz;
   gettimeofday(&tv, &tz);
   return ((double) tv.tv_sec + (double) tv.tv_usec * 1E-6);
   }

// @}

int main(int argc, char *argv[])
{
	const int NTT_SIZE = atoi(argv[1]); // number of elements to be processed (N)
	const int BITS_PER_ELEM = atoi(argv[2]); // number of bits to represent one element (n)
	// check if both of the required arguments are powers of two
	double chk1 = log2((double)NTT_SIZE);
	double chk2 = log2((double)BITS_PER_ELEM);
	// check whether all user parameters are valid
	if (argc != 3 || chk1 != round(chk1) || chk2 != round(chk2) || BITS_PER_ELEM > 16
			|| NTT_SIZE <= 1 || BITS_PER_ELEM <= 1)
	{
	  std::cerr << "Usage: " << argv[0]
			<< " <NTT size> <Bits per element>" << std::endl;
	  exit(1);
	}

	uint32_t *in = new uint32_t[NTT_SIZE];
	uint32_t *out = new uint32_t[NTT_SIZE];
	bool flag = true;	// flag to show whether input and output arrays are identical
	int k;
	double t_NTT, t_INTT, intervalNTT = 0, intervalINTT = 0;

	srand(time(NULL));
	for (int j = 0; j < 1000; j++) {
		for (int i = 0; i < NTT_SIZE; i++)
		{
			in[i] = (uint32_t)(rand() % (1 << BITS_PER_ELEM) + 1);
			out[i] = in[i];
		}
		t_NTT = gettime(); // get initial time-stamp;
		SSmulFFT(out, false, NTT_SIZE, BITS_PER_ELEM);
		t_NTT = gettime() - t_NTT;
		t_INTT = gettime(); // get initial time-stamp;
		SSmulFFT(out, true, NTT_SIZE, BITS_PER_ELEM);
		t_INTT = gettime() - t_INTT;

		// total time taken is summed up
		intervalNTT += t_NTT;
		intervalINTT += t_INTT;

		for (k = 0; k < NTT_SIZE; k++) {
			if (in[k] != out[k]) {
				flag = false;
				break;
			}
		}
		if (!flag) {
			std::cout << "j = " << j << ", k = " << k << "\nin = " << in[k] << ", out = " << out[k] << std::endl;
			break;
		}
	}

	if (flag){
		std::cout << "NTT matched. Average time taken for NTT is: " << intervalNTT / 1000 << " seconds" << std::endl;
		std::cout << "Average time taken for INTT is: " << intervalINTT / 1000 << " seconds" << std::endl;
	}

	delete[] in;
	delete[] out;
	return 0;
}

void SSmulFFT(uint32_t* X, bool inverse, const int FFT_PTS, const int BITS_PER_ELEM)
{
	//-----------------------------------------------------------------------
	//        Definition of NTT variables
	//-----------------------------------------------------------------------
	uint32_t* p; 	  // these variables represent the two elements of the container to be manipulated
	uint32_t* q;
	int k = (int)log2(FFT_PTS);
	int   twiddle;  // twiddle factor (exponent of 2)
	int   rt_unity; // root of unity (exponent of 2)
	int   i, j, m;  // i,j and m are indices used in the radix-2 FFT loops
	int   n; 		// number of elements in container
	uint32_t   temp;	// temporary variable to store original value of first element
						// in adding and subtracting procedure

	//-----------------------------------------------------------------------
	//        Preparation of NTT variables
	//-----------------------------------------------------------------------
	int modulus = (1 << BITS_PER_ELEM) + 1; // prime number for modulo arithmetic

	n = FFT_PTS;	   	  // n = 2^k --> n # of elements in container
	rt_unity = 2 * BITS_PER_ELEM / n; // calculate # bits for root of unity

	if (!inverse) {
		/*
		* Procedure:
		* Example: For 4 elements --> element 0 is paired with element 2 and element 1 is paired with element 3
		* in the first loop: So element 0 = element0 + element2 and element2 = (original) element0 - (twiddle * element2)
		*/
		for (m = n / 2; m >= 1; m >>= 1)    // start with m = half n (step for wheel calculation)
		{                                 // dividing m by two until m = 1;
			for (j = 0; j < m; j++)  // start with j = 0
			{                        // increasing j by 1 until j >= m
				twiddle = (rt_unity * j * n) / (2 * m); 	// calculate twiddle factor

				for (i = j; i < n; i += m << 1)   // place indices to elements in
				{               				  // container in p and q
					p = &X[i];
					q = &X[i + m];
					//
					//        Add and Sub
					//
					temp = *p;
					*p += *q;
					*p %= modulus;
					while (temp < *q) 	// if temp (original p) less than q
						temp += modulus; // add modulus to original p so that subtraction
										 // does not result in a negative no.
					temp -= *q; 		// subtract q from original p
					*q = temp; 			// copy temporary variable to q
										//
										//        Mul and Mod
										//
					if (twiddle != 0) {    // do only if Twiddle-Factor is not zero
						*q <<= twiddle;
						*q %= modulus;
					}
				}
			}
		}			// end of forward FFT
	}
	else {
		for (m = 1; m < n; m <<= 1) // start with m = 1, multiplying m by two until
		{
			for (j = 0; j < m; j++)  // m >= n; start with j = 0
			{                        // increasing j by 1 until j not < m
				twiddle = (rt_unity * j * n) / (2 * m);       // calculate twiddle factor
															  //twiddle = ((j + 1) * n) / 4;
				for (i = j; i < n; i += m << 1)
				{
					p = &X[i];           // place addresses to elements in container
					q = &X[i + m];         // in p and q
										   //
										   //        Mul and Mod
										   //
					if (twiddle != 0)   // do only if Twiddle Factor is not zero:
					{
						*q <<= twiddle;
						*q %= modulus;
					}
					//
					//        Add and Sub
					//
					while (*p < *q)  	   		 // if p < q, then
						*p += modulus;    // add modulus to p

					temp = *p;         // copy p to temporary field
					*p += *q;          // add q to p
					*p %= modulus;     // apply modulus to p
					temp -= *q;        // subtract q from temp
					*q = temp;         // copy temporary variable to q
				}
			}
		}		// end of backwards FFT
		short t = 2 * BITS_PER_ELEM - k; // convert division into multiplication
		bool rt_shift = false;	// should the shift be to the right and rotate bits?
		uint32_t ls, rs;	// variables used for right rotation

		if(t < 0){	// if t is negative, all shifts are to the right
			t = abs(t);
			rt_shift = true;
		}
		int temp_sh, shift;	// shifting will be done in an optimised manner
		for (int i = 0; i < n; i++) { // multiply every element by 2^t
			shift = 32 - (BITS_PER_ELEM + 1);
			if (shift > t)
				shift = t;	// if t is less than the maximum shift amount, then assign the shift amount to be t
			temp_sh = shift;
			p = &X[i];
			q = &X[FFT_PTS - i];
			if (rt_unity && i > 0 && i < n >> 1) {
				temp = *p;
				*p = *q;
				*q = temp;
			}
			for (int j = 0; j < t; j += shift) {
				shift = temp_sh;	// assign the shift value from the previous iteration
				if(!rt_shift)	// normal left shift
					*p <<= shift;
				else{	// right shift and bit rotation
					rs = *p >> shift;
					ls = *p << (32 - shift);
					*p = rs | ls;
				}
				*p %= modulus;
				if (t - shift <= (BITS_PER_ELEM - 1))
					temp_sh = t - shift;	// all remaining shifts will be done after modulus
			}
		}
	}
	return;
}

