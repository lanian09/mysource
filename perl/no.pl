#!/usr/bin/perl

use warnings;
print "Is two equal to four? ", 2==4, "\n";
print "OK, then, is six equal to six? ", 6==6, "\n";
print "So, two isn't equal to four? ", 2 != 4, "\n";
print "C:\\WINNT\\Profiles\\\n";
print 'C:\WINNT\Profiles\ ', "\n";
print 'ex\\ er\\' ,' ci\' se\'' ,"\n";
print 'ex\ er\ ' ,  ' ci\' se\'' ,"\n";

print qq|' "hi," said Jack. "Have you read/. today?"'\n|;
print qq#' "hi," said Jack. "Have you read/. today?"'\n#;
print qq(' "hi," said Jack. "Have you read/. today?"'\n);
print qq<' "hi," said Jack. "Have you read/. today?"'\n>;
print qq*' "hi," said Jack. "Have you read/. today?"'\n*;
print qq$' "hi," said Jack. "Have you read/. today?"'\n$;
print qq@' "hi," said Jack. "Have you read/. today?"'\n@;

my $tmp="";
print "tmp:",$tmp,"]\n";
$tmp = 2 == 4;
print "tmp:",$tmp,"]\n";
$tmp = 4 == 4;
print "tmp:",$tmp,"]\n";
print "tmp:","$tmp"x4,"]\n";

my $a="9"; print ++$a, "\n";
my $a="A9"; print ++$a, "\n";
my $a="bz"; print ++$a, "\n";
my $a="Zz"; print ++$a, "\n";
my $a="z9"; print ++$a, "\n";
my $a="9z"; print ++$a, "\n";

