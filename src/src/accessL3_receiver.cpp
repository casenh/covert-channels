//This file will used unsupervised clustering to determine if the program is sending a 0 or 1
#include <iostream>
#include <vector>
#include <fstream>
#include "../../papi-5.3.0/src/papi.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>

#include "../../dlib-18.2/dlib/clustering.h"
#include "../../dlib-18.2/dlib/rand.h"

#define NUM_INDICES 256 //The number of cache lines to hit
#define NUM_TRIALS 40

using namespace std;
using namespace dlib;

typedef matrix<double,1,1> sample_type; //Samples will be 2-D column vectors
typedef radial_basis_kernel<sample_type> kerneltype; //Use a radial type kernel

extern void getCompInfo(int);
extern void handle_error(int);
extern int PAPI_create_eventset(int*);
extern void RandomNumberGenerator(std::vector<int>*, int);
extern void analyzeData(FILE*, FILE*, std::vector<sample_type>&, long long int*, int);
extern bool coordinateVM_Receiver(int);
extern bool handshakeReceiver(char**);
extern void iterateOver();

extern long long int waitTime;
extern long long int timeDifference;
extern char* Addresses[];

char* iterate; //Chunk of memory we use to communicate over
long long int* results; //Pointer to the memory that is used to store the raw data
int NUM_OF_SETS = 0, BLOCK_LENGTH = 0, WAYS_ASSOCIATIVE = 0; //Cache info
int period, wait1, wait2; //Communication info



/* This fuction determines the correct name of the data file that should be produced 	*
 * It returns the value of the data in the counters array to dump 			*/
void createDataFile(char *name, int fileType){
  
    char *word;  //The timestamp that we will create the filename from
    char *prefix; //The beginning of the filename
    time_t timestamp; //The timestamp that we take for creating the file

    if(fileType == 2){
	prefix = "../data/L3Cache-2.";
    }
    else{
	prefix = "../data/L3Cache.";
    }
    timestamp = time(NULL);
    word = ctime(&timestamp);
    strcpy(name, prefix); 		//Here we are picking out the parts of the timestamp that we would like in the file name
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
 * seconds. Max it returns in about 16 seconds									*/
long long int getTime(void){
  
 long long int time = PAPI_get_real_nsec();
 time = time & 0x00000000FFFFFFFF; //Chop of the top of the bits so we only get seconds
 return time;
}


/* Receives in a given number of samples from the channel and outputs it to the vector that	*
 * is passed in.																			*/
int receiveCommunication(std::vector<sample_type> &samples, int numOfSamples) {
	
	/* Create the correct PAPI variables */
	int sampleNumber = 0, numdata = 0, loc = 0, n = 0; //The number of samples that were collected
    int eventSet = PAPI_NULL; //The EventSet that we will use with PAPI
    long long int counters[2]; //Stores the counter values (the number of events being monitored)
    int total = 0;
    //~ long long int prevL2 = 0, prevL3 = 0; //Store the previous value to try to reduce outliers
    sample_type M;
    
    /* Initialize the PAPI library and create the events that we wish to monitor */
    int retval = PAPI_library_init(PAPI_VER_CURRENT);
    if( retval != PAPI_VER_CURRENT) {
	fprintf(stderr,"PAPI library init error!\n");
	exit(1);
    }
    if( PAPI_create_eventset(&eventSet) != PAPI_OK){ handle_error(1);}   
    if(PAPI_add_event(eventSet, PAPI_L3_TCM) != PAPI_OK){ handle_error(1); }
    if( PAPI_start(eventSet) != PAPI_OK){ handle_error(1); }
	
	/* First, wait for the start signal */
	while(getTime() > 20000) { }

	/* Now, take in the starting times and begin reading the channel */
	long long int startTime = PAPI_get_real_nsec(); //Take the start time (in nanoseconds up to seconds)
	int index = 0, average = 1;
	
	int temp = 0;
	for(int i = 0; i < numOfSamples; i++) //Take a sample for every bit the sender will send
	{ 
		while(PAPI_get_real_nsec() < (startTime + (i * period))) { } //Only take once measurement per period length

		/* Prime the cache */
		for(int n = 0; n < NUM_INDICES; n++)
			temp += *(Addresses[n]);

		/* Wait for the sender to communicate */
		while(PAPI_get_real_nsec() < (startTime + (wait1 + wait2) + (i * period))) { } 
		
		/* Probe the cache */
		PAPI_reset(eventSet);
		for(int n = 0; n < NUM_INDICES; n++)
			temp += *(Addresses[n]);
		PAPI_read(eventSet, counters); 

		/* Store the results */
		results[numdata] = counters[0]; 
		total += counters[0];
		numdata += 1;
		results[numdata] = PAPI_get_real_nsec() - startTime;
		numdata += 1;
	
		sampleNumber += 1; //Add one to the number of samples
	}
	
	average = total/(sampleNumber);
	//cerr << average << endl;
	for(int i = 0; i < sampleNumber; i++) {
		if(results[i*2] > average + 10)
			results[i*2] = average + 10;
		//if(results[i*2] < average - 10)
		//	results[i*2] = average - 10;
		
		M(0) = results[i*2];
		samples.push_back(M);
	}

	/* Perform cleanup */
	PAPI_stop(eventSet, counters);
    PAPI_cleanup_eventset(eventSet);
    PAPI_destroy_eventset(&eventSet);
}


int main(int argc, char* argv[]){
    
/* Initializing everything for dlib */
    //~ kcentroid<kerneltype> kc(kerneltype(0.1),.01,15); //This is the kcentroid object used in the kmeans test
    //~ kkmeans<kerneltype> test(kc); //The kkmeans object that must be passed to the test
    
    //~ sample_type M; //The matrix that holds our results
	//std::vector<sample_type> samplesArray[10];
	std::vector<sample_type> samples;
	results = (long long int*) malloc(sizeof(long long int) * 30000);
    //std::vector<std::vector<sample_type> > samplesArray; //The vector we will stort the results in
    //~ std::vector<sample_type> initial_centers; //A vector to hold the inital centers we pick
     
     
/* Initialize all of the variables that will be used by the algorithm and PAPI */
	//data = (long long int*) malloc(sizeof(long long int) * 15000000); //This array will store all of the data that is collected
    FILE *rawData; //File for writing translate data (1's and 0's)
    FILE *clusteredData; //File for writing the raw hardware counters value
    FILE *receiverFiles; //File for keeping track of all the data files we have created
    char file[45], tempFile[45]; //Stores the name of the file that we want to write to
  
/* Create the files that will be used to store the data (MUST BE DONE FIRST) */    
    createDataFile(file, 0); //Create the file that the output will be stored in
    string timeStamp(&file[19]);
	strcat(file,"-A");
	string clusteredFilename(file);
    clusteredData = fopen(file, "a");
    receiverFiles = fopen("../data/receiverFiles", "a");
    strcpy(tempFile, file);
    strcat(tempFile, "\n");
    fprintf(receiverFiles, "%s", tempFile);
    strcat(file,"-2");
    rawData = fopen(file, "a");
	string rawDataFilename(file);
    
/* Read in the conditions for communication */
    period = atoi(argv[1]);
    wait1 = atoi(argv[2]);
    wait2 = atoi(argv[3]);
    
/* Do the handshake. This requires getting the L1 Cache info first */
    getCompInfo(3);
    bool handshakeComplete;
    do {
		handshakeComplete = handshakeReceiver(&iterate);
	} while(!handshakeComplete);    

	/* Read in the sequence from the sender */
	string sequence;
	cin >> sequence;
	
	int numSamples;
	cin >> numSamples;

	/* Carry out the communication */
	receiveCommunication(samples, numSamples);

	/* Analyze the collected data with dlib */  
	//~ analyzeData(rawData, clusteredData, samples[i], dataArray[i], 10000);
	
	for(int n = 0; n < numSamples*2; n += 2) {
		int point;
		if(results[n] > 0)	
			point = 1;
		else	
			point = 0;

		fprintf(clusteredData, "%lld %d\n", results[n+1],  point);
		fprintf(rawData, "%lld %lld\n", results[n+1],  results[n]);
	}
    
    /* Output the sent sequence to a file */
    string senderFileName("../data/Sender.");
    senderFileName.append(timeStamp);
    FILE* senderFiles = fopen("../data/senderFiles", "a");
    fprintf(senderFiles, "%s\n", &senderFileName[0]);
    FILE* senderFile = fopen(&senderFileName[0], "a");
    fprintf(senderFile, "%s", &sequence[0]);
    fclose(senderFiles);
    fclose(senderFile);
    
    fclose(rawData); //Perform cleanup
    fclose(clusteredData);
    fclose(receiverFiles);
    free(results);
	munlockall();    
    return 0;
}





