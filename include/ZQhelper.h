#ifndef ZQHELPER_H
#define ZQHELPER_H

#define DEBUG 1
//#define CUDA_RT 1

#ifdef DEBUG

#include <iostream>
#include <iomanip>

#endif

#ifdef CUDA_RT

#include "cuda_runtime.h"
#include "device_launch_parameters.h"


void cuCheck(cudaError_t&);

#endif

#include <cstdint>
#include <string>
#include <sstream>
#include <fstream>
#include <queue>
#include <vector>

#include "ZQvecs.h"
#include <GLFW/glfw3.h>
#include <sqlite3.h>
#include <tiny_gltf.h>

#endif