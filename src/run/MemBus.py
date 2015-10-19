#!/usr/bin/env python

import subprocess
from subprocess import call
import time

coeffs = [125, 125, 1, 1, 10]
readArgs = ["period", "sender", "readLoop", "sendLoop", "ignoreBits"]
sendArgs = ["period", "sender", "readLoop", "sendLoop", "ignoreBits"]

data = map(
        lambda s: {"period":s[0], "reader":s[1]
                   , "sender":s[2], "readLoop":s[3]
                   , "sendLoop":s[4], "ignoreBits":s[5]},
        [
        #Period,      Reader(useless),    Sender,   Reader_loop,     Sender_loop  IGNORE_bits
        #10000000, 3500000, 3500000, 10000,   50000  # 100 Bps
        #5000000, 500000, 500000, 1000,   10000  # 200 bps
        (1000000, 550000, 550000, 1500,        4000,   	1000), # 1 kB  98.53% gap=6mc sec
        #1000000, 500000, 500000, 1000,        5000  # 1 kB
        (500000, 400000, 250000, 500,        200, 	1000), #2 kb 99.47% skip=0 gap=2mc sec
        #500000, 200000, 200000, 500,        5000      # 2 k)B
        #200000, 120000, 120000, 200,        200, #5 kb
        (200000, 120000, 100000, 200,        500, 	2000), #5 kb 99.8% skip=1000 gap=3 or 5 mc sec (2 components)
        #200000, 50000, 50000, 250,        750      # 5 kB
        (100000, 50000, 50000, 200,        500,	3500), #10 kb 99.50% skip=2500  gap=5mc sec
        #100000, 50000, 50000, 200,        500      # 10 kB

        #50000, 25000, 20000, 80,        110,  # 20 kB
        (50000, 10000, 10000, 200,        400,      	5000), # 20 kB  98.99% skip=4000 gap=2 mc sec
        #35000, 10000, 15000, 60,        80,
        (35000, 10000, 10000, 200,        400,      	8000), # ~30 kB 98.73% skip=7000 gap=2 mc sec
        #25000, 12500, 10000, 60,        80,      # 40 kB
        (25000, 12500, 10000, 60,        120,      	10000), # 40 kB 99.72% skip = 7500 gap=0.75 mc sec
        #25000, 7000, 7000, 100,        400      # 40 kB skip)=8500 gap=2mc sec
        (20000, 10000, 10000, 60,        80,     	15000), # 50 kB 95.4% skip=11500 gap = 1 mc sec
        #20000, 10000, 7000, 60,        120,     # 50 kB 95.4)% skip=11500 gap = 1.2 mc sec
        #20000, 10000, 10000, 50,        80,     # 50 kB 95.4)% skip=11500 gap = 1 mc sec
        #20000, 5000, 5000, 100,        150      # 50 kB
        (15000, 10000, 7500, 50,        35,     	20000), # 67 kB 95.67% skip=13000 0.8 mc sec
        (15000, 10000, 8000, 50,        60,     	20000), # 67 kB 95.67% skip=13000 0.8 mc sec
        (10000, 5000, 5000, 35,        35,      	30000), # 100 kb max 94.79% av.=93.33% skip=23k gap=0.5mc sec
        #5000, 2500, 2000, 20,        15,      # 200 kB max =) 92.23% av.=90.57%
        (5000, 2500, 2000, 20,        25,      	60000), # 200 kB max=95.43% av=94.37% skip=48k gap=0.2 mc sec
        #3500, 2500, 1500, 20,        15,      # 300 kb
        (3500, 2500, 1500, 15,        20,      	100000), # 300 kb max=95.13% av.93.27% skip=80k gap=0.1 mc sec
        #3000, 2500, 1200, 15,        10,      # 330kb kb 84.0)6% gap=0.1 mc sec
        (3000, 2500, 1200, 15,        20,      	100000), # 330kb max=94.19% av.=92.63% skip= 92k gap= 0.1 mc sec
        (2500, 2500, 1200, 10,        20,      	160000), # 330kb max=95.10% av.=93.67% skip=140k gap= 0.2 mc sec
        #2500, 2500, 800, 15,        10,	      # 400kb
        #2000, 2500, 800, 15,        10,	      # 500kb
        (2000, 2500, 1000, 10,        20,		300000), #500kb max=94.55% av=93.89% skip = 215k gap = 0.1 mc sec
        (1800, 2500, 900, 10,        20,		700000), #555kb av=90.37%
        (1700, 2500, 900, 8,        20,		700000), #555kb av=77.65%
        (1700, 2500, 900, 8,        15,		700000), #588kb av=93.14%
        (1600, 2500, 800, 7,        15,		700000), #625kb av=92.64%
        (1400, 2500, 700, 7,        15,		700000), #714kb av=90.35%
        (1300, 2500, 700, 7,        12,		700000), # av=85.30%
        (1200, 2500, 500, 5,        12,		700000), #833kb av=93.11%
        (1100, 2500, 400, 5,        12,		700000), #909kb av=92.20%
        #1000, 2500, 300, 5,        12,	#1Mb av=81.19%
        (1000, 2500, 300, 4,        12,		700000), #1Mb av=91.19%
        (900, 2500, 300, 3,        10,			700000), #1.1Mb av=89.56%
        (800, 2500, 200, 3,        8,			700000), #1.25Mb av=86.38%
        (700, 2500, 200, 1,        4,			700000), #1.428Mb av=77.77%
        (600, 2500, 200, 1,        1,			700000), #1.428Mb av=55-67% ignore 600k
        ][16:17])
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
channel = covert_channel("MemBus", data
                         , coeffs=coeffs
                         , timeBetweenRuns=0.3
                         , runsPerTest=1
                         , readerCore=0
                         , senderCore=1
                         , readerArgs=readArgs
                         , senderArgs=sendArgs)

if __name__ == '__main__':
        args = arguments.mkparser("MemBus").parse_args()
        if ('buddy' in args and args.buddy != None):
                channel = covert_channel("MemBus", data
                                         , buddy=args.buddy
                                         , timeBetweenRuns=0.3
                                         , runsPerTest=1
                                         , readerCore=0
                                         , senderCore=2
                                         , readerArgs=readArgs
                                         , senderArgs=sendArgs)
        if (args.learn):
                env = genetic.Environment(channel, size=100, maxgenerations=100, optimum=1-0.015)
                env.run()
                env.best.dump("MemBusResults.json")
        else:
                channel.run()
