//new
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <vector>

#include <string.h>
#include "../../papi-5.3.0/src/papi.h"


using namespace std;    
int NUM_OF_SETS = 0;
int WAYS_ASSOCIATIVE = 0;
int BLOCK_LENGTH = 0;

extern void handle_error(int);
extern void RandomNumberGenerator(vector<int> *randomNumber, int length);

    
/* This function sends a 1 over the L2 cache covert channel */
int send1(int randomNumberHolder[], int sendLength){
    long long int PAPItime = 0;
    
    PAPItime = PAPI_get_real_nsec();
    while((PAPI_get_real_nsec() - PAPItime) < sendLength)
    {
		for(int i = 0; i < 1000; i++) 
			{
				randomNumberHolder[i] = 1;
				randomNumberHolder[i] = 2;
				randomNumberHolder[i] = 3;
				randomNumberHolder[i] = 4;
				randomNumberHolder[i] = 5;
				randomNumberHolder[i] = 6;
			}
		}
    return 0;  
    
}


/* This function sends a 0 over the L2 cache covert channel by idly waiting */
void send0(int sendLength){
    
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
    
    
    if(fp == NULL){ printf("Error creating the sender data file!\n"); }    
    
    /* Initialize the PAPI library and create the events that we wish to monitor */
//     int retval = PAPI_library_init(PAPI_VER_CURRENT);
//     if( retval != PAPI_VER_CURRENT) {
// 	fprintf(stderr,"PAPI library init error!\n");
// 	exit(1);
//     }
//     if( PAPI_create_eventset(&eventSet) != PAPI_OK){ handle_error(1);}   
//     if(PAPI_add_event(eventSet, PAPI_LD_INS) != PAPI_OK){ handle_error(1); }
//     if( PAPI_start(eventSet) != PAPI_OK){ handle_error(1); }
    
    
/* This is the main loop where the data is sent the number to be tested and the times to repeat */
/* it are generated in an array to be sent							  */
//     testedValue = atoi(argv[1]);
//     timesRepeated = atoi(argv[2]);
//     cout << "testedValue: " << testedValue << "	timesRepeated: " << timesRepeated << endl;
  

    int sum = 0;
    int period = tuning[0];
    int sendLength = tuning[1];
    RandomNumberGenerator(&randomNumber, 10000); 	//create the random sequence to send
    cout << (randomNumber.size()) << endl;
    string signal;
    cin >> signal;
    while(getTime() > 20000) {  }
    PAPItime = PAPI_get_real_nsec();		//Record the time we start sending the data (for deciphering later)
    startTime = getTime();
    for(n=0; n < randomNumber.size() + 1; n++){
	while(getTime() < startTime + (n * period)) { }
	//~ if(randomNumber[n] == 1){
//~ // 	    PAPI_reset(eventSet);
//~ // 	    long long int startTime = PAPI_get_real_nsec();
//~ // 	    if( PAPI_reset(eventSet) != PAPI_OK) { handle_error(1); }
	    //~ send1(randomNumberHolder, sendLength);
//~ // 	    if( PAPI_read(eventSet, counters) != PAPI_OK){ handle_error(1);}
//~ // 	    fprintf(fp, "Stores: %lld\n", PAPI_get_real_nsec() - startTime);
	//~ }
	//~ else{
//~ // 	    PAPI_reset(eventSet);
//~ // 	    long long int Starttime2 = PAPI_get_real_nsec();
	    //~ send0(sendLength);
//~ // 	    fprintf(fp, "Starttime2 = %lld\n", PAPI_get_real_nsec() - Starttime2);
	//~ }
 	if(n % 2 == 0)
 	{
 	    send1(randomNumberHolder, sendLength);
 	}
 	else{
 	    send0(sendLength);
 	}
    }
    time2 = PAPI_get_real_nsec();		//Record the ending time (for deciphering later)
    
    
    /* Record the important data like start and end time so we can use it later */
     fprintf(fp, "%lld\n", PAPItime); //Save the time when sending started
     fprintf(fp, "%lld\n", time2); //Save the time when sending ended
    for(n=0; n < randomNumber.size(); n++){
	fprintf(fp, "%d", randomNumber[n]);
    }
    
    cout << sum << endl;
    return 0;
}
