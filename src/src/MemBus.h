#ifndef MemBus_H
#define MemBus_H

#include "Channel.h"

#include <vector>
#include <string>

using namespace std;

class MemBus : public Channel {

	private:
		long long int receiver;
		long long int sender;
		long long int receiver_loop;
		long long int sender_loop;
		long long int ignore_bits;
		long long int period;
		int eventSet = PAPI_NULL;
		int* fillerData;

		int num_sets;
		int line_size;
		int ways_associative;

	public:
		MemBus(vector<long> params);
		void receive(int stream_length, long long int* probes);
		void send(int stream_length, long long int* bitstream);
		void send0(long long stop);
    void analyzeReceived(int numbits, long long int* probes, int * bits);
};

#endif // #ifndef MemBus_H
