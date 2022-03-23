# 1.gp
set title "Adding Time"
set xlabel "n th sequence of Fibonacci number"
set ylabel "time cost (ns)"
set terminal png font " Times_New_Roman,12 "
set output "time2.png"
set xrange [0:1000]
set yrange [0:]
set xtics 0, 100, 1000
set key left

plot \
"adding.txt" using 1:2 with points title "kernel", \
"adding.txt" using 1:3 with points title "user", \
"adding.txt" using 1:4 with points title "kernel to user", \