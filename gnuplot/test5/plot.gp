set title "Fib 0 to 50000"
set xlabel "n th sequence of Fibonacci number"
set ylabel "time cost (ns)"
set terminal png font " Times_New_Roman,12 "
set output "plot.png"
set xrange [0:50000]
set xtics 0, 10000, 50000
set key left

plot \
"adding.txt" using 1:2 with points title "Adding", \
"lm.txt" using 1:2 with points title "Long Multiplication", \
"ss.txt" using 1:2 with points title "Schonhange-Strassen", \