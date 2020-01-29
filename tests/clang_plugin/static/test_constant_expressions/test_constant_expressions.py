#!/usr/bin/env python

import subprocess
import os

source = "compute.cu"

def setup_module(module):
    THIS_DIR = os.path.dirname(os.path.abspath(__file__))
    os.chdir(THIS_DIR)

def teardown_module(module):
    cmd = ["make -f Makefile.0 clean"]
    cmdOutput = subprocess.check_output(cmd, stderr=subprocess.STDOUT, shell=True)

def test_1():
    cmd = ["make -f Makefile.0"]
    cmdOutput = subprocess.check_output(cmd, stderr=subprocess.STDOUT, shell=True)
   
    # it should find one time instrumention with 0.0, not with 0 (integer) 
    numbwerOfTransformations = 0
    fd = open(source, "r")
    for l in fd:
        if "_FPC_CHECK_(0." in l:
            numbwerOfTransformations = numbwerOfTransformations + 1
    fd.close()
    
    assert numbwerOfTransformations == 1
