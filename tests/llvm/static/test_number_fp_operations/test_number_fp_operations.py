#!/usr/bin/env python

import subprocess
import os

def setup_module(module):
    THIS_DIR = os.path.dirname(os.path.abspath(__file__))
    os.chdir(THIS_DIR)

def teardown_module(module):
    cmd = ["make clean"]
    cmdOutput = subprocess.check_output(cmd, stderr=subprocess.STDOUT, shell=True)


def test_1():
    cmd = ["make"]
    cmdOutput = subprocess.check_output(cmd, stderr=subprocess.STDOUT, shell=True)

    passed = False
    ontargetFunc = False
    for l in cmdOutput.split("\n"):
        if "#FPCHECKER: Instrumenting function:" in l and "powerd" in l:
            ontargetFunc = True
        
        if ontargetFunc == True:
            if "#FPCHECKER: Instrumented operations:" in l:
                val = int(l.split()[-1:][0])
                assert val == 6
		break
	
