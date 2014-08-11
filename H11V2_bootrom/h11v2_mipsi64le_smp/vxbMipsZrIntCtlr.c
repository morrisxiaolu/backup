/* vxbMipsZrIntCtlr.c - interrupt controller for BCM1480 */

/*
 * Copyright (c) 2007-2008 Wind River Systems, Inc. All rights are reserved.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
\INTERNAL
********************************************************************
*  Copyright 2000,2001
*  Broadcom Corporation. All rights reserved.
*
*  This software is furnished under license to Wind River Systems, Inc.
*  and may be used only in accordance with the terms and conditions
*  of this license.  No title or ownership is transferred hereby.
********************************************************************* */

/* 
modification history
--------------------
01q,04sep08,jpb  Renamed VSB header file
01p,03sep08,slk  fix vxbZrIpiDisconnect and vxbZrIpiDisable for support of 32
                 IPIs (defect 132891)
01o,18jun08,jpb  Renamed _WRS_VX_SMP to _WRS_CONFIG_SMP.  Added include path
                 for kernel configurations options set in vsb.
01n,11jun08,pmr  resource documentation
01m,03jun08,h_k  split ISR and IPI Emit portions out.
                 cleaned up compiler warnings in GNU.
                 removed redundant zero clear after hwMemAlloc().
01l,05may08,tor  update version
01k,22apr08,slk  modify for IPI support in SMP and AMP. also increase
                 supported IPIs to 32.
01j,22feb08,h_k  optimized IPI handling.
01i,11feb08,h_k  fixed vxIpiDisconnect. (CQ:116800)
01h,20sep07,tor  VXB_VERSION_3
01g,05sep07,p_g  modification done for ApiGen
01f,21jun07,slk  add support for vxBus IPIs
01e,31may07,h_k  changed to library build.
01d,23may07,jmt  Add support for AMP images.
01c,16may07,slk  clear mailbox interrupt only when staus bit for interrupt is
                 set
01b,01may07,slk  add interrupt routing to all available cpus
01a,04jan07,slk  created
*/

/*
DESCRIPTION

This file supplies routines that handle the vxbus interrupt controller
requirements for the Broadcom sibyte family of targets.  This driver
handles interrupt connect, disconnect, enable and disable functions.

Functions are published for interrupt connect, disconnect, enable, disable
and acknowledge.  The file hwconf.c in the BSP directory is used as the
configuration source.  It defines the device inputs into the interrupt
controller and controller output pin crossconnects and destination cpu
devices.

This file is included by cmdLineBuild.c when DRV_INTCTLR_MIPS_SBE
is defined.  The macro DRV_INTCTLR_MIPS also needs to be defined to include
the registration of the generic MIPS interrupt controller and 
INCLUDE_INTCTLR_LIB to bring in the vxbus interrupt controller library.

INCLUDE FILES:
vxbIntCtlrLib.h hwConf.h vxBus.h vxbPlbLib.h vxbAccess.h
*/

/* includes */

#include <vxWorks.h>
#include <vsbConfig.h>
#include <intLib.h>
#include <vxLib.h>
#include <stdio.h>
#include <arch/mips/excMipsLib.h>
#include <dllLib.h>
#include <sysLib.h>
#include <stdlib.h>
#include <string.h>
#include <sllLib.h>
#include <spinLockLib.h>

#include <vxBusLib.h>
#include <hwif/vxbus/hwConf.h>
#include <hwif/util/hwMemLib.h>
#include <hwif/vxbus/vxBus.h>
#include <hwif/vxbus/vxbPlbLib.h>
#include "../h/vxbus/vxbAccess.h"
#include <../src/hwif/intCtlr/vxbIntCtlrLib.h>

#include <vxIpiLib.h>
#include <taskLib.h>
#ifdef _WRS_CONFIG_SMP
#include <private/cpcLibP.h>
#endif

#include "vxbMipsZrIntCtlrSP.h"
#include "simpleprintf.h" /** 20091210 **/
//#include "zrBsp.h" /*add 20101112 avoid warning*/

/* defines */

#define BCM_SB1         1
#define BCM_SB1A        2

/* define IPI mailbox number */

#define R_IMR_INTERRUPT_MAP_COUNT	((vxbBcmFamily == BCM_SB1) ? R_IMR_INTERRUPT_MAP_COUNT_SB1 : R_IMR_INTERRUPT_MAP_COUNT_SB1A)

#define MBOX_INT_0	((vxbBcmFamily == BCM_SB1) ? \
			 SB1_MBOX_INT_0 : SB1A_MBOX_INT_0)
#define MBOX_INT_1	((vxbBcmFamily == BCM_SB1) ? \
			 SB1_MBOX_INT_1 : SB1A_MBOX_INT_1)
#define MBOX_INT_2	((vxbBcmFamily == BCM_SB1) ? \
			 SB1_MBOX_INT_2 : SB1A_MBOX_INT_2)
#define MBOX_INT_3	((vxbBcmFamily == BCM_SB1) ? \
			 SB1_MBOX_INT_3 : SB1A_MBOX_INT_3)

/* the ipi that's used for the CPC (IPI_INTR_ID_CPC) in SMP mode comes
 * in on bit 48 which translates to input pin MBOX_INT_0.  this IPI
 * must be the only bit assigned in a group of 16 bits (48-63).
 * all other ipis are assigned a bit position that is the same as the 
 * IPI number.  this places IPIs 1-15 on input pin MBOX_INT_3 and IPIs
 * 16-31 on input pin BOX_INT_2.  if IPIs are extended above 31 (32 total)
 * they would be assigned to input pin MBOX_INT_1
 */
/* changed by yinwx, 20100302 */
#if 1
#define MAILBOX_INPUT_PIN(ipi)					\
		((ipi==IPI_INTR_ID_CPC)?MBOX_INT_0:		\
		(ipi<MBOX_CHANNEL_SPACING)?MBOX_INT_3:		\
		(ipi<(MBOX_CHANNEL_SPACING*2))?MBOX_INT_2:	\
		MBOX_INT_1)
#else
#define MAILBOX_INPUT_PIN(ipi)					\
		((ipi==IPI_INTR_ID_CPC)?MBOX_INT_0:		\
		(ipi==4)?MBOX_INT_3:		\
		(ipi==IPI_INTR_ID_SCHED)?MBOX_INT_2:	\
		MBOX_INT_1)
#endif
/* IPI input pin storage index number for ISRs called from IPI demux */

#define IPI_ISR_INPUT_PIN(ipi)		(R_IMR_INTERRUPT_MAP_COUNT+ipi)

#define VXB_MIPS3_SD_BASE(reg, intrSource, intrLine)			\
    {									\
    if (vxbBcmFamily == BCM_SB1)					\
	MIPS3_SD(&SB1_INT_CTLR(reg)[intrSource], intrLine);		\
    else								\
	MIPS3_SD(&SB1_INT_CTLR(reg)[INT_SRC_OFFSET(intrSource)] + 	\
	INT_REG_OFFSET(intrSource), intrLine);				\
    }

#define VXB_MIPS3_SD(reg, intrSource, value)				\
    {									\
    if (vxbBcmFamily == BCM_SB1)					\
	MIPS3_SD(SB1_INT_CTLR(reg), value);				\
    else								\
	MIPS3_SD(SB1_INT_CTLR(reg) + INT_REG_OFFSET(intrSource),	\
		 value);					 	\
    }

#define VXB_MIPS3_LD(reg, intrSource, value)				\
    {									\
    if (vxbBcmFamily == BCM_SB1)					\
	value = MIPS3_LD(SB1_INT_CTLR(reg));				\
    else								\
	value = MIPS3_LD(SB1_INT_CTLR(reg) + 				\
			 INT_REG_OFFSET(intrSource));			\
    }

/* forward declarations */

#define  C0_IntrRoute     0x3ff01400
#define  C0_Intisr        0x3ff01420
#define  C0_Inten         0x3ff01424
#define  C0_Intenset      0x3ff01428
#define  C0_Intenclr      0x3ff0142c
#define  C0_Intedge       0x3ff01438
#define  C0_C0_Intisr     0x3ff01440
#define  C0_C1_Intisr     0x3ff01448
#define  C0_C2_Intisr     0x3ff01450
#define  C0_C3_Intisr     0x3ff01458

/* Zr interrupt controller functions */

LOCAL STATUS sb1IntConnectCore (int, int, int);
LOCAL STATUS sb1IntEnableCore (int, int);
LOCAL STATUS sb1IntDisconnectCore (int, int);
LOCAL STATUS sb1IntDisableCore (int, int);

/* vxBus initialization functions */

void vxbMipsZrIntCtlrRegister(void);
LOCAL void vxbMipsZrIntCtlrInstInit (VXB_DEVICE_ID pInst);
LOCAL void vxbMipsZrIntCtlrInstInit2 (VXB_DEVICE_ID pInst);


/* Zr interrupt controller functions */

LOCAL STATUS vxbMipsZrIntCtlrConnect
    (
    VXB_DEVICE_ID       pIntCtlr,
    VXB_DEVICE_ID       pDev,
    int                 index,
    void                (*pIsr)(void * pArg),
    void *              pArg,
    int *               pInputPin
    );
LOCAL STATUS vxbMipsZrIntCtlrDisconnect
    (
    VXB_DEVICE_ID       pIntCtlr,
    VXB_DEVICE_ID       pDev,
    int                 index,
    VOIDFUNCPTR         pIsr,
    void *              pArg
    );
LOCAL STATUS vxbMipsZrIntCtlrEnable
    (
    VXB_DEVICE_ID       pIntCtlr,
    VXB_DEVICE_ID       pDev,
    int                 index,
    VOIDFUNCPTR         pIsr,
    void *              pArg
    );
LOCAL STATUS vxbMipsZrIntCtlrDisable
    (
    VXB_DEVICE_ID       pIntCtlr,
    VXB_DEVICE_ID       pDev,
    int                 index,
    VOIDFUNCPTR         pIsr,
    void *              pArg
    );

#ifdef _WRS_CONFIG_SMP
LOCAL STATUS vxbMipsZrIntCtlrCpuReroute
    (
    VXB_DEVICE_ID       pDev,
    void *              destCpu
    );
LOCAL STATUS vxbMipsZrIntCtlrIntReroute
    (
    VXB_DEVICE_ID       pDev,
    int                 index,
    cpuset_t            destCpu
    );
#endif /* _WRS_CONFIG_SMP */
LOCAL VXIPI_CTRL_INIT * vxbMipsZrIpiControlGet
    (
    VXB_DEVICE_ID       pDev,
    void *              pArg
    );

#ifdef MIPS_SBE_INTCTLR_DEBUG_ON
LOCAL void mipsZrIntCtlrShow
    (
    VXB_DEVICE_ID       pInst,
    int                 verboseLevel
    );
#endif /* MIPS_SBE_INTCTLR_DEBUG_ON */

/* locals */

LOCAL struct drvBusFuncs mipsZrIntCtlrFuncs =
    {
    vxbMipsZrIntCtlrInstInit,        /* devInstanceInit */
    vxbMipsZrIntCtlrInstInit2,       /* devInstanceInit2 */
    NULL                           /* devConnect */
    };

LOCAL int vxbBcmFamily;
LOCAL SPIN_LOCK_ISR_DECL(vxbMipsZrIntCtlrLock, SPIN_LOCK_EMPTY);

LOCAL struct vxbDevRegInfo mipsZrIntCtlrDevRegistration =
    {
    NULL,                 /* pNext */
    VXB_DEVID_DEVICE,     /* devID */
    VXB_BUSID_PLB,        /* busID = PLB */
    VXB_VER_4_0_0,      /* busVer */
    "mipsZrIntCtlr",     /* drvName */
    &mipsZrIntCtlrFuncs, /* pDrvBusFuncs */
    NULL,                 /* pMethods */
    NULL                  /* devProbe */
    };


/* import the method _desc strings for the published methods */

METHOD_DECL(vxbIntCtlrConnect)
METHOD_DECL(vxbIntCtlrDisconnect)
METHOD_DECL(vxbIntCtlrEnable)
METHOD_DECL(vxbIntCtlrDisable)
#ifdef _WRS_CONFIG_SMP
METHOD_DECL(vxbIntCtlrCpuReroute)
METHOD_DECL(vxbIntCtlrIntReroute)
#endif /* _WRS_CONFIG_SMP */
METHOD_DECL(vxIpiControlGet)

#ifdef MIPS_SBE_INTCTLR_DEBUG_ON
METHOD_DECL(busDevShow)
#endif /* MIPS_SBE_INTCTLR_DEBUG_ON */

LOCAL device_method_t mipsZrIntCtlr_methods[] =
    {
    DEVMETHOD(vxbIntCtlrConnect, vxbMipsZrIntCtlrConnect),
    DEVMETHOD(vxbIntCtlrEnable, vxbMipsZrIntCtlrEnable),
    DEVMETHOD(vxbIntCtlrDisconnect, vxbMipsZrIntCtlrDisconnect),
    DEVMETHOD(vxbIntCtlrDisable, vxbMipsZrIntCtlrDisable),
#ifdef _WRS_CONFIG_SMP
    DEVMETHOD(vxbIntCtlrCpuReroute, vxbMipsZrIntCtlrCpuReroute),
    DEVMETHOD(vxbIntCtlrIntReroute, vxbMipsZrIntCtlrIntReroute),
#endif /* _WRS_CONFIG_SMP */
    DEVMETHOD(vxIpiControlGet,  vxbMipsZrIpiControlGet),

#ifdef MIPS_SBE_INTCTLR_DEBUG_ON
    DEVMETHOD(busDevShow, mipsZrIntCtlrShow),
#endif /* MIPS_SBE_INTCTLR_DEBUG_ON */

    { 0, 0}
    };

/* imports */
IMPORT int bslProcGetId (void);
IMPORT int    vxCpuIdGet(void);
IMPORT void   intCtlrStrayISR (void *, int);
IMPORT int intCtlrPinFind (VXB_DEVICE_ID                pDev,
                           int                          pinIndex,
                           VXB_DEVICE_ID                pIntCtlrCaller,
                           struct intCtlrHwConf *       pConf);
IMPORT void vxbZr1Int(int intr_line);
IMPORT void vxbZr1aInt(int intr_line);


//IMPORT void sysSmInt(int parameter); /*add by wangzx avoid warning*/

/* vxBus IPI support functions */

IMPORT STATUS vxbZrIpiEmit (VXB_DEVICE_ID pCtlr, INT32 ipiIntrptId, 
                           cpuset_t processor);
LOCAL STATUS vxbZrIpiConnect(VXB_DEVICE_ID pCtlr, INT32 ipiIntrptId, 
                             IPI_HANDLER_FUNC routine, void * parameter);
LOCAL STATUS vxbZrIpiDisconnect(VXB_DEVICE_ID pCtlr, INT32 ipiIntrptId, 
                             IPI_HANDLER_FUNC routine, void * parameter);
LOCAL STATUS vxbZrIpiEnable(VXB_DEVICE_ID pCtlr, INT32 ipiIntrptId);
LOCAL STATUS vxbZrIpiDisable(VXB_DEVICE_ID pCtlr, INT32 ipiIntrptId);
LOCAL INT32 vxbZrIpiPrioGet(VXB_DEVICE_ID pCtlr, INT32 ipiIntrptId);
LOCAL STATUS vxbZrIpiPrioSet(VXB_DEVICE_ID pCtlr, INT32 ipiIntrptId, INT32 prio);

/* globals */

_WRS_DATA_ALIGN_BYTES(32) INT_CTLR vxbZrIntCtlr[VX_MAX_SMP_CPUS];
MIPS_SB_INT_CTLR_DRVCTRL * pVxbPMipsZrIntCtlrDrvCtrl;
unsigned int vxbPMipsZrIntSourceTbl[4][4];

/*
 * This structure is initialized with the control functions for the IPI
 * interface.These set of functions allow the CPC layer to manipulate IPI
 * interrupts.
 */

VXIPI_CTRL_INIT vxIpiCtrlInit =
    {
    {NULL},
    0,
    vxbZrIpiEmit,
    vxbZrIpiConnect,
    vxbZrIpiEnable,
    vxbZrIpiDisable,
    vxbZrIpiDisconnect,
    vxbZrIpiPrioGet,
    vxbZrIpiPrioSet,
    MAX_MBOX_INT_CHANNELS, /* IPI count (CPC , debugger and 2 spares) */
    0  /* VXB_DEVICE_ID of this int controllers */
    };


/******************************************************************************
*
* vxbMipsZrIntCtlrRegister - register the mipsIntCtlr driver
*
* This routine registers the SB interrupt controller  driver and device
* recognition data with the vxBus subsystem.
*
* RETURNS: N/A
*
* ERRNO
*/

void vxbMipsZrIntCtlrRegister(void)
    {
    vxbDevRegister((struct vxbDevRegInfo *)&mipsZrIntCtlrDevRegistration);
    }

/******************************************************************************
*
* vxbMipsZrIntCtlrInstInit - initialize sibyte interrupt controller  device
*
* This is the sibyte intr ctlr initialization routine.  It create the
* driver control structure for the supplied device.  It also scans the hwconf.c
* file for the specific pin data for this device and publishes the methods
* for the device's control.
*
* RETURNS: N/A
*
* ERRNO
*/

LOCAL void vxbMipsZrIntCtlrInstInit
    (
    VXB_DEVICE_ID pInst
    )
    {
    /* get the HCF_DEVICE address */

    HCF_DEVICE * pHcf = hcfDeviceGet (pInst);
    /*printstr("vxbMipsZrIntCtlrInstInit.... \r\n");*/
    if (pHcf == NULL)
	return;

    /* get the BCM_FAMILY type. */

    /*
     * resourceDesc {
     * The bcmFamily resource must be provided in order to relay
     * the BCM_FAMILY configuration macro to the driver. }
     */
    if (devResourceGet (pHcf, "bcmFamily", HCF_RES_INT, (void *) &vxbBcmFamily)
	!= OK)
	return;

    /* create DrvCtrl structure for core */

    pVxbPMipsZrIntCtlrDrvCtrl = (MIPS_SB_INT_CTLR_DRVCTRL *)hwMemAlloc \
                              (sizeof(MIPS_SB_INT_CTLR_DRVCTRL));

    /* verify alloc worked */

    if (pVxbPMipsZrIntCtlrDrvCtrl == NULL)
        return;

    pVxbPMipsZrIntCtlrDrvCtrl->pInst = pInst;
    pInst->pDrvCtrl = pVxbPMipsZrIntCtlrDrvCtrl;

    /* set enabledCpus to show this CPU is running and available
     * for interrupts
     */

    pVxbPMipsZrIntCtlrDrvCtrl->enabledCpus = (1 << vxCpuIdGet());

    /* default CPU is the one that initialized the controller */

    pVxbPMipsZrIntCtlrDrvCtrl->defaultCpu = vxCpuIdGet();

    /* get connectivity info from hwconf */

    intCtlrHwConfGet(pInst,
                (HCF_DEVICE *)pInst->pBusSpecificDevInfo,
                &(pVxbPMipsZrIntCtlrDrvCtrl->isrHandle));

    /* advertise intConnect() and other methods */

    pInst->pMethods = &mipsZrIntCtlr_methods[0];
    }

/******************************************************************************
*
* vxbMipsZrIntCtlrInstInit2 - initialize sibyte interrupt controller
*
* Initialize the sibyte interrupt controller
*
* RETURNS: N/A
*
* ERRNO
*/

LOCAL void vxbMipsZrIntCtlrInstInit2 
    (
    VXB_DEVICE_ID pInst
    )
    {
    unsigned int cpunum, i;
    VOIDFUNCPTR func;
#if 0 /* mxl del */
	printstr("vxbMipsZrIntCtlrInstInit2.... \r\n");
#endif
    if (pVxbPMipsZrIntCtlrDrvCtrl->initialized == TRUE)
        return;

#if defined (_WRS_CONFIG_SMP)
    /* early exit if not core 0 */
    if (vxCpuIdGet())
        return;
#endif /* _WRS_CONFIG_SMP */
#if defined (_WRS_CONFIG_SMP)
    /* Walk through all cores, initializing their pointers */
    for (cpunum = 0; cpunum < SB_NUM_CORES; cpunum++)
#else /* AMP or UP single core */
    cpunum = pVxbPMipsZrIntCtlrDrvCtrl->defaultCpu;
#endif /* defined (_WRS_CONFIG_SMP) */
        {
#if 0 /* mxl del */
        printstr("cpunum = "); printnum(cpunum); printstr("\r\n");
#endif
        SB1_INT_CTLR(bcm1xxxIntMaskH) =
          (void *)IMR_REGISTER(R_IMR_INTERRUPT_MASK_H); /* intenclr */
        SB1_INT_CTLR(bcm1xxxIntStatusBaseH) =
          (void *)IMR_REGISTER(R_IMR_INTERRUPT_STATUS_BASE_H);
        SB1_INT_CTLR(bcm1xxxLdtIntClearH) =
          (void *)IMR_REGISTER(R_IMR_LDT_INTERRUPT_CLR_H);
        SB1_INT_CTLR(bcm1xxxIntMapBaseH) =
          (void *)IMR_REGISTER(R_IMR_INTERRUPT_MAP_BASE_H);

        /* Mask off all interrupts (they'll be enabled as needed later). */
        /* MIPS3_SD(SB1_INT_CTLR(bcm1xxxIntMaskH), 0xffffffffffffffffULL); */
		MIPS_SW64((unsigned int)SB1_INT_CTLR(bcm1xxxIntMaskH),0xffffffff); /* 20091210 */

	if (vxbBcmFamily != BCM_SB1)
	    {
            SB1_INT_CTLR(bcm1xxxIntMaskL) =
              (void *)IMR_REGISTER(R_IMR_INTERRUPT_MASK_L); /* intenset */
            SB1_INT_CTLR(bcm1xxxIntStatusBaseL) =
              (void *)IMR_REGISTER(R_IMR_INTERRUPT_STATUS_BASE_L);
            SB1_INT_CTLR(bcm1xxxLdtIntClearL) =
              (void *)IMR_REGISTER(R_IMR_LDT_INTERRUPT_CLR_L);
            SB1_INT_CTLR(bcm1xxxIntMapBaseL) =
              (void *)IMR_REGISTER(R_IMR_INTERRUPT_MAP_BASE_L);

            /* Mask off all interrupts (they'll be enabled as needed later). */
            /*MIPS3_SD(SB1_INT_CTLR(bcm1xxxIntMaskL), 0xffffffffffffffffULL);*/
	    }
#if 0 /* mxl del */
	printstr("bcm1xxxIntMaskH = "); printnum((unsigned long long)SB1_INT_CTLR(bcm1xxxIntMaskH)); printstr("\r\n");
	printstr("bcm1xxxIntStatusBaseH = "); printnum((unsigned long long)SB1_INT_CTLR(bcm1xxxIntStatusBaseH)); printstr("\r\n");
	printstr("bcm1xxxLdtIntClearH = "); printnum((unsigned long long)SB1_INT_CTLR(bcm1xxxLdtIntClearH)); printstr("\r\n");
	printstr("bcm1xxxIntMapBaseH = "); printnum((unsigned long long)SB1_INT_CTLR(bcm1xxxIntMapBaseH)); printstr("\r\n");
	printstr("bcm1xxxIntMaskL = "); printnum((unsigned long long)SB1_INT_CTLR(bcm1xxxIntMaskL)); printstr("\r\n");
	printstr("bcm1xxxIntStatusBaseL = "); printnum((unsigned long long)SB1_INT_CTLR(bcm1xxxIntStatusBaseL)); printstr("\r\n");
	printstr("bcm1xxxLdtIntClearL = "); printnum((unsigned long long)SB1_INT_CTLR(bcm1xxxLdtIntClearL)); printstr("\r\n");
	printstr("bcm1xxxIntMapBaseL = "); printnum((unsigned long long)SB1_INT_CTLR(bcm1xxxIntMapBaseL)); printstr("\r\n");
	printstr("================================================\r\n");
#endif
	}

    pVxbPMipsZrIntCtlrDrvCtrl->initialized = TRUE;

    /* setup vxbZrInt as the ISR for all interrupts coming from this
     * interrupt controller
     */

    if (vxbBcmFamily == BCM_SB1)
	func = vxbZr1Int;
    else
	func = vxbZr1aInt;

#if defined (_WRS_CONFIG_SMP)
    /* Walk through all cores, initializing their pointers */
    for (cpunum = 0; cpunum < SB_NUM_CORES; cpunum++)
#else /* AMP or UP single core */
    cpunum = 0;  /* cpu number for single core is always 0 */
#endif /* defined (_WRS_CONFIG_SMP) */
        {
	for (i = 0; i < 6; i++) /* 6 INTLINES? */
	    {
	    vxbIntConnect (pInst, MIPS_SB_CPU_OUTPUT_PIN((int)cpunum, i),
			   func, (void *)i);
	    vxbIntEnable (pInst, MIPS_SB_CPU_OUTPUT_PIN((int)cpunum, i),
			  func, (void *)i);
	    }
        }
    }


/******************************************************************************
*
* vxbMipsZrIntCtlrConnect - connect device interrupt
*
* configure the hardware and attach the supplied routine and arg to the
* interrupt for this device.  this routine handles the configuration of
* the SBE interrupt controller to direct the interrupt source to the desired
* cpu and pin.  it also calls the upstream interrupt controller to attach the
* routine and arg to the requested cpu and pin.
*
* RETURNS: OK if operation successful else ERROR
*
* ERRNO
*/

LOCAL STATUS vxbMipsZrIntCtlrConnect
    (
    VXB_DEVICE_ID       pIntCtlr,
    VXB_DEVICE_ID       pDev,
    int                 index,
    void                (*pIsr)(void * pArg),
    void *              pArg,
    int *               pInputPin
    )
    {
    int     inputPin;
    int     outputPin;
    int     destCpu;
    
    if (pVxbPMipsZrIntCtlrDrvCtrl->initialized == FALSE)
        vxbMipsZrIntCtlrInstInit2 (pIntCtlr);

    /* get interrupt input pin and make sure it is valid */

  
    inputPin =
        intCtlrPinFind (pDev, index, pIntCtlr, 
                        &(pVxbPMipsZrIntCtlrDrvCtrl->isrHandle));

    
    
    if (inputPin == ERROR)
        return (ERROR);

    *pInputPin = inputPin;

    /* get the output pin number */

    MIPS_SBE_OUTPUTPIN(&pVxbPMipsZrIntCtlrDrvCtrl->isrHandle,inputPin,
                       outputPin);
    

#ifdef _WRS_CONFIG_SMP
    /* and destination CPU */

    MIPS_SBE_DESTCPU(&pVxbPMipsZrIntCtlrDrvCtrl->isrHandle,inputPin,
                       destCpu);
    
     /* make sure destination CPU is enabled, if not route interrupt 
     * to default CPU
     */

    /*if (((1 << destCpu) & pVxbPMipsZrIntCtlrDrvCtrl->enabledCpus) == 0)
        destCpu = pVxbPMipsZrIntCtlrDrvCtrl->defaultCpu; comment by wangzx ,we decide all cpu are running*/
#else /* _WRS_CONFIG_SMP */

    /* AMP and UP use default CPU for destination CPU */

    destCpu = pVxbPMipsZrIntCtlrDrvCtrl->defaultCpu;
#endif /* _WRS_CONFIG_SMP */

    /* assign the ISR and arg to the specified interrupt ctrl input source */

    if (intCtlrISRAdd(&pVxbPMipsZrIntCtlrDrvCtrl->isrHandle, inputPin,
                      pIsr, pArg) != OK)
        return (ERROR);
#if 0
    /* do the crossbar connection in the SBE interrupt controller */
	printstr("inputpin is 0x"); printnum(inputPin);printstr("\r\n");
	printstr("outputpin is 0x"); printnum(outputPin);printstr("\r\n");
	printstr("destCpu is 0x");printnum(destCpu);printstr("\r\n");
#endif
    return (sb1IntConnectCore (inputPin, outputPin, destCpu));
    }


/******************************************************************************
*
* vxbMipsZrIntCtlrDisconnect - disconnect device interrupt
*
* disconnect the specified ISR from the cpu input pin and disable the 
* interrupt input route to the destination cpu.
*
* RETURNS: OK if operational successful else ERROR
*
* ERRNO
*/

LOCAL STATUS vxbMipsZrIntCtlrDisconnect
    (
    VXB_DEVICE_ID       pIntCtlr,
    VXB_DEVICE_ID       pDev,
    int                 index,
    VOIDFUNCPTR         pIsr,
    void *              pArg
    )
    {
    int     inputPin;
    int     outputPin = ERROR;
    int     destCpu;

    /* get interrupt controller info */

    inputPin =
        intCtlrPinFind (pDev, index, pIntCtlr, 
                        &(pVxbPMipsZrIntCtlrDrvCtrl->isrHandle));
    if (inputPin == ERROR)
        return (ERROR);

    /* get the destination cpu and output pin numbers */

    MIPS_SBE_OUTPUTPIN(&pVxbPMipsZrIntCtlrDrvCtrl->isrHandle,inputPin,
                       outputPin);

#ifdef _WRS_CONFIG_SMP
    MIPS_SBE_DESTCPU(&pVxbPMipsZrIntCtlrDrvCtrl->isrHandle,inputPin,
                       destCpu);

    /* make sure destination CPU is enabled, if not disconnect
     * route from default CPU
     */

    if (((1 << destCpu) & pVxbPMipsZrIntCtlrDrvCtrl->enabledCpus) == 0)
        destCpu = pVxbPMipsZrIntCtlrDrvCtrl->defaultCpu;
#else /* _WRS_CONFIG_SMP */

    /* AMP and UP use default CPU for destination CPU */

    destCpu = pVxbPMipsZrIntCtlrDrvCtrl->defaultCpu;
#endif /* _WRS_CONFIG_SMP */

    /* remove the ISR from the int ctlr */

    if (intCtlrISRRemove(&pVxbPMipsZrIntCtlrDrvCtrl->isrHandle, inputPin, pIsr,
                          pArg) != OK)
        return (ERROR);

    /* disconnect the crossbar route through the int ctlr */
    
   

    return (sb1IntDisconnectCore (inputPin, destCpu));
    }


/******************************************************************************
*
* vxbMipsZrIntCtlrEnable - enable device interrupt
*
* enable the route of the interrupt input to the destination cpu and mark as
* enabled the ISR attached to the input pin
*
* RETURNS: OK if operational successful else ERROR
*
* ERRNO
*/

LOCAL STATUS vxbMipsZrIntCtlrEnable
    (
    VXB_DEVICE_ID       pIntCtlr,
    VXB_DEVICE_ID       pDev,
    int                 index,
    VOIDFUNCPTR         pIsr,
    void *              pArg
    )
    {
    int inputPin;
    int outputPin;
    int destCpu;

    /*  make sure SB interrupt controller is initialized */
    /*printstr("WWWWWANGZXXXXXXXXX: Begin vxbMipsZrIntCtlrEnable....\r\n");*/
    if (pVxbPMipsZrIntCtlrDrvCtrl->initialized == FALSE)
        vxbMipsZrIntCtlrInstInit2 (pIntCtlr);

    /* get input pin, cputnum and output pin for this device */

    inputPin =
        intCtlrPinFind (pDev, index, pIntCtlr, 
                        &(pVxbPMipsZrIntCtlrDrvCtrl->isrHandle));
    if ( inputPin == ERROR )
    {
        /*printstr("Input Pin Error, intEnable Failed\r\n"); */ /* mxl del */
        return(ERROR);
    }

    MIPS_SBE_OUTPUTPIN(&pVxbPMipsZrIntCtlrDrvCtrl->isrHandle,inputPin,
                       outputPin);

#ifdef _WRS_CONFIG_SMP
    MIPS_SBE_DESTCPU(&pVxbPMipsZrIntCtlrDrvCtrl->isrHandle,inputPin,
                       destCpu);

    /* make sure destination CPU is enabled, if not disconnect
     * route from default CPU
     */

    if (((1 << destCpu) & pVxbPMipsZrIntCtlrDrvCtrl->enabledCpus) == 0)
        destCpu = pVxbPMipsZrIntCtlrDrvCtrl->defaultCpu;
#else /* _WRS_CONFIG_SMP */

    /* AMP and UP use default CPU for destination CPU */

    destCpu = pVxbPMipsZrIntCtlrDrvCtrl->defaultCpu;
#endif /* _WRS_CONFIG_SMP */

    /* set flag that the ISR is enabled */

    if (intCtlrISREnable(&pVxbPMipsZrIntCtlrDrvCtrl->isrHandle, inputPin, pIsr,
                          pArg) != OK)
        return (ERROR);

    /* enable interrupt source route through int ctlr */

    return (sb1IntEnableCore (inputPin, destCpu));
    }

/******************************************************************************
*
* vxbMipsZrIntCtlrDisable - disable device interrupt
*
* disable the route of the interrupt to the destination cpu and mark as
* disabled the ISR attached to the input pin
*
* RETURNS: OK if operational successful else ERROR
*
* ERRNO
*/

LOCAL STATUS vxbMipsZrIntCtlrDisable
    (
    VXB_DEVICE_ID       pIntCtlr,
    VXB_DEVICE_ID       pDev,
    int                 index,
    VOIDFUNCPTR         pIsr,
    void *              pArg
    )
    {
    int inputPin;
    int outputPin;
    int destCpu;

    /* get input pin, output pin and cpu destination for this device */

    inputPin =
        intCtlrPinFind (pDev, index, pIntCtlr, 
                        &(pVxbPMipsZrIntCtlrDrvCtrl->isrHandle));
    if ( inputPin == ERROR )
        return(ERROR);

    MIPS_SBE_OUTPUTPIN(&pVxbPMipsZrIntCtlrDrvCtrl->isrHandle,inputPin,
                       outputPin);

#ifdef _WRS_CONFIG_SMP
    MIPS_SBE_DESTCPU(&pVxbPMipsZrIntCtlrDrvCtrl->isrHandle,inputPin,
                       destCpu);

    /* make sure destination CPU is enabled, if not disconnect
     * route from default CPU
     */

    if (((1 << destCpu) & pVxbPMipsZrIntCtlrDrvCtrl->enabledCpus) == 0)
        destCpu = pVxbPMipsZrIntCtlrDrvCtrl->defaultCpu;
#else /* _WRS_CONFIG_SMP */

    /* AMP and UP use default CPU for destination CPU */

    destCpu = pVxbPMipsZrIntCtlrDrvCtrl->defaultCpu;
#endif /* _WRS_CONFIG_SMP */

    /* set flag that the ISR is disabled */

    if (intCtlrISRDisable(&pVxbPMipsZrIntCtlrDrvCtrl->isrHandle, inputPin, pIsr,
                          pArg) != OK)
        return (ERROR);

    /* disable interrupt source route through interrupt ctlr */

    return (sb1IntDisableCore (inputPin, destCpu));
    }

/* reroute functions are SMP only */

#ifdef _WRS_CONFIG_SMP

/******************************************************************************
*
* vxbMipsZrIntCtlrCpuReroute - reroute interrupts to specified CPU
*
* Reroute interrupts that are configured in hwconf.c for a CPU other than
* the default CPU to that CPU.  Also set enabledCpus in the driver control
* structure with this CPUs ID so it is marked as active
*
* RETURNS: OK if operational successful else ERROR
*
* ERRNO
*/

LOCAL STATUS vxbMipsZrIntCtlrCpuReroute
    (
    VXB_DEVICE_ID       pDev,
    void *              destCpu
    )
    {
    struct intCtlrHwConf *isrHandle = &(pVxbPMipsZrIntCtlrDrvCtrl->isrHandle);
    int i, cpunum, outputPin, configCpu;
    BOOL flag;
    void (*func)();

    /* verify int ctlr device is this controller and valid destination CPU */

    if (pDev != pVxbPMipsZrIntCtlrDrvCtrl->pInst || (int)destCpu >= SB_NUM_CORES)
        return (ERROR);

    /* move interrupts to requested cpu */
   /* printstr("\r\nReroute &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\r\n");
    printstr("dsetCpu is ");printnum((int)destCpu);printstr("\r\n");*/

    for (i = 0;i < SB1_NUM_INT_SOURCES;i++)
        {

        /* verify this is an allocated pin so NULL references are avoided */
        VXB_INTCTLR_PINENTRY_ALLOCATED(isrHandle, i, flag);
        if (flag)
            {
			/*printstr("intSource is ");printnum(i);printstr("\r\n");*/
            /* only move interrupts that are configured with an ISR */
            MIPS_SBE_ISR(isrHandle, i, func);
            if (func == NULL)
            {
            	/*printstr("no function found \r\n");*/
            	continue;
            }
            if(func == intCtlrStrayISR)
            {
            	/*printstr("func is intCtlrStrayISR\r\n");*/
            	continue;
            }
  
            /* move interrupts that are configured for the requested CPU */

            MIPS_SBE_DESTCPU(isrHandle, i, configCpu);
            if (configCpu == (int)destCpu)
            {
                /* disable this interrupt source to all cpus */

                for (cpunum = 0; cpunum < SB_NUM_CORES; cpunum++)
                    sb1IntDisableCore (i, cpunum);

                /* route from input to destination cpu input and enable the
                 * interrupt if currently enabled
                 */

                MIPS_SBE_OUTPUTPIN(isrHandle, i, outputPin);
                sb1IntConnectCore (i, outputPin, (int)destCpu);
                VXB_INTCTLR_PINENTRY_ENABLED(isrHandle, i, flag);
                if (flag)
                    sb1IntEnableCore (i, (int)destCpu);
            }
            else
            {
            	/*printstr("configCpu is ");printnum(configCpu);printstr("not match\r\n");*/
            }
         }
     }

    /* set flag showing this CPU is enabled and interrupts can be
     * routed to it
     */

    pVxbPMipsZrIntCtlrDrvCtrl->enabledCpus |= (1 << (int)destCpu);

    return (OK);
    }


/******************************************************************************
*
* vxbMipsZrIntCtlrIntReroute - reroute interrupt to specified CPU
*
* Reroute device interrupt to requested CPU.  note that the cpu is specified
* in a cpuset_t type.  this would allow for multiple cpus to be specified
* but only 1 cpu being specified is valid.  specifying more than 1 cpu
* will return an ERROR.
*
* RETURNS: OK if operational successful else ERROR
*
* ERRNO
*/

LOCAL STATUS vxbMipsZrIntCtlrIntReroute
    (
    VXB_DEVICE_ID       pDev,
    int                 index,
    cpuset_t            routeToCpu
    )
    {
    struct intCtlrHwConf *isrHandle = &(pVxbPMipsZrIntCtlrDrvCtrl->isrHandle);
    int inputPin, outputPin, cpunum, destCpu, bitCnt, i;
    struct vxbIntCtlrPin * pPin;
    BOOL flag;

    /* find the device input pin number */

    inputPin =
        intCtlrPinFind (pDev, index, pVxbPMipsZrIntCtlrDrvCtrl->pInst, isrHandle);
    if (inputPin == ERROR)
        return (ERROR);

    /* convert cpuset_t cpu number to numeric cpu number */

    for (bitCnt = 0, destCpu = 0, i = 0;i < SB_NUM_CORES;i++)
        if (routeToCpu & (1 << i))
            {
            bitCnt++;
            destCpu = i;
            }

    /* make sure routeToCpus is a proper cpuset_t value */

    if (bitCnt != 1)
        return (ERROR);

    pPin = vxbIntCtlrPinEntryGet(isrHandle, inputPin);

    /* disable this interrupt to all cpus */

    for (cpunum = 0; cpunum < SB_NUM_CORES; cpunum++)
        sb1IntDisableCore (inputPin, cpunum);

    /* get output pin to CPU */

    MIPS_SBE_OUTPUTPIN(isrHandle, inputPin, outputPin);

    /* do the crossconnect */

    sb1IntConnectCore (inputPin, outputPin, destCpu);
    pPin->pinCpu = destCpu;  /* update interrupt route dest cpu */

    /* enable interrupt if enable bit is set */

    VXB_INTCTLR_PINENTRY_ENABLED(isrHandle, inputPin, flag);
    if (flag)
        sb1IntEnableCore (inputPin, destCpu);

    return (OK);
    }

#endif /* _WRS_CONFIG_SMP */

/***************************
 * just for test
 */
void DumpIntSourceTbl()
{
	int i;
	int j;
	for(i = 0;i<4;i++)
	{
		for(j = 0;j<4;j++)
		{
			printf("%02d ",vxbPMipsZrIntSourceTbl[i][j]);
		}
		printf("\n");
	}
}

/*******************************************************************************
*
* sb1IntConnectCore - route the requested interrupt input to the requested
*                     processor core.
*
* This routine maps one of the 64 sb1 interrupt sources to a SB-1 core interrupt
* line.
*
* RETURNS:
* OK, or ERROR if the interrupt handler cannot be built.
*
*/

LOCAL STATUS sb1IntConnectCore
    (
    int intrSource,             /* interrupt level / sb1 source number */
    int intrLine,		/* interrupt output line */
    int cpunum			/* core number to operate on */
    )
    {
    /* initialized? */
	int zrID;
	int source;
	int val;

	zrID = bslProcGetId();
	

    if (pVxbPMipsZrIntCtlrDrvCtrl->initialized == FALSE)
        return (ERROR);
    
    if((intrLine < 0)||(intrLine >= 4)) return (ERROR);
    if((cpunum < 0)||(cpunum >= 4)) return (ERROR);
    
    /*printstr("\r\nsb1IntConnectcore ***********************************\r\n");
    printstr("intrLine is ");printnum(intrLine);printstr("core is ");printnum(cpunum);printstr("\r\n");*/
    
    SPIN_LOCK_ISR_TAKE(&vxbMipsZrIntCtlrLock);
    
    source = intrSource;
    if(source < 0) return (ERROR);
    /*source = source%0x20;no use*/
 

    /* make the crossbar connection */

    /*VXB_MIPS3_SD_BASE(bcm1xxxIntMapBaseH, intrSource, intrLine); commented 20091210 */
	/* This is for Configurable interrupt routing!!! 20091210 */
/*	MIPS_SB64((((unsigned int)(SB1_INT_CTLR(bcm1xxxIntMapBaseH)))+intrSource),(1<<(intrLine+4))|(1<<cpunum));wangzx comment*/ /*zxj 20091208*/
    CPU_WRITE8(C0_IntrRoute+source,zrID,(1<<(intrLine+4))|(1<<cpunum)); /*wangzx 20100830*/
    val = vxbPMipsZrIntSourceTbl[0][intrLine];
    vxbPMipsZrIntSourceTbl[cpunum][intrLine] = intrSource;
	
    
	/*if(intrSource==0xa){
		printstr("## intrSource=0xa ## address is 0x");
		printnum(((unsigned int)(SB1_INT_CTLR(bcm1xxxIntMapBaseH)))+intrSource);
		printstr("\r\n");
		printstr("value is 0x");
		printnum((1<<(intrLine+4))|(1<<cpunum));
		printstr("\r\n");		
		}*/

    SPIN_LOCK_ISR_GIVE(&vxbMipsZrIntCtlrLock);
    return (OK);
    }


/*****************************************************************************
*
* sb1IntDisconnectCore - disconnect the interrupt handler from
*    the SB1 interrupt for a specified core.
*
* This routine disconnects the interrupt handler from the SB1
* interrupt line.
*
* RETURNS:
* OK, or ERROR if the interrupt handler cannot be removed.
*
*/

LOCAL STATUS sb1IntDisconnectCore
    (
    int intrSource,
    int cpunum
    )
    {
/*    UINT64 imr;*//*comment by wangzx avoid warning*/
    STATUS retStatus;
    int source;
    int zrID;

    zrID = bslProcGetId();
    
    source = intrSource;
    if(source < 0) return (ERROR);
    
    /*source = source % 0x20;no use*/

    /* initialized? */

    if (pVxbPMipsZrIntCtlrDrvCtrl->initialized == FALSE)
        return (ERROR);

    if (intrSource < 0 || intrSource >= SB1_NUM_INT_SOURCES)
        return (ERROR);

    retStatus = OK;
    SPIN_LOCK_ISR_TAKE(&vxbMipsZrIntCtlrLock);
#if 0
    VXB_MIPS3_LD(bcm1xxxIntMaskH, intrSource, imr);

    imr |= _INT_MASK1(intrSource);

    VXB_MIPS3_SD(bcm1xxxIntMaskH, intrSource, imr);
#else
	/*printstr("zxj: sb1IntDisconnectCore\n\r");*/
    CPU_WRITE8(C0_IntrRoute+source,zrID,0); /*wangzx 20100830*/
 	/*MIPS_SB64((unsigned int)SB1_INT_CTLR(bcm1xxxIntMapBaseH)+intrSource,0); comment by wangzx*/
#endif

    SPIN_LOCK_ISR_GIVE(&vxbMipsZrIntCtlrLock);

    return(retStatus);
    }



/*****************************************************************************
*
* sb1IntEnableCore - enable an external interrupt source on one SB-1 core
*
* RETURNS:
* OK, or ERROR if the interrupt source cannot be enabled.
*
*/

LOCAL STATUS sb1IntEnableCore
    (
    int intrSource,
    int cpunum
    )
    {
/*    UINT64 imr;*//*comment by wangzx avoid warning*/
    int zrID;
    int source;
    STATUS retStatus;

    zrID = bslProcGetId();
    /* initialized? */
    source = intrSource;
    if(source < 0) return (ERROR);
    /*source = source % 0x20; no use*/
    
    if (pVxbPMipsZrIntCtlrDrvCtrl->initialized == FALSE)
        return (ERROR);

    /* sanity check interrupt source */

    if (source < 0 || source >= 32/* SB1_NUM_INT_SOURCES changed 20091210 */)
        return (ERROR);

    retStatus = OK;
    SPIN_LOCK_ISR_TAKE(&vxbMipsZrIntCtlrLock);
#if 0
    VXB_MIPS3_LD(bcm1xxxIntMaskH, intrSource, imr);

    imr &= ~(_INT_MASK1(intrSource));

    VXB_MIPS3_SD(bcm1xxxIntMaskH, intrSource, imr);
#endif
    CPU_WRITE32(C0_Intenset,zrID,(unsigned int)(1<<source)); /*wangzx 20100830*/
    	
	/*MIPS_SW64((unsigned int)SB1_INT_CTLR(bcm1xxxIntMaskL),(unsigned int)(1<<intrSource)); comment by wangzx*/ /* intenset 20091210 */
    SPIN_LOCK_ISR_GIVE(&vxbMipsZrIntCtlrLock);

    return (retStatus);
    }


/*****************************************************************************
*
* sb1IntDisableCore - disable an external interrupt source on one SB-1 core
*
* RETURNS:
* OK, or ERROR if the interrupt source is an invalid source number, or
*   the source does not currently have a routine attached.
*
*/

LOCAL STATUS sb1IntDisableCore
    (
    int intrSource,
    int cpunum
    )
    {
/*    UINT64 imr;*//*comment by wangzx avoid warning*/
    int zrID;
    int source;
    zrID = bslProcGetId();
    
    source = intrSource;
    if(source < 0) return (ERROR);
    /*source = source%0x20; */

    /*  make sure SB interrupt controller is initialized */

    if (pVxbPMipsZrIntCtlrDrvCtrl->initialized == FALSE)
        return (ERROR);

    if (source < 0 || source >= SB1_NUM_INT_SOURCES)
        return (ERROR);

    SPIN_LOCK_ISR_TAKE(&vxbMipsZrIntCtlrLock);
#if 0
    VXB_MIPS3_LD(bcm1xxxIntMaskH, intrSource, imr);

    imr |= (1ULL << intrSource);

    VXB_MIPS3_SD(bcm1xxxIntMaskH, intrSource, imr);
#else
	/*zxj 20091208, disable source int*/
	/*printstr("zxj: sb1IntDisableCore\n\r");*/
    CPU_WRITE32(C0_Intenclr,zrID,(unsigned int)(1<<source)); /*wangzx 20100830*/
	/*MIPS_SW64((unsigned int)SB1_INT_CTLR(bcm1xxxIntMaskH),1<<intrSource);*/
#endif

    SPIN_LOCK_ISR_GIVE(&vxbMipsZrIntCtlrLock);

    return (OK);
    }


/* 
 * These routines support the SMP vxBus IPI mechanism. 
 *
 * The following functions are provided:
 * vxbMipsZrIpiControlGet - register IPI functions
 * vxbZrIpiEmit - generate an IPI interrupt to one or more cores
 * vxbZrIpiConnect - connect the IPI interrupt handler
 * vxbZrIpiDisconnect - disconnect the IPI interrupt handler
 * vxbZrIpiEnable - enable an IPI interrupt
 * vxbZrIpiDisable - disable an IPI interrupt
 * vxbZrIpiPrioGet - get IPI interrupt priority
 * vxbZrIpiPrioSet - set IPI interrupt priority
 */


/*****************************************************************************
*
* vxbMipsZrIpiControlGet - method to register IPI controller
* 
* This function registers the IPI functions with the vxBus subsystem
*
* RETURNS: pointer to IPI control structure
*
* ERRNO: N/A
* \NOMANUAL
*
*/

LOCAL VXIPI_CTRL_INIT * vxbMipsZrIpiControlGet 
    (
    VXB_DEVICE_ID pInst, 
     void * pArg
    )
    {
    vxIpiCtrlInit.pCpus = (cpuset_t)((1 << SB_NUM_CORES) - 1);
    vxIpiCtrlInit.pCtlr = pInst;
    return (&vxIpiCtrlInit);
    }

/*****************************************************************************
*
* vxbZrIpiConnect - Connect the handler function for IPIs
* 
* This function calls sysZr1IntConnect to connect the interrupt handler
* to the configured output pin, and connects the mbox_x_int_3 interrupt
* to that interrupt pin.
*
* RETURNS:
* OK, or ERROR if the cpuset_t contains an invalid bit.
*
* ERRNO: N/A
* \NOMANUAL
*
*/

LOCAL STATUS vxbZrIpiConnect
    (
    VXB_DEVICE_ID pCtlr,             /* int cntlr device ID */
    INT32 ipiIntrptId,               /* interrupt ID */
    IPI_HANDLER_FUNC routine,        /* routine to be called */
    void * parameter                 /* parameter to be passed to routine */
    )
    {
#ifdef _WRS_CONFIG_SMP
    /*int cpunum;*//*comment by wangzx avoid warning*/
#endif /* _WRS_CONFIG_SMP */
    int outputPin, inputPin;
/*printstr("Entering vxbZrIpiConnect!!\r\n");*/
    /* make sure IPI channel ID is valid */

    if (ipiIntrptId >= MAX_MBOX_INT_CHANNELS)
        return (ERROR);

    /* get input pin number */

    inputPin = MAILBOX_INPUT_PIN(ipiIntrptId);

    /* get output pin based on IPI input pin */

    MIPS_SBE_OUTPUTPIN(&pVxbPMipsZrIntCtlrDrvCtrl->isrHandle,inputPin,
                       outputPin);

    /* connect the ISR to the ctlr interrupt source number,  IPI ISRs
     * are added at the end of the physical input pin array.
     */

    if (intCtlrISRAdd(&pVxbPMipsZrIntCtlrDrvCtrl->isrHandle,
                      IPI_ISR_INPUT_PIN(ipiIntrptId), routine, 
                      parameter) != OK)
        return (ERROR);
#if 0 /* added by yinwx, 20100302, no use for IPI in LS3A */
#ifdef _WRS_CONFIG_SMP
    for (cpunum = 0; cpunum < SB_NUM_CORES; cpunum++)
        {

        /* route int source to cpu input */

        if (sb1IntConnectCore (inputPin, outputPin, cpunum) != OK)
            return (ERROR);
        }
#else /* _WRS_CONFIG_SMP */
    if (sb1IntConnectCore (inputPin, outputPin, vxCpuIdGet()) != OK)
        return (ERROR);
#endif /* _WRS_CONFIG_SMP */
#endif
    return (OK);
    }

/*****************************************************************************
*
* vxbZrIpiDisconnect - Disconnect the handler function for IPIs
* 
* This function calls sysZr1IntDisconnect to discconnect the interrupt handler
* from the configured output pin, and disconnect the mbox_x_int_3 interrupt
* from that interrupt pin.
*
* RETURNS:
* OK, or ERROR if the cpuset_t contains an invalid bit.
*
* ERRNO: N/A
* \NOMANUAL
*
*/

LOCAL STATUS vxbZrIpiDisconnect
    (
    VXB_DEVICE_ID pCtlr,             /* int cntlr device ID */
    INT32 ipiIntrptId,               /* interript ID */
    IPI_HANDLER_FUNC routine,        /* routine to be called */
    void * parameter                 /* parameter to be passed to routine */
    )
    {
#ifdef _WRS_CONFIG_SMP
/*    int cpunum;*/ /*comment by wangzx avoid warning*/
#endif /* _WRS_CONFIG_SMP */
    int inputPin;

    /* make sure IPI channel ID is valid */

    if (ipiIntrptId >= MAX_MBOX_INT_CHANNELS)
        return (ERROR);

    inputPin = MAILBOX_INPUT_PIN(ipiIntrptId);

    /* remove the ISR from the ctlr interrupt source number */

    if (intCtlrISRRemove(&pVxbPMipsZrIntCtlrDrvCtrl->isrHandle,
                         IPI_ISR_INPUT_PIN(ipiIntrptId), routine, 
                         parameter) != OK)
        return (ERROR);

    return (OK);
    }


/*****************************************************************************
*
* vxbZrIpiEnable - Enable the IPI interrupt
* 
* RETURNS: OK always
*
* ERRNO: N/A
* \NOMANUAL
*
*/

LOCAL STATUS vxbZrIpiEnable
    (
    VXB_DEVICE_ID pCtlr,
    INT32 ipiIntrptId
    )
    {
#ifdef _WRS_CONFIG_SMP
    int cpunum;
#endif /* _WRS_CONFIG_SMP */
    int outputPin, inputPin;
    void (*func)();
    void * arg;
/*printstr("vxbZrIpiEnable!!\r\n");    */
    /* make sure IPI channel ID is valid */
    
    if (ipiIntrptId >= MAX_MBOX_INT_CHANNELS)
        return (ERROR);

    inputPin = MAILBOX_INPUT_PIN(ipiIntrptId);

    /* get output pin based on IPI input pin */

    MIPS_SBE_OUTPUTPIN(&pVxbPMipsZrIntCtlrDrvCtrl->isrHandle,inputPin,
                       outputPin);

    if (vxbBcmFamily == BCM_SB1)
	func = vxbZr1Int;
    else
	func = vxbZr1aInt;

#ifdef _WRS_CONFIG_SMP
    for (cpunum = 0; cpunum < SB_NUM_CORES; cpunum++)
	{
	if (vxbIntEnable (pCtlr, MIPS_SB_CPU_OUTPUT_PIN(cpunum,outputPin), 
			  func, (void *)outputPin) == ERROR)
	    return (ERROR);
	}
#else /* _WRS_CONFIG_SMP */
    if (vxbIntEnable (pCtlr, MIPS_SB_CPU_OUTPUT_PIN(0,outputPin), 
                      func, (void *)outputPin) == ERROR)
        return (ERROR);
#endif /* _WRS_CONFIG_SMP */

    MIPS_SBE_ISR(&pVxbPMipsZrIntCtlrDrvCtrl->isrHandle, 
                  IPI_ISR_INPUT_PIN(ipiIntrptId), func);
    MIPS_SBE_ARG(&pVxbPMipsZrIntCtlrDrvCtrl->isrHandle, 
                  IPI_ISR_INPUT_PIN(ipiIntrptId), arg);

    if (intCtlrISREnable(&pVxbPMipsZrIntCtlrDrvCtrl->isrHandle,
                          IPI_ISR_INPUT_PIN(ipiIntrptId), func, arg) != OK)
        return (ERROR);

    /* enable interrupt to cpu last */

#ifdef _WRS_CONFIG_SMP
    for (cpunum = 0; cpunum < SB_NUM_CORES; cpunum++)
	sb1IntEnableCore(inputPin, cpunum);
#else /* _WRS_CONFIG_SMP */
    sb1IntEnableCore(inputPin, vxCpuIdGet());
#endif /* _WRS_CONFIG_SMP */

    return OK;
    }

/*****************************************************************************
*
* vxbZrIpiDisable - Disable the IPI interrupt
* 
* RETURNS:
* OK, or ERROR if the cpuset_t contains an invalid bit.
*
* ERRNO: N/A
* \NOMANUAL
*
*/
LOCAL STATUS vxbZrIpiDisable
    (
    VXB_DEVICE_ID pCtlr,
    INT32 ipiIntrptId
    )
    {
#ifdef _WRS_CONFIG_SMP
/*    int cpunum;*//*comment by wangzx avoid warning*/
#endif /* _WRS_CONFIG_SMP */
    int outputPin, inputPin;
    void (*func)();
    void * arg;

    /* make sure IPI channel ID is valid */

    if (ipiIntrptId >= MAX_MBOX_INT_CHANNELS)
        return (ERROR);

    inputPin = MAILBOX_INPUT_PIN(ipiIntrptId);

    /* get output pin based on IPI input pin */

    MIPS_SBE_OUTPUTPIN(&pVxbPMipsZrIntCtlrDrvCtrl->isrHandle,inputPin,
                       outputPin);

    if (vxbBcmFamily == BCM_SB1)
	func = vxbZr1Int;
    else
	func = vxbZr1aInt;

    MIPS_SBE_ISR(&pVxbPMipsZrIntCtlrDrvCtrl->isrHandle, 
                  IPI_ISR_INPUT_PIN(ipiIntrptId), func);
    MIPS_SBE_ARG(&pVxbPMipsZrIntCtlrDrvCtrl->isrHandle, 
                  IPI_ISR_INPUT_PIN(ipiIntrptId), arg);

    if (intCtlrISRDisable(&pVxbPMipsZrIntCtlrDrvCtrl->isrHandle,
                          IPI_ISR_INPUT_PIN(ipiIntrptId), func, arg) != OK)
        return (ERROR);

    return OK;
    }

/*****************************************************************************
*
* vxbZrIpiPrioGet - Get the IPI interrupt priority
* 
* not supported on this controller (priority is set by output pin IPI
* is routed too)
* 
* RETURNS:
* 0 always
*
* ERRNO: N/A
* \NOMANUAL
*
*/

LOCAL INT32 vxbZrIpiPrioGet
    (
    VXB_DEVICE_ID pCtlr,
    INT32 ipiIntrptId
    )
    {
    return (0);
    }

/*****************************************************************************
*
* vxbZrIpiPrioSet - Set the IPI interrupt priority
* 
* not supported on this controller (priority is set by output pin IPI
* is routed too)
*
* RETURNS:
* OK always
*
* ERRNO: N/A
* \NOMANUAL
*
*/

LOCAL STATUS vxbZrIpiPrioSet
    (
    VXB_DEVICE_ID pCtlr,
    INT32 ipiIntrptId,
    INT32 prio
    )
    {
    return (OK);
    }

#ifdef MIPS_SBE_INTCTLR_DEBUG_ON

/******************************************************************************
*
* mipsZrIntCtlrShow - show pDrvCtrl for mipsIntCtlr
*
* Based on verboseLevel value, display interrupt controller connection
* and methods information
*
* RETURNS: N/A
*
* ERRNO
*/

LOCAL void mipsZrIntCtlrShow
    (
    VXB_DEVICE_ID       pInst,
    int                 verboseLevel
    )
    {
    MIPS_SB_INT_CTLR_DRVCTRL * pDrvCtrl = pInst->pDrvCtrl;
    device_method_t * pMethod;

    printf("\tinterrupt controller %s%d = 0x%08x\n",
        pInst->pName, pInst->unitNumber, pInst);

    if (verboseLevel == 0)
        return;

    if (verboseLevel > 50)
        intCtlrHwConfShow(&pDrvCtrl->isrHandle);

    if (verboseLevel > 500)
        {
        pMethod = &mipsZrIntCtlr_methods[0];
        printf("\tPublished Methods:\n");
        while ((pMethod->devMethodId != NULL) && (pMethod->handler != NULL))
            {
            printf("\t    0x%08x => '%s'\n", (int)pMethod->handler, 
                   pMethod->devMethodId);
            pMethod++;
            }
        }

    return;
    }

#endif /* MIPS_SBE_INTCTLR_DEBUG_ON */
