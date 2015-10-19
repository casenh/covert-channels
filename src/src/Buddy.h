#ifndef Buddy_H
#define Buddy_H

#include <string>
#include <sched.h>

using namespace std;

class Buddy {

	private:
        pid_t buddyPid;
        string type;

	public:
        Buddy();
        void start(string type);
        void stop();
};

#endif // #ifndef Buddy_H
