/**
	@file		cmd_load.c
	@author
	@version
	@date		2011-07-14
	@brief		cmd_load.c
*/

/**
 *	Include headers
 */
/* SYS HEADER */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* LIB HEADER */
#include "loglib.h"		/*< 로그 라이브러리 */
/* PRO HEADER */
#include "mmcdef.h"		/*< 구조체, 상수 정의 */
#include "config.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "cmd_load.h"

/**
	Define constants
*/
#define MAX_ISTR_LEN				2048
#define MAX_ENUM_TBL				32

/**
 *	Define structures
 */
typedef struct {
    char    enum_name[16];
    int     start;
    int     end;
} ENUM_TBL;

/**
 *	Declare variables
 */
static 	int	cmds_ptr;
static  int	num_enum_tbl;
static	char cmds_str[MAX_ISTR_LEN], cmds_rtr_str[128];
static	ENUM_TBL enum_tbl[MAX_ENUM_TBL];	
static	char *var_type_str[] = {
		"DECSTR", 	"DECIMAL", 	"STRING",
		"ENUM", 	"HEXA", 	"HEXSTR",
		"HTIME", 	"YTIME", 	"NONABS",
		"EPOS", "LONGDECIMAL", "MTIME"};

extern tENUMLIST	enumlist[];		/*< help_proc.h */
extern int			num_enumlist;	/*< mmcd_main.h */
extern int			num_cmd_tbl;	/*< mmcd_main.h */
extern char			JCstr_buff[];	/*< mmcd_main.h */
extern int			ptr_JCstr;		/*< mmcd_main.h */
extern LIB_TBL		lib_tbl[];		/*< lib_load.h */
extern COM_TBL		cmd_tbl[];		/*< help_proc.h */

/**
 *	Declare extern func.
 */
extern PARA_TBL *get_para_tbl(char *str);
extern int set_lib_entry(int idx, char *func_name);

/**
 *	Implement func.
 */
int cmd_cmp(const void *a, const void *b)
{
	return strcmp((char *)a, ((COM_TBL *)b)->com_name);
}

int cmd_cmp_sort(const void *a, const void *b)
{
	return strcmp(((COM_TBL *)a)->com_name, ((COM_TBL *)b)->com_name);
}

int is_valid_type(char *str)
{
	int i;

	for (i = 0;i < VT_NUM_TYPE;i++)
		if (strcmp(var_type_str[i], str) == 0)
			return i;
	return eINVALID_PAPA_CONTENT;
}

char *get_type_str(int idx)
{
	if (idx < 0 || idx > VT_MTIME )
		return "UNKNOWN TYPE CODE";
	else
		return var_type_str[idx];
}

void set_istr(char *str)
{
	strcpy(cmds_str, str);
	cmds_ptr = 0;
}

char *get_string(char *str)
{
	int i, k;
	char *cmds_rtr_strb;

	if (str == NULL)
	{
		cmds_rtr_strb = cmds_rtr_str;
	}
	else
	{
		cmds_rtr_strb = str;
	}

	for (i = cmds_ptr;i < MAX_ISTR_LEN;i++)
	{
		if (cmds_str[i] == 0) 
			return NULL;
		else if (strchr(",={}?", cmds_str[i]) != NULL) 
		{
			cmds_rtr_strb[0] = cmds_str[i];
			cmds_rtr_strb[1] = 0;
			cmds_ptr = i + 1;
			return cmds_rtr_strb;
		} 
		else if (cmds_str[i] > ' ') 
			break;
	}


	for (k = 0;i < MAX_ISTR_LEN;k++, i++) 
	{
		if (isalnum(cmds_str[i]) || cmds_str[i] == '-' || cmds_str[i] == '_' || 
			cmds_str[i] == '.'||cmds_str[i] == '|' || cmds_str[i] == '/' || cmds_str[i] == '#' || cmds_str[i] == '%' || cmds_str[i] == '@' ||
			cmds_str[i] == '~' || cmds_str[i] == '$' || cmds_str[i] == '&' || cmds_str[i] == '*' || cmds_str[i] == '!' ||
			cmds_str[i] == '(' || cmds_str[i] == ')' || cmds_str[i] == '+' || cmds_str[i] == '[' || cmds_str[i] == ']' ||
			cmds_str[i] == ':' || cmds_str[i] == ';' || cmds_str[i] == '<' || cmds_str[i] == '>' || cmds_str[i] == '`' ||
			cmds_str[i] == '^' )
			cmds_rtr_strb[k] = cmds_str[i];

		else if (strchr(",={}?", cmds_str[i]) != NULL) 
		{
			cmds_rtr_strb[k] = 0;
			cmds_ptr = i;
			return cmds_rtr_strb;

		} 
		else if (cmds_str[i] == '\"') 
		{
			if (k)
				return NULL;
			for (i++;i < MAX_ISTR_LEN;i++, k++)
			{
				if (cmds_str[i] == '\"' || cmds_str[i] == 0) 
				{
					cmds_rtr_strb[k] = 0;
					cmds_ptr = i + 1;
					return cmds_rtr_strb;
				} 
				else
				{
					cmds_rtr_strb[k] = cmds_str[i];
				}
			}

			return NULL;

		} else if (cmds_str[i] > ' ')
			return NULL;
		else 
			break;
	}

	cmds_rtr_strb[k] = 0;
	cmds_ptr = i + 1;

	return cmds_rtr_strb;
}

int get_digit_int()
{
	int i, k;

	for (i = cmds_ptr;i < MAX_ISTR_LEN;i++)
	{
		if (cmds_str[i] > ' ') 
			break;
		else if (cmds_str[i] == 0) 
			return (-1);
	}

	for (k = 0;i < MAX_ISTR_LEN;k++, i++)
	{
		if (isdigit(cmds_str[i]) || cmds_str[i] == '-')
			cmds_rtr_str[k] = cmds_str[i];
		else if (cmds_str[i] > ' ') 
		{
			return (-1);
		} 	
		else 
			break;
	}


	cmds_rtr_str[k] = 0;
	cmds_ptr = i + 1;

	return atoi(cmds_rtr_str);
}

long long get_digit_int64()
{
	int i, k;

	for (i = cmds_ptr;i < MAX_ISTR_LEN;i++)
	{
		if (cmds_str[i] > ' ') 
			break;
		else if (cmds_str[i] == 0) 
			return (-1);
	}


	for (k = 0;i < MAX_ISTR_LEN;k++, i++)
	{
		if (isdigit(cmds_str[i]) || cmds_str[i] == '-')
			cmds_rtr_str[k] = cmds_str[i];
		else if (cmds_str[i] > ' ') {
			return (-1);
		} else 
			break;
	}


	cmds_rtr_str[k] = 0;
	cmds_ptr = i + 1;

	return convert_int64( cmds_rtr_str );
}


long long convert_int64( char *p )
{
	long long t;
	
	int i, j;
	t = 0;

	j = strlen(p);

	for( i=0 ; i<j ; i++ )
	{
		t = t*10 + *p - '0';
		p++;
	}

	return t;
}

int load_def_enumlist(FILE *fp, char *istr)
{
	char	*sp;
	PARA_TBL *para_tbl;

	num_enumlist = 0;
	num_enum_tbl = 0;

	while (fgets(istr, MAX_ISTR_LEN, fp) != NULL && num_enum_tbl < MAX_ENUM_TBL - 1) 
	{
		if (islower(istr[0])) 
		{  
			return 0;
		}

		if (istr[0] != '#' || istr[1] != '@' || istr[2] != 'E')
            continue;

 		set_istr(&istr[3]);

        if ((sp = get_string(NULL)) == NULL) 
            return eENUM_LIST_FORMAT;

		strcpy(enum_tbl[num_enum_tbl].enum_name, sp);
		enum_tbl[num_enum_tbl].enum_name[15] = 0;
		enum_tbl[num_enum_tbl].start = num_enumlist;

		if ((sp = get_string(NULL)) == NULL) 
            return eENUM_LIST_FORMAT;
		if (*sp != '=')
            return eENUM_LIST_FORMAT;

		if ((sp = get_string(NULL)) == NULL) 
            return eENUM_LIST_FORMAT;
		if (*sp != '{')
            return eENUM_LIST_FORMAT;

		while ((sp = get_string(NULL)) != NULL) 
		{
			if (*sp == '}') 
			{
				enum_tbl[num_enum_tbl].end = num_enumlist;
				num_enum_tbl++;
				break;
			}

			if ((para_tbl = get_para_tbl(sp)) == NULL) 
			{
                log_print(LOGN_CRI,"[ERROR] ENUM NOT FOUND: %s", sp);
                return eENUM_LIST_ELEMENT;
            }
		
			enumlist[num_enumlist].code = para_tbl->code;
			enumlist[num_enumlist].para_name = para_tbl->para_name;
			num_enumlist++;
		}
	}
	
	return 0;
}

int set_enum_para(long long *start, long long *end)
{
	char *sp;
	int	i;
	PARA_TBL *para_tbl;

	if ((sp = get_string(NULL)) == NULL)
		return eENUM_LIST_FORMAT;

	if (*sp == '{') 
	{
		*start = num_enumlist;
		while ((sp = get_string(NULL)) != NULL) 
		{
			if (*sp == '}') 
			{
				*end = num_enumlist;
				return 0;
			}

			if ((para_tbl = get_para_tbl(sp)) == NULL) 
			{
                log_print(LOGN_CRI, "[ERROR] ENUM NOT FOUND: %s", sp);
                return eENUM_LIST_ELEMENT;
            }

			enumlist[num_enumlist].code = para_tbl->code;
			enumlist[num_enumlist].para_name = para_tbl->para_name;
			num_enumlist++;
		}
	} 
	else 
	{
		for (i = 0;i < num_enum_tbl;i++)
		{
			if (strcmp(sp, enum_tbl[i].enum_name) == 0) 
			{
				*start = enum_tbl[i].start;
				*end   = enum_tbl[i].end;
				return 0;
			}
		}
	}
	log_print(LOGN_CRI, "[ERROR] ENUM LIST NOT FOUND %s", sp);

	return eENUM_LIST_ELEMENT;
}


int load_cmd_tbl(FILE *fp)
{
	int     i, k;
	int		readreq;
    char    istr[MAX_ISTR_LEN];
	char	*sp;
	PARA_TBL *para_tbl;

	if ((i = load_def_enumlist(fp, istr)) < 0)
		return i;
	readreq = 1;

    num_cmd_tbl = 0;

	while (1) 
	{
		if (readreq == 0)
			if (fgets(istr, MAX_ISTR_LEN, fp) == NULL) 
				break;

		set_istr(istr);

		log_print(LOGN_DEBUG, "DEBUG: %s", istr);

		readreq = 0;

		if ((sp = get_string(NULL)) == NULL) 
			continue;

		i = 0;
		cmd_tbl[num_cmd_tbl].com_name = &JCstr_buff[ptr_JCstr];

		while (*sp) 
		{
			JCstr_buff[ptr_JCstr + i] = toupper(*sp++);
			i++;
		}

		JCstr_buff[ptr_JCstr + i] = 0;
		ptr_JCstr += (i + 1);

		if ((cmd_tbl[num_cmd_tbl].code = get_digit_int()) < 0) {
            log_print(LOGN_CRI, "INVALID CMD1 : [%d] ",
                             cmd_tbl[num_cmd_tbl].code );
            return eINVALID_CMD;
        }
		log_print(LOGN_INFO, "num_cmd_tbl[%d] cmd_tbl_mcode[%d]",
                    num_cmd_tbl, cmd_tbl[num_cmd_tbl].code );
		if (cmd_tbl[num_cmd_tbl].code >= MAX_LIB_TABLE)
            continue;

		if ((sp = get_string(NULL)) == NULL) {
            log_print(LOGN_CRI, "INVALID CMD2 : [%s] ",
                             sp );
            return eINVALID_CMD;
        }

		if (strcmp("MP", sp) != 0) return eINVALID_SUBSYSTEM;

		if ((sp = get_string(NULL)) == NULL) {
            log_print(LOGN_CRI, "INVALID CMD2 : [%s] ",
                             sp );
            return eINVALID_CMD;
        }
		
		if ((para_tbl = get_para_tbl(sp)) == NULL) 
		{
			log_print(LOGN_CRI, "[ERROR] NOT FOUND BLOCK ID: sp=[%s]%s", sp, &cmds_str[cmds_ptr]);
			return eINVALID_CMD;
		}
		cmd_tbl[num_cmd_tbl].dest = para_tbl->code;	


		if ((cmd_tbl[num_cmd_tbl].access = get_digit_int()) < 0) 
			return eINVALID_PAPA_IN_CMD;
		if ((cmd_tbl[num_cmd_tbl].num_para = get_digit_int()) < 0) 
			return eINVALID_PAPA_IN_CMD;
		if ((cmd_tbl[num_cmd_tbl].p_man = get_digit_int()) < 0) 
			return eINVALID_PAPA_IN_CMD;
		if (cmd_tbl[num_cmd_tbl].num_para >= MAX_PAR_IN_COM) 
			return eINVALID_PAPA_IN_CMD;

		for (i = 0;i < cmd_tbl[num_cmd_tbl].num_para;i++) 
		{
			if ((sp = get_string(NULL)) == NULL) 
				return eINVALID_PAPA_CONTENT;
			if ((para_tbl = get_para_tbl(sp)) == NULL) 
			{
				log_print(LOGN_CRI,"[ERROR] NOT FOUND: sp=[%s]%s I=%d TT()%s\n", 
								sp, &cmds_str[cmds_ptr], i, cmds_str);
				log_print(LOGN_CRI,"[ERROR] NOT FOUND: sp=[%s]%s I=%d TT()%s", 
								sp, &cmds_str[cmds_ptr], i, cmds_str);
				return eINVALID_PAPA_CONTENT;
			}
			cmd_tbl[num_cmd_tbl].para[i].idx = para_tbl->code;
			cmd_tbl[num_cmd_tbl].para[i].para_name = para_tbl->para_name;

			if ((sp = get_string(NULL)) == NULL) 
				return eINVALID_PAPA_CONTENT;

			if ((cmd_tbl[num_cmd_tbl].para[i].type = is_valid_type(sp)) < 0)
				return cmd_tbl[num_cmd_tbl].para[i].type; 

			if (cmd_tbl[num_cmd_tbl].para[i].type == VT_ENUM 
			|| cmd_tbl[num_cmd_tbl].para[i].type == VT_EPOS) 
			{
				if ((sp = get_string(NULL)) == NULL) 
					return eINVALID_PAPA_IN_CMD;
				if ((para_tbl = get_para_tbl(sp)) == NULL) 
				{
                	log_print(LOGN_CRI,"[ERROR] ENUM NOT FOUND: %s", sp);
                	return eENUM_LIST_ELEMENT;
				}
				cmd_tbl[num_cmd_tbl].para[i].def_value = para_tbl->code;
				cmd_tbl[num_cmd_tbl].para[i].def_name  = para_tbl->para_name;
				
				if ((k = set_enum_para(&cmd_tbl[num_cmd_tbl].para[i].start, 
				&cmd_tbl[num_cmd_tbl].para[i].end)) < 0)
					return k;

			} 
			else 
			{
				if ((sp = get_string(NULL)) == NULL) 
					return eINVALID_PAPA_IN_CMD;
				cmd_tbl[num_cmd_tbl].para[i].def_value = atol(sp);

				cmd_tbl[num_cmd_tbl].para[i].start = get_digit_int64(); 
				cmd_tbl[num_cmd_tbl].para[i].end   = get_digit_int64();
			}
		}
		

		/* #@D,1,2,3 확장 영역 */
		if (fgets(istr, MAX_ISTR_LEN, fp) == NULL) 
			break;
		
		/* 1번째 라인의 MAGIC CODE */
		readreq = 1;
		if (istr[0] != '#' || istr[1] != '@' || istr[2] != 'D')
			continue;

		set_istr(&istr[3]);

		/* PRN function loading */
		if ((sp = get_string(NULL)) == NULL) 
			return eNOT_FOUND_PRN_FUNC;
		if (set_lib_entry(cmd_tbl[num_cmd_tbl].code, sp) <= 0) 
			return eNOT_FOUND_PRN_FUNC;

		/* 메시지 코드 기록 */
		if ((sp = get_string(NULL)) == NULL || *sp != 'M') 
			return eMANDATORY_MISSED;
		else
			lib_tbl[cmd_tbl[num_cmd_tbl].code].mcode = atoi(sp + 1);

		lib_tbl[cmd_tbl[num_cmd_tbl].code].com_name = cmd_tbl[num_cmd_tbl].com_name;
		lib_tbl[cmd_tbl[num_cmd_tbl].code].block = cmd_tbl[num_cmd_tbl].dest;

		/* MMC HEADER 수록 */
		if ((sp = get_string(NULL)) == NULL) 
			return eMANDATORY_MISSED;
		lib_tbl[cmd_tbl[num_cmd_tbl].code].msg_header = &JCstr_buff[ptr_JCstr];
		strcpy(&JCstr_buff[ptr_JCstr], sp);
		ptr_JCstr += (strlen(&JCstr_buff[ptr_JCstr]) + 1);

		/* Option Value 초기화 */
		cmd_tbl[num_cmd_tbl].help[0] = NULL;
		cmd_tbl[num_cmd_tbl].help[1] = NULL;
		cmd_tbl[num_cmd_tbl].help[2] = NULL;

		num_cmd_tbl++;

		for (i = 0;i < 3;i++) 
		{
			if (fgets(istr, MAX_ISTR_LEN, fp) == NULL) 
				break;
			if (istr[0] == '#' && istr[1] == '@' && istr[2] == '1' + i) 
			{
				set_istr(&istr[3]);
				sp = get_string(NULL);
				cmd_tbl[num_cmd_tbl - 1].help[i] = &JCstr_buff[ptr_JCstr];
				strcpy(&JCstr_buff[ptr_JCstr], sp);
				ptr_JCstr += (strlen(&JCstr_buff[ptr_JCstr]) + 1);
				readreq = 0;
			} else 
				break;
		}
	}

	/* qsort */

	qsort((void *)cmd_tbl, num_cmd_tbl, sizeof(COM_TBL), cmd_cmp_sort);
	
	return 1;
}

COM_TBL *get_cmd_tbl(char *str)
{
	char	upstr[256];
    int     i = 0;
    
    while (*str != 0x00) 
        upstr[i++] = toupper(*str++);
    upstr[i] = 0;
        
    return (COM_TBL *)bsearch(upstr, cmd_tbl, num_cmd_tbl, sizeof(COM_TBL), cmd_cmp);
}
