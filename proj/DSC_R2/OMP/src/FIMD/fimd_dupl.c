#include "fimd_proto.h"

extern SFM_sfdb	*sfdb;

void discret_dupl_status()
{
	int		i,j,k = 0;
	stDupleCheck 	dup[SYSCONF_MAX_ASSO_SYS_NUM-1];

	memset(&dup, 0x00, sizeof(dup));
	dup[0].dup = NULL;
	dup[1].dup = NULL;

	for(i = 0; i < SYSCONF_MAX_ASSO_SYS_NUM; i++){
		if((!strncmp(sfdb->sys[i].commInfo.type, MP_SYSTEM_TYPE, COMM_MAX_NAME_LEN)) &&
		   (!strncmp(sfdb->sys[i].commInfo.group, MP_SYSTEM_GROUP, COMM_MAX_NAME_LEN))){
			strncpy(dup[k].sysname, sfdb->sys[i].commInfo.name, COMM_MAX_NAME_LEN);
			dup[k].sysindex = i;
			dup[k].dup = &(sfdb->sys[i].commInfo.systemDup);
			for(j = 0; j < sfdb->sys[i].commInfo.procCnt; j++){
				if(!strncasecmp(sfdb->sys[i].commInfo.procInfo[j].name, "IXPC", COMM_MAX_NAME_LEN)){
					dup[k].sysstatus = sfdb->sys[i].commInfo.procInfo[j].status;
				}
				/* 필요항목이 아님 ... sjjeon
				if(!strncasecmp(sfdb->sys[i].commInfo.procInfo[j].name, "SDMD", COMM_MAX_NAME_LEN)){
					dup[k].sdmdstatus = sfdb->sys[i].commInfo.procInfo[j].status;
				}
				*/
			}
			k++;
		}
	}

	//exception handle - both of MPs or one of MPs do not rise up.
	if(dup[0].dup == NULL && dup[1].dup == NULL){
		// what do i do? --?
		if(strncmp(sfdb->active_sys_name, "UNKNOWN", COMM_MAX_NAME_LEN)){
			strncpy(sfdb->last_active_name, sfdb->active_sys_name, COMM_MAX_NAME_LEN);
			strncpy(sfdb->active_sys_name, "UNKNOWN", COMM_MAX_NAME_LEN);
		}
		return;
	}else if(dup[0].dup != NULL &&  dup[1].dup == NULL){
		if(strncmp(sfdb->active_sys_name, dup[0].sysname, COMM_MAX_NAME_LEN)){
       		strncpy(sfdb->last_active_name, sfdb->active_sys_name, COMM_MAX_NAME_LEN);
       		strncpy(sfdb->active_sys_name,  dup[0].sysname, COMM_MAX_NAME_LEN);
       		// event notification to GUI
       		// active system changed
       	}
		return;
	}

	// MP system status check
	// System Status of both MPs is Normal 
	if(dup[0].sysstatus == SFM_STATUS_ALIVE &&
	   dup[1].sysstatus == SFM_STATUS_ALIVE){

		if(dup[0].sdmdstatus == SFM_STATUS_ALIVE &&
		   dup[1].sdmdstatus == SFM_STATUS_ALIVE){
			if((dup[0].dup->myStatus && dup[1].dup->yourStatus) &&
				(dup[1].dup->myStatus && dup[0].dup->yourStatus)){
				normal_active(dup);
			}else{
				asynch_active(dup);
			}
		}else if(dup[0].sdmdstatus == SFM_STATUS_ALIVE &&
		  dup[1].sdmdstatus == SFM_STATUS_DEAD){
			limited_active(dup[0], dup[1]);

		}else if(dup[0].sdmdstatus == SFM_STATUS_DEAD &&
		  dup[1].sdmdstatus == SFM_STATUS_ALIVE){
			limited_active(dup[1], dup[0]);
		} else{
			if(strncmp(sfdb->active_sys_name, "UNKNOWN", COMM_MAX_NAME_LEN)){
				strncpy(sfdb->last_active_name, sfdb->active_sys_name, COMM_MAX_NAME_LEN);
				strncpy(sfdb->active_sys_name, "UNKNOWN", COMM_MAX_NAME_LEN);
				// event notification to GUI
				// active system changed
			}
		}
		//System duplication synchronized
	}else if(dup[0].sysstatus == SFM_STATUS_ALIVE &&
	   		 dup[1].sysstatus == SFM_STATUS_DEAD){
		limited_active(dup[0], dup[1]);
	}else if(dup[1].sysstatus == SFM_STATUS_ALIVE &&
	   		 dup[0].sysstatus == SFM_STATUS_DEAD){
		limited_active(dup[1], dup[0]);
	}else{
		// unknown status
		// no change
		if(strncmp(sfdb->active_sys_name, "UNKNOWN", COMM_MAX_NAME_LEN)){
			strncpy(sfdb->last_active_name, sfdb->active_sys_name, COMM_MAX_NAME_LEN);
			strncpy(sfdb->active_sys_name, "UNKNOWN", COMM_MAX_NAME_LEN);
			// event notification to GUI
			// active system changed
		}
	}
}

void normal_active(stDupleCheck dup[SYSCONF_MAX_ASSO_SYS_NUM-1])
{
	// MP system status check
	// Duplication Status of both MPs is Normal 
	if((dup[0].dup->myStatus == DUPL_ACTIVE) &&
	   ((dup[0].dup->yourStatus == DUPL_STANDBY) ||
	    (dup[0].dup->yourStatus == DUPL_BOOT)    ||
		(dup[0].dup->yourStatus == DUPL_UNKNOWN))){
		if(strncmp(sfdb->active_sys_name, sfdb->sys[dup[0].sysindex].commInfo.name, COMM_MAX_NAME_LEN)){
			strncpy(sfdb->last_active_name, sfdb->active_sys_name, COMM_MAX_NAME_LEN);
			strncpy(sfdb->active_sys_name, sfdb->sys[dup[0].sysindex].commInfo.name, COMM_MAX_NAME_LEN);
			// event notification to GUI
			// active system changed
		}
	}else if((dup[1].dup->myStatus == DUPL_ACTIVE) &&
	   		 ((dup[1].dup->yourStatus == DUPL_STANDBY) ||
	    	  (dup[1].dup->yourStatus == DUPL_BOOT)    ||
			  (dup[1].dup->yourStatus == DUPL_UNKNOWN))){
		if(strncmp(sfdb->active_sys_name, sfdb->sys[dup[1].sysindex].commInfo.name, COMM_MAX_NAME_LEN)){
			strncpy(sfdb->last_active_name, sfdb->active_sys_name, COMM_MAX_NAME_LEN);
			strncpy(sfdb->active_sys_name, sfdb->sys[dup[1].sysindex].commInfo.name, COMM_MAX_NAME_LEN);
			// event notification to GUI
			// active system changed
		}
	}else if((dup[0].dup->myStatus == DUPL_ACTIVE) && // both of MPs are active
			 (dup[1].dup->myStatus == DUPL_ACTIVE)){
		if(strncmp(sfdb->active_sys_name, "ACTIVE", COMM_MAX_NAME_LEN)){
			strncpy(sfdb->last_active_name, sfdb->active_sys_name, COMM_MAX_NAME_LEN);
			strncpy(sfdb->active_sys_name, "ACTIVE", COMM_MAX_NAME_LEN);
			// event notification to GUI
			// active system changed
		}
	}else if((dup[0].dup->myStatus == DUPL_STANDBY) && // both of MPs are standby
			 (dup[1].dup->myStatus == DUPL_STANDBY)){
		if(strncmp(sfdb->active_sys_name, "STANDBY", COMM_MAX_NAME_LEN)){
			strncpy(sfdb->last_active_name, sfdb->active_sys_name, COMM_MAX_NAME_LEN);
			strncpy(sfdb->active_sys_name, "STANDBY", COMM_MAX_NAME_LEN);
			// event notification to GUI
			// active system changed
		}
	}else{			// both of MPs are standby
		if(strncmp(sfdb->active_sys_name, "UNKNOWN", COMM_MAX_NAME_LEN)){
			strncpy(sfdb->last_active_name, sfdb->active_sys_name, COMM_MAX_NAME_LEN);
			strncpy(sfdb->active_sys_name, "UNKNOWN", COMM_MAX_NAME_LEN);
			// event notification to GUI
			// active system changed
		}
	}
}

void asynch_active(stDupleCheck dup[SYSCONF_MAX_ASSO_SYS_NUM-1])
{
	// GUI alarm notification
	// No change active_sys_name,
	// because information of duplication is unreliable.
}

void limited_active(stDupleCheck knowdup, stDupleCheck unknowdup)
{
	// distrete only one side information

	if((knowdup.dup->myStatus == DUPL_ACTIVE) &&
	   ((knowdup.dup->yourStatus == DUPL_STANDBY) ||
		(knowdup.dup->yourStatus == DUPL_BOOT)    ||
		(knowdup.dup->yourStatus == DUPL_UNKNOWN))){
		if(strncmp(sfdb->active_sys_name, sfdb->sys[knowdup.sysindex].commInfo.name, COMM_MAX_NAME_LEN)){
			strncpy(sfdb->last_active_name, sfdb->active_sys_name, COMM_MAX_NAME_LEN);
			strncpy(sfdb->active_sys_name, sfdb->sys[knowdup.sysindex].commInfo.name, COMM_MAX_NAME_LEN);
			// event notification to GUI
			// active system changed
		}
	}else if(((knowdup.dup->myStatus == DUPL_UNKNOWN) || 
			  (knowdup.dup->myStatus == DUPL_STANDBY) || 
		      (knowdup.dup->yourStatus == DUPL_BOOT)) &&
	         (knowdup.dup->yourStatus == DUPL_ACTIVE)){
		if(strncmp(sfdb->active_sys_name, sfdb->sys[unknowdup.sysindex].commInfo.name, COMM_MAX_NAME_LEN)){
			strncpy(sfdb->last_active_name, sfdb->active_sys_name, COMM_MAX_NAME_LEN);
			strncpy(sfdb->active_sys_name, sfdb->sys[unknowdup.sysindex].commInfo.name, COMM_MAX_NAME_LEN);
			// event notification to GUI
			// active system changed
		}
	}else if((knowdup.dup->myStatus == DUPL_ACTIVE) &&
	   (knowdup.dup->yourStatus == DUPL_ACTIVE)){
		if(strncmp(sfdb->active_sys_name, "ACTIVE", COMM_MAX_NAME_LEN)){
			strncpy(sfdb->last_active_name, sfdb->active_sys_name, COMM_MAX_NAME_LEN);
			strncpy(sfdb->active_sys_name, "ACTIVE", COMM_MAX_NAME_LEN);
			// event notification to GUI
			// active system changed
		}
	}else if((knowdup.dup->myStatus == DUPL_STANDBY) &&
	   (knowdup.dup->yourStatus == DUPL_STANDBY)){
		if(strncmp(sfdb->active_sys_name, "STANDBY", COMM_MAX_NAME_LEN)){
			strncpy(sfdb->last_active_name, sfdb->active_sys_name, COMM_MAX_NAME_LEN);
			strncpy(sfdb->active_sys_name, "STANDBY", COMM_MAX_NAME_LEN);
			// event notification to GUI
			// active system changed
		}
	}else{
		if(strncmp(sfdb->active_sys_name, "UNKNOWN", COMM_MAX_NAME_LEN)){
			strncpy(sfdb->last_active_name, sfdb->active_sys_name, COMM_MAX_NAME_LEN);
			strncpy(sfdb->active_sys_name, "UNKNOWN", COMM_MAX_NAME_LEN);
			// event notification to GUI
			// active system changed
		}
	}

}
