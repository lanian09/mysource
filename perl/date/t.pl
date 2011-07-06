#!/bin/usr/perl
my ($id, $pwd, $dbn, $statement, $query);
$id = dqms;
$pwd = dqms123;
$dbn = dqmsDB;
$fn = "./tmp2";

open ST,"> $fn";
$statement = "insert into TBL_TEMP values ( 4 );";
print ST "$statement\n";
system ("cat $fn");

$query = "mysql -u $id -p$pwd $dbn < $fn";
system ($query);

$statement = "\"select count(1) from TBL_TEMP\"";
$query = "mysql -u $id -p$pwd $dbn -e $statement";
system ($query);

my $cdate = localtime(time);
print "$cdate\n";

my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);
my $cdate;
$year += 1900;
$cdate = "$year$mon$mday";
print "$cdate\n";

