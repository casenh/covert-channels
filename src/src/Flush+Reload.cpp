#include <iostream>
#include <stdlib.h>
#include <vector>

#define PERF_CTRS

#include "getCacheConfig.h"
#include "Flush+Reload.h"

extern void dummy_func (void);

/**
 * Flush+Reload Constructor
 * Grab all relevant parameters
 */
Flush_and_Reload::Flush_and_Reload(std::vector<long> params) : Channel::Channel(params) {

        if(params.size() < 2) {
                cout << "Please specify the period and reader_wait" << endl;
                exit(-1);
        }
        this->period = params[0];
        this->reader_wait = params[1];
}

int flush_reload(char* adrs) {

        volatile unsigned long time;

        __asm__ __volatile__ (
                "mfence \n"
                "lfence \n"
                "rdtsc \n"
                "lfence \n"
                "movl %%eax, %%esi \n"
                "movl (%1), %%eax\n"
                "lfence \n"
                "rdtsc \n"
                "subl %%esi, %%eax \n"
                "clflush 0(%1) \n"
                : "=a" (time)
                : "c" (adrs)
                : "%esi", "%edx"
                );

        return time;
}

long long int Flush_and_Reload::getTime() {

        long long int tsc;
        __asm__ volatile("rdtsc; " // read of tsc
                         "shl $32,%%rdx; "  // shift higher 32 bits stored in rdx up
                         "or %%rdx,%%rax"   // and or onto rax
                         : "=a"(tsc)        // output to tsc
                         :
                         : "%rcx", "%rdx", "memory");

        return tsc;
}

/**
 * Receive bits across the Flush+Reload channel. Specify the number of bits and where
 * to store the bits
 */
void Flush_and_Reload::receive(int numbits, long long int* probes) {

        dummy_func();
        int off = *((int *) (((char *)dummy_func) + 2));
        long long * ptr = *((long long **)(((char *)dummy_func) + off + 6));
        cout << "Ready\n";
        string ready;
        cin >> ready;
        while((getTime() & TIME_MASK) > 20000) {  }
        long long int startTime = getTime();

        for(int i = 0; i < numbits; i++) {

                while(getTime() < (startTime + (i * period) + reader_wait)) { }

                probes[2*i+1] = flush_reload((char *)ptr);
                probes[2*i] = getTime() & TIME_MASK;
        }
}

/**
 * Send a 1 across the Cache cache channel
 */
void Flush_and_Reload::send1(long long stop){ stop = stop;}

void Flush_and_Reload::send(int stream_length, long long int* bit_stream) {

        cout << "Ready\n";
        string ready;
        cin >> ready;
        while((getTime() & TIME_MASK) > 20000) {  }
        long long int startTime = getTime();

        for(int n=0; n < stream_length; n++){

                while(getTime() < startTime + (n * period)) { }
                if(bit_stream[n] == 1)
                        dummy_func();
        }
}

void Flush_and_Reload::analyzeReceived(int numbits, long long int* probes, int * bits){
        long avg = 0;
        for (int i = 0; i < numbits; ++i) {
                avg += probes[i*2+1];
        }
        avg = avg/numbits;

        for (int i = 0; i < numbits; ++i) {
                bits[i] = ((probes[i*2+1] < avg)) ? 0 : 1;
        }
}
