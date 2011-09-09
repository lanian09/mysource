%{
    #include <stdio.h>
    #include <stdlib.h>

	#include "common_stg.h"
    #include "widget.h"
	
    int             *pMsgType;
    int             len;

%}

%%

UTI							{
                                *pMsgType = APP_WIDGET_UTI;
                            }

SSD							{
                                *pMsgType = APP_WIDGET_SSD;
                            }

PID							{
                                *pMsgType = APP_WIDGET_PID;
                            }

SWD							{
                                *pMsgType = APP_WIDGET_SWD;
                            }

DWD							{
                                *pMsgType = APP_WIDGET_DWD;
                            }

WSC							{
                                *pMsgType = APP_WIDGET_WSC;
                            }

WAC							{
                                *pMsgType = APP_WIDGET_WAC;
                            }

WMU							{
                                *pMsgType = APP_WIDGET_WMU;
                            }

WCD							{
                                *pMsgType = APP_WIDGET_WCD;
                            }

(\r\n)						{ }

[ \t\n]						{ }

.							{ }

%%

int widgetmsgtypewrap(void)
{
    return 1;
}

int widgetmsgtype(char *sp, int slen, int *dMsgType)
{
    pMsgType = dMsgType;

    widgetmsgtype_scan_bytes(sp, slen);
    widgetmsgtypelex();

    widgetmsgtype_delete_buffer(yy_current_buffer);
    yy_init = 1;

    return 0;
}

#ifdef  TEST
int main(int argc, char *argv[])
{
    int   	dMsgType = 0;

    char    *sp;
    int     slen;

    pMsgType = &dMsgType;

    if (argc != 2) {
        printf("INVALID ARGUMENT\n");
        printf("EX) widgetmsgtype MSGTYPE\n");
        return(1);
    }

    sp = strdup(argv[1]);
    slen = strlen(sp);

    widgetmsgtype_scan_bytes(sp, slen);
    widgetmsgtypelex();
	
	printf("MSGTYPE=[%d]\n", dMsgType);

    return 0;
}
#endif
