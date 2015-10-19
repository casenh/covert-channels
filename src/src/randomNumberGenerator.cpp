#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <vector>

#include <string.h>
#include "../../papi-5.3.0/src/papi.h"

using namespace std;

/* This function creates a random sequence of 1000 bits to be transmitted */
/* This function creates a random sequence of 1000 bits to be transmitted */
void RandomNumberGenerator(vector<int> *randomNumber, int length){
    srand(time(NULL));
    for(int i = 0; i < length; i++) {
		int randomNum = rand();
		if(randomNum > RAND_MAX/2)
			randomNumber->push_back(1);
		else
			randomNumber->push_back(0);
	}
}
