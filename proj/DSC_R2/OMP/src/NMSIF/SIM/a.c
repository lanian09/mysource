#include <stdio.h>

char    conbuf[1024] = 
"    BSDM-BSDM 2006-07-31 20:00:00.13\n *** A1015 SUCC RATE ALARM OCCURED\n SYSTYPE  = BSDM\n RSC      = WAP1\n INFO     = UNDER THRESHOLD [80]\n COMPLETED";


main ()
{
	printf ("[*] buf=\n(%s)\n", conbuf);
	puts ("");
}
