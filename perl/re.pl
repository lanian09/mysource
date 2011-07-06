#!bin/usr/perl
#use warnings;
#use strict;


sub test
{
	my $file = shift;
	print $file,"\n";
	if ($file =~ /[0-9a-zA-Z\_]\_F(\w+)\_ID[0-9a-zA-Z\_\.]+/){
		print "ok 0:",$0," 1:",$1," 2:",$2," 3:",$3,"\n";
	} else {
		print "out\n";
	}
}

sub test1
{
	my $file = shift;
	if ($file =~ /([0-9a-zA-Z\_]+)\/([0-9a-zA-Z\_]+)\.([0-9a-zA-Z\_]+)/)
	{
		my $filename = "$2.$3";
		print "filename=",$filename,"\n";
		test($filename);
	}
}

my $ff ="/OTAS/SI_LOG/20100712/S1_FWIF_ID0232_T20100712191500.FIN";
test1($ff);
