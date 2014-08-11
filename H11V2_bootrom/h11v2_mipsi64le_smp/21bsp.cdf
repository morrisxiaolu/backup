/* 21bsp.cdf - BSP component description file */

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
01n,29oct08,dcc  removed MP_OPTIONS
01m,17oct08,slk  fix BOOTAPP profile
01l,17oct07,rlg  fix WIND001074444
01k,15oct07,kab  WIND00107552 - Set FP type
01j,29aug07,jmg  Added VX_SMP_NUM_CPUS
01i,09aug07,rlg  Changes for the MIPS Library restructuring
01h,26jul07,rlg  N32 compiler CPU variation name change
01g,25jul07,agf  add VX_CPU_ID component
01f,19jul07,ebh  Add AMP advertising option
01e,23feb07,jmt  Defect 88750 - Fix problem with network boot device CDF
                 defines
01d,17jan07,rlg  Updates to BSP structure adding CPU type and Endian type
01c,25sep06,rlg  fix for bootapp
01b,22aug06,rlg  move Memory Definitions from 20bsp.cdf to here so can set up
                 multi core options
01a,24jul06,jmt  created from 20bsp.cdf 

DESCRIPTION

This file contains updated descriptions for some vxWorks components
defined in 00bsp.cdf. It updates those definitions with new ones needed
to support the coprocessor abstraction.
*/

/* Describe the BSP to the project tools */

Bsp     h11v2_mipsi64le_smp {
	NAME		board support package
	CPU		MIPSI64
	MP_OPTIONS	SMP
	ENDIAN		little
	FP		hard
	REQUIRES	INCLUDE_KERNEL		\
			INCLUDE_BCM91480B	\
                        DRV_TIMER_MIPSR4K       \
                        DRV_INTCTLR_MIPS        \
                        DRV_SIO_NS16550         \ 
                        INCLUDE_GENERICPHY

}

/* Memory definitions */

Parameter LOCAL_MEM_LOCAL_ADRS {
        NAME            Physical memory base address
        DEFAULT         (INCLUDE_MAPPED_KERNEL)::(0xc0000000) \
                        0x80000000
}

Parameter RAM_HIGH_ADRS {
        NAME            Bootrom Copy region
        DEFAULT         (INCLUDE_BOOT_RAM_IMAGE & INCLUDE_MAPPED_KERNEL)::(0xcf000000) \
                        (INCLUDE_BOOT_RAM_IMAGE)::(0x8f000000) \
                        (INCLUDE_BOOT_APP & INCLUDE_MAPPED_KERNEL)::(0xcf000000)\
                        (INCLUDE_BOOT_APP)::(0x8f000000) \
                        (INCLUDE_MAPPED_KERNEL)::(0xc0700000) \
                        0x80700000
}

Parameter RAM_LOW_ADRS {
        NAME            Runtime kernel load address
        DEFAULT         (INCLUDE_BOOT_RAM_IMAGE & INCLUDE_MAPPED_KERNEL)::(0xc0c00000) \
                        (INCLUDE_BOOT_RAM_IMAGE)::(0x80c00000) \
                        (INCLUDE_BOOT_APP & INCLUDE_MAPPED_KERNEL)::(0xc1000000)\
                        (INCLUDE_BOOT_APP)::(0x81000000) \
                        (INCLUDE_MAPPED_KERNEL)::(0xc0200000) \
                        0x80200000
}
/* Serial IO definitions */

Component INCLUDE_SIO_POLL {
        INCLUDE_WHEN    += INCLUDE_KERNEL
        REQUIRES +=     INCLUDE_SB1_UART_CHAN_A \
			INCLUDE_SB1_UART_CHAN_B \
			INCLUDE_SB1_UART_CHAN_C \
			INCLUDE_SB1_UART_CHAN_D
}

