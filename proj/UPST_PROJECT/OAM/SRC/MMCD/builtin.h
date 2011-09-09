/**
	@file		builtin.h
	@author		hhbaek
	@date		2011-07-13
	@version
	@brief		builtin.c 에서 사용하는 헤더파일
*/

#ifndef __BUILTIN_H_
#define __BUILTIN_H_

/**
 *	Include headers
 */
#include "mmcdef.h"
#include "cmd_user.h"

/**
 * Declare functions
 */
extern int Exe_Builtin(mml_msg *ml, int sockfd, In_Arg  in_para[]);


#endif /** __BUILTIN_H_ */
