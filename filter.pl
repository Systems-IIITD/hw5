#!/bin/perl/

use FileHandle;
use POSIX;
no warnings 'portable';
use strict;
use warnings;


open (my $fp, "<tmp.txt") or die $!;

my $line;
my $sz;
my $num_random_loads;
my $misses = 0;

printf ("%-10s %-20s %-20s\n", "SIZE(KB)", "RANDOM_LOADS", "L1-MISSES");
print "---------------------------------------------\n";

while ($line = <$fp>) {
	if ($line =~ m/SZ:(\d+).*num-random-loads:(\d+)/) {
		$sz = $1;
		$num_random_loads = $2;
	}
	elsif ($line =~ m/\s+(\d+.*\s).*misses/) {
		$misses = $1;
		printf ("%-10s %-20s %-20s\n", $sz, $num_random_loads, $misses);
	}
}

print "---------------------------------------------\n";
