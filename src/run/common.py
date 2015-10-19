from __future__ import print_function
import subprocess
import time
import shutil
import os
import numpy
import json
import genetic
import random

env = os.environ.copy()
try :
    env["LD_LIBRARY_PATH"] += ":" + env["PWD"] + "/bin"
except KeyError:
    env["LD_LIBRARY_PATH"] = env["PWD"] + "/bin"

base_dir = os.path.dirname(os.path.dirname(__file__))
data_dir = os.path.join(base_dir , "data")

bin_dir = os.path.join(base_dir , "bin")
run_dir = os.path.join(base_dir , "run")
result_dir = os.path.join(base_dir, "results")
main_bin = os.path.join(bin_dir, "main")

class covert_channel:
    def __init__(self, channelName, tests, runsPerTest=10, buddy=None, coeffs=None
                       , senderCore=0, readerCore=2, timeBetweenRuns=1
                       , readerArgs=["period", "primeTime", "readerWait", "sendLength"]
                       , senderArgs=["period", "primeTime", "readerWait", "sendLength"]) :
        if buddy is not None:
            self.sender = ['taskset', '-c', str(senderCore)
                           , main_bin, "-b", buddy
                           , "-s", channelName, str(10000)]
            self.resultFile = os.path.join(result_dir, channelName +  "-"
                                           + buddy + ".json")
        else:
            self.sender = ['taskset', '-c', str(senderCore)
                           , main_bin, "-s", channelName, str(10000)]
            self.resultFile = os.path.join(result_dir, channelName + ".json")
        self.reader = ['taskset', '-c', str(readerCore)
                       , main_bin, "-r", channelName, str(10000)]
        if coeffs is None:
            coeffs = [125] * len(readerArgs)
        self.coeffs = coeffs
        self.cool_down = timeBetweenRuns
        self.readerArgs = readerArgs
        self.senderArgs = senderArgs
        self.runs = runsPerTest
        self.tests = tests

    def readChannelFile(self, f):
        with open(f) as fd:
            return [int(datum.split(" ")[1]) for datum in fd.readlines()]

    def check(self, sender, reader):
        readerData = self.readChannelFile(reader)
        senderData = self.readChannelFile(sender)
        transition = numpy.zeros((2,2),dtype=numpy.float64)
        if (len(senderData) != len(readerData)) :
            print("Data length mismatch!")
        elif (len(senderData) == 0) :
            print("No Data!")
        else:
            for sent,read in zip(senderData,readerData):
                transition[sent,read] += 1
            print(transition)
        return transition

    def capacity(self, transition):
        m,n = transition.shape
        r = numpy.ones((m,)) / m
        q = numpy.zeros((m,n))
        error_allowed = 1e-5/m
        for i in range(n):
            if transition[:,i].sum() == 0:
                print("error: transition matrix contains a zero columnn")
                return 0

        for i in range(m):
            if transition[i].sum() == 0:
                print("error: transition matrix contains a zero row")
                return 0
            transition[i] /= transition[i].sum()

        for iter in xrange(10000):
            for i in range(n):
                q[:,i] = r * transition[:,i]
                q[:,i] /= q[:,i].sum()
            rprime = r.copy()
            for i in range(m):
                rprime[i] = (q[i]**transition[i]).prod()
            rprime = rprime/rprime.sum()
            if (numpy.linalg.norm(rprime - r)) < error_allowed:
                break
            else:
                r = rprime

        cap = 0
        for i in range(m):
            for j in range(n):
                if (r[i] > 0 and q[i,j] > 0):
                    cap += r[i]*transition[i,j]*numpy.log2(q[i,j]/r[i])
        return cap


    def doTest(self, paramMap):
        print("Test parameters:{} ".format(paramMap))
        shutil.rmtree(data_dir)
        try: os.mkdir(data_dir)
        except (OSError) : pass
        contents = {}
        if paramMap["period"] != 0 :
	    bitsPerSec = 3500 * 1000 * 1000 / paramMap["period"]
        else:
            bitsPerSec = 0
        if not os.path.isfile(self.resultFile) \
           or os.path.getmtime(main_bin) < os.path.getmtime(self.resultFile):
            try:
                with open(self.resultFile) as results:
                    contents = json.load(results)
            except OSError:
                pass
            except Exception:
                print("Warning: corrupted results file")
        key = json.dumps([paramMap[arg] for arg in self.readerArgs])
	if (key in contents) and (len(contents[key]) >= 3):
      	    return max(contents[key]) * bitsPerSec
        for i in range(self.runs):
            print("  run #{}...".format(i), end='')
            readerOut = os.path.join(data_dir,"readerProcessed")
            senderOut = os.path.join(data_dir,"senderProcessed")
            reader = subprocess.Popen(self.reader
                                      + [str(os.path.join(data_dir,"readerRaw"))
                                         , str(readerOut)]
                                      + [str(paramMap[arg]) for arg in self.readerArgs]
                                      ,stdin=subprocess.PIPE
                                      ,stdout=subprocess.PIPE
                                      ,cwd=run_dir
                                      ,env=env)
            sender = subprocess.Popen(self.sender
                                      + [str(os.path.join(data_dir,"senderRaw"))
                                         , str(senderOut)]
                                      + [str(paramMap[arg]) for arg in self.senderArgs]
                                      ,stdin=subprocess.PIPE
                                      ,stdout=subprocess.PIPE
                                      ,cwd=run_dir
                                      ,env=env)
            while True:
                senderOutput = sender.stdout.readline()
                if senderOutput is not None: break
            while True:
                readerOutput = reader.stdout.readline()
                if readerOutput is not None: break
            sender.stdin.write("Start\n")
            reader.stdin.write("Start\n")
            time.sleep(self.cool_down)
            sender.wait()
            reader.wait()
            print("Done")
            if (key not in contents) :
                contents[key] = []
            cap = float(self.capacity(self.check(senderOut, readerOut)))
            print("  Capacity:{}".format(cap))
            print("  Troughput:{}".format(cap * bitsPerSec))
            contents[key] += [cap]
        with open(self.resultFile, "w+") as results:
            json.dump(contents,results)
            results.flush()
        return max(contents[key]) * bitsPerSec

    def run(self):
        for test in self.tests:
            self.doTest(test)

    def __call__(self):
        return channel_individual(self)

class channel_individual(genetic.Individual):
    def __init__(self, channel, chromosome=None ):
        self.channel = channel
        self.args = channel.readerArgs
        self.length = (len(self.args)) * 10
        self.chromosome = chromosome or self._makechromosome()
        self.score = None

    def _makechromosome(self):
        toRet = []
        for coeff in self.channel.coeffs:
            chromepart = random.randint(0,1023)
            for i in range(10):
                toRet += (chromepart & (1 << i)) >> i,
        return toRet

    def copy(self,chromosome = None):
        twin = self.__class__(self.channel,chromosome or self.chromosome[:])
        twin.score = self.score
        return twin

    def evaluate(self, optimum):
        grouped_chromosomes = [0 for _ in self.args]
        for i,chrom in zip(range(len(self.chromosome)),self.chromosome):
            grouped_chromosomes[i/10] *= 2
            grouped_chromosomes[i/10] += chrom
        parammap = {arg:(chrom*coeff) for arg,chrom,coeff in zip(self.args,grouped_chromosomes,self.channel.coeffs)}
        self.score = self.channel.doTest(parammap)
        return self.score

    def dump(self, fileish):
        grouped_chromosomes = [0 for _ in self.args]
        for i,chrom in zip(range(len(self.chromosome)),self.chromosome):
            grouped_chromosomes[i/10] *= 2
            grouped_chromosomes[i/10] += chrom
        parammap = {arg:(chrom*125) for arg,chrom in zip(self.args,grouped_chromosomes)}
        with open(fileish, "w") as fd :
            json.dump({"parameters":parammap, "score":self.score}, fd)
