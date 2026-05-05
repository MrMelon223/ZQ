#ifndef ZQCOMPUTE_H
#define ZQCOMPUTE_H

#include "ZQhelper.h"

class ZQcompute {
protected:
	//sycl::device main_cpu;
	//sycl::queue* cpu_queue;
public:
	ZQcompute();

	//sycl::queue* get_cpu_qptr() { return this->cpu_queue; }
};

#endif