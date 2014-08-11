/* sysBusPci.c - PCI Autoconfig support */

/* Copyright (c) 1999,2001,2003,2005 Wind River Systems, Inc. All Rights Reserved */

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
01n,05mar07,dtr  Remove unnecessary routines for vxBus.
01m,03jun05,pes  Correct compilation warning. (SPR #109845)
01l,02mar05,pes  Corrected copyright notation
01k,01mar05,pes  Incorporated review comments.
01j,23feb05,pes  Corrected sysParams assignments for 32-bit I/O space. 
01i,15feb05,pes  Added support for MIPS SOC-it (MSC01) system controller.
                 Implemented autodetect of system controller.
01h,08jul03,jmt  Modify for malta20kc support
01g,30oct01,pes  Add entries to intLine array. Replace numeric constants
		 in array with defined constants.
01f,16jul01,pes  Add CoE Copyright comment
01e,15jun01,pes  Corrections to allocations in PCI I/O space
01d,02may01,pes  Change designation of MALTA_PCIMEM1 to MALTA_PCIMEM0 in
                 sysPciAutoConfig.
01c,11apr01,pes  Initial Checkin/Development.
01b,08apr99,dat  SPR 26491, fixed PCI macro names
01a,02feb99,tm   written

*/
/*
DESCRIPTION

*/

/* includes */

#include "vxWorks.h"
#include "logLib.h"
#include "taskLib.h"
#include "config.h"

#include "drv/pci/pciConfigLib.h"
#include "drv/pci/pciAutoConfigLib.h"

/* defines */

/* typedefs */

/* globals */

PCI_SYSTEM sysParams;

#define UNUSED	0xff

/* some hardwired interrupt assignments */
#define USB	IV_PCICD_VEC	/* USB		INTD/DEV10 -> IRQ11 */
#define ENET	IV_PCIAB_VEC	/* Ethernet	INTA/DEV11 -> IRQ10 */
#define AUDIO	IV_PCICD_VEC	/* Audio	INTA/DEV12 -> IRQ11 */

/* accommodate interrupt line twist for 4 PCI slots */
#define PCI1A	IV_PCIAB_VEC	/* PCI slot 1	INTA/DEV18 -> IRQ10 */
#define PCI1B	IV_PCIAB_VEC	/* PCI slot 1	INTB/DEV18 -> IRQ10 */
#define PCI1C	IV_PCICD_VEC	/* PCI slot 1	INTC/DEV18 -> IRQ11 */
#define PCI1D	IV_PCICD_VEC	/* PCI slot 1	INTD/DEV18 -> IRQ11 */

#define PCI2A	IV_PCIAB_VEC	/* PCI slot 2	INTB/DEV18 -> IRQ10 */
#define PCI2B	IV_PCICD_VEC	/* PCI slot 2	INTC/DEV18 -> IRQ11 */
#define PCI2C	IV_PCICD_VEC	/* PCI slot 2	INTD/DEV18 -> IRQ11 */
#define PCI2D	IV_PCIAB_VEC	/* PCI slot 2	INTA/DEV18 -> IRQ10 */

#define PCI3A	IV_PCICD_VEC	/* PCI slot 3	INTC/DEV18 -> IRQ11 */
#define PCI3B	IV_PCICD_VEC	/* PCI slot 3	INTD/DEV18 -> IRQ11 */
#define PCI3C	IV_PCIAB_VEC	/* PCI slot 3	INTA/DEV18 -> IRQ10 */
#define PCI3D	IV_PCIAB_VEC	/* PCI slot 3	INTB/DEV18 -> IRQ10 */

#define PCI4A	IV_PCICD_VEC	/* PCI slot 4	INTD/DEV18 -> IRQ11 */
#define PCI4B	IV_PCIAB_VEC	/* PCI slot 4	INTA/DEV18 -> IRQ10 */
#define PCI4C	IV_PCIAB_VEC	/* PCI slot 4	INTB/DEV18 -> IRQ10 */
#define PCI4D	IV_PCICD_VEC	/* PCI slot 4	INTC/DEV18 -> IRQ11 */

static UCHAR intLine [][4] =
    {
 /*   IntA    IntB    IntC    IntD   - at the int ctlr point of view  */
    { UNUSED, UNUSED, UNUSED, UNUSED },  /* dev number 0 */
    { UNUSED, UNUSED, UNUSED, UNUSED },  /* dev number 1 */
    { UNUSED, UNUSED, UNUSED, UNUSED },  /* dev number 2 */
    { UNUSED, UNUSED, UNUSED, UNUSED },  /* dev number 3 */
    { UNUSED, UNUSED, UNUSED, UNUSED },  /* dev number 4 */
    { UNUSED, UNUSED, UNUSED, UNUSED },  /* dev number 5 */
    { UNUSED, UNUSED, UNUSED, UNUSED },  /* dev number 6 */
    { UNUSED, UNUSED, UNUSED, UNUSED },  /* dev number 7 */
    { UNUSED, UNUSED, UNUSED, UNUSED },  /* dev number 8 */
    { UNUSED, UNUSED, UNUSED, UNUSED },  /* dev number 9 */
    { UNUSED, UNUSED, UNUSED, USB    },  /* dev number 10 - USB via PIIX   */
    { ENET  , UNUSED, UNUSED, UNUSED },  /* dev number 11 - AMD Am79C973   */
    { AUDIO , UNUSED, UNUSED, UNUSED },  /* dev number 12 - Crystal CS4281 */
    { UNUSED, UNUSED, UNUSED, UNUSED },  /* dev number 13 */
    { UNUSED, UNUSED, UNUSED, UNUSED },  /* dev number 14 */
    { UNUSED, UNUSED, UNUSED, UNUSED },  /* dev number 15 */
    { UNUSED, UNUSED, UNUSED, UNUSED },  /* dev number 16 */
    { UNUSED, UNUSED, UNUSED, UNUSED },  /* dev number 17 */
    { PCI1A , PCI1B , PCI1C , PCI1D  },  /* dev number 18 - PCI slot 1*/
    { PCI2A , PCI2B , PCI2C , PCI2D  },  /* dev number 19 - PCI slot 2*/
    { PCI3A , PCI3B , PCI3C , PCI3D  },  /* dev number 20 - PCI slot 3*/
    { PCI4A , PCI4B , PCI4C , PCI4D  }   /* dev number 21 - PCI slot 4*/
};

/* locals */

/* forward declarations */

LOCAL UCHAR sysPciAutoConfigIntAsgn ( PCI_SYSTEM * pSys, PCI_LOC * pFunc,
    UCHAR intPin );
LOCAL STATUS sysPciAutoConfigInclude ( PCI_SYSTEM *pSys, PCI_LOC *pciLoc,
    UINT devVend );

/* subroutines */

/******************************************************************************
*
* sysPciAutoConfigInclude - Determine if function is to be autoConfigured
*
* This function is called with PCI bus, device, function, and vendor 
* information.  It returns an indication of whether or not the particular
* function should be included in the automatic configuration process.
* This capability is useful if it is desired that a particular function
* NOT be automatically configured.  Of course, if the device is not
* included in automatic configuration, it will be unusable unless the
* user's code made provisions to configure the function outside of the
* the automatic process.
*
* RETURNS: TRUE if function is to be included in automatic configuration,
* FALSE otherwise.
*/
 
LOCAL STATUS sysPciAutoConfigInclude
    (
    PCI_SYSTEM *pSys,		/* input: AutoConfig system information */
    PCI_LOC *pciLoc,		/* input: PCI address of this function */
    UINT     devVend		/* input: Device/vendor ID number      */
    )
    {
    BOOL retVal = OK;
    
    /* If it's the host bridge then exclude it */

    if ((pciLoc->bus == 0) && (pciLoc->device == 0) && (pciLoc->function == 0))
	return ERROR;


    switch(devVend)
	{

	/* TODO - add any excluded devices by device/vendor ID here */

#if defined(_WRS_MIPS_20KC)
        case 0x0001Df53:   /* Bonito Chip */
	    retVal = ERROR;
	    break;
#endif

	default:
	    retVal = OK;
	    break;
	}

    return retVal;
    }

/******************************************************************************
*
* sysPciAutoConfigIntAssign - Assign the "interrupt line" value
*
* RETURNS: "interrupt line" value.
*
*/

LOCAL UCHAR sysPciAutoConfigIntAsgn
    ( 
    PCI_SYSTEM * pSys,		/* input: AutoConfig system information */
    PCI_LOC * pFunc,
    UCHAR intPin 		/* input: interrupt pin number */
    )
    {
    UCHAR irqValue = 0xff;    /* Calculated value                */


    if (intPin == 0) 
	return irqValue;

    irqValue = intLine [(pFunc->device)][(intPin - 1)];

    /* TODO - add any non-standard interrupt assignments here */

    PCI_AUTO_DEBUG_MSG("intAssign called for device [%d %d %d] IRQ: %d\n",
		pFunc->bus, pFunc->device, pFunc->function,
		irqValue, 0, 0 );

    return (irqValue);
    }

#ifndef INCLUDE_PCI_BUS 
/*******************************************************************************
*
* sysPciAutoConfig - PCI autoConfig support routine
*
* This routine instantiates the PCI_SYSTEM structure needed to configure
* the system. This consists of assigning address ranges to each category
* of PCI system resource: Prefetchable and Non-Prefetchable 32-bit Memory, and
* 16- and 32-bit I/O. Global values for the Cache Line Size and Maximum
* Latency are also specified. Finally, the four supplemental routines for 
* device inclusion/exclusion, interrupt assignment, and pre- and
* post-enumeration bridge initialization are specified. 
*
* RETURNS: N/A
*/

void sysPciAutoConfig (void)
    {
#if defined (INCLUDE_BONITO64_BRIDGE)
    UINT32 coreId =
      (*((UINT32 *)KM_TO_K1(MALTA_REVISION)) & MALTA_REVISION_CORID_MSK) >>
      MALTA_REVISION_CORID_SHF;
#endif /* INCLUDE_BONITO64_BRIDGE */

    /* 32-bit Non-prefetchable Memory Space */
 
    sysParams.pciMemIo32 = MALTA_PCIMEM0_BASE;
    sysParams.pciMemIo32Size = MALTA_PCIMEM0_SIZE;

#if defined (INCLUDE_BONITO64_BRIDGE)
    if (coreId == MIPS_REVISION_CORID_BONITO64)
	{
	sysParams.pciMemIo32 = BONITO_PCILO_BASE;
	sysParams.pciMemIo32Size = BONITO_PCILO_SIZE;
	}
#endif /* INCLUDE_BONITO64_BRIDGE */


    /* 32-bit Prefetchable Memory Space */
 
    sysParams.pciMem32= 0;
    sysParams.pciMem32Size = 0;
 
    /* Reserve the bottom 16K of PCIIO0 space for legacy devices on
       the South bridge. Then allocate another 16K for 16-bit autoconfig'ed
       I/O and the remainder for 32-bit autoconfig'ed I/O */

    /* 16-bit Autoconfig'ed I/O Space: 16K at 16K offset */
    sysParams.pciIo16 = MALTA_PCIIO0_BASE + MALTA_PCIIO0_16BIT_OFFSET;
    sysParams.pciIo16Size = MALTA_PCIIO0_16BIT_SIZE;

#if defined (INCLUDE_BONITO64_BRIDGE)
    if (coreId == MIPS_REVISION_CORID_BONITO64)
	sysParams.pciIo16 = BONITO_PCIIO_BASE + MALTA_PCIIO0_16BIT_OFFSET;
#endif /* INCLUDE_BONITO64_BRIDGE */
    

    /* 32-bit PCI I/O Space: 32K at 32K offset */
 
    sysParams.pciIo32 = MALTA_PCIIO0_BASE + MALTA_PCIIO0_32BIT_OFFSET;
    sysParams.pciIo32Size = MALTA_PCIIO0_32BIT_SIZE;

#if defined (INCLUDE_BONITO64_BRIDGE)
    if (coreId == MIPS_REVISION_CORID_BONITO64)
	sysParams.pciIo32 = BONITO_PCIIO_BASE + MALTA_PCIIO0_32BIT_OFFSET;
#endif /* INCLUDE_BONITO64_BRIDGE */

    /* Configuration space parameters */
 
    sysParams.cacheSize = (_CACHE_ALIGN_SIZE/4);
    sysParams.maxLatency = PCI_LAT_TIMER;
    sysParams.autoIntRouting = TRUE;
    sysParams.includeRtn = sysPciAutoConfigInclude;
    sysParams.intAssignRtn = sysPciAutoConfigIntAsgn;
    sysParams.bridgePreConfigInit = NULL;
    sysParams.bridgePostConfigInit = NULL;
    sysParams.autoIntRouting = TRUE;
 
    /* Perform AutoConfig */
 
    pciAutoConfig (&sysParams);
 
    return;
    }
#endif
