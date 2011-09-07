#include <stdio.h>
#include <string.h>

typedef struct {
	int	 id;
	char *name;
} process_inf, *pprocess_inf;

#define SEQ_PROC_START 1

#define SEQ_PROC_CHSMD 1
#define SEQ_PROC_ALMD  2
#define SEQ_PROC_COND  3
#define SEQ_PROC_COND1  4
#define SEQ_PROC_COND2  5

#define SEQ_PROC_LAST 5

//process initialize
process_inf process[] = {
	{ SEQ_PROC_CHSMD, "CHSMD"},
	{ SEQ_PROC_ALMD,  "ALMD" },
	{ SEQ_PROC_COND,  "COND"},
	{ SEQ_PROC_COND1,  "COND1"},
	{ SEQ_PROC_COND2,  "COND2"},
};
	
int getProcID(char *pname){
	int i;
	printf("input process name = %s\n", pname);
	for( i = 0; i < SEQ_PROC_LAST; i++ ){
		if( !strncmp( process[i].name, pname, strlen(pname)) ){
			return process[i].id;
		}
	}
	return -1;
}
int main()
{
	printf(" CHSMD = %d\n", getProcID("CHSMD"));
	printf(" ALMD = %d\n", getProcID("ALMD"));
	printf(" COND = %d\n", getProcID("COND"));
	printf(" COND1 = %d\n", getProcID("COND1"));
	printf(" COND2 = %d\n", getProcID("COND2"));
	return 0;
}
