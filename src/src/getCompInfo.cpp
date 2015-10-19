#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace std;

extern int NUM_OF_SETS;
extern int BLOCK_LENGTH;
extern int WAYS_ASSOCIATIVE;


void getCompInfo(int cacheNumber){

    char input[20];
    char input2[20];
    FILE* numOfSetsFile;
    FILE* blockLengthFile;
    FILE* waysAssociativeFile;
    FILE* typeFile;
    FILE* levelFile;
    
    string level("/sys/devices/system/cpu/cpu0/cache/index0/level");
    string type("/sys/devices/system/cpu/cpu0/cache/index0/type");
    string numOfSets("/sys/devices/system/cpu/cpu0/cache/index0/number_of_sets");
    string blockLength("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size");
    string waysAssociative("/sys/devices/system/cpu/cpu0/cache/index0/ways_of_associativity");
    char indexNumber = '0';
    
    /* Keep searching until we find the correct folder for the cache info */ 
    for(int i = 0; i < 8; i++) {
	/* Check both the level and type */
	levelFile = fopen(&level[0], "r");
	typeFile = fopen(&type[0], "r");
	
	if(levelFile == NULL || typeFile == NULL) { //Make sure the files exist
	    cout << "Cannot locate cache location" << endl;
	    exit(0);
	}
	fread(input, 1, 20, levelFile);
	fread(input2, 1, 20, typeFile);
	
	string data("Data");
	if(atoi(input) == cacheNumber && data.compare(input2)) { //If we found the correct folder
	    break;
	}
	
	fclose(levelFile); //Move to the next folder
	fclose(typeFile);
	indexNumber += 1;
	level[40] = indexNumber; //Change the index number
	type[40] = indexNumber;
    }
    
    numOfSets[40] = indexNumber; //Get the rest of the files we'll need
    blockLength[40] = indexNumber;
    waysAssociative[40] = indexNumber;
    numOfSetsFile = fopen(&numOfSets[0], "r");
    blockLengthFile = fopen(&blockLength[0], "r");
    waysAssociativeFile = fopen(&waysAssociative[0], "r");
    
    if(numOfSetsFile == NULL) {
	cout << "Missing cache info files" << endl;
	exit(1);
    }
    
    fread(input,1,20,numOfSetsFile); //Get the info from the files
    NUM_OF_SETS = atoi(input);
    fread(input,1,20,blockLengthFile);
    BLOCK_LENGTH = atoi(input);
    fread(input,1,20,waysAssociativeFile);
    WAYS_ASSOCIATIVE = atoi(input);
    
    fclose(numOfSetsFile);
    fclose(blockLengthFile);
    fclose(waysAssociativeFile);
}
