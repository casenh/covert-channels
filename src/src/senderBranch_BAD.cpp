//new
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <vector>

#include <string.h>
#include "../../papi-5.3.0/src/papi.h"


using namespace std;    

extern void RandomNumberGenerator(vector<int> *randomNumber, int length);
    
int* randomNumbers;
/* This function sends a 1 over the branch covert channel. It does not use a loop, as this increases the branch predictor */
void send0(void){

    double randNum;
    long long int startTime;
    srand(time(NULL));
   
	randNum = rand()/(double) RAND_MAX; 
	if(randNum > .5)
	    randomNumbers[0] = 32;
	else
	    randomNumbers[1] = 33;
	
	randNum = rand()/(double) RAND_MAX;
	if(randNum > .5)
	    randomNumbers[2] = 322;
	else
	    randomNumbers[3] = 321;
	
	randNum = rand()/(double) RAND_MAX;
	if(randNum > .5)
	    randomNumbers[4] = 324;
	else
	    randomNumbers[5] = 325;
	
	randNum = rand()/(double) RAND_MAX;
	if(randNum > .5)
	    randomNumbers[6] = 326;
	else
	    randomNumbers[7] = 327;
	
	randNum = rand()/(double) RAND_MAX;
	if(randNum > .5)
	    randomNumbers[8] = 328;
	else
	    randomNumbers[9] = 329;
	
	randNum = rand()/(double) RAND_MAX;
	if(randNum > .5)
	    randomNumbers[10] = 320;
	else
	    randomNumbers[20] = 312;
	
	randNum = rand()/(double) RAND_MAX;
	if(randNum > .5)
	    randomNumbers[30] = 312;
	else
	    randomNumbers[40] = 132;
	
	randNum = rand()/(double) RAND_MAX;
	if(randNum > .5)
	    randomNumbers[50] = 232;
	else
	    randomNumbers[60] = 432;
	
	randNum = rand()/(double) RAND_MAX;
	if(randNum > .5)
	    randomNumbers[70] = 532;
	else
	    randomNumbers[80] = 632;

	/*randNum = rand()/(double) RAND_MAX; 
	if(randNum > .5)
	    randNum += 1;
	else
	    randNum -= 52;
	
	randNum = rand()/(double) RAND_MAX;
	if(randNum > .5)
	    randNum += 1;
	else
	    randNum -= 52;
	
	randNum = rand()/(double) RAND_MAX;
	if(randNum > .5)
	    randNum += 1;
	else
	    randNum -= 52;
	
	randNum = rand()/(double) RAND_MAX;
	if(randNum > .5)
	    randNum += 1;
	else
	    randNum -= 52;
	
	randNum = rand()/(double) RAND_MAX;
	if(randNum > .5)
	    randNum += 1;
	else
	    randNum -= 52;
	
	randNum = rand()/(double) RAND_MAX;
	if(randNum > .5)
	    randNum += 1;
	else
	    randNum -= 52;
	
	randNum = rand()/(double) RAND_MAX;
	if(randNum > .5)
	    randNum += 1;
	else
	    randNum -= 52;
	
	randNum = rand()/(double) RAND_MAX;
	if(randNum > .5)
	    randNum += 1;
	else
	    randNum -= 52;
	
	randNum = rand()/(double) RAND_MAX;
	if(randNum > .5)
	    randNum += 1;
	else
	    randNum -= 52;*/
	
	//fflush(NULL);
}


/* This function sends a 0 over the L2 cache covert channel by idly waiting */
void send1(int sendLength){
    long long int time;
    
    time = PAPI_get_real_nsec();
    while((PAPI_get_real_nsec() - time) < sendLength) {
	//for(int n = 0; n < 2000; n++) { }
    } 
//     usleep(1);
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


/* This function returns the current time returned by PAPI_get_real_nsec in	*
 * a truncated version								*/
long long int getTime(void){
  
 long long int time = PAPI_get_real_nsec();
 time = time & 0x00000000FFFFFFFF; //The max time that is returning is 9.999999999 seconds
 return time;
 
}



/* This program attempts to communicate a 1 or 0 via covert channels.		*/
/* When writing a 1 the program creates a large array and then 		*/
/* iterates over the array in order to replace data that's in the L2 cache.	*/
/* When writing a 0 the program simples idles, using a little resources as	*/
/* possible.									*/
int main(int argc, char* argv[]){

    long long int time = 0, time2 = 0, startTime; //Used for initial time
    FILE *fp, *senderFiles;
    char file[35]; //This will be the name of the file that we want to save the data to
    char** iterate; //The array that we will be iterating over
    int n = 0, loop = 0, i = 0, testedValue = 0, timesRepeated = 0; //Used for iteration
    vector<int> randomNumber;
    
    /* Create a data file to save information later used for deciphering the message */
    createDataFile(file);
    senderFiles = fopen("../data/senderFiles", "a");
    fprintf(senderFiles, "%s\n", file);
    fp = fopen(file, "a");
    if(fp == NULL){ printf("Error creating the sender data file!\n"); }    
    
	randomNumbers = (int*) malloc(sizeof(int) * 100);    

/* This is the main loop where the data is sent the number to be tested and the times to repeat */
/* it are generated in an array to be sent							  */
    int period = atoi(argv[1]);
    int sendLength = atoi(argv[2]);
    RandomNumberGenerator(&randomNumber, 10000); 	//create the random sequence to send
    cout << randomNumber.size() << endl;
    string signal;
    cin >> signal;
    while(getTime() > 20000) {  }
    time = PAPI_get_real_nsec();		//Record the time we start sending the data (for deciphering later)
    startTime = getTime();
	int temp = 0;
    for(n=0; n < randomNumber.size(); n++){
	
	while(getTime() < startTime + (n * period)) { }
	if(randomNumber[n] == 0){
	    long long int beginTime = PAPI_get_real_nsec();
	    while((PAPI_get_real_nsec() - beginTime) < sendLength)
	    {
			send0();
	 		send0();
	 		send0();
	 		send0();
		}
// 	    cout << (PAPI_get_real_nsec() - startTime) << endl;
	}
	else{
	    send1(sendLength);
	}

    /* THIS IS FOR SENDING A BASELINE OF 10101010... */
 	/*if(n % 2 == 0)
 	{
 	    while((PAPI_get_real_nsec() - startTime) < sendLength)
 	    {
 		send1();
 	    }
 	}
 	else{
 	    send0(sendLength);
 	}*/
// 	fprintf(fp, "%lld   500\n", getTime());
    }
    time2 = PAPI_get_real_nsec();		//Record the ending time (for deciphering later)
    
    
    /* Record the important data like start and end time so we can use it later */
//     fprintf(fp, "%lld\n", time); //Save the time when sending started
//     fprintf(fp, "%lld\n", time2); //Save the time when sending ended
//     fprintf(fp, "%d\n", testedValue); //Saved the value we were testing
//     fprintf(fp, "%d\n", timesRepeated); //Save the amount of times repeated we were testing
    for(n=0; n < randomNumber.size(); n++){
	fprintf(fp, "%d", randomNumber[n]);
    }
    //fprintf(fp, "%d", randomNumber[0]);
    
//     printf("Average Period for Sender: %lld\n", (PAPI_get_real_nsec() - time)/1000);

    
    
}
