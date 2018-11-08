HOMEWORK:

Run "sudo ./script.sh". It'll install some packages (if not already installed), 
compile and execute cache.c. "cache.c" takes the cache size (in KB) as a parameter 
and randomly accesses different cache-lines in a memory area of input cache size. 
The "script.sh" uses "perf" tool to collect the statistics about the number of 
"L1-cache" misses during load. The "script.sh" executes the given program for 
different cache sizes and generates a table of cache-size, number of random loads, 
and number of L1 misses.

Turn in:
1. Write a small summary of what "cache.c" is trying to achieve. 
2. Explain the numbers in stats.txt. 

Submit a pdf file of your writeup to the backpack.
