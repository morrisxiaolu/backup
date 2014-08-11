/* configNet.h - network configuration header */

/* Copyright 2001, 2002, 2005-2006 Wind River Systems, Inc. */

/*
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01f,07jul06,wap  Switch to new sbeVxbEnd driver
01e,14mar06,pes  Decouple mac support so it's configurable from config.h
01d,28jul05,rlg  spr 109086 and 109090 compiler warnings cleanup
01c,11apr05,kab  fix comments for apiGen (SPR 107842)
01b,10may02,tlc  Add C++ header protection.
01a,15nov01,agf  written
*/
 
#ifndef INCnetConfigh
#define INCnetConfigh

#ifdef __cplusplus
extern "C" {
#endif

#include "vxWorks.h"
#include "end.h"
#include "config.h"

#define DEC_LOAD_FUNC   sysDec21x40EndLoad
#define DEC_BUFF_LOAN   1
#if 0 /* added by yinwx, 20100202 */
/* added by yinwx, 20100127 */ 
#define SMEND_LOAD_FUNC sysSmEndLoad
#define SMEND_BUFF_LOAN   1
IMPORT END_OBJ* SMEND_LOAD_FUNC (char*, void*); /* added by yinwx, 20100127 */ 

#endif
IMPORT END_OBJ* DEC_LOAD_FUNC (char*, void*);

IMPORT END_OBJ* atsemacEndLoad (char*, void*);

END_TBL_ENTRY endDevTbl [] =
{
#ifdef INCLUDE_DEC  /* Tulip PCI devices */
#ifdef INCLUDE_DEC0
    { 0, DEC_LOAD_FUNC, "", DEC_BUFF_LOAN, NULL, FALSE},
#endif
#ifdef INCLUDE_DEC1
    { 1, DEC_LOAD_FUNC, "", DEC_BUFF_LOAN, NULL, FALSE},
#endif
#endif              /* on-chip devices */
#if 0 /* added by yinwx, 20100202 */
	/* added by yinwx, for sysSmEndLoad, 20100127 */
	{ 0, SMEND_LOAD_FUNC, "", SMEND_BUFF_LOAN, NULL, FALSE},
#endif

#ifdef INCLUDE_ATSEMAC
    { 0, atsemacEndLoad, "", DEC_BUFF_LOAN, NULL, FALSE},    
#endif
    { 0, END_TBL_END, NULL, 0, NULL, FALSE},
};

#ifdef __cplusplus
}
#endif
#endif /* INCnetConfigh */
