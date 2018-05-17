set terminal png font "arial,12"

set style data linespoints
set autoscale
set key above

set xlabel "Database Size (Nr of pairs w/id)"
set ylabel "Time (s)"

set decimal locale
set format x "%'0.f"

set xtics rotate by 45 right
#set key spacing 0.8
#set logscale y

set grid

set output outputfile
plot inputfile using 1:2 title columnheader(2) ps 1.5 lw 2, for [i=3:cols] '' using 1:i title columnheader(i) ps 1.5 lw 2
