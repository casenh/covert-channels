#!/usr/bin/env python

import subprocess
from subprocess import call
import time

data = [
    #Period,      Reader(useless),    Sender,   Reader_loop,     Sender_loop  IGNORE_bits
"1000000", "550000", "550000", "15000",        "40000", "0", # 1 kB  99.98% gap=100 mc sec
"500000", "400000", "250000", "1500",        "4000", 	"0", #2 kb 99.99% skip=0 gap=20 mc sec
"200000", "120000", "100000", "1500",        "4000", 	"0",#5 kb 99.998% skip=0 gap=20 mc sec
"100000", "50000", "50000", "1500",        "3000",	"0",#10 kb 99.45% skip=0  gap=18 mc sec
"50000", "10000", "25000", "1300",        "2500",      	"0", # 20 kB  99.78% skip=0 gap=10 mc sec
"35000", "10000", "10000", "800",        "2000",      	"0",# ~30 kB 99.23% skip=0 gap=7.5 mc sec
"25000", "12500", "10000", "800",        "1500",      	"0",# 40 kB 98.12% skip = 0 gap=8 mc sec
"20000", "10000", "10000", "600",        "1200",     	"0",# 50 kB 98.633% skip=0 gap = 6 mc sec
"15000", "10000", "7500", "500",        "700",     	"0",# 67 kB 96.64% skip=0 gap=4 mc sec
"10000", "5000", "5000", "400",        "500",      	"0",# 100 kb av=95.92% skip=0 gap=4 mc sec
	#increases frequency "5000", "2500", "1500", "300",        "400",      	"0",# 200 kB channel breaks
"5000", "2500", "1500", "100",        "100",      	"0",# 200 kB av=98.04% skip=0 gap=1 mc sec
"3500", "2500", "1500", "100",        "150",      	"0",# 300 kb av=96.65% skip=0 gap=0.8 mc sec
"3000", "2500", "1200", "100",        "80",      	"0",# 330kb av=96.83% skip=0 gap= 0.4 mc sec		
"2000", "2500", "1000", "50",        "40",		"0",#500kb av=96.61% skip = 0 gap = 0.3 mc sec
"1800", "2500", "900", "50",        "40",		"0",#555kb av=95.07%	gap=0.2 mc sec
"1700", "2500", "900", "50",        "35",		"0",#555kb av=94.04%
"1600", "2500", "800", "50",        "30",		"0",#625kb av=93.24% gap=0.25 mc sec
"1400", "2500", "600", "40",        "25",		"0",#714kb av=91.98% gap=0.15 mc sec
"1300", "2500", "700", "40",        "25",		"0",#769Kb av=86.06%
"1200", "2500", "500", "25",        "25",		"0",#833kb av=95.57% 
"1100", "2500", "400", "25",        "20",		"0",#909kb av=88.97% 
"1000", "2500", "300", "20",        "20",		"0",#1Mb av=93.26% gap=0.05 mc sec
"900", "2500", "300", "17",        "20",		"0",#1.1Mb av=90.8% gap=0.05 mc sec
"800", "2500", "300", "14",        "14",		"0",#1.25Mb av=87.07%
"700", "2500", "300", "10",        "10",		"0",#1.428Mb av=81.84% gap=0 mc sec
"600", "2500", "300", "6",        "6",			"0",#1.667Mb av=63.35%
    ]

if __name__ == '__main__':
	param_num = 6
	#call(["rm", "../data/*"])
	for n in range(0,len(data)/param_num):
		for i in range(0, 20): #Run the test 10 times
		    reader = subprocess.Popen(['taskset', '-c', '0', '../src/readerAES', data[param_num*n], data[(param_num*n)+1], data[(param_num*n)+3], data[(param_num*n)+5]], stdin=subprocess.PIPE, stdout=subprocess.PIPE)		    
		    sender = subprocess.Popen(['taskset', '-c', '2', '../src/senderAES', data[param_num*n], data[(param_num*n)+2], data[(param_num*n)+4], data[(param_num*n)+5]], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
		    while True:
			senderOutput = sender.stdout.readline()
			if senderOutput != None:
			    break
		    print senderOutput
		    reader.stdin.write(senderOutput)
		    sender.stdin.write("Start\n")
		    sender.wait()
		    reader.wait()
		    time.sleep(3)
		#call(["../src/checker"])
		#call(["rm", "../data/*"])
		#call(["mkdir", "../data/"])


'''
	for p in range(10000, 80000, 5000):
		for reader_time in range(3000, p, 5000):
			for sender_time in range(3000, p, 5000):
				for reader_inner_loop in range(30, 1000, 50):
					for sender_inner_loop in range(30, 2000, 50):
						if 40*sender_inner_loop > sender_time or 200*reader_inner_loop > reader_time:
							continue

						for i in range(0, 1): #Run the test 10 times
						    print " period= ", p, " sender= ", sender_time, " reader= ", reader_time, " reader_loop= ", reader_inner_loop, " sender_loop= ", sender_inner_loop, "i= ", i
						    reader = subprocess.Popen(['taskset', '-c', '0', '../src/readerMemBus', str(p), str(reader_time), str(reader_inner_loop)], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
						    sender = subprocess.Popen(['taskset', '-c', '1', '../src/senderMemBus', str(p), str(sender_time), str(sender_inner_loop)], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
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
						call(["rm", "-r", "../data/"])
						call(["mkdir", "../data/"])
'''
