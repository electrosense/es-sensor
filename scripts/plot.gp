#!/usr/bin/gnuplot

reset
unset key
set datafile separator ','

splot _datafile using ($2/1000000):($1+3600):3 with image
set output _plotfile
set terminal postscript eps enhanced color font 'Verdana,16'
set view map

set title _title
#set grid

set xlabel 'Frequency [MHz]'
#set xrange [_minFREQ to _maxFREQ]
set xrange [floor(GPVAL_DATA_X_MIN/100)*100:ceil(GPVAL_DATA_X_MAX/100)*100]
#set xtics _minFREQ,_freqSTEP,_maxFREQ
#set xtics GPVAL_DATA_X_MIN,4,GPVAL_DATA_X_MAX
set xtics 200
set mxtics 2

set ylabel 'Time [hh:mm]' offset -2,0

set ydata time
set timefmt "%.6s"
set format y "%H:%M"

set cblabel 'dB' offset +1,0
#set cbrange [floor(GPVAL_DATA_Z_MIN/10)*10 to 0]
set cbrange [floor(GPVAL_DATA_Z_MIN/5)*5 to ceil(GPVAL_DATA_Z_MAX/5)*5]
#set cbrange [_minDB to _maxDB]
#set cbrange [GPVAL_DATA_Z_MIN to GPVAL_DATA_Z_MAX]
#set cbtics GPVAL_DATA_Z_MIN,10,GPVAL_DATA_Z_MAX
#set cbtics 10

replot
