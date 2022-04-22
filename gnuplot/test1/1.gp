# 1.gp
set title "Fast Doubling Time"
set xlabel "n th sequence of Fibonacci number"
set ylabel "time cost (ns)"
set terminal png font " Times_New_Roman,12 "
set output "time.png"
set xrange [0:1000]
set xtics 0, 100, 1000
set key left

plot \
"fast_doubling.txt" using 1:2 with points title "kernel", \
"fast_doubling.txt" using 1:3 with points title "user", \
"fast_doubling.txt" using 1:4 with points title "kernel to user", \