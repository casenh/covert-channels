//This file will used unsupervised clustering to determine if the program is sending a 0 or 1

#include <iostream>
#include <vector>
#include <fstream>
#include "../../papi-5.3.0/src/papi.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../../dlib-18.2/dlib/clustering.h"
#include "../../dlib-18.2/dlib/rand.h"

using namespace std;
using namespace dlib;

extern void getCompInfo(int);
extern void handle_error(int);
extern int PAPI_create_eventset(int*);

/* This function returns the current time returned by PAPI_get_real_nsec in	*
 * seconds. Max it returns in about 16 seconds									*/
long long int getTime(void){
  
 long long int time = PAPI_get_real_nsec();
 time = time & 0x00000000FFFFFFFF; //Chop of the top of the bits so we only get seconds
 return time;
}


int NUM_OF_SETS = 0;
int BLOCK_LENGTH = 0;
int WAYS_ASSOCIATIVE = 0;
int doChannel(FILE* fp, FILE* fp2, std::vector<long long> tuning) {
    
/* Initializing everything for dlib */
    typedef matrix<double,1,1> sample_type; //Samples will be 2-D column vectors
    typedef radial_basis_kernel<sample_type> kerneltype; //Use a radial type kernel
    kcentroid<kerneltype> kc(kerneltype(0.1),.01,15); //This is the kcentroid object used in the kmeans test
    kkmeans<kerneltype> test(kc); //The kkmeans object that must be passed to the test
    
    sample_type M; //The matrix that holds our results
    std::vector<sample_type> samples; //The vector we will stort the results in
    std::vector<sample_type> initial_centers; //A vector to hold the inital centers we pick
     
     
/* Initialize all of the variables that will be used by the algorithm and PAPI */
    int numdata = 0;
    long long int PAPItime, startTime; //Used to hold the starting time
    long long int *data = (long long int*) malloc(sizeof(long long int) * 15000000); //This array will store all of the data that is collected
    char* iterate; //The large array to iterate over
  
    if( fp == NULL){
	printf("Error opening data dump! Aborting \n");
	exit(0);
    }  
    
/* Get the cache info for the cpu's */
    getCompInfo(1);
    
/* Allocate the memory that will be used to cause cache misses */
    iterate = (char*) malloc(sizeof(char) * (NUM_OF_SETS * WAYS_ASSOCIATIVE * BLOCK_LENGTH * 4)); //Total cache size = number of sets * lines per set * cache line length    
    
/* Initialize the PAPI library and create the events that we wish to monitor */
    int retval = PAPI_library_init(PAPI_VER_CURRENT);
    if( retval != PAPI_VER_CURRENT) {
	fprintf(stderr,"PAPI library init error!\n");
	exit(1);
    }
	#ifdef PERF_CTRS
    if( PAPI_create_eventset(&eventSet) != PAPI_OK){ handle_error(1);}   
    if(PAPI_add_event(eventSet, PAPI_L1_DCM) != PAPI_OK){ handle_error(1); }
    if( PAPI_start(eventSet) != PAPI_OK){ handle_error(1); }
	#endif
    
    
/* Take an initial run over the array to pad the cache. This will prevent any false '1' readings at the beginning */
    /*for(int n = 0; n < (NUM_OF_SETS * 8); n++)
    { 
	iterate[n * 64] = 1;
    }*/
    
    
/* Here is the main loop. Cycles over the given array for 10 seconds while keeping track of the events and 	*
* publishing them to a file.											*/
//NOTE THE ORIGIANAL TIME THAT WORKS WELL IS 20000 period, 15000 reader wait, and 8000 sender wait
    int period = tuning[0];
    int wait1 = tuning[1];
    int wait2 = tuning[2];
    int total = 0, numsamples = 0;
    int numberOfBitsSent;
    cout << "HERE\n";
    cin >> numberOfBitsSent; //Take in the number of bits that the sender is going to send
    while(getTime() > 20000) {  } //Wait until the signal before beginning transmission
    PAPItime = PAPI_get_real_nsec(); //Take the start time (in total nanoseconds)
    startTime = getTime(); //Take the start time (in nanoseconds up to seconds)
    
    for(int i = 0; i < numberOfBitsSent; i++) //Take a sample for every bit the sender will send
    { 
	while(getTime() < (startTime + (i * period))) { } //Only take once measurement per period length

	while(getTime() < (startTime + wait1 + (i * period))) //1) PRIME THE CACHE
	{
	    for(int n = 0; n < NUM_OF_SETS; n++){  
		iterate[n * BLOCK_LENGTH] = 1;
	    }
	}
	    
	while(getTime() < (startTime + (wait1 + wait2) + (i * period))) { } // 2) WAIT FOR THE SENDER TO COMMUNICATE
	    
	#ifdef PERF_CTRS
		if( PAPI_reset(eventSet) != PAPI_OK) { handle_error(1); } //3) PROBE THE CACHE
	#else
		long long int probeStart = PAPI_get_real_nsec();
	#endif
	
	for(int n = 0; n < NUM_OF_SETS*4; n++)
	{
		iterate[n * BLOCK_LENGTH] = 1;
	}
	
	#ifdef PERF_CTRS	
	if( PAPI_read(eventSet, counters) != PAPI_OK){ handle_error(1);} //Read the hardware counters and record the results
	#endif

	//total += counters[0]; //Keep an average for capping

	#ifdef PERF_CTRS	
		M(0) = counters[0]; //Store the counters as a sample	
		data[numdata] = counters[0]; //Dump the data
		numdata += 1;
		data[numdata] = PAPI_get_real_nsec() - PAPItime;
		numdata += 1;
	#else
		M(0) = PAPI_get_real_nsec() - probeStart;
		data[numdata] = PAPI_get_real_nsec() - probeStart;
		numdata++;
		data[numdata] = PAPI_get_real_nsec() - PAPItime;
		numdata++;
		total += M(0);
	#endif
	    
	samples.push_back(M); //Store the sample to be analyzed
	numsamples += 1; //Add one to the number of samples
    }
    
	/* Smooth out the values */
//	for(int i = 0; i < numdata; i += 2) {
//		if(data[i] > average + 100)
//			data[i] = average + 100;
//		else if(data[i] < average - 100)
//			data[i] = average - 100;
//	}
    
/* Analyze the collected data with dlib */    
    pick_initial_centers(2, initial_centers, samples, test.get_kernel()); //Pick initial centers for the kmeans test
    find_clusters_using_kmeans(samples, initial_centers, 6000); //Run the kmeans test
    
    /* Debugging */ 
// 	cout << "Initial initial_centers = " << initial_centers[0] << endl;
// 	cout << "Initial initial_centers = " << initial_centers[1] << endl;
// 	fprintf(fp, "Center1: %f    Center2: %f\n", initial_centers[0](0), initial_centers[1](0));  
    
    
/* Convert the data from raw hardware counter values to 1's and 0's by using the clusters */
    sample_type center0, center1;  //Make sure that the lowered valued center is center0
    if(initial_centers[0] < initial_centers[1])
    {
	center0 = initial_centers[0];
	center1 = initial_centers[1];   
    }
    else
    {
	center1 = initial_centers[0];
	center0 = initial_centers[1];
    }  
    
    //fprintf(fp, "%f  %f", center0(0), center1(0));
    //Loop over all samples and print out their predicted class.
    int currdata = 0;
    for( int i = 0; i < numsamples; i++){ //Dump the data
	int result;
	sample_type currSample = samples[i];
	int Distance0 = abs(center0(0) - currSample(0));
	int Distance1 = abs(center1(0) - currSample(0));
	if(Distance0 < Distance1)
	{
	    result = 0;
	}
	else
	{
	    result = 1;
	}
	
	fprintf(fp, "%lld %lld\n", data[currdata+1], data[currdata]);
	fprintf(fp2, "%lld %d\n", data[currdata+1], result);
	currdata += 2;
    }
    
    free(data);
    free(iterate);
	#ifdef PERF_CTRS
    PAPI_stop(eventSet, counters);
    PAPI_cleanup_eventset(eventSet);
    PAPI_destroy_eventset(&eventSet);
	#endif    

    return 0;
}


// bool outlier(long long int prevL2, long long int prevL3, long long int *counters){
//     
//     long long int L2 = prevL2 - counters[0];
//     long long int L3 = prevL3 - counters[1];
//     
//     if(L2 < 0)
// 	L2 = -L2;
//     if(L3 < 0)
// 	L3 = -L3;
//     
//     if(L2 > (.07*L2Baseline) || L3 > (.07*L3Baseline) || counters[0] > 8600)
// 	return true;
//     else
// 	return false;
// }





