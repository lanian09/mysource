#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>

#include "conf.h"
#define Line	printf("check file:%s,line:%d\n",__FILE__,__LINE__)

/*
    remove white space

	ex) " Hello World " -> "HelloWorld"
*/
void
whitespace (char *s)
{
	char buf[BUFSIZ], *p = s;
    int i, j, len = strlen (s);

    strcpy (buf, s);
    for (i = 0, j = 0; i < len; i++) {
    	if (buf[i] == ' ') p[j] = 0;
        else if (buf[i] == '\t') p[j] = 0;
        else p[j++] = buf[i];
    }
    p[j] = 0;
}

/*
    remove reft,right trip

	ex) " Hello World " -> "Hello World"
*/
void
btrip(char *s)
{
	char buf[BUFSIZ], *p = s;
    int i, j, len;

	/* "  Hello World  " ; len:15 */
    strcpy (buf, s);
	len = strlen(s);
    for (i=0,j=0;i<len;i++) {
    	if(buf[i]==' ' || buf[i]=='\t') continue;
		strcpy(p,&buf[i]); break;
	}
	/* "Hello World  " ; len:13 */
    strcpy (buf, s);
	len = strlen(s)-1;
    for (;len>0;len--) {
    	if(buf[len]==' ' || buf[len]=='\t' || buf[len]=='\n') { p[len]=0; continue; }
		strncpy(p,buf,len); p[len+1]=0; break;
	}
	/* "Hello World" ; len:11*/
}

void btrim(char *s)
{
	btrip(s);
}



/*
	make string lower case

	ex) "Hello World" -> "hello world"
*/
char *
strlwr (char *s)
{
    char *p = s;

    while (*p) {
    	if ('A' <= *p && 'Z' >= *p) *p += ' ';
        p++;
    }
    return s;
}

/*
	make string upper case

	ex) "Hello World" -> "HELLO WORLD"
*/
char *
strupr (char *s)
{
    char *p = s;
    while (*p) {
    	if ('a' <= *p && 'z' >= *p) *p -= ' ';
        p++;
    }
    return s;
}

/*
	read conf file (*.conf)

	A=a		; A:label, a=value
	# xxx   ; comment
*/
int
readconf(char* path,CONFTAB* tab)
{
    FILE* fp;
    char buf[BUFSIZ],*p;
    int i= 0;

    fp = fopen(path,"r");
    if(fp == NULL) return 0;

    while(fgets(buf, BUFSIZ, fp) != NULL) {
		fflush(fp);
    	if ((p = strchr (buf, '\n')) != NULL) *p = 0;
        if ((p = strchr (buf, '#')) != NULL) *p = 0;
        whitespace (buf);
        if (!strlen (buf)) continue;
        if ((p = strchr (buf, '=')) != NULL) *p = ' ';
        else continue;

        if (sscanf (buf, "%s%s\n", tab[i].label, tab[i].val) != 2) continue;
		strupr(tab[i].label);
        i++;
    }
    fclose(fp);

    return i;
}

/*
	get value from conf file with label

	ex) getconf("./my.conf","A",val);
	printf("A = %s \n",val);

	output: 
    A = a
	-----------------------------------
    $ cat my.conf

	A=a
	B=b
	C=c
*/
int
getconf(char* path,char* l,char* val)
{
    CONFTAB tab[100];
    char buf[BUFSIZ],label[BUFSIZ];
    int num,i,len;

	strcpy(label,l);
	strupr(label);
    len = strlen(label);
    if(len<=0) return -1;

    num = readconf(path,tab);
    for(i=0;i<num;i++) {
        if(len != strlen(tab[i].label)) continue;
        if(!strncmp(tab[i].label,label,len)) {
            strcpy(val,tab[i].val);
            return num;
        }
    }
    val[0]=0;
    return -1;
}

/*
	get value from conf file with label,delim

	ex) getconfdelim("./my.conf","A",',',1,val);
	printf("A (field #1) = %s \n",val);

	output: 
    A (filed #1) = a2
	-----------------------------------
    $ cat my.conf

	A=a1,a2
	B=b1,b2
	C=c1,c2
*/
int
getconfdelim(char* path,char* label,char delim,int field,char* val)
{
    char arg[BUFSIZ],*p,*s;
    int i;

    if(getconf(path,label,arg)<=0) return 0;
    whitespace(arg);
    for(i=0,s=arg;i<field;i++) {
        if((p=strchr(s,delim))!=NULL) s=p+1;
		else return 0;
    }
    if((p=strchr(s,delim))!=NULL) *p=0;
    strcpy(val,s);
    return 1;
}

/*
	find label in conf file and return result

	ex) findlabel("./my.conf","A");
	printf("find label 'A' in my.conf (%s)\n"
		,(findlabel("./my.conf","A")>0)? "ok":"fail");

	output: 
    find label 'A' in my.conf (ok)
	-----------------------------------
    $ cat my.conf

	A=a
*/
int
findlabel(char* path,char* label)
{
	char s[BUFSIZ],arg[BUFSIZ],*p;

	FILE* fp;
	if((fp=fopen(path,"r"))==NULL) return -1;
	while(fgets(s,BUFSIZ,fp)!=NULL) {
		fflush(fp);
		if((p=strchr(s,'#'))!=NULL) *p=0;
		if(sscanf(s,"%s",arg)<1) continue;
		if(strcmp(arg,label)) continue;

		fclose(fp);
		return 1;
	}
	fclose(fp);
	return -1;
}

int
getlabel(char* path,char* label,char* val)
{
	char s[BUFSIZ],arg[BUFSIZ],*p;

	FILE* fp;
	if((fp=fopen(path,"r"))==NULL) return -1;
	while(fgets(s,BUFSIZ,fp)!=NULL) {
		fflush(fp);
		if((p=strchr(s,'#'))!=NULL) *p=0;
		if(sscanf(s,"%s%s",arg,val)<2) continue;
		if(strcmp(arg,label)) continue;
		fclose(fp);
		return 1;
	}
	fclose(fp);
	return -1;
}

/*
	update value in conf file with label and return result

	ex) updlabel("./my.conf","A",val);
	getconf("./my.conf","A",val);
	printf("A = %s \n",val);
	printf("update label 'A' in my.conf (%s)\n"
		,(updlabel("./my.conf","A")>0)? "ok":"fail");
	printf("A = %s \n",val);

	output: 
	A = a
    update label 'A' in my.conf (ok)
	A = b
	-----------------------------------
    $ cat my.conf

	A=a
*/
int
updlabel(char* path,char* label,char* val)
{
    char swap[BUFSIZ],s[BUFSIZ],buf[BUFSIZ];
	char arg[BUFSIZ];
    FILE *fp,*out;

    sprintf(swap,"%s.swap",path);
    if((fp=fopen(path,"r"))==NULL) return -1;
    if((out=fopen(swap,"w"))==NULL) { fclose(out); return -1; }

    while(fgets(s,BUFSIZ,fp)!=NULL) {
		fflush(fp);
		if(sscanf(s,"%s",arg)<2) {
			if(!strcmp(arg,label)) {
				fprintf(out,"%s %s",label,val);
				continue;
			}
		}
		fprintf(out,"%s",s);
	}
	fclose(fp);
	fclose(out);

	unlink(path);
	if(link(swap,path)<0) perror("swap");
	unlink(swap);
}

/*
	make comment all line

	ex) mkcomment("./my.conf");
	-----------------------------------
    $ cat my.conf
	A=a
	B=b

    $ cat my.conf (after)
	# A=a
	# B=b
*/
int
mkcomment(char* path)
{
    char swap[BUFSIZ],s[BUFSIZ],buf[BUFSIZ];
    FILE *fp,*out;

    sprintf(swap,"%s.swap",path);
    if((fp=fopen(path,"r"))==NULL) return -1;
    if((out=fopen(swap,"w"))==NULL) { fclose(out); return -1; }

    while(fgets(s,BUFSIZ,fp)!=NULL) {
		fflush(fp);
		if(s[0]=='#') {
			fprintf(out,"%s",s);
			continue;
		}
		fprintf(out,"#%s",s);
	}
	fclose(fp);
	fclose(out);

	unlink(path);
	if(link(swap,path)<0) perror("swap");
	unlink(swap);
}

/*
	add label in conf file and return result

	ex) addlabel("./my.conf","B=b");
	-----------------------------------
    $ cat my.conf
	A=a

	$ cat my.conf
	A=a
	B=b
*/
int
addlabel(char *path,char* label)
{
    FILE *fp;

	if((fp=fopen(path,"a"))==NULL) return -1;
	fprintf(fp,"%s\n",label);
	fclose(fp);
}

/*
	modify value in conf file with label and return result

	ex) modconf("./my.conf","A","b","A is b not a");

	-----------------------------------
    $ cat my.conf
	A=a

	$ cat my.conf (after)
	A=b 	# A is b not a
*/
int
modconf(char* path,char* label,char* val,char* comment)
{
    char swap[BUFSIZ],s[BUFSIZ],buf[BUFSIZ],*p,l[BUFSIZ],v[BUFSIZ];
    FILE *fp,*out;
	int found=0;

    sprintf(swap,"%s.swap",path);
    if((fp=fopen(path,"r"))==NULL) {
		fp = fopen(path,"w");
	}
    if((out=fopen(swap,"r"))!=NULL) { 
		fclose(out); return -1; 
	}
    out=fopen(swap,"w");

    while(1) {
        bzero(s,BUFSIZ);
        if(fgets(s,BUFSIZ,fp)==NULL) break;
		fflush(fp);
		if((p=strchr(s,'='))!=NULL) *p=' ';
		bzero(l,BUFSIZ); bzero(v,BUFSIZ);
		sscanf(s,"%s%s",l,v);
        if(!strcmp(label,l)) {
            sprintf(buf,"%s = %s\t\t%c %s\n",
                label,val,
                (comment!=NULL)? '#':' ',
                (comment!=NULL)? comment :" "
            );
			fputs(buf,out);
			found=1;
			continue;
		}
		else {
			if(p!=NULL) *p='=';
		}
		fputs(s,out);
	}
	if(!found) {
		sprintf(buf,"%s = %s\t\t%c %s\n",
			label,val,
			(comment!=NULL)? '#':' ',
			(comment!=NULL)? comment :" "
		);
		fputs(buf,out);
	}
	fclose(fp);
	fclose(out);
	unlink(path);
	if(link(swap,path)<0) perror("swap");
	unlink(swap);

	return found;
}

