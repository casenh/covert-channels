#!/usr/bin/env python

readArgs = ["period", "readerWait"]
sendArgs = ["period", "readerWait"]
data = map(
        lambda s: {"period":s[0], "readerWait":s[1]},
        [#Period, readerWait
        (3000, 1800),
        (1400, 600),
        (1300, 600),
        (1200, 600),
        (1100, 600),
        (1000, 600),
        (900,  500),
        (800,  500),
        (700,  400),
        (600,  300),
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
channel = covert_channel("Flush+Reload", data
                         , coeffs=[1] * 2
                         , runsPerTest=1
                         , readerCore=0
                         , senderCore=2
                         , readerArgs=readArgs
                         , senderArgs=sendArgs)
if __name__ == '__main__':
        args = arguments.mkparser("Flush+Reload").parse_args()
        if ('buddy' in args and args.buddy != None):
                channel = covert_channel("Flush+Reload", data
                                         , buddy=args.buddy
                                         , coeffs=[1000] * 4
                                         , runsPerTest=1
                                         , readerCore=0
                                         , senderCore=2)
        if (args.learn):
                env = genetic.Environment(channel, size=100, maxgenerations=100, optimum=1-0.015)
                env.run()
                env.best.dump("Flush+ReloadResults.json")
        else:
                channel.run()

