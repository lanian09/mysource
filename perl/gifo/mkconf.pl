#!/bin/usr/perl

use warnings;
use strict;

my (%prochash, %grphash, %wrhash, %rhash);
my $chno = 0;
my $grpno = 0;
my @dest_list;

my ($cellCnt, $cellSize, $wSemKey, $rSemKey);
$cellCnt = 5000;
$cellSize = 8;
$wSemKey = 20000;
$rSemKey = 10000;

# PROCESS+ KEY define
%prochash = ( 'CHSMD',0, 'COND',1, 'ALMD',2);
my $string = "CHSMD > ALMD COND";

if( $string =~ /(\w+)\s+>\s+(.+)/)
	{
		print "SRC  :: $1\n";
		print "DEST :: $2\n";
		#@dest_list = split/\s+/,$2;
		$_ = $2;
		@dest_list = split;
		
		my $src_proc = $1;	# 시작 프로세스
		$chno = $#dest_list; # dest process -> channel count이며, 동시에 쫑나지 않는 process에 대하여 group count
		my $no;

		#cifo print #################################################
		print "#### cifo setting ####\n";
		print "#chId/cellCnt/cellSize/wBufCnt/rBufCnt/wSemflag/wSemKey/rSemFlag/rSemKey/#\n";
		foreach $no ( 0..$#dest_list ){
			print "$no/$cellCnt/$cellSize/1/1/0/$wSemKey/0/$rSemKey\t\t# $src_proc > $dest_list[$no]\n";
			$no=$no+1;
			$wSemKey=$wSemKey+1;
			$rSemKey=$rSemKey+1;
		}

		#gifo print #################################################
		print "#### group setting ####
#grId/chId(, chId)/#
		foreach $no ( 0..$#dest_list ){
			print "$no/$cellCnt/$cellSize/1/1/0/$wSemKey/0/$rSemKey\t\t# $src_proc > $dest_list[$no]\n";
			$no=$no+1;
			$wSemKey=$wSemKey+1;
			$rSemKey=$rSemKey+1;
		}

	}
