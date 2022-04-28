set title "Fib 0 to 200"
set xlabel "n th sequence of Fibonacci number"
set ylabel "time cost (ns)"
set terminal png font " Times_New_Roman,12 "
set output "plot.png"
set xrange [0:200]
set yrange [0:10000]
set xtics (0,10,20,30,40,50,60,70,80,90,100,120,140,160,180,200)
set grid
set key center top box 1

plot \
"adding.txt" using 1:2 with points title "Adding", \
"lm.txt" using 1:2 with points title "Long Multiplication", \
"ss.txt" using 1:2 with points title "Schonhange-Strassen", \
"karatsuba.txt" using 1:2 with points title "Karatsuba", \