#ifndef CHANNEL_H
#define CHANNEL_H

#ifdef PERF_CTRS
#endif

#include <vector>
#include <string>
#include "../../papi-5.3.0/src/papi.h"

using namespace std;

/* Note: This class primarily functions as an abstract class. The particular channel is
 * instantiated in the constructor
 */
class Channel {

	protected:
		long long int TIME_MASK = 0x00000000FFFFFFFF; //Chop of the top of the bits so we only get seconds

		/* Info for the cache
		 * TODO: Place this in the cache channel? */


	public:

		/* Delegate the construction to the channels
		 */
		Channel(vector<long> params);

		/* Functions used by all channels
		 */
		long long int getTime();

		/* These functions are defined in the specific channel type
		 * you wish to use
		 */
		virtual void receive(int numbits, long long int* probes)=0;
		virtual void send(int numbits, long long int* bitstream)=0;
    virtual void analyzeReceived(int numbits, long long int* probes, int * bits)=0;
};

#endif // #ifndef CHANNEL_H
