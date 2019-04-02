/*
 ============================================================================
 Name        : NTTFFTCUDA.cu
 Author      : Owen
 Version     :
 Copyright   : Your copyright notice
 Description : CUDA compute reciprocals
 ============================================================================
 */

#include <iostream>
#include <numeric>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdint.h>
#include <math.h>

static void CheckCudaErrorAux (const char *, unsigned, const char *, cudaError_t);
void ProcessandTime(const int size, const int bpe, const int thread_limit);
void initialize(uint32_t* arr, int size, const int bpe);
#define CUDA_CHECK_RETURN(value) CheckCudaErrorAux(__FILE__,__LINE__, #value, value)
/**
 * CUDA kernel that computes the integer FFT of an array of unsigned integers
 */
// both of the following constants must be a power of two
// the first part takes care of the first stages before the shuffling and the
// element transfer from shared memory to global memory
__global__ void MulSMNTTKernel1(uint32_t *data, uint32_t* final_temp, uint32_t* temp_data, int curr_k, int k, int mod, int g, bool repos) {
	unsigned short idx = threadIdx.x;
	unsigned short b = blockIdx.x;
	short gridSizeSh = logf(gridDim.x) / logf(2);
	const int BLOCK_SIZE = blockDim.x;	// the offset given for the second part of shared memory
	if (idx < BLOCK_SIZE){	// for maximum performance, the last idx should be a multiple of 32 (minus 1 since it starts from 0)
		uint32_t tempvar; 	  // a temporary variable to manage subtractions and hold temporary data
		short i = idx / 2;
		short j = idx % 2;	// even or odd thread
		// used for twiddle factor indexing
		short l = k - curr_k - 1;
		int off = b * BLOCK_SIZE;
		short m = i % 2;	// helps in indexing for each thread in a block
		short b_twid = b << (l - 1);	// twiddle factor indexing for each block
		short elem = b + (idx << gridSizeSh);
		short p, sign, tmp, x, x1, twid_1, twid_2, twid_index;

		// copy data from global memory to shared memory
		final_temp[idx + off] = data[elem];
		__syncthreads();	// ensure that the shared memory is properly populated


		// stg is the stage (or epoch)
		// k is the number of stages in the computation
		for(short stg = 1; stg <= curr_k; stg++){
			p = BLOCK_SIZE >> stg;	// this variable will help in indexing
			// sign takes care of the operations for even (addition)
			// and odd (subtraction) threads
			sign = ((-2) * j) + 1;  // can be 1 or -1
			// indexing
			tmp = i + ((i >> (k - curr_k - stg)) * (1 << (k - curr_k - stg)));
			x = tmp + (j * p);
			x1 = x + (sign * p);
			twid_1 = (b + (i << gridSizeSh)) * (curr_k - stg);		// twiddle for stage 1
			twid_2 = ((l - 1) * ((!m * b_twid) + (m * (b_twid + BLOCK_SIZE))) + (curr_k - l) * (b << l)) * (stg - 1);	// twiddle for stage 2
			// twid_index manages the indexing of the twiddle factors
			twid_index =  twid_1 + twid_2;
			// since the value should be unsigned, a subtraction cannot result in a negative number
			// so we add the modulus to the number being subtracted to prevent that from happening
			tempvar = final_temp[off + x1];

			if(j != 0 && tempvar < final_temp[off + x])
				tempvar += mod;
			// addition and subtraction is taken care of here
			// modulus is done after addition/subtraction
			temp_data[off + x] = (tempvar + (sign * final_temp[off + x])) % mod;
			// shift by twiddle factor and perform modulus
			if(repos && j && twid_index)
				temp_data[off + x] <<= g * twid_index;
			final_temp[off + x] = temp_data[off + x] % mod;
			__syncthreads();
		}
		// shuffle data from shared to global memory
		data[elem] = final_temp[off + idx];
	}
}

// the first part takes care of the first stages before the shuffling and the
// element transfer from shared memory to global memory
__global__ void MulSMNTTKernel2(uint32_t *data, uint32_t* final_temp, uint32_t* temp_data, int NTT_SIZE, int curr_k, int k, int mod, int g, bool repos) {
	unsigned short idx = threadIdx.x;
	unsigned short b = blockIdx.x;
	unsigned short glob_idx = b * blockDim.x + idx;
	short gridSizeSh = logf(gridDim.x) / logf(2);
	const int BLOCK_SIZE = blockDim.x;	// the offset given for the second part of shared memory
	if (idx < BLOCK_SIZE){	// for maximum performance, the last idx should be a multiple of 32 (minus 1 since it starts from 0)
		uint32_t tempvar; 	  // a temporary variable to manage subtractions and hold temporary data
		int off = b * BLOCK_SIZE;
		short i = idx / 2;
		short j = idx % 2;	// even or odd thread
		short p, sign, tmp, x, x1, twid_index;

		// copy data
		final_temp[idx + off] = data[glob_idx];
		__syncthreads();	// ensure that the shared memory is properly populated

		// stg is the stage (or epoch)
		// k is the number of stages in the computation
		for(short stg = curr_k; stg <= k; stg++){
			p = NTT_SIZE >> stg;	// this variable will help in indexing
			// sign takes care of the operations for even (addition)
			// and odd (subtraction) threads
			sign = ((-2) * j) + 1;  // can be 1 or -1
			// indexing
			tmp = i + ((i >> (k - stg)) * (1 << (k - stg)));
			x = tmp + (j * p);
			x1 = x + (sign * p);
			// twid_index manages the indexing of the twiddle factors
			twid_index = (i % p) << (stg - 1);
			// since the value should be unsigned, a subtraction cannot result in a negative number
			// so we add the modulus to the number being subtracted to prevent that from happening
			tempvar = final_temp[off + x1];

			if(j != 0 && tempvar < final_temp[off + x])
				tempvar += mod;
			// addition and subtraction is taken care of here
			// modulus is done after addition/subtraction
			temp_data[off + x] = (tempvar + (sign * final_temp[off + x])) % mod;
			// shift by twiddle factor and perform modulus
			if(repos && j && twid_index)
				temp_data[off + x] <<= g * twid_index;
			final_temp[off + x] = temp_data[off + x] % mod;
			__syncthreads();
		}
		// write finished data from shared to global memory
		data[glob_idx] = final_temp[off + idx];
	}
}

// kernel for the inverse NTT FFT
__global__ void MulSMINTTKernel1(uint32_t *data, uint32_t* final_temp, uint32_t* temp_data, const int NTT_SIZE, const int BITS_PER_ELEM, int curr_k, int k, int mod, int g, bool repos, bool fin) {
	unsigned short idx = threadIdx.x;
	unsigned short b = blockIdx.x;
	unsigned short glob_idx = b * blockDim.x + idx;
	short gridSizeSh = logf(gridDim.x) / logf(2);
	const int BLOCK_SIZE = NTT_SIZE >> gridSizeSh;	// the offset given for the second part of shared memory
	if (idx < BLOCK_SIZE){	// for maximum performance, the last idx should be a multiple of 32 (minus 1 since it starts from 0)
		uint32_t tempvar; 	  // a temporary variable to manage subtractions and hold temporary data
		int off = b * BLOCK_SIZE;
		short i = idx / 2;
		short j = idx % 2;	// even or odd thread
		// short l = (b * blockDim.x + idx) >> 1; 	// used for finding twiddle factors
		short p, sign, tmp, x, x1, twid_index;

		// copy data
		final_temp[idx + off] = data[glob_idx];
		__syncthreads();	// ensure that the shared memory is properly populated

		// stg is the stage (or epoch)
		// k is the number of stages in the computation
		for(short stg = k; stg >= curr_k; stg--){
			p = NTT_SIZE >> stg;	// this variable will help in indexing
			// sign takes care of the operations for even (addition)
			// and odd (subtraction) threads
			sign = ((-2) * j) + 1;  // can be 1 or -1
			// indexing
			tmp = i + ((i >> (k - stg)) * (1 << (k - stg)));
			x = tmp + (j * p);
			x1 = x + (sign * p);
			// twid_index manages the indexing of the twiddle factors
			twid_index = (i % p) << (stg - 1);
			if(repos && twid_index){
				// shift by twiddle factor and perform modulus
				tempvar = (final_temp[off + x1] << (!j * g * twid_index)) % mod;
				temp_data[off + x] = final_temp[off + x] << (j * g * twid_index);
				temp_data[off + x] %= mod;
			}
			else{
				tempvar = final_temp[off + x1];
				temp_data[off + x] = final_temp[off + x];
			}

			// since the value should be unsigned, a subtraction cannot result in a negative number
			// so we add the modulus to the number being subtracted to prevent that from happening
			while(j != 0 && tempvar < temp_data[off + x])
				tempvar += mod;
			// addition and subtraction is taken care of here
			// modulus is done after addition/subtraction
			temp_data[off + x] = (tempvar + (sign * temp_data[off + x])) % mod;
			final_temp[off + x] = temp_data[off + x];
			// new data is ready for next stage
			__syncthreads();
		}

		if(fin){	// if there is only 1 SM
			// divide each element by N
			short t = (BITS_PER_ELEM << 1) - k; // convert division into multiplication
			uint32_t ls, rs;
			short size = sizeof(uint32_t) << 3; // multiply 4 bytes by 8 in this case (32 bits)
			bool rt_shift = false;	// should the shift be to the right and rotate bits?
			if(t < 0){	// if t is negative, all shifts are to the right
				uint32_t mask = t >> (size - 1);     // make a mask of the sign bit
				t ^= mask;                   // toggle the bits
				t += mask & 1;               // add one
				rt_shift = true;
			}
			short temp_sh;
			short shift = size - (BITS_PER_ELEM + 1);
			// shifting will be done in an optimized manner
			if (shift > t)
				shift = t;	// if t is less than the maximum shift amount, then assign the shift amount to be t
			temp_sh = shift;
			for (short m = 0; m < t; m += shift) {
				shift = temp_sh;	// assign the shift value from the previous iteration

				// there is no thread divergence here since all threads execute the same branch
				if(!rt_shift){	// normal left shift
					final_temp[off + idx] <<= shift;
				}
				else{	// right shift and bit rotation
					rs = final_temp[off + idx] >> shift;
					ls = final_temp[off + idx] << (size - shift);
					final_temp[off + idx] = rs | ls;
				}
				final_temp[off + idx] %= mod;
				if (t - shift <= (BITS_PER_ELEM - 1))
					temp_sh = t - shift;	// all remaining shifts will be done after modulus
			}

			// copy final data from shared memory to global memory
			if(repos)
				data[(NTT_SIZE - glob_idx) % NTT_SIZE] = final_temp[off + idx];
			else
				data[glob_idx] = final_temp[off + idx];
		}
		else{
			// copy from shared memory to global memory in a coalesced manner
			data[glob_idx] = final_temp[off + idx];
		}
	}
}

__global__ void MulSMINTTKernel2(uint32_t *data, uint32_t* final_temp, uint32_t* temp_data, const int NTT_SIZE, const int BITS_PER_ELEM, int curr_k, int k, int mod, int g, bool repos) {
	unsigned short idx = threadIdx.x;
	unsigned short b = blockIdx.x;
	short gridSizeSh = logf(gridDim.x) / logf(2);
	const int BLOCK_SIZE = blockDim.x;	// the offset given for the second part of shared memory
	if (idx < BLOCK_SIZE){	// for maximum performance, the last idx should be a multiple of 32 (minus 1 since it starts from 0)
		uint32_t tempvar; 	  // a temporary variable to manage subtractions and hold temporary data
		int off = b * BLOCK_SIZE;
		short i = idx / 2;
		short j = idx % 2;	// even or odd thread
		short l = k - curr_k - 1;
		short m = i % 2;
		short b_twid = b << (l - 1);	// twiddle factor indexing for each block
		short elem = b + (idx << gridSizeSh);
		short p, sign, tmp, x, x1, twid_1, twid_2, twid_index;

		// copy data
		final_temp[idx + off] = data[elem];
		__syncthreads();	// ensure that the shared memory is properly populated

		// stg is the stage (or epoch)
		// k is the number of stages in the computation
		for(short stg = curr_k; stg >= 1; stg--){
			p = BLOCK_SIZE >> stg;	// this variable will help in indexing
			// sign takes care of the operations for even (addition)
			// and odd (subtraction) threads
			sign = ((-2) * j) + 1;  // can be 1 or -1
			// indexing
			tmp = i + ((i >> (k - curr_k - stg)) * (1 << (k - curr_k - stg)));
			x = tmp + (j * p);
			x1 = x + (sign * p);
			twid_1 = (b + (i << gridSizeSh)) * (curr_k - stg);		// twiddle for stage 1
			twid_2 = ((l - 1) * ((!m * b_twid) + (m * (b_twid + BLOCK_SIZE))) + (curr_k - l) * (b << l)) * (stg - 1);	// twiddle for stage 2

			// twid_index manages the indexing of the twiddle factors
			twid_index =  twid_1 + twid_2;
			// shift by twiddle factor and perform modulus
			if(repos && twid_index){
				tempvar = (final_temp[off + x1] << (!j * g * twid_index)) % mod;
				temp_data[off + x] = final_temp[off + x] << (j * g * twid_index);
				temp_data[off + x] %= mod;
			}
			else{
				tempvar = final_temp[off + x1];
				temp_data[off + x] = final_temp[off + x];
			}

			// since the value should be unsigned, a subtraction cannot result in a negative number
			// so we add the modulus to the number being subtracted to prevent that from happening
			while(j != 0 && tempvar < temp_data[off + x])
					tempvar += mod;
			// addition and subtraction is taken care of here
			// modulus is done after addition/subtraction
			temp_data[off + x] = (tempvar + (sign * temp_data[off + x])) % mod;
			final_temp[off + x] = temp_data[off + x];
			// new data is ready for next stage
			__syncthreads();
		}

		// divide each element by N
		short t = (BITS_PER_ELEM << 1) - k; // convert division into multiplication
		uint32_t ls, rs;
		short size = sizeof(uint32_t) << 3; // multiply 4 bytes by 8 in this case (32 bits)
		bool rt_shift = false;	// should the shift be to the right and rotate bits?
		if(t < 0){	// if t is negative, all shifts are to the right
			uint32_t mask = t >> (size - 1);     // make a mask of the sign bit
			t ^= mask;                   // toggle the bits
			t += mask & 1;               // add one
			rt_shift = true;
		}
		short temp_sh;
		short shift = size - (BITS_PER_ELEM + 1);
		// shifting will be done in an optimized manner
		if (shift > t)
			shift = t;	// if t is less than the maximum shift amount, then assign the shift amount to be t
		temp_sh = shift;
		for (short m = 0; m < t; m += shift) {
			shift = temp_sh;	// assign the shift value from the previous iteration

			// there is no thread divergence here since all threads execute the same branch
			if(!rt_shift){	// normal left shift
				final_temp[off + idx] <<= shift;
			}
			else{	// right shift and bit rotation
				rs = final_temp[off + idx] >> shift;
				ls = final_temp[off + idx] << (size - shift);
				final_temp[off + idx] = rs | ls;
			}
			final_temp[off + idx] %= mod;
			if (t - shift <= (BITS_PER_ELEM - 1))
				temp_sh = t - shift;	// all remaining shifts will be done after modulus
		}

		// copy final data from shared memory to global memory
		if(repos)
			data[(NTT_SIZE - elem) % NTT_SIZE] = final_temp[off + idx];
		else
			data[elem] = final_temp[off + idx];
	}
}

/*! \name Timing function */

//! Returns current time in seconds
inline double gettime()
{
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	return ((double) tv.tv_sec + (double) tv.tv_usec * 1E-6);
}

// @}

/*! \name GPU function caller and process timing function */
void ProcessandTime(const int size, const int bpe, const int thlimit){
	// define NTT variables
	double intervalNTT = 0, intervalINTT = 0;
	float NTTms, iNTTms;
	// declare and create events
	cudaEvent_t NTTstart, NTTstop, INTTstart, INTTstop;
	cudaEventCreate(&NTTstart);
	cudaEventCreate(&NTTstop);
	cudaEventCreate(&INTTstart);
	cudaEventCreate(&INTTstop);
	int kt = log2((double)size);
	int i;	// index for traversing through the arrays
	int modulus = (1 << bpe) + 1; // prime number for modulo arithmetic
	int rt_unity = 2 * bpe / size; // calculate # bits for root of unity
	// these flags determine whether the IFFT repositions the elements
	// and whether the result of the IFFT matches the input array or not, respectively
	bool repos_flag = true, flag = true;
	int runs = 1E3;	// number of times each kernel function is executed
	uint32_t *in = new uint32_t[size];
	uint32_t *NTT_out = new uint32_t[size];
	uint32_t *INTT_out = new uint32_t[size];
	uint32_t* gpuNTTData, *gpuINTTData, *gpuTwid, *gputempData1, *gputempData2;
	CUDA_CHECK_RETURN(cudaMalloc((void **)&gpuNTTData, sizeof(uint32_t) * size));
	CUDA_CHECK_RETURN(cudaMalloc((void **)&gpuTwid, sizeof(uint32_t)*size / 2));
	CUDA_CHECK_RETURN(cudaMalloc((void **)&gpuINTTData, sizeof(uint32_t)*size));
	CUDA_CHECK_RETURN(cudaMalloc((void **)&gputempData1, sizeof(uint32_t)*size));
	CUDA_CHECK_RETURN(cudaMalloc((void **)&gputempData2, sizeof(uint32_t)*size));
	uint32_t twiddle[size / 2];	// pre-compute twiddle factor array
	if(!rt_unity)
		repos_flag = false;

	int blk_size;
	bool isSingleSM = false;
	if(size <= thlimit){
		blk_size = size;
		isSingleSM = true;
	}
	else
		blk_size = thlimit;
	static const int BLOCK_SIZE = blk_size;	// amount of threads in each block
	const int blockCount = (size) / BLOCK_SIZE;	// amount of blocks in a grid
	short k = log2((double)blockCount);	// first kernel covers first k stages only out of kt for FFT, and vice-versa for IFFT kernels
	std::cout << "Launching kernels with " << blockCount << " block(s), each with " << BLOCK_SIZE << " threads." << std::endl;
	srand(time(NULL));	// generate the seed for the pseudo-random number generator
	for (int j = 0; j < runs; j++) {
		initialize(in, modulus, size);

		CUDA_CHECK_RETURN(cudaMemcpy(gpuNTTData, in, sizeof(uint32_t)*size, cudaMemcpyHostToDevice));
		CUDA_CHECK_RETURN(cudaMemcpy(gpuTwid, twiddle, sizeof(uint32_t)*size/2, cudaMemcpyHostToDevice));

		cudaEventRecord(NTTstart);
		if(!isSingleSM){
			// computes first k stages and shuffles data from shared memory to global memory
			MulSMNTTKernel1<<<blockCount, BLOCK_SIZE, BLOCK_SIZE * sizeof(uint32_t)>>> (gpuNTTData, gputempData1, gputempData2, k, kt, modulus, rt_unity, repos_flag);
		}
		// shuffles data from global memory to shared memory and computes last set of stages
		MulSMNTTKernel2<<<blockCount, BLOCK_SIZE, BLOCK_SIZE * sizeof(uint32_t)>>> (gpuNTTData, gputempData1, gputempData2, size, k + 1, kt, modulus, rt_unity, repos_flag);
		cudaEventRecord(NTTstop);
		cudaEventSynchronize(NTTstop);
		cudaEventElapsedTime(&NTTms, NTTstart, NTTstop);

		CUDA_CHECK_RETURN(cudaMemcpy(NTT_out, gpuNTTData, sizeof(uint32_t) * size, cudaMemcpyDeviceToHost));
		CUDA_CHECK_RETURN(cudaMemcpy(gpuINTTData, NTT_out, sizeof(uint32_t)*size, cudaMemcpyHostToDevice));

		cudaEventRecord(INTTstart);
		// computes first stages and shuffles data from shared memory to global memory
		MulSMINTTKernel1<<<blockCount, BLOCK_SIZE, BLOCK_SIZE * sizeof(uint32_t)>>> (gpuINTTData, gputempData1, gputempData2, size, bpe, k + 1, kt, modulus, rt_unity, repos_flag, isSingleSM);
		if(!isSingleSM){
			// shuffles data from global memory to shared memory and computes last set of stages
			MulSMINTTKernel2<<<blockCount, BLOCK_SIZE, BLOCK_SIZE * sizeof(uint32_t)>>> (gpuINTTData, gputempData1, gputempData2, size, bpe, k, kt, modulus, rt_unity, repos_flag);
		}
		cudaEventRecord(INTTstop);

		CUDA_CHECK_RETURN(cudaMemcpy(INTT_out, gpuINTTData, sizeof(uint32_t)*size, cudaMemcpyDeviceToHost));

		cudaEventSynchronize(INTTstop);
		cudaEventElapsedTime(&iNTTms, INTTstart, INTTstop);

		// total time taken is summed up for all runs
		intervalNTT += NTTms;
		intervalINTT += iNTTms;


		for (i = 0; i < size; i++) {
			if (in[i] != INTT_out[i]) {
				flag = false;
				break;
			}
		}

		if (!flag) {
			std::cout << "j = " << j << ", i = " << i << "\nin = " << in[i] << ", out = " << INTT_out[i] << std::endl;
			break;
		}
	}

	if (flag){
		std::cout << "NTT matched. Average time taken for NTT is: " << intervalNTT << " seconds" << std::endl;
		std::cout << "Average time taken for INTT is: " << intervalINTT << " seconds" << std::endl;
	}

	// clean up
	CUDA_CHECK_RETURN(cudaFree(gpuNTTData));
	CUDA_CHECK_RETURN(cudaFree(gpuINTTData));
	CUDA_CHECK_RETURN(cudaFree(gputempData1));
	CUDA_CHECK_RETURN(cudaFree(gputempData2));
	CUDA_CHECK_RETURN(cudaFree(gpuTwid));
	delete[] in;
	delete[] NTT_out;
	delete[] INTT_out;
	return;
}

// @}


/* \name Array initializer function */
void initialize(uint32_t* in, int mod, const int size)
{
	for (int i = 0; i < size; i++)
	{
		in[i] = (uint32_t)(rand() % mod);
	}
}

// @}

int main(int argc, char *argv[])
{
	std::cerr << "NTT FFT" << std::endl;
	const int NTT_SIZE = atoi(argv[1]); // number of elements to be processed (N)
	const int BITS_PER_ELEM = atoi(argv[2]); // number of bits to represent one element (n)
	const int TH_LIM = atoi(argv[3]); // maximum number of threads allowable per kernel
	// check if both of the required arguments are powers of two
	double chk1 = log2((double)NTT_SIZE);
	double chk2 = log2((double)BITS_PER_ELEM);
	double chk3 = (double)logf(TH_LIM) / logf(2);
	// check whether all user parameters are valid
	if (argc != 4 || chk1 != round(chk1) || chk2 != round(chk2) || chk3 != round(chk3) || TH_LIM < 0 || TH_LIM > 1024 || BITS_PER_ELEM > 16
				|| NTT_SIZE <= 1 || BITS_PER_ELEM <= 1)
	{
	  std::cerr << "Usage: " << argv[0]
			<< " <NTT size> <Bits per element> <thread limit per block>" << std::endl;
	  exit(1);
	}
	std::cout << "Computing..." << std::endl;
	ProcessandTime(NTT_SIZE, BITS_PER_ELEM, TH_LIM);
	return 0;
}

/**
 * Check the return value of the CUDA runtime API call and exit
 * the application if the call has failed.
 */
static void CheckCudaErrorAux (const char *file, unsigned line, const char *statement, cudaError_t err)
{
	if (err == cudaSuccess)
		return;
	std::cerr << statement<<" returned " << cudaGetErrorString(err) << "("<<err<< ") at "<<file<<":"<<line << std::endl;
	exit (1);
}
