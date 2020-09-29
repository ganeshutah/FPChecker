#!/usr/bin/env python3

import re
import json
import pathlib
import argparse
import subprocess
import sys
import os
from nvcc_parser import ClangCommand
from colors import prGreen,prCyan,prRed
import strace_module

# --------------------------------------------------------------------------- #
# --- Installation Paths ---------------------------------------------------- #
# --------------------------------------------------------------------------- #

# Main installation path
FPCHECKER_PATH      ='/usr/global/tools/fpchecker/blueos_3_ppc64le_ib_p9/fpchecker-0.1.3-clang-9.0.0'

# Clang version path
#FPCHECKER_LIB       =FPCHECKER_PATH+'/lib64/libfpchecker_plugin.so'
#FPCHECKER_RUNTIME   =FPCHECKER_PATH+'/src/Runtime_plugin.h'
FPCHECKER_LIB       ='/usr/workspace/wsa/laguna/fpchecker/FPChecker/build/libfpchecker_plugin.so'
FPCHECKER_RUNTIME   ='/usr/workspace/wsa/laguna/fpchecker/FPChecker/src/Runtime_plugin.h'
CLANG_PLUGIN        ='-Xclang -load -Xclang '+FPCHECKER_LIB+' -Xclang -plugin -Xclang instrumentation_plugin'
LLVM_PASS_CLANG     =CLANG_PLUGIN+' -include '+FPCHECKER_RUNTIME+' -emit-llvm'

# LLVM version path
LLVM_PASS_LLVM = '-Xclang -load -Xclang ' + FPCHECKER_PATH + '/lib64/libfpchecker.so -include ./Runtime.h ' 

# --------------------------------------------------------------------------- #
# --- Global Variables ------------------------------------------------------ #
# --------------------------------------------------------------------------- #

# Options added to all clang commands by default
ADD_OPTIONS = ['-Qunused-arguments', '-g']

# File extensions that can have CUDA code
CUDA_EXTENSION = ['.cu', '.cuda'] + ['.C', '.cc', '.cpp', '.CPP', '.c++', '.cp', '.cxx']

# Flags that will be added to nvcc when
# it is called to compile instrumented code
NVCC_ADDED_FLAGS = []

# Main database of commands to re-compile the application.
# This is a list of tuples [clang_command, nvcc_command]:
#   clang_command: converted nvcc command to clang
#   nvcc_command: original nvcc command with pre-included flags from NVCC_ADDED_FLAGS 
COMMANDS_DB = []

# Enable Clanf versus LLVM version
CLANG_VERSION = True 

# Functions to be omitted from instrumentation
OMIT_SOURCE_FILES = []

# Command from which we restart re-compilation
RESTART_COMMAND = 1

# Defines a mapping of original names and new names for files
FILE_NAMES_MAP = {'file1': 'file1_copy'}

# Regex: archive command
arPattern = re.compile(r'[/]?ar\s+(.+)\s+(.+\.a)\s+.+')

# --------------------------------------------------------------------------- #
# --- Classes --------------------------------------------------------------- #
# --------------------------------------------------------------------------- #

class CompilationCommand:
  """ This class parses a compilation line and categorizes it. """

  def __init__(self, line):
    self.line = line                  # un-parsed original command
    self.nvcc_command = False         # nvcc command
    self.link_command = False         # links something, e.g., a library
    self.program_link_command = False # links the final program
    self.archive_command = False      # archive command for static libraries
    self.ranlib_command = False       # randlib command for static libraries 

    tokens = line.split()

    # Link command or not?
    if ('-c ' not in line and 
        '--compile ' not in line and
        '-dc ' not in line and
        '--device-c ' not in line and
        '-o ' in line):
      self.link_command = True

    # Progam link command?
    if self.link_command:
      idx = tokens.index('-o')
      output = tokens[idx+1]
      if not output.endswith('.o'):
        self.program_link_command = True
    
    for t in tokens:
      # nvcc command?
      if t == 'nvcc' or t.endswith('/nvcc'):
        self.nvcc_command = True

      # archive command?
      if t == 'ar' or t.endswith('/ar'):
        self.archive_command = True
      
      # ranlib command?
      if t == 'ranlib' or t.endswith('/ranlib'):
        self.ranlib_command = True

  def convertArchiveCommand(self):
    """ Makes changes to the srchive command. """
    global arPattern
    if self.archive_command:
      foundAr = arPattern.search(self.line)
      if foundAr:
        x = self.line.find(foundAr.group(1))
        y = x + len(foundAr.group(1))
        sub = self.line[x:y].replace('q','r')
        return self.line.replace(foundAr.group(1), sub)
    return None

# --------------------------------------------------------------------------- #
# --- Functions ------------------------------------------------------------- #
# --------------------------------------------------------------------------- #

#def isRanlibCommand(line):
#  """ Return true is the line contains the ranlib command. """
#  for t in line.split():
#    if t == 'ranlib' or t.endswith('/ranlib'):
#      return True
#  return False

#def modifyArchiveCommandIfNeeded(line):
#  found = False
#  tokens = line.split()
#  ar_idx = None
#  for t in tokens:
#    if t == 'ar' or t.endswith('/ar'):
#      ar_idx = tokens.index(t)
#      break
#  # We found the ar command
#  library_idx = None
#  if ar_idx != None:
#    for t in tokens:
#      if t.endswith('.a'):
#        library_idx = tokens.index(t)
#        break
#  # We found the ar command and library:
#  if ar_idx != None and library_idx != None:
#    found = True
#    for i in range(ar_idx+1, library_idx):
#     tokens[i] = tokens[i].replace('q', 'r')
#  line = ' '.join(tokens)
#  return (found, line)

# Remove the object file option from the command line, i.e., -o file.o
def removeObjectFile(line, fileName):
  if '-o ' in line:
    tokens = line.split()
    idx = tokens.index('-o')
    objectName = tokens[idx+1]
    origName = os.path.splitext( os.path.split(objectName)[1] )[0]
    FILE_NAMES_MAP[origName] = origName
    del(tokens[idx])
    del(tokens[idx])
    line = ' '.join(tokens)
  else:
    name = os.path.splitext( os.path.split(fileName)[1] )[0]
    FILE_NAMES_MAP[name] = name+'_copy'

  return line

# Is it a link comand?
#def isLinkCommand(line):
#  if ('-c ' not in line and 
#    '--compile ' not in line and
#    '-dc ' not in line and
#    '--device-c ' not in line and
#    '-o ' in line):
#    return True
#  return False 

# Is it the command that links the final program?
#def isProgramLinkCommand(line):
#  if isLinkCommand(line):
#    tokens = line.split()
#    idx = tokens.index('-o')
#    output = tokens[idx+1]
#    if not output.endswith('.o'):
#      return True
#  return False 

def changeNameOfExecutable(line):
  tokens = line.split()
  idx = tokens.index('-o') + 1
  progName = tokens[idx]
  tokens[idx] = progName + '_fpc'
  line = ' '.join(tokens)
  return line

def changeNameOfObjectFiles(line):
  #print(FILE_NAMES_MAP)
  tokens = line.split()
  for i in range(len(tokens)):
    if tokens[i].endswith('.o'):
      name = os.path.splitext( os.path.split(tokens[i])[1] )[0]
      if name in FILE_NAMES_MAP:
        newObjectName = FILE_NAMES_MAP[name]
        newObjectName = os.path.join( os.path.split(tokens[i])[0], newObjectName+'.o')
        line = line.replace(tokens[i], newObjectName, 1)
  return line

def replaceFileName(line):
  fileName = getCodeFileName(line)
  newFileName = None
  if fileName != None:
    extension = fileName.split('.')[-1:][0]
    nameOnly = os.path.splitext(fileName)[0]
    newFileName = nameOnly+'_copy.'+extension
    line = line.replace(fileName, newFileName, 1)
  return (fileName, newFileName, line)

# Replace the original name of the source file
# Create a copy of the source file
def replaceFileNameAndCopy(line):
  (fileName, newFileName, line) = replaceFileName(line)
  if fileName != None:
    # Create a copy of the source file
    idx = line.index('clang++')
    copyCommand = '  cp -f ' + fileName + ' ' + newFileName + ' && '
    line = line[:idx] + copyCommand + line[idx:]
  return line, fileName

# Get the name of the file being compiled
def getCodeFileName(line):
  tokens = line.split()
  fileName = None
  for t in tokens:
    for ext in CUDA_EXTENSION:
      if t.endswith(ext):
        fileName = t
  return fileName

def isNVCC(line):
  return 'nvcc ' in line

#def isClang(line):
#  return 'clang++ ' in line

def convertCommand(line):
  cmpCmd = CompilationCommand(line)

#  if isProgramLinkCommand(line):
#    line = changeNameOfExecutable(line)
#    if CLANG_VERSION:
#      line = changeNameOfObjectFiles(line)
#    COMMANDS_DB.append([line, ''])
#    return

  if cmpCmd.program_link_command:
    line = changeNameOfExecutable(line)
    line = changeNameOfObjectFiles(line)
    COMMANDS_DB.append([line, ''])
    return

#  if isLinkCommand(line):
#    COMMANDS_DB.append([line, ''])
#    return

  if cmpCmd.link_command:
    COMMANDS_DB.append([line, ''])
    return

  # Check if it's archive command
#  (found, line) = modifyArchiveCommandIfNeeded(line)
#  if found:
#    COMMANDS_DB.append([line, ''])
#    return

  if cmpCmd.archive_command:
    line = cmpCmd.convertArchiveCommand()
    COMMANDS_DB.append([line, ''])
    return

  # Check if it's a randlib command
#  if isRanlibCommand(line):
#    COMMANDS_DB.append([line, ''])
#    return

  if cmpCmd.ranlib_command:
    COMMANDS_DB.append([line, ''])
    return

  # Skip is not an nvcc compilation command
#  if not isNVCC(line):
#    COMMANDS_DB.append([line, ''])
#    return

  if not cmpCmd.nvcc_command:
    COMMANDS_DB.append([line, ''])
    return

  newLine = ClangCommand(line).to_str()

  # Add options after clang command
  newLine = newLine.replace('clang++ ', 'clang++ '+' '.join(ADD_OPTIONS)+' ', 1)

  if CLANG_VERSION:
    newLine, origFileName = replaceFileNameAndCopy(newLine)
    newLine = removeObjectFile(newLine, origFileName)

  # Add original command
  origCommand = replaceFileName(line)[2]
  newNVCCCommand = 'nvcc -include ' + FPCHECKER_RUNTIME + ' '
  newNVCCCommand = newNVCCCommand + ' '.join(NVCC_ADDED_FLAGS) + ' '
  origCommand = origCommand.replace('nvcc ', newNVCCCommand)

  COMMANDS_DB.append([newLine, origCommand])

def replayCommands():
  global RESTART_COMMAND, OMIT_SOURCE_FILES

  checkTraceFileExists()
  fileName = getTraceFileName()

  with open(fileName) as fd:
    for line in fd:
      convertCommand(line)

  for i in range(len(COMMANDS_DB))[RESTART_COMMAND-1:]:
    cmd = COMMANDS_DB[i]

    # Omit some source files
    sourceFileName = getCodeFileName(cmd[0])
    if sourceFileName:
      for f in OMIT_SOURCE_FILES:
        if sourceFileName.endswith(f):
          cmd[0] = 'echo "Skipping: ' + f + '"'

    prCyan('Instrumenting ' + str(i+1) + '/' + str(len(COMMANDS_DB)))
    try:
      print(cmd[0])
      cmdOutput = subprocess.check_output(cmd[0], stderr=subprocess.STDOUT, shell=True)
      print(cmdOutput.decode('utf-8'))
    except subprocess.CalledProcessError as e:
      prRed('Error:')
      print(e.output.decode('utf-8'))
      exit(-1)

    if CLANG_VERSION:
      if cmd[1] != '':
        print(cmd[1])
        try:
          cmdOutput = subprocess.check_output(cmd[1], stderr=subprocess.STDOUT, shell=True)
          print(cmdOutput.decode('utf-8'))
        except subprocess.CalledProcessError as e:
          prRed('Error:')
          print(e.output.decode('utf-8'))
          exit(-1)

def execTraces():
  checkTraceFileExists()
  fileName = getTraceFileName()
  prGreen('Executing commands from ' + fileName)
  fd = open(fileName, 'r')
  allLines = fd.readlines()
  fd.close()

  i = 1
  total = str(len(allLines))
  for cmd in allLines:
    try:
      print(str(i) + '/' + total + ': ' + cmd[:-1])
      cmdOutput = subprocess.check_output(cmd, stderr=subprocess.STDOUT, shell=True)
    except subprocess.CalledProcessError as e:
      sys.exit('FPCHECKER: error: ' + cmd)
    i = i + 1

def loadConfigFile():
  global RESTART_COMMAND, OMIT_SOURCE_FILES
  confFile = './fpchecker_conf.json'
  if os.path.exists(confFile):
    print('Loading', confFile)
    data = None
    with open(confFile, 'r') as fd:
      data = json.load(fd)
    
    if data != None:
      for k in data.keys():
        if k == '--skip_files':
          OMIT_SOURCE_FILES = data[k]
        if k == '--restart_command':
          RESTART_COMMAND = data[k]

def checkTraceFileExists():
  fileName = getTraceFileName() 
  if not os.path.exists(fileName):
    sys.exit('FPCHCKER: error: no traces file found')

def getTraceFileName():
  return strace.getTracesDir() + '/executable_traces.txt'

# --------------------------------------------------------------------------- #
# --- Main ------------------------------------------------------------------ #
# --------------------------------------------------------------------------- #

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='FPChecker tool')
  parser.add_argument('build_command',  help='Build command (e.g., make).', nargs=argparse.REMAINDER)
  parser.add_argument('--no-subnormal', action='store_true', help='Disable checking for subnormal numbers (underflows).')
  parser.add_argument('--no-warnings', action='store_true', help='Disable warnings of small or large numbers (overflows and underflows).')
  parser.add_argument('--no-abort', action='store_true', help='Print reports without aborting; allows to check for errors/warnings in the entire execution the program.')
  parser.add_argument('--no-checking', action='store_true', help='Do not perform any checking.')
  parser.add_argument('--record', action='store_true', help='Record build traces only')
  parser.add_argument('--replay', action='store_true', help='Replay build traces (without instrumentation)')
  parser.add_argument('--inst-replay', action='store_true', help='Instrument and replay build traces')
  args = parser.parse_args()
  #print(args)
  #exit()

  prGreen('FPChecker')

  loadConfigFile()

  if args.no_subnormal:
    NVCC_ADDED_FLAGS.append('-DFPC_DISABLE_SUBNORMAL')

  if args.no_warnings:
    NVCC_ADDED_FLAGS.append('-DFPC_DISABLE_WARNINGS')

  if args.no_abort:
    NVCC_ADDED_FLAGS.append('-DFPC_ERRORS_DONT_ABORT')

  if args.no_checking:
    NVCC_ADDED_FLAGS.append('-DFPC_DISABLE_CHECKING')

  if CLANG_VERSION:
    ADD_OPTIONS.append(LLVM_PASS_CLANG)
  else:
    ADD_OPTIONS.append(LLVM_PASS_LLVM)

  prog = args.build_command
  strace = strace_module.CommandsTracing(prog)

  default_behavior = False
  # In the default behavior we only record and inst-replay
  if ((not args.record and 
      not args.replay and
      not args.inst_replay) or 
        (args.record and 
        args.replay and 
        args.inst_replay)):
    default_behavior = True

  if default_behavior:
    args.record = True
    args.inst_replay = True
    args.replay = False

  if args.record:
    prGreen('Command: ' + ' '.join(prog))
    prCyan('Tracing and saving compilation commands...')
    strace.startTracing()
    strace.analyzeTraces()
    strace.writeToFile()

  if args.replay:
    prCyan('Attempting to re-compile (without instrumentation)...')
    execTraces()

  if args.inst_replay:
    prCyan('Attempting to instrument and re-compile...')
    replayCommands()


