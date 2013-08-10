/**
	Include header
*/

/* SYS HEADER */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* LIB HEADER */
#include "commdef.h"
#include "loglib.h"
/* PRO HEADER */
#include "path.h"
#include "mmcdef.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "para_load.h"

/**
	Declare variables
*/

PARA_TBL	        	para_tbl[MAX_PARA_SYMBOL];
extern int				num_para_tbl;		/*< mmcd_main.h */
extern int				ptr_JCstr;			/*< mmcd_main.h */
extern char				JCstr_buff[];		/*< mmcd_main.h */

/**
 *	Implement func.
 */
void itom(int idx, char *res)
{
    strcpy(res, para_tbl[idx].para_name);
}


int para_cmp_sort(const void *a, const void *b)
{
	return strcmp(((PARA_TBL *)a)->para_name, ((PARA_TBL *)b)->para_name);
}


int para_cmp(const void *a, const void *b)
{

	return strcmp((char *)a, ((PARA_TBL *)b)->para_name);
}

PARA_TBL *get_para_tbl(char *str)
{
	char	upstr[256];
	int		i = 0;

	while (*str != 0x00)
		upstr[i++] = toupper(*str++);
	upstr[i] = 0;

	return (PARA_TBL *)bsearch(upstr, para_tbl, num_para_tbl, sizeof(PARA_TBL), para_cmp);
}


int get_para_idx(char *str)
{
	PARA_TBL *b;
	char	upstr[256];
	int		i = 0;

	while (*str != 0x00)
		upstr[i++] = toupper(*str++);
	upstr[i] = 0;

	if ((b = (PARA_TBL *)bsearch(upstr, para_tbl, num_para_tbl, sizeof(PARA_TBL), para_cmp)) == NULL)
		return (-1);
	else
		return b->code;
}

int load_para_tbl(void)
{
	int		i, j, dCode;
	char	sStrPara[BUF_SIZE], sIndexPara[256];
	FILE	*fp;

	if( (fp = fopen(ENUM_LIST_PATH, "r")) == NULL)
	{
		log_print(LOGN_CRI,"%s NOT FOUND, errno=%d\n", ENUM_LIST_PATH, errno);
		return 0;
	}

	/* Para_TBL 초기화 */
	num_para_tbl	= 0;
	ptr_JCstr		= 0;

	while(fgets(sStrPara, BUF_SIZE, fp) != NULL)
	{
		if(isupper(sStrPara[0]) && (sscanf(sStrPara, "%s %d", sIndexPara, &dCode) == 2))
		{
			/*	파라메터 코드 수록		*/
			para_tbl[num_para_tbl].code = dCode;

			/*	파라메터 이름 수록		*/
			para_tbl[num_para_tbl].para_name = &JCstr_buff[ptr_JCstr];
			strcpy(&JCstr_buff[ptr_JCstr], sIndexPara);
			ptr_JCstr += (strlen(&JCstr_buff[ptr_JCstr]) + 1);

			/*	파라메터 설명 수록		*/
			para_tbl[num_para_tbl].para_help = &JCstr_buff[ptr_JCstr];
			for(i = strlen(sIndexPara); i < BUF_SIZE; i++)
			{
				/*	HELP 스트링 발견	*/
				if(sStrPara[i] == '\"')
				{
					for(j = 0, i++; i < BUF_SIZE; i++, j++)
					{
						/*	HELP 스트링 종료	*/
						if (sStrPara[i] == '\"' || sStrPara[i] == 0)
						{
							JCstr_buff[ptr_JCstr + j] = 0;
							break;
						}
						else
							JCstr_buff[ptr_JCstr + j] = sStrPara[i];
					}
					ptr_JCstr += (j + 1);
					break;
				}
				else if (sStrPara[i] == 0)
					break;
			}
			num_para_tbl++;
		}
	}
	fclose(fp);
	qsort((void *)para_tbl, num_para_tbl, sizeof(PARA_TBL), para_cmp_sort);

	return 1;
}
