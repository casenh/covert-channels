#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include "../../papi-5.3.0/src/papi.h"
#include "../../dlib-18.2/dlib/clustering.h"
#include "../../dlib-18.2/dlib/rand.h"

#include "Load_Store.h"

using namespace std;

Load_Store::Load_Store(enum storeOrLoad typeish, vector<long> params) : Channel::Channel(params) {
        if (params.size() < 2) {
                std::cout << "Please sepcify all 2 load channel parameters"
                          << std::endl;
                exit(-1);
        }

        this->period = params[0];
        this->wait = params[1];
		fprintf(stderr, "Wait: %lld\n", this->period);
        this->loadP = typeish;

        int retval = PAPI_library_init(PAPI_VER_CURRENT);
        if( retval != PAPI_VER_CURRENT) {
                fprintf(stderr,"PAPI library init error!\n");
                exit(1);
        }
        if((retval = PAPI_create_eventset(&this->eventSet)) != PAPI_OK){
                fprintf(stderr, "PAPI Failed Create %d\n", retval);
                cerr << PAPI_strerror(retval) << endl;
        }
        if((retval = PAPI_add_event(this->eventSet
                                   ,(typeish == LOAD) ? PAPI_LD_INS : PAPI_SR_INS)) != PAPI_OK){
                fprintf(stderr, "PAPI Failed Add %d\n", retval);
                cerr << PAPI_strerror(retval) << endl;
        }
        if((retval = PAPI_start(this->eventSet)) != PAPI_OK){
                fprintf(stderr, "PAPI Failed Start %d\n", retval);
                cerr << PAPI_strerror(retval) << endl;
        }
}

void Load_Store::receive(int numbits, long long int* probes) {
        register int temp;
        int array[numbits];
        long long int counters[2];

        std::cout << "Ready\n";
        string ready;
        std::cin >> ready;

        while((getTime() & TIME_MASK) > 20000) {}
        long long startTime = getTime();

        for (int i = 0; i < numbits; i++){
                while (getTime() < (startTime + (i * period))) ;

                if(PAPI_reset(eventSet) != PAPI_OK)
                        fprintf(stderr,"PAPI Fail!\n");

                long long stopTime = startTime + wait + (i *period);
				int first = 1;
                if (this->loadP == LOAD) {
                        do {
							if(first == 1) {
							for(int n = 0; n < 1000; n++) {
                                temp = array[n];
                                temp = array[n];
                                temp = array[n];
							}
							}
							first = 0;
                        } while(getTime() < stopTime);
				}
                 else {
                        do {
							if(first == 1) {
								for(int n = 0; n < 1500; n++) {
                                array[i] = temp++;
                                array[i] = temp++;
                                array[i] = temp++;
								}
								first = 0;
							}
                        } while(getTime() < stopTime);
				 }
                if(PAPI_read(eventSet, counters) != PAPI_OK)
                        fprintf(stderr,"PAPI Fail!\n");

                probes[2*i] = getTime() & TIME_MASK;
                probes[2*i+1] = counters[0];
        }
}
/* unused function */
void Load_Store::send1(long long stopTime) { }

void Load_Store::send(int stream_length, long long int * bit_stream) {
        cout << "Ready\n";
        string ready;
        cin >> ready;
        int array[stream_length];
        register int temp;
        while((getTime() & TIME_MASK) > 20000) {}
        long long startTime = getTime();

        for (int i = 0; i < stream_length; i++) {
                while(getTime() < (startTime + (i * period))) {}
                if (bit_stream[i] == 0) {
                        long long stopTime = startTime + wait + (i * period);
                        if (this->loadP == LOAD) {
                                do{
									for(int n = 0; n < 500; n++) {
                                        temp = array[n];
                                        temp = array[n];
                                        temp = array[n];
									}
                                } while(getTime() < stopTime);
						}
                        else {
                                do {
									for(int n = 0; n < 1000; n++) {
                                        array[n] = temp++;
                                        array[n] = temp++;
                                        array[n] = temp++;
									}
                                } while(getTime() < stopTime);
						}
                }
        }
}

using namespace dlib;

void Load_Store::analyzeReceived(int numbits, long long int* probes, int * bits) {

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
}
