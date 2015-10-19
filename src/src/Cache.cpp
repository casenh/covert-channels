#include <iostream>
#include <stdlib.h>
#include <vector>
#include "../../papi-5.3.0/src/papi.h"
#include "../../dlib-18.2/dlib/clustering.h"
#include "../../dlib-18.2/dlib/rand.h"
#include <malloc.h>

#define PERF_CTRS

#include "getCacheConfig.h"
#include "Cache.h"

using namespace dlib;

/**
 * Cache Constructor
 * Grab all relevant parameters 
 */
Cache::Cache(int Layer, std::vector<long> params) : Channel::Channel(params) {

        if(params.size() < 4) {
                cout << "Please specify all 4 cache channel parameters" << endl;
                exit(-1);
        }
        this->period = params[0];
        this->primeLength = params[1];
        this->accessLength = params[2];
        this->probeLength = params[3];

        eventSet = PAPI_NULL;
        /* Initialize PAPI if we are using performance counters */
        int retval = PAPI_library_init(PAPI_VER_CURRENT);
        if( retval != PAPI_VER_CURRENT) {
                fprintf(stderr,"PAPI library init error!\n");
                exit(1);
        }
        if(PAPI_create_eventset(&(this->eventSet)) != PAPI_OK){
                fprintf(stderr, "PAPI Fail Create\n");}
        if (Layer == 1) {
                if(PAPI_add_event(this->eventSet, PAPI_L1_DCM) != PAPI_OK){
                        fprintf(stderr, "PAPI Fail Add\n"); } }
        else if (Layer == 2) {
                if(PAPI_add_event(this->eventSet, PAPI_L2_TCM) != PAPI_OK){
                        fprintf(stderr, "PAPI Fail Add\n"); } }
        else if (Layer == 3) {
                if(PAPI_add_event(this->eventSet, PAPI_L3_TCM) != PAPI_OK){
                        fprintf(stderr, "PAPI Fail Add\n"); } }
        if(PAPI_start(this->eventSet) != PAPI_OK){
                fprintf(stderr, "PAPI Fail Start\n"); }

        struct cacheStuff * cachestuff = getCacheConfig(Layer);
        this->ways_associative = cachestuff->ways_associative;
        this->num_sets = cachestuff->num_sets;
        this->line_size = cachestuff->line_size;
        free(cachestuff);
        this->fillerData = (char*) malloc(sizeof(char) * num_sets * line_size * ways_associative * 6);
}


/**
 * Receive bits across the Cache Cache channel. Specify the number of bits and where
 * to store the bits
 */
void Cache::receive(int numbits, long long int* probes) {

        int index = 0;
        long long int counters[2];
        cout << "Ready\n";
        string ready;
        cin >> ready;
        while((getTime() & TIME_MASK) > 20000) {  }
        long long int startTime = getTime();

        for(int i = 0; i < numbits; i++) {

                while(getTime() < (startTime + (i * period))) { }
                do {
                        for(int n = 0; n < num_sets; n++){
                                fillerData[n * line_size + index * line_size * num_sets] = 1;
                        }
                        index++;
                        if(index > (ways_associative))
                                index = 0;
                } while(getTime() < (startTime + primeLength + (i * period)));
                index = 0;

                while(getTime() < (startTime + (primeLength + accessLength) + (i * period))) { }

                PAPI_reset(eventSet);

                for(int n = 0; n < num_sets ; n++) {
                        fillerData[index * line_size * ways_associative] = 1;
                        index++;
                }
                index = 0;

                PAPI_read(eventSet, counters);

                probes[2*i] = getTime() & TIME_MASK;
                probes[2*i+1] = counters[0];
        }
}


/**
 * Send a 1 across the Cache cache channel
 */
void Cache::send1(long long stop){
        int loc = 0;
        do {
                for(int n = 0; n < num_sets;  n++) {
                        fillerData[loc * line_size * num_sets + n * line_size] = 1;
                }
                loc++;
                if(loc > ( ways_associative))
                        loc = 0;
        } while(getTime() < stop);
}

void Cache::send(int stream_length, long long int* bit_stream) {

        cout << "Ready\n";
        string ready;
        cin >> ready;
        while((getTime() & TIME_MASK) > 20000) {  }
        long long int startTime = getTime();

        for(int n=0; n < stream_length; n++){

                while(getTime() < startTime + (n * period)) { }
                while(getTime() < startTime + primeLength  + (n * period)) { }

                long long int stopTime = startTime + primeLength + accessLength + (n * period);
                if(bit_stream[n] == 1)
                        send1(stopTime);
        }
}
/*
void Cache::analyzeReceived(int numbits, long long int * probes, int* bits){
    typedef matrix<double,1,1> sample_type;
    typedef radial_basis_kernel<sample_type> kerneltype;
    kcentroid<kerneltype> kc(kerneltype(0.1),.01,15);
    kkmeans<kerneltype> test(kc);
    std::vector<sample_type> samples;
    std::vector<sample_type> centers;
    sample_type M; //The matrix that holds our results

    for (int i = 0; i < numbits; ++i) {
            M(0) = probes[i*2+1];
            samples.push_back(M);
    }

    pick_initial_centers(2, centers, samples, test.get_kernel());
    find_clusters_using_kmeans(samples, centers, 6000);

    if (centers[1](0) < centers[0](0)) {
            sample_type temp = centers[0];
            centers[0] = centers[1];
            centers[1] = temp;
    }

    for (int i = 0; i < numbits; ++i) {
            bits[i] = ((abs(centers[0](0) - samples[i](0))
                        < (abs(centers[1](0) - samples[i](0))))) ? 0 : 1;
    }
    } */

void Cache::analyzeReceived(int numbits, long long int* probes, int * bits){
        long avg = 0;
        for (int i = 0; i < numbits; ++i) {
                avg += probes[i*2+1];
        }
        avg = avg/numbits;

        for (int i = 0; i < numbits; ++i) {
                bits[i] = ((probes[i*2+1] < avg)) ? 0 : 1;
        }
}
