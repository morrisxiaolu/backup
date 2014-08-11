/* sb1PciLib.c - BCM12500 PCI/LDT configuration and access support */

/* Copyright 2007 Wind River Systems, Inc. */

/*
**********************************************************************
*
*  Copyright 2000,2001
*  Broadcom Corporation. All rights reserved.
*
*  This software is furnished under license to Wind River Systems, Inc.
*  and may be used only in accordance with the terms and conditions
*  of this license.  No title or ownership is transferred hereby.
***********************************************************************
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
01d,15nov04,mdo  Documentation fixes for apigen
01c,11nov02,agf  update PCI device mode logic
01b,03oct02,agf  changes for shared sentosa support
01a,11dec01,agf  coding standard updates
01a,15nov01, agf  written
*/

/*
DESCRIPTION

This is the board-specific component of the BCM1250 PCI/LDT configuration
code.  Most of the work of autoconfiguration is performed by the generic
pciAutoConfigLib, which in turn uses pciConfigLib.

The code here provides four basic functions.

  It initializes the PCI and LDT host bridges prior to configuration.
  It provides functions to access the configuration registers as implemented
    in the BCM1250.
  It supplies callback hooks for the configuration algorithm itself,
    to exclude devices/buses and to perform any special pre- and
    post-processing required by a device as part of configuration..
  It describes how the board maps bus 0 device interrupts to BCM1250
    interrupt sources.

The bulk of the code is associated with the hook that initializes an
LDT chain and assigns UnitIDs prior to probing or enumeration of that
chain. Thereafter, the chain can treated as if it were an ordinary PCI bus.

USAGE

The public entry points provide the top level control of configuration
as documented in pciAutoConfigLib.

INCLUDE FILES: bcm1250PciLib.h
*/


/* includes */

#include <vxWorks.h>
#include "config.h"

#if (BCM_FAMILY == BCM_SB1)
#include "bcm1250PciLib.c"
#elif (BCM_FAMILY == BCM_SB1A)
#include "bcm1480PciLib.c"
#else
#error "Unknown BCM_FAMILY"
#endif
