#!/bin/sh 

cp src/ndnSIM/examples/ndn-grid-v2v-simple.cpp scratch/ndn-grid-v2v-simple.cc

i=1
lim=120
step=$(($lim/4))

while [ $i -le $lim ];
do
echo $i
#run sim
if [ $i -le $step ];
then
./waf --run "ndn-grid-v2v-simple --traceFile=/home/jfpereira/opt/ndnSIM-VAL/ns-3/src/ndnSIM/examples/mobility-tracers/oneway-2_100_200s.tcl --nodeNum=100 --nTries=$i"    
fi
if [ $i -ge $(($step+1)) ] && [ $i -le $(($step*2)) ];
then
./waf --run "ndn-grid-v2v-simple --traceFile=/home/jfpereira/opt/ndnSIM-VAL/ns-3/src/ndnSIM/examples/mobility-tracers/oneway-2_70_200s.tcl --nodeNum=70 --nTries=$i"
fi
if [ $i -ge $(($step*2+1)) ] && [ $i -le $(($step*3)) ];
then
./waf --run "ndn-grid-v2v-simple --traceFile=/home/jfpereira/opt/ndnSIM-VAL/ns-3/src/ndnSIM/examples/mobility-tracers/oneway-2_50_200s.tcl --nodeNum=50 --nTries=$i"
fi
if [ $i -ge $(($step*3+1)) ] && [ $i -le $lim ];
then
./waf --run "ndn-grid-v2v-simple --traceFile=/home/jfpereira/opt/ndnSIM-VAL/ns-3/src/ndnSIM/examples/mobility-tracers/oneway-2_30_200s.tcl --nodeNum=30 --nTries=$i"
fi

# makes the cat of val data
cat val_pkts_*.txt > s${i}_val_pkts.txt
# moves to R
mv app-delays* rate-trace* s${i}_val_pkts.txt R
# rm val_pkts_*.txt
rm val_pkts_*.txt

i=$(($i+1))
done
# move to R
cd R
# run tests
sh bulk_tester.sh

echo "Finished"
