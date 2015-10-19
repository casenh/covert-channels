#channels += Branch
channels += Cache
channels += Load_Store
channels += MemBus
channels += Flush+Reload
#channels += MemSSE
#channels += Flops

.SUFFIXES:

debug ?= @

$(eval $(shell mkdir -p bin))

CFLAGS += -O0
CFLAGS += -std=c++11
CFLAGS += -g
CFLAGS += -msse4.1

LDFLAGS += -lboost_system
LDFLAGS += -lboost_program_options
LDFLAGS += -ldummy
LDFLAGS += -L${PWD}/bin

all: bin/main
.PHONY: all clean
clean:
	${debug}echo -n Cleaning Binaries...
	-${debug}rm -rf bin
	${debug}echo Done

bin/%.o: src/%.c
	${debug}echo "C++ " $(filter %.c,$^) " -> " $@
	${debug}g++ ${CFLAGS} -c -fPIC $(filter %.c,$^) $(filter %.o,$^) -o $@

bin/lib%.so: bin/%.o
	${debug}echo "LD  " $(filter %.o,$^) " -> " $@
	${debug}g++ -shared $(filter %.o,$^) $(filter %.a,$^) -o $@

bin/%: src/%.cpp
	${debug}echo "C++ " $(filter %.cpp,$^) " -> " $@
	${debug}g++ ${CFLAGS} $(filter %.cpp,$^) $(filter %.a,$^) -o $@ ${LDFLAGS}

bin/main: $(addprefix src/, $(addsuffix .cpp, ${channels} Channel CovertChannel Buddy getCacheConfig))
bin/main: ../papi-5.3.0/src/libpapi.a bin/libdummy.so
