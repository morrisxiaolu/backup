/* vxbMipsIntCtlrSP.h - generic interrupt controller for MIPS CPU */

/*
 * Copyright (c) 2008  Wind River Systems, Inc.
 *
 * The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.
 */

/*
modification history
--------------------
01a,03jun08,h_k  created.
*/

#ifndef __INCvxbMipsIntCtlrSPh
#define __INCvxbMipsIntCtlrSPh

/* typedefs */

typedef struct mipsIntCtlrDrvCtrl
    {
    VXB_DEVICE_ID	pInst;
    BOOL                initialized;
    struct intCtlrHwConf isrHandle; 
    } MIPS_INTCTLR_DRVCTRL;

#endif	/* __INCvxbMipsIntCtlrSPh */
