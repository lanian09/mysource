#!/bin/usr/perl
my %mhash;
$mhash{"123"}=1;
$mhash{"124"}=1;
foreach $key (keys %mhash){
	print "$key\n";
	print "$mhash{$key}\n";
}

print "===\n";
