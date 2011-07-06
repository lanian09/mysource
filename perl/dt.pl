#!/usr/bin/perl
use warnings;
use strict;
use Time::localtime;
use POSIX 'mktime';

my $tm = localtime;
my $DIR = sprintf("%04d%02d%02d", ($tm->year)+1900, ($tm->mon)+1, $tm->mday);

my $year = substr($DIR, 0, 4);
my $mon = substr($DIR, 4, 2);
my $day = substr($DIR, 6, 2);

print "$year-$mon-$day\n";

$tm = localtime(mktime( 0, 0, 0, $day, $mon-1,$year-1900 ) + 86400);
my $nextDir = sprintf("%04d%02d%02d", ($tm->year)+1900, ($tm->mon)+1, $tm->mday);

print $nextDir,"\n";

my $nextDir = "$year$mon$day";
print $nextDir,"\n";
