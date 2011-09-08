#!/usr/bin/perl

use warnings;
use strict;

## files #################################################
my $proc = "./process_list.conf";
my $input = "./flow_list.conf";
my $gifo = "./gifo.conf";
my $cifo = "./cifo.conf";
my $nifo = "./nifo_zone.conf";

## default values ########################################
my $defaultCellCnt = 1000000;
my $defaultWSemFlag = 0;
my $defaultRSemFlag = 0;
my $cellSize = 8;
my $wBufCnt = 1;
my $rBufCnt = 1;
my $wSemKey = 20000;
my $rSemKey = 10000;
my $zoneSemKey = 16000;

## fixed default values ##################################
my $uiShmKey = 10314;
my $uiHeadRoomSize = 5242880;
my $uiType = 2;
my $uiNIFOShmKey = 25022;
my $uiNIFOHeadRoomSize = 0;

my $line;
my ($temp_write, $read, @write, $write_cnt);
my $chid = 0;
my $grid = 0;
my $cellCnt;
my $wSemFlag;
my $rSemFlag;
my (%write_matrix, %read_matrix);
my ($key, $value);
my %process;

my (%zone_matrix, %zone_list, %zone_process);
my $zoneSemFlag = 0;
my $zoneid = 0;
my $zoneProcCnt = 0;

open PROC, "$proc" || die $!;
while($line = <PROC>)
{
	chop($line);
	if($line =~ /(\S+)(\s*)=(\s*)(\d+)/)
	{
		if( $1 =~ /(END_OF_ZONE_AREA)/)
		{
			zone_counting();		
			next;
		} 
		print "PROCESS $1\t$4\n";
		$process{"$1"}="$4";
		$zone_process{"$4/$zoneid/\t # $1"} = $zoneid * 1000 + $zoneProcCnt;
		#print NIFO "$4/0/	# $1\n";

		$zoneProcCnt++;

	}
}

zone_counting();

sub zone_counting
{
	if( $zoneProcCnt > 1 )
	{
		$zoneSemFlag = 1;
	}

	$zone_matrix{"$zoneid/$zoneSemFlag/$zoneSemKey/8192/30000/"} = $zoneid;
	$zoneid++;
	$zoneSemKey++;

	$zoneSemFlag = 0;
	$zoneProcCnt = 0;

	print " => zone area increased...\n";
}

## NIFO HEADER ##
open NIFO, ">$nifo" || die $!;
print NIFO "#### st_MEMSCONF setting (NIFO) ####\n";
print NIFO "uiType			= $uiType\n";
print NIFO "uiShmKey		= $uiNIFOShmKey\n";
print NIFO "uiHeadRoomSize	= $uiNIFOHeadRoomSize\n";
print NIFO "\n";
print NIFO "#### st_MEMSZONECONF setting (ZONE) ####\n";
print NIFO "#zoneId/SemFlag/SemKey/memNodeBodySize/memNodeTotCnt/#\n";
#print NIFO "0/0/16000/8192/30000/       # this is test1 #\n";
#foreach $key (keys %zone_matrix)
foreach $_ (sort{ $zone_matrix{$a} <=> $zone_matrix{$b} } keys %zone_matrix)
{
	#FOR DEBUG print NIFO "$_ || $zone_matrix{$_}\n";
	print NIFO "$_\n";
}
print NIFO "END_ZONE#\n";
print NIFO "\n";
print NIFO "#### ZONE MATRIX SETTING ####\n";
print NIFO "#procSeq/zoneID/#\n";
foreach $_ (sort{ $zone_process{$a} <=> $zone_process{$b}} keys %zone_process)
{
	#FOR DEBUG print NIFO "$_ || $zone_process{$_}\n";
	print NIFO "$_\n";
}

## GIFO HEADER ##
open GIFO, ">$gifo" || die $!;
print GIFO "#### group setting ####\n";
print GIFO "#grId/chId(, chId)/#\n";

## CIFO HEADER ##
open CIFO, ">$cifo" || die $!;
print CIFO "#### st_CIFOCONF setting ####\n";
print CIFO "uiShmKey		= $uiShmKey         # S_SSHM_CIFO_MEM 0x284A #\n";
print CIFO "uiHeadRoomSize	= $uiHeadRoomSize   # gifo size #\n";
print CIFO "\n";
print CIFO "#### st_CHANCONF setting ####\n";
print CIFO "#chId/cellCnt/cellSize/wBufCnt/rBufCnt/wSemflag/wSemKey/rSemFlag/rSemKey/#\n";


open INPUT, "$input" || die $!;
while($line = <INPUT>)
{
	chop($line);
	if($line =~ /([0-9a-zA-Z=_(), \t]+)(\s*)->(\s*)(\S+)/)
	{
		$temp_write = $1;
		$read = $4;
		print "t1=[$temp_write]\tt2=[$read]\n";
		@write = split(/,/, $temp_write);
		$write_cnt = @write;
		$rBufCnt = $write_cnt;
		print "write_cnt=$write_cnt\n";
		print GIFO "$grid/";

		for(my $i = 0; $i < $write_cnt; $i++)
		{
			#$cellCnt = 1000000;
			#$wSemFlag = 0;
			#$rSemFlag = 0;
			$cellCnt = $defaultCellCnt;
			$wSemFlag = $defaultWSemFlag;
			$rSemFlag = $defaultRSemFlag;
			print "==> TTT=$write[$i]\n";
			if($write[$i] =~ /cellCnt(\s*)=(\s*)(\d+)/)
			{
				$cellCnt=$3;
				print "==@ cellCnt=$cellCnt\n";
			}
			if($write[$i] =~ /wSemFlag(\s*)=(\s*)(\d+)/)
			{
				$wSemFlag=$3;
				print "==@ wSemFlag=$wSemFlag\n";
			}
			if($write[$i] =~ /rSemFlag(\s*)=(\s*)(\d+)/)
			{
				$rSemFlag=$3;
				print "==@ rSemFlag=$rSemFlag\n";
			}
			if($write[$i] =~ /([0-9a-zA-Z_]+)/)
			{
				$write[$i] = $1;
			}
			print "==> TTK=$write[$i]\n";

			print CIFO "$chid/$cellCnt/$cellSize/$wBufCnt/$rBufCnt/$wSemFlag/$wSemKey/$rSemFlag/$rSemKey/\t# $write[$i] - $read #\n";
			if($i == $write_cnt - 1)
			{
				print GIFO "$chid/\t# $read #\n";
			}
			else
			{
				print GIFO "$chid,";
			}

			if(!exists($process{$write[$i]}))
			{
				print "!!!!!!! INVALID PROCESS NAME=$write[$i]\n";
				exit;
			}
			if(!exists($process{$read}))
			{
				print "!!!!!!! INVALID PROCESS NAME=$read\n";
				exit;
			}

			$write_matrix{"$process{$write[$i]}/$process{$read}/$chid/\t# $write[$i]/$read/ #"}++;
			$read_matrix{"$process{$read}/$grid/\t# $read #"}++;
			$chid++;
			$wSemKey++;
			$rSemKey++;
		}
		$grid++;
	}
}

print GIFO "END_GROUP#\n";

print GIFO "#### write matrix setting ####\n";
print GIFO "#wProcSeq/rProcSeq/chId/#\n";
foreach $key (keys %write_matrix)
{
	print GIFO "$key\n";
}
print GIFO "END_WRITE_MATRIX#\n";

print GIFO "#### read matrix setting ####\n";
print GIFO "#procSeq/grId/#\n";
foreach $key (keys %read_matrix)
{
	print GIFO "$key\n";
}
print GIFO "END_READ_MATRIX#\n";

