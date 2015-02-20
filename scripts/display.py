#!/usr/bin/env python

import os, sys
from optparse import OptionParser
from os import system

usage = "Usage: %prog filename [options]"
parser = OptionParser(usage=usage)
parser.add_option("-t", "--title", dest="title", type="string", default="", help="Plot title")
parser.add_option("-d", "--outdir", dest="out_dirname", type="string", help="Output directory")
parser.add_option("-o", "--open", dest="open", action="store_true", help="Open plot")

# Parse arguments and options
(options, args) = parser.parse_args()
if len(args) != 1:
    parser.print_help()
    sys.exit(1)

file_dirname = os.path.dirname(args[0]) + '/'
file_basename = os.path.basename(args[0])
file_name, file_extension = os.path.splitext(file_basename)

if options.out_dirname is None:
    options.out_dirname = file_dirname

# Plot
cmd =  'gnuplot -e "'
cmd += '_datafile=\'' + file_dirname + file_basename + '\';'
cmd += '_plotfile=\'' + options.out_dirname + file_name + '.eps' + '\';'
cmd += '_title=\'' + options.title + '\';'
cmd += '" plot.gp'
system(cmd)

if options.open:
    cmd = 'gnome-open ' + options.out_dirname + file_name + '.eps'
    system(cmd)
