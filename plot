set terminal png
set output 'plot.png'
plot "plot.dat" using 1:3 title "Average Fitness", "plot.dat" using 1:2 title "Best Fitness"
