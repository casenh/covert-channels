#ifndef COVERTCHANNEL_H
#define COVERTCHANNEL_H

#include "Channel.h"

#include <vector>
#include <string>

using namespace std;

class CovertChannel {

	protected:
		Channel* channel;	/* The type of channel we're using */
		int stream_length;  /* Number of bits to send across the channel */
		long long int* bit_stream;	/* Where the receive stores the results, or the sender stores the bits to send */
		int* clustered_stream;	/* The clustered results after communication */ 
		FILE* rawFile;
		FILE* processedFile;
	
	public:
		CovertChannel(string type, int num_probes, vector<long> params, FILE* rawFile, FILE* processedFile);
		void receive(void);
		void send(void);
		void generateRandomSequence();
		void receiverSave();
    void receiverSaveProcessed();
		void senderSave();
};

#endif // #ifndef COVERTCHANNEL_H
