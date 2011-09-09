#ifndef PROC_H
#define PROC_H

#ifdef __cplusplus
extern "C" {
#endif

extern int background();
extern int exeproc(char* p,char* s,char* tag,char* envp[]);
extern int lockpid(char *path,char *f,int signo);
extern int unlockpid(char *path,char *f);

#ifdef __cplusplus
}
#endif

#endif
