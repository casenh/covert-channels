#ifndef Flush_and_Reload_H
#define Flush_and_Reload_H

#include "Channel.h"

#include <vector>
#include <string>

using namespace std;

class Flush_and_Reload : public Channel {

	private:
		long long int period;
		long long int reader_wait;

	public:
		Flush_and_Reload(vector<long> params);
		void receive(int stream_length, long long int* probes);
		void send(int stream_length, long long int* bitstream);
		void send1(long long stop);
    long long int getTime();
    void analyzeReceived(int numbits, long long int* probes, int * bits);
};

#endif // #ifndef Flush_and_Reload_H
