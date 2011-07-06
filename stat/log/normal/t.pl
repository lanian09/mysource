#!/usr/bin/perl

use warnings;
use strict;

my $STAT="./stat.log";
my $CNT=0;
my $myargs;
my $mypid;
my $myblock='';
open STG, "$STAT" || die $!;

my $sample=`bash cat stat.log | head -1 | awk '{print $1}'`;
$sample;
while (<STG>){
	$myargs=$_;
	print "$CNT----->$myargs";
	if( $myargs=~ /CHSMD/ ){
		$myblock="$1";
		print "$CNT      BLOCK=$myblock\n";
	}
	$CNT=($CNT)+1;
}
