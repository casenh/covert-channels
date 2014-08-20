/* 
 * Authors: Casen Hunger
 *			Mohit Tiwari
 *			Alex Dimakis
 *			Ankit Rawat
 * 
 * Description: This is the receiving program for covert communication across the L1
 *				cache. The pair to this script is ./senderL1. The covert-channel is 
 *				constructed by the two programs placing contention for space in the L1
 *				cache. Information is transmitted through the cache by converting cache
 *				misses into bits.
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../../papi-5.3.0/src/papi.h"
#include "../../dlib-18.2/dlib/clustering.h"
#include "../../dlib-18.2/dlib/rand.h"

using namespace std;
using namespace dlib;

extern void getCompInfo(int);
extern void handle_error(int);
extern int PAPI_create_eventset(int*);


/* This fuction determines the correct name of the file that the information
 * is dumped into 
 */
void createDataFile(char *name, int fileType){
  
    char *word;  //The timestamp that we will create the filename from
    char *prefix; //The beginning of the filename
    time_t timestamp; //The timestamp that we take for creating the file

    if(fileType == 2){
		prefix = "../data/L1Cache-2.";
    }
    else{
		prefix = "../data/L1Cache.";
    }
    timestamp = time(NULL);
    word = ctime(&timestamp);

	/* Here we are picking out the parts of the timestamp that we would like in the file name */
    strcpy(name, prefix); 		    
	strncat(name, &word[4], 3);
    if(word[8] == ' '){ word[8] = '0'; } //Get rid of spaces in the title
    strncat(name, &word[8], 2);
    strcat(name, ".");
    strncat(name, &word[11], 2);
    strcat(name, ".");
    strncat(name, &word[14], 2);
    strcat(name, ".");
    strncat(name, &word[17], 2);

    return;
}


/* This function returns the current time returned by PAPI_get_real_nsec in	*
 * seconds. Max it returns in about 16 seconds								*/
long long int getTime(void){
  
 long long int time = PAPI_get_real_nsec();
 time = time & 0x00000000FFFFFFFF; //Chop of the top of the bits so we only get seconds
 return time;
}


/* The program initializes everything required for PAPI and dlib, and then probes
 * the L1 Cache for a given number of samples. 
 */
int NUM_OF_SETS = 0;
int BLOCK_LENGTH = 0;
int WAYS_ASSOCIATIVE = 0;
int main(int argc, char* argv[]){
    
/* Initializing everything for dlib */
    typedef matrix<double,1,1> sample_type; //Samples will be 2-D column vectors
    typedef radial_basis_kernel<sample_type> kerneltype; //Use a radial type kernel
    kcentroid<kerneltype> kc(kerneltype(0.1),.01,15); //This is the kcentroid object used in the kmeans test
    kkmeans<kerneltype> test(kc); //The kkmeans object that must be passed to the test
    sample_type M; //The matrix that holds our results
    std::vector<sample_type> samples; //The vector we will stort the results in
    std::vector<sample_type> initial_centers; //A vector to hold the inital centers we pick
     
     
/* Initialize all of the variables that will be used by the algorithm and PAPI */
    int numdata = 0, loc = 0, n = 0; //The number of samples that were collected
    int eventSet = PAPI_NULL; //The EventSet that we will use with PAPI
    long long int PAPItime, startTime; //Used to hold the starting time
    long long int counters[2]; //Stores the counter values (the number of events being monitored)
    long long int prevL2 = 0, prevL3 = 0; //Store the previous value to try to reduce outliers
    long long int *data = (long long int*) malloc(sizeof(long long int) * 15000000); //This array will store all of the data that is collected
    char* iterate; //The large array to iterate over
    FILE *fp; //File for writing translate data (1's and 0's)
    FILE *fp2; //File for writing the raw hardware counters value
    FILE *fp3; //File for keeping track of all the data files we have created
    char file[45], tempFile[45]; //Stores the name of the file that we want to write to
  
/* Create the files that will be used to store the data (MUST BE DONE FIRST) */    
    createDataFile(file, 0); //Create the file that the output will be stored in
    fp2 = fopen(file, "a");
    fp3 = fopen("../data/receiverFiles", "a");
    strcpy(tempFile, file);
    strcat(tempFile, "\n");
    fprintf(fp3, "%s", tempFile);
    strcat(file,"-2");
    fp = fopen(file, "a");
    if(fp == NULL){
		printf("Error opening data dump! Aborting \n");
		exit(0);
    }  
    
/* Get the cache info for the cpu */
    getCompInfo(1);
    
/* Allocate the memory that will be used to cause cache misses */
    iterate = (char*) malloc(sizeof(char) * (NUM_OF_SETS * WAYS_ASSOCIATIVE * BLOCK_LENGTH)); //Total cache size = number of sets * lines per set * cache line length    
    
/* Initialize the PAPI library and create the events that we wish to monitor */
    int retval = PAPI_library_init(PAPI_VER_CURRENT);
    if( retval != PAPI_VER_CURRENT) {
		fprintf(stderr,"PAPI library init error!\n");
		exit(1);
    }
    if( PAPI_create_eventset(&eventSet) != PAPI_OK){ handle_error(1);}   
    if(PAPI_add_event(eventSet, PAPI_L1_DCM) != PAPI_OK){ handle_error(1); } //Monitor L1 data cache
    if( PAPI_start(eventSet) != PAPI_OK){ handle_error(1); }
    
/* Here is the main loop. Cycles over the given array for 10 seconds while keeping track of the events and 	*
* publishing them to a file.											*/
    int period = atoi(argv[1]);
    int wait1 = atoi(argv[2]);
    int wait2 = atoi(argv[3]);
    int total = 0, numsamples = 0;
    int numberOfBitsSent;
    cin >> numberOfBitsSent; //Take in the number of bits that the sender is going to send

	/* Wait until the start signal to begin transmission */
    while(getTime() > 20000) {  }     
	PAPItime = PAPI_get_real_nsec(); //Take the start time (in total nanoseconds)
    startTime = getTime(); //Take the start time (in nanoseconds up to seconds)
    
    int index = 0;
    for(int i = 0; i < numberOfBitsSent; i++) //Take a sample for every bit the sender will send
	{

		while(getTime() < (startTime + (i * period))) { } //Only take once measurement per period length
	
		/* STEP 1: Prime the cache */
		while(getTime() < (startTime + wait1 + (i * period)))
		{
			for(int n = 0; n < NUM_OF_SETS; n++){  
				iterate[n * BLOCK_LENGTH] = 1;
			}
		}
	
		/* STEP 2: Wait for the sender to communicate across the cache */	
		while(getTime() < (startTime + (wait1 + wait2) + (i * period))) { }
	
		/* STEP 3: Probe the cache */	
		if( PAPI_reset(eventSet) != PAPI_OK) { handle_error(1); } 
	
		for(int n = 0; n < NUM_OF_SETS*2; n++)
		{
			iterate[n * BLOCK_LENGTH] = 1;
		}
	
		if( PAPI_read(eventSet, counters) != PAPI_OK){ handle_error(1);} //Read the hardware counters and record the results
	
		/* STEP 4: Dump the results */	
		total += counters[0]; //Keep an average for capping
		M(0) = counters[0]; //Store the counters as a sample	
		data[numdata] = counters[0]; //Dump the data
		numdata += 1;
		data[numdata] = PAPI_get_real_nsec() - PAPItime;
		numdata += 1;
		samples.push_back(M); //Store the sample to be analyzed
		numsamples += 1; //Add one to the number of samples
	}
    
	/* Smooth out the values for better clustering */
	int average = total/numsamples;
	for(int i = 0; i < numdata; i += 2) {
		/*if(data[i] > average + 5)
			data[i] = average + 5;
		else if(data[i] < average - 5)
			data[i] = average - 5;*/
	}
    
	/* Analyze the collected data with dlib */    
    pick_initial_centers(2, initial_centers, samples, test.get_kernel());
    find_clusters_using_kmeans(samples, initial_centers, 6000);
    
	/* Determine which center is lowest (0) and highest (1) */
    sample_type center0, center1;  
    if(initial_centers[0] < initial_centers[1]) {
		center0 = initial_centers[0];
		center1 = initial_centers[1];   
    }
    else {
		center1 = initial_centers[0];
		center0 = initial_centers[1];
    }  
    
    /* Loop over all samples and print out their predicted class. */
    int currdata = 0;
	for( int i = 0; i < numsamples; i++){ //Dump the data
		int result;
		sample_type currSample = samples[i];
		int Distance0 = abs(center0(0) - currSample(0));
		int Distance1 = abs(center1(0) - currSample(0));
		if(Distance0 < Distance1) {
			result = 0;
		}
		else {
			result = 1;
		}

		fprintf(fp, "%lld %lld\n", data[currdata+1], data[currdata]);
		fprintf(fp2, "%lld %d\n", data[currdata+1], result);
		currdata += 2;
	}

	/* Cleanup */
    fclose(fp); 
	fclose(fp2);
    fclose(fp3);
    free(data);
    free(iterate);
    PAPI_stop(eventSet, counters);
    PAPI_cleanup_eventset(eventSet);
    PAPI_destroy_eventset(&eventSet);
    
    return 0;
}

