#!/bin/sh 

i=1
lim=120
step=$(($lim/4))

while [ $i -le $lim ];
do
#run sim
if [ $i -le $step ];
then
Rscript eval4.R app-delays-trace_200s_V100_${i}.txt rate-trace_200s_V100_${i}.txt s${i}_val_pkts.txt $i 100
fi
if [ $i -ge $(($step+1)) ] && [ $i -le $(($step*2)) ];
then
Rscript eval4.R app-delays-trace_200s_V70_${i}.txt rate-trace_200s_V70_${i}.txt s${i}_val_pkts.txt $i 70
fi
if [ $i -ge $(($step*2+1)) ] && [ $i -le $(($step*3)) ];
then
Rscript eval4.R app-delays-trace_200s_V50_${i}.txt rate-trace_200s_V50_${i}.txt s${i}_val_pkts.txt $i 50
fi
if [ $i -ge $(($step*3+1)) ] && [ $i -le $lim ];
then
Rscript eval4.R app-delays-trace_200s_V30_${i}.txt rate-trace_200s_V30_${i}.txt s${i}_val_pkts.txt $i 30
fi


i=$(($i+1))
done

# makes the cat total sim csv
cat header_results.txt results-sim-*.txt > final_result.csv
# rm 
rm results-sim-*.txt
# make graphs
Rscript evalGraph.R final_result.csv 0.95
