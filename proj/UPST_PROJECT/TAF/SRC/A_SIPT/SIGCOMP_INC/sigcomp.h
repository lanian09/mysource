#ifndef _SIGCOMP_H_
#define _SIGCOMP_H_


int comp_sig(int cid, char *in, int ilen, char *out);
int decomp_sig(char *in, int ilen, char *out, int *olen);
int decomp_confirm(int lid, int cid);
int sigcomp_release(int cid, int flag);
int sigcomp_type(char *buf);

#endif
