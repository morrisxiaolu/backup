/* hwconf.c - Hardware configuration support module */

/*
 * Copyright (c) 2006-2008 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01t,10sep08,slk  don't allocate 4 MIPS int controllers if only 3 CPUs
                 configured
01s,29aug08,kab  Change _WRS_VX_SMP to _WRS_CONFIG_SMP
01r,01may08,slk  add ipi support on all ipi input pins
01q,31oct07,slk  add legacy interrupt support example
01p,27sep07,bwa  fixed WIND105938 (redefinition of vxTas)
01o,02jul07,bwa  updated dshmSm.h path
01n,22jun07,bwa  added support for concurrent SM and DSHM.
01m,22jun07,bwa  added DSHM devices.
01l,18jun07,slk  add shared memory device and move the sb1FfsTbl table
                 back to sysLib.c
01k,07jun07,h_k  added "bcmFamily" parameter.
01j,24may07,slk  fix UART B regBase defines based on sb1250 or sb1480 BSP
01i,03may07,slk  clean up warnings
01h,27apr07,slk  Move MIPS_CPU_NUM_INTERRUPT_INPUTS to config.h
01g,04apr07,slk  add MIPS interrupt controllers
01f,03apr07,slk  Removed #include <drv/multi/sb1Lib.h>
                 This was causing many redefinitions
		 and preventing the kernel from booting
01e,05mar07,ami  Timer Device Resources Added
01d,05feb07,wap  Add 4th sbe ethernet port
01c,14nov06,pmr  serial on second core
01b,21sep06,pmr  add support for DRV_SIO_SB1
01a,28jun06,wap  written
*/

#include <vxWorks.h>
#include <vsbConfig.h>
#include <vxBusLib.h>
#include <hwif/vxbus/vxBus.h>
#include <hwif/vxbus/hwConf.h>
#include <hwif/vxbus/vxbIntrCtlr.h>
#include <hwif/util/vxbParamSys.h>
#include <hwif/util/hwMemLib.h>

#include "pciAutoConfigLib.h"
#include "config.h"

#ifdef INCLUDE_ATSEMAC
#define ATSEMAC_UNIT0_DEVICE_ID 0
#define ATSEMAC_CARD_NAME       "atse"
#define ATSEMAC_CARD_DESC       "atse"
#define ATSEMAC_NAME    "atse"


const struct hcfResource atsemac0Resources[] = {
	    { "deviceId", HCF_RES_INT, { (void *)(ATSEMAC_UNIT0_DEVICE_ID) } },
	    { "regBase", HCF_RES_INT, { (void *)(0) } },
	    { "intr0", HCF_RES_INT, { (void *)(0) } },
	    { "intr0Level", HCF_RES_INT, { (void *)(0) } },        
	    { "cacheLineSize", HCF_RES_INT, { (void *)(_CACHE_ALIGN_SIZE) } },
	    { "mtuSize", HCF_RES_INT, { (void *)(1500) } },
	    { "bufferAddr", HCF_RES_INT, { (void *)(0) } },
	    { "bufferSize", HCF_RES_INT, { (void *)(0) } },
	    { "bdAddr", HCF_RES_INT, { (void *)(0) } },
	    { "bdSize", HCF_RES_INT, { (void *)(0) } },
	    { "rxBdCount", HCF_RES_INT, { (void *)(0) } },
	    { "txBdCount", HCF_RES_INT, { (void *)(0) } },
	    { "phyAddr", HCF_RES_INT, {(void *)0x0 }},     
        { "miiIfName",      HCF_RES_STRING, { (void *)"atse" } },  
        { "miiIfUnit",      HCF_RES_INT,    { (void *)0 } } 
};

#define atsemac0Num NELEMENTS(atsemac0Resources)

#endif

#define LS3A_FREQ 500000000    //mxl: 495000000
#ifdef INCLUDE_SBE0
LOCAL const struct hcfResource sbeVxbEnd0Resources[] = {
    { "regBase", HCF_RES_INT, { (void *)(PHYS_TO_K1(A_MAC_BASE_0)) } },
};
#define sbeVxbEnd0Num NELEMENTS(sbeVxbEnd0Resources)
#endif

#ifdef INCLUDE_SBE1
LOCAL const struct hcfResource sbeVxbEnd1Resources[] = {
    { "regBase", HCF_RES_INT, { (void *)(PHYS_TO_K1(A_MAC_BASE_1)) } },
};
#define sbeVxbEnd1Num NELEMENTS(sbeVxbEnd1Resources)
#endif

#if 1
/* Added for ns16550 serial resources! 20091211 */
#ifdef	DRV_SIO_NS16550
struct hcfResource ns16550Dev0Resources[] = {
    { "regBase",     HCF_RES_INT, {(void *)PHYS_TO_K1(/*0x1fe001e8*/A_DUART0)} },
    { "clkFreq",     HCF_RES_INT, {(void *)33000000/*3686400*/} },
    { "regInterval", HCF_RES_INT, {(void *)1} },
};
#define ns16550Dev0Num NELEMENTS(ns16550Dev0Resources) 
#endif	/* DRV_SIO_NS16550 */
#endif 

//#ifdef HRMATRIXDMA0_DEVICE
#ifdef INCLUDE_HRMATRIXDMA0
struct hcfResource hrMatrixDma0Resources[] = {
	{"regBase", HCF_RES_INT,{(void *)(0)}},
};
#define hrMatrixDma0Num NELEMENTS(hrMatrixDma0Resources)
#endif

//#ifdef HRMATRIXDMA1_DEVICE
#ifdef INCLUDE_HRMATRIXDMA1
struct hcfResource hrMatrixDma1Resources[] = {
		{"regBase", HCF_RES_INT,{(void *)(0)}},
};
#define hrMatrixDma1Num NELEMENTS(hrMatrixDma1Resources)
#endif

#ifdef INCLUDE_OBC_INT0
struct hcfResource hrObcintdev0Resources[] = {
		{"regBase", HCF_RES_INT,{(void *)(0)}},
};
#define hrObcintdev0Num NELEMENTS(hrObcintdev0Resources)
#endif


#ifdef INCLUDE_OBC_INT1
struct hcfResource hrObcintdev1Resources[] = {
		{"regBase", HCF_RES_INT,{(void *)(0)}},
};
#define hrObcintdev1Num NELEMENTS(hrObcintdev1Resources)
#endif


#ifdef INCLUDE_OBC_INT2
struct hcfResource hrObcintdev2Resources[] = {
		{"regBase", HCF_RES_INT,{(void *)(0)}},
};
#define hrObcintdev2Num NELEMENTS(hrObcintdev2Resources)
#endif

#ifdef INCLUDE_OBC_INT3
struct hcfResource hrObcintdev3Resources[] = {
		{"regBase", HCF_RES_INT,{(void *)(0)}},
};
#define hrObcintdev3Num NELEMENTS(hrObcintdev3Resources)
#endif


struct hcfResource r4KTimerDevResources[] =  {
    {"regBase", HCF_RES_INT, {(void *)0} },
    {"minClkRate",HCF_RES_INT, {(void *)SYS_CLK_RATE_MIN} },
    {"maxClkRate",HCF_RES_INT, {(void *)SYS_CLK_RATE_MAX} },
    /* Fix the cpuClkRate for Loongson 3A by yinwx, 20100518
	   The COUNTER Reg increase by 2 clk periods, we must use 2 to divide !! */
    {"cpuClkRate", HCF_RES_INT/*HCF_RES_ADDR*/, {(void *) (LS3A_FREQ / 2)/*(vxbR4KTimerFrequencyGet)*/} }
};
#define r4TimerDevNum NELEMENTS(r4KTimerDevResources)

#ifdef DRV_TIMER_SB1
struct hcfResource sb1TimerDevResources0 [] =  {
#if defined(SB1_CPU_1)
    {"regBase", HCF_RES_INT, {(void *) PHYS_TO_K1(A_SCD_TIMER_BASE(1))} },
#else /* default to timer 0 */
    {"regBase", HCF_RES_INT, {(void *) PHYS_TO_K1(A_SCD_TIMER_BASE(0))} },
#endif
    {"minClkRate",HCF_RES_INT, {(void *)AUX_CLK_RATE_MIN} },
    {"maxClkRate",HCF_RES_INT, {(void *) AUX_CLK_RATE_MAX} },
#ifdef _SIMULATOR_
    {"cpuClkRate", HCF_RES_INT, {(void *) 10000} }
#else
    {"cpuClkRate", HCF_RES_INT, {(void *) 1000000} }
#endif
};
#define sb1TimerDevNum0         NELEMENTS(sb1TimerDevResources0)
#endif /* DRV_TIMER_SB1 */

#if 0
#ifdef INCLUDE_SM_COMMON
struct hcfResource smDevResources[] =  {
    {"regBase", HCF_RES_INT, {(void *)0} },
};
#define smDevNum NELEMENTS(smDevResources)
#endif /* INCLUDE_SM_COMMON */

#else /* added by yinwx, 20100202 */
#ifdef INCLUDE_SM_COMMON
/* added by yinwx, 20100127 */AAA
struct hcfResource smEndDevResources[] =  {
    {"regBase", HCF_RES_INT, {(void *)0} },
    {"SM_MEM_SIZE", HCF_RES_INT, {(void *)0x8000000/* 128M DEFAULT_MEM_SIZE*/}},
    {"SM_MEM_ADRS", HCF_RES_INT, {(void *)0x42000000/*NULL*/}},
    #if 0
    {"SM_TAS_TYPE", HCF_RES_INT, {(void *)SM_TAS_HARD}},
	#endif
    {"SM_PKTS_SIZE", HCF_RES_INT, {(void *)DEFAULT_PKT_SIZE}},
    {"SM_MAX_PKTS", HCF_RES_INT, {(void *)DEFAULT_PKTS_MAX}},
    {"SM_INT_TYPE", HCF_RES_INT, {(void *)SM_INT_USER_1}},
    {"SM_INT_ARG1", HCF_RES_INT, {(void *)4}}, /* SM MailBox Interrupt bit! */
    {"SM_INT_ARG2", HCF_RES_INT, {(void *)0}},
    {"SM_INT_ARG3", HCF_RES_INT, {(void *)0}},
    {"SM_MBLK_NUM", HCF_RES_INT, {(void *)0}},
    {"SM_CBLK_NUM", HCF_RES_INT, {(void *)0}},
    /* anchor dynamiclly assigned in bootline or add this with localaddrs */
    {"SM_ANCHOR_OFFSET", HCF_RES_INT, {(void *)0x80000000}} 
};
#define smEndDevNum NELEMENTS(smEndDevResources)
#endif /* INCLUDE_SM_COMMON */
#endif

/* sibyte interrupt controller input pin to device assignments */

const struct intrCtlrInputs mipsSbIntCtlrInputs[] =
    {
    /* pin, driver, unit, index */

    /* aux. clock input pin */
#ifdef DRV_TIMER_SB1
/* secondary CPU (SB1_CPU_1) defaults to timer 1 */
#if defined(SB1_CPU_1)
    {K_INT_TIMER_1, "sb1TimerDev", 0, 0},
#else /* default primary to timer 0 */
    {K_INT_TIMER_0, "sb1TimerDev", 0, 0},
#endif /* defined(SB1_CPU_1) */
#endif /* DRV_TIMER_SB1 */
/* mipsSbIntCtlrInputs 20091210 */
#ifdef DRV_SIO_NS16550
	{10, "ns16550", 0, 0}, 
#endif 

#ifdef INCLUDE_ATSEMAC
	{4, "atse", 0, 0}, 
#endif 
//#ifdef HRMATRIXDMA0_DEVICE
#ifdef INCLUDE_HRMATRIXDMA0
	HRMATRIXDMA0_INT_DESC,
#endif

//#ifdef HRMATRIXDMA1_DEVICE
#ifdef INCLUDE_HRMATRIXDMA1
	HRMATRIXDMA1_INT_DESC,
#endif

#ifdef INCLUDE_OBC_INT0
 	{0,"obcintdev0",0,0},
#endif

#ifdef INCLUDE_OBC_INT1
 	{1,"obcintdev1",0,0},
#endif

#ifdef INCLUDE_OBC_INT2
 	{2,"obcintdev2",0,0},
#endif

#ifdef INCLUDE_OBC_INT3
 	{3,"obcintdev3",0,0},
#endif

#if 0
    /* shared memory device input pin */
#ifndef INCLUDE_DSHM
#ifdef INCLUDE_SM_COMMON
#if (BCM_FAMILY == BCM_SB1A)
    {K_INT_MBOX_0_3, "sm", 0, 0},
#elif (BCM_FAMILY == BCM_SB1)
    {K_INT_MBOX_3, "sm", 0, 0}, /* shared memory or IPI */
#endif /* (BCM_FAMILY == BCM_SB1A) */
#endif /* INCLUDE_SM_COMMON */

    /* distributed shared memory device input pin */
#else
#if (BCM_FAMILY == BCM_SB1A)
    {K_INT_MBOX_0_3, "dshmBusCtlrSibyte", 0, 0},
#elif (BCM_FAMILY == BCM_SB1)
    {K_INT_MBOX_3, "dshmBusCtlrSibyte", 0, 0}, /* shared memory or IPI */
#endif /* (BCM_FAMILY == BCM_SB1A) */
#endif /* INCLUDE_DSHM */
#endif

    /* mailbox interrupts used for IPIs.  the mailbox 0 input pin is
     * used for CPCs in SMP mode
     */
#if (BCM_FAMILY == BCM_SB1A)
    {K_INT_MBOX_0_0, "ipi", 0, 0},
    {K_INT_MBOX_0_1, "ipi", 0, 1},
    {K_INT_MBOX_0_2, "ipi", 0, 2},
    {K_INT_MBOX_0_3, "ipi", 0, 3},
#elif (BCM_FAMILY == BCM_SB1)
    {K_INT_MBOX_0, "ipi", 0, 0},
    {K_INT_MBOX_1, "ipi", 0, 1},
    {K_INT_MBOX_2, "ipi", 0, 2},
    {K_INT_MBOX_3, "ipi", 0, 3},
#endif /* (BCM_FAMILY == BCM_SB1A) */

    };

/* sibyte interrupt controller input pin to output pin assignments */

const struct intrCtlrXBar mipsSbIntCtlrXBar[] =
    {

    /* aux. clock output pin */
#ifdef DRV_TIMER_SB1
/* secondary CPU (SB1_CPU_1) defaults to timer 1 */
#if defined(SB1_CPU_1)
    { K_INT_TIMER_1, 2 },
#else /* primary CPU defaults to timer 0 */
    { K_INT_TIMER_0, 2 },
#endif /* defined(SB1_CPU_1) */
#endif /* DRV_TIMER_SB1 */

    
#if 1
/* mipsSbIntCtlrXBar 20091210 */
#ifdef DRV_SIO_NS16550
	{10, 1}, 
#endif 
#endif

#ifdef INCLUDE_ATSEMAC
	{4, 0}, 
#endif 

//#ifdef HRMATRIXDMA0_DEVICE
#ifdef INCLUDE_HRMATRIXDMA0
 HRMATRIXDMA0_XBAR_DESC,
#endif

//#ifdef HRMATRIXDMA1_DEVICE
#ifdef INCLUDE_HRMATRIXDMA1
HRMATRIXDMA1_XBAR_DESC,
#endif

#ifdef INCLUDE_OBC_INT0
		{0,3},
#endif

#ifdef INCLUDE_OBC_INT1
		{1,3},
#endif

#ifdef INCLUDE_OBC_INT2
		{2,3},
#endif

#ifdef INCLUDE_OBC_INT3
		{3,3},
#endif

    /* mailbox interrupts used for IPIs.  output pin 4 is reserved for
     * CPC IPIs when in SMP mode
     */
#if (BCM_FAMILY == BCM_SB1A)
    {K_INT_MBOX_0_0, 4},
    {K_INT_MBOX_0_1, 4},
    {K_INT_MBOX_0_2, 4},
    {K_INT_MBOX_0_3, 4},
#elif (BCM_FAMILY == BCM_SB1)
    {K_INT_MBOX_0, 4},
    {K_INT_MBOX_1, 4},
    {K_INT_MBOX_2, 4},
    {K_INT_MBOX_3, 4},
#endif /* (BCM_FAMILY == BCM_SB1A) */

    };

#if defined(_WRS_CONFIG_SMP)  /* only route to multiple CPUs in SMP */

/* sibyte interrupt controller input pin to destination CPU assignments */

const struct intrCtlrCpu mipsSbIntCtlrCpuRoute[] =
    {

    /* aux. clock dest. cpu */
#ifdef DRV_TIMER_SB1
/* secondary CPU (SB1_CPU_1) defaults to timer 1 */
#if defined(SB1_CPU_1)
    { K_INT_TIMER_1, 0 },
#else /* primary CPU defaults to timer 0 */
    { K_INT_TIMER_0, 0 },
#endif /* defined(SB1_CPU_1) */
#endif /* DRV_TIMER_SB1 */

   
#if 1
/* mipsSbIntCtlrCpuRoute 20091210 */
#ifdef DRV_SIO_NS16550
	{10, 0}, 
#endif 
#endif

#ifdef INCLUDE_ATSEMAC
	{4, 0}, 
#endif 

//#ifdef HRMATRIXDMA0_DEVICE
#ifdef INCLUDE_HRMATRIXDMA0
	HRMATRIXDMA0_ROUTE_DESC,
#endif

//#ifdef HRMATRIXDMA1_DEVICE
#ifdef INCLUDE_HRMATRIXDMA1
	HRMATRIXDMA1_ROUTE_DESC,
#endif

#ifdef INCLUDE_OBC_INT0
			{0,0},
#endif

#ifdef INCLUDE_OBC_INT1
			{1,0},
#endif

#ifdef INCLUDE_OBC_INT2
			{2,0},
#endif

#ifdef INCLUDE_OBC_INT3
			{3,0},
#endif


    };
#endif /* defined(_WRS_CONFIG_SMP) */

/* sibyte interrupt controller including all resources defined above */

const struct hcfResource mipsSbIntCtlrResources[] = {
    { "regBase",        HCF_RES_INT,    {(void *)TRUE} },

    { "input",  HCF_RES_ADDR, {(void *)&mipsSbIntCtlrInputs[0]} },
    { "inputTableSize", HCF_RES_INT, {(void *)NELEMENTS(mipsSbIntCtlrInputs)}},
    { "crossBar", HCF_RES_ADDR, {(void *)&mipsSbIntCtlrXBar[0]} },
    { "crossBarTableSize", HCF_RES_INT, {(void *)NELEMENTS(mipsSbIntCtlrXBar)}},
    { "bcmFamily", HCF_RES_INT, {(void *)BCM_FAMILY} },

#if defined(_WRS_CONFIG_SMP)  /* only route to multiple CPUs in SMP */
    { "cpuRoute", HCF_RES_ADDR, {(void *)&mipsSbIntCtlrCpuRoute[0]} },
    { "cpuRouteTableSize", HCF_RES_INT, {(void *)NELEMENTS(mipsSbIntCtlrCpuRoute)} }
#endif /* defined(_WRS_CONFIG_SMP) */

};
#define mipsSbIntCtlrNum NELEMENTS(mipsSbIntCtlrResources)

/* CPU 0 interrupt controller input pin to device assignments
 * pin 7 has an example entry added for support of legacy
 * API calls intConnect and intDisconnect.  the first three fields
 * of the entry need to be the input pin number followed by the 
 * device name "legacy" with a unit number of 0.  the final field
 * specifies the index (old style vector) for the ISR and this number
 * needs to match the vector specified in the intConnect/intDisconnect
 * call.
 */

const struct intrCtlrInputs mipsIntCtlr0Inputs[] =
    {
    /* pin, driver, unit, index */

    /*  interrupts inputs into cpu */
    {0, "swtrap", 0, 0},
    {1, "swtrap", 1, 0},
    {2, "mipsZrIntCtlr", 0, 0},
    {3, "mipsZrIntCtlr", 0, 1},
    {4, "mipsZrIntCtlr", 0, 2},
    {5, "mipsZrIntCtlr", 0, 3},
    {6, "mipsZrIntCtlr", 0, 4},

    /* MIPS cpu decrement counter */
    {7, "r4KTimerDev", 0, 0},
    {7, "legacy", 0, 7},

    };
const struct hcfResource cpu0Resources[] = {
    { "regBase",       HCF_RES_INT,    {(void *)TRUE} },

    { "input",  HCF_RES_ADDR,  {(void *)&mipsIntCtlr0Inputs[0]} },
    { "inputTableSize", HCF_RES_INT, {(void *)NELEMENTS(mipsIntCtlr0Inputs)}},

};
#define cpu0Num NELEMENTS(cpu0Resources)

#if (VX_SMP_NUM_CPUS > 1) && defined(_WRS_CONFIG_SMP)

/* CPU 1 interrupt controller input pin to device assignments  */

const struct intrCtlrInputs mipsIntCtlr1Inputs[] =
    {
    /* pin, driver, unit, index */

    /*  interrupts inputs into cpu */
    {0, "swtrap", 0, 1},
    {1, "swtrap", 1, 1},
    {2, "mipsZrIntCtlr", 0, 8},
    {3, "mipsZrIntCtlr", 0, 9},
    {4, "mipsZrIntCtlr", 0, 10},
    {5, "mipsZrIntCtlr", 0, 11},
    {6, "mipsZrIntCtlr", 0, 12},
    {7, "r4KTimerDev", 1, 0},

    };
const struct hcfResource cpu1Resources[] = {
    { "regBase",       HCF_RES_INT,    {(void *)TRUE} },

    { "input",  HCF_RES_ADDR,  {(void *)&mipsIntCtlr1Inputs[0]} },
    { "inputTableSize", HCF_RES_INT, {(void *)NELEMENTS(mipsIntCtlr1Inputs)}},

};
#define cpu1Num NELEMENTS(cpu1Resources)

#if (VX_SMP_NUM_CPUS > 2)

/* CPU 2 interrupt controller input pin to device assignments  */

const struct intrCtlrInputs mipsIntCtlr2Inputs[] =
    {
    /* pin, driver, unit, index */

    /*  interrupts inputs into cpu */
    {0, "swtrap", 0, 2},
    {1, "swtrap", 1, 2},
    {2, "mipsZrIntCtlr", 0, 16},
    {3, "mipsZrIntCtlr", 0, 17},
    {4, "mipsZrIntCtlr", 0, 18},
    {5, "mipsZrIntCtlr", 0, 19},
    {6, "mipsZrIntCtlr", 0, 20},
    {7, "r4KTimerDev", 2, 0},

    };
const struct hcfResource cpu2Resources[] = {
    { "regBase",       HCF_RES_INT,    {(void *)TRUE} },

    { "input",  HCF_RES_ADDR, {(void *)&mipsIntCtlr2Inputs[0]} },
    { "inputTableSize", HCF_RES_INT, {(void *)NELEMENTS(mipsIntCtlr2Inputs)}},

};
#define cpu2Num NELEMENTS(cpu2Resources)

#if (VX_SMP_NUM_CPUS > 3)

/* CPU 3 interrupt controller input pin to device assignments  */

const struct intrCtlrInputs mipsIntCtlr3Inputs[] =
    {
    /* pin, driver, unit, index */

    /*  interrupts inputs into cpu */
    {0, "swtrap", 0, 3},
    {1, "swtrap", 1, 3},
    {2, "mipsZrIntCtlr", 0, 24},
    {3, "mipsZrIntCtlr", 0, 25},
    {4, "mipsZrIntCtlr", 0, 26},
    {5, "mipsZrIntCtlr", 0, 27},
    {6, "mipsZrIntCtlr", 0, 28},
    {7, "r4KTimerDev", 3, 0},

    };
const struct hcfResource cpu3Resources[] = {
    { "regBase",       HCF_RES_INT,    {(void *)TRUE} },

    { "input",  HCF_RES_ADDR, {(void *)&mipsIntCtlr3Inputs[0]} },
    { "inputTableSize", HCF_RES_INT, {(void *)NELEMENTS(mipsIntCtlr3Inputs)}},

};
#define cpu3Num NELEMENTS(cpu3Resources)

#endif /* (VX_SMP_NUM_CPUS > 3) */
#endif /* (VX_SMP_NUM_CPUS > 2) */
#endif /* defined(_WRS_CONFIG_SMP) && (VX_SMP_NUM_CPUS > 1) */

/* Distributed SHared Memory support
 */
#ifdef INCLUDE_DSHM
#include <dshm/util/dshmSm.h>

void * dshmRegBaseDummy [10];   /* needed to appease vxbus */

#ifdef INCLUDE_SM_COMMON
    /* if SM is included, push DSHM after it */
    #include <smLib.h>
    #define DSHM_ANCHOR_OFFSET  (ROUND_UP (sizeof (SM_ANCHOR), \
                                    _CACHE_ALIGN_SIZE))
#else
    extern BOOL vxTas (void *);
    #define DSHM_ANCHOR_OFFSET  0
#endif

#define DSHM_SIBYTE_N_CORES 2
#define DSHM_SM_POOL_SIZE   (0x80000)
#define DSHM_ANCHOR_BASE    (0x80000600 + DSHM_ANCHOR_OFFSET)
#define DSHM_ANCHOR_CORE(core) \
    (DSHM_ANCHOR_BASE + \
        core * ROUND_UP (DSHM_ANCHOR_SIZE(DSHM_SIBYTE_N_CORES), \
                                    _CACHE_ALIGN_SIZE))

/* add entries here if more cores are supported */
#if ((defined SB1_CPU_0) || (defined SB_CPU_0))
    #define DSHM_ANCHOR_LOCAL DSHM_ANCHOR_CORE(0)
#elif ((defined SB1_CPU_1) || (defined SB_CPU_1))
    #define DSHM_ANCHOR_LOCAL DSHM_ANCHOR_CORE(1)
#endif

const struct hcfResource dshmBusCtlrSibyteRes[] = {
    { "regBase",    RES_INT,  (void *) dshmRegBaseDummy },
    { "bus_type",   RES_ADDR, (void *) "plb" },
    { "anchor",     RES_ADDR, (void *) DSHM_ANCHOR_LOCAL },
    { "smStart",    RES_ADDR, (void *) -1 },
    { "szPool",     RES_INT,  (void *) DSHM_BUS_PLB_POOLSIZE },
    { "node",       RES_INT,  (void *) DSHM_BUS_PLB_NODE },
    { "rmw",        RES_ADDR, (void *) vxTas },
    { "nRetries",   RES_INT,  (void *) DSHM_BUS_PLB_NRETRIES },
    { "nNodesMax",  RES_INT,  (void *) DSHM_BUS_PLB_MAXNODES },
    { "nEntries",   RES_INT,  (void *) DSHM_BUS_PLB_NENTRIES },
    { "szEntry",    RES_INT,  (void *) DSHM_BUS_PLB_ENTRY_SIZE },
};

/* add entries here if more cores are supported */
const struct hcfResource dshmPeerVxSibyte0Res[] = {
    { "regBase",    RES_INT,  (void *)dshmRegBaseDummy },
    { "node",       RES_INT,  (void *)0 },
    { "anchor",     RES_ADDR, (void *)DSHM_ANCHOR_CORE(0) },
};

const struct hcfResource dshmPeerVxSibyte1Res[] = {
    { "regBase",    RES_INT,  (void *)dshmRegBaseDummy },
    { "node",       RES_INT,  (void *)1 },
    { "anchor",     RES_ADDR, (void *)DSHM_ANCHOR_CORE(1) },
};
#endif  /* INCLUDE_DSHM */

/* device list */

const struct hcfDevice hcfDeviceList[] = {

/* sibyte and MIPS cpu interrupt controllers */
#ifdef DRV_INTCTLR_MIPS
    { "mipsZrIntCtlr", 0, VXB_BUSID_PLB, 0, mipsSbIntCtlrNum, \
      mipsSbIntCtlrResources },
    { "mipsIntCtlr", 0, VXB_BUSID_PLB, 0, cpu0Num, cpu0Resources },
#if defined(_WRS_CONFIG_SMP) && (VX_SMP_NUM_CPUS > 1)
    { "mipsIntCtlr", 1, VXB_BUSID_PLB, 0, cpu1Num, cpu1Resources },
#if (VX_SMP_NUM_CPUS > 2)
    { "mipsIntCtlr", 2, VXB_BUSID_PLB, 0, cpu2Num, cpu2Resources },
#if (VX_SMP_NUM_CPUS > 3)
    { "mipsIntCtlr", 3, VXB_BUSID_PLB, 0, cpu3Num, cpu3Resources },
#endif /* (VX_SMP_NUM_CPUS > 3) */
#endif /* (VX_SMP_NUM_CPUS > 2) */
#endif /* defined(_WRS_CONFIG_SMP) && (VX_SMP_NUM_CPUS > 1) */
#endif /* DRV_INTCTLR_MIPS */

#if 1
/* Added for ns16550 serial devices! 20091211 */
#ifdef	DRV_SIO_NS16550
    { "ns16550", 0, VXB_BUSID_PLB, 0, ns16550Dev0Num, ns16550Dev0Resources },
    /*{ "ns16550", 1, VXB_BUSID_PLB, 0, ns16550Dev1Num, ns16550Dev1Resources },*/
#endif	/* DRV_SIO_NS16550 */ 
#endif 

#ifdef INCLUDE_ATSEMAC
    { ATSEMAC_NAME, ATSEMAC_UNIT0_DEVICE_ID, VXB_BUSID_PLB, 0, atsemac0Num, atsemac0Resources },
#endif

//#ifdef HRMATRIXDMA0_DEVICE
#ifdef INCLUDE_HRMATRIXDMA0
	HRMATRIXDMA0_DEVICE_DESC,
#endif

//#ifdef HRMATRIXDMA1_DEVICE
#ifdef INCLUDE_HRMATRIXDMA1
	HRMATRIXDMA1_DEVICE_DESC,
#endif

#ifdef INCLUDE_OBC_INT0
	{ "obcintdev0",0,VXB_BUSID_PLB,0,hrObcintdev0Num,hrObcintdev0Resources},
#endif


#ifdef INCLUDE_OBC_INT1
	{ "obcintdev1",0,VXB_BUSID_PLB,0,hrObcintdev1Num,hrObcintdev1Resources},
#endif


#ifdef INCLUDE_OBC_INT2
	{ "obcintdev2",0,VXB_BUSID_PLB,0,hrObcintdev2Num,hrObcintdev2Resources},
#endif

#ifdef INCLUDE_OBC_INT3
	{ "obcintdev3",0,VXB_BUSID_PLB,0,hrObcintdev3Num,hrObcintdev3Resources},
#endif

#if 1  // mxl debug
/* system clock */
    { "r4KTimerDev", 0, VXB_BUSID_PLB, 0, r4TimerDevNum, r4KTimerDevResources},
#if defined(_WRS_CONFIG_SMP) && (VX_SMP_NUM_CPUS > 1)
    { "r4KTimerDev", 1, VXB_BUSID_PLB, 0, r4TimerDevNum, r4KTimerDevResources},
#if (VX_SMP_NUM_CPUS > 2)
    { "r4KTimerDev", 2, VXB_BUSID_PLB, 0, r4TimerDevNum, r4KTimerDevResources},
    { "r4KTimerDev", 3, VXB_BUSID_PLB, 0, r4TimerDevNum, r4KTimerDevResources},
#endif /* (VX_SMP_NUM_CPUS > 2) */
#endif /* defined(_WRS_CONFIG_SMP) && (VX_SMP_NUM_CPUS > 1) */
#endif

#if 1  // mxl debug
/* sibyte aux. clock */
#ifdef DRV_TIMER_SB1
    { "sb1TimerDev", 0, VXB_BUSID_PLB, 0, sb1TimerDevNum0, sb1TimerDevResources0},
#endif /* DRV_TIMER_SB1 */
#endif
#if 0
    /* shared memory device */
#ifdef INCLUDE_SM_COMMON
    { "sm", 0, VXB_BUSID_PLB, 0, smDevNum, smDevResources},
#endif /* INCLUDE_SM_COMMON */
#else /* added by yinwx, 20100202 */
#ifdef INCLUDE_SM_COMMON
	/* added by yinwx, 20100127 for vxbus virtual NIC device */
	{ "smEnd", 0, VXB_BUSID_PLB, 0, smEndDevNum, smEndDevResources},
#endif /* INCLUDE_SM_COMMON */
#endif

#ifdef INCLUDE_DSHM
#ifdef INCLUDE_DSHM_BUS_PLB
    { "dshmBusCtlrSibyte",  0, VXB_BUSID_PLB,     0,
        NELEMENTS(dshmBusCtlrSibyteRes), dshmBusCtlrSibyteRes },
    { "dshmPeerVxSibyte", 0, VXB_BUSID_VIRTUAL, 0,
        NELEMENTS(dshmPeerVxSibyte0Res), dshmPeerVxSibyte0Res },
    { "dshmPeerVxSibyte", 1, VXB_BUSID_VIRTUAL, 0,
        NELEMENTS(dshmPeerVxSibyte1Res), dshmPeerVxSibyte1Res },
#endif  /* INCLUDE_DSHM_BUS_PLB */
#endif  /* INCLUDE_DSHM */

};

const int hcfDeviceNum = NELEMENTS(hcfDeviceList);

VXB_INST_PARAM_OVERRIDE sysInstParamTable[] =
    {
    { NULL, 0, NULL, VXB_PARAM_END_OF_LIST, {(void *)0} }
    };
