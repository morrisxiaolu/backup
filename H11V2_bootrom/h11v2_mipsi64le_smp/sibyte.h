/* sibyte.h - Broadcom SiByte board I/O, CS, & GPIO constants & macros */

/*
 * Copyright (c) 2001-2002,2006-2008 Wind River Systems, Inc.
 *
 * The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.
 */

/*
 *  Copyright 2000,2001
 *  Broadcom Corporation. All rights reserved.
 *
 *  This software is furnished under license to Wind River Systems, Inc.
 *  and may be used only in accordance with the terms and conditions
 *  of this license.  No title or ownership is transferred hereby.
 */

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
01h,28mar08,jmt  Defect 119033- Fix Apigen errors.
01g,05jan07,rlg  fix compiler warning as per phil's instructions
01f,02nov06,pes  Fix makedepend warning/error by moving board type
                 decision tree here from config.h.
01e,12aug06,pes  Add SB_MAX_SMP_CPUS definition based on cpu type.
01d,08may06,pes  Add board designation #defines.
01c,03oct02,agf  Add defines for M24LV128 serial EEPROM
01b,10may02,tlc  Add C++ header protection.
01a,15nov01,agf  written.
*/

#ifndef __INCsibyteh
#define __INCsibyteh

#ifdef __cplusplus
extern "C" {
#endif

#define	BCM_SB1		1
#define	BCM_SB1A	2

#define BCM91250A	1	/* swarm */
#define BCM91250E	2	/* sentosa */
#define BCM91125PCIX	3	/* cheech */
#define BCM91480B	4	/* bigsur */

/* Only one of these should be defined */
#if defined (_WRS_MIPS_SB1250A)
#define  INCLUDE_BCM91250A
#define BOARD_TYPE  BCM91250A
#define BCM_FAMILY  BCM_SB1
#define BOARD_NAME "Broadcom BCM91250A/swarm"
#define SB_MAX_SMP_CPUS         2
#include "swarm.h"
#elif defined (_WRS_MIPS_SB1250E)
#define  INCLUDE_BCM91250E
#define BOARD_TYPE  BCM91250E
#define BCM_FAMILY  BCM_SB1
#define BOARD_NAME "Broadcom BCM91250A/sentosa"
#define SB_MAX_SMP_CPUS         2
#include "sentosa.h"
#elif defined (_WRS_MIPS_SB1125)
#define  INCLUDE_BCM91125PCIX
#define BOARD_TYPE  BCM91125PCIX
#define BCM_FAMILY  BCM_SB1
#define BOARD_NAME "Broadcom BCM91125PCIX/cheech"
#define SB_MAX_SMP_CPUS         1
#include "swarm.h"
#elif defined (_WRS_MIPS_SB1480)
#define INCLUDE_BCM91480B
#define BOARD_TYPE  BCM91480B
#define BCM_FAMILY  BCM_SB1A
#define BOARD_NAME "Hr11v2 SMP"
#define SB_MAX_SMP_CPUS         4
#include "bcm91480b.h"
#else
#error "unknown BOARD_TYPE"
#endif

#ifndef BOARD_TYPE
#error "unknown BOARD_TYPE"
#endif  /* BOARD_TYPE */

#ifndef BCM_FAMILY
#error "BCM_FAMILY must be defined"
#endif

#ifdef __cplusplus
}
#endif
#endif /* __INCsibyteh */
