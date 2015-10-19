#!/usr/bin/env python

import subprocess
from subprocess import call
import time

data = map(
        lambda s: {"period":s[0], "primeTime":0, "readerWait":s[1], "sendLength":s[2]},
        [
                #Period, Reader, Sender
                (50000, 20000, 20000),
                #"19000", "5000", "5000",
                #"18000", "5000", "5000",
                #"17000", "5000", "5000",
                #"16000", "5000", "5000",
                #"15000", "5000", "5000",
                #"14000", "5000", "5000",
                #"13000", "5000", "5000",
                #"12000", "5000", "5000",
                #"11000", "5000", "5000",
                #"10000", "5000", "5000",
                #"9000", "5000", "5000",
                #"8000", "5000", "5000",
                #"7000", "5000", "5000",
                #"6000", "4000", "4000",
                #"5000", "5000", "5000",
                #"4000", "5000", "5000",
                #"3000", "5000", "5000",
                #"2000", "5000", "5000"
        ])

if __package__ is None :
        import sys
        from os import path
        sys.path.append(path.dirname(path.abspath(__file__)))
        from common import covert_channel
else:
        from .common import covert_channel
channel = covert_channel("Flops", data
                       , runsPerTest=10
                       , readerCore=0
                       , senderCore=1)

if __name__ == "__main__":
        channel.run()
