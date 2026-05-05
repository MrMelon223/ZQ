	// ZQhelper.cpp
#include "../include/ZQhelper.h"

#ifdef CUDA_RT

void cuCheck(cudaError_t& e) {
#ifdef DEBUG
	if (e != cudaSuccess) {
		std::cout << "CUDA Error: " << cudaGetErrorString(e) << std::endl;
	}
#endif
}

#endif