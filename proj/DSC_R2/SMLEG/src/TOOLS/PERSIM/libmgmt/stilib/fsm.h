/* 
 * Author : albamc <albamc@gmail.com> :
 *   Copyright 2004 hwang-hae-yeun
 *   
 *   This program is NOT-free software; You can redistribute it 
 *   and/or modify it under the terms of the Albam's General Public 
 *   License (AGPL) as published by the albamc 
 *   
 *   < AGPL (Albam's General Public License) >
 *   DO NOT USE MY SOURCES!
 */

#ifndef FSM_H
#define FSM_H

#include <stdio.h>
#include <stdlib.h>

#define FSM_MAX_STATE 	256
#define FSM_MAX_ACTION 	256

#pragma pack(1)

typedef void (*fsmfunc_t)(void*);

typedef struct fsmaction {
	fsmfunc_t 	func;
	int 		next_state;
} fsmaction_t;

typedef struct fsmstate {
	fsmaction_t 	act[FSM_MAX_ACTION];
} fsmstate_t;

typedef struct fsmroot {
	fsmstate_t 	state[FSM_MAX_STATE];
} fsmroot_t;

typedef struct fsm {
	fsmroot_t* 	fsm;
	int 		curr_state;
	int 		prev_state;
} fsm_t;

#pragma pack()

#ifdef __cplusplus
extern "C" {
#endif

extern void fsmroot_init(fsmroot_t* fr);
extern void fsm_init(fsm_t* ft, fsmroot_t* fr);
extern void fsm_add(fsm_t* ft, int curr_sn, int action, int next_sn, fsmfunc_t fp);
extern void fsm_setstate(fsm_t* ft, int sn);
extern void fsm_do (fsm_t* ft, int an, void* dp);

#ifdef __cplusplus
}
#endif

#endif
