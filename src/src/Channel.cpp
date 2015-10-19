#include "Channel.h"

#include "../../papi-5.3.0/src/papi.h"
#define PERF_CTRS
#ifdef PERF_CTRS
#endif

#include <iostream>
#include <string>
#include <vector>



Channel::Channel(vector<long> params) {
}

/**
 * Uses rdtsc because it's universal 
 */
long long int Channel::getTime() {

	long long int tsc;
	__asm__ volatile("rdtsc; " // read of tsc
			"shl $32,%%rdx; "  // shift higher 32 bits stored in rdx up
			"or %%rdx,%%rax"   // and or onto rax
			: "=a"(tsc)        // output to tsc
			:
			: "%rcx", "%rdx", "memory");

	//return tsc;
	return PAPI_get_real_nsec();
}
