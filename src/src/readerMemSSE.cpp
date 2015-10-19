//This file will used unsupervised clustering to determine if the program is sending a 0 or 1


#include <iostream>
#include <vector>
#include <fstream>
#include "../../papi-5.3.0/src/papi.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <smmintrin.h>
#include <time.h>
//#include <emmintrin.h>
//#include <xmmintrin.h>
//#include <ammintrin.h>

#include "../../dlib-18.2/dlib/clustering.h"
#include "../../dlib-18.2/dlib/rand.h"
#include "analyzeData.hpp"
int NUM_OF_SETS = 4096;
int WAYS_ASSOCIATIVE = 0;
int BLOCK_LENGTH = 64;

using namespace std;
using namespace dlib;

extern void handle_error(int);
extern int PAPI_create_eventset(int*);

/* This function returns the current time returned by PAPI_get_real_nsec in	*
 * seconds									*/
long long int getTime(void){
  
 long long int time = PAPI_get_real_nsec();
 time = time & 0x00000000FFFFFFFF; //The max time that is returning is 9.999999999 seconds
 return time;
 
}


/* This function returns the lowest value out of the three centers */
template <class T> matrix<double,1,1> findLowerCenter (T initialCenters){
    
    if(initialCenters[0] < initialCenters[1] && initialCenters[0] < initialCenters[2])
    {
	return initialCenters[0];
    }
    else if(initialCenters[1] < initialCenters[0] && initialCenters[1] < initialCenters[2])
    {
	return initialCenters[1];
    }
    else 
    {
	return initialCenters[2];
    }    
}


/* This function returns the middle value out of the three centers */
template <class T> matrix<double,1,1> findMiddleCenter (T initialCenters){
    
    if(initialCenters[0] < initialCenters[1] && initialCenters[0] > initialCenters[2])
    {
	return initialCenters[0];
    }
    else if(initialCenters[1] < initialCenters[0] && initialCenters[1] > initialCenters[2])
    {
	return initialCenters[1];
    }
    else 
    {
	return initialCenters[2];
    }    
}


/* This function returns the highest value out of the three centers */
template <class T> matrix<double,1,1> findUpperCenter (T initialCenters){
    
    if(initialCenters[0] > initialCenters[1] && initialCenters[0] > initialCenters[2])
    {
	return initialCenters[0];
    }
    else if(initialCenters[1] > initialCenters[0] && initialCenters[1] > initialCenters[2])
    {
	return initialCenters[1];
    }
    else 
    {
	return initialCenters[2];
    }    
}


int doChannel(FILE* fp, FILE* fp2, std::vector<long long> tuning) {
/* Initializing everything for dlib */
    typedef matrix<double,1,1> sample_type; //Samples will be 2-D column vectors
    typedef radial_basis_kernel<sample_type> kerneltype; //Use a radial type kernel
    kcentroid<kerneltype> kc(kerneltype(0.1),.01,15);
    kkmeans<kerneltype> test(kc);
    
    std::vector<sample_type> samples;
    std::vector<sample_type> trainingSamples;
    std::vector<sample_type> initial_centers; 
    sample_type M;
 
/* Initialize all of the variables that will be used by the algorithm and PAPI */
    int eventSet = PAPI_NULL; //The EventSet that we will use with PAPI
    long long int PAPItime, startTime; //Used to hold the starting time
    volatile __m128i m128_temp;
    __m128i *memory_buffer = (__m128i *) malloc(1000*sizeof(__m128i));	
    
    
    long long int *data = (long long int*) malloc(sizeof(long long int) * 15000000); //This array will store all of the data that is collected
    
    
/* Initialize the PAPI library and create the events that we wish to monitor */
    int retval = PAPI_library_init(PAPI_VER_CURRENT);
    if( retval != PAPI_VER_CURRENT) {
	fprintf(stderr,"PAPI library init error!\n");
	exit(1);
    }
    if( PAPI_create_eventset(&eventSet) != PAPI_OK){ handle_error(1);}   
    if(PAPI_add_event(eventSet, PAPI_LD_INS) != PAPI_OK){ handle_error(1); }
    //if(PAPI_add_event(eventSet, PAPI_SSE_MEM_EXEC) != PAPI_OK){ handle_error(1); }
    if( PAPI_start(eventSet) != PAPI_OK){ handle_error(1); }
  
/* Here is the main loop. Cycles over the given array for 10 seconds while keeping track of the events and 	*
* publishing them to a file.											*/
//     cout << "Working..." << endl;
    int numberOfBitsSent;
    int period = tuning[0];
    int wait = tuning[1];
    long long int average = 0;
    std::cin >> numberOfBitsSent;
    while(getTime() > 20000) {  }
    PAPItime = PAPI_get_real_nsec();
    startTime = getTime();
    int num_mem_accesses = wait/800;
    startTime = getTime();

    
    for(int i = 0; i < 2*numberOfBitsSent; i+=2){ //Only run for 5 seconds (time returned in nano seconds)

		while(getTime() < (startTime + (i/2 * period))) { } //Wait to begin taking the measurements	

		/* One load + mfence instruction pair takes ~200ns */
		long papi_start_time = PAPI_get_real_nsec();
		for(int j = 0; j < num_mem_accesses; j++) 
		{	
		   m128_temp = _mm_stream_load_si128 (&memory_buffer[j%1000]);
		   __asm__ __volatile__( "mfence\n\t" );
		   m128_temp = _mm_stream_load_si128 (&memory_buffer[j*j%1000]);
		   __asm__ __volatile__( "mfence\n\t" );
		   m128_temp = _mm_stream_load_si128 (&memory_buffer[j%333]);
		   __asm__ __volatile__( "mfence\n\t" );
		   m128_temp = _mm_stream_load_si128 (&memory_buffer[j*j%567]);
		   __asm__ __volatile__( "mfence\n\t" );		   			   
		}
		long papi_end_time = PAPI_get_real_nsec();
		data[i] = papi_end_time - papi_start_time;
		data[i+1] = getTime();	
    }
    
    if( fp == NULL){
	printf("Error opening data dump! Aborting \n");
	exit(0);
    }

   
    for(int i = 20; i < numberOfBitsSent*2; i +=2 )
    {
	average += data[i];
    }
    average /= (numberOfBitsSent-10);
    for(int i = 20; i < numberOfBitsSent*2; i +=2 )
    {
		data[i] = data[i] > average ? average + 25 : average - 25;
    }


    /* Loop over all samples and print out their predicted class. */
    sample_type st;
    for(int i = 20; i < numberOfBitsSent*2; i+=2){ //Dump the data		
		fprintf(fp, "%lld %lld\n", data[i+1], data[i]);
		st(0) = data[i];
		samples.push_back(st);
	}
    analyzeData(fp, fp2, samples, data, numberOfBitsSent - 10);

	//~ long thisTime = PAPI_get_real_nsec();
	//~ for(int i = 0; i < 1000; i++) {
		//~ long temp3 = PAPI_get_real_nsec();
	//~ }
	//~ thisTime = (PAPI_get_real_nsec() - thisTime)/1000;
	//~ fprintf(fp, "%lld\n", thisTime);


    free(data);
    free(memory_buffer);
    printf("Reader done \n");
    return 0;
}




