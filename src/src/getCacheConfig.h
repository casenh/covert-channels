#ifndef __getCacheConfig__
#define __getCacheConfig__

struct cacheStuff {
        long long int num_sets;
        long long int line_size;
        long long int ways_associative;
};

struct cacheStuff * getCacheConfig(int layer);

#endif
