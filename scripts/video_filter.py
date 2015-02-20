#!/usr/bin/env python

import os, sys, csv
import numpy as np
from optparse import OptionParser

# Usage
usage = "Usage: %prog filename [options]"
parser = OptionParser(usage=usage)
parser.add_option("-t", "--time", dest="time_range", type="string",
                  help="Time range 'minT:step:maxT' in secs since UNIX epoch (microsecs precision)")
parser.add_option("-f", "--frequency", dest="freq_range", type="string",
                  help="Frequency range 'minF:step:maxF' in Hz")
parser.add_option("-g", "--gain", dest="gain_range", type="string",
                  help="Gain range 'minG:step:maxG' in dB")
parser.add_option("-i", "--interpolation", dest="interpolation", action="store_true",
                  help="Perform interpolation")

# Parse arguments
(options, args) = parser.parse_args()
if len(args) != 1:
  parser.print_help()
  sys.exit(1)
file_name, file_extension = os.path.splitext(args[0])

# Determine data ranges
t_full_range = [None, None]
f_full_range = [None, None]
g_full_range = [None, None]

sys.stdout.write("Determining data ranges...\n")

## Data ranges have already been calculated
if os.path.isfile(file_name + "_dr" + file_extension):
  with open(file_name + "_dr" + file_extension, "rb") as f:
    reader = csv.reader(f)

    line = reader.next()
    line_cnt = int(line[0])
    
    line = reader.next()
    t_full_range[0] = float(line[0])
    t_full_range[1] = float(line[1])

    line = reader.next()
    f_full_range[0] = int(line[0])
    f_full_range[1] = int(line[1])

    line = reader.next()
    g_full_range[0] = float(line[0])
    g_full_range[1] = float(line[1])
    
## Calculate data ranges
else:
  line_cnt = 0
  with open(file_name + file_extension, "rb") as f:
    reader = csv.reader(f)
    for row in reader:
      try:
        time = float(row[0]) + float(row[1])/1e6
        freq = int(row[2])
        gain = float(row[3])
      except:
        continue
        
      line_cnt += 1
      if(t_full_range[0] is None or t_full_range[0] > time): t_full_range[0] = time
      if(t_full_range[1] is None or t_full_range[1] < time): t_full_range[1] = time
      if(f_full_range[0] is None or f_full_range[0] > freq): f_full_range[0] = freq
      if(f_full_range[1] is None or f_full_range[1] < freq): f_full_range[1] = freq
      if(g_full_range[0] is None or g_full_range[0] > gain): g_full_range[0] = gain
      if(g_full_range[1] is None or g_full_range[1] < gain): g_full_range[1] = gain

      sys.stdout.write("\r%d" % line_cnt)
      sys.stdout.flush()

    sys.stdout.write("\n")

## Store data ranges
with open(file_name + "_dr" + file_extension, "wb") as f:
  writer = csv.writer(f)
  writer.writerow([int(line_cnt)])
  writer.writerow([float(t_full_range[0]), float(t_full_range[1])])
  writer.writerow([int(f_full_range[0]), int(f_full_range[1])])
  writer.writerow([float(g_full_range[0]), float(g_full_range[1])])

# Parse options
if(options.time_range is None): options.time_range = "::"
time_range = options.time_range.split(":")
try: t_min = float(time_range[0])
except: t_min = t_full_range[0]
try: t_stp = float(time_range[1])
except: t_stp = max(1,(t_max-t_min) / 1000)
try: t_max = float(time_range[2])
except: t_max = t_full_range[1]

if(options.freq_range is None): options.freq_range = "::"
freq_range = options.freq_range.split(":")
try: f_min = int(freq_range[0])
except: f_min = f_full_range[0]
try: f_stp = int(freq_range[1])
except: f_stp = max(1,(f_max-f_min) / 1000)
try: f_max = int(freq_range[2])
except: f_max = f_full_range[1]

if(options.gain_range is None): options.gain_range = "::"
gain_range = options.gain_range.split(":")
try: g_min = float(gain_range[0])
except: g_min = g_full_range[0]
try: g_stp = float(gain_range[1])
except: g_stp = max(1,(g_max-g_min) / 100)
try: g_max = float(gain_range[2])
except: g_max = g_full_range[1]

sys.stdout.write("Preparing spectrum grid...\n")

# Determine bins
t_bin = np.linspace(t_min, t_max+t_stp, (t_max+t_stp-t_min)/t_stp+1)
f_bin = np.linspace(f_min, f_max+f_stp, (f_max+f_stp-f_min)/f_stp+1)
g_bin = np.linspace(g_min, g_max+g_stp, (g_max+g_stp-g_min)/g_stp+1)

# Initialize spectrum grid
init_cnt = 0
init_gain = 0
sgrid = [[[init_cnt, init_gain] for f in xrange(len(f_bin)-1)] for t in xrange(len(t_bin)-1)]

# Calculate spectrum grid
sys.stdout.write("Calculating spectrum grid...\n")
line = 0
line_cnt = float(line_cnt)
t_bin_len = len(t_bin) - 1
f_bin_len = len(f_bin) - 1
g_bin_len = len(g_bin) - 1
with open(file_name + file_extension, "rb") as f:
  reader = csv.reader(f)
  for row in reader:
    try:
      time = float(row[0]) + float(row[1])/1e6
      freq = int(row[2])
      gain = float(row[3])
    except:
      continue

    line += 1
    t = np.digitize([time], t_bin)[0] - 1
    if t < 0 or t >= t_bin_len: continue

    f = np.digitize([freq], f_bin)[0] - 1
    if f < 0 or f >= f_bin_len: continue

    g = np.digitize([gain], g_bin)[0] - 1
    if g < 0:
      g = g_min
    elif g >= g_bin_len:
      g = g_max
    else:
      g = g_bin[g]
            
    sgrid[t][f][0] += 1
    sgrid[t][f][1] += g
    sys.stdout.write("\r%3.1f%%" % (line/line_cnt*100))
    sys.stdout.flush()
    
  sys.stdout.write("\n")

for t in xrange(t_bin_len):
  for f in xrange(f_bin_len):
    if sgrid[t][f][0] > 0:
      g = sgrid[t][f][1] / sgrid[t][f][0]
      if g <= g_min:
        if f > 0: sgrid[t][f][1] = sgrid[t][f-1][1]
        else: sgrid[t][f][1] = g_min
      elif g > g_max:
        sgrid[t][f][1] = g_max
      else: sgrid[t][f][1] = g
    else:
      sgrid[t][f][1] = g_min
sgrid[0][0][1] = g_max
sgrid[1][0][1] = g_min

# Write spectrum grid
sys.stdout.write("Storing spectrum grid...\n")
out_file = file_name + "_t:%.6f:%.6f:%.6f_f:%d:%d:%d_g:%.1f:%.1f:%.1f_vf" % (t_min, t_stp, t_max, f_min, f_stp, f_max, g_min, g_stp, g_max) + file_extension
with open(out_file, "wb") as f:
  writer = csv.writer(f)
  for t in xrange(len(t_bin)-1):
    for f in xrange(len(f_bin)-1):
      writer.writerow([float(t_bin[t]), int(f_bin[f]), round(sgrid[t][f][1], 1)])
    writer.writerow([])

# Interpolate spectrum grid
if options.interpolation:
  sys.stdout.write("Interpolating spectrum grid...\n")
  for t in xrange(len(t_bin)-1):
    for f in xrange(len(f_bin)-1):
      if sgrid[t][f][0] <= 0 and t > 0:
        sgrid[t][f][1] = sgrid[t-1][f][1]

  # Write spectrum grid
  sys.stdout.write("Storing spectrum grid...\n")
  out_file = file_name + "_t:%.6f:%.6f:%.6f_f:%d:%d:%d_g:%.1f:%.1f:%.1f_ivf" % (t_min, t_stp, t_max, f_min, f_stp, f_max, g_min, g_stp, g_max) + file_extension
  with open(out_file, "wb") as f:
    writer = csv.writer(f)
    for t in xrange(len(t_bin)-1):
      for f in xrange(len(f_bin)-1):
        writer.writerow([float(t_bin[t]), int(f_bin[f]), round(sgrid[t][f][1], 1)])
      writer.writerow([])
