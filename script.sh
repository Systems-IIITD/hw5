#!/bin/bash

make > /dev/null

install=$(dpkg-query -W -f='${Status}' linux-tools-common 2>/dev/null | grep -c "ok installed")
if [ "$install" == "0" ]; then
	apt-get install linux-tools-common
fi

install=$(dpkg-query -W -f='${Status}' linux-tools-`uname -r` 2>/dev/null | grep -c "ok installed")

if [ "$install" == "0" ]; then
	apt-get install linux-tools-`uname -r`
fi

echo "" > stats.txt

for (( i=4; i<=100; i+=4 ))
do
	perf stat -e L1-dcache-load-misses,L1-dcache-loads  ./cache $i >> tmp.txt 2>&1
done
perl filter.pl > stats.txt
rm -rf tmp.txt
