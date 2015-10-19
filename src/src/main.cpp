#include <string>
#include <vector>
#include <iostream>
#include <stdlib.h>

#include "CovertChannel.h"
#include "Channel.h"

#include <boost/program_options.hpp>
#include "channel.hpp"
#include "Buddy.h"
namespace po = boost::program_options;

int main(int argc, char* argv[]) {

        po::options_description ops("Allowed Options");
        ops.add_options()
                ("sender,s","create a sender in this process")
                ("reader,r","create a reader in this process")
                ("buddy,b", po::value<std::string>(), "enable buddy threading with noise")
                ("type", po::value<std::string>(),"type of the covert chael (L1 cache, MemBuss, etc.)")
                ("numprobes", po::value<long>(),"number of bits to send over the channel")
                ("raw", po::value<std::string>(),"raw output file")
                ("processed", po::value<std::string>(),"proscessed output file")
                ("tuning", po::value<std::vector<long> >(),"tuning parameters")
                ;
        po::positional_options_description args;
        args.add("type",1);
        args.add("numprobes",1);
        args.add("raw",1);
        args.add("processed",1);
        args.add("tuning",-1);
        po::variables_map vm;
        po::store(po::command_line_parser(argc,argv).options(ops).positional(args).run(),vm);
        po::notify(vm);
        FILE * raw, * processed;
        if (vm.count("raw") < 1
            || vm.count("processed") < 1
            || (vm.count("sender") && vm.count("reader"))) {
                std::cout << ops << "\n";
                exit(1);
        }
        /* TODO: pass these in */
        raw = fopen(vm["raw"].as<std::string>().c_str(),"w");
        processed = fopen(vm["processed"].as<std::string>().c_str(),"w");

        CovertChannel cc = CovertChannel(vm["type"].as<std::string>()
                                         ,vm["numprobes"].as<long>()
                                         ,vm["tuning"].as<std::vector<long> >()
                                         ,raw
                                         ,processed);

        Buddy bt;
        if(vm.count("buddy")) {
                bt.start(vm["buddy"].as<std::string>());
        }
        if(vm.count("reader")) {
                cc.receive();
                cc.receiverSave();
                cc.receiverSaveProcessed();
        }
        else if (vm.count("sender")) {
                cc.send();
                cc.senderSave();
        }
        fclose(raw);
        fclose(processed);
        if(vm.count("buddy")) {
                bt.stop();
        }

        return 0;
}
