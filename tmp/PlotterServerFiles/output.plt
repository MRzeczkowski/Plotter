set xlabel 'x'
set ylabel 'y'
set grid
set style data lines
unset key
set terminal png enhanced font 'Verdana, 20'
set output "tmp/PlotterServerFiles/output.png"
plot "tmp/PlotterServerFiles/receive.dat" lw 3 linecolor rgb "red"
quit
