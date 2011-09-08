#include "mmcd_proto.h"

extern char		trcBuf[4096], trcTmp[1024];


//------------------------------------------------------------------------------
// mml_commands 파일을 읽어 command_table과 command_help_table을 구성한다.
// - 읽기가 끝난 후 bsearch를 위해 sorting한다. (오름차순으로 정렬한다.)
// - GUI에서 명령어 리스트와 syntax를 인식할 수 있도록 구성된 table을 DB에 넣는다.
//------------------------------------------------------------------------------
int mmcd_loadCmdTbl (MMLCmdContext *cmdTbl, MMLHelpContext *helpTbl, char *errBuf)
{
	int		lNum,state,ret;
	int		cmdIndex,paraIndex;
	FILE	*fp;
	char	*env,fname[256*4],lineBuf[2048],token[8][50];//category size 32->50, lineBuf size 1024->2048

	if ((env = getenv(IV_HOME)) == NULL) {
		sprintf(errBuf,"[mmcd_loadCmdTbl] not found %s environment name\n", IV_HOME);
		return -1;
	}
	sprintf(fname,"%s/%s", env, MML_COMMANDS_FILE);

	if ((fp = fopen(fname,"r")) == NULL) {
		sprintf(errBuf,"[mmcd_loadCmdTbl] fopen fail[%s]; err=%d(%s)n", fname, errno, strerror(errno));
		return -1;
	}

	lNum = cmdIndex = 0;
	state = MML_SET_CMD_NEXT;

	while (fgets(lineBuf, sizeof(lineBuf), fp) != NULL)
	{
		lNum++;
		if ((lineBuf[0]=='#' || lineBuf[0]=='\n') && state != MML_SET_CMD_HELP)
			continue;

		memset(token, 0x00, sizeof(token));
		switch (state) {
			case MML_SET_CMD_NEXT: // 다음 command를 읽어야 할 차례

#if 0 /* jhnoh : 030812 */
				if ((ret = sscanf(lineBuf,"%s%s%s %s%s%s", token[0], token[1], token[2],
								token[3], token[4], token[5])) != 6) {
					sprintf(errBuf,"[mmcd_loadCmdTbl] syntex error; line=%d\n", (lNum));
					fclose(fp); return -1;
				}
#else
				ret = sscanf(lineBuf,"%s%s%s%s%s%s%s", token[0], token[1], token[2], token[3], token[4], token[5], token[6]);
				if (!((ret==6) || (ret==7))){
					sprintf(errBuf,"[mmcd_loadCmdTbl] syntex error; line=%d\n", (lNum));
					fclose(fp); 
					return -1;
				}
#endif

				if (strcasecmp(token[0],"COMMAND")) {
					sprintf(errBuf,"[mmcd_loadCmdTbl] syntex error; line=%d\n", (lNum));
					fclose(fp); return -1;
				}
				strcpy (cmdTbl[cmdIndex].cmdName, token[1]);
				strcpy (helpTbl[cmdIndex].cmdName, token[1]);
				strcpy (cmdTbl[cmdIndex].dstSysName, token[2]);
				strcpy (cmdTbl[cmdIndex].dstAppName, token[3]);
				if ((cmdTbl[cmdIndex].privilege = mmcd_setCmdClass2CmdTbl(token[4])) < 0) {
					sprintf(errBuf,"[1.mmcd_loadCmdTbl] syntex error; line=%d; unknown class,%s\n", (lNum),lineBuf);
					fclose(fp); return -1;
				}

				strcpy (cmdTbl[cmdIndex].category, token[5]);

	 			if (!(strcmp(token[6], "CONFIRM")))
					cmdTbl[cmdIndex].confirm = 'Y';
				else
					cmdTbl[cmdIndex].confirm = 'N';

				state = MML_SET_CMD_COMMAND;
				break;
 
			case MML_SET_CMD_COMMAND: // 파라미터들을 읽어야 할 차례
				paraIndex = 0;
				state = MML_SET_CMD_PARAMETER;

			case MML_SET_CMD_PARAMETER: // 파라미터들을 읽고 있는 중
				sscanf(lineBuf,"%s", token[0]);
				if (!strcasecmp(token[0],"@PARA")) { // 파라미터를 계속 읽고 있는 중
					if (mmcd_setParameter2CmdTbl (cmdTbl, cmdIndex, paraIndex, lineBuf) < 0) {
						sprintf(errBuf,"[2. mmcd_loadCmdTbl] syntex error; line=%d, %s\n", (lNum),lineBuf);
						fclose(fp); return -1;
					}
					paraIndex++;
				} else if (!strcasecmp(token[0],"@SLOGAN")) { // 파라미터를 모두 읽고, @SLOGAN을 읽을 차례
					cmdTbl[cmdIndex].paraCnt = paraIndex;
					if (mmcd_setSlogan2HelpTbl (helpTbl, cmdIndex, lineBuf) < 0) {
						sprintf(errBuf,"[3. mmcd_loadCmdTbl] syntex error; line=%d,%s\n", (lNum),lineBuf);
						fclose(fp); return -1;
					}
					state = MML_SET_CMD_SLOGAN;
				} else {
					sprintf(errBuf,"[4.mmcd_loadCmdTbl] syntex error; line=%d,%s\n", (lNum),lineBuf);
					fclose(fp); return -1;
				}
				break;
 
			case MML_SET_CMD_SLOGAN: // @HELP를 읽어야 할 차례
				sscanf(lineBuf,"%s", token[0]);
				if (strcasecmp(token[0],"@HELP")) {
					sprintf(errBuf,"[5.mmcd_loadCmdTbl] syntex error; line=%d,%s\n", (lNum),lineBuf);
					fclose(fp); return -1;
				}
				state = MML_SET_CMD_HELP;
				break;
 
			case MML_SET_CMD_HELP:
				sscanf(lineBuf,"%s", token[0]);
				if (strcasecmp(token[0],"@HELP")) { // help table에 계속 넣는다.
					strcat (helpTbl[cmdIndex].cmdHelp, lineBuf);
				} else { // @HELP를 다 읽었다.
					state = MML_SET_CMD_NEXT;
					cmdIndex++;
				}
				break;
		} //-- end of switch(state) --//

	} //-- end of while(fgets()) --//
	fclose(fp);


	// 명령어 이름에 따라 오름차순으로 정렬한다.
	//
	qsort ((void*)cmdTbl, cmdIndex, sizeof(MMLCmdContext), mmcd_qsortCmp); 
	qsort ((void*)helpTbl, cmdIndex, sizeof(MMLHelpContext), mmcd_qsortCmp); 

#if 0
	/* command mmc table */
	mmcd_dumpCmdTbl (cmdTbl, helpTbl);
#endif


	return cmdIndex;

} //----- End of mmcd_loadCmdTbl -----//




//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mmcd_setParameter2CmdTbl (MMLCmdContext *cmdTbl, int cmdIndex, int paraIndex, char* lineBuf)
{
	int		ret, enumIndex=0, secFlag, j, k;
	char	token[13][32],*ptr,*next,*enumItem, *enumPtr;

	memset(token, 0x00, sizeof(token));
#if 0 /* jhnoh : 030623 */
	if ((ret = sscanf(lineBuf,"%s%s%s%s%s%s",
				token[0],token[1],token[2],token[3],token[4],token[5])) < 6) {
		return -1;
	}
#else
	if ((ret = sscanf(lineBuf,"%s%s%s%s%s%s%s%s%s",
				token[0],token[1],token[2],token[3],token[4],token[5],token[6],token[7],token[8])) < 6) {
printf("mmcd_setParameter2CmdTbl-1111\n");
		return -1;
	}
#endif
	strcpy (cmdTbl[cmdIndex].paraInfo[paraIndex].paraName, token[1]);
	if ((cmdTbl[cmdIndex].paraInfo[paraIndex].mandFlag = mmcd_setParaOpt2CmdTbl(token[2])) < 0) return -1;
	if ((cmdTbl[cmdIndex].paraInfo[paraIndex].paraType = mmcd_setParaType2CmdTbl(token[3])) < 0) return -1;

	// ENUM type이 아니면 min/max가 있고, ENUM이면 enum_list가 (...)로 들어있다.
	if (cmdTbl[cmdIndex].paraInfo[paraIndex].paraType == MML_PTYPE_FIXSTR) {
		cmdTbl[cmdIndex].paraInfo[paraIndex].fixVal[0] = strtol(token[4],0,0);
		cmdTbl[cmdIndex].paraInfo[paraIndex].fixVal[1] = strtol(token[5],0,0);
		cmdTbl[cmdIndex].paraInfo[paraIndex].fixVal[2] = strtol(token[6],0,0);
		cmdTbl[cmdIndex].paraInfo[paraIndex].fixVal[3] = strtol(token[7],0,0);
		cmdTbl[cmdIndex].paraInfo[paraIndex].fixVal[4] = strtol(token[8],0,0);

		strcpy(cmdTbl[cmdIndex].paraInfo[paraIndex].enumList[0].enumStr, token[4]);
		strcpy(cmdTbl[cmdIndex].paraInfo[paraIndex].enumList[1].enumStr, token[5]);
		strcpy(cmdTbl[cmdIndex].paraInfo[paraIndex].enumList[2].enumStr, token[6]);
		strcpy(cmdTbl[cmdIndex].paraInfo[paraIndex].enumList[3].enumStr, token[7]);
		strcpy(cmdTbl[cmdIndex].paraInfo[paraIndex].enumList[4].enumStr, token[8]);
	
	}
	else if (cmdTbl[cmdIndex].paraInfo[paraIndex].paraType == MML_PTYPE_FIXDEC) {
		cmdTbl[cmdIndex].paraInfo[paraIndex].fixVal[0] = strtol(token[4],0,0);
		cmdTbl[cmdIndex].paraInfo[paraIndex].fixVal[1] = strtol(token[5],0,0);
		cmdTbl[cmdIndex].paraInfo[paraIndex].fixVal[2] = strtol(token[6],0,0);
		cmdTbl[cmdIndex].paraInfo[paraIndex].fixVal[3] = strtol(token[7],0,0);
		cmdTbl[cmdIndex].paraInfo[paraIndex].fixVal[4] = strtol(token[8],0,0);

		strcpy(cmdTbl[cmdIndex].paraInfo[paraIndex].enumList[0].enumStr, token[4]);
		strcpy(cmdTbl[cmdIndex].paraInfo[paraIndex].enumList[1].enumStr, token[5]);
		strcpy(cmdTbl[cmdIndex].paraInfo[paraIndex].enumList[2].enumStr, token[6]);
		strcpy(cmdTbl[cmdIndex].paraInfo[paraIndex].enumList[3].enumStr, token[7]);
		strcpy(cmdTbl[cmdIndex].paraInfo[paraIndex].enumList[4].enumStr, token[8]);
	
	}
	else if (cmdTbl[cmdIndex].paraInfo[paraIndex].paraType == MML_PTYPE_HEXA) {
		if ( ( token[4][0] == '0' && ( token[4][1] == 'x' || token[4][1] == 'X' ) ) ||
			 ( ( token[4][0] == 'H' || token[4][0] == 'h' ) && token[4][1] == '\'') ) {
			cmdTbl[cmdIndex].paraInfo[paraIndex].minVal = strlen(&token[4][2]);
		} else {
			cmdTbl[cmdIndex].paraInfo[paraIndex].minVal = strlen(token[4]);
		}
		if ( ( token[5][0] == '0' && ( token[5][1] == 'x' || token[5][1] == 'X' ) ) ||
			 ( ( token[5][0] == 'H' || token[5][0] == 'h' ) && token[5][1] == '\'') ) {
			cmdTbl[cmdIndex].paraInfo[paraIndex].maxVal = strlen(&token[5][2]);
		}
		else {
			cmdTbl[cmdIndex].paraInfo[paraIndex].maxVal = strlen(token[5]);
		}
	}
	else if (cmdTbl[cmdIndex].paraInfo[paraIndex].paraType != MML_PTYPE_ENUM) {
		cmdTbl[cmdIndex].paraInfo[paraIndex].minVal = strtol(token[4],0,0);
		cmdTbl[cmdIndex].paraInfo[paraIndex].maxVal = strtol(token[5],0,0);
	} else {
        // 맨끝에 있는 white-space를 지운다.
        for (ptr = &lineBuf[strlen(lineBuf)-1]; isspace(*ptr); ptr--) *ptr = '\n';

		// enum_list를 하나씩 읽는다.
		if ((next = strstr(lineBuf,"ENUM")) == NULL)
			return -1;
		next += 4; // next를 "ENUM" 뒤쪽으로 옮긴다. -> enum_list 앞쪽으로 옮긴다.
		while (next != NULL) {
			for (ptr=next; isspace(*ptr); ptr++) ; // remove white-space
			enumItem = (char*)strtok_r(ptr," \t",&next);
	
			/* find '=', 2003/12/17. by limsh */
			secFlag=0;
			enumPtr = (char *)cmdTbl[cmdIndex].paraInfo[paraIndex].enumList[enumIndex].enumStr;
			for(j=0, k=0; j<strlen(enumItem); j++) {
				/* second enum string */
				if( enumItem[j]=='=' ) {
					enumPtr = (char *)cmdTbl[cmdIndex].paraInfo[paraIndex].enumList[enumIndex].enumStr2;
					j++; k=0; secFlag++;
				}
				enumPtr[k++]=enumItem[j];
			}

			if (next == NULL) { // 마지막 enumItem 뒤쪽 '\n'을 지운다.
				if( secFlag ) {
					enumPtr = (char *)cmdTbl[cmdIndex].paraInfo[paraIndex].enumList[enumIndex].enumStr2;
					enumPtr[strlen(enumPtr)-1] = 0; 
				}
				else {
					enumPtr = (char *)cmdTbl[cmdIndex].paraInfo[paraIndex].enumList[enumIndex].enumStr;
					enumPtr[strlen(enumPtr)-1] = 0; 
				}
				break;
			}
			enumIndex++;
		}
	}

	return 1;

} //----- End of mmcd_setParameter2CmdTbl -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mmcd_setSlogan2HelpTbl (MMLHelpContext *helpTbl, int cmdIndex, char* lineBuf)
{
	char	*next;

	strtok_r(lineBuf," \t",&next); // skip "@SLOGAN"
	if (next == NULL) {
		return -1;
	}
	for (; isspace(*next); next++) ; // remove white-space
	next[strlen(next)-1] = 0;        // 맨끝에 있는 '\n'을 지운다.
	strcpy (helpTbl[cmdIndex].cmdSlogan, next);

	return 1;

} //----- End of mmcd_setSlogan2HelpTbl -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mmcd_setCmdClass2CmdTbl (char *class)
{
	if (!strcasecmp (class, MML_PRIVILEGE_SU_STR))
		return MML_PRIVILEGE_SU;
	else if (!strcasecmp (class, MML_PRIVILEGE_NU_STR))
		return MML_PRIVILEGE_NU;
	else if (!strcasecmp (class, MML_PRIVILEGE_GUEST_STR))
		return MML_PRIVILEGE_GUEST;
	else
		return -1;
} //----- End of mmcd_setCmdClass2CmdTbl -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mmcd_setParaOpt2CmdTbl (char *mand_or_opt)
{
	if (!strcasecmp(mand_or_opt,"MAND"))
		return 1;
	else if (!strcasecmp(mand_or_opt, "OPT2"))
		return 2;
	else if (!strcasecmp(mand_or_opt,"OPT"))
		return 0;
	else
		return -1;
} //----- End of mmcd_setParaOpt2CmdTbl -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mmcd_setParaType2CmdTbl (char *type)
{
	if (!strcasecmp(type,"DECIMAL"))
		return MML_PTYPE_DECIMAL;
	else if (!strcasecmp(type,"HEXA"))
		return MML_PTYPE_HEXA;
	else if (!strcasecmp(type,"STRING"))
		return MML_PTYPE_STRING;
	else if (!strcasecmp(type,"ENUM"))
		return MML_PTYPE_ENUM;
#if 1 /* jhnoh : 030815 */
	else if (!strcasecmp(type,"FIXSTR"))
		return MML_PTYPE_FIXSTR;
	else if (!strcasecmp(type,"DECSTR"))
		return MML_PTYPE_DECSTR;
	else if (!strcasecmp(type,"FIXDEC"))
		return MML_PTYPE_FIXDEC;
#endif
	else
		return -1;
} //----- End of mmcd_setParaType2CmdTbl -----//



#define MMCD_MYSQL_QUERY(conn,query)			\
do {											\
	if (mysql_query ((conn), (query)) != 0) {	\
		sprintf(trcBuf,">>> mysql_query fail; err=%s\n  query = %s\n", mysql_error(&sql), (query));	\
		trclib_writeLogErr (FL,trcBuf);			\
        fprintf(stderr, "\nERROR => %s",  trcBuf);         \
		mysql_close((conn));					\
		return -1;								\
	}											\
} while(0);

//------------------------------------------------------------------------------
// 이전에 들어있는 tuple들을 모두 delete하고 다시 insert한다.
//------------------------------------------------------------------------------
int mmcd_saveCmdInfo2DB (MMLCmdContext *cmdTbl, MMLHelpContext *helpTbl, int cmdCnt)
{
	int			i, j, k, m,find;
	char		query[12000], tmp[1024]; // original : query=>8192
	MYSQL		sql, *conn;
	char		tmpHelp[10000];

	mysql_init (&sql);

	if ((conn = mysql_real_connect (&sql, "localhost", "root", "mysql",
									MML_COMMAND_DB_NAME, 0, 0, 0)) == NULL) {
		sprintf(trcBuf,">>> mysql_real_connect fail; err=%s\n", mysql_error(&sql));
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
//printf("mmcd_saveCmdInfo2DB : %s\n", MML_COMMAND_DB_NAME);

	// 이전놈을 모두 지운다.
	//
	sprintf (query, "DELETE FROM %s", MML_COMMAND_DB_TABLE_NAME);
	MMCD_MYSQL_QUERY(conn,query);

	sprintf (query, "DELETE FROM %s", MML_PARAMETER_DB_TABLE_NAME);
	MMCD_MYSQL_QUERY(conn,query);

	commlib_microSleep(100000); // 100 MS

	// DB에 넣는다.
	//
	for (i=0; i<cmdCnt; i++)
	{
        if (i%50==0) commlib_microSleep(100000); // 10 MS

		// command table에 명령어 권한, 등급, 파라미터 갯수 등을 넣는다.
		//
		//yhshin
		//if ((!strncmp(cmdTbl[i].cmdName, "set-rule-sce", 12)) || (!strncmp(cmdTbl[i].cmdName, "SET-RULE-SCE", 12))) {
		//	return;
		//}

		//sjjeon : set-rule-sce만 DB Insert에서 제외한다.
		if(!strncasecmp(cmdTbl[i].cmdName, "set-rule-sce", 12)) continue;

#ifdef DEBUG_MODE
        fprintf(stderr, "\nSIZE = %d", strlen(helpTbl[i].cmdHelp));
		fprintf(stderr, "\nSTART: %s, %s, \n       %s, %s, %d, %d, \n      %s",  cmdTbl[i].cmdName,
        cmdTbl[i].category,
        cmdTbl[i].dstSysName,
        cmdTbl[i].dstAppName,
        cmdTbl[i].privilege,
        cmdTbl[i].paraCnt,
        helpTbl[i].cmdHelp);
#endif 

#if 0
	fprintf(stderr, "*** bef help %s\n", helpTbl[i].cmdHelp);
#endif

		/* help에 "'"를 column으로 인식되는 문제가 있어서 
		 * "'"에는 "\"를 붙혀서 DB에 넣자..limsh. 04/10/12*/
		memset(tmpHelp,0x00,sizeof(tmpHelp));
		find = m = 0;
		for(k=0; k<strlen(helpTbl[i].cmdHelp); k++) {
			if( helpTbl[i].cmdHelp[k]=='\'') {
				tmpHelp[m++] = '\\';
				tmpHelp[m++] = helpTbl[i].cmdHelp[k];
				find++;
			}
			else tmpHelp[m++] = helpTbl[i].cmdHelp[k];
		}
		if ( find ) {
			tmpHelp[m++] = NULL;
			/* DB에는 항상 \'이 들어가게, 메모리에는 '만 들어가게 */
//			strcpy (helpTbl[i].cmdHelp, tmpHelp );
		}

#if 0
/*debug*/fprintf(stderr, "*** after help %s\n", helpTbl[i].cmdHelp);
#endif
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', '%s', %d, %d, '%s')",
				MML_COMMAND_DB_TABLE_NAME,
				cmdTbl[i].cmdName,
				cmdTbl[i].category,
				cmdTbl[i].dstSysName,
				cmdTbl[i].dstAppName,
				cmdTbl[i].privilege,
				cmdTbl[i].paraCnt,
				tmpHelp 			/* helpTbl[i].cmdHelp, */
				);
		MMCD_MYSQL_QUERY(conn,query);
#if 0 /* jhnoh : 030817 */
        fprintf(stderr, "\nINSERT TABLE : %3d, %s", i, cmdTbl[i].cmdName); 
#endif
		// parameter table에 각 파라미터들의 syntax 정보를 넣는다.
		//
		for (j=0; j<cmdTbl[i].paraCnt; j++) {
#ifdef DEBUG_MODE
            fprintf(stderr, "\n %3d/%3d = %s, %s", j+1, cmdTbl[i].paraCnt, cmdTbl[i].cmdName, cmdTbl[i].paraInfo[j].paraName);
#endif
			sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', %d, %d, %d, %d, %d,'%s',",
					MML_PARAMETER_DB_TABLE_NAME,
					cmdTbl[i].cmdName,
					cmdTbl[i].paraInfo[j].paraName,
					j,
					cmdTbl[i].paraInfo[j].mandFlag,
					cmdTbl[i].paraInfo[j].paraType,
					cmdTbl[i].paraInfo[j].minVal,
					cmdTbl[i].paraInfo[j].maxVal, "");
			if ((cmdTbl[i].paraInfo[j].paraType == MML_PTYPE_ENUM) || (cmdTbl[i].paraInfo[j].paraType == MML_PTYPE_FIXSTR) || (cmdTbl[i].paraInfo[j].paraType == MML_PTYPE_FIXDEC) ) {
				// enum type이면 각 item이름을 space를 delimiter로 붙여 넣는다.
				strcat (query, "'");

				/* service이면 sorting하고 DB에 넣는다. 04/01/30, limsh */
				if( cmdTbl[i].paraInfo[j].paraType == MML_PTYPE_ENUM ) {
					funcEnumSort(cmdTbl[i].paraInfo[j].enumList);
				}

				for (k=0; k<MML_MAX_ENUM_ITEM; k++) {
					if (!strcmp (cmdTbl[i].paraInfo[j].enumList[k].enumStr, ""))
						break;
				
					if (k==0) {
						sprintf (tmp, "%s", cmdTbl[i].paraInfo[j].enumList[k].enumStr);
#if 0 /* GUI help는 enumlist만 2003/12/19 limsh */
						if( strlen(cmdTbl[i].paraInfo[j].enumList[k].enumStr2)!=0 )
							sprintf (tmp, "=%s", cmdTbl[i].paraInfo[j].enumList[k].enumStr2);
#endif
					}
					else {
						sprintf (tmp, " %s", cmdTbl[i].paraInfo[j].enumList[k].enumStr);
#if 0 /* GUI help는 enumlist만 2003/12/19 limsh */
						if( strlen(cmdTbl[i].paraInfo[j].enumList[k].enumStr2)!=0 )
							sprintf (tmp, "=%s", cmdTbl[i].paraInfo[j].enumList[k].enumStr2);
#endif
					}
					strcat  (query, tmp);
				}
				strcat (query, "'");
			} else {
				strcat (query, "NULL");
			}
			strcat (query, ")");
			MMCD_MYSQL_QUERY(conn,query);
   
		}
#if 0 /* jhnoh : 030817 */
            fprintf(stderr, "\nINSERT PARA  : %3d, %s", i, cmdTbl[i].cmdName); 
#endif	
	}

#ifdef DEBUG_MODE
    fprintf(stderr, "\n  THE END : DB INSERT ");
#endif 
	mysql_close(conn);

	return 1;

} //----- End of mmcd_saveCmdInfo2DB -----//

int enum_cmp_sort (const void *a, const void *b)
{
    return strcmp(((EnumPara *)a)->enumStr, ((EnumPara *)b)->enumStr);
}

void funcEnumSort(EnumPara *enumList)
{
	int k;

	for (k=0; k<MML_MAX_ENUM_ITEM; k++) {
		if (!strcmp (enumList[k].enumStr, "")) break;
	}

	qsort((void *)enumList, k, sizeof(EnumPara), enum_cmp_sort);
}


