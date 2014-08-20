#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <vector>

#include <string.h>
#include "../../papi-5.3.0/src/papi.h"

using namespace std; 

extern void getCompInfo(int);
extern void RandomNumberGenerator(vector<int> *randomNumber, int length);
   
int NUM_OF_SETS = 0;
int BLOCK_LENGTH = 0;
int WAYS_ASSOCIATIVE = 0;

/* This function creates the appropriate timestamp for the data file that is being saved */
void createDataFile(char *name){

    char *word; //The timestamp that we will create the filename from
    time_t timestamp; //The beginning of the filename
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
}


/* This function returns the current time returned by PAPI_get_real_nsec in	*
 * seconds. Max it returns in about 16 seconds									*/
long long int getTime(void){
  
 long long int time = PAPI_get_real_nsec();
 time = time & 0x00000000FFFFFFFF; //Chop of the top of the bits so we only get seconds
 return time;
}


/* This function sends a 1 over the L2 cache covert channel */
int loc = 0;
void send1(char* iterate, int sendLength){
    long long int retval, time;
    
    time = PAPI_get_real_nsec();
    while((PAPI_get_real_nsec() - time) < sendLength)
    {
	for(int n = 0; n < NUM_OF_SETS; n++) //Hit every line in the L1 cache
	{
	    iterate[loc * BLOCK_LENGTH] = 1;
	    loc++;
	}
	if(loc > (NUM_OF_SETS * WAYS_ASSOCIATIVE))
	    loc = 0;
    }
}


/* This function sends a 0 over the L2 cache covert channel by idly waiting */
void send0(int length){
    long long int time;
    
    time = PAPI_get_real_nsec();
    while((PAPI_get_real_nsec() - time) < length) { } 
}



/* This program attempts to communicate a 1 or 0 via covert channels.		*/
/* When writing a 1 [slowupCPU()] the program creates a large array and then 	*/
/* iterates over the array in order to replace data that's in the L2 cache.	*/
/* When writing a 0 the program simples idles, using a little resources as	*/
/* possible.									*/
int main(int argc, char* argv[]){

    long long int time = 0, time2 = 0, startTime; //Used for initial time
    FILE *fp, *senderList;
    char file[35]; //This will be the name of the file that we want to save the data to
    char* iterate; //The array that we will be iterating over
    int n = 0, loop = 0, i = 0, testedValue = 0, timesRepeated = 0; //Used for iteration
    vector<int> randomNumber;
    
    /* Create a data file to save information later used for deciphering the message */
    createDataFile(file);
    senderList = fopen("../data/senderFiles", "a");
    fprintf(senderList, "%s\n", file);
    fp = fopen(file, "a");
    if(fp == NULL){ printf("Error creating the sender data file!\n"); }    

    /* Get the cache info */
    getCompInfo(1);
    
    /* Initialize the array to iterate over to cause cache misses */
    iterate = (char*) malloc(sizeof(char) * (NUM_OF_SETS * WAYS_ASSOCIATIVE * BLOCK_LENGTH));
    
    
/* This is the main loop where the data is sent the number to be tested and the times to repeat */
/* it are generated in an array to be sent							  */
    RandomNumberGenerator(&randomNumber, 10000); 	//create the random sequence to send
    int period = atoi(argv[1]);
    int wait = atoi(argv[2]);
    int sendLength = atoi(argv[3]);;
    cout << randomNumber.size() << endl;
    string signal;
    cin >> signal;
    while(getTime() > 20000) {  }
    time = PAPI_get_real_nsec();		//Record the time we start sending the data (for deciphering later)
    startTime = getTime();
    for(n=0; n < randomNumber.size(); n++){
	
	while(getTime() < startTime + (n * period)) { } //Synchronize the threads with the period
	
	while(getTime() < startTime + wait + (n * period)) { } //Wait for the reader to prime

	//~ if(randomNumber[n] == 1){
	    //~ send1(iterate, sendLength);
	//~ }
	//~ else{
	    //~ send0(sendLength);
	//~ }
 	if(n % 2 == 0)
 	{
 	    send1(iterate, sendLength);
 	}
 	else{
 	send0(sendLength);
 	}
    }
    time2 = PAPI_get_real_nsec();		//Record the ending time (for deciphering later)
    
    
    /* Record the important data like start and end time so we can use it later */
//     fprintf(fp, "%lld\n", time); //Save the time when sending started
//     fprintf(fp, "%lld\n", time2); //Save the time when sending ended
    for(n=6; n < randomNumber.size(); n++){
	fprintf(fp, "%d", randomNumber[n]);
    }
    
    free(iterate);
}
