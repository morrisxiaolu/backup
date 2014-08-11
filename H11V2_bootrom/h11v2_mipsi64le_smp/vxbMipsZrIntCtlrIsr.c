/* vxbMipsSbIntCtlr.c - interrupt controller for BCM1480 ISR and IPI Emit portion */

/*
 * Copyright (c) 2008 Wind River Systems, Inc. All rights are reserved.
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
01c,27aug08,jpb  Renamed VSB header file
01b,18jul08,kk  added inclusion of private/vsbConfigP.h to pick up macros
01a,03jun08,h_k  split ISR and IPI Emit portions out.
*/

/*
DESCRIPTION

This file supplies routines that handle the vxbus interrupt controller
requirements for the Broadcom sibyte family of targets.  This driver
supports interrupt handling and IPI emit.

INCLUDE FILES:
vxbIntCtlrLib.h
*/

/* includes */

#include <vxWorks.h>
#include <vsbConfig.h>
#include <vxLib.h>
#include <ffsLib.h>
#include <vxIpiLib.h>
#include <../src/hwif/intCtlr/vxbIntCtlrLib.h>
#include "vxbMipsZrIntCtlrSP.h"
#include "simpleprintf.h" /* yinwx 20091219 */

/* imports */

IMPORT INT_CTLR vxbZrIntCtlr[];
IMPORT MIPS_SB_INT_CTLR_DRVCTRL * pVxbPMipsZrIntCtlrDrvCtrl;
IMPORT unsigned int vxbPMipsZrIntSourceTbl[4][4];
IMPORT int vxCpuIdGet(void);
//IMPORT void sysSmInt(int parameter); /*add by wangzx to avoid warning*/

#define R_IMR_INTERRUPT_MAP_COUNT	((vxbBcmFamily == BCM_SB1) ? R_IMR_INTERRUPT_MAP_COUNT_SB1 : R_IMR_INTERRUPT_MAP_COUNT_SB1A)

/******************************************************************************
*
* ffsMsbLL - find most significant bit set in a long long
*
* This routine finds the most significant bit set in the 64 bit argument
* passed to it and returns the index of that bit.  Bits are numbered starting
* at 1 from the least signifficant bit.  A return value of zero indicates that
* the value passed is zero.
*
* RETURNS: index of most significant bit set, or zero
* ERRNO: N/A
*/

_WRS_INLINE
int ffsMsbLL
    (
    UINT64 i            /* value in which to find first set bit */
    )
    {
    FAST UINT32 msw;    /* most significant word */

    if ((msw = (UINT32)(i >> 32)) != 0)
        return (ffsMsb(msw) + 32);
    else
        return (ffsMsb(i & 0xffffffff));
    }

/*******************************************************************************
* vxbZr1Int - interrupt handler for shared SB1 interrupt.
*
* This routine executes multiple interrupt handlers for a SB1 CPU
* interrupt.  Prior to execution of the interrupt handler this routine
* checks the SB1 interupt_status_n register to determine if source
* of the interrupt matches the interrupt handler in the link list.
*
* RETURNS: N/A
*
*/

void vxbZr1Int
    (
    int intr_line               /* INT line that caused the interrupt */
    )
    {
    struct intCtlrHwConf * pEnt;
    unsigned int intSource, ix;
    unsigned int cpunum = vxCpuIdGet();
    UINT64 intStatus;
    UINT64 mailboxInts;

    intStatus = MIPS3_LD(&SB1_INT_CTLR(bcm1xxxIntStatusBaseH)[intr_line]);
    mailboxInts = *(UINT64 *)IMR_REGISTER(MAILBOX_READ);

    pEnt = &pVxbPMipsZrIntCtlrDrvCtrl->isrHandle;

    /* handle CPC IPI if set first */

    if ((mailboxInts & SMP_CPC_MAILBOX_INT_BIT))
        {

        /* clr interrupt & call ISR */

	*(long long *)IMR_REGISTER(MAILBOX_CLR) = SMP_CPC_MAILBOX_INT_BIT;
        VXB_INTCTLR_ISR_CALL(pEnt, IPI_ISR_INPUT_PIN_SB1(IPI_INTR_ID_CPC));
        MIPS3_SD(SB1_INT_CTLR(bcm1xxxLdtIntClearH), 
                              (1LL << MAILBOX_INPUT_PIN_SB1(IPI_INTR_ID_CPC)));

        /* only need to clr SMP_CPC_MAILBOX_INT_BIT bit in mailboxInts if
         * interrupt processing will continue (not in SMP mode)
         */

#ifndef _WRS_CONFIG_SMP
        mailboxInts ^= SMP_CPC_MAILBOX_INT_BIT;
#endif /* _WRS_CONFIG_SMP */
        }
#ifdef _WRS_CONFIG_SMP
    else    /* remove for non-SMP mode so interrupt processing will continue
             * if SMP_CPC_MAILBOX_INT_BIT was set above
             */
#endif /* _WRS_CONFIG_SMP */
        {

        /* Clear any of the interrupts that came from LDT */

        MIPS3_SD(SB1_INT_CTLR(bcm1xxxLdtIntClearH), intStatus);

        /* demux IPIs above CPC */

        if (mailboxInts != 0)
            {
	    *(long long *)IMR_REGISTER(MAILBOX_CLR) = mailboxInts;

            /* start scanning for IPIs with the debug channel */

            while ((ix = ffsMsbLL(mailboxInts)) != 0)
                {
                VXB_INTCTLR_ISR_CALL(pEnt, IPI_ISR_INPUT_PIN_SB1(ix-1));
                mailboxInts ^= (1LL << (ix - 1));
                intStatus ^= (1LL << (MAILBOX_INPUT_PIN_SB1(ix-1)));
                }
            }

        while (intStatus)
            {
            intSource = ffsMsbLL(intStatus) - 1;

            VXB_INTCTLR_ISR_CALL(pEnt, intSource);

            intStatus ^= (1ULL << intSource);   /* clear the bit */
            }
        }
    }

/********************************************************************************
* vxbZr1aInt - interrupt handler for shared SB1A interrupt.
*
* This routine executes multiple interrupt handlers for a SB1A CPU
* interrupt.  Prior to execution of the interrupt handler this routine
* checks the SB1A interupt_status_n register to determine if source
* of the interrupt matches the interrupt handler in the link list.
*
* RETURNS: N/A
*
*/

void vxbZr1aInt(int intr_line /* INT line that caused the interrupt */)
{
	struct intCtlrHwConf * pEnt;
	unsigned int intSource, ix;
/*	unsigned int outputPin;
	unsigned int configCpu;
	UINT64 intStatusH, intStatusL;
	unsigned int i;*//*comment by wangzx avoid warning*/
	unsigned int cpunum = vxCpuIdGet();
	UINT64 mailboxInts;
	ULONG srValue, crValue;
	unsigned int temp1,temp2;

	pEnt = &pVxbPMipsZrIntCtlrDrvCtrl->isrHandle;
	crValue = intCRGet();
	/* interrupt processing for Loongson 3A,  yinwx 20091215 */
	if((crValue & 0x4000)==0x4000)
	{ /* IPI interrupts ! */
		mailboxInts = MIPS_LW64((UINT32)IMR_REGISTER(MAILBOX_READ));
		/* handle CPC IPI if set first then just return */
		if((mailboxInts & SMP_CPC_MAILBOX_INT_BIT) != 0LL)
        {
			/* clr interrupt & call ISR , CPC IPI */
			MIPS_SW64((UINT32)IMR_REGISTER(MAILBOX_CLR),SMP_CPC_MAILBOX_INT_BIT);
			VXB_INTCTLR_ISR_CALL(pEnt, IPI_ISR_INPUT_PIN_SB1A(IPI_INTR_ID_CPC));
			/* only need to clr SMP_CPC_MAILBOX_INT_BIT bit in mailboxInts if
         		* interrupt processing will continue (not in SMP mode)
        	*/
		}
#if 0
		else if ((mailboxInts & (1<<4)) != 0LL)
		{
			#if 0   /* mxl: del for bootrom */
			/*printstr("SM INTERRUPT!!\r\n");
			printstr("IN vxbZr1aInt CPUNUM vxCpuIdGet() is 0x");
			printnum(cpunum);
			printstr("\r\n");*/
			sysSmInt(4); /* THE SAME with BELOW!! yinwx, 20100302 */
		/*VXB_INTCTLR_ISR_CALL(pEnt, IPI_ISR_INPUT_PIN_SB1A(4));*/
			#endif
		}
#endif
		else if (mailboxInts != 0)
        {
			/* clr interrupt & call ISR , other IPIs*/
			MIPS_SW64((UINT32)IMR_REGISTER(MAILBOX_CLR),(mailboxInts));
			while ((ix = ffsMsbLL(mailboxInts)) != 0)
			{
				VXB_INTCTLR_ISR_CALL(pEnt, IPI_ISR_INPUT_PIN_SB1A(ix-1));
				mailboxInts ^= (1LL << (ix - 1));                
			}	        
		}
	}
	/* Normal I/O interrupts */

	/*add by wangzx begin 20100830*/
	if((crValue & 0x3c00) != 0)
	{
		srValue = intSRGet();
	    srValue &= ~SR_IE;
		intSRSet(srValue); /* Clear interrupts! */
		temp1 = crValue & 0x3c00;
		temp1 >>= 10;
		temp2 = 0;
		while(temp1 != 0)
		{
			temp2 ++;
			if((temp1 & 0x1)!= 0)
			{
				intSource = vxbPMipsZrIntSourceTbl[cpunum][temp2-1];
				if(intSource < 32)	VXB_INTCTLR_ISR_CALL(pEnt, HIGH(intSource));
			}
			temp1 >>= 1;
		}
		srValue = intSRGet();
    	srValue |= SR_IE;	
    	intSRSet(srValue);
	}
	/*add by wangzx begin 20100830*/
		
#if 0
    intStatusH = MIPS3_LD(&SB1_INT_CTLR(bcm1xxxIntStatusBaseH)[intr_line]);
    mailboxInts = *(UINT64 *)IMR_REGISTER(MAILBOX_READ);

    /* handle CPC IPI if set first then just return */

    if ((mailboxInts & SMP_CPC_MAILBOX_INT_BIT) != 0LL)
        {

        /* clr interrupt & call ISR */
	*(long long *)IMR_REGISTER(MAILBOX_CLR) = SMP_CPC_MAILBOX_INT_BIT;
        VXB_INTCTLR_ISR_CALL(pEnt, IPI_ISR_INPUT_PIN_SB1A(IPI_INTR_ID_CPC));
        MIPS3_SD(SB1_INT_CTLR(bcm1xxxLdtIntClearH),
                              (1LL << MAILBOX_INPUT_PIN_SB1A(IPI_INTR_ID_CPC)));

        /* only need to clr SMP_CPC_MAILBOX_INT_BIT bit in mailboxInts if
         * interrupt processing will continue (not in SMP mode)
         */
#ifndef _WRS_CONFIG_SMP
        mailboxInts ^= SMP_CPC_MAILBOX_INT_BIT;
#endif /* _WRS_CONFIG_SMP */
        }
#ifdef _WRS_CONFIG_SMP
    else    /* remove for non-SMP mode so interrupt processing will continue
             * if SMP_CPC_MAILBOX_INT_BIT was set above
             */
#endif /* _WRS_CONFIG_SMP */
#endif
        {
	#if 0
        intStatusL = MIPS3_LD(&SB1_INT_CTLR(bcm1xxxIntStatusBaseL)[intr_line]);

        /* Clear any of the interrupts that came from LDT */

        MIPS3_SD(SB1_INT_CTLR(bcm1xxxLdtIntClearH), intStatusH);
        MIPS3_SD(SB1_INT_CTLR(bcm1xxxLdtIntClearL), intStatusL);
	

        /* demux IPIs above CPC */

        if (mailboxInts != 0)
            {
	    *(long long *)IMR_REGISTER(MAILBOX_CLR) = mailboxInts;

            /* start scanning for IPIs with the debug channel */

            while ((ix = ffsMsbLL(mailboxInts)) != 0)
                {
                VXB_INTCTLR_ISR_CALL(pEnt, IPI_ISR_INPUT_PIN_SB1A(ix-1));
                mailboxInts ^= (1LL << (ix - 1));
                intStatusH ^= (1LL << (MAILBOX_INPUT_PIN_SB1A(ix-1)));
                }
            }

        /* bit 0 is the summary bit for intStatusL */
		
        intStatusH &= ~1LL;
		
        /* scan for any other pending interrupts */
		
        while (intStatusH)
            {
            intSource = ffsMsbLL(intStatusH) - 1;

            VXB_INTCTLR_ISR_CALL(pEnt, HIGH(intSource));

            intStatusH ^= (1ULL << intSource);   /* clear the bit */
            }

        while (intStatusL)
            {
            intSource = ffsMsbLL(intStatusL) - 1;

            VXB_INTCTLR_ISR_CALL(pEnt, LOW(intSource));

            intStatusL ^= (1ULL << intSource);   /* clear the bit */
            }
		#endif			
        }

    }

/*****************************************************************************
*
* vxbZrIpiEmit - Generate an IPI interrupt at the target processor
* 
* This function generates an IPI interrupt at the target CPU(s)
* specified by the cpuset_t mask.
*
* RETURNS:
* OK, or ERROR if the cpuset_t contains an invalid bit.
*
* ERRNO: N/A
* \NOMANUAL
*
*/

STATUS vxbZrIpiEmit
    (
    VXB_DEVICE_ID pCtlr,
    INT32 ipiIntrptId,
    cpuset_t processor
    )
    {
    int cpunum;			/* processor index */
    long long mbSelect;

    /* get register bit for this channel for IPI request */

    mbSelect = MBOX_INT_REQUEST_SET(ipiIntrptId);

    /* get the least CPU number */

    cpunum = ffsLsb (processor) - 1;

    /* shift the processor bits if the least CPU number is not 0 */

    processor >>= cpunum;

    /* send IPI to requested destination cpus */

    do
	{
	if (processor & 1)
	    {
	    #if 0 /* added by yinwx, 20091219 */
	    *(long long *)IMR_REGISTER(MAILBOX_SET) = mbSelect;
		#else
	    MIPS_SW64(IMR_REGISTER(MAILBOX_SET),mbSelect); 
		MIPS_SW64(IMR_REGISTER(MAILBOX_ENABLE),mbSelect);
		#endif
		}

	cpunum++;
	processor >>= 1;
	}
    while (processor);

    return OK;
    }
