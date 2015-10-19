#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <vector>

#include <string.h>
#include <emmintrin.h>
#include <assert.h>
#include "../../papi-5.3.0/src/papi.h"

#define NUM_OF_SETS 4096 //The number of sets in the cache (8-way set associative with 64 byte block length) 
#define BLOCK_LENGTH 64
#define CACHE_LINE 64
#define MEM_BUFFER_SIZE 128*1024*1024
#define NUM_BITS 30000

using namespace std;   
#define SKIP 0 
long long int IGNORE =-1;
//#define CHECK_TIME 1

extern void handle_error(int);
int *memory_buffer;
long long int int_out;
 int inner_loop_iter;
 
/* This function sends a 1 over the L2 cache covert channel */
inline int send1(int sendLength){
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
			//10-11 ns per block (16 inst)
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
		   	
		}
		//~ long end = PAPI_get_real_nsec();
		//~ printf("Time elapsed: %ld\n", (end-start)/10000);
		//~ exit(0);
#ifdef CHECK_TIME
		long long papi_end_time = PAPI_get_real_nsec();
		cerr << "Time per instruction (sender), ns " << (papi_end_time - papi_start_time) << endl ; ///inner_loop_iter << endl;
		//cout << "Time elapsed " << (papi_end_time - papi_start_time)/1000 << endl;
		sum += (papi_end_time - papi_start_time)/inner_loop_iter;
		count ++;
		//exit(0);
#endif				
		stop_flag = !((PAPI_get_real_nsec() - PAPItime) < sendLength);
    }
    return 0;  
}


/* This function sends a 0 over the L2 cache covert channel by idly waiting */
inline void send0(int sendLength){
    
	long long int time = PAPI_get_real_nsec();
	while((PAPI_get_real_nsec() - time) < sendLength) { }
}


/* This function creates the appropriate timestamp for the data file that is being saved */
void createDataFile(char *name, char *suffix){

    char *word;
    time_t timestamp;
    int n = 0, m = 0; //Used for looping

    timestamp = time(NULL);
    word = ctime(&timestamp);
    strcpy(name, "../data/Sender."); //Here we are picking out the parts of the timestamp that we would like in the file name
    strncat(name, &word[4], 3);
    if(word[8] == ' ')
	word[8] = '0';
    strncat(name, &word[8], 2);
    strcat(name, ".");
    strncat(name, &word[11], 2);
    strcat(name, ".");
    strncat(name, &word[14], 2);
    strcat(name, ".");
    strncat(name, &word[17], 2);
    strcat(name, "_");
    strcat(name, suffix);

//     printf("Data being saved to %s \n", name);

}


/* This function creates a random sequence of 1000 bits to be transmitted */
/* This function creates a random sequence of 1000 bits to be transmitted */
void RandomNumberGenerator(vector<int> *randomNumber, int length){
    
    int randNum;
    int i = 0, n = 0, j = 0, otherValue;
    int bit;
     
    srand(time(NULL));
    for(i = 0; i < length; i ++){ //First pad the number with the value we are not trying to test
		bit = rand()%2;
		randomNumber->push_back(bit);
    /*
		randNum = rand()/ (RAND_MAX/8);
		n = randNum +1;
		randomNumber->insert(randomNumber->end(), n, 0);
		i += n;
		
		randNum = rand()/ (RAND_MAX/8);
		n = randNum +1;
		randomNumber->insert(randomNumber->end(), n, 1);
		i += n;			
		* */
	}
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
int main(int argc, char* argv[]){
    IGNORE = atoll(argv[4]);
    long long int PAPItime = 0, time2 = 0, startTime, counters[2]; //Used for initial time
    int randomNumberHolder[5000];
    int eventSet = PAPI_NULL;
    FILE *fp, *senderFiles, *fp_sender;
    char file[256]; //This will be the name of the file that we want to save the data to
    char** iterate; //The array that we will be iterating over
    int n = 0, loop = 0, i = 0, testedValue = 0, timesRepeated = 0; //Used for iteration
    vector<int> randomNumber;
    //memory_buffer = (int *) malloc(MEM_BUFFER_SIZE*sizeof(int));
    //assert(memory_buffer && "memory_buffer is null");
    
	
    
    /*
    __asm__ volatile ( 
			".align 4 \n\t"
			".val: \n\t"
			".long 0x34897346 \n\t"
			".long 0xa6b82047 \n\t"
			".long 0xc9d8f3e7 \n\t"
			".long 0x84198327 \n\t"			
			"func: \n\t"
			"movdqa .LC0($rip), %xmm0 \n\t"								 							
		   	: 
		   	: )
		   	: );
    */

	fp_sender = fopen("../data/sender.txt", "w");
    //send0(randomNumberHolder, 10);
    /* Create a data file to save information later used for deciphering the message */
    createDataFile(file, argv[1]);
    senderFiles = fopen("../data/senderFiles", "a");
    fprintf(senderFiles, "%s\n", file);
    fp = fopen(file, "a");
    if(fp == NULL){ printf("Error creating the sender data file!\n"); }  
    //~ unsigned long ik = (((unsigned long)array) | 0x7F) - 3;  
    //~ array_ptr = (int *) ik;
    inner_loop_iter = atoi(argv[3]);
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
  

    int sum = 0;
    long long int period = atoll(argv[1]);
    long long int sendLength = atoll(argv[2]);
#ifdef CHECK_TIME
	randomNumber.insert(randomNumber.begin(), 20000, 0);
#else
    RandomNumberGenerator(&randomNumber, IGNORE+NUM_BITS); 	//create the random sequence to send
#endif
    cout << randomNumber.size() << endl;
    string signal;
    cin >> signal;
    while(getTime() > 20000) {  }
    PAPItime = PAPI_get_real_nsec();		//Record the time we start sending the data (for deciphering later)
    startTime = PAPI_get_real_nsec();
 
 #define ROUND_NUM 1    
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
				send0(sendLength);				
		// 	    fprintf(fp, "Starttime2 = %lld\n", PAPI_get_real_nsec() - Starttime2);
			}
		}
	}
    time2 = PAPI_get_real_nsec();		//Record the ending time (for deciphering later)
    
    
    /* Record the important data like start and end time so we can use it later */
//     fprintf(fp, "%lld\n", PAPItime); //Save the time when sending started
//     fprintf(fp, "%lld\n", time2); //Save the time when sending ended
    for(n=SKIP + IGNORE; n < randomNumber.size(); n++){
		fprintf(fp, "%d", randomNumber[n]);
		fprintf(fp_sender, "%d\n", randomNumber[n]);	
    }    
    //cout << sum << endl;
    //free(memory_buffer);
    fclose(fp_sender);
}
