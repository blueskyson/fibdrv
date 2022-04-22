set title "Kernel Module vs Userspace App"
set xlabel "n th sequence of Fibonacci number"
set ylabel "time cost (ns)"
set terminal png font " Times_New_Roman,12 "
set output "time4.png"
set xrange [0:1000]
set xtics 0, 100, 1000
set key left

plot \
"kernel.txt" using 1:3 with points title "kernel module", \
"user.txt" using 1:3 with points title "userspace app", \