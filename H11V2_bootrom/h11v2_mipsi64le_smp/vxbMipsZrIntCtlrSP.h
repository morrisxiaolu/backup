/* vxbMipsZrIntCtlrSP.h - interrupt controller for BCM1480 */

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
01c,27aug08,jpb  Renamed VSB header file
01b,18jul08,kk  added inclusion of private/vsbConfigP.h to pick up macros
01a,03jun08,h_k  created.
*/

#ifndef __INCvxbMipsZrIntCtlrSPh
#define __INCvxbMipsZrIntCtlrSPh

#include <vsbConfig.h>
#include <drv/multi/bcm1480Lib.h>

/* defines */

/* define IPI mailbox number */

#define R_IMR_INTERRUPT_MAP_COUNT_SB1	64
#define R_IMR_INTERRUPT_MAP_COUNT_SB1A	64 /*32 20100901,wangzx modify to 64*/ /* 128  20091211 */

/* bit number of Mailbox interrupt source for CPC , debugger & spare channels */

#define SB1_MBOX_INT_0		26
#define SB1_MBOX_INT_1		27
#define SB1_MBOX_INT_2		28
#define SB1_MBOX_INT_3		29

#define SB1A_MBOX_INT_0		56
#define SB1A_MBOX_INT_1		57
#define SB1A_MBOX_INT_2		58
#define SB1A_MBOX_INT_3		59

/* allow for 32 IPIs */

#define MAX_MBOX_INT_CHANNELS		32

/* 16 bits per input pin */

#define MBOX_CHANNEL_SPACING		16

/* the ipi that's used for the CPC (IPI_INTR_ID_CPC) in SMP mode comes
 * in on bit 48 which translates to input pin MBOX_INT_0.  this IPI
 * must be the only bit assigned in a group of 16 bits (48-63).
 * all other ipis are assigned a bit position that is the same as the 
 * IPI number.  this places IPIs 1-15 on input pin MBOX_INT_3 and IPIs
 * 16-31 on input pin BOX_INT_2.  if IPIs are extended above 31 (32 total)
 * they would be assigned to input pin MBOX_INT_1
 */

#define MAILBOX_INPUT_PIN_SB1(ipi)					\
		((ipi==IPI_INTR_ID_CPC)?SB1_MBOX_INT_0:			\
		(ipi<MBOX_CHANNEL_SPACING)?SB1_MBOX_INT_3:		\
		(ipi<(MBOX_CHANNEL_SPACING*2))?SB1_MBOX_INT_2:		\
		SB1_MBOX_INT_1)

#define MAILBOX_INPUT_PIN_SB1A(ipi)					\
		((ipi==IPI_INTR_ID_CPC)?SB1A_MBOX_INT_0:		\
		(ipi<MBOX_CHANNEL_SPACING)?SB1A_MBOX_INT_3:		\
		(ipi<(MBOX_CHANNEL_SPACING*2))?SB1A_MBOX_INT_2:		\
		SB1_MBOX_INT_1)

/* mailbox int bit assignment for IPIs 0 - 31 */

#define SMP_CPC_MAILBOX_INT_BIT		(1UL<<16) /* (1LL<<48) yinwx, 20091219 */
#define MAILBOX_INT_BIT(ipi)		(1UL<<ipi) /* (1LL<<ipi) yinwx, 20091219 */

/* channel interrupt request bit */

#define MBOX_INT_REQUEST_SET(ipi)   \
	((ipi == IPI_INTR_ID_CPC)?SMP_CPC_MAILBOX_INT_BIT:MAILBOX_INT_BIT(ipi))

/* IPI input pin storage index number for ISRs called from IPI demux */

#define IPI_ISR_INPUT_PIN_SB1(ipi)	(R_IMR_INTERRUPT_MAP_COUNT_SB1+ipi)
#define IPI_ISR_INPUT_PIN_SB1A(ipi)	(R_IMR_INTERRUPT_MAP_COUNT_SB1A+ipi)

/* mailbox register offsets */

#define R_IMR_MAILBOX_READ_CPU		0x1000 /* 0x00C0 yinwx, 20091219 */
#define R_IMR_MAILBOX_ENABLE_CPU	0x1004 /* added by yinwx, 20091219 */
#define R_IMR_MAILBOX_SET_CPU		0x1008 /* 0x00C8 yinwx, 20091219 */
#define R_IMR_MAILBOX_CLR_CPU		0x100c /* 0x00D0 yinwx, 20091219 */
#define MAILBOX_READ                    R_IMR_MAILBOX_READ_CPU
#define MAILBOX_ENABLE					R_IMR_MAILBOX_ENABLE_CPU /* added by yinwx, 20091219 */
#define MAILBOX_SET                     R_IMR_MAILBOX_SET_CPU
#define MAILBOX_CLR                     R_IMR_MAILBOX_CLR_CPU

#if defined (_WRS_CONFIG_SMP)
#define SB_NUM_CORES	vxCpuConfiguredGet()
#else /* _WRS_CONFIG_SMP */
  /* For UP or AMP, treat as if each core does its own interrupt handling */
#define SB_NUM_CORES	1
#endif /* _WRS_CONFIG_SMP */

/* macros to fields in intr cntlr pin array */

#define MIPS_SBE_OUTPUTPIN(pEnt,inputPin,outputPin)          \
        {                                                    \
        struct vxbIntCtlrPin * pPin =                        \
           vxbIntCtlrPinEntryGet(pEnt,inputPin);             \
        outputPin = pPin->pinOutput;                         \
        }

#define MIPS_SBE_DESTCPU(pEnt,inputPin,destCpu)              \
        {                                                    \
        struct vxbIntCtlrPin * pPin =                        \
           vxbIntCtlrPinEntryGet(pEnt,inputPin);             \
        destCpu = pPin->pinCpu;                              \
        }

#define MIPS_SBE_ISR(pEnt,inputPin,func)                     \
        {                                                    \
        struct vxbIntCtlrPin * pPin =                        \
           vxbIntCtlrPinEntryGet(pEnt,inputPin);             \
        func = pPin->isr;                                   \
        }

#define MIPS_SBE_ARG(pEnt,inputPin,arg)                      \
        {                                                    \
        struct vxbIntCtlrPin * pPin =                        \
           vxbIntCtlrPinEntryGet(pEnt,inputPin);             \
        arg = pPin->pArg;                                   \
        }

#define IMR_REGISTER(reg)       /*PHYS_TO_K1 20091211*/(A_IMR_REGISTER(cpunum,(reg)))
#define SB1_INT_CTLR(reg)       vxbZrIntCtlr[cpunum].reg

#define INT_SRC_OFFSET(source)  ((source)&0x3f)
#define INT_REG_OFFSET(source)  (((source)&0x40) << 3)

/* eight outputs per core */

#define SB1_NUM_INT_LINES       8

/* input interrupt sources */

#define SB1_NUM_INT_SOURCES     R_IMR_INTERRUPT_MAP_COUNT

/* IPI support macros */

/* calculate the output pin number based on destination CPU (cpunum) and
 * the output requested pin.  there are SB1_NUM_INT_LINES interrupt
 * outputs per cpu and they are numbered from 0 to (SB1_NUM_INT_LINES - 1)
 * for core 0 up to (cpunum * SB1_NUM_INT_LINES) to 
 * ((cpunum * SB1_NUM_INT_LINES) + (SB1_NUM_INT_LINES - 1))
 */

#define MIPS_SB_CPU_OUTPUT_PIN(cpunum,vect)  ((cpunum * SB1_NUM_INT_LINES) + \
                                               vect)

typedef struct mipsZrIntCtlrDrvCtrl
    {
    VXB_DEVICE_ID        pInst;
    struct intCtlrHwConf isrHandle;
    cpuset_t             enabledCpus;
    int                  defaultCpu;
    BOOL                 initialized;
    } MIPS_SB_INT_CTLR_DRVCTRL;

typedef struct sb1IntCtlr
    {
    volatile UINT64 *bcm1xxxIntMaskH;
    volatile UINT64 *bcm1xxxIntStatusBaseH;
    volatile UINT64 *bcm1xxxIntMapBaseH;
    volatile UINT64 *bcm1xxxLdtIntClearH;

    /* Used by BCM_SB1A only */

    volatile UINT64 *bcm1xxxIntMaskL;
    volatile UINT64 *bcm1xxxIntStatusBaseL;
    volatile UINT64 *bcm1xxxIntMapBaseL;
    volatile UINT64 *bcm1xxxLdtIntClearL;
    } INT_CTLR;

#endif	/* __INCvxbMipsZrIntCtlrSPh */
