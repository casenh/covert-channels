#!/usr/bin/env python

data = map(
        lambda s: {"period":s[0], "primeTime":s[1], "readerWait":s[2], "sendLength":s[3]},
        [#Period, primeTime, readerWait, sendLength
        (3000000, 800000, 800000, 800000),
        (1400000, 400000, 600000, 600000),
        (1300000, 400000, 600000, 600000),
        (1200000, 400000, 600000, 600000),
        (1100000, 400000, 600000, 600000),
        (1000000, 400000, 600000, 600000),
        (900000,  400000, 500000, 500000),
        (800000,  300000, 500000, 500000),
        (700000,  300000, 400000, 400000),
        (600000,  300000, 300000, 300000),
        #"1500000", "400000", "600000", "600000",
        #"1500000", "400000", "600000", "600000",
        #"1500000", "400000", "600000", "600000",
        #"900000", "400000", "00000", "400000",
        #"800000", "350000", "350000", "350000",
        #"750000", "350000", "350000", "350000",
        #"700000", "300000", "300000", "300000",
        #"650000", "250000", "250000", "250000",
        #"600000", "250000", "250000", "250000",
        #"550000", "225000", "225000", "225000"
        #"500000", "200000", "200000", "200000"
        #"450000", "175000", "175000", "175000",
        #"400000", "150000", "150000", "150000",
        #"350000", "125000", "125000", "125000",
        #"300000", "100000", "100000", "100000",
        #"250000", "100000", "100000", "100000"
        #"200000", "75000", "75000", "75000"
        #"150000", "50000", "50000", "50000"
        #"125000", "50000", "50000", "50000"
        #"100000", "50000", "25000", "50000"
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
channel = covert_channel("L3", data
                         , coeffs=[1000] * 4
                         , runsPerTest=1
                         , readerCore=0
                         , senderCore=2)
if __name__ == '__main__':
        args = arguments.mkparser("L3").parse_args()
        if ('buddy' in args and args.buddy != None):
                channel = covert_channel("L3", data
                                         , buddy=args.buddy
                                         , coeffs=[1000] * 4
                                         , runsPerTest=1
                                         , readerCore=0
                                         , senderCore=2)
        if (args.learn):
                env = genetic.Environment(channel, size=100, maxgenerations=100, optimum=1-0.015)
                env.run()
                env.best.dump("L3Results.json")
        else:
                channel.run()

