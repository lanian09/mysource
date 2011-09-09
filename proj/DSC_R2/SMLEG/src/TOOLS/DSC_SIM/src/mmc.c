/*
	$Id: mmc.c,v 1.1.1.1 2011/04/19 14:13:43 june Exp $
*/

/*
	-----------------------------------------------------------------------------
	Copyright (C) 2004-2005 by LINK@sys Inc, Korea
	All Rights Reserved.
*/


#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <termio.h>
#include <signal.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stropts.h>
#include <strings.h>

#include "mmc.h"

struct termios	tsaveorg;
struct termios	tsave;

int parse_type = 0;
struct sockaddr_in mmcsin;
struct sockaddr_in mmisin;

mmc_t *mmclist=NULL;

int 	mmcfd	= 0;
int 	mmifd	= 0;
int 	mmcfunc	= 0;
int 	mmcsucc	= 0;
char 	mmcbuf[BUFSIZ]		;
char 	mmclocbuf[BUFSIZ]	;

void (*mmc_cb_err)(char*) 		= NULL;		/* MMC ERROR CALLBACK FUNCTION				*/

void regmmc(void* list)
{
	mmclist = (mmc_t*)list;
}

void mprintf(char * fmt, ...)
{
	va_list args;

	va_start(args,fmt);

	vsprintf(mmclocbuf,fmt,args);
//	vprintf(fmt,args);
	printf ("%s", mmclocbuf);
	if(strlen(mmcbuf)+strlen(mmclocbuf) >= BUFSIZ) bzero(mmcbuf,BUFSIZ);
	strcat(mmcbuf,mmclocbuf);
	va_end(args);
}

/*
	init TERM I/O attributes
*/
void
initmmckey()
{
    tcgetattr(0,&tsaveorg);
}

/*
	restore TERM I/O attributes
*/
void
exitmmckey()
{
    tcsetattr(0,TCSAFLUSH,&tsaveorg);
}

int
mmcgetc()
{
	int ch;
	struct termios	tbuf;

	tcgetattr(0,&tsave);
	tbuf= tsave;
	tbuf.c_lflag &= ~ICANON;
	tbuf.c_lflag &= ~ECHO;
	tbuf.c_cc[VMIN] = 1;
	tbuf.c_cc[VTIME] = 0;
	tcsetattr(0,TCSAFLUSH,&tbuf);
	ch = getchar();
	tcsetattr(0,TCSAFLUSH,&tsave);

	return ch;
}

int
mmcgets(char* s)
{
	int ch,i=0;

/*
	s[0]=0;
*/
	while(1) {
		i = strlen(s);
		s[i] = 0;
		ch = mmcgetc();
		i = strlen(s);
		if(ch=='\n') {
			putchar(ch); s[i++]=ch; s[i]=0;
			break;
		}
		else if(ch=='\t') {
			putchar(' '); s[i++]=' '; s[i]=0;
            return 0;
		}
		else if(ch==CTRL('h')) {
			if(strlen(s)>0) {
				putchar(CTRL('h')); putchar(' '); putchar(CTRL('h'));
				s[--i]=0;
			}
		}
		else if(ch==CTRL('?')) {
			if(strlen(s)>0) {
				putchar(CTRL('?')); putchar(' '); putchar(CTRL('?'));
				s[--i]=0;
			}
		}
		else if(isalnum(ch) || isprint(ch)) {
			putchar(ch);
			s[i++]=ch;
		}
	}
	return i;
}

int
passwdgets(char* s)
{
	int ch,i=0;

	while(1) {
		i = strlen(s);
		s[i] = 0;
		ch = mmcgetc();
		i = strlen(s);
		if(ch=='\n') {
			putchar('\n'); s[i++]=ch; s[i]=0;
			break;
		}
		else if(ch==CTRL('h')) {
			if(strlen(s)>0) {
				putchar(CTRL('h')); putchar(' '); putchar(CTRL('h'));
				s[--i]=0;
			}
		}
		else if(ch==CTRL('?')) {
			if(strlen(s)>0) {
				putchar(CTRL('?')); putchar(' '); putchar(CTRL('?'));
				s[--i]=0;
			}
		}
		else if(isalnum(ch) || isprint(ch)) {
			putchar('*');
			s[i++]=ch;
		}
	}
	return i;
}

#define hist_hash_size  1

int mmcid=0;
hist_t *hhash[hist_hash_size]   = { NULL, };

unsigned int hhashfn(unsigned int mmcid)    { return ((hist_hash_size - 1) & mmcid); }

void hist_hash(hist_t *p)
{
    hist_t **pp = &hhash[hhashfn(p->mmcid)];
    if((p->next = *pp))
        p->next->prev = &p->next;
    p->prev = pp;
    *pp = p;
}

void hist_unhash(hist_t *p)
{
    if(p->prev) {
        if((*(p->prev) = p->next))
            p->next->prev = p->prev;
        p->next = NULL;
        p->prev = NULL;
    }
}


hist_t* hist_find(int mmcid)
{
    hist_t *p;

    for(p=hhash[hhashfn(mmcid)];p;p=p->next) {
        if(p->mmcid != mmcid) continue;
        return p;
    }
    return NULL;
}

hist_t* hist_add(int mmcid)
{
    hist_t *p;
    if(p=hist_find(mmcid)) return NULL;
    p = (hist_t*)malloc(sizeof(hist_t));
    if(p) {
        p->mmcid  = mmcid;
        p->next = NULL;
        p->prev = NULL;
        hist_hash(p);
    }
    return p;
}

void hist_del(int mmcid)
{
    hist_t *p;
    if(!(p=hist_find(mmcid))) return; /* not found */
    hist_unhash(p);
    free(p);
}

#define MAXHIST     20

void hist_show()
{
    hist_t *p;
    int i;

	mmcfunc = 1;
    if(!hhash[0]) mprintf("-- Empty History --\n\n");
    else {
        for(p=hhash[0],i=0;p && i<MAXHIST;p=p->next,i++) {
            mprintf("%-3d: %-s\n",p->mmcid,p->s);
        }
    }
}

#define MAXTOK  20
#define TOKLEN  32

int     ntok=0;
token_t token[MAXTOK];

int
findlex(int depth,char *s,char* f)
{
    int i,idx,tl,fl,found;

    if((tl=strlen(s))<=0) return 0;
    fl = strlen(f);
    for(i=0,found=0;*mmclist[i].s;i++) {
        if(mmclist[i].depth!=depth) continue;
        if(strncasecmp(s,mmclist[i].s,tl)) continue;
        if(strncasecmp(f,mmclist[i].f,fl)) continue;
        idx=i; found++;

        if(!strncasecmp(s,mmclist[i].s,strlen(mmclist[i].s))) {
            strcpy(s,mmclist[idx].s);
            return 1;
        }
    }
    if(found==1) {
        /* found correct command : convert short-word to long-word */
        strcpy(s,mmclist[idx].s);
    }
    return found;
}

int
findmmc(int depth,char *f,int *idx)
{
    int i,len,found;

    if((len=strlen(f))<=0) return 0;
    for(i=0,found=0;*mmclist[i].s;i++) {
        if(mmclist[i].depth!=depth) continue;

        if(strncasecmp(f,mmclist[i].f,len)) continue;
        *idx = i; found++;
		if(!strncasecmp(f, mmclist[i].f, strlen(mmclist[i].f))) return 1;
    }
    return (found);
}

int 
findsh(char *f, int *idx)
{
	int i, len;
    if((len=strlen(f))<=0) return 0;
    for(i=0;*mmclist[i].s;i++) {
        if(mmclist[i].depth!=0) 	continue;
		if(mmclist[i].cont!=0) 		continue;
        if(mmclist[i].s[0]!=f[0]) 	continue;
        *idx = i;
		return 1; 
    }
    return -1;
}

#define COLCNT  5

int
prn_amb(int depth,char *f,int cr)
{
    int i,len,cnt=0;


    for(i=0,len=strlen(f);*mmclist[i].s;i++) {
        if(mmclist[i].depth!=depth) continue;
        if(strncasecmp(f,mmclist[i].f,len)) continue;
        if(!(cnt++%COLCNT)) mprintf("\n\t");
		fprintf (stderr, "%s ", mmclist[i].s); 
  //      mprintf("%s ",mmclist[i].s); cnt++;

    }
    if(cr) {
        if(!(cnt++%COLCNT)) mprintf("\n\t");
        mprintf("<cr> "); cnt++;
    }
    if(!cnt) mprintf("\n\t<cr>");
    mprintf("\n\n");
    return (cnt);
}

int
prn_nextc(int depth,char *f,int cr)
{
    int i,len,cnt=0;


    for(i=0,len=strlen(f);*mmclist[i].s;i++) {
        if(mmclist[i].depth!=depth) continue;
        if(strncasecmp(f,mmclist[i].f,len)) continue;
        if(mmclist[i].f[len]!='-') continue;
        if(!(cnt++%COLCNT)) mprintf("\n\t");
		fprintf (stderr, "%s ", mmclist[i].s); 
 //       mprintf("%s ",mmclist[i].s); cnt++;
    }
    if(cr) {
        if(!(cnt++%COLCNT)) mprintf("\n\t");
        mprintf("<cr> "); cnt++;
    }
    if(!cnt) mprintf("\n\t<cr>");
    mprintf("\n\n");
    return (cnt);
}

int
makefmt(int ntok,char* f)
{
    token_t *tp;
    int i;

    if(!ntok) {
        bzero(f,strlen(f));
        return 1;
    }

    for(i=0,bzero(f,strlen(f));i<ntok;i++) {
        tp = &token[i];
        if(tp->t & T_STRING)    strcat(f,"S");
        if(tp->t & T_IPADDR)    strcat(f,"I");
        if(tp->t & T_NUMERIC)   strcat(f,"N");
        if(tp->t & T_COMMAND)   strcat(f,tp->s);
        strcat(f,"-");
    }
    return 1;
}

int chksh(char* cmd)
{
	int idx,rv;
	if(*cmd=='@' || *cmd=='#' || *cmd=='$' || *cmd=='%' 
		|| *cmd=='^' || *cmd=='&' || *cmd=='*') {
			rv =  findsh(cmd, &idx);	
			if(rv>=0) {
				mmcid++;
				mmcsucc = 1;
				mmclist[idx].func((cmd+1));
				return 1;	
			}
			return -1;
	}
	return 0;
}

int
chkhist(char *cmd,int cont)
{
    hist_t *hp;

    if(cont) mprintf("\n");
    /* Is a history command ? */
    if(*cmd=='!') { /* historty character '!' */
        if(*(cmd+1)=='!') { /* run previous command */
            hp = hhash[0];
            if(hp==NULL || hp->s==NULL) {
                mprintf("\nPrevious command not found!\n");
                return 0;
            }
            strcpy(cmd,hp->s);  /* copy previous command */
        }
        else {
            hp = hist_find(atoi(cmd+1));
            if(hp==NULL || hp->s==NULL) {
                mprintf("\n#%d history not found!\n",atoi(cmd+1));
                return 0;
            }
            strcpy(cmd,hp->s);  /* copy previous command */
        }
    }
    if(!strncmp(cmd,"hist",4)) {
        hist_show();;
        return 0;
    }
    return 1;
}

int
pparse(char *cmd,int cont)
{
    token_t *tp;
    hist_t  *hp;
    char *p, *sp;
    char sh_cmd[BUFSIZ], tmp_cmd[BUFSIZ], f[BUFSIZ],ip1[BUFSIZ];
    int i,j;
    int d1,d2,d3,d4;
	int rv;
    int idx,ret,tl,len=strlen(cmd); /* <-- do not delete */
	

	if(!mmclist) return 0;

	strcpy(tmp_cmd, cmd);
	sp = &tmp_cmd[0];
	mmcfunc 	= 0;
	parse_type 	= 0;
   	ntok 		= 0;

	bzero(mmclocbuf,BUFSIZ);
	bzero(mmcbuf,BUFSIZ);
	bzero(&token, sizeof(token_t) * MAXTOK);

	/* CHECK SH COMMAND	*/
	rv = chksh(sp);
	if(rv!=0) 
		return mmcid;

	/* CHECK HISTORY */
    if(!chkhist(cmd,cont)) {
        cont=0;
        goto mmccont;
    }
    len=strlen(cmd);

    /* extract token from string */
    if((p=strtok(cmd," -\t"))!=NULL) {
        strcpy(token[ntok++].s,p);
        while((p=strtok(NULL," -\t"))!=NULL) {
            strcpy(token[ntok++].s,p);
        }
    }

    /* find null string */
    if(!ntok) {
		char tt[BUFSIZ];

		bzero(tt,BUFSIZ);
        mprintf("\nAvailable commands:");
        prn_amb(ntok,tt,0);
        goto mmccont;
    }

    for(i=0,ret=0,bzero(f,BUFSIZ);i<ntok;i++) {
        tp = &token[i];
        tp->t = 0;

        tl = strlen(tp->s);

        /* Estimate token type */

        /* Numeric */
        tp->t |= T_NUMERIC;
        for(j=0;j<tl;j++) {
            if(!isdigit(tp->s[j]) && tp->s[j]!='-' && tp->s[j]!='+') {
                tp->t &= ~T_NUMERIC;
                break;
            }
        }

        /* IP address */
        tp->t |= T_IPADDR;
        strcpy(ip1,tp->s);
        for(j=0;j<3;j++) if((p=strchr(ip1,'.'))!=NULL) *p=' ';

        if(sscanf(ip1,"%d%d%d%d",&d1,&d2,&d3,&d4)<4) tp->t &= ~T_IPADDR;
        if(d1<0 || d1>255) tp->t &= ~T_IPADDR;
        if(d2<0 || d2>255) tp->t &= ~T_IPADDR;
        if(d3<0 || d3>255) tp->t &= ~T_IPADDR;
        if(d4<0 || d4>255) tp->t &= ~T_IPADDR;

        if(!tp->t) {
            tp->t = ((ret=findlex(i,tp->s,f))==1)? T_COMMAND : T_STRING;
        }

        /* case : command duplicated */
        if(ret>1) { strcat(f,tp->s); goto godup; }

        if(tp->t & T_STRING)    strcat(f,"S");
        if(tp->t & T_IPADDR)    strcat(f,"I");
        if(tp->t & T_NUMERIC)   strcat(f,"N");
        if(tp->t & T_COMMAND)   strcat(f,tp->s);
        if(i+1<ntok) strcat(f,"-");
    }
    ret = findmmc(ntok-1,f,&idx);

godup:
    switch(ret) {
    case    0:  /* not found */
	if(mmc_cb_err)
		mmc_cb_err(tmp_cmd);
        fprintf(stderr, "\nSyntax error at token %s     ",token[ntok-1].s);
//        mprintf("\nSyntax error at token %s     ",token[ntok-1].s);
        makefmt(--ntok,f);
        prn_amb(ntok,f,0);
        break;
    case   1:
        if(!mmclist[idx].cont && mmclist[idx].func!=NULL && !cont) {    /* completed command.. */
            hp = hist_add(++mmcid);

            /* create complete command for history */
            for(i=0,bzero(cmd,len);i<ntok;i++) { strcat(cmd,token[i].s); strcat(cmd," "); }
            strcpy(hp->s,cmd);

            /* terminal function? */
            if(!strncasecmp(token[ntok-1].s,"quit",4) || !strncasecmp(token[ntok-1].s,"exit",4))
                for(hp=hhash[0];hp;hp=hhash[0]) hist_del(hp->mmcid);

            /* run/call registered callback function */
            if(mmclist[idx].func!=NULL) {
				mmcsucc=1;
					mmclist[idx].func(
    	            	NULL, token[ 0].s,token[ 1].s,token[ 2].s,token[ 3].s,token[ 4].s,
						token[ 5].s,token[ 6].s,token[ 7].s,token[ 8].s,token[ 9].s,
						token[10].s,token[11].s,token[12].s,token[13].s,token[14].s, NULL
        	    	); 
					printf("\n");
			
			}
			bzero(cmd,len);  /* init command line buffer for stdin */
            return (mmcid);
        }
        else {
		if(mmc_cb_err)
			mmc_cb_err(tmp_cmd);
            mprintf("\nNext possible completions:     ");
            prn_nextc(ntok,f,(mmclist[idx].func!=NULL)? 1:0);
        }
        break;
    default :   /* ambigious */
	if(mmc_cb_err)
		mmc_cb_err(tmp_cmd);
        fprintf(stderr, "\nAmbiguous token %s     ",token[ntok-1].s);
//        mprintf("\nAmbiguous token %s     ",token[ntok-1].s);
        prn_amb(--ntok,f,0);
        break;
    }

mmccont:
    /* Create Completed Command */
    bzero(cmd,len);
    if(cont) for(i=0;i<ntok;i++) { strcat(cmd,token[i].s); strcat(cmd," "); }

    return (mmcid);
}

#define LINE_COL80  "--------------------------------------------------------------------------------"
void mmchelp(char* arg,...)
{
    int i;

	mmcfunc = 1;
	mprintf("%-60s%20s\n","(help)","uPresto,Inc");
    mprintf("%s\n",LINE_COL80);
    for(i=0;*mmclist[i].s;i++) {
        if(mmclist[i].depth) continue;

		mprintf("%-20s%-60s\n",mmclist[i].s,mmclist[i].c);
    }
    mprintf("%s\n",LINE_COL80);
}

int
mparse(char *cmd,int cont,int fd,struct sockaddr_in sin)
{
	parse_type = 1;
	
	mmifd 	= fd;
	mmisin 	= sin;
	return pparse(cmd,cont);
}


int
mmc_syntax_check(char *cmd,int cont)
{
    token_t tok[MAXTOK];
    token_t *tp;
    char *p,*pp;
    char tmp_cmd[BUFSIZ], f[BUFSIZ],ip1[BUFSIZ];
    int i,j;
    int d1,d2,d3,d4;
    int n_tok, idx,ret,tl,len=strlen(cmd); /* <-- do not delete */
	

	if(!mmclist) return 0;
    n_tok = 0;
	bzero(&token, sizeof(token_t) * MAXTOK);

    len=strlen(cmd);

  //  printf("syntax_check() : cmd=%s \n", cmd);

    /* extract token from string */
    if((p=strtok_r(cmd," -\t", &pp))!=NULL) {
        strcpy(tok[n_tok++].s,p);
        while((p=strtok_r(NULL," -\t", &pp))!=NULL) {
            strcpy(tok[n_tok++].s,p);
        }
    }

 //   printf("syntax_check() : n_tok = %d\n", n_tok);
    /* find null string */
    if(!n_tok) return 0;     /* syntax error */

    for(i=0,ret=0,bzero(f,BUFSIZ);i<n_tok;i++) {
        tp = &tok[i];
        tp->t = 0;

//        printf("syntax_check() : tok[i] = %s \n", tp->s);
        tl = strlen(tp->s);

        /* Estimate token type */

        /* Numeric */
        tp->t |= T_NUMERIC;
        for(j=0;j<tl;j++) {
            if(!isdigit(tp->s[j]) && tp->s[j]!='-' && tp->s[j]!='+') {
                tp->t &= ~T_NUMERIC;
                break;
            }
        }

        /* IP address */
        tp->t |= T_IPADDR;
        strcpy(ip1,tp->s);
        for(j=0;j<3;j++) if((p=strchr(ip1,'.'))!=NULL) *p=' ';

        if(sscanf(ip1,"%d%d%d%d",&d1,&d2,&d3,&d4)<4) tp->t &= ~T_IPADDR;
        if(d1<0 || d1>255) tp->t &= ~T_IPADDR;
        if(d2<0 || d2>255) tp->t &= ~T_IPADDR;
        if(d3<0 || d3>255) tp->t &= ~T_IPADDR;
        if(d4<0 || d4>255) tp->t &= ~T_IPADDR;

        if(!tp->t) {
            tp->t = ((ret=findlex(i,tp->s,f))==1)? T_COMMAND : T_STRING;
        }

        /* case : command duplicated */
        if(ret>1) { strcat(f,tp->s); goto godup;}

        if(tp->t & T_STRING)    strcat(f,"S");
        if(tp->t & T_IPADDR)    strcat(f,"I");
        if(tp->t & T_NUMERIC)   strcat(f,"N");
        if(tp->t & T_COMMAND)   strcat(f,tp->s);
        if(i+1<n_tok) strcat(f,"-");

//       printf("syntax_check() : f=%s \n", f);
    }
    ret = findmmc(n_tok-1,f,&idx);

//    printf("syntax_check() : ret=%d, tok=%s f=%s \n", ret, tok[n_tok].s, f);
godup:
    switch(ret) {
    case    0:  /* not found */
        return 0;   /* mmc syntax error   */
        break;
    case   1:
        if(!mmclist[idx].cont && mmclist[idx].func!=NULL && !cont) {    /* completed command.. */
            /* run/call registered callback function */
            if(mmclist[idx].func!=NULL) 
                return 1;   /* mmc syntax valid    */
        } else 
            return 0;
        
        break;
    default :   /* ambigious */
        return 0;   
        break;
    }

mmc_err:
    return 0;   /* syntax error     */
}

/*
	$Log: mmc.c,v $
	Revision 1.1.1.1  2011/04/19 14:13:43  june
	성능 패키지
	
	Revision 1.1.1.1  2011/01/20 12:18:51  june
	DSC CVS RECOVERY
	
	Revision 1.1  2009/05/09 09:41:01  dsc
	init
	
	Revision 1.1  2008/12/12 00:07:21  yhshin
	*** empty log message ***
	
	Revision 1.2  2008/11/25 14:31:30  yhshin
	*** empty log message ***
	
	Revision 1.1  2008/11/06 05:43:03  sjjeon
	oam lib
	
	Revision 1.1.1.1  2008/10/20 06:55:48  sjjeon
	accelerator server
	
	Revision 1.1  2008/01/11 12:13:26  june
	oam
	
	Revision 1.4  2005/03/30 10:51:50  yhshin
	다른 mmc lib로 변경
	
	Revision 1.3  2005/01/28 07:23:00  yhshin
	lib v1.0 init
*/
