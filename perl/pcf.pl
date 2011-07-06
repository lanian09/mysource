#!/usr/bin/perl

use warnings;
use strict;

my $key;
my (%hash, %skiphash);
my %pcfhash;
my $i = 1;
my ($file, $line);
my ($brip, $branchid, $sysid, $bscid, $btsid, $sec, $fa, $pcfip, $pcfbip, $pcftype, $data);
my $out = "./PCFINFO.txt";
my $skipout = "./SKIP_PCFINFO.txt";
my @files = glob("/TAMAPP/LOG/O_SVCMON/DEBUG/debuglog.*");

$pcfhash{"10.160.40"} = 3;
$pcfhash{"10.160.41"} = 1;
$pcfhash{"10.160.42"} = 5;
$pcfhash{"10.160.43"} = 4;
$pcfhash{"10.160.44"} = 6;
$pcfhash{"10.160.45"} = 10;
$pcfhash{"10.160.46"} = 8;
$pcfhash{"10.160.47"} = 7;
$pcfhash{"10.160.48"} = 9;

$pcfhash{"10.160.141"} = 2;
$pcfhash{"10.160.143"} = 2;
$pcfhash{"10.160.170"} = 2;
$pcfhash{"10.160.171"} = 2;
$pcfhash{"10.160.174"} = 2;
$pcfhash{"10.160.175"} = 2;
$pcfhash{"10.160.176"} = 2;
$pcfhash{"10.210.1"} = 2;
$pcfhash{"10.210.26"} = 2;

$pcfhash{"10.160.147"} = 5;
$pcfhash{"10.212.26"} = 5;
$pcfhash{"10.212.27"} = 5;

$pcfhash{"10.160.149"} = 4;

$pcfhash{"10.160.153"} = 10;
$pcfhash{"10.160.190"} = 10;
$pcfhash{"10.160.191"} = 10;
$pcfhash{"10.160.222"} = 10;

$pcfhash{"10.160.155"} = 8;
$pcfhash{"10.160.194"} = 8;
$pcfhash{"10.160.195"} = 8;

$pcfhash{"10.160.157"} = 7;
$pcfhash{"10.160.198"} = 7;
$pcfhash{"10.160.199"} = 7;

$pcfhash{"10.160.159"} = 9;
$pcfhash{"10.160.202"} = 9;
$pcfhash{"10.160.203"} = 9;
$pcfhash{"10.160.220"} = 9;

$pcfhash{"10.160.27"} = 3;
$pcfhash{"10.160.35"} = 3;

foreach $file (sort @files) {
    open LOG, "$file" || die $!;

    while($line = <LOG>) {
        chop($line);

        if($line =~ /dProcMON (\S+) (\S+) NULL dRet=(\d+) MSGTYPE=(\S+)\:(\d+) OFFICE=(\S+)\:(\d+) SYSID=(\d+) BSCID=(\d+) BTSID=(\d+) SEC=(\d+) FA=(\d+) SVCL4=(\S+)\:(\d+) SVCTYPE=(\S+)\:(\d+) PCFTYPE=(\S+)\:(\d+) PCF=(\d+)\.(\d+)\.(\d+)\.(\d+)\:(\d+)/)
        {
            $sysid = "$8";
            $bscid = "$9";
            $btsid = "$10";
            $sec = "$11";
            $fa = "$12";
            $pcfip = "$19.$20.$21.$22";
            $pcfbip = "$23";
			$brip = "$19.$20.$21";
#            $data = "bscid=$bscid\tbtsid=$btsid\tsec=$sec\tfa=$fa\tpcfip=$pcfip";
#           if($bscid == 0 && $btsid == 0) {
#               $pcftype = 1;
#           } else {
#               $pcftype = 4;
#           }
            $pcftype = 4;
            if(!defined $pcfhash{$brip}) {
                $branchid = 0;
            } else {
                $branchid = $pcfhash{$brip};
            }
			if($branchid > 0) {
				$data = "insert ignore into TB_MACCESSC values($branchid, \'$pcfip\', $pcfbip, $pcftype, $bscid, $btsid, $sysid, \'\', \'\', \'\', \'\', \'\', \'\', 0, \'\');";
print "$data\n";
            	$hash{$data}++;
			} else {
				$data = "insert ignore into TB_MACCESSC values($branchid, \'$pcfip\', $pcfbip, $pcftype, $bscid, $btsid, $sysid, \'\', \'\', \'\', \'\', \'\', \'\', 0, \'\');";
print "$data\n";
            	$skiphash{$data}++;

			}
        }
    }
}

open OUT, ">$out" || die $!;
foreach $key (keys %hash) {
    print OUT "$key\n";
}
print OUT "commit;\n";

open SKIPOUT, ">$skipout" || die $!;
foreach $key (keys %skiphash) {
    print SKIPOUT "$key\n";
}
