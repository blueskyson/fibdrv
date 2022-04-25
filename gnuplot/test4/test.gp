set title "Test"
set xlabel "n th sequence of Fibonacci number"
set ylabel "time cost (ns)"
set terminal png font " Times_New_Roman,12 "
set output "time4.png"
set xrange [0:1000]
set xtics 0, 100, 1000
set key left

plot \
"kernel2.txt" using 1:2 with points title "fast doubling", \