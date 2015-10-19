#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <vector>

#include <string.h>
#include <emmintrin.h>
#include <assert.h>
#include "../../papi-5.3.0/src/papi.h"
#include "randomNumberGenerator.hpp"

#define CACHE_LINE 64
#define MEM_BUFFER_SIZE 100*1024*1024
int NUM_OF_SETS = 4096;
int WAYS_ASSOCIATIVE = 0;
int BLOCK_LENGTH = 64;

using namespace std;   
#define SKIP 0 
long long int IGNORE =-1;
//#define CHECK_TIME 1

extern void handle_error(int);
int *memory_buffer;
long long int int_out;
 int inner_loop_iter;
 
/* This function sends a 1 over the L2 cache covert channel */
int send0(int randomNumberHolder[], int sendLength){
    long long int PAPItime = 0;
    bool stop_flag = false;

    PAPItime = PAPI_get_real_nsec();
    unsigned int seed = rand();
    unsigned int ind;
#ifdef CHECK_TIME
	long long sum= 0, count = 0;
#endif    

    for(int i = 0; !stop_flag; i++)    
    {
		// lock cmpxchng takes 35 ns
		//long start = PAPI_get_real_nsec();
#ifdef CHECK_TIME
		long long papi_start_time = PAPI_get_real_nsec();
#endif		
		for(int n=0; n < inner_loop_iter; n++) {
			ind = seed + n;
			ind *= ind;
			//"mfence\n\t"	
			__asm__ volatile ( "lock xchg %%rax, (%0) \n\t"								 							
		   	: 
		   	: "r"(memory_buffer+ind % (MEM_BUFFER_SIZE-4))
		   	: "%rax");		   
		   	
		}
		//~ long end = PAPI_get_real_nsec();
		//~ printf("Time elapsed: %ld\n", (end-start)/10000);
		//~ exit(0);
#ifdef CHECK_TIME
		long long papi_end_time = PAPI_get_real_nsec();
		//cerr << "Time per instruction (sender), ns " << (papi_end_time - papi_start_time)/inner_loop_iter << endl;
		cout << "Time elapsed " << (papi_end_time - papi_start_time)/1000 << endl;
		sum += (papi_end_time - papi_start_time)/inner_loop_iter;
		count ++;
		//exit(0);
#endif				
		stop_flag = !((PAPI_get_real_nsec() - PAPItime) < sendLength);
    }
    return 0;  
}


/* This function sends a 0 over the L2 cache covert channel by idly waiting */
void send1(int sendLength){
    
	long long int time = PAPI_get_real_nsec();
	while((PAPI_get_real_nsec() - time) < sendLength) { }
}

/* This function returns the current time returned by PAPI_get_real_nsec in	*
 * seconds									*/
long long int getTime(void){
  
 long long int time = PAPI_get_real_nsec();
 time = time & 0x00000000FFFFFFFF; //The max time that is returning is 9.999999999 seconds
 return time;
 
}

/* This program attempts to communicate a 1 or 0 via covert channels.		*/
/* When writing a 1 [slowupCPU()] the program creates a large array and then 	*/
/* iterates over the array in order to replace data that's in the L2 cache.	*/
/* When writing a 0 the program simples idles, using a little resources as	*/
/* possible.									*/
int doChannel(FILE* ignored, FILE* fp, std::vector<long long> tuning) {
        long long int PAPItime = 0, time2 = 0, startTime;
    int randomNumberHolder[5000];
    unsigned int n = 0;
    vector<int> randomNumber;
    memory_buffer = (int *) malloc(MEM_BUFFER_SIZE*sizeof(int));
    assert(memory_buffer && "memory_buffer is null");

    if(fp == NULL){ printf("Error creating the sender data file!\n"); }  
    //~ unsigned long ik = (((unsigned long)array) | 0x7F) - 3;  
    //~ array_ptr = (int *) ik;
    /* Initialize the PAPI library and create the events that we wish to monitor */
	int retval = PAPI_library_init(PAPI_VER_CURRENT);
		if( retval != PAPI_VER_CURRENT) {
		fprintf(stderr,"PAPI library init error!\n");
		exit(1);
    }
//     if( PAPI_create_eventset(&eventSet) != PAPI_OK){ handle_error(1);}   
//     if(PAPI_add_event(eventSet, PAPI_LD_INS) != PAPI_OK){ handle_error(1); }
//     if( PAPI_start(eventSet) != PAPI_OK){ handle_error(1); }
    
    
    
/* This is the main loop where the data is sent the number to be tested and the times to repeat */
/* it are generated in an array to be sent							  */
//     testedValue = atoi(argv[1]);
//     timesRepeated = atoi(argv[2]);
//     cout << "testedValue: " << testedValue << "	timesRepeated: " << timesRepeated << endl;
  

    long long int period = tuning[0];
    long long int sendLength = tuning[1];
    inner_loop_iter = tuning[2];
    IGNORE = tuning[3];
#ifdef CHECK_TIME
	randomNumber.insert(randomNumber.begin(), 20000, 0);
#else
    RandomNumberGenerator(&randomNumber, IGNORE+150*1000); 	//create the random sequence to send
#endif
    cout << randomNumber.size() << endl;
    string signal;
    cin >> signal;
    while(getTime() > 20000) {  }
    PAPItime = PAPI_get_real_nsec();		//Record the time we start sending the data (for deciphering later)
    startTime = PAPI_get_real_nsec();
 
 #define ROUND_NUM 1    
    for(int round = 0; round < ROUND_NUM; round ++) {
		//cerr << "Exp #" << round << endl;
		while(PAPI_get_real_nsec() < startTime + round*2*1000000000L);
		for(n=0; n < randomNumber.size() + 1; n++){
			while(PAPI_get_real_nsec() < startTime + (n * period)) { }
			if(randomNumber[n] == 1){
		// 	    PAPI_reset(eventSet);
		// 	    long long int startTime = PAPI_get_real_nsec();
		// 	    if( PAPI_reset(eventSet) != PAPI_OK) { handle_error(1); }
				send1(sendLength);
		// 	    if( PAPI_read(eventSet, counters) != PAPI_OK){ handle_error(1);}
		// 	    fprintf(fp, "Stores: %lld\n", PAPI_get_real_nsec() - startTime);
			}
			else{
		// 	    PAPI_reset(eventSet);
		// 	    long long int Starttime2 = PAPI_get_real_nsec();
				send0(randomNumberHolder, sendLength);
		// 	    fprintf(fp, "Starttime2 = %lld\n", PAPI_get_real_nsec() - Starttime2);
			}
		}
	}
    time2 = PAPI_get_real_nsec();		//Record the ending time (for deciphering later)
    
    
    /* Record the important data like start and end time so we can use it later */
     fprintf(fp, "%lld\n", PAPItime); //Save the time when sending started
     fprintf(fp, "%lld\n", time2); //Save the time when sending ended
    for(n=SKIP + IGNORE; n < randomNumber.size(); n++){
		fprintf(fp, "%d", randomNumber[n]);
    }    
    //cout << sum << endl;
    free(memory_buffer);
    return 0;
}
