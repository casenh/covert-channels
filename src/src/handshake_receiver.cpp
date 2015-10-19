#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/mman.h>
#include <algorithm>
#include <vector>
#include "../../papi-5.3.0/src/papi.h"

#define NUM_INDICES 256 //Number of cache lines to hit

using namespace std;

/* Some external functions and values */
void getCompInfo(int);
extern long long int getTime();
extern int NUM_OF_SETS;
extern int WAYS_ASSOCIATIVE;
extern int BLOCK_LENGTH;

/* These are the values used for the handshake */
long long int PRIMELENGTH = 3000;
long long int SENDLENGTH = 10000;
long long int PERIOD = 20000;

char* data; //The array to iterate over
char* Addresses[NUM_INDICES]; //The addresses that map to the same cache set
char* Addresses2[NUM_INDICES];

void iterateOver() {
	int temp = 0;

	for(int i = 0; i < NUM_INDICES; i++) {
		temp += *(Addresses[i]);
		temp += *(Addresses2[i]);
	}
}


/**
 * This function takes the first address in the allocated array (data), and
 * computes addresses that will map to the same L3 Cache Set
 */
void makeMatchingSets() {
	
	/* Add the offsets to the starting location to hit the same set */
	char* startAddress = &data[0];
	Addresses[0] = startAddress;
	Addresses2[0] = startAddress + 64;
	for(int i = 1; i < NUM_INDICES; i++) {
		Addresses[i] = startAddress + (NUM_OF_SETS * BLOCK_LENGTH * i * 2);
		Addresses2[i] = Addresses[i] + 64;
	}
	
	/* Now randomize the access pattern to mess up pre-fetching*/
	srand(time(NULL));
	for(int i = 0; i < NUM_INDICES; i++) {
		
			int temp = rand() % (NUM_INDICES - 1);
			char* temp2 = Addresses[temp];
			Addresses[temp] = Addresses[i];
			Addresses[i] = temp2;
	}
}


/** 
 * This funnctions works with the sender in finding a common set to communicate across. The sender
 * attempts to hit every possible set in the L3 cachem, and the receiver reports which attempt worked
 * the best
 */
int handshakeReceiver(char** iterate) {
	
	/* First allocate the 8 mB */
	data = (char*) malloc(1024 * 1024 * NUM_INDICES);
		
	/* Put the memory into the page table */
	long long int temp = 5;
	for(int i = 0; i < 1024 * 1024 * NUM_INDICES; i++) {
		data[i] = 7;
	}
	if(mlockall(MCL_CURRENT | MCL_FUTURE) < 0) {
		FILE* ERROR = fopen("errorR.txt", "a");
		fclose(ERROR);
	}
	
	/* Get matching sets */
	makeMatchingSets();
	iterateOver();
	long int startAddress = (long int) &data[0];
	cout << startAddress << endl;
	cout.flush();
	return true;
}
