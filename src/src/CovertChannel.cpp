#include <iostream>
#include <string>
#include <time.h>

#include "CovertChannel.h"
#include "Channel.h"
#include "Cache.h"
#include "MemBus.h"
#include "Load_Store.h"
#include "Flush+Reload.h"

using namespace std;

/* Initialize the type of channel that we will be communicating across */
CovertChannel::CovertChannel(string type, int stream_length, vector<long> params, FILE* rawFile, FILE* processedFile) {

/* Parameters all channels need */
        this->stream_length = stream_length;
        this->rawFile = rawFile;
        this->processedFile = processedFile;

/* Initialize the specific type of channel */
                                // TODO: Add all of the channels
        if(type == "L1")
                this->channel = new Cache(1,params);
        else if(type == "L2")
                this->channel = new Cache(2,params);
        else if(type == "L3")
                this->channel = new Cache(3,params);
        else if(type == "MemBus")
                this->channel = new MemBus(params);
        else if(type == "Load")
                this->channel = new Load_Store(LOAD, params);
        else if(type == "Store")
                this->channel = new Load_Store(STORE, params);
        else if(type == "Flush+Reload")
                this->channel = new Flush_and_Reload(params);
        else
                cout << "No matching channel type\n";
}


/* Simple wrapper for receiving across a channel. Delegate most of the
 * work to the channel itself.
 */
void CovertChannel::receive() {
	this->bit_stream = (long long int*) malloc(sizeof(bit_stream[0]) * stream_length * 2);
	this->channel->receive(this->stream_length, this->bit_stream);
}


/* Simple wrapper for sending across a channel. Delegate most of the
 * work to the channel itself.
 */
void CovertChannel::send() {
	this->bit_stream = (long long int*) malloc(sizeof(bit_stream[0]) * stream_length);
	generateRandomSequence();
	this->channel->send(this->stream_length, this->bit_stream);
}


/* Creates a random stream of bits to be sent across the channel.
 * NOTE: stream_length should be set first 
 */
void CovertChannel::generateRandomSequence() {

        srand(time(NULL));
        for(int i = 0; i < this->stream_length; i++) {
                int randomnum = rand();
                (randomnum > RAND_MAX/2) ? bit_stream[i] = 0 : bit_stream[i] = 1;
        }
}


void CovertChannel::receiverSave() {

	for(int i = 0; i < this->stream_length*2; i+=2) {
		fprintf(this->rawFile, "%lld %lld\n", this->bit_stream[i], this->bit_stream[i+1]);
	}

}

void CovertChannel::receiverSaveProcessed() {
        int * bit_buffer = (int *) malloc(sizeof(bit_buffer[0]) * this->stream_length);
        this->channel->analyzeReceived(this->stream_length
                                       , this->bit_stream
                                       , bit_buffer);
        for(int i = 0; i < this->stream_length; ++i) {
                fprintf(this->processedFile
                        , "%lld %d\n"
                        , this->bit_stream[i*2] , bit_buffer[i]);
        }
        free((void *) bit_buffer);
}

void CovertChannel::senderSave() {
        cerr << this->stream_length;
        for(int i = 0; i < this->stream_length; i+=1) {
                fprintf(this->processedFile, "%d %lld\n", i, this->bit_stream[i]);
        }
}
