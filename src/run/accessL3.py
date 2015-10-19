#!/usr/bin/env python

import subprocess
from subprocess import call
import time

data = [
    #Period, primeTime, readerWait, sendLength
    "40000", "0", "8000", "8000",
    "39000", "0", "5000", "5000",
    "38000", "0", "5000", "5000",
    "37000", "0", "5000", "5000",
    "36000", "0", "5000", "5000",
    "35000", "0", "5000", "5000",
    "34000", "0", "5000", "5000",
    "33000", "0", "5000", "5000",
    "32000", "0", "5000", "5000",
    "31000", "0", "5000", "5000",
    "30000", "0", "5000", "5000",
    "29000", "0", "5000", "5000",
    "28000", "0", "5000", "5000",
    "27000", "0", "5000", "5000",
    "26000", "0", "5000", "5000",
    "25000", "0", "5000", "5000",
    "24000", "0", "5000", "5000",
    "23000", "0", "5000", "5000",
    "22000", "0", "5000", "5000",
    "21000", "0", "5000", "5000",
    "20000", "0", "5000", "5000",
    "19000", "0", "5000", "5000",
    "18000", "0", "5000", "5000",
    "17000", "0", "5000", "5000",
    "16000", "0", "5000", "5000",
    "15000", "0", "5000", "5000",
    "14000", "0", "5000", "5000",
    ]
    
if __name__ == '__main__':
	for n in range(0, 27):
		for i in range(0, 10): 
			#Start the threads on the same core so the handshake can be carried out
			reader = subprocess.Popen(['taskset', '-c', '0', '../src/accessL3_receiver', data[n*4], data[(n*4)+1], data[(n*4)+2]], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
			sender = subprocess.Popen(['taskset', '-c', '1', '../src/accessL3_sender', data[n*4], data[(n*4)+1], data[(n*4)+3]], stdin=subprocess.PIPE, stdout=subprocess.PIPE)		
			# Read in the start address from the reader
			startAddress = reader.stdout.readline()
			sender.stdin.write(startAddress)
			sender.stdin.flush()
				#~ print "Result: ", result
			#Read in the sequence from the sender
			sequence = sender.stdout.readline()
			reader.stdin.write(sequence)
			reader.stdin.flush()
			numOfBits = sender.stdout.readline()
			reader.stdin.write(numOfBits)
			reader.stdin.flush()
			print "Handshake Completed"
			#Communicate
			sender.wait()
			reader.wait()
			time.sleep(1)
		call(["../src/accessChecker"])
		call(["rm", "-r", "../data/"])
		call(["mkdir", "../data/"])
		print "Test ", n, " Complete"
