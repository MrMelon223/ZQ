	// ZQhelper.cpp
#define GL_GLEXT_PROTOTYPES
#include "../include/ZQhelper.h"

const float PI = 3.1415f;

#ifdef CUDA_RT

void cuCheck(cudaError_t& e) {
#ifdef DEBUG
	if (e != cudaSuccess) {
		std::cout << "CUDA Error: " << cudaGetErrorString(e) << std::endl;
	}
#endif
}

#endif