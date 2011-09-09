#ifndef __PCIV_LEX_H__
#define __PCIV_LEX_H__

extern int pciv_header(char *sp, int slen, int *version);
extern int pciv_parse(char *sp, int slen, int *cmd, int *last);

#endif /* __PCIV_LEX_H__ */
