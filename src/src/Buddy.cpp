#include "Buddy.h"

#include <unistd.h>
#include <sys/syscall.h>
#include <linux/sched.h>
#include <sched.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include "../../papi-5.3.0/src/papi.h"

#include "getCacheConfig.h"

#define STACK_SIZE 1024*1024

Buddy::Buddy() {
}

int noop(void*) {
        while(true);
}

int const_noise(void*) {
        struct cacheStuff * cacheinfo = getCacheConfig(3);
        char * fillerData = (char*) malloc(sizeof(char) * cacheinfo->num_sets * cacheinfo->line_size * cacheinfo->ways_associative * 6);
        int loc = 0;
        long long coeff = cacheinfo->line_size;
        long long max = cacheinfo->ways_associative * cacheinfo->num_sets;
        while (true) {
                fillerData[loc * coeff] = 1;
                asm volatile("prefetcht2 %0"
                             :
                             : "m"(fillerData[loc * coeff])
                             :);
                loc++;
                if(loc > ( max ))
                        loc = 0;
        }
}

#define period (100000)

int pseudo_noise(void*) {
        struct cacheStuff * cacheinfo = getCacheConfig(3);
        char * fillerData = (char*) malloc(sizeof(char) * cacheinfo->num_sets * cacheinfo->line_size * cacheinfo->ways_associative * 6);
        int loc = 0;
        long long coeff = cacheinfo->line_size;
        long long max = cacheinfo->ways_associative * cacheinfo->num_sets;
        int lsr = 1;
        int bit = 1;
        long long startTime = PAPI_get_real_nsec();
        long long stopTime = 0;
        int i = 1;
        while (true) {
                bit = 1;
                lsr >>= 1;
                lsr ^= (-bit) & 0x90000000;
                stopTime = startTime + (i * period);
                if (bit == 1) {
                        do {
                                fillerData[loc * coeff] = 1;
                                asm volatile("prefetcht2 %0"
                                             :
                                             : "m"(fillerData[loc * coeff])
                                             :);
                                loc++;
                                if(loc > ( max ))
                                        loc = 0;
                        }while (PAPI_get_real_nsec() < stopTime);
                } else {
                        while (PAPI_get_real_nsec() < stopTime);
                }
                i++;
        }
}

long buddy_thread_pair(pid_t buddy) {
        long int ret;
        do {
                ret = syscall(323, buddy);
        } while (ret == -1 && errno == EAGAIN);
        return (ret == 0) ? 0 : errno;
}


void Buddy::start(string type) {
        this->type = type;
        void* stack;
        void* stack_top;
        pid_t pid;
        pid_t myPid;
        char statfilestring[30];
        FILE * statFD;
        int mycpu;
        int (*noise)( void *);
        myPid = getpid();
        setpgid(myPid, myPid);
        snprintf(&statfilestring[0], 30, "/proc/%d/stat", myPid);
        if ((statFD = fopen(statfilestring,"r")) == NULL) {
                fprintf(stderr,"could not open my pid file\n");
                exit(1);
        }
        fscanf(statFD, "%*d "); //pid
        fscanf(statFD, "%*s "); //exe
        fscanf(statFD, "%*c "); //state
        fscanf(statFD, "%*d "); //ppid
        fscanf(statFD, "%*d "); //pgrp
        fscanf(statFD, "%*d "); //session
        fscanf(statFD, "%*d "); //tty_nr
        fscanf(statFD, "%*d "); //tpgid
        fscanf(statFD, "%*u "); //flags
        fscanf(statFD, "%*u "); //minflt
        fscanf(statFD, "%*u "); //cminflt
        fscanf(statFD, "%*u "); //majflt
        fscanf(statFD, "%*u "); //cmajflt
        fscanf(statFD, "%*u "); //utime
        fscanf(statFD, "%*u "); //stime
        fscanf(statFD, "%*d "); //cutime
        fscanf(statFD, "%*d "); //cstime
        fscanf(statFD, "%*d "); //priority
        fscanf(statFD, "%*d "); //nice
        fscanf(statFD, "%*d "); //num_threads
        fscanf(statFD, "%*d "); //itrealvalue
        fscanf(statFD, "%*u "); //starttime
        fscanf(statFD, "%*d "); //vsize
        fscanf(statFD, "%*d "); //rss
        fscanf(statFD, "%*d "); //rsslim
        fscanf(statFD, "%*d "); //startcode
        fscanf(statFD, "%*d "); //encode
        fscanf(statFD, "%*u "); //startstack
        fscanf(statFD, "%*u "); //kstkesp
        fscanf(statFD, "%*u "); //kstkeip
        fscanf(statFD, "%*u "); //signal
        fscanf(statFD, "%*u "); //blocked
        fscanf(statFD, "%*u "); //sigignero
        fscanf(statFD, "%*u "); //sigcatch
        fscanf(statFD, "%*u "); //wchan
        fscanf(statFD, "%*u "); //nswap
        fscanf(statFD, "%*u "); //cnswap
        fscanf(statFD, "%*d "); //exit_signal
        fscanf(statFD, "%d ", &mycpu); //processor
        stack = malloc(STACK_SIZE);
        stack_top = (void*) ((char *) stack + STACK_SIZE);

        if(this->type == "noop") noise = noop;
        else if(this->type == "const-noise") noise = const_noise;
        else if(this->type == "pseudo-noise") noise = pseudo_noise;
        else {
                fprintf(stderr, "no noise specified\n");
                noise = noop;
        }

        pid = clone(noise, stack_top, 0, NULL);

        cpu_set_t mask;
        CPU_ZERO(&mask);
        CPU_SET(mycpu ^ 1, &mask);
        sched_setaffinity(pid, sizeof(mask), &mask);

        CPU_ZERO(&mask);
        CPU_SET(mycpu, &mask);
        sched_setaffinity(0, sizeof(mask), &mask);

        long amma = buddy_thread_pair( pid );
        this->buddyPid = pid;
}

void Buddy::stop() {
        // we are going down hard
        kill(0,SIGKILL);
}

