#include "mmcr.h"


void strtoupper( char headName[50] ) {
   int count = 0, i=0;
   count = strlen( headName );
   if ( count>0 )
        for( i=0; i<count; i++ )
            headName[i] = toupper( headName[i] );
}

int  AllTrim (char *_buf)
{
    int  ch;
    int   i, first, last;

    first = last = -1;
    for( i = 0; (ch = _buf[i]) ; i++ )
    {
        if(i > MAX_COL_SIZE) break;

        if ( !isspace(ch)  ||  (((0x80000000) & ch) == 0x80000000) )
        {
            if  ( first < 0 )
            {
                first = i;
            }
            last = i;
        }
    }

    if  ( last < 0 )
    {
        _buf[ 0 ] = '\0';
        return 0;
    }
    else
    {
        int    len;

        memmove( _buf, &_buf[first], (len = last - first + 1) );

        _buf[ len ] = '\0';

        return len;
    }
}

//=======================head==============================
void headMessage ( pMmcMemberTable pmmcHdlr, char result[BUFSIZE] )
{
    int i = 0, j=0;
    char titleMsg[BUFSIZE], tmpMsg[BUFSIZE], line[BUFSIZE];
    char headName[20];
    //int convert;
    memset(titleMsg, 0, sizeof(titleMsg));
    memset(tmpMsg, 0, sizeof(tmpMsg));
    memset(line, 0, sizeof(line));
    memset(headName, 0, sizeof(headName));

    strcat(line, "\n    ======");
    strcat(titleMsg, "\n      " );
    for( i=0; i<pmmcHdlr->paraCnt; i++ ){
       	if( !strcasecmp(pmmcHdlr->cmdName, "dis-svc-type") && i == 9) continue; 
		strtoupper(pmmcHdlr->paraName[i]);
        sprintf( tmpMsg, "%-*s", pmmcHdlr->printFormat[i],pmmcHdlr->paraName[i]);
        strcat( titleMsg, tmpMsg );
        for( j=0; j<pmmcHdlr->printFormat[i]; j++ ){
            strcat( line,"=" );
        }

     }
     strcat( titleMsg, line );
     strcat( line, titleMsg );
     strcpy ( result , line );
}

//========================tail===============================
void tailMessage ( pMmcMemberTable mmcHdlr,char result[BUFSIZE], int cnt )
{
    int i = 0, j=0;
    char totalMsg[BUFSIZE], line[BUFSIZE], sumLine[BUFSIZE];
    memset(totalMsg, 0, sizeof(totalMsg));
    memset(line, 0, sizeof(line));
    memset(sumLine, 0, sizeof(sumLine));
    strcat(line, "\n    ======");
    strcat(sumLine, "\n    ------");
    strcat(totalMsg, "\n      " );
    for( i=0; i<mmcHdlr->paraCnt; i++ ){
        if(!strcasecmp(mmcHdlr->cmdName, "dis-svc-type") && i == 9) continue;
		for( j=0; j<mmcHdlr->printFormat[i]; j++ ){
            strcat( line,"=" );
            strcat( sumLine,"-" );
        }
    }

    //jean add
    strcat( line,"\n" );

    sprintf(totalMsg, "\n      TOTAL COUNT = %d ",cnt);
    strcat( sumLine, totalMsg );
    strcat( sumLine, line );
    strcpy( result , sumLine);
}

void conTailMessage ( pMmcMemberTable mmcHdlr,char result[BUFSIZE] )
{
    int i = 0, j=0;
    char sumLine[BUFSIZE];
    memset(sumLine, 0, sizeof(sumLine));
    strcat(sumLine, "\n    ------");
    for( i=0; i<mmcHdlr->paraCnt; i++ ){
        for( j=0; j<mmcHdlr->printFormat[i]; j++ ){
            strcat( sumLine,"-" );
        }
     }

     //jean add
     strcat( sumLine,"\n" );

     strcpy( result , sumLine);
}

//==========================body==================================
void bodyLineMessage ( pMmcMemberTable mmcHdlr,DLinkedList *node, char result[BUFSIZE] )
{
    int i = 0, len=0;
    char totalMsg[BUFSIZE], tmpMsg[BUFSIZE];
    memset(totalMsg, 0, sizeof(totalMsg));
    memset(tmpMsg, 0, sizeof(tmpMsg));
    strcat(totalMsg, "\n      " );
    for( i=0; i<mmcHdlr->paraCnt; i++ ){
		len=AllTrim( node->item.columns[i] );
		strtoupper(node->item.columns[i]);
		sprintf( tmpMsg, "%-*s", 
		mmcHdlr->printFormat[i],( !strcasecmp( node->item.columns[i], "$")==1 ? node->next->item.columns[i]: node->item.columns[i]) );
		strcat( totalMsg, tmpMsg );
     }
     strcpy(result , totalMsg);
}

void bodyLineUrlMessage ( pMmcMemberTable mmcHdlr,DLinkedList *node, char result[BUFSIZE] )
{
    int i = 0, len=0;
    char totalMsg[BUFSIZE], tmpMsg[BUFSIZE];
    memset(totalMsg, 0, sizeof(totalMsg));
    memset(tmpMsg, 0, sizeof(tmpMsg));
    strcat(totalMsg, "\n      " );
    for( i=0; i<mmcHdlr->paraCnt; i++ ){
        len=AllTrim( node->item.columns[i] );
        if( i!= 1)
        strtoupper(node->item.columns[i]);

                sprintf( tmpMsg, "%-*s", mmcHdlr->printFormat[i],( !strcasecmp( node->item.columns[i], "$")==1 ? "": node->item.columns[i]) );
                strcat( totalMsg, tmpMsg );
     }
     strcpy(result , totalMsg);
}

void bodyLineAaaMessage ( pMmcMemberTable mmcHdlr,DLinkedList *node, char result[BUFSIZE] )
{
    int i = 0, len=0;
    char totalMsg[BUFSIZE], tmpMsg[BUFSIZE];
    memset(totalMsg, 0, sizeof(totalMsg));
    memset(tmpMsg, 0, sizeof(tmpMsg));
    strcat(totalMsg, "\n      " );
    for( i=0; i<mmcHdlr->paraCnt; i++ ){
        len=AllTrim( node->item.columns[i] );
        if( i!=2 && i!=5 )
        strtoupper(node->item.columns[i]);
                sprintf( tmpMsg, "%-*s", mmcHdlr->printFormat[i],( !strcasecmp( node->item.columns[i], "$")==1 ? "": node->item.columns[i]) );
                strcat( totalMsg, tmpMsg );
     }
     strcpy(result , totalMsg);
}

void bodyLineSvcMessage ( pMmcMemberTable mmcHdlr,DLinkedList *node, char result[BUFSIZE] )
{
    int i = 0, len=0;
    char totalMsg[BUFSIZE], tmpMsg[BUFSIZE];
    memset(totalMsg, 0, sizeof(totalMsg));
    memset(tmpMsg, 0, sizeof(tmpMsg));
    strcat(totalMsg, "\n      " );
    for( i=0; i<mmcHdlr->paraCnt; i++ ){
        len=AllTrim( node->item.columns[i] );
        strtoupper(node->item.columns[i]);
       	if (i == 9) continue; 
		if( i>3 ) {
        	if( i==6 )
            	strcpy( node->item.columns[i-1] , ( !strcasecmp( node->item.columns[i-1], "0")==1 ? "ALL": node->item.columns[i-1]) );

            sprintf( tmpMsg, "%-*s", mmcHdlr->printFormat[i],( !strcasecmp( node->item.columns[i-1], "$")==1 ? "": node->item.columns[i-1]) );
            strcat( totalMsg, tmpMsg );
        } else if ( i== 3 ) {
            sprintf( tmpMsg, "%-*s", mmcHdlr->printFormat[i],( !strcasecmp( node->item.columns[mmcHdlr->paraCnt-1], "$")==1 ? "": node->item.columns[mmcHdlr->paraCnt-1]) );
            strcat( totalMsg, tmpMsg );
        } else {
            sprintf( tmpMsg, "%-*s", mmcHdlr->printFormat[i],( !strcasecmp( node->item.columns[i], "$")==1 ? "": node->item.columns[i]) );
             strcat( totalMsg, tmpMsg );
        }
    }
    strcpy(result , totalMsg);
}


void bodyLineTrcMessage ( pMmcMemberTable mmcHdlr,DLinkedList *node, char result[BUFSIZE] )
{
    int i = 0, len=0;
    time_t date;
    struct tm   *pLocalTime;
    char totalMsg[BUFSIZE], tmpMsg[BUFSIZE];
    memset(totalMsg, 0, sizeof(totalMsg));
    memset(tmpMsg, 0, sizeof(tmpMsg));
    strcat(totalMsg, "\n      " );
    for( i=0; i<mmcHdlr->paraCnt; i++ ){
        len=AllTrim( node->item.columns[i] );
        strtoupper(node->item.columns[i]);
                if( i == mmcHdlr->paraCnt-1 ){
                    date = atol( node->item.columns[i] );
                    if ((pLocalTime = (struct tm*)localtime((time_t*)&date)) == NULL) {
                        strcpy(node->item.columns[i], "");
                    } else {
                        strftime (node->item.columns[i], 32, "%Y-%m-%d %H:%M", pLocalTime);
                    }
                }

                if( i== 0 )
                    sprintf( tmpMsg, "%-*s", mmcHdlr->printFormat[i],( !strcasecmp( node->item.columns[i], "1")==1 ? "IMSI":"IP" ) );
                else
                    sprintf( tmpMsg, "%-*s", mmcHdlr->printFormat[i],( !strcasecmp( node->item.columns[i], "$")==1 ? node->next->item.columns[i]: node->item.columns[i]) );

                strcat( totalMsg, tmpMsg );
     }
     strcpy(result , totalMsg);
}

void bodyLinePdsnMessage ( pMmcMemberTable mmcHdlr,DLinkedList *node, char result[BUFSIZE] )
{
    int i = 0, len=0;
    //time_t date;
    //struct tm   *pLocalTime;
    char totalMsg[BUFSIZE], tmpMsg[BUFSIZE];
    memset(totalMsg, 0, sizeof(totalMsg));
    memset(tmpMsg, 0, sizeof(tmpMsg));
    strcat(totalMsg, "\n      " );
    for( i=0; i<mmcHdlr->paraCnt; i++ ){
        len=AllTrim( node->item.columns[i] );
        strtoupper(node->item.columns[i]);
                //if( i== 1 )
                //    sprintf( tmpMsg, "%-*s", mmcHdlr->printFormat[i],( !strcasecmp( node->item.columns[i], "1")==1 ? "A":"B" ) );
                //else
                    sprintf( tmpMsg, "%-*s", mmcHdlr->printFormat[i],( !strcasecmp( node->item.columns[i], "$")==1 ? "": node->item.columns[i]) );

                strcat( totalMsg, tmpMsg );
     }
     strcpy(result , totalMsg);
}

//===========================old&new=====================================
void addHeadMessage ( pMmcMemberTable pmmcHdlr, char result[BUFSIZE] )
{
    int i = 0, j=0;
    char titleMsg[BUFSIZE], tmpMsg[BUFSIZE], line[BUFSIZE];
    char headName[20];
    //int convert;
    memset(titleMsg, 0, sizeof(titleMsg));
    memset(tmpMsg, 0, sizeof(tmpMsg));
    memset(line, 0, sizeof(line));
    memset(headName, 0, sizeof(headName));

    strcat(line, "\n    ==============");
    strcat(titleMsg, "\n               " );
    for( i=0; i<pmmcHdlr->paraCnt; i++ ){
       	if (!strcasecmp(pmmcHdlr->cmdName, "add-svc-type") && i == 9)
			continue;
		if (!strcasecmp(pmmcHdlr->cmdName, "chg-svc-type") && i == 9)
		    continue;	
		if (!strcasecmp(pmmcHdlr->cmdName, "del-svc-type") && i == 9)
		    continue;	
		
		strtoupper(pmmcHdlr->paraName[i]);
        sprintf( tmpMsg, "%-*s", pmmcHdlr->printFormat[i],pmmcHdlr->paraName[i]);
        strcat( titleMsg, tmpMsg );
        for( j=0; j<pmmcHdlr->printFormat[i]; j++ ){
            strcat( line,"=" );
        }

    }
    strcat( titleMsg, line );
    strcat( line, titleMsg );
    strcpy ( result , line );
}

void addTailMessage ( pMmcMemberTable mmcHdlr,char result[BUFSIZE] )
{
    int i = 0, j=0;
    char line[BUFSIZE];
    memset(line, 0, sizeof(line));
    strcat(line, "\n    ==============");
    for( i=0; i<mmcHdlr->paraCnt; i++ ){
       	if( !strcasecmp(mmcHdlr->cmdName, "add-svc-type") && i == 9) continue;  
		if( !strcasecmp(mmcHdlr->cmdName, "chg-svc-type") && i == 9) continue;	
		if( !strcasecmp(mmcHdlr->cmdName, "del-svc-type") && i == 9) continue;	
		for( j=0; j<mmcHdlr->printFormat[i]; j++ ){
            strcat( line,"=" );
        }
     }

     //jean add
     strcat( line,"\n" );

     strcpy( result , line );
}

void addBodyLineMessage ( pMmcMemberTable mmcHdlr,char columns[MAX_COL_COUNT][MAX_COL_SIZE],char result[BUFSIZE], int type )
{
    int i = 0, len=0;
    char totalMsg[BUFSIZE], tmpMsg[BUFSIZE];
    memset(totalMsg, 0, sizeof(totalMsg));
    memset(tmpMsg, 0, sizeof(tmpMsg));
    if( type > 0 ){
        strcat(totalMsg, "\n   " );
        strcat(totalMsg, ( type==1 ? "     OLD    ": "     NEW    " ) );
    }else{
        strcat(totalMsg, "\n               " );
    }

    for( i=0; i<mmcHdlr->paraCnt; i++ ){
        len=AllTrim( columns[i] );
        strtoupper(columns[i]);
        sprintf( tmpMsg, "%-*s", mmcHdlr->printFormat[i],( !strcasecmp( columns[i], "$")==1 ? "": columns[i]) );
        strcat( totalMsg, tmpMsg );
     }
     strcpy(result , totalMsg);
}

void addBodyLineUrlMessage ( pMmcMemberTable mmcHdlr,char columns[MAX_COL_COUNT][MAX_COL_SIZE], char result[BUFSIZE], int type )
{
    int i = 0, len=0;
    char totalMsg[BUFSIZE], tmpMsg[BUFSIZE];
    memset(totalMsg, 0, sizeof(totalMsg));
    memset(tmpMsg, 0, sizeof(tmpMsg));
    if( type > 0 ){
        strcat(totalMsg, "\n   " );
        strcat(totalMsg, ( type==1 ? "     OLD    ": "     NEW    " ) );
    }else{
        strcat(totalMsg, "\n               " );
    }

    for( i=0; i<mmcHdlr->paraCnt; i++ ){
        len=AllTrim( columns[i] );
        if( i!= 1)
        strtoupper(columns[i]);

                sprintf( tmpMsg, "%-*s", mmcHdlr->printFormat[i],( !strcasecmp( columns[i], "$")==1 ? "": columns[i]) );
                strcat( totalMsg, tmpMsg );
     }
     strcpy(result , totalMsg);
}

void addBodyLineAaaMessage ( pMmcMemberTable mmcHdlr,char columns[MAX_COL_COUNT][MAX_COL_SIZE], char result[BUFSIZE] , int type)
{
    int i = 0, len=0;
    char totalMsg[BUFSIZE], tmpMsg[BUFSIZE];
    memset(totalMsg, 0, sizeof(totalMsg));
    memset(tmpMsg, 0, sizeof(tmpMsg));
    if( type > 0 ){
        strcat(totalMsg, "\n   " );
        strcat(totalMsg, ( type==1 ? "     OLD    ": "     NEW    " ) );
    }else{
        strcat(totalMsg, "\n               " );
    }

    for( i=0; i<mmcHdlr->paraCnt; i++ ){
        len=AllTrim( columns[i] );
        if( i!=2 && i!=5 )
        strtoupper(columns[i]);
                sprintf( tmpMsg, "%-*s", mmcHdlr->printFormat[i],( !strcasecmp( columns[i], "$")==1 ? "": columns[i]) );
                strcat( totalMsg, tmpMsg );
     }
     strcpy(result , totalMsg);
}

void addBodyLineSvcMessage ( pMmcMemberTable mmcHdlr,char columns[MAX_COL_COUNT][MAX_COL_SIZE], char result[BUFSIZE], int type )
{
    int i = 0, len=0;
    char totalMsg[BUFSIZE], tmpMsg[BUFSIZE];
    memset(totalMsg, 0, sizeof(totalMsg));
    memset(tmpMsg, 0, sizeof(tmpMsg));
    if( type > 0 ){
        strcat(totalMsg, "\n   " );
        strcat(totalMsg, ( type==1 ? "     OLD    ": "     NEW    " ) );
    }else{
        strcat(totalMsg, "\n               " );
    }
    
    for( i=0; i<mmcHdlr->paraCnt; i++ ){
        len=AllTrim( columns[i] );
        strtoupper(columns[i]);
		if(i == 9 ) continue; // CDRIP_FLAG Á¦¿Ü
		if( i>3 ) {
           	if( i==6 )
               	strcpy( columns[i-1] , ( !strcasecmp( columns[i-1], "0")==1 ? "ALL": columns[i-1]) );

            sprintf( tmpMsg, "%-*s", mmcHdlr->printFormat[i],( !strcasecmp( columns[i-1], "$")==1 ? "": columns[i-1]) );
            strcat( totalMsg, tmpMsg );
        } else if ( i== 3 ) {
          	sprintf( tmpMsg, "%-*s", mmcHdlr->printFormat[i],( !strcasecmp( columns[mmcHdlr->paraCnt-1], "$")==1 ? "": columns[mmcHdlr->paraCnt-1]) );
            strcat( totalMsg, tmpMsg );
        } else {
            sprintf( tmpMsg, "%-*s", mmcHdlr->printFormat[i],( !strcasecmp( columns[i], "$")==1 ? "": columns[i]) );
            strcat( totalMsg, tmpMsg );
        }
    }
    strcpy(result , totalMsg);
}


void addBodyLineTrcMessage ( pMmcMemberTable mmcHdlr,char columns[MAX_COL_COUNT][MAX_COL_SIZE], char result[BUFSIZE],int type )
{
    int i = 0, len=0;
    time_t date;
    struct tm   *pLocalTime;
    char totalMsg[BUFSIZE], tmpMsg[BUFSIZE];
    memset(totalMsg, 0, sizeof(totalMsg));
    memset(tmpMsg, 0, sizeof(tmpMsg));
    if( type > 0 ){
        strcat(totalMsg, "\n   " );
        strcat(totalMsg, ( type==1 ? "     OLD    ": "     NEW    " ) );
    }else{
        strcat(totalMsg, "\n               " );
    }

    for( i=0; i<mmcHdlr->paraCnt; i++ ){
        len=AllTrim( columns[i] );
        strtoupper(columns[i]);
                if( i == mmcHdlr->paraCnt-1 ){
                    date = atol( columns[i] );
                    if ((pLocalTime = (struct tm*)localtime((time_t*)&date)) == NULL) {
                        strcpy(columns[i], "");
                    } else {
                        strftime (columns[i], 32, "%Y-%m-%d %H:%M", pLocalTime);
                    }
                }

                if( i== 0 )
                    sprintf( tmpMsg, "%-*s", mmcHdlr->printFormat[i],( !strcasecmp( columns[i], "1")==1 ? "IMSI":"IP" ) );
                else
                    sprintf( tmpMsg, "%-*s", mmcHdlr->printFormat[i],( !strcasecmp( columns[i], "$")==1 ? columns[i+1]: columns[i]) );

                strcat( totalMsg, tmpMsg );
     }
     strcpy(result , totalMsg);
}

void addBodyLinePdsnMessage ( pMmcMemberTable mmcHdlr,char columns[MAX_COL_COUNT][MAX_COL_SIZE], char result[BUFSIZE], int type )
{
    int i = 0, len=0;
    //time_t date;
    //struct tm   *pLocalTime;
    char totalMsg[BUFSIZE], tmpMsg[BUFSIZE];
    memset(totalMsg, 0, sizeof(totalMsg));
    memset(tmpMsg, 0, sizeof(tmpMsg));
    if( type > 0 ){
        strcat(totalMsg, "\n   " );
        strcat(totalMsg, ( type==1 ? "     OLD    ": "     NEW    " ) );
    }else{
        strcat(totalMsg, "\n               " );
    }

    for( i=0; i<mmcHdlr->paraCnt; i++ ){
        len=AllTrim( columns[i] );
        strtoupper(columns[i]);
                //if( i== 1 )
                //    sprintf( tmpMsg, "%-*s", mmcHdlr->printFormat[i],( !strcasecmp( columns[i], "1")==1 ? "A":"B" ) );
                //else
                    sprintf( tmpMsg, "%-*s", mmcHdlr->printFormat[i],( !strcasecmp( columns[i], "$")==1 ? "": columns[i]) );

                strcat( totalMsg, tmpMsg );
     }
     strcpy(result , totalMsg);
}

void addBodyLinePpsMessage(pMmcMemberTable mmcHdlr, char columns[MAX_COL_COUNT][MAX_COL_SIZE],
                           char result[BUFSIZE], int type )
{
    int i = 0, len=0;
    char totalMsg[BUFSIZE], tmpMsg[BUFSIZE];
    memset(totalMsg, 0, sizeof(totalMsg));
    memset(tmpMsg, 0, sizeof(tmpMsg));
    if( type > 0 ){
        strcat(totalMsg, "\n   " );
        strcat(totalMsg, ( type==1 ? "     OLD    ": "     NEW    " ) );
    }else{
        strcat(totalMsg, "\n      " );
    }

    for( i=0; i<mmcHdlr->paraCnt; i++ ){
        len=AllTrim( columns[i] );

        switch(i){
            case 0:
            case 1:
                sprintf( tmpMsg, "%-*s", mmcHdlr->printFormat[i],( !strcmp( columns[i], "$")==1 ? "": columns[i]) );
                break;
            case 2:
                sprintf( tmpMsg, "%-*s", mmcHdlr->printFormat[i],( !strcmp( columns[i], "0")==1 ? "OFF": "ON") );
                break;
        }

        strcat( totalMsg, tmpMsg );
     }
     strcpy(result , totalMsg);
}

void addBodyLineIcmpMessage(pMmcMemberTable mmcHdlr, char columns[MAX_COL_COUNT][MAX_COL_SIZE],
                           char result[BUFSIZE], int type )
{
    int len=0;
    char totalMsg[BUFSIZE], tmpMsg[BUFSIZE];
    memset(totalMsg, 0, sizeof(totalMsg));
    memset(tmpMsg, 0, sizeof(tmpMsg));
    if( type > 0 ){
        strcat(totalMsg, "\n   " );
        strcat(totalMsg, ( type==1 ? "     OLD    ": "     NEW    " ) );
    }else{
        strcat(totalMsg, "\n      " );
    }

     len=AllTrim( columns[0] );

     if(!strcmp(columns[0], "0"))
         sprintf( tmpMsg, "%-*s", mmcHdlr->printFormat[0], "OFF");
     else
         sprintf( tmpMsg, "%-*s", mmcHdlr->printFormat[0], "ON");

     strcat( totalMsg, tmpMsg );

     len=AllTrim( columns[1] );
     if(!strcmp(columns[0], "1"))
         sprintf( tmpMsg, "%-*s", mmcHdlr->printFormat[1], "-");
     else
         sprintf( tmpMsg, "%-*s", mmcHdlr->printFormat[1], columns[1]);

     strcat( totalMsg, tmpMsg );

     strcpy(result , totalMsg);
}
