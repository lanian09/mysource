/**
	@file		cmd_load.h
	@author
	@version
	@date		2011-07-14
	@brief		cmd_load.c �������
*/

#ifndef __CMD_LOAD_H__
#define __CMD_LOAD_H__

/**
	Include headers
*/
#include <stdio.h>
#include "para_load.h"

#define MAX_LIB_TABLE		    	10000

typedef struct {
	int	mcode;
	int	block;
	void (*mmc_res) (char *, void *, short *, short * );
	char *msg_header;	/* MMC	RESULT�� HEAD PRINT�� */
	char *com_name;
} LIB_TBL;

typedef struct {
	int			code;
	char		*para_name;
} tENUMLIST;

typedef struct {
	int			idx;
	int			type;
	long long	def_value;
	long long	start;
	long long	end;
	char		*para_name;
	char		*def_name;
} PARA_CONT;

typedef struct {
#define MAX_PAR_IN_COM			24
	char	*com_name;
	char	*help[3];		/* HELP string */
	int		code;			/* Command index */
	int		dest;			/* Block Name */
	char	access;		 /* ���� �ڵ� : ����� ��� */
	char	p_man;			/* �ʼ� �Ķ���� ���� */
	char	p_opt;			/* ������� ����; num_para - p_man */
	char	num_para;		/* �Ķ���� ��ü ���� */
	PARA_CONT para[MAX_PAR_IN_COM];
} COM_TBL;

/**
	Declare functions
*/
extern long long convert_int64( char *p );
extern int cmd_cmp(const void *a, const void *b);
extern int cmd_cmp_sort(const void *a, const void *b);
extern int is_valid_type(char *str);
extern char *get_type_str(int idx);
extern void set_istr(char *str);
extern char *get_string(char *str);
extern int get_digit_int();
extern long long get_digit_int64();
extern long long convert_int64( char *p );
extern int load_def_enumlist(FILE *fp, char *istr);
extern int set_enum_para(long long *start, long long *end);
extern int load_cmd_tbl(FILE *fp);
extern COM_TBL *get_cmd_tbl(char *str);

#endif	/* __CMD_LOAD_H__ */
