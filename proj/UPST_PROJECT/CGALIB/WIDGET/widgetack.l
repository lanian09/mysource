%{
    #include <stdio.h>
    #include <stdlib.h>

	#include "common_stg.h"
    #include "widget.h"
		
    int             *pAck;
    int             len;

%}

%%

ack=y						{
                                *pAck = 1;
								return 0;
                            }

objectdownload				{
								*pAck = 2;
								return 0;
							}

installnoti					{
								*pAck = 3;
								return 0;
							}


(\r\n)						{ }

[ \t\n]						{ }

.							{ }

%%

int widgetackwrap(void)
{
    return 1;
}

int widgetack(char *sp, int slen, int *dAck)
{
    pAck = dAck;

    widgetack_scan_bytes(sp, slen);
    widgetacklex();

    widgetack_delete_buffer(yy_current_buffer);
    yy_init = 1;

    return 0;
}

#ifdef  TEST
int main(int argc, char *argv[])
{
    int   	dAck = 0;

    char    *sp;
    int     slen;

    pAck = &dAck;

    if (argc != 2) {
        printf("INVALID ARGUMENT\n");
        printf("EX) widgetack ACK\n");
        return(1);
    }

    sp = strdup(argv[1]);
    slen = strlen(sp);

    widgetack_scan_bytes(sp, slen);
    widgetacklex();
	
	printf("ACK=[%d]\n", dAck);

    return 0;
}
#endif
