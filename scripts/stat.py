#!/usr/bin/env python

import os, sys, math
from optparse import OptionParser

# Usage
usage = "Usage: %prog filename"
parser = OptionParser(usage=usage)

# Parse options and arguments
(options, args) = parser.parse_args()
if len(args) != 1:
  parser.print_help()
  sys.exit(1)

# Statistics
def read_floats(filename):
    with open(filename) as f:
        return map(float, f)
      
def average(x): return sum(x) * 1.0 / len(x)

      
dat = read_floats(args[0])
avg = average(dat)
var = map(lambda x: (x - avg)**2, dat)
std = math.sqrt(len(dat) / (len(dat)-1) * average(var))

print "%d %.3f %.3f" % (len(dat), avg, std)
