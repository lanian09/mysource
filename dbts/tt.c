#includestdio.h>

 void CheckTAMDB(int dMyID)
 {
   int         dLine, dRate, dIndex;
   char        sCmd[BUFSIZ], sResult[BUFSIZ];
   long long   llMemFree, llMemBuffer, llMemCache;
	long long  *lMax, *llCur
   time_t      tUpdate;
   size_t      szResult;
 
   tUpdate = time(NULL);
   sprintf(sCmd, "%s/SCRIPT/SNMP/SNMP_TAMDB.sh", START_PATH);
   if( (stPthrdTAMDB.fPipe = popen(sCmd, "r")) == NULL)
       return;
 
   dLine   = 0;
   while(fgets(sResult, BUFSIZ, stPthrdTAMDB.fPipe) != NULL)
   {
       szResult = strlen(sResult);
       while(isspace(sResult[szResult-1]))
           sResult[--szResult] = 0x00;
 
       if(dLine  MAX_TAMDB_CPU_COUNT)
       {

           fidb->stTAMDBTOT.stTAMDB.stCPU.lMax = (long long)1000;
           fidb->stTAMDBTOT.stTAMDB.stCPU.llCur = (long long)(atof(sResult)*10);
			lMax = &(fidb->stTAMDBTOT.stTAMDB.stCPU.lMax);
			llCur= &(fidb->stTAMDBTOT.stTAMDB.stCPU.llCur);
			lMax = (long long)1000;
			llCur= (long long)(atof(sResult)*10);
       
           dRate   = (int)( ((float)fidb->stTAMDBTOT.stTAMDB.stCPU.llCur/(float)fidb->stTAMDBTOT.stTAMDB.stCPU.lMax)*100);
			dRate = (int)( ((float)llCur/(float)lMax) *100);

			kMinor = &keepalive->stTAMDBLOAD.stCPU.usMinor;
			kMajor = &keepalive->stTAMDBLOAD.stCPU.usMajor;
			kCri   = &keepalive->stTAMDBLOAD.stCPU.usCritical;

			ucCPUStatus = &fidb->stTAMDBTOT.stTAMDB.ucCPUStatus;
 
           if(dRate  keepalive->stTAMDBLOAD.stCPU.usMinor)
           {
               if(fidb->stTAMDBTOT.stTAMDB.ucCPUStatus != NORMAL)
                   Send_CondMess(SYSTYPE_TAMDB, LOCTYPE_LOAD, INVTYPE_CPU, 0, NORMAL, fidb->stTAMDBTOT.stTAMDB.ucCPUStatus);
 
               fidb->stTAMDBTOT.stTAMDB.ucCPUStatus = NORMAL;
           }

			if( dRate < kMinor ){
				if( ucCPUStatus != NORMAL ) Send_CondMess(SYSTYPE_TAMDB, LOCTYPE_LOAD, INVTYPE_CPU, 0, NORMAL, ucCPUStatus );
				ucCPUStatus = NORMAL;
			}



		
           else if(dRate  keepalive->stTAMDBLOAD.stCPU.usMajor)
           {
               if(fidb->stTAMDBTOT.stTAMDB.ucCPUStatus != MINOR)
                   Send_CondMess(SYSTYPE_TAMDB, LOCTYPE_LOAD, INVTYPE_CPU, 0, MINOR, fidb->stTAMDBTOT.stTAMDB.ucCPUStatus);
 
               fidb->stTAMDBTOT.stTAMDB.ucCPUStatus = MINOR;
           }

			else if(dRate < kMajor){
				if( ucCPUStatus != MINOR ) Send_CondMess(SYSTYPE_TAMDB, LOCTYPE_LOAD, INVTYPE_CPU, 0, MINOR, ucCPUStatus );
				ucCPUStatus = MINOR;
			}

           else if(dRate  keepalive->stTAMDBLOAD.stCPU.usCritical)
           {
               if(fidb->stTAMDBTOT.stTAMDB.ucCPUStatus != MAJOR)
                   Send_CondMess(SYSTYPE_TAMDB, LOCTYPE_LOAD, INVTYPE_CPU, 0, MAJOR, fidb->stTAMDBTOT.stTAMDB.ucCPUStatus);
 
               fidb->stTAMDBTOT.stTAMDB.ucCPUStatus = MAJOR;
           }

			else if( dRate < kCri ){
				if( ucCPUStatus != MAJOR ) ... MAJOR
				ucCPUStatus = MAJOR;
			}
           else
           {
               if(fidb->stTAMDBTOT.stTAMDB.ucCPUStatus != CRITICAL)
                   Send_CondMess(SYSTYPE_TAMDB, LOCTYPE_LOAD, INVTYPE_CPU, 0, CRITICAL, fidb->stTAMDBTOT.stTAMDB.ucCPUStatus);
 
               fidb->stTAMDBTOT.stTAMDB.ucCPUStatus = CRITICAL;
           }
       }
       else if(dLine  (MAX_TAMDB_CPU_COUNT+MAX_TAMDB_MEM_COUNT))
       {
           switch(dLine)
           {
               case 1:
					unsigned long *plMax = &fidb->stTAMDBTOT.stTAMDB.stMEM.lMax;
					*plMax = (unsigned long)atol(sResult);

                   fidb->stTAMDBTOT.stTAMDB.stMEM.lMax = (unsigned long)atol(sResult);
                   dLine++;
                   continue;
               case 2:
                   llMemFree   = (unsigned long)atol(sResult);
                   dLine++;
                   continue;
               case 3:
                   llMemBuffer = (unsigned long)atol(sResult);
                   dLine++;
                   continue;
               case 4:
                   llMemCache  = (unsigned long)atol(sResult);
                   fidb->stTAMDBTOT.stTAMDB.stMEM.llCur    = fidb->stTAMDBTOT.stTAMDB.stMEM.lMax - llMemFree - llMemBuffer - llMemCache;
                   dRate   = (int)( ((float)fidb->stTAMDBTOT.stTAMDB.stMEM.llCur/(float)fidb->stTAMDBTOT.stTAMDB.stMEM.lMax)*100);
                   break;
           }
 
           if(dRate  keepalive->stTAMDBLOAD.stMEM.usMinor)
           {
               if(fidb->stTAMDBTOT.stTAMDB.ucMEMStatus != NORMAL)
                   Send_CondMess(SYSTYPE_TAMDB, LOCTYPE_LOAD, INVTYPE_MEMORY, 0, NORMAL, fidb->stTAMDBTOT.stTAMDB.ucMEMStatus);
 
               fidb->stTAMDBTOT.stTAMDB.ucMEMStatus = NORMAL;
           }
           else if(dRate  keepalive->stTAMDBLOAD.stMEM.usMajor)
           {
               if(fidb->stTAMDBTOT.stTAMDB.ucMEMStatus != MINOR)
                   Send_CondMess(SYSTYPE_TAMDB, LOCTYPE_LOAD, INVTYPE_MEMORY, 0, MINOR, fidb->stTAMDBTOT.stTAMDB.ucMEMStatus);
 
               fidb->stTAMDBTOT.stTAMDB.ucMEMStatus = MINOR;
           }
           else if(dRate  keepalive->stTAMDBLOAD.stMEM.usCritical)
           {
               if(fidb->stTAMDBTOT.stTAMDB.ucMEMStatus != MAJOR)
                   Send_CondMess(SYSTYPE_TAMDB, LOCTYPE_LOAD, INVTYPE_MEMORY, 0, MAJOR, fidb->stTAMDBTOT.stTAMDB.ucMEMStatus);
 
               fidb->stTAMDBTOT.stTAMDB.ucMEMStatus = MAJOR;
           }
           else
           {
               if(fidb->stTAMDBTOT.stTAMDB.ucMEMStatus != CRITICAL)
                   Send_CondMess(SYSTYPE_TAMDB, LOCTYPE_LOAD, INVTYPE_MEMORY, 0, CRITICAL, fidb->stTAMDBTOT.stTAMDB.ucMEMStatus);
 
               fidb->stTAMDBTOT.stTAMDB.ucMEMStatus = CRITICAL;
           }
       }
       else if(dLine  (MAX_TAMDB_CPU_COUNT+MAX_TAMDB_MEM_COUNT+MAX_TAMDB_DISK_COUNT))
       {
           dIndex  = (dLine-(MAX_TAMDB_CPU_COUNT+MAX_TAMDB_MEM_COUNT)) / 2;
           if( (fidb->stTAMDBTOT.stTAMDB.ucDISKStatus[dIndex] == NOT_EQUIP) || (fidb->stTAMDBTOT.stTAMDB.ucDISKStatus[dIndex] == MASK))
           {
               if( fidb->stTAMDBTOT.stTAMDB.stDISK[dIndex].llCur || fidb->stTAMDBTOT.stTAMDB.stDISK[dIndex].lMax)
               {
                   fidb->stTAMDBTOT.stTAMDB.stDISK[dIndex].llCur   = 0L;
                   fidb->stTAMDBTOT.stTAMDB.stDISK[dIndex].lMax    = 0L;
               }
               dLine++;
               continue;
           }
 
           switch(dLine%2)
           {
               case 0:
                   fidb->stTAMDBTOT.stTAMDB.stDISK[dIndex].llCur   = (unsigned long)atol(sResult);
                   dRate   = (int)( ((float)fidb->stTAMDBTOT.stTAMDB.stDISK[dIndex].llCur/(float)fidb->stTAMDBTOT.stTAMDB.stDISK[dIndex].lMax)*100);
                   break;
               case 1:
                   fidb->stTAMDBTOT.stTAMDB.stDISK[dIndex].lMax    = (unsigned long)atol(sResult);
                   dLine++;
                   continue;
           }
 
           if(dRate  keepalive->stTAMDBLOAD.stDISK.usMinor)
           {
               if(fidb->stTAMDBTOT.stTAMDB.ucDISKStatus[dIndex] != NORMAL)
                   Send_CondMess(SYSTYPE_TAMDB, LOCTYPE_LOAD, INVTYPE_DISK, dIndex, NORMAL, fidb->stTAMDBTOT.stTAMDB.ucDISKStatus[dIndex]);
 
               fidb->stTAMDBTOT.stTAMDB.ucDISKStatus[dIndex] = NORMAL;
           }
           else if(dRate  keepalive->stTAMDBLOAD.stDISK.usMajor)
           {
               if(fidb->stTAMDBTOT.stTAMDB.ucDISKStatus[dIndex] != MINOR)
                   Send_CondMess(SYSTYPE_TAMDB, LOCTYPE_LOAD, INVTYPE_DISK, dIndex, MINOR, fidb->stTAMDBTOT.stTAMDB.ucDISKStatus[dIndex]);
 
               fidb->stTAMDBTOT.stTAMDB.ucDISKStatus[dIndex] = MINOR;
           }
           else if(dRate  keepalive->stTAMDBLOAD.stDISK.usCritical)
           {
               if(fidb->stTAMDBTOT.stTAMDB.ucDISKStatus[dIndex] != MAJOR)
                   Send_CondMess(SYSTYPE_TAMDB, LOCTYPE_LOAD, INVTYPE_DISK, dIndex, MAJOR, fidb->stTAMDBTOT.stTAMDB.ucDISKStatus[dIndex]);
 
               fidb->stTAMDBTOT.stTAMDB.ucDISKStatus[dIndex] = MAJOR;
           }
           else
           {
               if(fidb->stTAMDBTOT.stTAMDB.ucDISKStatus[dIndex] != CRITICAL)
                   Send_CondMess(SYSTYPE_TAMDB, LOCTYPE_LOAD, INVTYPE_DISK, dIndex, CRITICAL, fidb->stTAMDBTOT.stTAMDB.ucDISKStatus[dIndex]);
 
               fidb->stTAMDBTOT.stTAMDB.ucDISKStatus[dIndex] = CRITICAL;
           }
       }
       else if(dLine  (MAX_TAMDB_CPU_COUNT+MAX_TAMDB_MEM_COUNT+MAX_TAMDB_DISK_COUNT+MAX_TAMDB_LAN_COUNT))
       {
           dIndex = dLine-(MAX_TAMDB_CPU_COUNT+MAX_TAMDB_MEM_COUNT+MAX_TAMDB_DISK_COUNT);
           if( (fidb->stTAMDBTOT.stTAMDB.ucLinkStatus[dIndex] != NOT_EQUIP) && (fidb->stTAMDBTOT.stTAMDB.ucLinkStatus[dIndex] != MASK))
           {
               if(strncasecmp(sResult, "UP", 2) == 0)
               {
                   if(fidb->stTAMDBTOT.stTAMDB.ucLinkStatus[dIndex] != NORMAL)
                       Send_CondMess(SYSTYPE_TAMDB, LOCTYPE_PHSC, INVTYPE_ETH_INF, dIndex, NORMAL, fidb->stTAMDBTOT.stTAMDB.ucLinkStatus[dIndex]);
                   pthread_mutex_lock(&stPthrdTAMDB.PthrdMutex);
                   fidb->stTAMDBTOT.stTAMDB.ucLinkStatus[dIndex] = NORMAL;
                   pthread_mutex_unlock(&stPthrdTAMDB.PthrdMutex);
               }
               else
               {
                   if(fidb->stTAMDBTOT.stTAMDB.ucLinkStatus[dIndex] != CRITICAL)
                       Send_CondMess(SYSTYPE_TAMDB, LOCTYPE_PHSC, INVTYPE_ETH_INF, dIndex, CRITICAL, fidb->stTAMDBTOT.stTAMDB.ucLinkStatus[dIndex]);
 
                   pthread_mutex_lock(&stPthrdTAMDB.PthrdMutex);
                   fidb->stTAMDBTOT.stTAMDB.ucLinkStatus[dIndex] = CRITICAL;
                   pthread_mutex_unlock(&stPthrdTAMDB.PthrdMutex);
               }
           }
       }
       else
           dAppLog(LOG_CRI, "F=%s:%s.%d: SNMP_TAMDB.sh dLine[%d] ERROR", __FILE__, __FUNCTION__, __LINE__, dLine);
 
       dLine++;
   }
 
   if(!dLine)
   {
       for(dIndex = 0; dIndex  MAX_TAMDB_LAN_COUNT; dIndex++)
       {
           if( (fidb->stTAMDBTOT.stTAMDB.ucLinkStatus[dIndex] != NOT_EQUIP) && (fidb->stTAMDBTOT.stTAMDB.ucLinkStatus[dIndex] != MASK))
           {
               if(fidb->stTAMDBTOT.stTAMDB.ucLinkStatus[dIndex] != CRITICAL)
                   Send_CondMess(SYSTYPE_TAMDB, LOCTYPE_PHSC, INVTYPE_ETH_INF, dIndex, CRITICAL, fidb->stTAMDBTOT.stTAMDB.ucLinkStatus[dIndex]);
 
               pthread_mutex_lock(&stPthrdTAMDB.PthrdMutex);
               fidb->stTAMDBTOT.stTAMDB.ucLinkStatus[dIndex] = CRITICAL;
               pthread_mutex_unlock(&stPthrdTAMDB.PthrdMutex);
           }
       }
   }
 
   pclose(stPthrdTAMDB.fPipe);
}

int main()
{
	exit(0);
}
