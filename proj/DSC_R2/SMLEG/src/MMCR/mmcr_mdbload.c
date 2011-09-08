#include <stdio.h>
#include <fcntl.h>
#include <ipaf_svc.h>
#include "mmcr.h"

void load_svc_group( char columns[MAX_COL_COUNT][MAX_COL_SIZE], st_CategoryInfo *pstCat );
unsigned char get_enum_layer(char *layer);
unsigned char get_enum_block(char *block);
unsigned char get_svc_group(char *layer);

/*
 * return -1 : fail
 * 		   0 : success
 * 		   1 : end of file
 */
void load_svc_group( char columns[MAX_COL_COUNT][MAX_COL_SIZE], st_CategoryInfo *pstCat )
{
    short     temp;

    /* Catetory ID */
    pstCat->usCategory = atoi(columns[0]);
    /* Layer */
    pstCat->ucLayer = get_enum_layer(columns[1]);
    /* Filter out */
    pstCat->ucFilterOut = ((strcasecmp(columns[2], "ON") == 0)?1:0); // ON->1, OFF->0

    /* Service Block */
    pstCat->ucSvcBlk = get_enum_block(columns[6]); 

    /* Group */
    if(pstCat->ucSvcBlk != 2)   // 2 means WAP1ANA
        pstCat->ucGroup = (unsigned char)get_svc_group(columns[1]);
    else                        // In the case of WAP1ANA, set 1 as group.
        pstCat->ucGroup = 1;    // 1 of group means DESTIP.

    /* Sequence */
    temp = atoi(columns[11]);
    if(temp > 255){ // sequnece is bigger than 0xFF
        pstCat->szServiceID[0] = (0x00FF & temp);
        pstCat->ucGroup <<= 4;
        pstCat->ucGroup |= (char)(0x000F & (temp >> 8));
    }else{
        pstCat->szServiceID[0] = temp;
        pstCat->ucGroup <<= 4;
    }

    /* URL Char */
    pstCat->ucMode = ((strcasecmp(columns[7], "ON") == 0)?1:0); // ON->1, OFF->0
    /* IP Address */
    strcpy(pstCat->szCon[0], columns[3]);
    /* Net Mask */
    strcpy(pstCat->szCon[1], columns[4]);
    /* Port */
    if(!strcasecmp(columns[5], "ALL"))
        strcpy(pstCat->szCon[2], "0");
    else
        strcpy(pstCat->szCon[2], columns[5]);

    /* CDR IP Flag */
    //pstCat->szReserved[0] = ((strcasecmp(columns[8], "ON") == 0)?1:0); // ON->1, OFF->0
	if(!strcasecmp(columns[6], "CDR"))
        pstCat->szReserved[0] = 1;
    else
        pstCat->szReserved[0] = 0;
	
	/* FIRT_UDR add by helca 2007.02.02 */
	pstCat->szReserved[1] = ((strcasecmp(columns[9], "ON") == 0)?1:0); // ON->1, OFF->0
}

unsigned char get_enum_layer(char *layer)
{
    if(!strcasecmp(layer, "IP")){
        return 1;
    }else if(!strcasecmp(layer, "TCP")){
        return 2;
    }else if(!strcasecmp(layer, "UDP")){
        return 3;
    }

    return 0;
}

unsigned char get_enum_block(char *block)
{
    if(!strcasecmp(block, "CDR")){
        return 1;
    }else if(!strcasecmp(block, "WAP1ANA")){
        return 2;
    }else if(!strcasecmp(block, "UAWAPANA")){
        return 3;
    }else if(!strcasecmp(block, "WAP2ANA")){
        return 4;
    }else if(!strcasecmp(block, "HTTPANA")){
        return 5;
    }else if(!strcasecmp(block, "VODSANA")){	/* ADDED BY LYH 2006.11.27 */
		return 6;
	}else if(!strcasecmp(block, "WIPINWANA")){
		return 7;
	}else if(!strcasecmp(block, "JAVANWANA")){
		return 8; 
	}else if(!strcasecmp(block, "LOGM")){
		return 9; 
	}else if(!strcasecmp(block, "VTANA")){
		return 10; 
	}else if(!strcasecmp(block, "FBANA")){
		return 11; 
	}else if(!strcasecmp(block, "WVANA")){
		return 12; 
	}

    return 0;
}

unsigned char get_svc_group(char *layer)
{
    if(!strcasecmp(layer, "IP")){
        return 1;
    }else if(!strcasecmp(layer, "TCP")){
        return 2;
    }else if(!strcasecmp(layer, "UDP")){
        return 3;
    }

    return 0;
}
