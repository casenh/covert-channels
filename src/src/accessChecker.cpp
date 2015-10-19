//This script takes a file as the first input (usually starting with ../data/). The second argument is a sequence of 1's and 0's (ex. 100101). 
//It then scans the file, and displays the percentage correct that the file has. 

#include <stdio.h>
#include <iostream>
#include <math.h>
#include "../../papi-5.3.0/src/papi.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <vector>

using namespace std;

/* This function takes the input function where the data is stored, and finds the corresponding "sender." file for it */
void getFileTime(char* dataFile, char* fileTime){
    int n = 2, i = 0;
    
    while(dataFile[n] != '.'){ n++; }
    n++;
    strcpy(fileTime, ".");
    strcat(fileTime, &dataFile[n]);
    printf("The file time is: %s\n", fileTime);
    return;
}

float standardDeviationCalculation(int deletionArray[], int sentDataArray[], int numberOfFilesRead, float mean){
    int n = 0;
    float stdDeviation = 0;
    
    /* Calculate the Standard Deviation */
    for(n = 0; n < numberOfFilesRead; n++)
    {
	stdDeviation += ((((float)deletionArray[n]/(float)sentDataArray[n]) - mean) * (((float)deletionArray[n]/(float)sentDataArray[n]) - mean));
    }   
    stdDeviation = stdDeviation/((float)numberOfFilesRead-1);
    
    return sqrt(stdDeviation);
}

int flippedBitCalculator(int flippedBits[], int numberOfFilesRead){
    int temp = 0;
    
    for(int i = 0; i < numberOfFilesRead; i++)
    {
	temp += flippedBits[i];
    }
    
    return temp;  
}


float meanCalculation(int deletionArray[], int sentDataArray[], int numberOfFilesRead){
    
    int totalDeletions = 0, totalSent = 0;
    float mean = 0;
    int n;
    
    /* Calculate the mean */
    for(n = 0; n < numberOfFilesRead; n++)
    {
	totalDeletions += deletionArray[n];
	totalSent += sentDataArray[n];
    }
    printf("TOTAL DEL: %d	TOTALSENT: %d\n", totalDeletions, totalSent);
    mean = (float)totalDeletions/(float)totalSent;
    
    return mean;
}

int main(int argc, char* argv[]){
    int n = 0, totalOneSent = 0, totalZeroSent = 0, index=0, numOfBits = 0; //The total number of bits sent
    int oneToOne = 0, oneToZero = 0, zeroToZero = 0, zeroToOne = 0; //Number of bit flips that are seen
    float oneToOnePercent = 0, oneToZeroPercent = 0, zeroToZeroPercent = 0, zeroToOnePercent = 0; //The percent of bit flips seen
    int zeroToZeroArray[2000], zeroToOneArray[2000], oneToOneArray[2000], oneToZeroArray[2000], numberOfFilesRead = 0;
    float mean = 0, standardDeviation = 0;
    char fileName[45], readValue[2000], timeStamp[30], channelType[10], testedValue = 0, timesRepeatedCharacter = 0;
    char period[10], senderWait[10], readerWait[10];
    vector<char> inputData, senderData;
    FILE *results, *senderFile, *receiverFile, *receiverFiles, *senderFiles;
    
    /* Grab the files that contain the receiver and sender time stamps */
    receiverFiles = fopen("../data/receiverFiles", "r");
    senderFiles = fopen("../data/senderFiles", "r");
    while(fgets(readValue, 2000, receiverFiles))
    {
	zeroToZero = zeroToOne = oneToOne = oneToZero = 0; //Reset all the values before going again
	inputData.clear();
	senderData.clear();
	
	n = 0;
	while(readValue[n] != '\n') //Extract the name of the file that we will be reading from
	{
	    fileName[n] = readValue[n];
	    n++;
	}
	fileName[n] = '\0'; //Add the null terminated character to end of name
	
	/* Open the data files for the sender and receiver. Go line by line and determine the number of 1's 
	 * and 0's received */
	receiverFile = fopen(fileName, "r");
	if(receiverFile == NULL){ printf("The data file does not exist. Please enter a valid data file.\n"); exit(0); }
	
	fgets(readValue, 2000, senderFiles);
	n = 0;
	while(readValue[n] != '\n') {
	    fileName[n] = readValue[n];
	    n++;
	}
	fileName[n] = '\0';
	
	printf("Sender file: %s\n", fileName);
	senderFile = fopen(fileName, "r");
	if(senderFile == NULL){ printf("Error opening the sender file!\n"); exit(0); }
	
	/* Read in the results that was seen by the receiver */
	while(fgets(readValue, 100, receiverFile) != NULL){
	    n = 0;
	    while(readValue[n] != ' '){ n++;}
	    n++;
	    if(readValue[n] == '1'){ //Determine if a 1 or 0 was recorded by the reciever
		inputData.push_back('1');
	    }
	    else{
		inputData.push_back('0');
	    }
	}
	
	/* Read in the data that was sent by the sender */
// 	fgets(readValue, 100, senderFile); //The first two lines are starting and ending time, so ignore them
// 	fgets(readValue, 2000, senderFile);
	while(fgets(readValue, 2, senderFile)) //Extract the sequence that was sent by the sender
	{
	    if(readValue[0] == '1')
	    {
		senderData.push_back('1');
		totalOneSent += 1;
	    }
	    else
	    {
		senderData.push_back('0');
		totalZeroSent += 1;
	    }
	}
    
	/* Now we go back through the data and count the deletions */
	index = 0;
	n = 0;
	if(inputData.size() != senderData.size()) //If we didn't receive the same amount that was sent, throw it away
	{
	    cout << "ERROR! The sender and receiver differ in the number of bits sent" << endl;
	    cout << "Input size: " << inputData.size() << "	Sender size: " << senderData.size() << endl;
	}
	else{
	    while(index < inputData.size())
	    {
		/* Compare the bit that was received to the bit that was sent */
		if(inputData[index] == '0' && senderData[index] == '0') { zeroToZero += 1; }
		else if(inputData[index] == '1' && senderData[index] == '0') { zeroToOne += 1; }
		else if(inputData[index] == '1' && senderData[index] == '1') { oneToOne += 1; }
		else if(inputData[index] == '0' && senderData[index] == '1') { oneToZero += 1; }	
		index++;
	    }
	}
	/* If there's a bad handshake that throws off the data, ignore it */
	if((zeroToOne+500) > zeroToZero || (oneToZero+500) > oneToOne) {
	    
	    totalOneSent -= (oneToOne + oneToZero);
	    totalZeroSent -= (zeroToOne + zeroToZero);
	}
	else {
	    printf("Zero to Zero: %d	Zero to One: %d	     One to One: %d      One to Zero: %d\n", zeroToZero, zeroToOne, oneToOne, oneToZero);
	    zeroToZeroArray[numberOfFilesRead] = zeroToZero;
	    zeroToOneArray[numberOfFilesRead] = zeroToOne;
	    oneToOneArray[numberOfFilesRead] = oneToOne;
	    oneToZeroArray[numberOfFilesRead] = oneToZero;
	    numberOfFilesRead++;
	    fclose(senderFile);
	    fclose(receiverFile);
	}
    }
    
    strcpy(fileName, "TestResults");
    results = fopen(fileName, "a");
    
    cout << "Total Sent: " << totalOneSent + totalZeroSent << endl;
    printf("numberOfFilesRead: %d\n", numberOfFilesRead);
    zeroToZeroPercent = (float)flippedBitCalculator(zeroToZeroArray, numberOfFilesRead)/(float)totalZeroSent; //Calculates the percent correct and percent error
    zeroToOnePercent = (float)flippedBitCalculator(zeroToOneArray, numberOfFilesRead)/(float)totalZeroSent;
    oneToOnePercent = (float)flippedBitCalculator(oneToOneArray, numberOfFilesRead)/(float)totalOneSent;
    oneToZeroPercent = (float)flippedBitCalculator(oneToZeroArray, numberOfFilesRead)/(float)totalOneSent;
    
    //fprintf(results, "Period: %sReaderWait: %sSenderWait: %s", period, readerWait, senderWait);
     fprintf(results, "Percent Zero to Zero Flips: %f \nPercent Zero to One Flips: %f \nPercent One to One Flips: %f \nPercent One to Zero Flips: %f\n\n", zeroToZeroPercent, zeroToOnePercent, oneToOnePercent, oneToZeroPercent);
    //fprintf(results, "%f\n", (zeroToZeroPercent * .5) + (oneToOnePercent * .5));
    fclose(results);
    
    
}
