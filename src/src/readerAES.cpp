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
long long int IGNORE = -1;
//#define CHECK_TIME

using namespace std;
using namespace dlib;
typedef matrix<double,1,1> sample_type;

extern void handle_error(int);
extern int PAPI_create_eventset(int*);
extern void analyzeData(FILE* rawData, FILE* clusteredResults, std::vector<sample_type> &samples, long long int* data, int numSamples);


/* This fuction determines the correct name of the data file that should be produced 	*
 * It returns the value of the data in the counters array to dump 			*/
void createDataFile(char *name, int argc){
  
    char *word, *prefix;
    time_t timestamp;
    int n = 0, m = 0; //Used for looping
    int data = 0; //Used to determine which data to dump

    fflush(NULL);
    if(argc == 2){
		prefix = (char *) "../data/AES-2.";
    }
    else{
		prefix = (char *) "../data/AES.";
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


int main(int argc, char* argv[]){
    
/* Initializing everything for dlib */
    typedef matrix<double,1,1> sample_type; //Samples will be 2-D column vectors
    typedef radial_basis_kernel<sample_type> kerneltype; //Use a radial type kernel
    kcentroid<kerneltype> kc(kerneltype(0.1),.01,15);
    kkmeans<kerneltype> test(kc);
    IGNORE = atoll(argv[4]);
    
    std::vector<sample_type> samples;
    std::vector<sample_type> trainingSamples;
    std::vector<sample_type> initial_centers; 
    sample_type M;    
 
/* Initialize all of the variables that will be used by the algorithm and PAPI */
    int numsamples = 0, numdata = 0, loc = 0, n = 0; //The number of samples that were collected
    int eventSet = PAPI_NULL; //The EventSet that we will use with PAPI
    int temp;
    long long int PAPItime, startTime; //Used to hold the starting time
    long long int counters[2]; //Stores the counter values (the number of events being monitored)
    long long int prevL2 = 0, prevL3 = 0; //Store the previous value to try to reduce outliers
    char** iterate; //The large array to iterate over
    FILE *fp, *fp2, *fp3, *fp_reader; //The file that we will write the data to
    char file[256], tempFile[256]; //Stores the name of the     cout << "Numsamples = " << numsamples << endl;file that we want to write to
    int array[5000];
    int *memory_buffer = (int *) malloc(MEM_BUFFER_SIZE*sizeof(int));	
    
#define ROUND_NUM 1    
    createDataFile(file, 0); //Create the file that the output will be stored in
    long long int data[ROUND_NUM][2*250*1000];// = (long long int [20][20000]) malloc(sizeof(long long int) * 20*20000); //This array will store all of the data that is collected
    fp_reader = fopen("../data/reader.txt", "w");
    
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
    long long int period = atoll(argv[1]);
    //long long int wait = atoll(argv[2]);;
    long long int average = 0;
    long long int total = 0;
    long long int previousResult = 300000;
    std::cin >> numberOfBitsSent;
    //cerr << "numberOfBitsSent " << numberOfBitsSent << endl;
    //cerr << "argv[1] " << argv[1] << endl;
    while(getTime() > 20000) {  }
    PAPItime = PAPI_get_real_nsec();
    
    int count = 0;
    startTime = PAPI_get_real_nsec();
	int inner_loop_iter = atoi(argv[3]);      
	__m128i a = _mm_setr_epi32(0x34897346, 0xa6b82047, 0xc9d8f3e7, 0x84198327), /* low dword first! */
			b = _mm_setr_epi32(0x12576456, 0x23889235, 0xabefdcba, 0x92783746);   
	
	 __asm__ volatile  (
        "movdqa %0, %%xmm0;"      /* xmm0 <- a */
        "movdqa %1, %%xmm1;"      /* xmm1 <- b */
        "movdqa %0, %%xmm2;"      /* xmm0 <- a */
        "movdqa %1, %%xmm3;"      /* xmm1 <- b */   
        "movdqa %1, %%xmm4;"      /* xmm1 <- b */        
        //"movdqa %%xmm0, %0;"      /* x <- xmm0 */

        : 	          /* output operand, %0 */
        :"x"(a), "x"(b)   /* input operands, %1, %2 */
        :"%xmm0","%xmm1"  /* clobbered registers */
    );    
    
    //27-60 ns per block when sending 1s
    //14-15 ns per block when sending 0s
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
			  __asm__ volatile ( 
				"aesenc %xmm0, %xmm1\n\t"
				"aesenc %xmm0, %xmm2\n\t"
				"aesenc %xmm0, %xmm3\n\t"
				"aesenc %xmm0, %xmm4\n\t"
				
				"aesenc %xmm0, %xmm1\n\t"
				"aesenc %xmm0, %xmm2\n\t"
				"aesenc %xmm0, %xmm3\n\t"
				"aesenc %xmm0, %xmm4\n\t"
				
				"aesenc %xmm0, %xmm1\n\t"
				"aesenc %xmm0, %xmm2\n\t"
				"aesenc %xmm0, %xmm3\n\t"
				"aesenc %xmm0, %xmm4\n\t"
				
				"aesenc %xmm0, %xmm1\n\t"
				"aesenc %xmm0, %xmm2\n\t"
				"aesenc %xmm0, %xmm3\n\t"
				"aesenc %xmm0, %xmm4\n\t"
				);	
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
		cerr << "loop: " << data[round][i]/inner_loop_iter << endl;
	#endif
		}		
		cerr << "Exp (reader 1) # " << round << endl;
    }
    cerr << "after loop" << endl;
    strcat(file,"_A_");
    cerr << file << endl;    
    for(int round = 0; round < ROUND_NUM; round ++) {
		strcpy(tempFile, file);
		file[strlen(tempFile)-2] = 'A' + round;
		strcat(tempFile, argv[1]);
		cerr << tempFile << endl;
		FILE *f_dump = fopen(tempFile, "w");
		for(int i = 2*SKIP; i < (numberOfBitsSent-IGNORE)*2; i+=2){ //Dump the data		
			fprintf(f_dump, "%lld %lld\n", data[round][i+1], data[round][i]); // time    sample		
		}
		fclose(f_dump);
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
		fprintf(fp_reader, "%lld %d\n", data[i+1], data[i]);		
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


    fclose(fp);
    fclose(fp2);
    fclose(fp3);
    */
    //free(data);
    free(memory_buffer);
    printf("Reader done \n");

    return 0;
}




