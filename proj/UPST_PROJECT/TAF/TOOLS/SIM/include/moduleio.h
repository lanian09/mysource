/**********************************************************************
*
* Copyright (c) 2004-2005 Endace Technology Ltd, Hamilton, New Zealand.
* All rights reserved.
*
* This source code is proprietary to Endace Technology Limited and no
* part of it may be redistributed, published or disclosed except as 
* outlined in the written contract supplied with this product.
*
* $Id: moduleio.h,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
*
**********************************************************************/

/**********************************************************************
* FILE:		moduleio.h
* DESCRIPTION:	I/O for a module
*
*
* HISTORY:
*    LDM 9/9/04 - Initial version
**********************************************************************/

#ifndef MODULE_IO_H
#define MODULE_IO_H "moduleio.h"

/* Endace headers. */
#include "dag_platform.h"
#include "dagreg.h"


typedef struct module_io MODULE_IO;
typedef MODULE_IO *PMODULE_IO;

struct module_io
{
  dag_reg_t *pdrReg;
  uint32_t wOffset;
  unsigned long wLoc;
  
  /* ctor and dtor */
  PMODULE_IO (*New)(uint8_t *pucDagIom, dag_reg_t *pdrReg);
  void       (*Delete)(PMODULE_IO pThis);

  /* methods */
  void     (*Write)(PMODULE_IO pThis, uint32_t wAddr, uint32_t wVal);
  uint32_t (*Read)(PMODULE_IO pThis, uint32_t wAddr);
};

PMODULE_IO MIONew(unsigned char *pucDagIom, dag_reg_t *pdrReg);

/* accessors */
static INLINE uint32_t MIOGetOffset(PMODULE_IO pThis)
{
  return pThis->wOffset;
}

static INLINE unsigned long MIOGetLoc(PMODULE_IO pThis)
{
  return pThis->wLoc;
}

#endif /* MODULE_IO_H */
