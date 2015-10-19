#!/usr/bin/env python

import subprocess
from subprocess import call
import time

# The Period length, reader numBranches, sender numBranches (3 is max for both)
data = [
    "20000", "3", "3",
    #~ "19000", "3", "3",
    #~ "18000", "3", "3",
    #~ "17000", "3", "3",
    #~ "16000", "3", "3",
    #~ "15000", "3", "3",
    #~ "14000", "3", "3",
    #~ "13000", "3", "3",
    #~ "12000", "2", "2",
    #~ "11000", "2", "2",
    #~ "10000", "1", "1",
    #~ "9000", "0", "1",
    "8000", "-1", "1",
    "7000", "-1", "-1",
    "6000", "-2", "-1",
    "5000", "-3", "-1",
    "4000", "1", "1",
    ]

if __name__ == '__main__':
	for n in range(0, 3):
		for i in range(0, 20): #Run the test 10 times
		    reader = subprocess.Popen(['taskset', '-c', '0', '../src/readerBranch', data[n*3], data[(n*3)+1]], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
		    sender = subprocess.Popen(['taskset', '-c', '4', '../src/senderBranch', data[n*3], data[(n*3)+2]], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
		    while True:
			senderOutput = sender.stdout.readline()
			if senderOutput != None:
			    break
		    print senderOutput
		    reader.stdin.write(senderOutput)
		    sender.stdin.write("Start\n")
		    sender.wait()
		    reader.wait()
		    time.sleep(1)
		call(["../src/checker"])
		rootFolder = "../Branch/" + str(data[n*3])
		call(["mv", "../data/", rootFolder])
		call(["mkdir", "../data/"])
