	// ZQcompute.cpp
#include "../include/ZQcompute.h"

ZQcompute::ZQcompute() {
	//this->cpu_queue = new sycl::queue(sycl::cpu_selector_v);
	//this->main_cpu = this->cpu_queue->get_device();
#ifndef DEBUG
	std::cout << "   Chosen CPU" << std::endl;
	std::cout << "        Name: " << this->main_cpu.get_info<sycl::info::device::name>() << std::endl;
	std::cout << "     Threads: " << this->main_cpu.get_info<sycl::info::device::max_compute_units>() << std::endl << std::endl;
#endif
}