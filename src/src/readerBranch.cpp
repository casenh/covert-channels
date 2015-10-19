//This file will used unsupervised clustering to determine if the program is sending a 0 or 1

#include <iostream>
#include <vector>
#include <fstream>
#include "../../papi-5.3.0/src/papi.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <chrono>

#include "../../dlib-18.2/dlib/clustering.h"
#include "../../dlib-18.2/dlib/rand.h"

//#define PERF_CTRS
#define RISCV

using namespace std;
using namespace dlib;
using namespace std::chrono;

extern void handle_error(int);
extern int PAPI_create_eventset(int*);


/* This fuction determines the correct name of the data file that should be produced 	*
 * It returns the value of the data in the counters array to dump 			*/
void createDataFile(char *name, int argc){
  
    char *word, *prefix;
    time_t timestamp;
    int n = 0, m = 0; //Used for looping
    int data = 0; //Used to determine which data to dump

    fflush(NULL);
    if(argc == 2){
		prefix = "../data/BranchIns-2.";
    }
    else{
		prefix = "../data/BranchIns.";
    }
    timestamp = time(NULL);
    word = ctime(&timestamp);
    strcpy(name, prefix); 	//Here we are picking out the parts of the timestamp that we would like in the file name
    strncat(name, &word[4], 3);
    if(word[8] == ' '){ word[8] = '0'; } //Get rid of spaces in the title
    strncat(name, &word[8], 2);
    strcat(name, ".");
    strncat(name, &word[11], 2);
    strcat(name, ".");
    strncat(name, &word[14], 2);
    strcat(name, ".");
    strncat(name, &word[17], 2);


//     printf("Data being saved to %s \n", name);
    return;
}

long long getCycles()
{
   long long cycles = 1337;
#ifdef RISCV
   __asm__ __volatile__( "rdcycle %0" : "=r" (cycles) );
#endif
   return cycles;
}


/* This function returns the current time returned by PAPI_get_real_nsec with	*
 * a truncated version								*/
long long int getTime(void){
	
   long long int time;
#ifdef PERF_CTRS
	time = PAPI_get_real_nsec();
#else
   time = getCycles();
#endif
	time = time & 0x00000000FFFFFFFF; 

	return time;
 
}

void readBranches(int);
int main(int argc, char* argv[]){
    
/* Initializing everything for dlib */
    typedef matrix<double,1,1> sample_type; //Samples will be 2-D column vectors
    typedef radial_basis_kernel<sample_type> kerneltype; //Use a radial type kernel
    kcentroid<kerneltype> kc(kerneltype(0.1),.01,15);
    kkmeans<kerneltype> test(kc);
    
    std::vector<sample_type> samples;
    std::vector<sample_type> initial_centers; 
    sample_type M;
 
/* Initialize all of the variables that will be used by the algorithm and PAPI */
    int numsamples = 0, numdata = 0, loc = 0, n = 0; //The number of samples that were collected
#ifdef PERF_CTRS
    int eventSet = PAPI_NULL; //The EventSet that we will use with PAPI
#endif
    long long int PAPItime, startTime; //Used to hold the starting time
    long long int counters[2]; //Stores the counter values (the number of events being monitored)
    long long int prevL2 = 0, prevL3 = 0; //Store the previous value to try to reduce outliers
    char** iterate; //The large array to iterate over
    FILE *fp, *fp2, *fp3; //The file that we will write the data to
    char file[45], tempFile[45]; //Stores the name of the     cout << "Numsamples = " << numsamples << endl;file that we want to write to
    
    
    createDataFile(file, 0); //Create the file that the output will be stored in
    long long int *data = (long long int*) malloc(sizeof(long long int) * 15000000); //This array will store all of the data that is collected
    
/* Initialize the PAPI library and create the events that we wish to monitor */
#ifdef PERF_CTRS
    int retval = PAPI_library_init(PAPI_VER_CURRENT);
    if( retval != PAPI_VER_CURRENT) {
		fprintf(stderr,"PAPI library init error!\n");
		exit(1);
    }
    if( PAPI_create_eventset(&eventSet) != PAPI_OK){ handle_error(1);}   
    if(PAPI_add_event(eventSet, PAPI_BR_PRC) != PAPI_OK){ handle_error(1); } //Branches predicted correctly
    if( PAPI_start(eventSet) != PAPI_OK){ handle_error(1); }
#endif
    
    
/* Here is the main loop. Cycles over the given array for 10 seconds while keeping track of the events and 	*
* publishing them to a file.											*/
//     cout << "Working..." << endl;
    int numberOfBitsSent;
    int period = atoi(argv[1]); //Read in the period and time to wait
    int numBranches = atoi(argv[2]);
    int average = 0;
    int total = 0;
    cin >> numberOfBitsSent;
    while(getTime() > 20000) {  }
#ifdef PERF_CTRS
    PAPItime = PAPI_get_real_nsec();
#endif
    startTime = getTime();
    int temp = 0;
	long long int probeTime;
    for(int n = 0; n < numberOfBitsSent; n++){ //Only run for 5 seconds (time returned in nano seconds)

		while(getTime() < (startTime + 0 + (n * period))) { } //Wait to begin taking the measurements

		#ifdef PERF_CTRS
		if( PAPI_reset(eventSet) != PAPI_OK) { handle_error(1); }
		#else
		//high_resolution_clock::time_point chrono_start = std::chrono::high_resolution_clock::now();
      long long chrono_start = getTime();
		#endif


		//~ long long int tempTime = PAPI_get_real_nsec();
		readBranches(numBranches);
		//~ cerr << PAPI_get_real_nsec() - tempTime << endl;
	
		#ifdef PERF_CTRS
		if( PAPI_read(eventSet, counters) != PAPI_OK){ handle_error(1);} //Continue to add up the amount of cache misses after each iteration

		#else
		//high_resolution_clock::time_point chrono_end = std::chrono::high_resolution_clock::now() - probeTime;
      long long chrono_end = getTime() - probeTime;
		#endif

		/* Save the results */
		#ifdef PERF_CTRS
		total += counters[0];
		numsamples += 1; //Add one to the number of samples
		data[numdata] = counters[0]; //Dump the data
		numdata += 1;
		data[numdata] = getTime();
		numdata += 1;
		#else
		//duration<double> time_span = duration_cast<duration<double>>(chrono_end - chrono_start);
      long long time_span = chrono_end - chrono_start;
		total += time_span;
		numsamples += 1; //Add one to the number of samples
		data[numdata] = time_span;//Dump the data
		numdata += 1;
		data[numdata] = getTime();
		numdata += 1;
      #endif
    }
//     cout << "Average time for reader: " << (PAPI_get_real_nsec() - PAPItime) << endl;

	/* Smooth out the data */
	average = total/numsamples;
	for(int i = 0; i < numsamples*2; i += 2) {
		if(data[i] > average + 5)
			data[i] = average + 5;
		if(data[i] < average - 5)
			data[i] = average - 5;
		M(0) = data[i];
		samples.push_back(M);
	}

    
    fp2 = fopen(file, "a");
    fp3 = fopen("../data/receiverFiles", "a");
    strcpy(tempFile, file);
    strcat(tempFile, "\n");
    fprintf(fp3, "%s", tempFile);
    strcat(file,"-2_");
    string rawDataFile(file);
    char buf[50];
    sprintf(buf, "%d", period);
    string tmp(buf);
    rawDataFile.append(tmp);
    fp = fopen(&rawDataFile[0], "a");
    if( fp == NULL){
	printf("Error opening data dump! Aborting \n");
	exit(0);
    }


/* Analyze the collected data with dlib */    
    test.set_number_of_centers(2); //Tell the kkmeans object that we want to run the k-means with k = 2
    
    //Pick some initial centers for the k-means algorithm. Can use the 
    pick_initial_centers(2, initial_centers, samples, test.get_kernel()); 

/* Debugging */ 
//     cout << "Initial initial_centers = " << initial_centers[0] << endl;
//     cout << "Initial initial_centers = " << initial_centers[1] << endl;
    
    /* Runs the kmeans algorithm separating the data */
    find_clusters_using_kmeans(samples, initial_centers, 6000); 

     
    //~ /* Make sure that the lowered valued center is center0 */
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
    for( int i = 0; i < samples.size(); i++){ //Loop through all the data points
	int result;
	sample_type currSample = samples[i];
	int Distance0 = abs(center0(0) - currSample(0));
	int Distance1 = abs(center1(0) - currSample(0));
	if(Distance0 < Distance1) //Determine what center the data point we looked at is closer to
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
    
    //~ int result = 0;
    //~ for(int i = 0; i < numsamples*2; i += 2) {
		//~ 
		//~ if(i != 0) {
			//~ if((data[i] - data[i-2]) > 0)
				//~ result = 1;
			//~ if((data[i] - data[i-2]) < 0)
				//~ result = 0;
		//~ }
		//~ 
		//~ fprintf(fp, "%lld %lld\n", data[i+1], data[i]);
		//~ fprintf(fp2, "%lld %d\n", data[i+1], result);
	//~ }
    //~ fprintf(fp2, "%lld %d\n", data[numsamples*2+1], 0);
    
    
    
    
    
    
    fclose(fp);
    fclose(fp2);
    fclose(fp3);
    free(data);
    
    return 0;
}


/* THESE FUNCTIONS ARE NOT USED, JUST KEPT IN CASE THEY'RE NEEDED */
/* This function returns the lowest value out of the three centers */
// template <class T> matrix<double,1,1> findLowerCenter (T initialCenters){
//     
//     if(initialCenters[0] < initialCenters[1] && initialCenters[0] < initialCenters[2])
//     {
// 	return initialCenters[0];
//     }
//     else if(initialCenters[1] < initialCenters[0] && initialCenters[1] < initialCenters[2])
//     {
// 	return initialCenters[1];
//     }
//     else 
//     {
// 	return initialCenters[2];
//     }    
// }
// 
// 
// /* This function returns the middle value out of the three centers */
// template <class T> matrix<double,1,1> findMiddleCenter (T initialCenters){
//     
//     if(initialCenters[0] < initialCenters[1] && initialCenters[0] > initialCenters[2])
//     {
// 	return initialCenters[0];
//     }
//     else if(initialCenters[1] < initialCenters[0] && initialCenters[1] > initialCenters[2])
//     {
// 	return initialCenters[1];
//     }
//     else 
//     {
// 	return initialCenters[2];
//     }    
// }
// 
// 
// /* This function returns the highest value out of the three centers */
// template <class T> matrix<double,1,1> findUpperCenter (T initialCenters){
//     
//     if(initialCenters[0] > initialCenters[1] && initialCenters[0] > initialCenters[2])
//     {
// 	return initialCenters[0];
//     }
//     else if(initialCenters[1] > initialCenters[0] && initialCenters[1] > initialCenters[2])
//     {
// 	return initialCenters[1];
//     }
//     else 
//     {
// 	return initialCenters[2];
//     }    
// }



void readBranches(int numBranches){

    double randNum;
    long long int startTime;
    srand(time(NULL));
   
	randNum = std::rand(); 
	if(randNum > (RAND_MAX/2))
	    randNum += 1;
	else
	    randNum -= 52;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 2;
	else
	    randNum -= 53;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 3;
	else
	    randNum -= 54;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 4;
	else
	    randNum -= 55;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 5;
	else
	    randNum -= 56;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 6;
	else
	    randNum -= 57;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 7;
	else
	    randNum -= 58;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 8;
	else
	    randNum -= 59;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 9;
	else
	    randNum -= 60;
	    
	randNum = std::rand(); 
	if(randNum > (RAND_MAX/2))
	    randNum += 10;
	else
	    randNum -= 521;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 12;
	else
	    randNum -= 531;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 13;
	else
	    randNum -= 541;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 14;
	else
	    randNum -= 551;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 15;
	else
	    randNum -= 561;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 16;
	else
	    randNum -= 571;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 17;
	else
	    randNum -= 581;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 18;
	else
	    randNum -= 591;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 19;
	else
	    randNum -= 601;
	    
	randNum = std::rand(); 
	if(randNum > (RAND_MAX/2))
	    randNum += 1;
	else
	    randNum -= 52;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 2;
	else
	    randNum -= 53;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 3;
	else
	    randNum -= 54;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 4;
	else
	    randNum -= 55;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 5;
	else
	    randNum -= 56;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 6;
	else
	    randNum -= 57;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 7;
	else
	    randNum -= 58;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 8;
	else
	    randNum -= 59;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 9;
	else
	    randNum -= 60;
	    
	randNum = std::rand(); 
	if(randNum > (RAND_MAX/2))
	    randNum += 10;
	else
	    randNum -= 521;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 12;
	else
	    randNum -= 531;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 13;
	else
	    randNum -= 541;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 14;
	else
	    randNum -= 551;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 15;
	else
	    randNum -= 561;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 16;
	else
	    randNum -= 571;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 17;
	else
	    randNum -= 581;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 18;
	else
	    randNum -= 591;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 19;
	else
	    randNum -= 601;
	    
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 6;
	else
	    randNum -= 571;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 73;
	else
	    randNum -= 582;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 83;
	else
	    randNum -= 593;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 9;
	else
	    randNum -= 603;
	    
	randNum = std::rand(); 
	if(randNum > (RAND_MAX/2))
	    randNum += 103;
	else
	    randNum -= 523;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 123;
	else
	    randNum -= 5313;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 133;
	else
	    randNum -= 5413;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 143;
	else
	    randNum -= 5513;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 153;
	else
	    randNum -= 5631;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 136;
	else
	    randNum -= 5731;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 137;
	else
	    randNum -= 5831;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 138;
	else
	    randNum -= 5931;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 139;
	else
	    randNum -= 6031;
	    
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 6;
	else
	    randNum -= 571;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 73;
	else
	    randNum -= 582;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 83;
	else
	    randNum -= 593;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 9;
	else
	    randNum -= 603;
	    
	randNum = std::rand(); 
	if(randNum > (RAND_MAX/2))
	    randNum += 103;
	else
	    randNum -= 523;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 123;
	else
	    randNum -= 5313;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 133;
	else
	    randNum -= 5413;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 143;
	else
	    randNum -= 5513;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 153;
	else
	    randNum -= 5631;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 136;
	else
	    randNum -= 5731;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 137;
	else
	    randNum -= 5831;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 138;
	else
	    randNum -= 5931;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 139;
	else
	    randNum -= 6031;
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 6;
	else
	    randNum -= 571;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 73;
	else
	    randNum -= 582;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 83;
	else
	    randNum -= 593;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 9;
	else
	    randNum -= 603;
	    
	randNum = std::rand(); 
	if(randNum > (RAND_MAX/2))
	    randNum += 103;
	else
	    randNum -= 523;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 123;
	else
	    randNum -= 5313;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 133;
	else
	    randNum -= 5413;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 143;
	else
	    randNum -= 5513;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 153;
	else
	    randNum -= 5631;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 136;
	else
	    randNum -= 5731;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 137;
	else
	    randNum -= 5831;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 138;
	else
	    randNum -= 5931;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 139;
	else
	    randNum -= 6031;
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 6;
	else
	    randNum -= 571;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 73;
	else
	    randNum -= 582;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 83;
	else
	    randNum -= 593;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 9;
	else
	    randNum -= 603;
	    
	randNum = std::rand(); 
	if(randNum > (RAND_MAX/2))
	    randNum += 103;
	else
	    randNum -= 523;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 123;
	else
	    randNum -= 5313;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 133;
	else
	    randNum -= 5413;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 143;
	else
	    randNum -= 5513;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 153;
	else
	    randNum -= 5631;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 136;
	else
	    randNum -= 5731;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 137;
	else
	    randNum -= 5831;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 138;
	else
	    randNum -= 5931;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 139;
	else
	    randNum -= 6031;
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 6;
	else
	    randNum -= 571;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 73;
	else
	    randNum -= 582;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 83;
	else
	    randNum -= 593;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 9;
	else
	    randNum -= 603;
	    
	randNum = std::rand(); 
	if(randNum > (RAND_MAX/2))
	    randNum += 103;
	else
	    randNum -= 523;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 123;
	else
	    randNum -= 5313;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 133;
	else
	    randNum -= 5413;
	
	randNum = std::rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 143;
	else
	    randNum -= 5513;
	    
	    
	    
/***************************/	    
	
	if(numBranches >= -2) {
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
			
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum = std::rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 66;
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 3;
		else
			randNum -= 55;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 44;
		else
			randNum -= 582;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 65803;
			
		randNum = std::rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 68;
		else
			randNum -= 87;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 7;
		else
			randNum -= 3456;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 436;
		else
			randNum -= 234;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 2;
		else
			randNum -= 1;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 12153;
		else
			randNum -= 56231;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 11;
		else
			randNum -= 34;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
	}
	
	
	
/**************************************/	
	if(numBranches >= -1) {
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum = std::rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum = std::rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
			
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum = std::rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 345;
		else
			randNum -= 345;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 3345;
		else
			randNum -= 593;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum = std::rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 32;
		else
			randNum -= 5631;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 11;
		else
			randNum -= 5731;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 111;
		else
			randNum -= 5831;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 222;
		else
			randNum -= 444;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 333;
		else
			randNum -= 555;
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 77;
		else
			randNum -= 666;
		}
	
	
	
/************************************/	
	
	if(numBranches >= 0) {
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 54;
		else
			randNum -= 888;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum = std::rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum = std::rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
			
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum = std::rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum = std::rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum = std::rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
	}
	
	
/*************************************/
	if(numBranches >= 1) {

		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum = std::rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
			
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum = std::rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum = std::rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
	}
	    
	    
	    
/***********************************************/

	if(numBranches >= 2) {
	
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 222;
		else
			randNum -= 444;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 333;
		else
			randNum -= 555;
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 77;
		else
			randNum -= 666;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 54;
		else
			randNum -= 888;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum = std::rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum = std::rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
			
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum = std::rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum = std::rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		}
		
/***********************************************************/	
	
	if(numBranches >= 3) {
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum = std::rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum = std::rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
			
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum = std::rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum = std::rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum = std::rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		}
	
	
	
	
	
	
	//~ fflush(NULL);
}


