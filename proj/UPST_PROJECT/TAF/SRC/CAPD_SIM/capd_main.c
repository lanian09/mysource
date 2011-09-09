/*******************************************************************************
                DQMS Project

   Author   : Lee Dong-Hwan
   Section  : CAPD 
   SCCS ID  : @(#)capd_main.c (V1.0)
   Date     : 07/02/09
   Revision History :
        '09.    07. 02. initial

   Description :

   Copyright (c) uPRESTO 2005
*******************************************************************************/

/** A. FILE INCLUSION *********************************************************/
#include "capd_main.h"

/** B. DEFINITION OF NEW CONSTANTS ********************************************/
/** C. DEFINITION OF NEW TYPES ************************************************/
/** D. DECLARATION OF VARIABLES ***********************************************/
/** E.1 DEFINITION OF FUNCTIONS ***********************************************/
/** E.2 DEFINITION OF FUNCTIONS ***********************************************/
/*******************************************************************************

*******************************************************************************/
int main(int argc, char* argv[])
{
	/* FOR TESTING */
	if(argc == 3) {
		test_func(argv[1], atoi(argv[2]));
	} else {
		printf("Usage: ./%s 127.0.0.1 9000\n", argv[0]);
		exit(0);
	}

	// Assign process name
	memcpy(&gszMyProc[0], argv[0], strlen(argv[0]));

	// Assign process sequence
	guiSeqProcID = SEQ_PROC_CAPD;

	// Initiate log, gifo, fidb, port status ..
	dInitCapd(&pstMEMSINFO, &pstCIFO);

	open_device("dag0");
//	open_device("dag1");

	do_packet_capture();

	close_device();

	return 1;
}
