#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include "../../papi-5.3.0/src/papi.h"

using namespace std;

void getCompInfo(int);

long long  PRIMELENGTH = 3000;
long long int SENDLENGTH = 10000;
long long int PERIOD = 20000;

extern int NUM_OF_SETS;
extern int BLOCK_LENGTH;
extern int WAYS_ASSOCIATIVE;
extern long long int getTime();

int INDEX;
#define numOfIndices 1024	
char* Addresses[1][numOfIndices];
char* Addresses2[1][numOfIndices];

int send1() {
	int temp = 0;
	
	for(int n = 0; n < numOfIndices; n++) {
		temp += *Addresses[0][n];
		temp += *Addresses2[0][n];
	}
				
	return temp;
}


//~ long long int numOfIndices = 256;
void makeMatchingSets(long int receiverAddress, char* data) {
	
	int mask = 0x000FFFFF;
	receiverAddress = receiverAddress & mask;
	int i = 0;
	while(i < (1024*1024*numOfIndices)) {

		long int senderAddress = (long int) &data[i];
		senderAddress = (senderAddress & mask);
		if(senderAddress == receiverAddress) {

			senderAddress = (long int) &data[i];
			for(int n = 0; n < numOfIndices; n++) {

				Addresses[0][n] = (char*) senderAddress + (8192 * 64 * n * 2);
				Addresses2[0][n] = Addresses[0][n] + 64;
			}
			break;
		}
		i++;
	}
	return;
}


int handshakeSender(char* data) {
	
	/* First allocate the 8 mB */
	data = (char*) malloc(1024 * 1024 * numOfIndices);
		
	/* Get the cache info */
	getCompInfo(3);
	
	/* Put the memory into the page table */
	long long int temp = 5;
	for(int i = 0; i < 1024 * 1024 * numOfIndices; i++) {
		data[i] = 7;
	}
	if(mlockall(MCL_CURRENT | MCL_FUTURE) < 0) {
		FILE* ERROR = fopen("errorS.txt", "a");
		fclose(ERROR);
	}
	
	/* Get matching sets */
	long int receiverAddress;
	cin >> receiverAddress;
	makeMatchingSets(receiverAddress, data);
	return true;
}

