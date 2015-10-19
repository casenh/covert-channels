#ifndef Cache_H
#define Cache_H

#include "Channel.h"

#include <vector>
#include <string>

using namespace std;

class Cache : public Channel {

	private:
		long long int primeLength;
		long long int accessLength;
		long long int probeLength;
		long long int period;
		int eventSet = PAPI_NULL;
		char* fillerData;

		int num_sets;
		int line_size;
		int ways_associative;

	public:
		Cache(int Layer,vector<long> params);
		void receive(int stream_length, long long int* probes);
		void send(int stream_length, long long int* bitstream);
		void send1(long long stop);
    void analyzeReceived(int numbits, long long int* probes, int * bits);
};

#endif // #ifndef Cache_H
