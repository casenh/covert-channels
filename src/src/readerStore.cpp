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
    int numsamples = 0, numdata = 0, n = 0; //The number of samples that were collected
    int eventSet = PAPI_NULL; //The EventSet that we will use with PAPI
    long long int PAPItime, startTime; //Used to hold the starting time
    long long int counters[2]; //Stores the counter values (the number of events being monitored)
    int array[5000];
    
    long long int *data = (long long int*) malloc(sizeof(long long int) * 15000000); //This array will store all of the data that is collected
    
    
/* Initialize the PAPI library and create the events that we wish to monitor */
    int retval = PAPI_library_init(PAPI_VER_CURRENT);
    if( retval != PAPI_VER_CURRENT) {
	fprintf(stderr,"PAPI library init error!\n");
	exit(1);
    }
    if( PAPI_create_eventset(&eventSet) != PAPI_OK){ handle_error(1);}   
    if(PAPI_add_event(eventSet, PAPI_SR_INS) != PAPI_OK){ handle_error(1); }
    if( PAPI_start(eventSet) != PAPI_OK){ handle_error(1); }
    
    
/* Here is the main loop. Cycles over the given array for 10 seconds while keeping track of the events and 	*
* publishing them to a file.											*/
//     cout << "Working..." << endl;
    int numberOfBitsSent;
    int period = tuning[0];
    int wait = tuning[1];
    long long int total = 0;
    std::cin >> numberOfBitsSent;
    while(getTime() > 20000) {  }
    PAPItime = PAPI_get_real_nsec();
    startTime = getTime();
    for(int i = 0; i < numberOfBitsSent; i++){ //Only run for 5 seconds (time returned in nano seconds)

	while(getTime() < (startTime + (i * period))) { } //Wait to begin taking the measurements

	if( PAPI_reset(eventSet) != PAPI_OK) { handle_error(1); }	
	long long int time = PAPI_get_real_nsec();
	int first = 1;
	while((PAPI_get_real_nsec() - time) < wait) 
	{	
	    if(first == 1)
	    {
			for(n = 0; n < 1500; n++)
			{
				array[n] = 1;
				array[n] = 2;
				array[n] = 3;
	 		    //array[i] = 4;
	 		    //array[i] = 5;
			}
	    }
	    first = 0;
	}
	
	if( PAPI_read(eventSet, counters) != PAPI_OK){ handle_error(1);} //Continue to add up the amount of cache misses after each iteration

	//~ if(counters[0] < (average - 25) && count >= 300) //Prevent outliers from scewing the data
	//~ {
	    //~ counters[0] = (average - 25);
	//~ }
	//~ if(counters[0] > (average + 25) && count >= 300)
	//~ {
	    //~ counters[0] = average + 25;
	//~ }
	
	//~ M(0) = counters[0]; //Store the counters as a sample
	data[numdata] = counters[0]; //Dump the data
	
	//~ if(count < 300) //Ignore the first couple of bits, because they are usually bad
	//~ {
	    //~ count++;
	    //~ total += counters[0];
	    //~ average = total/count;
	//~ }
	//~ else
	//~ {
	    //~ samples.push_back(M);
	    numdata += 1;
	    data[numdata] = PAPI_get_real_nsec() - PAPItime;
	    numdata += 1;
	    //~ previousResult = counters[0];
	    
	    total += counters[0];
	    numsamples += 1; //Add one to the number of samples
	    //~ average = total/(numsamples + 300);
	//~ }
    }
    
    /* Smooth out the data */
    for(int i = 0; i < numsamples*2; i += 2) {
		//~ if(data[i] > average + 25)
			//~ data[i] = average + 25;
		//~ if(data[i] < average - 25)
			//~ data[i] = average - 25;
			
		M(0) = data[i];
		samples.push_back(M);		
	}
    
    if( fp == NULL){
	printf("Error opening data dump! Aborting \n");
	exit(0);
    }
    //fprintf(fp, "Period: %d	Wait: %d\n", period, wait);

    
/* Analyze the collected data with dlib */    
    test.set_number_of_centers(2); //Tell the kkmeans object that we want to run the k-means with k = 2
    
    //Pick some initial centers for the k-means algorithm. Can use the 
    pick_initial_centers(2, initial_centers, samples, test.get_kernel()); 

/* Debugging */ 
//     cout << "Initial initial_centers = " << initial_centers[0] << endl;
//     cout << "Initial initial_centers = " << initial_centers[1] << endl;
//     
    find_clusters_using_kmeans(samples, initial_centers, 6000); 
//     test.train(samples, initial_centers, 2000); //Run the k-means algorithm
     
//      int currdata = 0;
//     previousResult = 0;
//     for( int i = 3; i < numsamples; i++){ //Dump the data
// 	int result;
// 	
// 	if(test(samples[i]) == center0) //The lowest center is a downward slope (aka after a 1 was sent)
// 	{
// 	    result = 0;
// 	    previousResult = result;
// 	}
// 	else if(test(samples[i]) == center2) //The highest center is a downward slope (aka after a 1 was sent)
// 	{
// 	    result = 1;
// 	    previousResult = result;
// 	}
// 	else{
// 	    result = previousResult;
// 	}
// 	
// 	fprintf(fp, "%lld %lld\n", data[currdata+1], data[currdata]);
// 	
// 	currdata += 2;
//     }
     
     
     
     
//     
//     cout << "initialcenters[0] = " << initial_centers[0] << endl;
//     cout << "initialcenters[1] = " << initial_centers[1] << endl; 
//     
//     cout << "Samples.size = " << samples.size() << endl;
//     cout << "Numsamples = " << numsamples << endl;
// 
//     cout << "Test.number_of_centers = " << test.number_of_centers() << endl;
//     cout << "Initial_centers.size = " << initial_centers.size() << endl;
     
     
//     /* Make sure that the lowered valued center is center0 */
    sample_type center0, center1;
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
    
    fprintf(fp, "%f %f\n", center0(0), center1(0));
    
    /* Loop over all samples and print out their predicted class. */
    int currdata = 0;
    for(int i = 0; i < numsamples; i++){ //Dump the data
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





