#ifndef Load_Store_H
#define Load_Store_H

#include "Channel.h"

#include <vector>
#include <string>

using namespace std;

enum storeOrLoad {
        STORE,
        LOAD,
};

class Load_Store : public Channel {

	private:
		long long int period;
    long long wait;
    enum storeOrLoad loadP;
		int eventSet = PAPI_NULL;

	public:
		Load_Store(enum storeOrLoad typeish, vector<long> params);
		void receive(int stream_length, long long int* probes);
		void send(int stream_length, long long int* bitstream);
		void send1(long long stop);
    void analyzeReceived(int numbits, long long int* probes, int * bits);
};

#endif // #ifndef Load_Store_H
