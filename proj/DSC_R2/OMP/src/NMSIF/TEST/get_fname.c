#include <stdio.h>


typedef struct {
	int		num;
	char	name[24];
} FLIST;

//char	flist[5][23];

char	iname[20];
FLIST	flist[5];

main ()
{
	int		i;

	strcpy (iname, "10BSM01-M-200608");

	for (i=0; i<5; i++) {
		flist[i].num = i+1;
		sprintf (flist[i].name, "%s%02d", iname, i+1);
	}

	for (i=0; i<5; i++)
		printf ("[%d] %s\n", flist[i].num, flist[i].name);

	puts ("");
}
