#!/usr/bin/env python

import subprocess
from subprocess import call
import time

data = map(
        lambda s: {"period":s[0], "primeTime":s[1], "readerWait":s[2], "sendLength":s[3]},
        [
              #Period, prime, read, send
                #    , time , wait, length
                (45000,14000,14000,14000),
                (40000,12000,12000,12000),
                (30000,10000,10000,10000),
                (30000, 8000, 8000, 8000),
                (20000, 4000, 4000, 4000),
                (19000, 4000, 4000, 4000),
                (18000, 4000, 4000, 4000),
                (17000, 4000, 4000, 4000),
                (16000, 4000, 4000, 4000),
                (15000, 4000, 4000, 4000),
                (14000, 4000, 4000, 4000),
                (13000, 4000, 4000, 4000),
                (13000, 4000, 4000, 4000),
                (12000, 4000, 4000, 4000),
                # these don't make sence
                (11000, 4000, 4000, 4000),
                (10000, 4000, 4000, 4000),
                (9000,  4000, 4000, 4000),
                (8000,  3000, 4000, 4000),
                (7000,  2000, 4000, 4000),
                (6000,  0,    4000, 4000),
                (5000,  5000, 5000, 5000),
                (4000,  5000, 5000, 5000),
                (3000,  5000, 5000, 5000)
        ][1:2])

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
channel = covert_channel("L2", data
		         , timeBetweenRuns=0.3
                         , runsPerTest=1
                         , readerCore=0
                         , senderCore=4)
if __name__ == '__main__':
        args = arguments.mkparser("L2").parse_args()
        if ('buddy' in args and args.buddy != None):
                channel = covert_channel("L2", data
                                         , buddy=args.buddy
                                         , timeBetweenRuns=0.3
                                         , runsPerTest=1
                                         , readerCore=0
                                         , senderCore=4)
        if (args.learn):
                env = genetic.Environment(channel, size=100, maxgenerations=100, optimum=1-0.015)
                env.run()
                env.best.dump("L2Results.json")
        else:
                channel.run()
