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


/* This function returns the current time returned by PAPI_get_real_nsec with	*
 * a truncated version								*/
long long int getTime(void){
  
 long long int time = PAPI_get_real_nsec();
 time = time & 0x00000000FFFFFFFF; 
 return time;
 
}


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
    int eventSet = PAPI_NULL; //The EventSet that we will use with PAPI
    long long int PAPItime, startTime; //Used to hold the starting time
    long long int counters[2]; //Stores the counter values (the number of events being monitored)
    long long int prevL2 = 0, prevL3 = 0; //Store the previous value to try to reduce outliers
    char** iterate; //The large array to iterate over
    FILE *fp, *fp2, *fp3; //The file that we will write the data to
    char file[45], tempFile[45]; //Stores the name of the     cout << "Numsamples = " << numsamples << endl;file that we want to write to
    
    
    createDataFile(file, 0); //Create the file that the output will be stored in
    long long int *data = (long long int*) malloc(sizeof(long long int) * 15000000); //This array will store all of the data that is collected
    
/* Initialize the PAPI library and create the events that we wish to monitor */
    int retval = PAPI_library_init(PAPI_VER_CURRENT);
    if( retval != PAPI_VER_CURRENT) {
	fprintf(stderr,"PAPI library init error!\n");
	exit(1);
    }
    if( PAPI_create_eventset(&eventSet) != PAPI_OK){ handle_error(1);}   
    if(PAPI_add_event(eventSet, PAPI_BR_PRC) != PAPI_OK){ handle_error(1); } //Branches predicted correctly
    if( PAPI_start(eventSet) != PAPI_OK){ handle_error(1); }
    
    
/* Here is the main loop. Cycles over the given array for 10 seconds while keeping track of the events and 	*
* publishing them to a file.											*/
//     cout << "Working..." << endl;
    int numberOfBitsSent;
    int period = atoi(argv[1]); //Read in the period and time to wait
    int wait = atoi(argv[2]);
    int average = 0;
    int total = 0;
    long long int previousResult = 300000;
    cin >> numberOfBitsSent;
    while(getTime() > 20000) {  }
    PAPItime = PAPI_get_real_nsec();
    startTime = getTime();
    for(int i = 0; i < numberOfBitsSent; i++){ //Only run for 5 seconds (time returned in nano seconds)

		while(getTime() < (startTime + 1000 + (i * period))) { } //Wait to begin taking the measurements
		if( PAPI_reset(eventSet) != PAPI_OK) { handle_error(1); }
		long long int time = PAPI_get_real_nsec();

		while((PAPI_get_real_nsec() - time) < wait) {
					for(int n = 0; n < 100; n++) {
						if(n > 0)
							average++;
						else
							average--;
					}
		}
		if( PAPI_read(eventSet, counters) != PAPI_OK){ handle_error(1);} //Continue to add up the amount of cache misses after each iteration

		/* Ignore outliers of the data */
		//~ if(counters[0] > (average - 50)){
			total += counters[0];
			numsamples += 1; //Add one to the number of samples
			//~ average = total/numsamples;
		//~ }
			
			//~ if(numsamples > 0) //Used for capping the values (better sorting later)
			//~ {
	    
				//~ if(counters[0] < (average - 25)) //Prevent outliers from scewing the data
				//~ {
				//~ counters[0] = (average - 25);
				//~ }
				//~ if(counters[0] > (average + 25)) //Prevent outliers from scewing the data
				//~ {
				//~ counters[0] = (average + 25);
				//~ }
				//~ M(0) = counters[0]; //Store the counters as a sample
				data[numdata] = counters[0]; //Dump the data
				//~ samples.push_back(M);
				numdata += 1;
				data[numdata] = getTime();
				numdata += 1;
				previousResult = counters[0];
			//~ }
		//}
    }
//     cout << "Average time for reader: " << (PAPI_get_real_nsec() - PAPItime) << endl;

	/* Smooth out the data */
	average = total/numsamples;
	for(int i = 0; i < numsamples*2; i += 2) {
		if(data[i] > average + 25)
			data[i] = average + 25;
		if(data[i] < average - 25)
			data[i] = average - 25;
		M(0) = data[i];
		samples.push_back(M);
	}

    
    fp2 = fopen(file, "a");
    fp3 = fopen("../data/receiverFiles", "a");
    strcpy(tempFile, file);
    strcat(tempFile, "\n");
    fprintf(fp3, "%s", tempFile);
    strcat(file,"-2");
    fp = fopen(file, "a");
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

     
    /* Make sure that the lowered valued center is center0 */
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
    previousResult = 0;
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


