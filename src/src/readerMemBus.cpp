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
#include <numeric>
#include <functional>
#include <algorithm>
#include <iterator>


#include "../../dlib-18.2/dlib/clustering.h"
#include "../../dlib-18.2/dlib/rand.h"
#define MEM_BUFFER_SIZE 128*1024*1024
#define SKIP 0
int NUM_OF_SETS = 4096;
int WAYS_ASSOCIATIVE = 0;
int BLOCK_LENGTH = 64;
long long int IGNORE = -1;
//#define CHECK_TIME

using namespace std;
using namespace dlib;
typedef matrix<double,1,1> sample_type;

extern void handle_error(int);
extern int PAPI_create_eventset(int*);
extern void analyzeData(FILE* rawData, FILE* clusteredResults, std::vector<sample_type> &samples, long long int* data, int numSamples);

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
    IGNORE = tuning[3];
    
    std::vector<sample_type> samples;
    std::vector<sample_type> trainingSamples;
    std::vector<sample_type> initial_centers; 
    sample_type M;
 
/* Initialize all of the variables that will be used by the algorithm and PAPI */
    long long int startTime; //Used to hold the starting time
    int *memory_buffer = (int *) malloc(MEM_BUFFER_SIZE*sizeof(int));	
    
#define ROUND_NUM 1    
    long long int data[ROUND_NUM][2*250*1000];// = (long long int [20][20000]) malloc(sizeof(long long int) * 20*20000); //This array will store all of the data that is collected
    
/* Initialize the PAPI library and create the events that we wish to monitor */
/*
    int retval = PAPI_library_init(PAPI_VER_CURRENT);
    if( retval != PAPI_VER_CURRENT) {
		fprintf(stderr,"PAPI library init error!\n");
		exit(1);
    }
    if( PAPI_create_eventset(&eventSet) != PAPI_OK){ handle_error(1);}   
    if(PAPI_add_event(eventSet, PAPI_LD_INS) != PAPI_OK){ handle_error(1); }
    //if(PAPI_add_event(eventSet, PAPI_SSE_MEM_EXEC) != PAPI_OK){ handle_error(1); }
    if( PAPI_start(eventSet) != PAPI_OK){ handle_error(1); }
  */
/* Here is the main loop. Cycles over the given array for 10 seconds while keeping track of the events and 	*
* publishing them to a file.											*/
    int numberOfBitsSent;
    long long int period = tuning[0];
    //long long int wait = tuning[1];
    std::cin >> numberOfBitsSent;
    //numberOfBitsSent *= 2;
    while(getTime() > 20000) {  }
    startTime = PAPI_get_real_nsec();
    int inner_loop_iter = tuning[2];
    
    for(int round = 0; round < ROUND_NUM; round ++) {
		cerr << "Exp (reader) # " << round << endl;
		while(PAPI_get_real_nsec() < startTime + round*2*1000000000L + round*period/20);
		
		for(int i = 0; i < 2*numberOfBitsSent; i+=2){ //Only run for 5 seconds (time returned in nano seconds)
			while(PAPI_get_real_nsec() < (startTime + (i/2 * period))) { } //Wait to begin taking the measurements						
			
			/* One load + mfence instruction pair takes ~200ns */
			long papi_start_time = PAPI_get_real_nsec();
			for(int j = 0; j < inner_loop_iter; j++) 
			{	
			   //~ count += data[j*j*j%15000000];
			   //~ __asm__ __volatile__( "mfence\n\t" );
				unsigned int ind = i*i + j;
				ind = (ind*ind);
				ind = ind % MEM_BUFFER_SIZE;
				ind &= 0xFFFFFFFB;
			   _mm_stream_si32(&memory_buffer[ind], ind);
			   __asm__ __volatile__( "mfence\n\t" );
			   //~ m128_temp = _mm_stream_load_si128 (&memory_buffer[j*j%1000]);
			   //~ __asm__ __volatile__( "mfence\n\t" );
			   //~ m128_temp = _mm_stream_load_si128 (&memory_buffer[j%333]);
			   //~ __asm__ __volatile__( "mfence\n\t" );
			   //~ m128_temp = _mm_stream_load_si128 (&memory_buffer[j*j%567]);
			   //~ __asm__ __volatile__( "mfence\n\t" );
				
			   
			}
			long papi_end_time = PAPI_get_real_nsec();
			if(i >= 2*IGNORE) {
				data[round][i-2*IGNORE] = papi_end_time - papi_start_time;
				data[round][i+1-2*IGNORE] = PAPI_get_real_nsec();
			}
	#ifdef CHECK_TIME		
		cerr << "loop: " << data[round][i] << endl;
	#endif
		}		
		cerr << "Exp (reader 1) # " << round << endl;
    }
    cerr << "after loop" << endl;
    for(int round = 0; round < ROUND_NUM; round ++) {
            for(int i = 2*SKIP; i < (numberOfBitsSent-IGNORE)*2; i+=2){ //Dump the data
                    fprintf(fp2, "%lld %lld\n", data[round][i+1], data[round][i]); // time    sample
            }
    }
	
	/*
    //fp2 = fopen(file, "a");
    fp3 = fopen("../data/receiverFiles", "a");
    strcpy(tempFile, file);
    strcat(tempFile, "\n");
    fprintf(fp3, "%s", tempFile);
    //strcat(file,"-2_");
    strcat(file,"_");
    strcat(file, argv[1]);
    cerr << file << endl;
    fp = fopen(file, "w");
    if( fp == NULL){
		printf("Error opening data dump! Aborting \n");
		exit(0);
    }

    std::vector<long long int> vec_samples;
    for(int i = 2*SKIP; i < numberOfBitsSent*2; i +=2 )
    {
		average += data[i];
		vec_samples.push_back(data[i]);		
    }
    average /= (numberOfBitsSent-SKIP);
    
    size_t sd = vec_samples.size()/2;
    nth_element(vec_samples.begin(), vec_samples.begin()+sd, vec_samples.end());
    long long int median = vec_samples[sd];
    
    double sum = std::accumulate(std::begin(vec_samples), std::end(vec_samples), 0.0);
    double m = sum/ vec_samples.size();
    double accum = 0.0;
    std::for_each(std::begin(vec_samples), std::end(vec_samples), [&](const double d) {
		accum += (d-m)*(d-m);
	});
	double stdev = sqrt(accum/vec_samples.size()-1);
    
    std::cerr << "Average: " << average << std::endl;
    std::cerr << "Median: " << median << std::endl;
    std::cerr << "Stdev: " << stdev << std::endl;
    //~ double upper_limit = average + 2*stdev;
    //~ double low_limit = average - 2*stdev;
    double upper_limit = average + 150;
    double low_limit = average - 150;
    for(int i = 2*SKIP; i < numberOfBitsSent*2; i +=2 )
    {
		
		//~ data[i] = data[i] > upper_limit ? upper_limit : data[i];
		//~ data[i] = data[i] < low_limit ? low_limit : data[i];
		//~ data[i] = data[i] > average + 6000 ? average + 6000 : data[i];
		//~ data[i] = data[i] < average - 6000 ? average - 6000: data[i];
		//~ 
		//~ data[i] = data[i] > median ? median + 100 : median - 100;
		bool bit = data[i] > median+200 ? 1 : 0;
		fprintf(fp, "%lld %lld\n", data[i+1], data[i]);
		//fprintf(fp2, "%lld %d\n", data[i+1], bit);
    }
     return 0;

    // Loop over all samples and print out their predicted class. 
    int currdata = 0;
    previousResult = 0;
    sample_type st;
    for(int i = 2*SKIP; i < numberOfBitsSent*2; i+=2){ //Dump the data		
		fprintf(fp, "%lld %lld\n", data[i+1], data[i]); // time    sample
		st(0) = data[i];
		samples.push_back(st);
	}
    analyzeData(fp, fp2, samples, &data[2*SKIP], numberOfBitsSent - SKIP	);


    */
    //free(data);
    free(memory_buffer);
    printf("Reader done \n");

    return 0;
}




