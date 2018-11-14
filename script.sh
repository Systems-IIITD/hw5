#!/bin/bash

make > /dev/null

pacman -Qi perf 2>/dev/null
if [ $? -ne 0 ]
then
	pacman -S perf
fi

echo "" > stats.txt

for (( i=4; i<=100; i+=4 ))
do
	perf stat -e L1-dcache-load-misses,L1-dcache-loads  ./cache $i >> tmp.txt 2>&1
done
perl filter.pl > stats.txt
rm -rf tmp.txt
