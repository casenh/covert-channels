#!/usr/bin/env python

import subprocess
from subprocess import call
import time

readArgs = ["period", "reader"]
sendArgs = ["period", "sender"]
data = map(
        lambda s: {"period":s[0], "reader":s[1], "sender":s[2]},
        [
                #Period, Reader, Sender
                #"23000", "6000", "6000",
                #"22000", "6000", "6000",
                #"21000", "6000", "6000",
                #"20000", "6000", "6000",
                #"19000", "6000", "6000",
                #"18000", "6000", "6000",
                #"17000", "6000", "6000",
                #"16000", "6000", "6000",
                #"15000", "6000", "6000",
                #"14000", "6000", "6000",
                #"13000", "6000", "6000",
                #"12000", "6000", "6000",
                #"11000", "6000", "6000",
                #"10000", "6000", "6000",
                #"9000", "6000", "6000",
                (23000, 6000, 6000),
                (8000, 6000, 6000),
                (7000, 6000, 6000),
                (6000, 5500, 5500),
        ][0:1])

if __package__ is None :
        import sys
        from os import path
        sys.path.append(path.dirname(path.abspath(__file__)))
        from common import covert_channel
else:
        from .common import covert_channel
channel = covert_channel("Store", data
                         , runsPerTest=1
                         , readerCore=0
                         , senderCore=4
                         , readerArgs=readArgs
                         , senderArgs=sendArgs)

if __name__ == '__main__':
        channel.run()
