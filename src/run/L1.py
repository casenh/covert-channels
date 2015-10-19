#!/usr/bin/env python

import subprocess
from subprocess import call
import time

data = map(
        lambda s: {"period":s[0], "primeTime":s[1], "readerWait":s[2], "sendLength":s[3]},
        [
                #Period, ReaderPrime, ReaderWait, SendLength
                (20000, 20875, 15000, 53250), # results of genetic algorithm. Capacity: .9852
                (20000, 5000, 5000, 5000),
                (19000, 5000, 5000, 5000),
                (18000, 5000, 5000, 5000),
                (17000, 5000, 5000, 5000),
                (16000, 4000, 5000, 5000),
                (15000, 3000, 5000, 5000),
                (14000, 2000, 5000, 5000),
                (13000, 1000, 5000, 5000),
                (12000, 0, 5000, 5000),
                (11000, 0, 4000, 4000),
                (10000, 0, 4000, 4000),
                (9000, 0, 3000, 3000),
                (8000, 0, 3000, 3000),
                (7000, 0, 2000, 2000),
                (6000, 0, 2000, 2000),
                (5000, 0, 2000, 2000),
                (4000, 0, 2000, 2000),
                (3000, 0, 1000, 1500),
                (2000, 0, 1000, 1000)
        ][0:1])

if __package__ is None :
        import sys
        from os import path
        sys.path.append(path.dirname(path.abspath(__file__)))
        from common import covert_channel
        import genetic
        import arguments
else:
        from .common import covert_channel
        from . import genetic
        from . import arguments
channel = covert_channel("L1", data
		       , timeBetweenRuns=0.3
                       , runsPerTest=1
                       , readerCore=0
                       , senderCore=4)

if __name__ == '__main__':
        args = arguments.mkparser("L1").parse_args()
        if ('buddy' in args and args.buddy != None):
                channel = covert_channel("L1", data
                                         , buddy=args.buddy
                                         , timeBetweenRuns=0.3
                                         , runsPerTest=1
                                         , readerCore=0
                                         , senderCore=4)
        if (args.learn):
                env = genetic.Environment(channel, size=100, maxgenerations=100, optimum=1-0.015)
                env.run()
                env.best.dump("L1Results.json")
        else:
                channel.run()
