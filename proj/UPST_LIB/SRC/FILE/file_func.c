/**A.1*  File Inclusion *******************************************************/                       
#include <stdio.h>     /* memset */
#include <string.h>    /* memset */
#include <stdlib.h>    /* memset */
#include <errno.h>     /* errno  */
#include <unistd.h>		/* lseek() */
#include <sys/types.h>	/* ssize_t */
#include <fcntl.h>		/* open(), O_RDWR, O_CREAT */

#include "filelib.h"

/**B.1* DEFINITION OF NEW CONSTANTS *******************************************/                       
/**B.2* DEFINITION OF NEW TYPE ************************************************/                       
/**C.1* DECLARATION OF VARIABLES **********************************************/                       
/**C.2* DECLARATION OF FUNCTIONS **********************************************/

//=============================================================================//

//=============================================================================//
int write_file(char *szFilePath, char *data_ptr, int write_len, int write_idx)
{
	int     fd;
	ssize_t ret;
	
	if( (fd = open(szFilePath, O_WRONLY|O_CREAT, F_PERM)) < 0 ){
		return E_FILE_OPEN;
	}

	if( write_idx ){
		lseek(fd, write_idx, SEEK_SET);
	}

	if( (ret = write( fd, (const void*)data_ptr, write_len )) < 0 ){
		close(fd);
		return E_FILE_WRITE;
	}

	if( ret != write_len ){
		close(fd);
		return E_FILE_PARTIAL;
	}

	close(fd);
	return FILE_SUCCESS;
}

int read_file(char *szFilePath, char *data_ptr, int read_len, int read_idx)
{
	int		fd;
	ssize_t ret;

	if( (fd = open(szFilePath, O_RDONLY|O_CREAT,F_PERM)) < 0 ){
		return E_FILE_OPEN;
	}

	if( read_idx ){
		lseek(fd, read_idx, SEEK_SET);
	}

	if( (ret = read(fd, (void*)data_ptr, read_len)) < 0 ){
		close(fd);
		return E_FILE_READ;
	}else if( ret != read_len ){
		close(fd);
		return E_FILE_PARTIAL;
	}

	close(fd);
	return FILE_SUCCESS;
}

int get_ip_conf(char *szFilePath, char *primary, char *secondary)
{
	FILE *fp;
	int   cnt = 0, d1,d2,d3,d4;
	char  szBuf[BUFLEN];

	primary[0]   = '\0';
	secondary[0] = '\0';	

	if( (fp = fopen( szFilePath, "r" )) == NULL ){
		return E_FILE_GENERIC;
	}

	while( fgets( szBuf, BUFLEN, fp ) != NULL ){
		if( szBuf[0] == '#' || szBuf[0] == '/' ){
			continue;
		}

		szBuf[strlen(szBuf)-1] = '\0';
		if( sscanf(szBuf, "%d.%d.%d.%d", &d1, &d2, &d3, &d4 ) != 4 ){
			continue;
		}
		if( d1 < 0 || d1 > 255 || d2 < 0 || d2 > 255 || 
			d3 < 0 || d3 > 255 || d4 < 0 || d4 > 255 ){
			continue;
		}

		if( cnt == 0 ){
			sprintf( primary, "%s", szBuf );
		}else if ( cnt == 1 ){
			sprintf( secondary, "%s", szBuf );
		}else{
			break;
		}

		cnt++;
	}

	fclose(fp);

	if( primary[0] == '\0' ){
		return E_FILE_NIL;
	}
	return FILE_SUCCESS;
}

int get_db_conf(char *szFilePath, char *szIP, char *szName, char *szPass, char *szAlias) 
{
	int     dScanCount, len, lcnt = 0;
	char    szBuf[BUFLEN], szIPVal[17], szNameVal[33], szPassVal[33], szAliasVal[33];
	FILE    *fp;

	if( (fp = fopen(szFilePath, "r")) == NULL)
	{
		return E_FILE_GENERIC;
	}

	while(fgets(szBuf, BUFLEN, fp) != NULL)
	{
		/*  from Source to Target: sscanf   */
		if(szBuf[0] != '#')
		{
			fclose(fp);
			return E_FILE_TYPE;
		}
		else if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@')
		{
			lcnt++;
			if( (dScanCount = sscanf(&szBuf[2], "%s %s %s %s", szIPVal, szNameVal, szPassVal, szAliasVal)) == 4 ){
				len = strlen(szIPVal);
				strncpy( szIP, szIPVal, len );
				szIP[ len ] = 0x00;

				len = strlen(szNameVal);
				strncpy( szName, szNameVal, len );
				szName[ len ] = 0x00;

				len = strlen(szPassVal);
				strncpy( szPass, szPassVal, len );
				szPass[ len ] = 0x00;

				if ( strstr( szAliasVal, "-" ) != NULL ){
					szAlias = NULL;
				} else {
					len = strlen(szAliasVal);
					strncpy( szAlias, szAliasVal, len );
					szAlias[ len ] = 0x00;
				}
			}
		}
	}

	fclose(fp);
	return lcnt;
}
