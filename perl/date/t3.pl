#!/bin/usr/perl

# added by uamyd 20101111, for insert into @total-omp database
## date calculate
my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);
$year += 1900;
my $cdate = "$year$mon$mday";

## script file
my $shfn = "./update_access_$cdate.sh";

my ( $thost, $id, $pwd, $dbn, $statement, $query );
$thost = "localhost";
$id    = "dqms";
$pwd   = "dqms123";
$dbn   = "dqmsDB";

$query = "mysql -h $thost -u $id -p$pwd $dbn < $out";
open RSH,"> $shfn";
print RSH $query;

system("chmod +755 $shfn");
