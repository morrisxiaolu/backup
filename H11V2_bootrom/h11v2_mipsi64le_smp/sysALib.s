/* sysALib.s - BCM1250 system-dependent assembly routines */

/*
 * Copyright (c) 2001-2002, 2004-2007 Wind River Systems, Inc.
 *
 * The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.
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
02g,29aug08,kab  Change _WRS_VX_SMP to _WRS_CONFIG_SMP
02f,14feb07,pes  Clear BEV bit in SR when starting secondary cores.
02e,02feb07,pes  Add initialization of -1 page for secondary cores if mapped
                 kernel.
02d,19oct06,pes  Move sysCpuInit() function here from sysLib.c to provide
                 assembler entry point for secondary cores.
02c,26sep06,pes  Eliminate use of sysCpuLoop. Remove some unused code.
02b,26jul06,pes  Remove references to asmMipsP.h and the CPU_INDEX macro.
02a,20jul06,pes  Add #include of asmMipsP.h.
01z,21jun06,pes  Fix sysBootParamsGet to not overwrite beginning of text.
01y,24sep06,rlg  Fix to cfeArgs storage enables both romable and bootable images
01x,15aug06,rlg  Fix to avoid overwriting the first 4 bytes of kernel text. 
                 caused target server not to connect. Checksum problems.
01w,08jul06,wap  Fix compilation when built without CFE support
01v,16may06,pes  Another Macro correction
01u,16may06,pes  Correct conditional selection around CPU_INDEX macro.
01t,12may06,pes  Add handling of -1 page for multiple cores.
01s,11may06,pes  Add support for secondary CPU core startup.
01r,13apr06,pes  Add backward support for vxWorks 6.2.
01q,10apr06,pes  Fix Mapped kernel support.
01p,14mar06,pes  Add CFE booting support. Use sibyte.h to determine target
                 type.
01o,10jan06,pes  SPR 109182: Switch to use of common mipsTlbClear routine.
01n,03aug05,dr   removed copyright_wind_river.
01m,07oct04,agf  remove use of LOCAL_MEM_LOCAL_ADRS_RUNT
01l,07oct04,agf  Modify MMU init code to use standard routines, apply
                 addt'l vxWorks 6.0 clean-up
01k,06oct04,pes  Changed INCLUDE_MMU_BASIC with INCLUDE_MAPPED_KERNEL
01j,04aug04,slk  Replace MAPPED base MACRO
01i,03aug04,agf  change MMU include from FULL to BASIC
01h,05jun04,pes  Remove references to sysUseExcTlbPage, which are no longer in
                 use.
01g,17may04,agf  AIM AD-MMU support (kernel in KSEG2)
01f,03oct02,agf  changes for shared sentosa support
01e,21jan02,tlc  Removed special version of sysBusTas().  The ll-sc version of
                 sysBusTas is now provided by vxTas(). (SPR #70336 fix).
01d,14jan02,pes  Eliminate '#'-style comments
01c,19dec01,tlc  Remove 'c0' instruction.
01b,06dec01,agf  add sysConfig1Set routine
01a,15nov01,agf  written
*/

/*
DESCRIPTION
This module contains system-dependent routines written in assembly
language.

This module must be the first specified in the \f3ld\f1 command used to
build the system.  The sysInit() routine is the system start-up code.
*/

#define _ASMLANGUAGE
#include <vxWorks.h>
#include <vsbConfig.h>
#include <sysLib.h>
#include <asm.h>
#include <esf.h>
#include <arch/mips/mmuMipsLib.h>
#include "config.h"
#include "sibyte.h"
	
#if (BCM_FAMILY == BCM_SB1)
#include <drv/multi/sb1Lib.h>
#elif (BCM_FAMILY == BCM_SB1A)
#include <drv/multi/bcm1480Lib.h>
#else
#error "Unknown BCM_FAMILY"
#endif

	/* defines */
#define LED_CHAR0       (32+8*3)
#define LED_CHAR1       (32+8*2)
#define LED_CHAR2       (32+8*1)
#define LED_CHAR3       (32+8*0)

/* Seal indicating CFE's presence, passed to user program. */
#define CFE_EPTSEAL 0x43464531
	
/* in case not defined in installed version of Tornado */
#ifndef TLBHI_VPN2MASK
#define	TLBHI_VPN2MASK	0xffffe000
#endif  /* TLBHI_VPN2MASK */

#define N_TLB_ENTRIES   64     /* bcm1250 has 64 rows */


	/* internals */

	.globl	sysInit			/* start of system code */
	.globl	sysWbFlush		/* flush write buffers */
	.globl	sysFpaDeMux		/* which FPA int is bugging us */
	.globl	sysFpaAck		/* clear FPA interrupt */
	.globl  sysLedDsply
	.globl  sysAltGo
	.globl	sysSetTlbEntry		/* Set entry in tlb, addded from 2F code */
	.globl	sysSMSetTlbEntry	/* Set entry in tlb, for SM only, yinwx 20100204 */

	/* externals */

	.extern	usrInit		/* system initialization routine */
	.extern	mipsTlbClear
	.extern vxCpuIndexGet

/*added by lw*/
  .globl  sysTlbReadIdx	
 	.globl  sysTlbMaskGet
 	.globl  sysTlbIdxSet
	.globl  sysTlbLO0Get 
	.globl  sysTlbLO1Get
	.globl  sysTlbHIGet	 	
/*end */ 	
   
	/* macros */
/* in archMips.h these macros are not assembler friendly, so fix for here */
#undef  PHYS_TO_K0
#define PHYS_TO_K0(x)   (K0BASE | (x))
#undef  PHYS_TO_K1
#define PHYS_TO_K1(x)   (K1BASE | (x))

#define SETLEDS(a,b,c,d)                                /* \
       li     a0,(((a)<<24)|((b)<<16)|((c)<<8)|(d)) ;    \
       jal    sysLedDsply*/ /* commented by yinwx, 2009-11-20 */
       
#define PRINTSTR(x) \
        .rdata;98: .asciz x; .text; la a0, 98b; bal stringserial; nop
        
       
	.text
/*******************************************************************************
*
* sysInit - start after boot
*
* This routine is the system start-up entry point for VxWorks in RAM, the
* first code executed after booting.  It disables interrupts, sets up the
* stack, and jumps to the C routine usrInit() in usrConfig.c.
*
* The initial stack is set to grow down from the address of sysInit().  This
* stack is used only by usrInit() and is never used again.  Memory for the
* stack must be accounted for when determining the system load address.
*
* NOTE: This routine should not be called by the user.
*
* RETURNS: N/A

* sysInit (void)	/@ THIS IS NOT A CALLABLE ROUTINE @/

*/

	.ent	sysInit
sysInit:
#if 0 /* yinwx, 09-11-20 */
#ifdef INCLUDE_CFE_SUPPORT
	/* 
	 * The boot strategy works like this:	
	 * 
	 * First, we detect the boot method
	 * 
	 * a0 == 0 this was a warm boot (BOOT_NORMAL)
	 * a0 == 1 this was a non-autoboot (BOOT_NO_AUTOBOOT)
	 * a0 == 2 this was a cold boot (BOOT_CLEAR)
	 * a0 == 4 this was a quick autoboot (BOOT_QUICK_AUTOBOOT)
	 * a0 <  0 if booted from CFE (a0 contains CFE handle)
	 *
	 *
	 * Cold boot occurs in one of two situations:
	 * 1. Booting from CFE
	 * 2. Reboot with type BOOT_CLEAR requested.
	 *
	 * When booting from CFE, a0-a3 have the CFE-provided
	 * parameters:
	 * a0 = CFE handle
	 * a1 = 0
	 * a2 = CFE firmware entry vector
	 * a3 = CFE entry point seal (0x43464531, 'CFE1')
	 *
	 * These values are unconditionally stored immediately after
	 * the bootline for use by subsequent reboots. Then, startType 
	 * is set to BOOT_CLEAR and booting proceeds as usual.
	 *
	 * When a software cold reboot occurs, the CFE parameters are
	 * assumed to already be in place before the text segment, so 
	 * they don't have to be copied there. It also means that 
	 * startType already contains BOOT_CLEAR.
	 *
	 * We have one more cold boot situation to account for:
	 * After booting the bootApp, it downloads and starts the 
	 * runtime kernel at RAM_LOW_ADRS. But the CFE parameters
	 * have not been stored before the downloaded text.
	 *
	 * This is where the sysBootParamsGet function comes in. 
	 * After loading the runtime kernel, the bootApp passes the 
	 * address where it was loaded (and begins execution) to 
	 * sysBootParamsGet. sysBootParamsGet then places a copy of 
	 * CFE boot parameters in the 4 double words preceeding
	 * that entry point, thus providing the parameters to the
	 * runtime kernel.
	 *
	 * At this point, startType is BOOT_CLEAR, and the bootApp
	 * is ready to begin executing the runtime kernel, which it
	 * does, passing the startType in a0, as always.
	 *
	 * For any type of warm boot, we enter the bootApp with the
	 * required startType. This start type can be distinguished
	 * from a CFE (cold) boot by the sign of a0: a negative value is
	 * passed by CFE, a positive one by a runtime kernel. 
	 * In the case of all positive values, the bootApp reloads the 
	 * runtime kernel, calls sysBootParamsGet, and re-enters the 
	 * loaded kernel with the start type requested.
	 */

	bltz	a0,cfeBoot	/* if a0 is a pointer, this is cfe booting */
	
	/* vxWorks boot */
	move	s0,a0		/* save boot type */

	b	fromBootApp
	
cfeBoot:
	
	/* save CFE information before text. */
        li      t0, CFEARGS
#if defined(INCLUDE_MAPPED_KERNEL)
	/* for mapped kernel, force t0 into kseg0 */
	and	t0, ADDRESS_SPACE_MASK
	or	t0, K0BASE
#endif	/* INCLUDE_MAPPED_KERNEL */
	sd	a0,   (t0)
	sd	a1,  8(t0)
	sd	a2, 16(t0)
	sd	a3, 24(t0)
	
	li	s0, BOOT_CLEAR	/* CFE boot is cold by definition */
	
fromBootApp:	
#endif	/* INCLUDE_CFE_SUPPORT */
#endif  /* #if 0 */

    /* vxWorks boot */
	move	s7,a0		/* save boot type */
	
	/* disable all interrupts, fpa usable */
	li	t0, SR_CU0
	mtc0	t0, C0_SR
	mtc0	zero, C0_CAUSE

	/* clear WIRED register */

	mtc0	zero, C0_WIRED
	HAZARD_CP_WRITE

	/* clear TLB setup done by bootrom */
	
#if (_WRS_VXWORKS_MAJOR >= 6) && (_WRS_VXWORKS_MINOR >= 3)
	jal	mipsTlbClear
#else
	jal	sysClearTlb
#endif

	
        /* initialize stack pointer */
	
	/* 
	 * set stack to grow down from code. The four parameters on 
	 * stack are the 4 CFE parameters. For mapped kernels,
	 * coerce stack to be in kseg0.
	 */
	la	sp, (sysInit - 4*(_RTypeSize))

#if defined(INCLUDE_MAPPED_KERNEL)
	and	sp, ADDRESS_SPACE_MASK
	or	sp, K0BASE


	/* install initial memory map */

	li a0, '3'
	bal tgt_putchar
	nop
	
	move	a0,sp
	bal		printnum
	#if 0
	dli	    sp,0xffffffff8edffef0 /*zxj 20100203*/
	move	a0,sp
	bal		printnum
	#endif
	nop

	
	li	a0, EXC_PAGE_PHYS_ADRS
	li	a1, LOCAL_MEM_LOCAL_ADRS
	li	a2, (LOCAL_MEM_LOCAL_ADRS + LOCAL_MEM_SIZE)
	jal	mmuMipsInitialMemoryMapSet
	nop
	beqz	v0, 1f
	nop
0:
	/* memory mapping error */

	
	li a0, 'A'
	bal tgt_putchar
	nop
	b	0b
	nop

1:
	
	/* tlb setup in place, switch to mapped address */
	li a0, 'B'
	bal tgt_putchar
	nop

	la	t0, 0f
	j	t0
	nop
0:
#endif /* INCLUDE_MAPPED_KERNEL */
	
	/* give as long as possible before a clock interrupt */
	li	v0, 1
	mtc0	v0, C0_COUNT
	mtc0	zero, C0_COMPARE
	
	la	gp, _gp

	SETLEDS('v','x','W','o')
	li a0, '.'
	bal tgt_putchar
	nop

	li a0, '\r'
	bal tgt_putchar
	nop

	li a0, '\n'
	bal tgt_putchar
	nop
	
	move	a0, s7			 /* restore start type */

	/*li	a0, BOOT_WARM_AUTOBOOT 	 push start type arg = WARM_BOOT */
	jal	usrInit			/* never returns - starts up kernel */
	li	ra, R_VEC		/* load prom reset address */
	j	ra			/* just in case */
	nop
	.end	sysInit
	
#include "serialPrint.s" /* added by yinwx, 2009-11-20 */

/*******************************************************************************
*
* sysSetTlbEntry - Translation lookaside buffer entry
*
* RETURNS: N/A <<<Added from 2F Code>>>
*
* STATUS sysSetTlbEntry
*     (
*     UINT entry,
*     ULONG virtual,
*     ULONG physical,
*     UINT mode		 Bits 31:12 size, bits 5:0 is cache mode/enable
*     )
*/

#define TLBLO_PAGEMASK		(0x01ffe000)
#define TLBLO_MODE		(0x3f)
#define TLBLO_PAGEMASK_SIZE	(TLBLO_PAGEMASK | TLBLO_MODE | 0x1000)
#define TLBLO_PFNMASK	(0x3fffffc0)

	.ent	sysSetTlbEntry
sysSetTlbEntry:
	subu	t0, a0, TLB_ENTRIES - 1 /* how many tlb entries are there */
	bgtz	t0, badEntry		/* is my index bad ? */

	and	t1, a3, ~TLBLO_PAGEMASK_SIZE/* Check for invalid size */
	bgtz	t1, badEntry		/* Invalid size parameter ? */
	and	t0, a3, (TLBLO_PAGEMASK_SIZE & ~TLBLO_MODE)
	sll	t0, 1
	sub	t0, 1

	/* Set the page size mask */
	and	t0, TLBLO_PAGEMASK
	mtc0	t0, C0_PAGEMASK		

	/* Set the virtual address */
	and	t0, a1, TLBHI_VPN2MASK	/* ASID not used   */
	dmtc0	t0, C0_TLBHI		/* zero tlbhi entry */
	
	/* Set physical address, LO0 register */
	srl	t0, a2, 6			/* Physical address */
	and	t0, TLBLO_PFNMASK
	move	t1, a3			/* Caching characteristic */
	and	t1, TLBLO_MODE
	or	t0, t0, t1
	dmtc0	t0, C0_TLBLO0		/* set valid bit to zero */

	/* Set physical address, LO1 register */
	add	t0, a2, a3		/* Next physical address */
	srl	t0, 6
	and	t0, TLBLO_PFNMASK
	or	t0, t0, t1
	dmtc0	t0, C0_TLBLO1		/* set valid bit to zero */

	/* Set the index into the TLB */
	dmtc0	a0,C0_INX		

 	/* Write the TLB entries */
	tlbwi				/* write indexed tlb entry */
	/* This for fixing Loongson3A's TLB bug, by yinwx, 20100511 */
	mtc0 zero, C0_PAGEMASK
	li	v0, OK
	b	goodEntry
	nop

badEntry:
	li	v0, ERROR
goodEntry:
	j	ra
	nop
	.end	sysSetTlbEntry

/************ sysSMSetTlbEntry for SM only !! ***************/

	.ent	sysSMSetTlbEntry
sysSMSetTlbEntry:
	subu	t0, a0, TLB_ENTRIES - 1 /* how many tlb entries are there */
	bgtz	t0, badEntry2		/* is my index bad ? */
#if 1
    /* get, increment and save the current WIRED value */
    
	mfc0	t5, C0_WIRED   /* added by yinwx, 20100204 */
	HAZARD_CP_READ
	addu	t0, t5, 1
	mtc0	t0, C0_WIRED
	HAZARD_CP_WRITE
#endif
	and	t1, a3, ~TLBLO_PAGEMASK_SIZE/* Check for invalid size */
	bgtz	t1, badEntry2		/* Invalid size parameter ? */
	and	t0, a3, (TLBLO_PAGEMASK_SIZE & ~TLBLO_MODE)
	sll	t0, 1
	sub	t0, 1

	/* Set the page size mask */
	and	t0, TLBLO_PAGEMASK
	mtc0	t0, C0_PAGEMASK		

	/* Set the virtual address */
	and	t0, a1, TLBHI_VPN2MASK	/* ASID not used   */
	dmtc0	t0, C0_TLBHI		/* zero tlbhi entry */
	
	/* Set physical address, LO0 register */
	dli		t0,0x100000000      /*zxj 20100202*/
	or		a2,a2,t0
	dsrl	t0, a2, 6			/* Physical address */
	and	t0, TLBLO_PFNMASK
	move	t1, a3			/* Caching characteristic */
	and	t1, TLBLO_MODE
	or	t0, t0, t1
	dmtc0	t0, C0_TLBLO0		/* set valid bit to zero */

	/* Set physical address, LO1 register */
	dadd	t0, a2, a3		/* Next physical address */
	dsrl	t0, 6
	and	t0, TLBLO_PFNMASK
	or	t0, t0, t1
	dmtc0	t0, C0_TLBLO1		/* set valid bit to zero */

	/* Set the index into the TLB */
	dmtc0	a0,C0_INX		

 	/* Write the TLB entries */
	tlbwi				/* write indexed tlb entry */
	/* This for fixing Loongson3A's TLB bug, by yinwx, 20100511 */
	mtc0 zero, C0_PAGEMASK

	li	v0, OK
	b	goodEntry2
	nop

badEntry2:
	li	v0, ERROR
goodEntry2:
	j	ra
	nop
	.end	sysSMSetTlbEntry

	
/*******************************************************************************
*
* sysWbFlush - flush the write buffer
*
* This routine flushes the write buffers, making certain all subsequent
* memory writes have occurred.  It is used during critical periods only, e.g.,
* after memory-mapped I/O register access.
*
* RETURNS: N/A

* sysWbFlush (void)

*/
	.ent	sysWbFlush
sysWbFlush:
	.set noreorder
        j       ra
	sync
	.set reorder
	.end	sysWbFlush

/*******************************************************************************
*
* sysFpaDeMux - determine which FPA exception is pending
*
* This routine reads the floating point unit (FPU) status to determine which
* FPU exception generated an interrupt to the processor.  It returns an
* index to the vector table.
*
* This routine is loaded into the static interrupt priority table.
* It is called by jumping to the address in this table, not by
* user calls.
*
* RETURNS: An interrupt vector.

* int sysFpaDeMux
*     (
*     int vecbase	/@ base location of FPA vectors in excBsrTbl @/
*     )

*/

	.ent	sysFpaDeMux
sysFpaDeMux:
	.set	noreorder
	cfc1	v0, C1_SR			/* grab FPA status	*/
	nop
	.set	reorder
	li	a2, FP_EXC_MASK			/* load cause mask	*/
	and	a2, v0				/* look at cause only	*/
	srl	a2, FP_EXC_SHIFT		/* place cause in lsb	*/
	li	a1, FP_ENABLE_MASK		/* load enable mask	*/
	and	a1, a1, v0			/* look at enable only	*/
	srl	a1, FP_ENABLE_SHIFT		/* place enabled in lsb	*/
	li	a3, (FP_EXC_E>>FP_EXC_SHIFT)	/* ld unimp op bit	*/
	or	a1, a3				/* no mask bit for this	*/
	and	a2, a1				/* look at just enabled	*/
	and	v1, v0, ~FP_EXC_MASK		/* clear the exceptions */
	lbu	v0, ffsLsbTbl(a2)		/* lkup first set bit	*/
	addu	v0, a0				/* increment io vector	*/
	ctc1	v1, C1_SR			/* clear fp condition	*/
	j	ra				/* return to caller	*/
	.end	sysFpaDeMux

/*******************************************************************************
*
* sysFpaAck - acknowledge a floating point unit interrupt
*
* This routine writes the floating point unit (FPU) status register to
* acknowledge the appropriate FPU interrupt.  It returns an index to the vector
* table.
*
* RETURNS: An interrupt vector.

* int sysFpaAck (void)

*/

	.ent	sysFpaAck
sysFpaAck:
	cfc1	v0, C1_SR		/* read control/status reg	*/
	and	t0, v0, ~FP_EXC_MASK	/* zero bits		*/
	ctc1	t0, C1_SR		/* acknowledge interrupt	*/
	j	ra			/* return to caller		*/
	.end	sysFpaAck


/**************************************************************************
*
* sysLedDsply -  put 4 ascii characters on LED display
*
* RETURNS : N/A

* void sysLedDsply (int )

*/
        .ent	sysLedDsply
sysLedDsply:
#if (BOARD_TYPE == BCM91250A) || (BOARD_TYPE == BCM91480B)
        li      t0,PHYS_TO_K1(LEDS_PHYS)

        rol     a0,a0,8
        and     t1,a0,0xFF
        sb      t1,LED_CHAR0(t0)

        rol     a0,a0,8
        and     t1,a0,0xFF
        sb      t1,LED_CHAR1(t0)

        rol     a0,a0,8
        and     t1,a0,0xFF
        sb      t1,LED_CHAR2(t0)

        rol     a0,a0,8
        and     t1,a0,0xFF
        sb      t1,LED_CHAR3(t0)
#elif (BOARD_TYPE == BCM91250E)  /* SENTOSA or RHONE */
#ifdef DEBUG_LEDS_TO_UART
        li      t0,PHYS_TO_K1(A_DUART_CHANREG(0,R_DUART_STATUS))
1:      ld      t1,0(t0)
        andi    t1,0x08
        beqz    t1,1b            /* wait for buffer to empty */

        li      t0,PHYS_TO_K1(A_DUART_CHANREG(0,R_DUART_TX_HOLD))
        rol     a0,a0,8
        and     t1,a0,0xFF
        sd      t1,0(t0)

        rol     a0,a0,8
        and     t1,a0,0xFF
        sd      t1,0(t0)

        rol     a0,a0,8
        and     t1,a0,0xFF
        sd      t1,0(t0)

        rol     a0,a0,8
        and     t1,a0,0xFF
        sd      t1,0(t0)

        li      t1,13            /* carriage return */
        sd      t1,0(t0)

        li      t1,10            /* line-feed       */
        sd      t1,0(t0)

#endif /* DEBUG_LEDS_TO_UART */
#endif /* BOARD_TYPE */

        j       ra

	.end    sysLedDsply

#ifdef SB1_CPU_0
/**************************************************************************
*
* sysAltGo -  give cpu1 an address where it should start executing from 
*
* Cpu1 is in a tight loop monitoring its mailbox register. If the register
* is written to with a non-zero value, cpu1 will leave its tight loop
* and begin executing from the value that was written to the register.
*
* RETURNS : N/A

* void sysAltGo (int )

*/

        .ent	sysAltGo
sysAltGo:

/* check for cpu type 1480 has more mailbox combinations than the 1250 */
#if (BCM_FAMILY == BCM_SB1 )
                la      t1,PHYS_TO_K1(A_IMR_REGISTER(1,R_IMR_MAILBOX_SET_CPU))
#else
                la      t1,PHYS_TO_K1(A_IMR_REGISTER(1,R_IMR_MAILBOX_0_SET_CPU))
#endif /* BCM_SB1 */
                sd      a0,0(t1)        /* Write to mailbox register */

                j       ra

        .end	sysAltGo

#endif /* SB1_CPU_0 */

#ifdef _WRS_CONFIG_SMP

/**************************************************************************
* godson3_cpu_start(cpu,pc,sp,gp,a1) by zxj 20091212
**************************************************************************/

#define MAILBOX_BUF1    0x900000003ff01120
#define MAILBOX_BUF2    0x900000003ff01220
#define MAILBOX_BUF3    0x900000003ff01320
	.globl godson3_cpu_start
	.ent godson3_cpu_start
godson3_cpu_start:
	li	t0,0x1
	bne a0,t0,1f
	nop
	dli	t0,MAILBOX_BUF1
	b	2f
1:
	li	t0,0x2
	bne	a0,t0,1f
	nop
	dli	t0,MAILBOX_BUF2
	b	2f
1:
	li	t0,0x3
	bne	a0,t0,1f
	nop
	dli	t0,MAILBOX_BUF3
	b	2f
2:
	sd  a1,0(t0)
	sd	a2,8(t0)
	sd	a3,0x10(t0)
	sd	a4,0x18(t0)
1:
	jr	ra
	.end godson3_cpu_start
	
	
/******************************************************************************
 *
 * sysCpuInit - Secondary core startup
 *
 * This routine is the first code executed on a secondary processor after
 * startup. It is responsible for ensuring that the SR, CAUSE,
 * COUNT, and COMPARE registers are set appropriately.
 *
 * In a mapped kernel, it is also responsible for setting up the core's MMU.
 * 
 * Upon completion of these tasks, the core is ready to begin accepting
 * kernel work. The address of the initial kernel code (typically
 * windCpuEntry) has been placed in sysCpuInitTable[coreNum].
 * 
 * N.B.: since the MMU is not initialized when this function is entered,
 * it must be coerced to run and access data in kseg0 until the initial
 * MMU mapping has been completed.
 *
 * void sysCpuInit(int a0, WIND_CPU_STATE *a1)
 *
 * RETURNS: NONE
 * 
 */
	/* starting bit of core number field in C0_PRID */
#define CORENUM_BIT	(25)
#define CORENUM_MASK	(7)
	
	.globl sysCpuInit
	.ent sysCpuInit
sysCpuInit:
	mtc0	zero, C0_CAUSE
	
	/* give as long as possible before a clock interrupt */
	li	v0, 1
	mtc0	v0, C0_COUNT
	mtc0	zero, C0_COMPARE
	
	/* 
	 * need to clear the BEV bit. Exception vectors have
	 * already been set up by the boot processor, so we are
	 * able to use them.
	 */
	mfc0	t0, C0_SR
	HAZARD_CP_READ
	
	and	t0, ~SR_BEV
	mtc0	t0, C0_SR
	HAZARD_CP_WRITE

	/* 
	 * important: this needs to be an intrasegment jal. in a mapped
	 * kernel, we are entered in kseg0, and we need to stay here
	 * until the TLB is set up. The jal instruction stays within the
	 * segment. DO NOT change this to a 'la t0,mipsTlbClear;  jal t0'
	 * because this would take us to mapped space.
	 */
	jal	mipsTlbClear

#if defined (INCLUDE_MAPPED_KERNEL)
	/* coerce sp into kseg0 */
	and	sp, ADDRESS_SPACE_MASK
	or	sp, K0BASE

	/* determine what core we're running on */
	#if 0 /* added for godson3 yinwx, 20091219 */ 
	mfc0	v0, C0_PRID
	srl	v0, CORENUM_BIT - 13	 /* adjust for page index */
	and	v0, (CORENUM_MASK << 13) /* adjust for page index */
	#else
	mfc0	v0, $15, 1
	sync
	andi	v0,	0x3 /* !! changed by yinwx, 20100224 !! */
	sll	v0,	15
	#endif

	li	a0, EXC_PAGE_PHYS_ADRS
	add	a0, v0			/* adjust exception page start addr */
	li	a1, LOCAL_MEM_LOCAL_ADRS
	li	a2, (LOCAL_MEM_LOCAL_ADRS + LOCAL_MEM_SIZE)
	jal	mmuMipsInitialMemoryMapSet
	beqz	v0, 1f
0:	
	/* memory mapping error #1 */
	SETLEDS('m','m','u','1')
	b	0b
1:	
	jal	mmuMipsExcpageInit	/* set up values in -1 page */
	beqz	v0, 3f
2:	
	/* memory mapping error */
	SETLEDS('m','m','u','2')
	b	2b

	/* tlb setup in place, switch to mapped address */
3:	
	la	t0, 0f
	j	t0
0:
#endif /* INCLUDE_MAPPED_KERNEL */
	lw	a0, taskSrDefault
	and	a0, ~SR_IE
	jal	intSRSet	
		
	/* determine what core we're running on */
	#if 0 /* added for godson3 yinwx, 20091219 */
	mfc0	v0, C0_PRID
	srl	v0, CORENUM_BIT - 2	/* adjust for word index */
	and	v0, (CORENUM_MASK << 2)	/* adjust for word index */
	#else
	mfc0	v0, $15, 1
	sync
	andi	v0,	0x3 /* !! changed by yinwx, 20100224 !! */
	sll	v0,	2
	#endif
	
	la	t0, sysCpuInitTable
	add	t0, t0, v0
	lw	t0, 0(t0)		/* get table entry for this core */
	beqz	t0, 1f			/* branch if no entry */
	jal	t0			/* no return */

	/* 
	 * If we return here for some reason, just increment a counter
	 * to tie up the core. This would be visible from other cores.
	 */
	la	t0, sysCpuLoopCount
	add	t0, v0
	li	t1, 1
2:
	lw	t2, 0(t1)
	add	t2, t1
	sw	t2, 0(t1)
	b	2b

	/* dead end. only get here if we can't start kernel */
1:	
	/* "no kernel" */
	SETLEDS('n','o',' ','k')
	b	1b
	.end sysCpuInit
#endif /* _WRS_CONFIG_SMP */ 
/*******************************************************************************
*
* sysTlbReadIdx - Initiate Coprocessor 0 TLB Read Index operation
*
* RETURNS: N/A
*
* void sysTlbReadIdx (void)
*/

	.ent	sysTlbReadIdx
sysTlbReadIdx:
	tlbr
	j	ra
	.end	sysTlbReadIdx

/*******************************************************************************
*
* sysTlbMaskGet - get the R4000 CP0 TLB TlbMask Register
*
* RETURNS: N/A
*
* UINT sysTlbMaskGet (void)
*/

	.ent	sysTlbMaskGet
sysTlbMaskGet:
	mfc0	v0,C0_PAGEMASK
	j	ra
	.end	sysTlbMaskGet
/*******************************************************************************
*
* sysTlbIdxSet - set the R4000 CP1 TLB TlbIdx Register
*
* RETURNS: N/A
*
* void sysTlbIdxSet (UINT regVal)
*/

	.ent	sysTlbIdxSet
sysTlbIdxSet:
	mtc0	a0,C0_INX
	j	ra
	.end	sysTlbIdxSet
/*******************************************************************************
*
* sysTlbLO0Get - get the R4000 CP0 TLB TlbLO0 Register
*
* RETURNS: N/A
*
* UINT sysTlbLO0Get (void)
*/

	.ent	sysTlbLO0Get
sysTlbLO0Get:
	mfc0	v0,C0_TLBLO0
	j	ra
	.end	sysTlbLO0Get
/*******************************************************************************
*
* sysTlbLO1Get - get the R4000 CP0 TLB TlbLO1 Register
*
* RETURNS: N/A
*
* UINT sysTlbLO1Get (void)
*/

	.ent	sysTlbLO1Get
sysTlbLO1Get:
	mfc0	v0,C0_TLBLO1
	j	ra
	.end	sysTlbLO1Get

/*******************************************************************************
*
* sysTlbHIGet - get the R4000 CP0 TLB TlbHI Register
*
* RETURNS: N/A
*
* UINT sysTlbHIGet (void)
*/

	.ent	sysTlbHIGet
sysTlbHIGet:
	mfc0	v0,C0_TLBHI
	j	ra
	.end	sysTlbHIGet
				
	
#include "sysMipsALib.s"

#include "cacheLsn2eALib.s" /* added by yinwx, 20100125 */
