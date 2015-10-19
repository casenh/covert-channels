//new
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <chrono>

#include <string.h>
#include "../../papi-5.3.0/src/papi.h"


using namespace std;    
#define RISCV

extern void RandomNumberGenerator(vector<int> *randomNumber, int length);

/* This function sends a 0 over the L2 cache covert channel by idly waiting */
void send1(int sendLength){
    long long int time;
    
#ifdef PERF_CTRS
    time = PAPI_get_real_nsec();
    while((PAPI_get_real_nsec() - time) < sendLength) { } 
#else
   	//time = std::chrono::high_resolution_clock::now();
   	//while((std::chrono::high_resolution_clock::now() - time) < sendLength) { }
#endif
	
}


/* This function creates the appropriate timestamp for the data file that is being saved */
void createDataFile(char *name){

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

//     printf("Data being saved to %s \n", name);

}

long long getCycles()
{
   long long cycles = 1337;
   #ifdef RISCV
   __asm__ __volatile__( "rdcycle %0" : "=r" (cycles) );
   #endif
   return cycles;
}                                                                                                                                        


/* This function returns the current time returned by PAPI_get_real_nsec in	*
 * a truncated version								*/
long long int getTime(void){
	
	long long int time;
#ifdef PERF_CTRS
	time = PAPI_get_real_nsec();
#else
//	time = std::chrono::high_resolution_clock::now();
   time = getCycles();
#endif
	time = time & 0x00000000FFFFFFFF; 

	return time;
 
}



/* This program attempts to communicate a 1 or 0 via covert channels.		*/
/* When writing a 1 the program creates a large array and then 		*/
/* iterates over the array in order to replace data that's in the L2 cache.	*/
/* When writing a 0 the program simples idles, using a little resources as	*/
/* possible.									*/
void send0(int);
int main(int argc, char* argv[]){

    long long int startTime; //Used for initial time
    FILE *fp, *senderFiles;
    char file[35]; //This will be the name of the file that we want to save the data to
    char** iterate; //The array that we will be iterating over
    int n = 0, loop = 0, i = 0, testedValue = 0, timesRepeated = 0; //Used for iteration
    vector<int> randomNumber;
    
    /* Create a data file to save information later used for deciphering the message */
    createDataFile(file);
    senderFiles = fopen("../data/senderFiles", "a");
    //~ fprintf(senderFiles, "%s\n", file);
    string senderFilename(file);
    //~ fp = fopen(file, "a");
    //~ if(fp == NULL){ printf("Error creating the sender data file!\n"); }    
    
    
/* This is the main loop where the data is sent the number to be tested and the times to repeat */
/* it are generated in an array to be sent							  */
    int period = atoi(argv[1]);
    int numBranches = atoi(argv[2]);
    char buf[50];
    sprintf(buf, "%d", period);
    string tmp(buf);
	senderFilename.append("_");
	senderFilename.append(tmp);
	fp = fopen(&senderFilename[0], "a");
	fprintf(senderFiles, "%s\n", &senderFilename[0]);

	srand(time(NULL));
	RandomNumberGenerator(&randomNumber, 10000); 	//create the random sequence to send
	cout << randomNumber.size() << endl;
	string signal;
	cin >> signal;
	while(getTime() > 20000) {  }
	startTime = getTime();
	for(n=0; n < randomNumber.size(); n++){

		while(getTime() < startTime + (n * period)) { }
		if(randomNumber[n] == 0){
			long long int tempTime = getTime();
			send0(numBranches);
		}
		else{
			send1(period/2);
		}

		/* THIS IS FOR SENDING A BASELINE OF 10101010... */
		/*if(n % 2 == 0) {
			while((PAPI_get_real_nsec() - startTime) < sendLength) {
				send1();
			}
		}
		else{
			send0(sendLength);
		}*/
	}
    
    /* Save the sent bits to a file */
    for(n=0; n < randomNumber.size(); n++){
	fprintf(fp, "%d", randomNumber[n]);
    }    
    
}



void send0(int numBranches){

    double randNum;
    long long int startTime;
	
	randNum  = rand(); 
	if(randNum > (RAND_MAX/2))
	    randNum += 1;
	else
	    randNum -= 52;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 2;
	else
	    randNum -= 53;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 3;
	else
	    randNum -= 54;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 4;
	else
	    randNum -= 55;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 5;
	else
	    randNum -= 56;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 6;
	else
	    randNum -= 57;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 7;
	else
	    randNum -= 58;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 8;
	else
	    randNum -= 59;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 9;
	else
	    randNum -= 60;
	    
	randNum  = rand(); 
	if(randNum > (RAND_MAX/2))
	    randNum += 10;
	else
	    randNum -= 521;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 12;
	else
	    randNum -= 531;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 13;
	else
	    randNum -= 541;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 14;
	else
	    randNum -= 551;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 15;
	else
	    randNum -= 561;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 16;
	else
	    randNum -= 571;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 17;
	else
	    randNum -= 581;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 18;
	else
	    randNum -= 591;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 19;
	else
	    randNum -= 601;
	    
	randNum  = rand(); 
	if(randNum > (RAND_MAX/2))
	    randNum += 1;
	else
	    randNum -= 52;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 2;
	else
	    randNum -= 53;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 3;
	else
	    randNum -= 54;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 4;
	else
	    randNum -= 55;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 5;
	else
	    randNum -= 56;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 6;
	else
	    randNum -= 57;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 7;
	else
	    randNum -= 58;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 8;
	else
	    randNum -= 59;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 9;
	else
	    randNum -= 60;
	    
	randNum  = rand(); 
	if(randNum > (RAND_MAX/2))
	    randNum += 10;
	else
	    randNum -= 521;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 12;
	else
	    randNum -= 531;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 13;
	else
	    randNum -= 541;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 14;
	else
	    randNum -= 551;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 15;
	else
	    randNum -= 561;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 16;
	else
	    randNum -= 571;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 17;
	else
	    randNum -= 581;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 18;
	else
	    randNum -= 591;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 19;
	else
	    randNum -= 601;

	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 6;
	else
	    randNum -= 571;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 73;
	else
	    randNum -= 582;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 83;
	else
	    randNum -= 593;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 9;
	else
	    randNum -= 603;
	    
	randNum  = rand(); 
	if(randNum > (RAND_MAX/2))
	    randNum += 103;
	else
	    randNum -= 523;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 123;
	else
	    randNum -= 5313;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 133;
	else
	    randNum -= 5413;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 143;
	else
	    randNum -= 5513;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 153;
	else
	    randNum -= 5631;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 136;
	else
	    randNum -= 5731;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 137;
	else
	    randNum -= 5831;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 138;
	else
	    randNum -= 5931;
	
	randNum  = rand();
	if(randNum > (RAND_MAX/2))
	    randNum += 139;
	else
	    randNum -= 6031;
	    
	    
	    
	    
	    
	    
	    
	    
	if(numBranches >= 0) {    
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum  = rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum  = rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum  = rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum  = rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
			
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum  = rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 66;
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 3;
		else
			randNum -= 55;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 44;
		else
			randNum -= 582;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 65803;
			
		randNum  = rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 68;
		else
			randNum -= 87;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 7;
		else
			randNum -= 3456;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 436;
		else
			randNum -= 234;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 2;
		else
			randNum -= 1;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 12153;
		else
			randNum -= 56231;
		}
	    
	    
/****************************************************/	    
	if(numBranches >= 1) {    
	
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 11;
		else
			randNum -= 34;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum  = rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum  = rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
			
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum  = rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 345;
		else
			randNum -= 345;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 3345;
		else
			randNum -= 593;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum  = rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 32;
		else
			randNum -= 5631;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 11;
		else
			randNum -= 5731;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 111;
		else
			randNum -= 5831;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 222;
		else
			randNum -= 444;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 333;
		else
			randNum -= 555;
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 77;
		else
			randNum -= 666;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 54;
		else
			randNum -= 888;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum  = rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
		}
	    
	    
/**********************************************************/
	    
	if(numBranches >= 2) {	    
		randNum  = rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
			
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum  = rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum  = rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum  = rand();
	}
	
	
	/*************************/
	
	if(numBranches >= 3) {
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum  = rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum  = rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
			
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum  = rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 6;
		else
			randNum -= 571;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 73;
		else
			randNum -= 582;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 83;
		else
			randNum -= 593;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 9;
		else
			randNum -= 603;
			
		randNum  = rand(); 
		if(randNum > (RAND_MAX/2))
			randNum += 103;
		else
			randNum -= 523;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 123;
		else
			randNum -= 5313;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 133;
		else
			randNum -= 5413;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 143;
		else
			randNum -= 5513;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 153;
		else
			randNum -= 5631;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 136;
		else
			randNum -= 5731;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 137;
		else
			randNum -= 5831;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 138;
		else
			randNum -= 5931;
		
		randNum  = rand();
		if(randNum > (RAND_MAX/2))
			randNum += 139;
		else
			randNum -= 6031;
	    
	}
}


