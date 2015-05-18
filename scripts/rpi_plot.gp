#!/usr/bin/gnuplot

reset
unset key
set datafile separator ','

splot _datafile using ($2/1000000):($1+7200):3 with image
set output _plotfile
#set terminal postscript eps enhanced color font 'Verdana,9'
set terminal pngcairo size 800,600 enhanced font 'Verdana,9'
set view map

set title _title
set grid

set xlabel 'Frequency [MHz]'
set xrange [_minFREQ to _maxFREQ]
set xtics _minFREQ,_freqSTEP,_maxFREQ

set ylabel 'Time [hh:mm:ss]' offset -2,0

set ydata time
set timefmt "%s"
set format y "%H:%M:%S"

set cblabel 'dB'
set cbrange [_minDB to _maxDB]

replot
