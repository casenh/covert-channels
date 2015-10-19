#include <iostream>
#include <vector>
#include <fstream>
#include "../../papi-5.3.0/src/papi.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../../dlib-18.2/dlib/clustering.h"
#include "../../dlib-18.2/dlib/rand.h"

using namespace std;
using namespace dlib;

typedef matrix<double,1,1> sample_type; //Samples will be 2-D column vectors
typedef radial_basis_kernel<sample_type> kerneltype; //Use a radial type kernel


/* This function goes through a given sample set, and outputs the raw data and clustered results	*
 * to a file.																						*/
void analyzeData(FILE* rawData, FILE* clusteredResults, std::vector<sample_type> &samples, long long int* data, int numSamples) {


	kcentroid<kerneltype> kc(kerneltype(0.1),.01,15); //This is the kcentroid object used in the kmeans test
	kkmeans<kerneltype> test(kc); //The kkmeans object that must be passed to the test
	std::vector<sample_type> initial_centers; //A vector to hold the inital centers we pick
	test.set_number_of_centers(2); //Tell the kkmeans object that we want to run the k-means with k = 2
		
	//Pick some initial centers for the k-means algorithm. Can use the 
	pick_initial_centers(2, initial_centers, samples, test.get_kernel()); 
	find_clusters_using_kmeans(samples, initial_centers, 6000);        
	
	/* Make sure that the lowered valued center is center0 */
	sample_type center0, center1;
	if(initial_centers[0] < initial_centers[1])
	{
		center0 = initial_centers[0];
		center1 = initial_centers[1];   
	}
	else
	{
		center1 = initial_centers[0];
		center0 = initial_centers[1];
	}  
		
//     fprintf(fp, "%f %f\n", center0(0), center1(0));
	/* Loop over all samples and print out their predicted class */
	int currdata = 0, result;
	for( int i = 0; i < numSamples; i++){ //Dump the data
		sample_type currSample = samples[i];
		int Distance0 = abs(center0(0) - currSample(0));
		int Distance1 = abs(center1(0) - currSample(0));
		if(Distance0 < Distance1)
		{
			result = 0;
		}
		else
		{
			result = 1;
		}
	
		fprintf(rawData, "%lld %lld\n", data[currdata+1], data[currdata]);
		fprintf(clusteredResults, "%lld %d\n", data[currdata+1], result);
		currdata += 2;
	}
}
