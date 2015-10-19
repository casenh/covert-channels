#include <iostream>
#include <stdlib.h>
#include <smmintrin.h>
#include <time.h>

#include "../../dlib-18.2/dlib/clustering.h"
#include "../../dlib-18.2/dlib/rand.h"

#include "MemBus.h"

using namespace std;

// TODO: Make this a input parameter
#define MEM_BUFFER_SIZE 128*1024*1024

/**
 * Memory Bus Constructor
 * Grab all relevant parameters 
 */
MemBus::MemBus(vector<long> params) : Channel::Channel(params) {

	if(params.size() < 5) {
		cerr << "Please specify all 4 cache channel parameters" << endl;
		exit(-1);
	}

	this->period = params[0];
	this->sender  = params[1];
	this->receiver = params[2];
	this->sender_loop = params[3];
  this->ignore_bits = params[4];

	this->fillerData = (int*) malloc(sizeof(int) * MEM_BUFFER_SIZE);
}


/**
 * Receive bits across the Memory Bus channel. Specify the number of bits and where
 * to store the bits
 */
void MemBus::receive(int numbits, long long int* probes) {
        cout << "Ready\n";
        string ready;
        cin >> ready;
        while((getTime() & TIME_MASK) > 20000) {  }
        long long int startTime = getTime();

        for(int i = 0; i < numbits + ignore_bits; i++){
                while(getTime() < (startTime + (i * period))) { }

/* One load + mfence instruction pair takes ~200ns */
                long long probeStart = getTime();
                for(int j = 0; j < receiver; j++)
                {
//~ count += data[j*j*j%15000000];
                                //~ __asm__ __volatile__( "mfence\n\t" );
                        unsigned int ind = i*i + j;
                        ind = (ind*ind);
                        ind = ind % MEM_BUFFER_SIZE;
                        ind &= 0xFFFFFFFB;
                        _mm_stream_si32(&fillerData[ind], ind);
                        __asm__ __volatile__( "mfence\n\t" );
                }
                long long probeStop = getTime();
                if ( i >= ignore_bits) {
                        long index = i - ignore_bits;
                        probes[index*2+1] = probeStop - probeStart;
                        probes[index*2] = getTime();
                }
        }
}

/**
 * Send a 0 across the Memory Bus channel
 */
void MemBus::send0(long long stop){
	unsigned long ind;
	unsigned int seed = rand();
	do {
		for(int n=0; n < sender_loop; n++) {
			ind = seed + n;
			ind *= ind;
			__asm__ volatile ( "lock xchg %%rax, (%0) \n\t"
                         :
                         : "r"(fillerData+(ind % (MEM_BUFFER_SIZE-3)))
                         : "%rax");
		}

	} while(getTime() < stop);
}

void MemBus::send(int stream_length, long long int* bit_stream) {

        srand(time(NULL));
	cout << "Ready\n";
	string ready;
	cin >> ready;
	while((getTime() & TIME_MASK) > 20000) {  }
	long long int startTime = getTime();

        for(int n=0; n < stream_length + ignore_bits; n++){

                while(getTime() < startTime + (n * period)) { }

                long long int stopTime = startTime + sender + (n * period);
                if (n < ignore_bits){
                        if (n % 2 == 0)
                                send0(stopTime);
                } else if(bit_stream[n - ignore_bits] == 0)
                        send0(stopTime);
        }
}

using namespace dlib;

void MemBus::analyzeReceived(int numbits, long long int* probes, int * bits){
        long avg = 0;
        for (int i = 0; i < numbits; ++i) {
                avg += probes[i*2+1];
        }
        avg = avg/numbits;

        for (int i = 0; i < numbits; ++i) {
                bits[i] = ((probes[i*2+1] < avg)) ? 0 : 1;
        }
}
