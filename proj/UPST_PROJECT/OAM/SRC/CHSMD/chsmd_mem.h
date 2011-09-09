#ifndef __CHSMD_MEM_H__
#define __CHSMD_MEM_H__

typedef struct _st_mem_state
{
    int real_act;
    int real_tot;
    int virtual_act;
    int virtual_tot;
    int free;
    double use;
} st_mem_state;

extern int mem_compute(st_mem_state *mem);

#endif /* __CHSMD_MEM_H__ */
