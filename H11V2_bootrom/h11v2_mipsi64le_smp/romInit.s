/* romInit.s - MIPS ROM initialization module */

/*
 * Copyright (c) 2001-2002, 2005-2006 Wind River Systems, Inc.
 *
 * The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.
 */

/*
modification history
--------------------
01h,24Apr12,mxl  modified all code for support h11v2 
01g,25oct06,pes  Conditionalize for CFE support. Use CFEARGS to save args
             passed by CFE. Correct initial stack setup.
01f,14mar06,pes  Use sibyte.h to determine board type.
01e,03aug05,dr   added #include "bcm1250DramInit.s".
01d,03oct02,agf  changes for shared sentosa support
01c,23jan02,agf  Add .set noreorder to ROM vector section
01b,14jan02,pes  Eliminate '#'-style comments
01a,15nov01,agf  Created for the bcm1250 BSP

*/
        
/*
DESCRIPTION
This module contains the entry code for the VxWorks bootrom.
The entry point romInit, is the first code executed on power-up.

The routine sysToMonitor() jumps to romInit() to perform a
"warm boot".

*/

/* includes */
#define _ASMLANGUAGE
#include <vxWorks.h>
#include <sysLib.h>
#include <cacheLib.h>
#include "config.h"
#include <asm.h>
#include <esf.h>
#include <drv/multi/sb1Lib.h>
#include "sibyte.h"
    
/* defines */
#define  CFG_MULTI_CPUS   1
#define  INIT_CPU0
#define  MULTI_CHIP
#define  MEM_CHAN_BOTH


/* macros */
/* in archMips.h these macros are not assembler friendly, so fix for here */

#define ROM_ISP_BASE    0xa0010000
#define CORE0_MAILBOX0  0x900000003ff01020
#define CORE0_MAILBOX1  0x900000003ff01028
#define CORE0_MAILBOX2  0x900000003ff01030
#define CORE0_MAILBOX3  0x900000003ff01038

#define HR_UART0_BASE    0xbfe001e0 
#define HR_UART1_BASE    0xbfe001e8

#define	NS16550_DATA	0
#define	NS16550_IER	1
#define	NS16550_IIR	2
#define	NS16550_FIFO	2
#define	NS16550_CFCR	3
#define	NS16550_MCR	4
#define	NS16550_LSR	5
#define	NS16550_MSR	6	
#define	NS16550_SCR	7
#define	LSR_TXRDY	0x20	/* transmitter ready */
#define	LSR_RXRDY	0x01	/* receiver ready */

#define PRINTSTR(x)   .rdata; 98: .asciz x; .text; la a0, 98b; bal stringserial; nop
#define TTYDBG  PRINTSTR

#undef  PHYS_TO_K0
#define PHYS_TO_K0(x)   (K0BASE | (x))
#undef  PHYS_TO_K1
#define PHYS_TO_K1(x)   (K1BASE | (x))

#define RELOC(toreg,address) \
    bal 9f; \
    nop;   \
    9:; \
    la  toreg,address; \
    addu    toreg,ra; \
    la  ra,9b; \
    subu    toreg,ra

#define RVECENT(f,n)       b f; nop
#define XVECENT(f,bev)     b f; li k0,bev


.globl  romInit         /* start of system code */
.extern romStart        /* system initialization routine */
.extern cacheSb1Reset   /* L1 cache reset */

    .data
    .align  4           /* ensure data segment is 16-byte aligned */

_sdata:
    .asciiz "start of data"
hexchar:
    .ascii  "0123456789abcdef"

    .text
    .set    noreorder
    
promEntry:
romInit:
_romInit:
    RVECENT(__romInit,0)        /* PROM entry point */
    RVECENT(romReboot,1)        /* software reboot */
    RVECENT(romReserved,2)
    RVECENT(romReserved,3)
    RVECENT(romReserved,4)
    RVECENT(romReserved,5)
    RVECENT(romReserved,6)
    RVECENT(romReserved,7)
    RVECENT(romReserved,8)
    RVECENT(romReserved,9)
    RVECENT(romReserved,10)
    RVECENT(romReserved,11)
    RVECENT(romReserved,12)
    RVECENT(romReserved,13)
    RVECENT(romReserved,14)
    RVECENT(romReserved,15)
    RVECENT(romReserved,16)
    RVECENT(romReserved,17) 
    RVECENT(romReserved,18)
    RVECENT(romReserved,19)
    RVECENT(romReserved,20)
    RVECENT(romReserved,21)
    RVECENT(romReserved,22)
    RVECENT(romReserved,23)
    RVECENT(romReserved,24)
    RVECENT(romReserved,25)
    RVECENT(romReserved,26)
    RVECENT(romReserved,27)
    RVECENT(romReserved,28)
    RVECENT(romReserved,29)
    RVECENT(romReserved,30)
    RVECENT(romReserved,31)
    RVECENT(romReserved,32)
    RVECENT(romReserved,33)
    RVECENT(romReserved,34)
    RVECENT(romReserved,35)
    RVECENT(romReserved,36)
    RVECENT(romReserved,37)
    RVECENT(romReserved,38)
    RVECENT(romReserved,39)
    RVECENT(romReserved,40)
    RVECENT(romReserved,41)
    RVECENT(romReserved,42)
    RVECENT(romReserved,43)
    RVECENT(romReserved,44)
    RVECENT(romReserved,45)
    RVECENT(romReserved,46)
    RVECENT(romReserved,47)
    RVECENT(romReserved,48)
    RVECENT(romReserved,49)
    RVECENT(romReserved,50)
    RVECENT(romReserved,51)
    RVECENT(romReserved,52)
    RVECENT(romReserved,53)
    RVECENT(romReserved,54)
    RVECENT(romReserved,55)
    RVECENT(romReserved,56)
    RVECENT(romReserved,57)
    RVECENT(romReserved,58)
    RVECENT(romReserved,59)
    RVECENT(romReserved,60)
    RVECENT(romReserved,61)
    RVECENT(romReserved,62)
    RVECENT(romReserved,63) 
    XVECENT(romExcHandle,0x200) /* bfc00200: R4000 tlbmiss vector */
    RVECENT(romReserved,65)
    RVECENT(romReserved,66)
    RVECENT(romReserved,67)
    RVECENT(romReserved,68)
    RVECENT(romReserved,69)
    RVECENT(romReserved,70)
    RVECENT(romReserved,71)
    RVECENT(romReserved,72)
    RVECENT(romReserved,73)
    RVECENT(romReserved,74)
    RVECENT(romReserved,75)
    RVECENT(romReserved,76)
    RVECENT(romReserved,77)
    RVECENT(romReserved,78)
    RVECENT(romReserved,79) 
    XVECENT(romExcHandle,0x280) /* bfc00280: R4000 xtlbmiss vector */
    RVECENT(romReserved,81)
    RVECENT(romReserved,82)
    RVECENT(romReserved,83)
    RVECENT(romReserved,84)
    RVECENT(romReserved,85)
    RVECENT(romReserved,86)
    RVECENT(romReserved,87)
    RVECENT(romReserved,88)
    RVECENT(romReserved,89)
    RVECENT(romReserved,90)
    RVECENT(romReserved,91)
    RVECENT(romReserved,92)
    RVECENT(romReserved,93)
    RVECENT(romReserved,94)
    RVECENT(romReserved,95) 
    XVECENT(romExcHandle,0x300) /* bfc00300: R4000 cache vector */
    RVECENT(romReserved,97)
    RVECENT(romReserved,98)
    RVECENT(romReserved,99)
    RVECENT(romReserved,100)
    RVECENT(romReserved,101)
    RVECENT(romReserved,102)
    RVECENT(romReserved,103)
    RVECENT(romReserved,104)
    RVECENT(romReserved,105)
    RVECENT(romReserved,106)
    RVECENT(romReserved,107)
    RVECENT(romReserved,108)
    RVECENT(romReserved,109)
    RVECENT(romReserved,110)
    RVECENT(romReserved,111)
    XVECENT(romExcHandle,0x380) /* bfc00380: R4000 general vector */
    RVECENT(romReserved,113)
    RVECENT(romReserved,114)
    RVECENT(romReserved,115)
    RVECENT(romReserved,116)
    RVECENT(romReserved,116)
    RVECENT(romReserved,118)
    RVECENT(romReserved,119)
    RVECENT(romReserved,120)
    RVECENT(romReserved,121)
    RVECENT(romReserved,122)
    RVECENT(romReserved,123)
    RVECENT(romReserved,124)
    RVECENT(romReserved,125)
    RVECENT(romReserved,126)
    RVECENT(romReserved,127)

    /* We hope there are no more reserved vectors!
     * 128 * 8 == 1024 == 0x400
     * so this is address R_VEC+0x400 == 0xbfc00400
     */
    
    .align 4
    .set    reorder

/******************************************************************************
*
* romInit - entry point for VxWorks in ROM
*
* romInit 
*     (
*     int startType
*     )
******************************************************************************/

    .set noreorder    
__romInit:
    
    li  a0, BOOT_CLEAR   /* force power-on startType */

romReboot:              /* sw reboot inherits a0 startType */

    move  s6, a0         /* move startType to s6 to keep it 'safe' */

    mtc0    zero, C0_SR
    mtc0    zero, C0_CAUSE

    li      t0, 0x00400000        /* Exception to Boostrap Location */
    mtc0    t0, C0_SR
    mtc0    zero, C0_CAUSE

    mfc0    t0, C0_SR            /* Open 64-bit address space  */
    HAZARD_CP_READ

    li      t1, 0x008000e0            /* Enable PX KX SX UX */
    or      t0, t0, t1         
    mtc0    zero, C0_CAUSE           
    mtc0    t0, C0_SR                 

    mfc0    t0, $15, 1  
    HAZARD_CP_READ
    andi     t0, 0x3ff   

    /* clear Mail BOX  */     
    dli     t1, CORE0_MAILBOX0   

    andi    t3, t0, 0x3               
    dsll    t3, 8
    or      t1, t1, t3

    andi    t4, t0, 0xc                
    dsll32  t4, t4, 10
    or      t1, t1, t4

    bne   t0, zero, 3f                
    nop

    dli     a0, 0x0
    sd      a0, 0x0(t1)               
    
    dli     t1, CORE0_MAILBOX1       

    andi    a0, t0, 0x3
    dsll    a0, a0, 0x8
    or      t1, t1, a0

    andi    a0, t0, 0xc  
    dsll32  a0, a0, 10  
    or      t1, t1, a0

    dli     a0, 0x0
    sd      a0, 0x0(t1)   
    nop

3:
    beq   t0, zero, 1f        /* mxl:  if not core0,  should wait for mailbox  */
    nop
     
    RELOC(k1, alt_romInit)          
    jal   k1
    nop

1:
    #if 0
    /* don't know why need this  */
    li      a0, 0xbff00080
    li      t0, 0xcc
    sb      t0, 0x0(a0)
    nop
    nop
    nop
    #endif
#if 0
    /* HT initialization. */
    bal InitHtBus
    nop
#endif    
    /* mxl : serial init need move to here: characters are not displayed properly */
    /* Init the CPU0 serial 0 */
    bal InitNs16550Serial0
    nop

    #if 0
    bal InitNs16550Serial1   /* backplane serial */
    nop
    #endif
    
    PRINTSTR("\r\n")
    PRINTSTR("UART Init ...done\r\n")  

#if 1  /* move HT init at first place */
    /* HT initialization. */
    bal InitHtBus
    nop
    PRINTSTR("\r\n")
    PRINTSTR("HT bus init ...done\r\n")
#endif

    bal sysclkInfoPrint
    nop
    

    PRINTSTR("\r\n")
    PRINTSTR("start Xbar config:\r\n")
    bal XbarInit
    nop     
    PRINTSTR("\r\n")
    PRINTSTR("Xbar init ...done\r\n")
    
    PRINTSTR("\r\n")
    PRINTSTR("start DDR config:\r\n")
    bal DDRInit
    nop      
    PRINTSTR("DDR init ...done\r\n")

    /*   Initialize cpu0's core     */

    bal Tlb_L1Cache_init
    nop 
    PRINTSTR("\r\n")
    PRINTSTR("Init Tlb & L1 cache ...done\r\n")

    /*
    * Do low-level processor initialization. Again, only
    * needed once, so have cpu0 do it.
    */
    bal L2Cache_init
    nop
    PRINTSTR("\r\n")
    PRINTSTR("Init L2Cache  ...done\r\n")

	PRINTSTR("\r\n")
    PRINTSTR("Release other core ...\r\n")
	
    bal start_altCpu
    nop

    /* delay for  other core startup */   
    li      a0, 0x10000
    
WaitCoreUp:    
    addiu   a0, -1
    bnez    a0, WaitCoreUp
    nop
    

    dli     t0, CORE0_MAILBOX1 //buf of cpu0
    li      t1, 0x55aa
    sw      t1, 0(t0)
    nop
    
	PRINTSTR("\r\n")

    /* enable cache */
    mfc0    v1,C0_CONFIG            /* get current CONFIG register */
    srl     v1,v1,3                 /* strip out K0 bits */
    sll     v1,v1,3                 /* k0 bits now zero */
    or      v1,v1,CFG_K0_CACHEABLE  /* K0 is cached, non-coherent */
    mtc0    v1,C0_CONFIG

    li      v0,-1
    mtc0    v0,C0_COMPARE

    /* Switch from KSEG1 to KSEG0 */    
    bal cpu_kseg0_switch
    nop
    PRINTSTR("\r\n")
    PRINTSTR("Switch from kseg1 to kseg0  ...done\r\n")


	
	/* mxl : DDR test */
#define DDR_TEST
#ifdef  DDR_TEST
		PRINTSTR("\r\nDo you need to test DDR? ('y' or other) ")
		bal tgt_getchar
		nop
		li   v1,'y'
		bne v0,v1,3f
		nop
	
		PRINTSTR("\r\n**************start DDR test*************** \r\n")
		
		dli 	s1, 0x0008000040000000	//NODE 0, start from 0x80000000
	
		PRINTSTR("\r\ndefault s1 = 0x");
		dsrl	a0, s1, 32
		bal 	hexserial
		nop
		PRINTSTR("__")
		move	a0, s1
		bal 	hexserial
		nop
	
	1:
		dli 	t1, 0x0010
		bal 	test_mem
		nop
		move	t1, v0
		PRINTSTR("\r\n")
		dsrl	a0, t1, 32
		bal 	hexserial
		nop
		move	a0, t1
		bal 	hexserial
		nop
		beqz	t1, 3f
		nop
		PRINTSTR("	Error found!!\r\n")
	2:
#if 1
		b		2b
		nop
#endif
	
	3:
		PRINTSTR("\r\n************** DDR test end*************** \r\n")
#endif
	

    
    /*
    * Finally, do the typical romInit stuff and go to romStart
    */
    li      t0, 1
    mtc0    t0, C0_COUNT
    mtc0    zero, C0_COMPARE


    la      gp, _gp
    la      sp, STACK_ADRS-(4*_RTypeSize)   /* set stack   */

    move    a0, s6                  /* restore startType */
    sync
    
    PRINTSTR("\r\nromStart ... ")
    
    RELOC(k1,romStart)

	move    a0, k1
    bal     hexserial
    nop
    
    #if 0
    li  a0, BOOT_CLEAR                  /* moxiaolu : test  */
/* added by moxiaolu for test */
    bne    a0, 2, 4f
    nop
    PRINTSTR("\r\na0 == 2 ")
    b 5f
    nop
    
    4:
    bne    a0, 0, 5f
    nop

    PRINTSTR("\r\na0 == 0 ")
    
    5:
    #endif
    
    jal     k1                      /* should not return */
    li  a0, BOOT_CLEAR          /* moxiaolu for 延时槽加载参数，解决参数传递错误bug */
    nop


1:      
    PRINTSTR("\r\nError!, loop here !\r\n")
2:
    b   2b                              /* should never get here */
    nop
        
/***************************************************************************
*
* alt_romInit - initialization sequence for the secondary CPU's
*
*  Do the initialization of the local core and then notify cpu0 that 
*  we're done.
* 
*  This routine is executes in KSEG1.
*
*  Input parameters:
*      t0 - CPU identifier
*
*  Return value:
*      nothing
*/

    .ent    alt_romInit
alt_romInit:

        /*
         * cpu0 just entered start_altCpu and released us from RESET. It will
         * wait there until we ring its doorbell.
         *
         * Initialize core
         */

        mfc0	t0, C0_SR
        HAZARD_CP_READ
        
        li      t1, 0x008000e0      /* Enable PX KX SX UX */
        or      t0, t0, t1
        mtc0    t0, C0_SR
    	

    	li      t1, 0x40        /* BEV in Status REG */
    	or      t0, t0, t1
    	mtc0    t0, C0_SR
    	mtc0    zero, C0_CAUSE

 #if 0   	
    /* set own GPIO window */      
        andi    a0, t0, 0x3               /* 计算Core偏移 */ 
        bnez    a0,   1f
        nop     

        dli     t1, 0x900000003ff00018
        andi    a0, t0, 0xc               /* 计算cpu偏移 */
        dsll32  a0, a0, 10                  
        or      t1, t1, a0
        
        dli     t2, 0x0000000010000000
        or      t2, t2, a0
        
        dli     t3, 0xfffffffff0000000
        dli     a1, 0x10000082
        
        sd  zero, 0x80(t1)
        sd  t2, 0x0(t1)
        sd  t3, 0x40(t1)
        sd  a1, 0x80(t1)
        
        dli    t2,  0x900000001fe00120
        or     t2, t2, a0
        li     t3,   0x0
        sw     t3,   0x0(t2)
        sync
        dli    t2,   0x900000001fe0011c
        or     t2, t2, a0
    /* trun on own led and clear watch dog flag(auto reset)*/    
        li   t3,   0x14
        sw   t3,   0x0(t2)
        sync
 #endif      

        
        dli     t0, CORE0_MAILBOX0  //buf of cpu0
myidle:

        li      a0, 0x1000   //0x1000000
    delay1000:    
        addiu   a0, -1
        bnez    a0, delay1000
        nop
        
        lw      a0, 0(t0)
        beqz    a0, myidle
        nop
        
        li      a0, 0x1001   //0x1000000
    delay1001:    
        addiu   a0, -1
        bnez    a0, delay1001
        nop

        
        RELOC(k1, Tlb_L1Cache_init)
        jal   k1        
        nop
        
        mfc0    t2, C0_PRID, 1            /* get cpu PRID */
        HAZARD_CP_READ
        and     t2, t2, 0x3ff             /* determine cpu number */
        
        andi    t3, t2, 0x3  //local cpuid
        bnez    t3,   1f
        nop
        
        dli a0, 0x9800000000000000
        andi    t4, t2, 0xc //node id
        dsll32  t4, t4, 10
        or      a0, a0, t4
        RELOC(k1, CPU_L2cache_init_64)
        jal   k1
        nop
        

        mfc0    t2, C0_PRID, 1            /* get cpu PRID */
        HAZARD_CP_READ
        and     t2, t2, 0x3ff             /* determine cpu number */
        
        dli     t0, CORE0_MAILBOX1 //buf of cpu0
        andi    t4, t2, 0xc  //node id
        dsll32  a0, a0, 10
        or      t0, t0, t4
        li      t1, 0x55aa
        sw      t1, 0(t0)
        nop 
  
        b   2f
        nop
1:
        dli     t0, CORE0_MAILBOX1 //buf of cpu0
        andi    t4, t2, 0xc  //node id
        dsll32  t4, t4, 10
        or      t0, t0, t4
lockcacheidle:
        lw      a0, 0(t0)
        beqz    a0, lockcacheidle
        nop
2:
        /*
         * Notify the SCD that we're done initializing.  Do this by
         * ringing cpu0's doorbell.
         */
        /*
         * enable cache
         */
            mfc0    v1,C0_CONFIG            /* get current CONFIG register */
            srl     v1,v1,3                 /* strip out K0 bits */
            sll     v1,v1,3                 /* k0 bits now zero */
            or      v1,v1,CFG_K0_CACHEABLE  /* K0 is cached, non-coherent */
            mtc0    v1,C0_CONFIG

        /*
         * This is probably not the right init value for C0_COMPARE,
         * but it seems to be necessary for the sim model right now.
         */
            li      v0,-1
            mtc0    v0,C0_COMPARE
            

        /*
         * Switch from KSEG1 to KSEG0
         */

        bal     cpu_kseg0_switch
        nop
	
        mfc0    t0, C0_PRID, 1            /* get cpu PRID */
        HAZARD_CP_READ
        and     t0, t0, 0x3ff             /* determine cpu number */
        
        bne     t0, 1, 2f
        nop
    PRINTSTR("1")
        b       16f
        nop
2:
        nop
        bne     t0, 2, 3f
        nop
        nop
        nop
    PRINTSTR("2")
        b       16f
        nop
3:
        bne     t0, 3, 4f
        nop
        nop
        nop
        nop
        nop
        nop
    PRINTSTR("3")  
        b       16f
        nop  
4:
        bne     t0, 4, 5f
        nop
    PRINTSTR("4......")  
        b       16f
        nop    
5:
        bne     t0, 5, 6f
        nop
    PRINTSTR("5......")  
        b       16f
        nop    
6:
        bne     t0, 6, 7f
        nop
    PRINTSTR("6......")  
        b       16f
        nop         
7:
        bne     t0, 7, 8f
        nop
    PRINTSTR("7")  
        b       16f
        nop  
8:
        bne     t0, 8, 9f
        nop
    PRINTSTR("8")  
        b       16f
        nop    
9:
        bne     t0, 9, 10f
        nop
    PRINTSTR("9")  
        b       16f
        nop  
10:
        bne     t0, 10, 11f
        nop
    PRINTSTR("A")  
        b       16f
        nop 
11:
        bne     t0, 11, 12f
        nop
    PRINTSTR("B")  
        b       16f
        nop 
12:
        bne     t0, 12, 13f
        nop
    PRINTSTR("C")  
        b       16f
        nop          
13:
        bne     t0, 13, 14f
        nop
    PRINTSTR("D")  
        b       16f
        nop  
14:
        bne     t0, 14, 15f
        nop
    PRINTSTR("E")  
        b       16f
        nop  
15:
        nop
        nop
        nop
        nop
        bne     t0, 15, 16f
        nop
    PRINTSTR("F looping...\n")  
        b       16f
        nop                                                           
16:      
        
    
        dli     t1, CORE0_MAILBOX0        /*t1- CORE0_BUF0*/

        andi    a0, t0, 0x3               /* 计算Core偏移 */ 
        dsll    a0, a0, 0x8
        or      t1, t1, a0     

        andi    a0, t0, 0xc               /* 计算cpu偏移 */
        dsll32  a0, a0, 10                  
        or      t1, t1, a0        
        

1:

        /*
         * Wait on doorbell
         */
waitMail:
        li      a0, 0x10000   //0x1000000
idle1000:    
        addiu   a0, -1
        bnez    a0, idle1000
        nop    
        
        
        lw      v0, 0x0(t1)             //read MailBOX0 used for "PC" to jump
        beqz    v0, waitMail
        nop    

        andi    a0, t0, 0x3               /* 计算Core偏移 */
        bne     a0, 0, 1f
     
      1:
      
        dli     t0, 0xffffffff00000000 
        or      v0, t0
        
        
        dli     t0, 0x9800000000000000 
        ld      sp, 0x08(t1)            // read MailBOX1 for sp
        or      sp, t0
        ld      gp, 0x10(t1)            // read MailBOX2 for gp
        or      gp, t0
        ld      a1, 0x18(t1)            // read MailBOX3 for a1


        jalr    v0                      // byebye 
        nop
              
                
1:
        b       1b
        nop


    .end    alt_romInit



/***************************************************************************
*
* start_altCpu - initialization sequence for the secondary CPU's
*
* This routine is executed by the primary core. It releases the secondary(s)
* from RESET then waits for them to finish their init sequence to the 
* point of L1 cache initalization. Once the secondary(s) are finished, they
* will ring the primary's door bell so it can return.
* 
*  Input parameters:
*      nothing
*
*  Return value:
*      nothing
*/

    .ent    start_altCpu
start_altCpu:
    /* snd mail to running other core */
        dli     t0, CORE0_MAILBOX0  //buf of cpu0
        li      t1, 0x5a5a
        sw      t1, 0(t0)
        sync
        nop
        
        j       ra
        nop

    .end    start_altCpu
    
/***************************************************************************
*
* cpu_kseg0_switch - manipulate the return address so the program counter 
*                  returns to the KSEG0 region
*
*  Input parameters:
*      nothing
*
*  Return value:
*      nothing
*/

    .ent    cpu_kseg0_switch
cpu_kseg0_switch:

	#if 0 // mxl  :  print config windows 
//dump L1-L2-HT config windows
    move    s1, ra 
    PRINTSTR("\r\n======X1 core0 map windows:\r\n")
    li      t1, 23
    dli     t2, 0x900000003ff02000
1:
    move    a0, t2
    bal    hexserial64
    nop
    PRINTSTR(": ")

    ld      a0, 0x0(t2)
    bal    hexserial64
    nop

    PRINTSTR("\r\n")

    daddiu  t2, t2, 8
    bnez    t1, 1b
    addiu   t1, t1, -1


    PRINTSTR("\r\n======X2 cpu map windows:\r\n")
    li      t1, 23
    dli     t2, 0x900000003ff00000
1:
    move    a0, t2
    bal    hexserial64
    nop
    PRINTSTR(": ")

    ld      a0, 0x0(t2)
    bal    hexserial64
    nop
    PRINTSTR("\r\n")

    daddiu  t2, t2, 8
    bnez    t1, 1b
    addiu   t1, t1, -1

    PRINTSTR("\r\n======X2 pci map windows:\r\n")
    li      t1, 23
    dli     t2, 0x900000003ff00100
1:
    move    a0, t2
    bal    hexserial64
    nop
    PRINTSTR(": ")

    ld      a0, 0x0(t2)
    bal    hexserial64
    nop
    PRINTSTR("\r\n")

    daddiu  t2, t2, 8
    bnez    t1, 1b
    addiu   t1, t1, -1

    
    move    ra, s1 
#endif

        and     ra,(K0SIZE-1)
        or      ra,K0BASE
        j       ra
        nop

    .end    cpu_kseg0_switch

/***************************************************************************
*
* romReserved -  Handle a jump to an unknown vector
*
*
* 
*/

    .ent    romReserved
romReserved:
    b   romInit     /* just start over */
    nop
    .end    romReserved


/******************************************************************************
*
* romExcHandle - rom based exception/interrupt handler
*
* This routine is invoked on an exception or interrupt while
* the status register is using the bootstrap exception vectors.
* It saves a state frame to a known uncached location so someone
* can examine the data over the VME.
*
* THIS ROUTIINE IS NOT CALLABLE FROM "C"
*
*/

        .ent    romExcHandle
romExcHandle:

        .set    noat
        move    k1, sp                  /* save fault sp */
        li      sp, ROM_ISP_BASE        /* sp to known uncached location */
        SW      sp, E_STK_SP-ESTKSIZE(sp) /* save sp in new intstk frame */
        subu    sp, ESTKSIZE            /* make new exc stk frame       */
        SW      k0, E_STK_K0(sp)        /* save k0, (exception type)    */
        SW      k1, E_STK_SP(sp)        /* save SP at fault */
        SW      AT, E_STK_AT(sp)        /* save asmbler resvd reg       */
        .set    at
        SW      v0, E_STK_V0(sp)        /* save func return 0, used
                                           to hold masked cause         */
        mfc0    k1, C0_BADVADDR         /* read bad VA reg      */
        sw      k1, E_STK_BADVADDR(sp)  /* save bad VA on stack */
        mfc0    k1, C0_EPC              /* read exception pc    */
        sw      k1, E_STK_EPC(sp)       /* save EPC on stack    */
        mfc0    v0, C0_CAUSE            /* read cause register  */
        sw      v0, E_STK_CAUSE(sp)     /* save cause on stack  */
        mfc0    k1, C0_SR               /* read status register */
        sw      k1, E_STK_SR(sp)        /* save status on stack */

        .set    noat
        mflo    AT                      /* read entry lo reg    */
        SW      AT,E_STK_LO(sp)         /* save entry lo reg    */
        mfhi    AT                      /* read entry hi reg    */
        SW      AT,E_STK_HI(sp)         /* save entry hi reg    */
        .set    at
        SW      zero, E_STK_ZERO(sp)    /* save zero ?!         */
        SW      v1,E_STK_V1(sp)         /* save func return 1   */
        SW      a0,E_STK_A0(sp)         /* save passed param 0  */
        SW      a1,E_STK_A1(sp)         /* save passed param 1  */
        SW      a2,E_STK_A2(sp)         /* save passed param 2  */
        SW      a3,E_STK_A3(sp)         /* save passed param 3  */
        SW      t0,E_STK_T0(sp)         /* save temp reg 0      */
        SW      t1,E_STK_T1(sp)         /* save temp reg 1      */
        SW      t2,E_STK_T2(sp)         /* save temp reg 2      */
        SW      t3,E_STK_T3(sp)         /* save temp reg 3      */
        SW      t4,E_STK_T4(sp)         /* save temp reg 4      */
        SW      t5,E_STK_T5(sp)         /* save temp reg 5      */
        SW      t6,E_STK_T6(sp)         /* save temp reg 6      */
        SW      t7,E_STK_T7(sp)         /* save temp reg 7      */
        SW      t8,E_STK_T8(sp)         /* save temp reg 8      */
        SW      t9,E_STK_T9(sp)         /* save temp reg 9      */
        SW      s0,E_STK_S0(sp)         /* save saved reg 0     */
        SW      s1,E_STK_S1(sp)         /* save saved reg 1     */
        SW      s2,E_STK_S2(sp)         /* save saved reg 2     */
        SW      s3,E_STK_S3(sp)         /* save saved reg 3     */
        SW      s4,E_STK_S4(sp)         /* save saved reg 4     */
        SW      s5,E_STK_S5(sp)         /* save saved reg 5     */
        SW      s6,E_STK_S6(sp)         /* save saved reg 6     */
        SW      s7,E_STK_S7(sp)         /* save saved reg 7     */
        SW      s8,E_STK_FP(sp)         /* save saved reg 8     */
        SW      gp,E_STK_GP(sp)         /* save global pointer? */
        SW      ra,E_STK_RA(sp)         /* save return address  */
blink:

        b       blink

        .end    romExcHandle            /* that's all folks */




.globl  InitHtBus
.ent    InitHtBus
InitHtBus:
    move    s1, ra      

    PRINTSTR("\r\n")

    dli t0, 0x90000cfdfb000000

    lw  a1, 0x48(t0)   

    #if 1
    PRINTSTR("   HT0 frequency * width config : 200 X 16 \r\n")
    li  a2, 0xfffff0ff     /* set 200 Mhz HT HOST */
    and a1, a1, a2
    #else
    PRINTSTR("HT0 frequency reconfig  400 X 16 ")
    li  a2, 0x200    /* 400Mhz */
    or  a1, a1, a2  
    #endif
    
    sw  a1, 0x48(t0)
    sync

    lw  a1, 0x44(t0)

    #if 0
    li  a2, 0x88ffffff       /* 8bit mode */
    and a1, a1, a2     
    #else
    li  a2, 0x11000000       /* 16bit mode */ 
    or  a1, a1, a2
    #endif
    
    sw  a1, 0x44(t0)
    sync  

    /* Set Mem space post write */
	//TTYDBG("Set HT0 Memory space write post\r\n")
	dli	    t2, 0x90000cfdfb000000
	li	    t0, 0x80000000
	sw	    t0, 0xd0(t2)
	sync
	li	    t0, 0x0000fff0
	sw	    t0, 0xd4(t2)
	sync

    dli a0,0x90000cfdfb000000
    lw a1,0x3c(a0)
    li a2,0x400000     /* 0->1 reset */
    or a1,a1,a2
    sw a1,0x3c(a0)
    sync  
    
    li a2,0xffbfffff
    and a1,a1,a2       /* 1->0 reset over */  
    sw a1,0x3c(a0)
    sync  

#if 1 /* mzw debug  */
    PRINTSTR("   Waiting HyperTransport bus HT0 to be up.")
    dli     t0, 0x90000cfdfb000000
    li      t1, 0x1f
1:
    lw      a0, 0x44(t0)
    nop
    beqz    t1,2f
    nop
    PRINTSTR(">")
    addi    t1, t1, -1
    b       3f
    nop
2:
    PRINTSTR("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b=")
    li      t1, 0x1f

3:
    lw      a0, 0x44(t0)
    li      a1, 0x20
    and     a0, a0, a1

    beqz    a0, 1b
    nop

	#if 1
    PRINTSTR("\r\n")
    PRINTSTR("The value of the HT Local register 0x44 is : ");  
    lw      a0, 0x44(t0)
    bal     hexserial
    nop    

    PRINTSTR("\r\n")
        
    dli     t1, 0x90000cfdfe000000
    PRINTSTR("The value of the HT bus register 0x44 is : "); 
    lw      a0, 0x44(t1)
    bal     hexserial
    nop
    
    lw      a0, 0x44(t1)
    nop
	#else
	/* read HT bus twice , make sure HT init succeed */
	dli     t1, 0x90000cfdfe000000
    lw      a0, 0x44(t1)
    nop
    
    lw      a0, 0x44(t1)
    nop
    
    #endif

#if 1
    dli t0, 0x90000efdfb000000

    lw  a1, 0x48(t0)   
	PRINTSTR("\r\n")
    #if 1
    PRINTSTR("   HT1 frequency * width config : 200 X 8 \r\n")
    li  a2, 0xfffff0ff     /* set 200 Mhz HT HOST */
    and a1, a1, a2
    #else
    PRINTSTR("HT1 frequency * width config  400 X 16 ")
    li  a2, 0x200    /* 400Mhz */
    or  a1, a1, a2  
    #endif
    
    sw  a1, 0x48(t0)
    sync

    lw  a1, 0x44(t0)

    #if 1
    li  a2, 0x88ffffff       /* 8bit mode */
    and a1, a1, a2     
    #else
    li  a2, 0x11000000       /* 16bit mode */ 
    or  a1, a1, a2
    #endif
    
    sw  a1, 0x44(t0)
    sync  

    /* Set Mem space post write */
	//TTYDBG("Set HT0 Memory space write post\r\n")
	dli	    t2, 0x90000efdfb000000
	li	    t0, 0x80000000
	sw	    t0, 0xd0(t2)
	sync
	li	    t0, 0x0000fff0
	sw	    t0, 0xd4(t2)
	sync

    dli a0,0x90000efdfb000000
    lw a1,0x3c(a0)
    li a2,0x400000     /* 0->1 reset */
    or a1,a1,a2
    sw a1,0x3c(a0)
    sync  
    
    li a2,0xffbfffff
    and a1,a1,a2       /* 1->0 reset over */  
    sw a1,0x3c(a0)
    sync  

    PRINTSTR("   Waiting HyperTransport bus HT1 to be up.")
    dli     t0, 0x90000efdfb000000
    li      t1, 0x1f
1:
    lw      a0, 0x44(t0)
    nop
    beqz    t1,2f
    nop
    PRINTSTR(">")
    addi    t1, t1, -1
    b       3f
    nop
2:
    PRINTSTR("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b=")
    li      t1, 0x1f

3:
    lw      a0, 0x44(t0)
    li      a1, 0x20
    and     a0, a0, a1

    beqz    a0, 1b
    nop

	#if 1
    PRINTSTR("\r\n")
    PRINTSTR("The value of the HT1 Local register 0x44 is : ");  
    lw      a0, 0x44(t0)
    bal     hexserial
    nop    

    PRINTSTR("\r\n")
        
    dli     t1, 0x90000efdfe000000
    PRINTSTR("The value of the HT1 bus register 0x44 is : "); 
    lw      a0, 0x44(t1)
    bal     hexserial
    nop

	lw      a0, 0x44(t1)
	nop
	#else
	#if 1
	/* read HT bus twice , make sure HT init succeed */
	dli     t1, 0x90000efdfe000000
    lw      a0, 0x44(t1)
    nop
    
    lw      a0, 0x44(t1)
    nop
    #endif
    #endif
#endif

#endif  /* mzw debug end */

    /* map all core : 0x90000c00_00000000 --> 0x1e000000 */
#define   HT_32bit_TRANS
#ifdef HT_32bit_TRANS
    dli t0, 0x900000003ff02000
    dli t1, 0x000000001e000000
    sd  t1, 0x20(t0)
    dli t1, 0xffffffffff000000
    sd  t1, 0x60(t0)
    dli t1, 0x00000c00000000f6
    sd  t1, 0xa0(t0)

    dli t0, 0x900000003ff02100
    dli t1, 0x000000001e000000
    sd  t1, 0x20(t0)
    dli t1, 0xffffffffff000000
    sd  t1, 0x60(t0)
    dli t1, 0x00000c00000000f6
    sd  t1, 0xa0(t0)

    dli t0, 0x900000003ff02200
    dli t1, 0x000000001e000000
    sd  t1, 0x20(t0)
    dli t1, 0xffffffffff000000
    sd  t1, 0x60(t0)
    dli t1, 0x00000c00000000f6
    sd  t1, 0xa0(t0)

    dli t0, 0x900000003ff02300
    dli t1, 0x000000001e000000
    sd  t1, 0x20(t0)
    dli t1, 0xffffffffff000000
    sd  t1, 0x60(t0)
    dli t1, 0x00000c00000000f6
    sd  t1, 0xa0(t0)



    dli t0, 0x900000003ff02400
    dli t1, 0x000000001f000000
    sd  t1, 0x20(t0)
    dli t1, 0xffffffffff000000
    sd  t1, 0x60(t0)
    dli t1, 0x00000e00000000f7
    sd  t1, 0xa0(t0)

    dli t0, 0x900000003ff02500
    dli t1, 0x000000001f000000
    sd  t1, 0x20(t0)
    dli t1, 0xffffffffff000000
    sd  t1, 0x60(t0)
    dli t1, 0x00000e00000000f7
    sd  t1, 0xa0(t0)

    dli t0, 0x900000003ff02600
    dli t1, 0x000000001f000000
    sd  t1, 0x20(t0)
    dli t1, 0xffffffffff000000
    sd  t1, 0x60(t0)
    dli t1, 0x00000e00000000f7
    sd  t1, 0xa0(t0)

    dli t0, 0x900000003ff02700
    dli t1, 0x000000001f000000
    sd  t1, 0x20(t0)
    dli t1, 0xffffffffff000000
    sd  t1, 0x60(t0)
    dli t1, 0x00000e00000000f7
    sd  t1, 0xa0(t0)
#endif


#if 0   /* just for mzw test:  read ht0 ht1 100 times */
	dli  t8 , 0x100
	dli  t0, 0x90000cfdfe000000
	dli  t1, 0x90000efdfe000000

22:

	addu    t8, -1
	PRINTSTR("\r\n")
	PRINTSTR("Current loop counter  : 0x");  
	dli  a0 , 0x0
    move      a0, t8
    bal     hexserial
    nop
	
	PRINTSTR("\r\n")
    PRINTSTR("    The value of the HT0 bus register 0x44 is : ");  
    lw      a0, 0x44(t0)
    bal     hexserial
    nop    

    PRINTSTR("\r\n")
    PRINTSTR("        The value of the HT1 bus register 0x44 is : ");  
    lw      a0, 0x44(t1)
    bal     hexserial
    nop 

	bnez t8, 22b

	
#endif

    move    ra, s1            
    j       ra      
    nop
    .end    InitHtBus


/*
 * Print sysClk configure info
 */

    .globl  sysclkInfoPrint
    .ent    sysclkInfoPrint

sysclkInfoPrint:

    move    s1, ra   

    PRINTSTR ("\r\n0xbfe00190  : ")
    li      t2, 0xbfe00190
    ld      t1, 0x0(t2)
    dsrl    a0, t1, 32
    bal     hexserial
    nop
    move    a0, t1
    bal     hexserial
    nop
    PRINTSTR ("\r\nCPU CLK SEL : ")
    dsrl    t1, t1, 32
    andi    a0, t1, 0x1f
    bal     hexserial
    nop


    PRINTSTR ("\r\nCPU clk frequency = SYSCLK x 0x")
    andi    t0, t1, 0x1f
    li      a0, 0x1f
    bne     t0, a0, 1f
    nop
    PRINTSTR ("1\r\n")
    b   2f
    nop
1:
    andi    t0, t1, 0x1f
    andi    a0, t0, 0xf
    addi    a0, a0, 0x1e
    bal     hexserial
    nop
    PRINTSTR (" / ")
    srl     a0, t0, 4
    beqz    a0, 3f
    nop
    PRINTSTR (" 2\r\n")
    b       2f
3:        
    nop
    PRINTSTR (" 1\r\n")
2:      
    PRINTSTR ("\r\nMEM CLK SEL : ")
    dsrl    t0, t1, 5
    andi    a0, t0, 0x1f
    bal     hexserial
    nop

    PRINTSTR ("\r\nDDR clk frequency = MEMCLK x 0x")
    dsrl    t0, t1, 5
    andi    t0, t0, 0x1f
    li      a0, 0x1f
    bne     t0, a0, 1f
    nop
    PRINTSTR ("1\r\n")
    b   2f
    nop
1:
    dsrl    t0, t1, 5
    andi    t0, t0, 0x1f
    andi    a0, t0, 0xf
    addi    a0, a0, 0x1e
    bal     hexserial
    nop
    PRINTSTR (" / ")
    srl     a0, t0, 4
    beqz    a0, 3f
    nop
    PRINTSTR (" 4\r\n")
    b       2f
    nop
3:
    PRINTSTR (" 3\r\n")
2:      

    move    ra, s1           /* restore return address */
    j       ra      
    nop
    .end    sysclkInfoPrint
            


/* serial init */
    
    .set noreorder    

/* serial 0  */    
    .text
    .global InitNs16550Serial0
    .ent    InitNs16550Serial0

InitNs16550Serial0:
    
    li  a0, HR_UART0_BASE   

 	li	t1,128              
 	sb	t1,3(a0)

 	li	t1,0x12      	    
 	sb	t1,0(a0)

 	li	t1,0x0     	        
 	sb	t1,1(a0)

 	li	t1,3		        
 	sb	t1,3(a0)

 	li	t1,0		        
 	sb	t1,1(a0)

 	li	t1,71		        
 	sb	t1,2(a0)

	jr	ra
	nop	

    .end    InitNs16550Serial0


/* serial 1  */
    .text
    .global InitNs16550Serial1
    .ent    InitNs16550Serial1
    
InitNs16550Serial1:
    
    li  a0, HR_UART1_BASE

 	li	t1,128              
 	sb	t1,3(a0)

 	li	t1,0x12      	    
 	sb	t1,0(a0)

 	li	t1,0x0     	        
 	sb	t1,1(a0)

 	li	t1,3		        
 	sb	t1,3(a0)

 	li	t1,0		        
 	sb	t1,1(a0)

 	li	t1,71		        
 	sb	t1,2(a0)

	jr	ra
	nop

    .end    InitNs16550Serial1

/* tgt_putchar */
    .text
    .global tgt_putchar
    .ent    tgt_putchar
tgt_putchar:
    la     v0, HR_UART0_BASE 
1:
    lbu    v1, NS16550_LSR(v0)
    and    v1, LSR_TXRDY
    beqz   v1, 1b
    nop

    sb     a0, 0(v0)
    move   v1, v0
    la     v0, HR_UART0_BASE
    bne    v0, v1, 1b
    nop

    j   ra
    nop 
    .end    tgt_putchar    

	/* tgt_getchar */
    .text
    .global tgt_getchar
    .ent    tgt_getchar
tgt_getchar:
    la     v0, HR_UART0_BASE 
1:
	lbu    v1, NS16550_LSR(v0)
	and v1, LSR_RXRDY
	beqz	v1, 1b
	nop
	lb	v0, NS16550_DATA(v0)
	j ra
	nop
    
    .end    tgt_getchar 
    

/* stringserial */
    .global stringserial    
    .ent    stringserial
stringserial:
    move   a2, ra

    la     a1, __romInit
    subu   a1, ra, a1
    and    a1, 0xffff0000
    addu   a1, a1, a0       
   
    lbu    a0, 0(a1)        

1:
    beqz   a0, 2f
    nop
    bal    tgt_putchar
    addiu  a1, 1
    b      1b
    lbu    a0, 0(a1)

2:
    j     a2
    nop
    .end    stringserial

/* hexserial   mxl:  use s0, s2 ,  cannot find any register to use */
    .global hexserial
    .ent    hexserial
hexserial:
    move    s0, ra
    move    a1, a0
    li      a3, 7

    la      s2, __romInit
    subu    s2, ra,s2
    and     s2, 0xffff0000

1:
    rol     a0, a1, 4   
    move    a1, a0
    and     a0, 0xf

    la      a2, hexchar
    addu    a2, s2      

    addu    a2, a0
    bal     tgt_putchar
    lbu     a0, 0(a2)

    bnez    a3, 1b
    addu    a3, -1

    j       s0
    nop
    .end    hexserial
    
/* hexserial64 */
    .global hexserial64
    .ent    hexserial64
    
hexserial64:

    move    t9, ra
    move    t8, a0
    dsrl    a0, a0,32
    bal     hexserial
    nop

    move    a0, t8
    bal     hexserial
    nop
    
    jr      t9
    nop
    .end    hexserial64


/* ############  fixed up ############ */

#define setwin(baseaddr, baseval, maskval, mmapval) \
    dli a0, (baseaddr);  \
    dli t0, (baseval);   \
    dli t1,(maskval);    \
    dli t2, (mmapval);   \
    sd  $0, 0x80(a0) ;   \
    sd  t0, 0x0(a0);     \
    sd  t1, 0x40(a0);    \
    sd  t2, 0x80(a0)
    
    .text
    .set noreorder
    
    .global XbarInit
    .ent    XbarInit
    
XbarInit:
    move    s1, ra    

    //set scache hash index
    //dli   a0, 0xf     //using 37:36
    dli a0, 0x2         //using 11:10
    dli t0, 0x900000003ff00400
    sd  a0, 0x0(t0)
    PRINTSTR("   Scache hash index setup ...done")


#if 0  /* mxl: config XBAR address route for HT  */
// #define  FIX_HT_CONFIG
    dli t2, 0x900000003ff02000
    dli t1, 0x900000003ff02600
    PRINTSTR("\r\n   CPU0 Fix L1xbar illegal access ")
1:

#if 0
    dli t0, 0x0000000017000000  // mxl :  17000000 -> cfbfb000000
    sd  t0, 0x00(t2)
    dli t0, 0xffffffffff000000
    sd  t0, 0x40(t2)
    dli t0, 0x00000cfdfb0000f6
    sd  t0, 0x80(t2)
    
    dli t0, 0x0000000018000000   
    sd  t0, 0x08(t2)
    dli t0, 0xfffffffffc000000
    sd  t0, 0x48(t2)
    dli t0, 0x00000cfdfc0000f6
    sd  t0, 0x88(t2)
/*
    dli t0, 0x0000000010000000
    sd  t0, 0x10(t2)
    dli t0, 0xfffffffff8000000
    sd  t0, 0x50(t2)
    dli t0, 0x00000c00100000f6
    sd  t0, 0x90(t2)
*/
    dli t0, 0x000000001e000000
    sd  t0, 0x20(t2)
    dli t0, 0xffffffffff000000
    sd  t0, 0x60(t2)
    dli t0, 0x00000c00000000f6
    sd  t0, 0xa0(t2)


    daddiu  t2, t2, 0x100
    bne     t2, t1, 1b
    nop
    
#else

#ifdef FIX_HT_CONFIG
        dli t0, 0x000000fdfe000000
        sd  t0, 0x10(t2)
        dli t0, 0x000000fffe000000
        sd  t0, 0x50(t2)
        dli t0, 0x000000fdfe0000f6
        sd  t0, 0x90(t2)
    
        dli t0, 0x00000c0000000000
        sd  t0, 0x18(t2)
        dli t0, 0x00000e0000000000
        sd  t0, 0x58(t2)
        dli t0, 0x00000c00000000f6
        sd  t0, 0x98(t2)
#endif  /* FIX_HT_CONFIG */
        daddiu  t2, t2, 0x100
        bne     t2, t1, 1b
        nop
        
#endif

#endif

	PRINTSTR("\r\n   L1-L2 xbar address route config  ...done ")


#if 0 /* dump L1-L2-HT config windows for debugging */

    PRINTSTR("\r\n======X1 core0 map windows:\r\n")
    li      t1, 23
    dli     t2, 0x900000003ff02000
1:
    move    a0, t2
    bal    hexserial64
    nop
    PRINTSTR(": ")

    ld      a0, 0x0(t2)
    bal    hexserial64
    nop

    PRINTSTR("\r\n")

    daddiu  t2, t2, 8
    bnez    t1, 1b
    addiu   t1, t1, -1


    PRINTSTR("\r\n======X2 cpu map windows:\r\n")
    li      t1, 23
    dli     t2, 0x900000003ff00000
1:
    move    a0, t2
    bal    hexserial64
    nop
    PRINTSTR(": ")

    ld      a0, 0x0(t2)
    bal    hexserial64
    nop
    PRINTSTR("\r\n")

    daddiu  t2, t2, 8
    bnez    t1, 1b
    addiu   t1, t1, -1

    PRINTSTR("\r\n======X2 pci map windows:\r\n")
    li      t1, 23
    dli     t2, 0x900000003ff00100
1:
    move    a0, t2
    bal    hexserial64
    nop
    PRINTSTR(": ")

    ld      a0, 0x0(t2)
    bal    hexserial64
    nop
    PRINTSTR("\r\n")

    daddiu  t2, t2, 8
    bnez    t1, 1b
    addiu   t1, t1, -1

    PRINTSTR("\r\n======read HT config reg:\r\n")
    dli     t2, 0x90000cfdfb000000

    move    a0, t2
    bal    hexserial64
    nop
    PRINTSTR(": ")

    ld      a0, 0x0(t2)
    bal    hexserial64
    nop
    PRINTSTR("\r\n")

    daddiu    a0, t2, 0x60
    bal    hexserial64
    nop
    PRINTSTR(": ")

    ld      a0, 0x60(t2)
    bal    hexserial64
    nop
    PRINTSTR("\r\n")

    daddiu    a0, t2, 0x68
    bal    hexserial64
    nop
    PRINTSTR(": ")

    ld      a0, 0x68(t2)
    bal    hexserial64
    nop
    PRINTSTR("\r\n")

    daddiu    a0, t2, 0x70
    bal    hexserial64
    nop
    PRINTSTR(": ")

    ld      a0, 0x70(t2)
    bal    hexserial64
    nop
    PRINTSTR("\r\n")

#endif 

    move    ra, s1
    j       ra
    nop
    .end    XbarInit



/*    DDR init function    */
    .text
    .set    noreorder    
    .global DDRInit
    .ent    DDRInit
    
DDRInit:    

    move    s1,ra                   

    /*PRINTSTR("\r\nEnable register space of MEMORY\r\n")*/
    li  t2, 0xbfe00180
    lw  a1, 0x0(t2)
    li  a0, 0xfffffeff
    and a1, a1, a0
    sw  a1, 0x0(t2)

    li  s2, 0x1f000000       //512M
    //li  s2, 0x3f000000    //1G
    //li  s2, 0x0f000000    //256
        
#ifdef MEM_CHAN_ONLY_MC1   /* mxl:  not defined  */

    dli     t0, 0x900000003ff00080  //#mmap
    dli     t1, 0x00000000000000f1               /*  route to mc1*/
    sd      t1, 0(t0)
    sync

    PRINTSTR("MC1 High DDR2 space open : 0x20000000 - 0x2FFFFFFF\r\n")
    dli t0, 0x900000003ff00010  //base
    dli t1, 0x20000000        
    sd  t1, 0(t0)
    sync    

    dli t0, 0x900000003ff00050  //mask
    dli t1, 0xfffffffff0000000       
    sd  t1, 0(t0)
    sync    

    dli t0, 0x900000003ff00090  //mmap
    dli t1, 0x100000f1               
    sd  t1, 0(t0)
    sync

    PRINTSTR("MC1 DDR2 config begin\r\n")
    dli a0, 0x900000000ff00000
    bal ddr2_config
    nop
    PRINTSTR("MC1 DDR2 config end\r\n")

#else

#if 0
    PRINTSTR("   MC0 High DDR2 space open : 0x20000000 - 0x2FFFFFFF\r\n")
    dli t0, 0x900000003ff00010  //base
    dli t1, 0x20000000        
    sd  t1, 0(t0)
    sync    

    dli t0, 0x900000003ff00050  //mask
    dli t1, 0xfffffffff0000000       
    sd  t1, 0(t0)
    sync    

    dli t0, 0x900000003ff00090  //mmap
    dli t1, 0x100000f0         /* mxl:  why is  0x100000f0 
                                  maybe low address can be access from 0x80000000 */
    sd  t1, 0(t0)
    sync

    PRINTSTR("   MC0 DDR2 config begin\r\n")
    dli a0, 0x900000000ff00000
    bal ddr2_config
    nop
    PRINTSTR("   MC0 DDR2 config end\r\n")

#ifdef MEM_CHAN_BOTH   /* mxl: defined */
    PRINTSTR("   MC1 DDR2 space open : 0x60000000 - 0x7FFFFFFF\r\n")
    dli t0, 0x900000003ff00018  //base
    dli t1, 0x60000000
    sd  t1, 0(t0)
    sync    

    dli t0, 0x900000003ff00058  //mask
    dli t1, 0xffffffffe0000000 
    sd  t1, 0(t0)
    sync    

    dli t0, 0x900000003ff00098  //mmap
    dli t1, 0xf1
    sd  t1, 0(t0)
    sync

    PRINTSTR("   MC1 DDR2 config begin\r\n")
    dli a0, 0x900000006ff00000
    bal ddr2_config_mc1
    nop
    PRINTSTR("   MC1 DDR2 config end\r\n")  
#endif  /* MEM_CHAN_BOTH */
#else
	PRINTSTR("   MC0 High DDR2 space open : 0x60000000 - 0x7FFFFFFF\r\n")
    dli t0, 0x900000003ff00010  //base
    dli t1, 0x60000000        
    sd  t1, 0(t0)
    sync    

    dli t0, 0x900000003ff00050  //mask
    dli t1, 0xffffffffe0000000       
    sd  t1, 0(t0)
    sync    

    dli t0, 0x900000003ff00090  //mmap
    dli t1, 0xf0         /* mxl:  why is  0x100000f0 
                                  maybe low address can be access from 0x80000000 */
    sd  t1, 0(t0)
    sync

    PRINTSTR("   MC0 DDR2 config begin\r\n")
    dli a0, 0x900000006ff00000
    bal ddr2_config
    nop
    PRINTSTR("   MC0 DDR2 config end\r\n")

#ifdef MEM_CHAN_BOTH   /* mxl: defined */
    PRINTSTR("   MC1 DDR2 space open : 0x00000000 - 0x0FFFFFFF")
    dli t0, 0x900000003ff00000  //base
    dli t1, 0x00000000
    sd  t1, 0(t0)
    sync    

    dli t0, 0x900000003ff00040  //mask
    dli t1, 0xfffffffff0000000 
    sd  t1, 0(t0)
    sync    

    dli t0, 0x900000003ff00080  //mmap
    dli t1, 0xf1
    sd  t1, 0(t0)
    sync

	PRINTSTR("\r\n")
	PRINTSTR("   MC1 DDR2 space open : 0x20000000 - 0x2FFFFFFF\r\n")
	dli t0, 0x900000003ff00018	//base
	dli t1, 0x20000000
	sd	t1, 0(t0)
	sync	

	dli t0, 0x900000003ff00058	//mask
	dli t1, 0xfffffffff0000000 
	sd	t1, 0(t0)
	sync	

	dli t0, 0x900000003ff00098	//mmap
	dli t1, 0x100000f1
	sd	t1, 0(t0)
	sync

	#if 1  /* mxl : for ddr  test conveniently, mapped  Continuous address  */
	PRINTSTR("\r\n")
	PRINTSTR("   MC1 DDR2 space open : 0x40000000 - 0x5FFFFFFF\r\n")
	dli t0, 0x900000003ff00020	//base
	dli t1, 0x40000000
	sd	t1, 0(t0)
	sync	

	dli t0, 0x900000003ff00060	//mask
	dli t1, 0xffffffffe0000000 
	sd	t1, 0(t0)
	sync	

	dli t0, 0x900000003ff000a0	//mmap
	dli t1, 0x000000f1
	sd	t1, 0(t0)
	sync
	#endif

    PRINTSTR("   MC1 DDR2 config begin\r\n")
    dli a0, 0x900000000ff00000
    bal ddr2_config_mc1
    nop
    PRINTSTR("   MC1 DDR2 config end\r\n")  

	PRINTSTR("Config PCI address windows\r\n")
	dli t0, 0x900000003ff00100	//base
	dli t1, 0x80000000
	sd	t1, 0(t0)
	sync	

	dli t0, 0x900000003ff00140	//mask
	dli t1, 0xffffffff80000000 
	sd	t1, 0(t0)
	sync	

	dli t0, 0x900000003ff00180	//mmap
	dli t1, 0x000000f1
	sd	t1, 0(t0)
	sync

#endif  /* MEM_CHAN_BOTH */



#endif

#endif //MEM_CHAN_ONLY_MC1
        
    //PRINTSTR("Disable register space of MEMORY\r\n")
    li  t2, 0xbfe00180
    lw  a1, 0x0(t2)
    or  a1, a1,0x100
    sw  a1, 0x0(t2)

    //PRINTSTR("Disable read buffer\r\n")
    li  t0, 0xbfe00180
    lw  t1, 0x4(t0)
    li  a0, 0x18
    or  t1, t1, a0
    sw  t1, 0x4(t0)

    //PRINTSTR("Disable cpu buffered read\r\n")
    li  t0, 0xbfe00180
    lw  t1, 0x0(t0)
    li  a0, 0xfffffdff
    and t1, t1, a0
    sw  t1, 0x0(t0)       




#if 0 /* dump L1-L2-HT config windows for debugging */
	
		PRINTSTR("\r\n======X1 core0 map windows:\r\n")
		li		t1, 23
		dli 	t2, 0x900000003ff02000
	1:
		move	a0, t2
		bal    hexserial64
		nop
		PRINTSTR(": ")
	
		ld		a0, 0x0(t2)
		bal    hexserial64
		nop
	
		PRINTSTR("\r\n")
	
		daddiu	t2, t2, 8
		bnez	t1, 1b
		addiu	t1, t1, -1
	
	
		PRINTSTR("\r\n======X2 cpu map windows:\r\n")
		li		t1, 23
		dli 	t2, 0x900000003ff00000
	1:
		move	a0, t2
		bal    hexserial64
		nop
		PRINTSTR(": ")
	
		ld		a0, 0x0(t2)
		bal    hexserial64
		nop
		PRINTSTR("\r\n")
	
		daddiu	t2, t2, 8
		bnez	t1, 1b
		addiu	t1, t1, -1
	
		PRINTSTR("\r\n======X2 pci map windows:\r\n")
		li		t1, 23
		dli 	t2, 0x900000003ff00100
	1:
		move	a0, t2
		bal    hexserial64
		nop
		PRINTSTR(": ")
	
		ld		a0, 0x0(t2)
		bal    hexserial64
		nop
		PRINTSTR("\r\n")
	
		daddiu	t2, t2, 8
		bnez	t1, 1b
		addiu	t1, t1, -1
	
		PRINTSTR("\r\n======read HT config reg:\r\n")
		dli 	t2, 0x90000cfdfb000000
	
		move	a0, t2
		bal    hexserial64
		nop
		PRINTSTR(": ")
	
		ld		a0, 0x0(t2)
		bal    hexserial64
		nop
		PRINTSTR("\r\n")
	
		daddiu	  a0, t2, 0x60
		bal    hexserial64
		nop
		PRINTSTR(": ")
	
		ld		a0, 0x60(t2)
		bal    hexserial64
		nop
		PRINTSTR("\r\n")
	
		daddiu	  a0, t2, 0x68
		bal    hexserial64
		nop
		PRINTSTR(": ")
	
		ld		a0, 0x68(t2)
		bal    hexserial64
		nop
		PRINTSTR("\r\n")
	
		daddiu	  a0, t2, 0x70
		bal    hexserial64
		nop
		PRINTSTR(": ")
	
		ld		a0, 0x70(t2)
		bal    hexserial64
		nop
		PRINTSTR("\r\n")
	
#endif 


    
    move    ra, s1
    j       ra
    nop

    .end DDRInit


/*ddr2_config */

    .global ddr2_config
    .ent    ddr2_config
    
ddr2_config:
    move    a2, ra
    RELOC(t0, ddr2_reg_data)

    li      t1, 180     //0x72    
    daddiu  t2, a0, 0x0
    
reg_write:
    ld      a1, 0x0(t0)
    sd      a1, 0(t2)

    subu    t1, t1, 0x1
    addiu   t0, t0, 0x8
    bne     t1, $0, reg_write
    daddiu  t2, t2, 0x10
    
    li      t2, 0xfff
1:
    bnez    t2, 1b
    addi    t2, t2, -1
    nop
    sync
    
    daddiu  t2, a0, 0x0
    RELOC(t0, MC0_CTL_start_DATA_LO)   
    
    ld      a1, 0x0(t0)
    sd      a1, 0x30(t2)

    daddiu  t2, a0, 0x960  //wait int status 
1:
    ld      a1, 0x0(t2)
    andi    a1, a1, 0x100
    beqz    a1, 1b
    nop

    daddiu  t2, a0, 0x30  //Set to srefresh
    dli     a1, 0x0000000100000000
    ld      t0, 0x0(t2)
    or      a1, a1, t0
    sd      a1, 0x0(t2)

    li      t2, 0xfff
1:
    bnez    t2, 1b
    addi    t2, t2, -1
    nop
    sync
        
    daddiu  t2, a0, 0x30  //Out of srefresh
    dli     a1, 0xffffff00ffffffff
    ld      t0, 0x0(t2)
    and     a1, a1, t0
    sd      a1, 0x0(t2)

    li      t2, 0xfff
1:
    bnez    t2, 1b
    addi    t2, t2, -1
    nop
    sync
    
    daddiu  t2, a0, 0x40  //Write mode regs
    dli     a1, 0x0000000001000000
    ld      t0, 0x0(t2)
    or      a1, a1, t0
    sd      a1, 0x0(t2)

    jr      a2
    nop
    .end    ddr2_config

/* ddr2_config_mc1 */

    .global ddr2_config_mc1
    .ent    ddr2_config_mc1
    
ddr2_config_mc1:
    move    a2, ra
    RELOC(t0,ddr2_reg_data_mc1)    
    
    li      t1, 180   //0x72    
    daddiu  t2, a0, 0x0

reg_write_mc1:
    ld      a1, 0x0(t0)
    sd      a1, 0(t2)

    subu    t1, t1, 0x1
    addiu   t0, t0, 0x8
    bne     t1, $0, reg_write_mc1
    daddiu  t2, t2, 0x10
    
    li      t2, 0xfff
1:
    bnez    t2, 1b
    addi    t2, t2, -1
    nop
    sync
    
    /* start*/    
    daddiu  t2, a0, 0x0    
    RELOC(t0,MC1_CTL_start_DATA_LO)
    
    ld      a1, 0x0(t0)
    sd      a1, 0x30(t2)

    daddiu  t2, a0, 0x960  //wait int status 
1:
    ld      a1, 0x0(t2)
    andi    a1, a1, 0x100
    beqz    a1, 1b
    nop

    daddiu  t2, a0, 0x30   //Set to srefresh
    dli     a1, 0x0000000100000000
    ld      t0, 0x0(t2)
    or      a1, a1, t0
    sd      a1, 0x0(t2)

    li      t2, 0xfff
1:
    bnez    t2, 1b
    addi    t2, t2, -1
    nop
    sync

    daddiu  t2, a0, 0x30   //Out of srefresh
    dli     a1, 0xffffff00ffffffff
    ld      t0, 0x0(t2)
    and     a1, a1, t0
    sd      a1, 0x0(t2)

    li      t2, 0xfff
1:
    bnez    t2, 1b
    addi    t2, t2, -1
    nop
    sync

    daddiu  t2, a0, 0x40   //Write mode regs
    dli     a1, 0x0000000001000000
    ld      t0, 0x0(t2)
    or      a1, a1, t0
    sd      a1, 0x0(t2)

    jr      a2
    nop
    .end    ddr2_config_mc1
   



/*################# TLB init ########*/
 
    
/******************************************************************************
* build tlb
* VPN(ENTRYHI)  PFN(ENTRYLO0)  PFN(ENTRYLO0)
* 0x8000,0000   0x0000,0000    0x0000,0000
* 0x8000,2000   0x0000,0000    0x0000,0000
* 0x8000,4000   0x0000,0000    0x0000,0000
* ....
*step is 0x2000
*num of total items is 64
*
*
*/
    .text
    .set noreorder

/******************************************************************************
* tlb_init
* used reg: v0,v1
*/ 
    .global CPU_TLBClear
    .ent    CPU_TLBClear
CPU_TLBClear:
        li     v0, 0x00000000      // PG_SIZE_4K
        mtc0   v0, C0_PAGEMASK     // Whatever...

        li     v1, 0               // First TLB index.
        li     v0, 64

1:
        MTC0   $0, C0_TLBHI         // Clear entry high.
        MTC0   $0, C0_TLBLO0        // Clear entry low0.
        MTC0   $0, C0_TLBLO1        // Clear entry low1.

        mtc0   v1, C0_INX           // Set the index.
        addiu  v1, 1
        nop
        nop
        tlbwi                       // Write the TLB

        bne    v1, v0, 1b
        nop

        jr     ra
        nop
    .end    CPU_TLBClear
    
      
/******************************************************************************
* tlb_init
* used reg: a0,a1,a2,v0,v1
*/    
    .global tlb_init
    .ent    tlb_init
tlb_init:
        mtc0    $0, C0_WIRED
        mtc0    $0, C0_PAGEMASK
tlb_flush_all:
        lui     a0, 0x8000
        addiu   a1, $0, 64
        //a0=KSEG0,a1 = tlbsize, v0, v1, a3 used as local registers
        MTC0    $0, C0_TLBLO0
        MTC0    $0, C0_TLBLO1
        mfc0    v0, C0_WIRED
        addu    v1, $0, a0
1:      sltu    a2, v0, a1
        beq     a2, $0, 1f
        nop
        MTC0    v1, C0_TLBHI
        mtc0    v0, C0_INX
        tlbwi
        addiu   v1, v1, 0x2000
        beq     $0, $0, 1b
        addiu   v0, v0, 1
1:
        //tlb_init finish
        tlbp
    .end    tlb_init






/******************************************************************************
* CPU_L1cache_init
* used reg: a0,a1,a2,a3,t0,t1,t2,t3,t4,v0,v1
*/
    .global CPU_L1cache_init
    .ent    CPU_L1cache_init
CPU_L1cache_init:    

        lui     a0, 0x8000
        li      a1, (1<<14)     //64k/4way,16k=(1<<14)
        li      a2, (1<<14)
//cache_init_d4way:
//a0=0x80000000, a1=icache_size, a2=dcache_size
//a3, v0 and v1 used as local registers
        mtc0    $0, C0_TAGHI
        li      t0, 0x22
        MTC0    t0, C0_ECC
        addu    v0, $0, a0
        addu    v1, a0, a2
1:      slt     a3, v0, v1      //a3 = (v0<v1)?1:0
        beq     a3, $0, 1f
        nop
        mtc0    $0, C0_TAGLO
        cache   0x09, 0x0(v0)   //Index_Store_Tag_D
        cache   0x09, 0x1(v0)
        cache   0x09, 0x2(v0)
        cache   0x09, 0x3(v0)
        beq     $0, $0, 1b
        addiu   v0, v0, 0x20
1:
//cache_flush_i4way:
        addu    v0, $0, a0
        addu    v1, a0, a1
        mtc0    $0, C0_TAGLO
        mtc0    $0, C0_TAGHI
        MTC0    $0, C0_ECC
1:      slt     a3, v0, v1
        beq     a3, $0, 1f
        nop
        cache   0x08, 0x0(v0)   //Index_Store_Tag_I
        cache   0x08, 0x1(v0)
        cache   0x08, 0x2(v0)
        cache   0x08, 0x3(v0)
        beq     $0, $0, 1b
        addiu   v0, v0, 0x20
1:
//cache_init_finish:    

        jr      ra
        nop

1:      b       1b
        nop
    .end    CPU_L1cache_init

/******************************************************************************
* scache_init_64
* used reg: a0,a1,v0,v1
*/
    .global CPU_L2cache_init_64
    .ent    CPU_L2cache_init_64
CPU_L2cache_init_64:  
        dli     a1, 0x00100000   //4M/4way
//scache_init_4way_64:
//a0=0x80000000, a1=scache_size
//v0 and v1 used as local registers
        li      t0, 0x22
        MTC0    t0, C0_ECC
        mtc0    $0, C0_TAGHI
        mtc0    $0, C0_TAGLO
        daddu   v0, $0, a0
        daddu   v1, a0, a1
1:      beq     v0, v1, 1f
        nop        
        
        cache   0x0b, 0x0(v0)   //Index_Store_Tag_S
        cache   0x0b, 0x1(v0)
        cache   0x0b, 0x2(v0)
        cache   0x0b, 0x3(v0)        
        
        beq     $0, $0, 1b
        daddiu  v0, v0, 0x20
1:
        jr      ra
        nop

1:      b       1b
        nop
    .end    CPU_L2cache_init_64


.extern mipsTlbClear

/* Tlb_L1Cache_init */
    .text
    .set noreorder
    .ent    Tlb_L1Cache_init
    
Tlb_L1Cache_init:

        move    s1,ra           
        
        RELOC(k1, cp0_init)
        jal   k1
        nop
        
        RELOC(k1, CPU_L1cache_init)
        jal   k1
        nop 
        
        
        j   s1
        nop
        
    .end    Tlb_L1Cache_init

/***************************************************************************
*
* L2Cache_init - initialization sequence for on-chip peripherials that
*                  are required during boot-up
*
*  Input parameters:
*      none
*
*  Return value:
*      none
*/

    .ent    L2Cache_init
L2Cache_init:

    move    s1,ra                   /* will be trashing RA */

    /*
     * Init the L2 cache.  We don't init L2 on
     * secondary CPU(s), since it is shared by all CPUs.  We'll
     * do it here by cpu0 only.
     */
    dli a0, 0x9800000000000000
    RELOC(k1, CPU_L2cache_init_64)
    jal   k1
    nop

        
    j   s1
    nop
            
    .end    L2Cache_init


/***************************************************************************
*
* cp0_init - initialize CP0 registers for an SB1 core
*
*  Input parameters:
*      none
*
*  Return value:
*      none
*/

        .ent    cp0_init
cp0_init:
        .set noreorder                  /* so as to minimize HAZARDS */
        
    move    s2,ra   /* save ra */


    //RELOC(k1,mipsTlbClear)
    RELOC(k1,CPU_TLBClear)
    jal     k1
    nop

    RELOC(k1,tlb_init)
    jal     k1
    nop

    jr      s2      /* return via saved ra */
    nop

    .end    cp0_init


#include "test_mem.s"


/*  ddr2_reg_data */
//#define DDR_DEBUG
#if 1
    .data
    .align 5
    .global ddr2_reg_data
    
ddr2_reg_data:
MC0_CTL_000: .double 0x0000000000000101
//000000000000000_0 concurrentap(RW) 0000000_1 bank_split_en(RW) 0000000_0 auto_refresh_mode(RW) 0000000_0 arefresh(WR) 0000000_0 ap(RW) 0000000_1 addr_cmp_en(RW) 0000000_1 active_aging(RW) 
MC0_CTL_010: .double 0x0001000100010000
//0000000_0 fwc(WR) 0000000_0 fast_write(RW) 0000000_1 enable_quick_srefresh(RW) 0000000_1 eight_bank_mode(RW) 0000000_0 ecc_disable_w_uc_err(RW) 0000000_1 dqs_n_en(RW) 0000000_0 dll_bypass_mode(RW) 0000000_0 dlllockreg(RD) 
MC0_CTL_020: .double 0x0100010100000000
// 0x0100010101000000 mxl modified 
//0000000_1 priority_en(RW) 0000000_0 power_down(RW) 0000000_1 placement_en(RW) 0000000_1 odt_add_turn_clk_en(RW) 0000000_0 no_cmd_init(RW) 0000000_0 intrptwritea(RW) 0000000_0 intrptreada(RW) 0000000_0 intrptapburst(RW) 
MC0_CTL_030: .double 0x0101000001000000
//0000000_1 swap_port_rw_same_en(RW) 0000000_1 swap_en(RW) 0000000_0 start(RW) 0000000_0 srefresh(RW+) 0000000_1 rw_same_en(RW) 0000000_0 reg_dimm_enable(RW) 0000000_0 reduc(RW) 0000000_0 pwrup_srefresh_exit(RW) 
MC0_CTL_040: .double 0x0100010100010101
//000000_01 rtt_0(RW) 000000_11 ctrl_raw(RW) 000000_01 axi0_w_priority(RW) 000000_01 axi0_r_priority(RW) 0000000_0 write_modereg(WR) 0000000_1 writeinterp(RW) 0000000_1 tref_enable(RW) 0000000_1 tras_lockout(RW) 
MC0_CTL_050: .double 0x0000000404050100
//00000_000 q_fullness(RW) 00000_000 port_data_error_type(RD) 000_00000 out_of_range_type(RD) 00000_000 max_cs_reg(RD) 00000_100 column_size(RW) 0000_0101 caslat(RW) 00000_010 addr_pins(RW) 00000000
MC0_CTL_060: .double 0x0a04040603040003
//0000_1010 aprebit(RW) 0000_0101 wrlat(RW) 0000_0100 twtr(RW) 000_00110 twr_int(RW) 00000_011 trtp(RW) 00000_100 trrd(RW) 0000000000000_011 tcke(RW) 
MC0_CTL_070: .double 0x0f0e020000010a08
//0000_0000 max_row_reg(RD) 0000_0000 max_col_reg(RD) 0000_0000 initaref(RW) 00000000000000000000_1111 cs_map(RW) 000_01010 caslat_lin_gate(RW) 000_01010 caslat_lin(RW) 
MC0_CTL_080: .double 0x0104040101000400
//0000_0001 odt_wr_map_cs3(RW) 0000_0010 odt_wr_map_cs2(RW) 0000_0100 odt_wr_map_cs1(RW) 0000_1000 odt_wr_map_cs0(RW) 0000_0001 odt_rd_map_cs3(RW) 0000_0010 odt_rd_map_cs2(RW) 0000_0100 odt_rd_map_cs1(RW) 0000_1000 odt_rd_map_cs0(RW) 
MC0_CTL_090: .double 0x0000050b00000000
//000_00000 ocd_adjust_pup_cs_0(RW) 000_00000 ocd_adjust_pdn_cs_0(RW) 0000_0101 trp(RW) 000_01011 tdal(RW) 000000000000_0000 port_cmd_error_type(RD) 0000000000000000
MC0_CTL_0a0: .double 0x0000003f3f140612
//MC0_CTL_0a0: .double 0x0000003f3f14021b
//00000000000000000000000000_111111 command_age_count(RW) 00_111111 age_count(RW) 000_10100 trc(RW) 000_00110 tmrd(RW) 000_10010 tfaw(RW) 
MC0_CTL_0b0: .double 0x0000000000000000
MC0_CTL_0c0: .double 0x00002c050f000000
//000000000000000000101100 trfc(RW) 00000101 trcd_int(RW) 00001111 tras_min(RW) 00000000 out_of_range_length(RD) 00000000 ecc_u_synd(RD) 00000000 ecc_c_synd(RD) 
MC0_CTL_0d0: .double 0x0000000000000000
MC0_CTL_0e0: .double 0x0000000000000000
MC0_CTL_0f0: .double 0x0000000000000000
MC0_CTL_100: .double 0x0000000000000000
MC0_CTL_110: .double 0x0000000000001c2d
//0_000000000000000 emrs2_data_1(RW) 0_000000000000000 emrs2_data_0(RW) 000000000000000000_00110000101101 tref(RW) 
MC0_CTL_120: .double 0xffff000000000000
//0000000000011100 axi0_en_size_lt_width_instr(RW) 00000000000000000_000000000000000 emrs2_data_3(RW) 0_000000000000000 emrs2_data_2(RW) 
MC0_CTL_130: .double 0x0d56000302000000
//0110110101010110 tras_max(RW) 0000000000000011 tpdex(RW) 0000001000000000 tdll(RW) 0000000000000000 tcpd(RW) 
MC0_CTL_140: .double 0x0000204002000030
//0000000000000000 xor_check_bits(RW) 0000000000000000 version(RD) 0000001000000000 txsr(RW) 0000000000110000 txsnr(RW) 
MC0_CTL_150: .double 0x0000000011000004
//000_0000000000000000000000000000000000000 ecc_c_addr(RD) 000000000000000000011011 tinit(RW) 
MC0_CTL_160: .double 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 ecc_u_addr(RD) 
MC0_CTL_170: .double 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 out_of_range_addr(RD) 
MC0_CTL_180: .double 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 port_cmd_error_addr(RD) 
MC0_CTL_190: .double 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000000 ecc_c_data(RD) 
MC0_CTL_1a0: .double 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000000 ecc_u_data(RD) 
MC0_CTL_1b0: .double 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000_000 cke_delay(RW) 
MC0_CTL_1c0: .double 0x0000000000000000
MC0_CTL_1d0: .double 0x0203070400000101
//0000_0010 tdfi_phy_wrlat_base(RW) 0000_0000 tdfi_phy_wrlat(RD) 000_00111 tdfi_phy_rdlat(RW) 0000_0000 tdfi_ctrlupd_min(RD) 000000000000_0000 dram_clk_disable(RW) 0000000_1 odt_alt_en(RW) 0000000_1 drive_dq_dqs(RW) 
MC0_CTL_1e0: .double 0x0c2d0c2d0c2d0205
//00_00000000000000 tdfi_phyupd_type0(RD) 00_00000000000000 tdfi_phyupd_resp(RD) 00_00000000000000 tdfi_ctrlupd_max(RD) 000_00000 tdfi_rddata_en_base(RW) 000_00000 tdfi_rddata_en(RD) 
#ifdef DDR_DEBUG
//MC0_CTL_1f0: .double 0x00120e8000000000
//MC0_CTL_200: .double 0x00160e8000160e80
//MC0_CTL_210: .double 0x00100e8000130e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC0_CTL_220: .double 0x00130e8000010e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC0_CTL_230: .double 0x00200e8000100e80

#if 1
MC0_CTL_1f0: .double 0x000a0e8000000000
MC0_CTL_200: .double 0x000a0e80000a0e80
MC0_CTL_210: .double 0x000a0e80000a0e80
//00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
MC0_CTL_220: .double 0x000a0e80000a0e80
//00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
MC0_CTL_230: .double 0x000a0e80000a0e80
#else
MC0_CTL_1f0: .double 0x00300e8000000000
MC0_CTL_200: .double 0x00300e8000300e80
MC0_CTL_210: .double 0x00300e8000300e80
//00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
MC0_CTL_220: .double 0x00300e8000300e80
//00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
MC0_CTL_230: .double 0x00300e8000300e80

#endif

//#debug write
//MC0_CTL_1f0: .double 0x00000e8000000000
//MC0_CTL_200: .double 0x00000e8000000e80
//MC0_CTL_210: .double 0x00000e8000000e80
//00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC0_CTL_220: .double 0x00000e8000000e80
//00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC0_CTL_230: .double 0x00200e8000000e80

//#debug read
//MC0_CTL_1f0: .double 0x00120e8000000000
//MC0_CTL_200: .double 0x00160e8000160e80
//MC0_CTL_210: .double 0x00100e8000140e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC0_CTL_220: .double 0x00100e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC0_CTL_230: .double 0x00200e8000140e80

#else
MC0_CTL_1f0: .double 0x00180e8000000000
MC0_CTL_200: .double 0x00180e8000180e80
MC0_CTL_210: .double 0x00180e8000180e80
//00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
MC0_CTL_220: .double 0x00180e8000180e80
//00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
MC0_CTL_230: .double 0x00180e8000180e80

//MC0_CTL_1f0: .double 0x00130e8000000000
//MC0_CTL_200: .double 0x00130e8000130e80
//MC0_CTL_210: .double 0x00130e8000130e80
////00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC0_CTL_220: .double 0x00100e8000130e80
////00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC0_CTL_230: .double 0x00150e8000120e80

//MC0_CTL_1f0: .double 0x00230eff00000000
//MC0_CTL_200: .double 0x00230eff00230eff
//MC0_CTL_210: .double 0x00230eff00230eff
////00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC0_CTL_220: .double 0x00200eff00230eff
////00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC0_CTL_230: .double 0x00150eff00200eff

//MC0_CTL_1f0: .double 0x00120e8000000000
//MC0_CTL_200: .double 0x00160e8000160e80
//MC0_CTL_210: .double 0x00100e8000180e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC0_CTL_220: .double 0x000c0e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC0_CTL_230: .double 0x00200e8000100e80

//150M MC0
//MC0_CTL_1f0: .double 0x001a0e8000000000
//MC0_CTL_200: .double 0x00120e8000120e80
//MC0_CTL_210: .double 0x00100e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC0_CTL_220: .double 0x00100e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC0_CTL_230: .double 0x00200e8000120e80

//150M MC0
//MC0_CTL_1f0: .double 0x00160e8000000000
//MC0_CTL_200: .double 0x00100e8000100e80
//MC0_CTL_210: .double 0x00100e8000160e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC0_CTL_220: .double 0x00160e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC0_CTL_230: .double 0x00200e8000100e80
#endif
MC0_CTL_240: .double 0x00000e0000000e00
//00000000000000000000111000000000 dll_ctrl_reg_1_1(RW) 00000000000000000000111000000000 dll_ctrl_reg_1_0(RW) 
MC0_CTL_250: .double 0x00000e0000000e00
MC0_CTL_260: .double 0x0000100000000e00
// 0x00000e0000000e00 mxl modified 
MC0_CTL_270: .double 0x00000e0000000e00
MC0_CTL_280: .double 0x0000000000000e00
MC0_CTL_290: .double 0x0000000000000000
MC0_CTL_2a0: .double 0x0000000000000000
MC0_CTL_2b0: .double 0x0000000000000000
MC0_CTL_2c0: .double 0x0000000000000000
MC0_CTL_2d0: .double 0xf3004946003c099c
// 0xf300494603c0019d   mxl  modified
//11110100000000000011101100100111 phy_ctrl_reg_0_0(RD) 000000_00000000000000000110011101 pad_ctrl_reg_0(RW) 
MC0_CTL_2e0: .double 0xf3004946f3004946
MC0_CTL_2f0: .double 0xf3004946f3004946
MC0_CTL_300: .double 0xf3004946f3004946
MC0_CTL_310: .double 0xf3004946f3004946
//MC0_CTL_2d0: .double 0xf3005a470000019d
////11110100000000000011101100100111 phy_ctrl_reg_0_0(RD) 000000_00000000000000000110011101 pad_ctrl_reg_0(RW) 
//MC0_CTL_2e0: .double 0xf3005a47f3005a47
//MC0_CTL_2f0: .double 0xf3005a47f3005a47
//MC0_CTL_300: .double 0xf3005a47f3005a47
//MC0_CTL_310: .double 0xf3005a47f3005a47
MC0_CTL_320: .double 0x0680000006800000
//0x07a0000007a00000 mxl modified
MC0_CTL_330: .double 0x0680000006800000
//0x07a0000007a00000 mxl modified
MC0_CTL_340: .double 0x0680000006800000
//0x07a0000007a00000 mxl modified
MC0_CTL_350: .double 0x0680000006800000
//0x07a0000007a00000 mxl modified
MC0_CTL_360: .double 0x0800c00506800000
//0x0800c00507a00000 mxl modified

// 
//00000000000000001100000000000101 phy_ctrl_reg_2(RW) 00000111110000000000001100000001 phy_ctrl_reg_1_8(RD) 
MC0_CTL_370: .double 0x0000000000000000
MC0_CTL_380: .double 0x0000000000000000
MC0_CTL_390: .double 0x0000000000000000
MC0_CTL_3a0: .double 0x0000000000000000
MC0_CTL_3b0: .double 0x0000000000000000
MC0_CTL_3c0: .double 0x0000000000000000
MC0_CTL_3d0: .double 0x0000000000000000
MC0_CTL_3e0: .double 0x0000000000000000
MC0_CTL_3f0: .double 0x0000000000000000
MC0_CTL_400: .double 0x0000000000000000
MC0_CTL_410: .double 0x0000000000000000
MC0_CTL_420: .double 0x0000000000000000
MC0_CTL_430: .double 0x0000000000000000
MC0_CTL_440: .double 0x0000000000000000
MC0_CTL_450: .double 0x0000000000000000
MC0_CTL_460: .double 0x0000000000000000
MC0_CTL_470: .double 0x0000000000000000
MC0_CTL_480: .double 0x0000000000000000
MC0_CTL_490: .double 0x0000000000000000
MC0_CTL_4a0: .double 0x0000000000000000
MC0_CTL_4b0: .double 0x0000000000000000
MC0_CTL_4c0: .double 0x0000000000000000
MC0_CTL_4d0: .double 0x0000000000000000
MC0_CTL_4e0: .double 0x0000000000000000
MC0_CTL_4f0: .double 0x0000000000000000
MC0_CTL_500: .double 0x0000000000000000
MC0_CTL_510: .double 0x0000000000000000
MC0_CTL_520: .double 0x0000000000000000
MC0_CTL_530: .double 0x0000000000000000
MC0_CTL_540: .double 0x0000000000000000
MC0_CTL_550: .double 0x0000000000000000
MC0_CTL_560: .double 0x0000000000000000
MC0_CTL_570: .double 0x0000000000000000
MC0_CTL_580: .double 0x0000000000000000
MC0_CTL_590: .double 0x0000000000000000
MC0_CTL_5a0: .double 0x0000000000000000
MC0_CTL_5b0: .double 0x0000000000000000
MC0_CTL_5c0: .double 0x0000000000000000
MC0_CTL_5d0: .double 0x0000000000000000
MC0_CTL_5e0: .double 0x0000000000000000
MC0_CTL_5f0: .double 0x0000000000000000
MC0_CTL_600: .double 0x0000000000000000
MC0_CTL_610: .double 0x0000000000000000
MC0_CTL_620: .double 0x0000000000000000
MC0_CTL_630: .double 0x0000000000000000
MC0_CTL_640: .double 0x0000000000000000
MC0_CTL_650: .double 0x0000000000000000
MC0_CTL_660: .double 0x0000000000000000
MC0_CTL_670: .double 0x0000000000000000
MC0_CTL_680: .double 0x0000000000000000
MC0_CTL_690: .double 0x0000000000000000
MC0_CTL_6a0: .double 0x0000000000000000
MC0_CTL_6b0: .double 0x0000000000000000
MC0_CTL_6c0: .double 0x0000000000000000
MC0_CTL_6d0: .double 0x0000000000000000
MC0_CTL_6e0: .double 0x0000000000000000
MC0_CTL_6f0: .double 0x0000000000000000
MC0_CTL_700: .double 0x0000000000000000
MC0_CTL_710: .double 0x0000000000000000
MC0_CTL_720: .double 0x0000000000000000
MC0_CTL_730: .double 0x0000000000000000
MC0_CTL_740: .double 0x0100000000000000
//MC0_CTL_750: .double 0x0100000101020101
MC0_CTL_750: .double 0x0101000101020101
//000000_01 wrlvl_cs(RW) 000000_00 sw_leveling_mode(RW) 000000_00 rdlvl_cs(RW) 000000_01 axi2_w_priority(RW) 000000_01 axi2_r_priority(RW) 000000_10 axi2_port_ordering(RW) 000000_01 axi1_w_priority(RW) 000000_01 axi1_r_priority(RW) 
MC0_CTL_760: .double 0x0303030000020001
//0000_0011 axi0_priority2_relative_priority(RW) 0000_0011 axi0_priority1_relative_priority(RW) 0000_0011 axi0_priority0_relative_priority(RW) 0000_0000 address_mirroring(RW) 00000_000 tdfi_dram_clk_disable(RW) 00000_010 bstlen(RW) 000000_00 zq_req(WR) 000000_01 zq_on_sref_exit(RW) 
MC0_CTL_770: .double 0x0101010202020203
//0000_0001 axi2_priority2_relative_priority(RW) 0000_0001 axi2_priority1_relative_priority(RW) 0000_0001 axi2_priority0_relative_priority(RW) 0000_0010 axi1_priority3_relative_priority(RW) 0000_0010 axi1_priority2_relative_priority(RW) 0000_0010 axi1_priority1_relative_priority(RW) 0000_0010 axi1_priority0_relative_priority(RW) 0000_0011 axi0_priority3_relative_priority(RW) 
MC0_CTL_780: .double 0x0102020400040001
//0000_0001 tdfi_dram_clk_enable(RW) 0000_0010 tdfi_ctrl_delay(RW) 0000_0010 rdlvl_gate_dq_zero_count(RW) 0000_0100 rdlvl_dq_zero_count(RW) 0000_0000 lowpower_refresh_enable(RW) 0000_0110 dram_class(RW) 0000_1100 burst_on_fly_bit(RW) 0000_0001 axi2_priority3_relative_priority(RW) 
MC0_CTL_790: .double 0x281900000f000303
//00_101000 wlmrd(RW) 00_011001 wldqsen(RW) 000_00000 lowpower_control(RW) 000_00000 lowpower_auto_enable(RW) 0000_1111 zqcs_chip(RW) 0000_0000 wrr_param_value_err(RD) 0000_0011 tdfi_wrlvl_dll(RW) 0000_0011 tdfi_rdlvl_dll(RW) 
MC0_CTL_7a0: .double 0x00000000000000ff
MC0_CTL_7b0: .double 0x0000000000000000
MC0_CTL_7c0: .double 0x0000000000000000
MC0_CTL_7d0: .double 0x0000000000000000
MC0_CTL_7e0: .double 0x0000000000000000
//00000000 rdlvl_gate_delay_2(RD) 00000000 rdlvl_gate_delay_1(RD) 00000000 rdlvl_gate_delay_0(RD) 00000000 rdlvl_gate_clk_adjust_8(RW) 00000000 rdlvl_gate_clk_adjust_7(RW) 00000000 rdlvl_gate_clk_adjust_6(RW) 00000000 rdlvl_gate_clk_adjust_5(RW) 00000000 rdlvl_gate_clk_adjust_4(RW) 
MC0_CTL_7f0: .double 0xff08000000000000
//11111111 rdlvl_max_delay(RW) 00001000 rdlvl_gate_max_delay(RW) 00000000 rdlvl_gate_delay_8(RD) 00000000 rdlvl_gate_delay_7(RD) 00000000 rdlvl_gate_delay_6(RD) 00000000 rdlvl_gate_delay_5(RD) 00000000 rdlvl_gate_delay_4(RD) 00000000 rdlvl_gate_delay_3(RD) 
MC0_CTL_800: .double 0x0000000000000000
//00000000 rdlvl_midpoint_delay_7(RD) 00000000 rdlvl_midpoint_delay_6(RD) 00000000 rdlvl_midpoint_delay_5(RD) 00000000 rdlvl_midpoint_delay_4(RD) 00000000 rdlvl_midpoint_delay_3(RD) 00000000 rdlvl_midpoint_delay_2(RD) 00000000 rdlvl_midpoint_delay_1(RD) 00000000 rdlvl_midpoint_delay_0(RD) 
MC0_CTL_810: .double 0x000000000000000e
//00000000 rdlvl_offset_delay_6(RW) 00000000 rdlvl_offset_delay_5(RW) 00000000 rdlvl_offset_delay_4(RW) 00000000 rdlvl_offset_delay_3(RW) 00000000 rdlvl_offset_delay_2(RW) 00000000 rdlvl_offset_delay_1(RW) 00000000 rdlvl_offset_delay_0(RW) 00000000 rdlvl_midpoint_delay_8(RD) 
MC0_CTL_820: .double 0x0420000c20400000
//00000100 tdfi_wrlvl_resplat(RW) 00000000 tdfi_wrlvl_resp(RD) 00000000 tdfi_rdlvl_rr(RW) 00001100 tdfi_rdlvl_resplat(RW) 00000000 tdfi_rdlvl_resp(RD) 01000000 refresh_per_zq(RW) 00000000 rdlvl_offset_delay_8(RW) 00000000 rdlvl_offset_delay_7(RW) 
//MC0_CTL_830: .double 0x0000000000000c0a
MC0_CTL_830: .double 0x0f29292929290c0a
//0x1313131313130c0a mxl modified
//00000000 wrlvl_delay_5(RD) 00000000 wrlvl_delay_4(RD) 00000000 wrlvl_delay_3(RD) 00000000 wrlvl_delay_2(RD) 00000000 wrlvl_delay_1(RD) 00000000 wrlvl_delay_0(RD) 00000010 tmod(RW) 00001010 tdfi_wrlvl_ww(RW) 
MC0_CTL_840: .double 0x0000640064001515
// 0x0000640064001313 mxl modified
//00000000000000_0001100100 axi1_priority_relax(RW) 000000_0001100100 axi0_priority_relax(RW) 00000000 wrlvl_delay_8(RD) 00000000 wrlvl_delay_7(RD) 00000000 wrlvl_delay_6(RD) 
MC0_CTL_850: .double 0x0000000000000064
MC0_CTL_860: .double 0x0200004000000000
MC0_CTL_870: .double 0x0000000000000046
//0x0044004400440046  mxl modified
//0_000000000000010 emrs1_data_3(RW) 0_000000000000010 emrs1_data_2(RW) 0_000000000000010 emrs1_data_1(RW) 0_000000000000010 emrs1_data_0(RW) 
MC0_CTL_880: .double 0x0000000000000000
MC0_CTL_890: .double 0x0a520a520a520a5a
//0x0a520a520a520a52  mxl modified 
//0_000010000010000 mrs_data_3(RW) 0_000010000010000 mrs_data_2(RW) 0_000010000010000 mrs_data_1(RW) 0_000010000010000 mrs_data_0(RW) 
MC0_CTL_8a0: .double 0x00000000001c001c
MC0_CTL_8b0: .double 0x0000000000000000
MC0_CTL_8c0: .double 0x0004000000000000
MC0_CTL_8d0: .double 0x00000000c8000000
MC0_CTL_8e0: .double 0x0000000000000050
//MC0_CTL_8f0: .double 0x0000000020202080
MC0_CTL_8f0: .double 0x0000000010141380 //clk skew cleared
//0x000000000a100080  mxl modified
//MC0_CTL_8f0: .double 0x000000002b352180
//MC0_CTL_8f0: .double 0x0000000040404080
//0000000000000000000000000111100_000000000000000000000000001111000 dll_ctrl_reg_2(RW) 
MC0_CTL_900: .double 0x0000000000000000
MC0_CTL_910: .double 0x0000000000000000
MC0_CTL_920: .double 0x0000000000000000
MC0_CTL_930: .double 0x0000000000000000
MC0_CTL_940: .double 0x0300000000050500
MC0_CTL_950: .double 0x0000000000000a03
MC0_CTL_960: .double 0x0604000100000000
//000_00101 rdlat_adj(RW) 0000_0100 wrlat_adj(RW) 0000000_0 swlvl_start(WR) 0000000_0 swlvl_load(WR) 0000000_0 swlvl_exit(WR) 000000_000000000000000000 int_status(RD) 
MC0_CTL_970: .double 0x000000000003e805
MC0_CTL_980: .double 0x0001000001000000
//0000000_0 zq_in_progress(RD) 0000000_1 zqcs_rotate(RW) 0000000_0 wrlvl_reg_en(RW) 0000000_0 wrlvl_en(RW) 0000000_1 resync_dll_per_aref_en(RW) 0000000_0 resync_dll(WR) 0000000_0 rdlvl_reg_en(RW) 0000000_0 rdlvl_gate_reg_en(RW) 
MC0_CTL_990: .double 0x0001020202000100
//00000_000 w2w_samecs_dly(RW) 00000_001 w2w_diffcs_dly(RW) 00000_010 tbst_int_interval(RW) 00000_010 r2w_samecs_dly(RW) 00000_010 r2w_diffcs_dly(RW) 00000_000 r2r_samecs_dly(RW) 00000_001 r2r_diffcs_dly(RW) 00000_000 axi_aligned_strobe_disable(RW) 
MC0_CTL_9a0: .double 0x0707030200060100
//00000111 tdfi_wrlvl_load(RW) 00000111 tdfi_rdlvl_load(RW) 000_00011 tckesr(RW) 000_00010 tccd(RW) 000_00000 add_odt_clk_difftype_diffcs(RW) 0000_0110 trp_ab(RW) 0000_0001 add_odt_clk_sametype_diffcs(RW) 0000_0000 add_odt_clk_difftype_samecs(RW) 
MC0_CTL_9b0: .double 0x02000100000a000f
//0000_001000000000 zqinit(RW) 0000_000100000000 zqcl(RW) 000000_0000001010 tdfi_wrlvl_ww(RW) 000000_0000001111 tdfi_rdlvl_rr(RW) 
MC0_CTL_9c0: .double 0x0a520c2d0c2d0c2d
//MC0_CTL_9c0: .double 0x04100c2d0c2d0c2d
//0_000101001010010 mr0_data_0(RW) 00_00110000101101 tdfi_phyupd_type3(RW) 00_00110000101101 tdfi_phyupd_type2(RW) 00_00110000101101 tdfi_phyupd_type1(RW) 
//MC0_CTL_9d0: .double 0x0004041004100410
MC0_CTL_9d0: .double 0x00440a520a520a52
//0_000000000000100 mr1_data_0(RW) 0_000101001010010 mr0_data_3(RW) 0_000101001010010 mr0_data_2(RW) 0_000101001010010 mr0_data_1(RW) 
MC0_CTL_9e0: .double 0x0000004400440044
//0_000000000000000 mr2_data_0(RW) 0_000000000000100 mr1_data_3(RW) 0_000000000000100 mr1_data_2(RW) 0_000000000000100 mr1_data_1(RW) 
MC0_CTL_9f0: .double 0x0000000000000000
//0_000000000000000 mr3_data_0(RW) 0_000000000000000 mr2_data_3(RW) 0_000000000000000 mr2_data_2(RW) 0_000000000000000 mr2_data_1(RW) 
MC0_CTL_a00: .double 0x00ff000000000000
//0000000011111111 dfi_wrlvl_max_delay(RW) 0_000000000000000 mr3_data_3(RW) 0_000000000000000 mr3_data_2(RW) 0_000000000000000 mr3_data_1(RW) 
MC0_CTL_a10: .double 0x0000000000000000
//0000000000000000 rdlvl_begin_delay_3(RD) 0000000000000000 rdlvl_begin_delay_2(RD) 0000000000000000 rdlvl_begin_delay_1(RD) 0000000000000000 rdlvl_begin_delay_0(RD) 
MC0_CTL_a20: .double 0x0000000000000000
//0000000000000000 rdlvl_begin_delay_7(RD) 0000000000000000 rdlvl_begin_delay_6(RD) 0000000000000000 rdlvl_begin_delay_5(RD) 0000000000000000 rdlvl_begin_delay_4(RD) 
MC0_CTL_a30: .double 0x0e0e0e0e0e0e0000
//0000111000001110 rdlvl_delay_2(RW) 0000111000001110 rdlvl_delay_1(RW) 0000111000001110 rdlvl_delay_0(RW) 0000000000000000 rdlvl_begin_delay_8(RD) 
MC0_CTL_a40: .double 0x0e0e0e0e0e0e0e0e
//0000111000001110 rdlvl_delay_6(RW) 0000111000001110 rdlvl_delay_5(RW) 0000111000001110 rdlvl_delay_4(RW) 0000111000001110 rdlvl_delay_3(RW) 
MC0_CTL_a50: .double 0x000000000e0e0e0e
//0000000000000000 rdlvl_end_delay_1(RD) 0000000000000000 rdlvl_end_delay_0(RD) 0000111000001110 rdlvl_delay_8(RW) 0000111000001110 rdlvl_delay_7(RW) 
MC0_CTL_a60: .double 0x0000000000000000
//0000000000000000 rdlvl_end_delay_5(RD) 0000000000000000 rdlvl_end_delay_4(RD) 0000000000000000 rdlvl_end_delay_3(RD) 0000000000000000 rdlvl_end_delay_2(RD) 
MC0_CTL_a70: .double 0x0000000000000000
//0000000000000000 rdlvl_gate_delay_0(RW+) 0000000000000000 rdlvl_end_delay_8(RD) 0000000000000000 rdlvl_end_delay_7(RD) 0000000000000000 rdlvl_end_delay_6(RD) 
MC0_CTL_a80: .double 0x0000000000000000
//0000000000000000 rdlvl_gate_delay_4(RW+) 0000000000000000 rdlvl_gate_delay_3(RW+) 0000000000000000 rdlvl_gate_delay_2(RW+) 0000000000000000 rdlvl_gate_delay_1(RW+) 
MC0_CTL_a90: .double 0x0000000000000000
//0000000000000000 rdlvl_gate_delay_8(RW+) 0000000000000000 rdlvl_gate_delay_7(RW+) 0000000000000000 rdlvl_gate_delay_6(RW+) 0000000000000000 rdlvl_gate_delay_5(RW+) 
MC0_CTL_aa0: .double 0x0000ffff00000000
//0000000000000000 rdlvl_midpoint_delay_0(RD) 1111111111111111 rdlvl_max_delay(RW) 0000000000000000 rdlvl_gate_refresh_interval(RW) 0000000000010000 rdlvl_gate_max_delay(RW) 
MC0_CTL_ab0: .double 0x0000000000000000
//0000000000000000 rdlvl_midpoint_delay_4(RD) 0000000000000000 rdlvl_midpoint_delay_3(RD) 0000000000000000 rdlvl_midpoint_delay_2(RD) 0000000000000000 rdlvl_midpoint_delay_1(RD) 
MC0_CTL_ac0: .double 0x0000000000000000
//0000000000000000 rdlvl_midpoint_delay_8(RD) 0000000000000000 rdlvl_midpoint_delay_7(RD) 0000000000000000 rdlvl_midpoint_delay_6(RD) 0000000000000000 rdlvl_midpoint_delay_5(RD) 
MC0_CTL_ad0: .double 0x0000000000000000
//0000000000000000 rdlvl_offset_delay_3(RW) 0000000000000000 rdlvl_offset_delay_2(RW) 0000000000000000 rdlvl_offset_delay_1(RW) 0000000000000000 rdlvl_offset_delay_0(RW) 
MC0_CTL_ae0: .double 0x0000000000000000
//0000000000000000 rdlvl_offset_delay_7(RW) 0000000000000000 rdlvl_offset_delay_6(RW) 0000000000000000 rdlvl_offset_delay_5(RW) 0000000000000000 rdlvl_offset_delay_4(RW) 
MC0_CTL_af0: .double 0x0020002000000000
//0000000000000000 wrlvl_delay_1(RW+) 0000000000000000 wrlvl_delay_0(RW+) 0000000000000000 rdlvl_refresh_interval(RW) 0000000000000000 rdlvl_offset_delay_8(RW) 
MC0_CTL_b00: .double 0x0020002000200020
//0000000000000000 wrlvl_delay_5(RW+) 0000000000000000 wrlvl_delay_4(RW+) 0000000000000000 wrlvl_delay_3(RW+) 0000000000000000 wrlvl_delay_2(RW+) 
MC0_CTL_b10: .double 0x0000000000200020
//0000000000000000 wrlvl_refresh_interval(RW) 0000000000000000 wrlvl_delay_8(RW+) 0000000000000000 wrlvl_delay_7(RW+) 0000000000000000 wrlvl_delay_6(RW+) 
MC0_CTL_b20: .double 0x00000c2d00000c2d
//00000000000000000000110000101101 tdfi_rdlvl_resp(RW) 00000000000000000000110000101101 tdfi_rdlvl_max(RW) 
MC0_CTL_b30: .double 0x00000c2d00000000
//00000000000000000000110000101101 tdfi_wrlvl_resp(RW) 00000000000000000000000000000000 tdfi_wrlvl_max(RW) 
MC0_CTL_start_DATA_LO: .word  0x00000000
//0000000_1 rw_same_en 0000000_0 reg_dimm_enable 0000000_0 reduc 0000000_0 pwrup_srefresh_exit
MC0_CTL_start_DATA_HI: .word  0x01010100
//0000000_1 swap_port_rw_same_en 0000000_1 swap_en 0000000_0 start 0000000_0 srefresh


#else

/*  ddr2_reg_data_mc0 */

	.data
    .align 5
    .global ddr2_reg_data
    
ddr2_reg_data:
	
MC0_CTL_000: .double 0x0000010000000100
//000000000000000_0 concurrentap(RW) 0000000_1 bank_split_en(RW) 0000000_0 auto_refresh_mode(RW) 0000000_0 arefresh(WR) 0000000_0 ap(RW) 0000000_1 addr_cmp_en(RW) 0000000_1 active_aging(RW) 
MC0_CTL_010: .double 0x0001000100010000
//0000000_0 fwc(WR) 0000000_0 fast_write(RW) 0000000_1 enable_quick_srefresh(RW) 0000000_1 eight_bank_mode(RW) 0000000_0 ecc_disable_w_uc_err(RW) 0000000_1 dqs_n_en(RW) 0000000_0 dll_bypass_mode(RW) 0000000_0 dlllockreg(RD) 
MC0_CTL_020: .double 0x0100010101000000
// 0x0100010101000000 mxl modified 
//0000000_1 priority_en(RW) 0000000_0 power_down(RW) 0000000_1 placement_en(RW) 0000000_1 odt_add_turn_clk_en(RW) 0000000_0 no_cmd_init(RW) 0000000_0 intrptwritea(RW) 0000000_0 intrptreada(RW) 0000000_0 intrptapburst(RW) 
MC0_CTL_030: .double 0x0000000001000000
//0000000_1 swap_port_rw_same_en(RW) 0000000_1 swap_en(RW) 0000000_0 start(RW) 0000000_0 srefresh(RW+) 0000000_1 rw_same_en(RW) 0000000_0 reg_dimm_enable(RW) 0000000_0 reduc(RW) 0000000_0 pwrup_srefresh_exit(RW) 
MC0_CTL_040: .double 0x0100010100010101
//000000_01 rtt_0(RW) 000000_11 ctrl_raw(RW) 000000_01 axi0_w_priority(RW) 000000_01 axi0_r_priority(RW) 0000000_0 write_modereg(WR) 0000000_1 writeinterp(RW) 0000000_1 tref_enable(RW) 0000000_1 tras_lockout(RW) 
MC0_CTL_050: .double 0x0000000404050100
//00000_000 q_fullness(RW) 00000_000 port_data_error_type(RD) 000_00000 out_of_range_type(RD) 00000_000 max_cs_reg(RD) 00000_100 column_size(RW) 0000_0101 caslat(RW) 00000_010 addr_pins(RW) 00000000
MC0_CTL_060: .double 0x0a04040603040003
//0000_1010 aprebit(RW) 0000_0101 wrlat(RW) 0000_0100 twtr(RW) 000_00110 twr_int(RW) 00000_011 trtp(RW) 00000_100 trrd(RW) 0000000000000_011 tcke(RW) 
MC0_CTL_070: .double 0x0f0e020000010a0a
//0000_0000 max_row_reg(RD) 0000_0000 max_col_reg(RD) 0000_0000 initaref(RW) 00000000000000000000_1111 cs_map(RW) 000_01010 caslat_lin_gate(RW) 000_01010 caslat_lin(RW) 
MC0_CTL_080: .double 0x0804020108040200
//0000_0001 odt_wr_map_cs3(RW) 0000_0010 odt_wr_map_cs2(RW) 0000_0100 odt_wr_map_cs1(RW) 0000_1000 odt_wr_map_cs0(RW) 0000_0001 odt_rd_map_cs3(RW) 0000_0010 odt_rd_map_cs2(RW) 0000_0100 odt_rd_map_cs1(RW) 0000_1000 odt_rd_map_cs0(RW) 
MC0_CTL_090: .double 0x0000050b00000000
//000_00000 ocd_adjust_pup_cs_0(RW) 000_00000 ocd_adjust_pdn_cs_0(RW) 0000_0101 trp(RW) 000_01011 tdal(RW) 000000000000_0000 port_cmd_error_type(RD) 0000000000000000
MC0_CTL_0a0: .double 0x0000003f3f140612
//MC0_CTL_0a0: .double 0x0000003f3f14021b
//00000000000000000000000000_111111 command_age_count(RW) 00_111111 age_count(RW) 000_10100 trc(RW) 000_00110 tmrd(RW) 000_10010 tfaw(RW) 
MC0_CTL_0b0: .double 0x0000000000000000
MC0_CTL_0c0: .double 0x00002c050f000000
//000000000000000000101100 trfc(RW) 00000101 trcd_int(RW) 00001111 tras_min(RW) 00000000 out_of_range_length(RD) 00000000 ecc_u_synd(RD) 00000000 ecc_c_synd(RD) 
MC0_CTL_0d0: .double 0x0000000000000000
MC0_CTL_0e0: .double 0x0000000000000000
MC0_CTL_0f0: .double 0x0000000000000000
MC0_CTL_100: .double 0x0000000000000000
MC0_CTL_110: .double 0x0000000000001c2d
//0_000000000000000 emrs2_data_1(RW) 0_000000000000000 emrs2_data_0(RW) 000000000000000000_00110000101101 tref(RW) 
MC0_CTL_120: .double 0xffff000000000000
//0000000000011100 axi0_en_size_lt_width_instr(RW) 00000000000000000_000000000000000 emrs2_data_3(RW) 0_000000000000000 emrs2_data_2(RW) 
MC0_CTL_130: .double 0x0d56000302000000
//0110110101010110 tras_max(RW) 0000000000000011 tpdex(RW) 0000001000000000 tdll(RW) 0000000000000000 tcpd(RW) 
MC0_CTL_140: .double 0x0000204002000030
//0000000000000000 xor_check_bits(RW) 0000000000000000 version(RD) 0000001000000000 txsr(RW) 0000000000110000 txsnr(RW) 
MC0_CTL_150: .double 0x0000000011000004
//000_0000000000000000000000000000000000000 ecc_c_addr(RD) 000000000000000000011011 tinit(RW) 
MC0_CTL_160: .double 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 ecc_u_addr(RD) 
MC0_CTL_170: .double 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 out_of_range_addr(RD) 
MC0_CTL_180: .double 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 port_cmd_error_addr(RD) 
MC0_CTL_190: .double 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000000 ecc_c_data(RD) 
MC0_CTL_1a0: .double 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000000 ecc_u_data(RD) 
MC0_CTL_1b0: .double 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000_000 cke_delay(RW) 
MC0_CTL_1c0: .double 0x0000000000000000
MC0_CTL_1d0: .double 0x0203070400000101
//0000_0010 tdfi_phy_wrlat_base(RW) 0000_0000 tdfi_phy_wrlat(RD) 000_00111 tdfi_phy_rdlat(RW) 0000_0000 tdfi_ctrlupd_min(RD) 000000000000_0000 dram_clk_disable(RW) 0000000_1 odt_alt_en(RW) 0000000_1 drive_dq_dqs(RW) 
MC0_CTL_1e0: .double 0x0c2d0c2d0c2d0205
//00_00000000000000 tdfi_phyupd_type0(RD) 00_00000000000000 tdfi_phyupd_resp(RD) 00_00000000000000 tdfi_ctrlupd_max(RD) 000_00000 tdfi_rddata_en_base(RW) 000_00000 tdfi_rddata_en(RD) 
#ifdef DDR_DEBUG
//MC0_CTL_1f0: .double 0x00120e8000000000
//MC0_CTL_200: .double 0x00160e8000160e80
//MC0_CTL_210: .double 0x00100e8000130e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC0_CTL_220: .double 0x00130e8000010e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC0_CTL_230: .double 0x00200e8000100e80

#if 1
MC0_CTL_1f0: .double 0x000a0e8000000000
MC0_CTL_200: .double 0x000a0e80000a0e80
MC0_CTL_210: .double 0x000a0e80000a0e80
//00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
MC0_CTL_220: .double 0x000a0e80000a0e80
//00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
MC0_CTL_230: .double 0x000a0e80000a0e80
#else
MC0_CTL_1f0: .double 0x00300e8000000000
MC0_CTL_200: .double 0x00300e8000300e80
MC0_CTL_210: .double 0x00300e8000300e80
//00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
MC0_CTL_220: .double 0x00300e8000300e80
//00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
MC0_CTL_230: .double 0x00300e8000300e80

#endif

//#debug write
//MC0_CTL_1f0: .double 0x00000e8000000000
//MC0_CTL_200: .double 0x00000e8000000e80
//MC0_CTL_210: .double 0x00000e8000000e80
//00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC0_CTL_220: .double 0x00000e8000000e80
//00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC0_CTL_230: .double 0x00200e8000000e80

//#debug read
//MC0_CTL_1f0: .double 0x00120e8000000000
//MC0_CTL_200: .double 0x00160e8000160e80
//MC0_CTL_210: .double 0x00100e8000140e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC0_CTL_220: .double 0x00100e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC0_CTL_230: .double 0x00200e8000140e80

#else
MC0_CTL_1f0: .double 0x00200e8000000000
MC0_CTL_200: .double 0x00200e8000200e80
MC0_CTL_210: .double 0x00200e8000200e80
//00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
MC0_CTL_220: .double 0x00200e8000200e80
//00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
MC0_CTL_230: .double 0x00200e8000200e80

//MC0_CTL_1f0: .double 0x00130e8000000000
//MC0_CTL_200: .double 0x00130e8000130e80
//MC0_CTL_210: .double 0x00130e8000130e80
////00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC0_CTL_220: .double 0x00100e8000130e80
////00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC0_CTL_230: .double 0x00150e8000120e80

//MC0_CTL_1f0: .double 0x00230eff00000000
//MC0_CTL_200: .double 0x00230eff00230eff
//MC0_CTL_210: .double 0x00230eff00230eff
////00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC0_CTL_220: .double 0x00200eff00230eff
////00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC0_CTL_230: .double 0x00150eff00200eff

//MC0_CTL_1f0: .double 0x00120e8000000000
//MC0_CTL_200: .double 0x00160e8000160e80
//MC0_CTL_210: .double 0x00100e8000180e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC0_CTL_220: .double 0x000c0e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC0_CTL_230: .double 0x00200e8000100e80

//150M MC0
//MC0_CTL_1f0: .double 0x001a0e8000000000
//MC0_CTL_200: .double 0x00120e8000120e80
//MC0_CTL_210: .double 0x00100e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC0_CTL_220: .double 0x00100e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC0_CTL_230: .double 0x00200e8000120e80

//150M MC0
//MC0_CTL_1f0: .double 0x00160e8000000000
//MC0_CTL_200: .double 0x00100e8000100e80
//MC0_CTL_210: .double 0x00100e8000160e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC0_CTL_220: .double 0x00160e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC0_CTL_230: .double 0x00200e8000100e80
#endif
MC0_CTL_240: .double 0x00000e0000000e00
//00000000000000000000111000000000 dll_ctrl_reg_1_1(RW) 00000000000000000000111000000000 dll_ctrl_reg_1_0(RW) 
MC0_CTL_250: .double 0x00000e0000000e00
MC0_CTL_260: .double 0x00000e0000000e00
// 0x00000e0000000e00 mxl modified 
MC0_CTL_270: .double 0x00000e0000000e00
MC0_CTL_280: .double 0x0000000000000e00
MC0_CTL_290: .double 0x0000000000000000
MC0_CTL_2a0: .double 0x0000000000000000
MC0_CTL_2b0: .double 0x0000000000000000
MC0_CTL_2c0: .double 0x0000000000000000
MC0_CTL_2d0: .double 0xf300494603c0019c
// 0xf300494603c0019d	mxl  modified
//11110100000000000011101100100111 phy_ctrl_reg_0_0(RD) 000000_00000000000000000110011101 pad_ctrl_reg_0(RW) 
MC0_CTL_2e0: .double 0xf3004946f3004946
MC0_CTL_2f0: .double 0xf3004946f3004946
MC0_CTL_300: .double 0xf3004946f3004946
MC0_CTL_310: .double 0xf3004946f3004946
//MC0_CTL_2d0: .double 0xf3005a470000019d
////11110100000000000011101100100111 phy_ctrl_reg_0_0(RD) 000000_00000000000000000110011101 pad_ctrl_reg_0(RW) 
//MC0_CTL_2e0: .double 0xf3005a47f3005a47
//MC0_CTL_2f0: .double 0xf3005a47f3005a47
//MC0_CTL_300: .double 0xf3005a47f3005a47
//MC0_CTL_310: .double 0xf3005a47f3005a47
MC0_CTL_320: .double 0x07a0000007a00000
//0x07a0000007a00000 mxl modified
MC0_CTL_330: .double 0x07a0000007a00000
//0x07a0000007a00000 mxl modified
MC0_CTL_340: .double 0x07a0000007a00000
//0x07a0000007a00000 mxl modified
MC0_CTL_350: .double 0x07a0000007a00000
//0x07a0000007a00000 mxl modified
MC0_CTL_360: .double 0x0800c00507a00000
//0x0800c00507a00000 mxl modified

// 
//00000000000000001100000000000101 phy_ctrl_reg_2(RW) 00000111110000000000001100000001 phy_ctrl_reg_1_8(RD) 
MC0_CTL_370: .double 0x0000000000000000
MC0_CTL_380: .double 0x0000000000000000
MC0_CTL_390: .double 0x0000000000000000
MC0_CTL_3a0: .double 0x0000000000000000
MC0_CTL_3b0: .double 0x0000000000000000
MC0_CTL_3c0: .double 0x0000000000000000
MC0_CTL_3d0: .double 0x0000000000000000
MC0_CTL_3e0: .double 0x0000000000000000
MC0_CTL_3f0: .double 0x0000000000000000
MC0_CTL_400: .double 0x0000000000000000
MC0_CTL_410: .double 0x0000000000000000
MC0_CTL_420: .double 0x0000000000000000
MC0_CTL_430: .double 0x0000000000000000
MC0_CTL_440: .double 0x0000000000000000
MC0_CTL_450: .double 0x0000000000000000
MC0_CTL_460: .double 0x0000000000000000
MC0_CTL_470: .double 0x0000000000000000
MC0_CTL_480: .double 0x0000000000000000
MC0_CTL_490: .double 0x0000000000000000
MC0_CTL_4a0: .double 0x0000000000000000
MC0_CTL_4b0: .double 0x0000000000000000
MC0_CTL_4c0: .double 0x0000000000000000
MC0_CTL_4d0: .double 0x0000000000000000
MC0_CTL_4e0: .double 0x0000000000000000
MC0_CTL_4f0: .double 0x0000000000000000
MC0_CTL_500: .double 0x0000000000000000
MC0_CTL_510: .double 0x0000000000000000
MC0_CTL_520: .double 0x0000000000000000
MC0_CTL_530: .double 0x0000000000000000
MC0_CTL_540: .double 0x0000000000000000
MC0_CTL_550: .double 0x0000000000000000
MC0_CTL_560: .double 0x0000000000000000
MC0_CTL_570: .double 0x0000000000000000
MC0_CTL_580: .double 0x0000000000000000
MC0_CTL_590: .double 0x0000000000000000
MC0_CTL_5a0: .double 0x0000000000000000
MC0_CTL_5b0: .double 0x0000000000000000
MC0_CTL_5c0: .double 0x0000000000000000
MC0_CTL_5d0: .double 0x0000000000000000
MC0_CTL_5e0: .double 0x0000000000000000
MC0_CTL_5f0: .double 0x0000000000000000
MC0_CTL_600: .double 0x0000000000000000
MC0_CTL_610: .double 0x0000000000000000
MC0_CTL_620: .double 0x0000000000000000
MC0_CTL_630: .double 0x0000000000000000
MC0_CTL_640: .double 0x0000000000000000
MC0_CTL_650: .double 0x0000000000000000
MC0_CTL_660: .double 0x0000000000000000
MC0_CTL_670: .double 0x0000000000000000
MC0_CTL_680: .double 0x0000000000000000
MC0_CTL_690: .double 0x0000000000000000
MC0_CTL_6a0: .double 0x0000000000000000
MC0_CTL_6b0: .double 0x0000000000000000
MC0_CTL_6c0: .double 0x0000000000000000
MC0_CTL_6d0: .double 0x0000000000000000
MC0_CTL_6e0: .double 0x0000000000000000
MC0_CTL_6f0: .double 0x0000000000000000
MC0_CTL_700: .double 0x0000000000000000
MC0_CTL_710: .double 0x0000000000000000
MC0_CTL_720: .double 0x0000000000000000
MC0_CTL_730: .double 0x0000000000000000
MC0_CTL_740: .double 0x0100000000000000
//MC0_CTL_750: .double 0x0100000101020101
MC0_CTL_750: .double 0x0101000101020101
//000000_01 wrlvl_cs(RW) 000000_00 sw_leveling_mode(RW) 000000_00 rdlvl_cs(RW) 000000_01 axi2_w_priority(RW) 000000_01 axi2_r_priority(RW) 000000_10 axi2_port_ordering(RW) 000000_01 axi1_w_priority(RW) 000000_01 axi1_r_priority(RW) 
MC0_CTL_760: .double 0x0303030000020001
//0000_0011 axi0_priority2_relative_priority(RW) 0000_0011 axi0_priority1_relative_priority(RW) 0000_0011 axi0_priority0_relative_priority(RW) 0000_0000 address_mirroring(RW) 00000_000 tdfi_dram_clk_disable(RW) 00000_010 bstlen(RW) 000000_00 zq_req(WR) 000000_01 zq_on_sref_exit(RW) 
MC0_CTL_770: .double 0x0101010202020203
//0000_0001 axi2_priority2_relative_priority(RW) 0000_0001 axi2_priority1_relative_priority(RW) 0000_0001 axi2_priority0_relative_priority(RW) 0000_0010 axi1_priority3_relative_priority(RW) 0000_0010 axi1_priority2_relative_priority(RW) 0000_0010 axi1_priority1_relative_priority(RW) 0000_0010 axi1_priority0_relative_priority(RW) 0000_0011 axi0_priority3_relative_priority(RW) 
MC0_CTL_780: .double 0x0102020400040001
//0000_0001 tdfi_dram_clk_enable(RW) 0000_0010 tdfi_ctrl_delay(RW) 0000_0010 rdlvl_gate_dq_zero_count(RW) 0000_0100 rdlvl_dq_zero_count(RW) 0000_0000 lowpower_refresh_enable(RW) 0000_0110 dram_class(RW) 0000_1100 burst_on_fly_bit(RW) 0000_0001 axi2_priority3_relative_priority(RW) 
MC0_CTL_790: .double 0x281900000f000303
//00_101000 wlmrd(RW) 00_011001 wldqsen(RW) 000_00000 lowpower_control(RW) 000_00000 lowpower_auto_enable(RW) 0000_1111 zqcs_chip(RW) 0000_0000 wrr_param_value_err(RD) 0000_0011 tdfi_wrlvl_dll(RW) 0000_0011 tdfi_rdlvl_dll(RW) 
MC0_CTL_7a0: .double 0x00000000000000ff
MC0_CTL_7b0: .double 0x0000000000000000
MC0_CTL_7c0: .double 0x0000000000000000
MC0_CTL_7d0: .double 0x0000000000000000
MC0_CTL_7e0: .double 0x0000000000000000
//00000000 rdlvl_gate_delay_2(RD) 00000000 rdlvl_gate_delay_1(RD) 00000000 rdlvl_gate_delay_0(RD) 00000000 rdlvl_gate_clk_adjust_8(RW) 00000000 rdlvl_gate_clk_adjust_7(RW) 00000000 rdlvl_gate_clk_adjust_6(RW) 00000000 rdlvl_gate_clk_adjust_5(RW) 00000000 rdlvl_gate_clk_adjust_4(RW) 
MC0_CTL_7f0: .double 0xff08000000000000
//11111111 rdlvl_max_delay(RW) 00001000 rdlvl_gate_max_delay(RW) 00000000 rdlvl_gate_delay_8(RD) 00000000 rdlvl_gate_delay_7(RD) 00000000 rdlvl_gate_delay_6(RD) 00000000 rdlvl_gate_delay_5(RD) 00000000 rdlvl_gate_delay_4(RD) 00000000 rdlvl_gate_delay_3(RD) 
MC0_CTL_800: .double 0x0000000000000000
//00000000 rdlvl_midpoint_delay_7(RD) 00000000 rdlvl_midpoint_delay_6(RD) 00000000 rdlvl_midpoint_delay_5(RD) 00000000 rdlvl_midpoint_delay_4(RD) 00000000 rdlvl_midpoint_delay_3(RD) 00000000 rdlvl_midpoint_delay_2(RD) 00000000 rdlvl_midpoint_delay_1(RD) 00000000 rdlvl_midpoint_delay_0(RD) 
MC0_CTL_810: .double 0x000000000000000e
//00000000 rdlvl_offset_delay_6(RW) 00000000 rdlvl_offset_delay_5(RW) 00000000 rdlvl_offset_delay_4(RW) 00000000 rdlvl_offset_delay_3(RW) 00000000 rdlvl_offset_delay_2(RW) 00000000 rdlvl_offset_delay_1(RW) 00000000 rdlvl_offset_delay_0(RW) 00000000 rdlvl_midpoint_delay_8(RD) 
MC0_CTL_820: .double 0x0420000c20400000
//00000100 tdfi_wrlvl_resplat(RW) 00000000 tdfi_wrlvl_resp(RD) 00000000 tdfi_rdlvl_rr(RW) 00001100 tdfi_rdlvl_resplat(RW) 00000000 tdfi_rdlvl_resp(RD) 01000000 refresh_per_zq(RW) 00000000 rdlvl_offset_delay_8(RW) 00000000 rdlvl_offset_delay_7(RW) 
//MC0_CTL_830: .double 0x0000000000000c0a
MC0_CTL_830: .double 0x1313131313130c0a
//0x1313131313130c0a mxl modified
//00000000 wrlvl_delay_5(RD) 00000000 wrlvl_delay_4(RD) 00000000 wrlvl_delay_3(RD) 00000000 wrlvl_delay_2(RD) 00000000 wrlvl_delay_1(RD) 00000000 wrlvl_delay_0(RD) 00000010 tmod(RW) 00001010 tdfi_wrlvl_ww(RW) 
MC0_CTL_840: .double 0x0000640064001313
// 0x0000640064001313 mxl modified
//00000000000000_0001100100 axi1_priority_relax(RW) 000000_0001100100 axi0_priority_relax(RW) 00000000 wrlvl_delay_8(RD) 00000000 wrlvl_delay_7(RD) 00000000 wrlvl_delay_6(RD) 
MC0_CTL_850: .double 0x0000000000000064
MC0_CTL_860: .double 0x0200004000000000
MC0_CTL_870: .double 0x0044004400440046
//0x0044004400440046  mxl modified
//0_000000000000010 emrs1_data_3(RW) 0_000000000000010 emrs1_data_2(RW) 0_000000000000010 emrs1_data_1(RW) 0_000000000000010 emrs1_data_0(RW) 
MC0_CTL_880: .double 0x0000000000000000
MC0_CTL_890: .double 0x0a520a520a520a52
//0x0a520a520a520a52  mxl modified 
//0_000010000010000 mrs_data_3(RW) 0_000010000010000 mrs_data_2(RW) 0_000010000010000 mrs_data_1(RW) 0_000010000010000 mrs_data_0(RW) 
MC0_CTL_8a0: .double 0x00000000001c001c
MC0_CTL_8b0: .double 0x0000000000000000
MC0_CTL_8c0: .double 0x0004000000000000
MC0_CTL_8d0: .double 0x00000000c8000000
MC0_CTL_8e0: .double 0x0000000000000050
//MC0_CTL_8f0: .double 0x0000000020202080
MC0_CTL_8f0: .double 0x000000000a100080 //clk skew cleared
//0x000000000a100080  mxl modified
//MC0_CTL_8f0: .double 0x000000002b352180
//MC0_CTL_8f0: .double 0x0000000040404080
//0000000000000000000000000111100_000000000000000000000000001111000 dll_ctrl_reg_2(RW) 
MC0_CTL_900: .double 0x0000000000000000
MC0_CTL_910: .double 0x0000000000000000
MC0_CTL_920: .double 0x0000000000000000
MC0_CTL_930: .double 0x0000000000000000
MC0_CTL_940: .double 0x0300000000050500
MC0_CTL_950: .double 0x0000000000000a03
MC0_CTL_960: .double 0x0604000100000000
//000_00101 rdlat_adj(RW) 0000_0100 wrlat_adj(RW) 0000000_0 swlvl_start(WR) 0000000_0 swlvl_load(WR) 0000000_0 swlvl_exit(WR) 000000_000000000000000000 int_status(RD) 
MC0_CTL_970: .double 0x000000000003e805
MC0_CTL_980: .double 0x0001000001000000
//0000000_0 zq_in_progress(RD) 0000000_1 zqcs_rotate(RW) 0000000_0 wrlvl_reg_en(RW) 0000000_0 wrlvl_en(RW) 0000000_1 resync_dll_per_aref_en(RW) 0000000_0 resync_dll(WR) 0000000_0 rdlvl_reg_en(RW) 0000000_0 rdlvl_gate_reg_en(RW) 
MC0_CTL_990: .double 0x0001020202000100
//00000_000 w2w_samecs_dly(RW) 00000_001 w2w_diffcs_dly(RW) 00000_010 tbst_int_interval(RW) 00000_010 r2w_samecs_dly(RW) 00000_010 r2w_diffcs_dly(RW) 00000_000 r2r_samecs_dly(RW) 00000_001 r2r_diffcs_dly(RW) 00000_000 axi_aligned_strobe_disable(RW) 
MC0_CTL_9a0: .double 0x0707030200060100
//00000111 tdfi_wrlvl_load(RW) 00000111 tdfi_rdlvl_load(RW) 000_00011 tckesr(RW) 000_00010 tccd(RW) 000_00000 add_odt_clk_difftype_diffcs(RW) 0000_0110 trp_ab(RW) 0000_0001 add_odt_clk_sametype_diffcs(RW) 0000_0000 add_odt_clk_difftype_samecs(RW) 
MC0_CTL_9b0: .double 0x02000100000a000f
//0000_001000000000 zqinit(RW) 0000_000100000000 zqcl(RW) 000000_0000001010 tdfi_wrlvl_ww(RW) 000000_0000001111 tdfi_rdlvl_rr(RW) 
MC0_CTL_9c0: .double 0x08520c2d0c2d0c2d
//MC0_CTL_9c0: .double 0x04100c2d0c2d0c2d
//0_000101001010010 mr0_data_0(RW) 00_00110000101101 tdfi_phyupd_type3(RW) 00_00110000101101 tdfi_phyupd_type2(RW) 00_00110000101101 tdfi_phyupd_type1(RW) 
//MC0_CTL_9d0: .double 0x0004041004100410
MC0_CTL_9d0: .double 0x0044085208520852
//0_000000000000100 mr1_data_0(RW) 0_000101001010010 mr0_data_3(RW) 0_000101001010010 mr0_data_2(RW) 0_000101001010010 mr0_data_1(RW) 
MC0_CTL_9e0: .double 0x0000004400440044
//0_000000000000000 mr2_data_0(RW) 0_000000000000100 mr1_data_3(RW) 0_000000000000100 mr1_data_2(RW) 0_000000000000100 mr1_data_1(RW) 
MC0_CTL_9f0: .double 0x0000000000000000
//0_000000000000000 mr3_data_0(RW) 0_000000000000000 mr2_data_3(RW) 0_000000000000000 mr2_data_2(RW) 0_000000000000000 mr2_data_1(RW) 
MC0_CTL_a00: .double 0x00ff000000000000
//0000000011111111 dfi_wrlvl_max_delay(RW) 0_000000000000000 mr3_data_3(RW) 0_000000000000000 mr3_data_2(RW) 0_000000000000000 mr3_data_1(RW) 
MC0_CTL_a10: .double 0x0000000000000000
//0000000000000000 rdlvl_begin_delay_3(RD) 0000000000000000 rdlvl_begin_delay_2(RD) 0000000000000000 rdlvl_begin_delay_1(RD) 0000000000000000 rdlvl_begin_delay_0(RD) 
MC0_CTL_a20: .double 0x0000000000000000
//0000000000000000 rdlvl_begin_delay_7(RD) 0000000000000000 rdlvl_begin_delay_6(RD) 0000000000000000 rdlvl_begin_delay_5(RD) 0000000000000000 rdlvl_begin_delay_4(RD) 
MC0_CTL_a30: .double 0x0024002400240000
//0000111000001110 rdlvl_delay_2(RW) 0000111000001110 rdlvl_delay_1(RW) 0000111000001110 rdlvl_delay_0(RW) 0000000000000000 rdlvl_begin_delay_8(RD) 
MC0_CTL_a40: .double 0x0024002400240024
//0000111000001110 rdlvl_delay_6(RW) 0000111000001110 rdlvl_delay_5(RW) 0000111000001110 rdlvl_delay_4(RW) 0000111000001110 rdlvl_delay_3(RW) 
MC0_CTL_a50: .double 0x0000000000240024
//0000000000000000 rdlvl_end_delay_1(RD) 0000000000000000 rdlvl_end_delay_0(RD) 0000111000001110 rdlvl_delay_8(RW) 0000111000001110 rdlvl_delay_7(RW) 
MC0_CTL_a60: .double 0x0000000000000000
//0000000000000000 rdlvl_end_delay_5(RD) 0000000000000000 rdlvl_end_delay_4(RD) 0000000000000000 rdlvl_end_delay_3(RD) 0000000000000000 rdlvl_end_delay_2(RD) 
MC0_CTL_a70: .double 0x0010000000000000
//0000000000000000 rdlvl_gate_delay_0(RW+) 0000000000000000 rdlvl_end_delay_8(RD) 0000000000000000 rdlvl_end_delay_7(RD) 0000000000000000 rdlvl_end_delay_6(RD) 
MC0_CTL_a80: .double 0x0010001000100010
//0000000000000000 rdlvl_gate_delay_4(RW+) 0000000000000000 rdlvl_gate_delay_3(RW+) 0000000000000000 rdlvl_gate_delay_2(RW+) 0000000000000000 rdlvl_gate_delay_1(RW+) 
MC0_CTL_a90: .double 0x0010001000100010
//0000000000000000 rdlvl_gate_delay_8(RW+) 0000000000000000 rdlvl_gate_delay_7(RW+) 0000000000000000 rdlvl_gate_delay_6(RW+) 0000000000000000 rdlvl_gate_delay_5(RW+) 
MC0_CTL_aa0: .double 0x0000ffff00000000
//0000000000000000 rdlvl_midpoint_delay_0(RD) 1111111111111111 rdlvl_max_delay(RW) 0000000000000000 rdlvl_gate_refresh_interval(RW) 0000000000010000 rdlvl_gate_max_delay(RW) 
MC0_CTL_ab0: .double 0x0000000000000000
//0000000000000000 rdlvl_midpoint_delay_4(RD) 0000000000000000 rdlvl_midpoint_delay_3(RD) 0000000000000000 rdlvl_midpoint_delay_2(RD) 0000000000000000 rdlvl_midpoint_delay_1(RD) 
MC0_CTL_ac0: .double 0x0000000000000000
//0000000000000000 rdlvl_midpoint_delay_8(RD) 0000000000000000 rdlvl_midpoint_delay_7(RD) 0000000000000000 rdlvl_midpoint_delay_6(RD) 0000000000000000 rdlvl_midpoint_delay_5(RD) 
MC0_CTL_ad0: .double 0x0000000000000000
//0000000000000000 rdlvl_offset_delay_3(RW) 0000000000000000 rdlvl_offset_delay_2(RW) 0000000000000000 rdlvl_offset_delay_1(RW) 0000000000000000 rdlvl_offset_delay_0(RW) 
MC0_CTL_ae0: .double 0x0000000000000000
//0000000000000000 rdlvl_offset_delay_7(RW) 0000000000000000 rdlvl_offset_delay_6(RW) 0000000000000000 rdlvl_offset_delay_5(RW) 0000000000000000 rdlvl_offset_delay_4(RW) 
MC0_CTL_af0: .double 0x0034003400000000
//0000000000000000 wrlvl_delay_1(RW+) 0000000000000000 wrlvl_delay_0(RW+) 0000000000000000 rdlvl_refresh_interval(RW) 0000000000000000 rdlvl_offset_delay_8(RW) 
MC0_CTL_b00: .double 0x0034003400340034
//0000000000000000 wrlvl_delay_5(RW+) 0000000000000000 wrlvl_delay_4(RW+) 0000000000000000 wrlvl_delay_3(RW+) 0000000000000000 wrlvl_delay_2(RW+) 
MC0_CTL_b10: .double 0x0000003400340034
//0000000000000000 wrlvl_refresh_interval(RW) 0000000000000000 wrlvl_delay_8(RW+) 0000000000000000 wrlvl_delay_7(RW+) 0000000000000000 wrlvl_delay_6(RW+) 
MC0_CTL_b20: .double 0x00000c2d00000c2d
//00000000000000000000110000101101 tdfi_rdlvl_resp(RW) 00000000000000000000110000101101 tdfi_rdlvl_max(RW) 
MC0_CTL_b30: .double 0x00000c2d00000000
//00000000000000000000110000101101 tdfi_wrlvl_resp(RW) 00000000000000000000000000000000 tdfi_wrlvl_max(RW) 
MC0_CTL_start_DATA_LO: .word  0x01000000
//0000000_1 rw_same_en 0000000_0 reg_dimm_enable 0000000_0 reduc 0000000_0 pwrup_srefresh_exit
MC0_CTL_start_DATA_HI: .word  0x00000100
//0000000_1 swap_port_rw_same_en 0000000_1 swap_en 0000000_0 start 0000000_0 srefresh

#endif

#if 1
ddr2_reg_data_mc1:
MC1_CTL_000: .double 0x0000000000000101
//000000000000000_0 concurrentap(RW) 0000000_1 bank_split_en(RW) 0000000_0 auto_refresh_mode(RW) 0000000_0 arefresh(WR) 0000000_0 ap(RW) 0000000_1 addr_cmp_en(RW) 0000000_1 active_aging(RW) 
MC1_CTL_010: .double 0x0001000100010000
//0000000_0 fwc(WR) 0000000_0 fast_write(RW) 0000000_1 enable_quick_srefresh(RW) 0000000_1 eight_bank_mode(RW) 0000000_0 ecc_disable_w_uc_err(RW) 0000000_1 dqs_n_en(RW) 0000000_0 dll_bypass_mode(RW) 0000000_0 dlllockreg(RD) 
MC1_CTL_020: .double 0x0100010100000000
// 0x0100010101000000 mxl modified 
//0000000_1 priority_en(RW) 0000000_0 power_down(RW) 0000000_1 placement_en(RW) 0000000_1 odt_add_turn_clk_en(RW) 0000000_0 no_cmd_init(RW) 0000000_0 intrptwritea(RW) 0000000_0 intrptreada(RW) 0000000_0 intrptapburst(RW) 
MC1_CTL_030: .double 0x0101000001000000
//0000000_1 swap_port_rw_same_en(RW) 0000000_1 swap_en(RW) 0000000_0 start(RW) 0000000_0 srefresh(RW+) 0000000_1 rw_same_en(RW) 0000000_0 reg_dimm_enable(RW) 0000000_0 reduc(RW) 0000000_0 pwrup_srefresh_exit(RW) 
MC1_CTL_040: .double 0x0100010100010101
//000000_01 rtt_0(RW) 000000_11 ctrl_raw(RW) 000000_01 axi0_w_priority(RW) 000000_01 axi0_r_priority(RW) 0000000_0 write_modereg(WR) 0000000_1 writeinterp(RW) 0000000_1 tref_enable(RW) 0000000_1 tras_lockout(RW) 
MC1_CTL_050: .double 0x0000000404050100
//00000_000 q_fullness(RW) 00000_000 port_data_error_type(RD) 000_00000 out_of_range_type(RD) 00000_000 max_cs_reg(RD) 00000_100 column_size(RW) 0000_0101 caslat(RW) 00000_010 addr_pins(RW) 00000000
MC1_CTL_060: .double 0x0a04040603040003
//0000_1010 aprebit(RW) 0000_0101 wrlat(RW) 0000_0100 twtr(RW) 000_00110 twr_int(RW) 00000_011 trtp(RW) 00000_100 trrd(RW) 0000000000000_011 tcke(RW) 
MC1_CTL_070: .double 0x0f0e020000010a08
//0000_0000 max_row_reg(RD) 0000_0000 max_col_reg(RD) 0000_0000 initaref(RW) 00000000000000000000_1111 cs_map(RW) 000_01010 caslat_lin_gate(RW) 000_01010 caslat_lin(RW) 
MC1_CTL_080: .double 0x0104040101000400
//0000_0001 odt_wr_map_cs3(RW) 0000_0010 odt_wr_map_cs2(RW) 0000_0100 odt_wr_map_cs1(RW) 0000_1000 odt_wr_map_cs0(RW) 0000_0001 odt_rd_map_cs3(RW) 0000_0010 odt_rd_map_cs2(RW) 0000_0100 odt_rd_map_cs1(RW) 0000_1000 odt_rd_map_cs0(RW) 
MC1_CTL_090: .double 0x0000050b00000000
//000_00000 ocd_adjust_pup_cs_0(RW) 000_00000 ocd_adjust_pdn_cs_0(RW) 0000_0101 trp(RW) 000_01011 tdal(RW) 000000000000_0000 port_cmd_error_type(RD) 0000000000000000
MC1_CTL_0a0: .double 0x0000003f3f140612
//MC1_CTL_0a0: .double 0x0000003f3f14021b
//00000000000000000000000000_111111 command_age_count(RW) 00_111111 age_count(RW) 000_10100 trc(RW) 000_00110 tmrd(RW) 000_10010 tfaw(RW) 
MC1_CTL_0b0: .double 0x0000000000000000
MC1_CTL_0c0: .double 0x00002c050f000000
//000000000000000000101100 trfc(RW) 00000101 trcd_int(RW) 00001111 tras_min(RW) 00000000 out_of_range_length(RD) 00000000 ecc_u_synd(RD) 00000000 ecc_c_synd(RD) 
MC1_CTL_0d0: .double 0x0000000000000000
MC1_CTL_0e0: .double 0x0000000000000000
MC1_CTL_0f0: .double 0x0000000000000000
MC1_CTL_100: .double 0x0000000000000000
MC1_CTL_110: .double 0x0000000000001c2d
//0_000000000000000 emrs2_data_1(RW) 0_000000000000000 emrs2_data_0(RW) 000000000000000000_00110000101101 tref(RW) 
MC1_CTL_120: .double 0xffff000000000000
//0000000000011100 axi0_en_size_lt_width_instr(RW) 00000000000000000_000000000000000 emrs2_data_3(RW) 0_000000000000000 emrs2_data_2(RW) 
MC1_CTL_130: .double 0x0d56000302000000
//0110110101010110 tras_max(RW) 0000000000000011 tpdex(RW) 0000001000000000 tdll(RW) 0000000000000000 tcpd(RW) 
MC1_CTL_140: .double 0x0000204002000030
//0000000000000000 xor_check_bits(RW) 0000000000000000 version(RD) 0000001000000000 txsr(RW) 0000000000110000 txsnr(RW) 
MC1_CTL_150: .double 0x0000000011000004
//000_0000000000000000000000000000000000000 ecc_c_addr(RD) 000000000000000000011011 tinit(RW) 
MC1_CTL_160: .double 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 ecc_u_addr(RD) 
MC1_CTL_170: .double 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 out_of_range_addr(RD) 
MC1_CTL_180: .double 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 port_cmd_error_addr(RD) 
MC1_CTL_190: .double 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000000 ecc_c_data(RD) 
MC1_CTL_1a0: .double 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000000 ecc_u_data(RD) 
MC1_CTL_1b0: .double 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000_000 cke_delay(RW) 
MC1_CTL_1c0: .double 0x0000000000000000
MC1_CTL_1d0: .double 0x0203070400000101
//0000_0010 tdfi_phy_wrlat_base(RW) 0000_0000 tdfi_phy_wrlat(RD) 000_00111 tdfi_phy_rdlat(RW) 0000_0000 tdfi_ctrlupd_min(RD) 000000000000_0000 dram_clk_disable(RW) 0000000_1 odt_alt_en(RW) 0000000_1 drive_dq_dqs(RW) 
MC1_CTL_1e0: .double 0x0c2d0c2d0c2d0205
//00_00000000000000 tdfi_phyupd_type0(RD) 00_00000000000000 tdfi_phyupd_resp(RD) 00_00000000000000 tdfi_ctrlupd_max(RD) 000_00000 tdfi_rddata_en_base(RW) 000_00000 tdfi_rddata_en(RD) 
#ifdef DDR_DEBUG
//MC1_CTL_1f0: .double 0x00120e8000000000
//MC1_CTL_200: .double 0x00160e8000160e80
//MC1_CTL_210: .double 0x00100e8000130e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC1_CTL_220: .double 0x00130e8000010e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC1_CTL_230: .double 0x00200e8000100e80

#if 1
MC1_CTL_1f0: .double 0x000a0e8000000000
MC1_CTL_200: .double 0x000a0e80000a0e80
MC1_CTL_210: .double 0x000a0e80000a0e80
//00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
MC1_CTL_220: .double 0x000a0e80000a0e80
//00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
MC1_CTL_230: .double 0x000a0e80000a0e80
#else
MC1_CTL_1f0: .double 0x00300e8000000000
MC1_CTL_200: .double 0x00300e8000300e80
MC1_CTL_210: .double 0x00300e8000300e80
//00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
MC1_CTL_220: .double 0x00300e8000300e80
//00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
MC1_CTL_230: .double 0x00300e8000300e80

#endif

//#debug write
//MC1_CTL_1f0: .double 0x00000e8000000000
//MC1_CTL_200: .double 0x00000e8000000e80
//MC1_CTL_210: .double 0x00000e8000000e80
//00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC1_CTL_220: .double 0x00000e8000000e80
//00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC1_CTL_230: .double 0x00200e8000000e80

//#debug read
//MC1_CTL_1f0: .double 0x00120e8000000000
//MC1_CTL_200: .double 0x00160e8000160e80
//MC1_CTL_210: .double 0x00100e8000140e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC1_CTL_220: .double 0x00100e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC1_CTL_230: .double 0x00200e8000140e80

#else
MC1_CTL_1f0: .double 0x00180e8000000000
MC1_CTL_200: .double 0x00180e8000180e80
MC1_CTL_210: .double 0x00180e8000180e80
//00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
MC1_CTL_220: .double 0x00180e8000180e80
//00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
MC1_CTL_230: .double 0x00180e8000180e80

//MC1_CTL_1f0: .double 0x00130e8000000000
//MC1_CTL_200: .double 0x00130e8000130e80
//MC1_CTL_210: .double 0x00130e8000130e80
////00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC1_CTL_220: .double 0x00100e8000130e80
////00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC1_CTL_230: .double 0x00150e8000120e80

//MC1_CTL_1f0: .double 0x00230eff00000000
//MC1_CTL_200: .double 0x00230eff00230eff
//MC1_CTL_210: .double 0x00230eff00230eff
////00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC1_CTL_220: .double 0x00200eff00230eff
////00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC1_CTL_230: .double 0x00150eff00200eff

//MC1_CTL_1f0: .double 0x00120e8000000000
//MC1_CTL_200: .double 0x00160e8000160e80
//MC1_CTL_210: .double 0x00100e8000180e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC1_CTL_220: .double 0x000c0e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC1_CTL_230: .double 0x00200e8000100e80

//150M MC0
//MC1_CTL_1f0: .double 0x001a0e8000000000
//MC1_CTL_200: .double 0x00120e8000120e80
//MC1_CTL_210: .double 0x00100e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC1_CTL_220: .double 0x00100e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC1_CTL_230: .double 0x00200e8000120e80

//150M MC0
//MC1_CTL_1f0: .double 0x00160e8000000000
//MC1_CTL_200: .double 0x00100e8000100e80
//MC1_CTL_210: .double 0x00100e8000160e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC1_CTL_220: .double 0x00160e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC1_CTL_230: .double 0x00200e8000100e80
#endif
MC1_CTL_240: .double 0x00000e0000000e00
//00000000000000000000111000000000 dll_ctrl_reg_1_1(RW) 00000000000000000000111000000000 dll_ctrl_reg_1_0(RW) 
MC1_CTL_250: .double 0x00000e0000000e00
MC1_CTL_260: .double 0x0000100000000e00
// 0x00000e0000000e00 mxl modified 
MC1_CTL_270: .double 0x00000e0000000e00
MC1_CTL_280: .double 0x0000000000000e00
MC1_CTL_290: .double 0x0000000000000000
MC1_CTL_2a0: .double 0x0000000000000000
MC1_CTL_2b0: .double 0x0000000000000000
MC1_CTL_2c0: .double 0x0000000000000000
MC1_CTL_2d0: .double 0xf3004946003c099c
// 0xf300494603c0019d   mxl  modified
//11110100000000000011101100100111 phy_ctrl_reg_0_0(RD) 000000_00000000000000000110011101 pad_ctrl_reg_0(RW) 
MC1_CTL_2e0: .double 0xf3004946f3004946
MC1_CTL_2f0: .double 0xf3004946f3004946
MC1_CTL_300: .double 0xf3004946f3004946
MC1_CTL_310: .double 0xf3004946f3004946
//MC1_CTL_2d0: .double 0xf3005a470000019d
////11110100000000000011101100100111 phy_ctrl_reg_0_0(RD) 000000_00000000000000000110011101 pad_ctrl_reg_0(RW) 
//MC1_CTL_2e0: .double 0xf3005a47f3005a47
//MC1_CTL_2f0: .double 0xf3005a47f3005a47
//MC1_CTL_300: .double 0xf3005a47f3005a47
//MC1_CTL_310: .double 0xf3005a47f3005a47
MC1_CTL_320: .double 0x0680000006800000
//0x07a0000007a00000 mxl modified
MC1_CTL_330: .double 0x0680000006800000
//0x07a0000007a00000 mxl modified
MC1_CTL_340: .double 0x0680000006800000
//0x07a0000007a00000 mxl modified
MC1_CTL_350: .double 0x0680000006800000
//0x07a0000007a00000 mxl modified
MC1_CTL_360: .double 0x0800c00506800000
//0x0800c00507a00000 mxl modified

// 
//00000000000000001100000000000101 phy_ctrl_reg_2(RW) 00000111110000000000001100000001 phy_ctrl_reg_1_8(RD) 
MC1_CTL_370: .double 0x0000000000000000
MC1_CTL_380: .double 0x0000000000000000
MC1_CTL_390: .double 0x0000000000000000
MC1_CTL_3a0: .double 0x0000000000000000
MC1_CTL_3b0: .double 0x0000000000000000
MC1_CTL_3c0: .double 0x0000000000000000
MC1_CTL_3d0: .double 0x0000000000000000
MC1_CTL_3e0: .double 0x0000000000000000
MC1_CTL_3f0: .double 0x0000000000000000
MC1_CTL_400: .double 0x0000000000000000
MC1_CTL_410: .double 0x0000000000000000
MC1_CTL_420: .double 0x0000000000000000
MC1_CTL_430: .double 0x0000000000000000
MC1_CTL_440: .double 0x0000000000000000
MC1_CTL_450: .double 0x0000000000000000
MC1_CTL_460: .double 0x0000000000000000
MC1_CTL_470: .double 0x0000000000000000
MC1_CTL_480: .double 0x0000000000000000
MC1_CTL_490: .double 0x0000000000000000
MC1_CTL_4a0: .double 0x0000000000000000
MC1_CTL_4b0: .double 0x0000000000000000
MC1_CTL_4c0: .double 0x0000000000000000
MC1_CTL_4d0: .double 0x0000000000000000
MC1_CTL_4e0: .double 0x0000000000000000
MC1_CTL_4f0: .double 0x0000000000000000
MC1_CTL_500: .double 0x0000000000000000
MC1_CTL_510: .double 0x0000000000000000
MC1_CTL_520: .double 0x0000000000000000
MC1_CTL_530: .double 0x0000000000000000
MC1_CTL_540: .double 0x0000000000000000
MC1_CTL_550: .double 0x0000000000000000
MC1_CTL_560: .double 0x0000000000000000
MC1_CTL_570: .double 0x0000000000000000
MC1_CTL_580: .double 0x0000000000000000
MC1_CTL_590: .double 0x0000000000000000
MC1_CTL_5a0: .double 0x0000000000000000
MC1_CTL_5b0: .double 0x0000000000000000
MC1_CTL_5c0: .double 0x0000000000000000
MC1_CTL_5d0: .double 0x0000000000000000
MC1_CTL_5e0: .double 0x0000000000000000
MC1_CTL_5f0: .double 0x0000000000000000
MC1_CTL_600: .double 0x0000000000000000
MC1_CTL_610: .double 0x0000000000000000
MC1_CTL_620: .double 0x0000000000000000
MC1_CTL_630: .double 0x0000000000000000
MC1_CTL_640: .double 0x0000000000000000
MC1_CTL_650: .double 0x0000000000000000
MC1_CTL_660: .double 0x0000000000000000
MC1_CTL_670: .double 0x0000000000000000
MC1_CTL_680: .double 0x0000000000000000
MC1_CTL_690: .double 0x0000000000000000
MC1_CTL_6a0: .double 0x0000000000000000
MC1_CTL_6b0: .double 0x0000000000000000
MC1_CTL_6c0: .double 0x0000000000000000
MC1_CTL_6d0: .double 0x0000000000000000
MC1_CTL_6e0: .double 0x0000000000000000
MC1_CTL_6f0: .double 0x0000000000000000
MC1_CTL_700: .double 0x0000000000000000
MC1_CTL_710: .double 0x0000000000000000
MC1_CTL_720: .double 0x0000000000000000
MC1_CTL_730: .double 0x0000000000000000
MC1_CTL_740: .double 0x0100000000000000
//MC1_CTL_750: .double 0x0100000101020101
MC1_CTL_750: .double 0x0101000101020101
//000000_01 wrlvl_cs(RW) 000000_00 sw_leveling_mode(RW) 000000_00 rdlvl_cs(RW) 000000_01 axi2_w_priority(RW) 000000_01 axi2_r_priority(RW) 000000_10 axi2_port_ordering(RW) 000000_01 axi1_w_priority(RW) 000000_01 axi1_r_priority(RW) 
MC1_CTL_760: .double 0x0303030000020001
//0000_0011 axi0_priority2_relative_priority(RW) 0000_0011 axi0_priority1_relative_priority(RW) 0000_0011 axi0_priority0_relative_priority(RW) 0000_0000 address_mirroring(RW) 00000_000 tdfi_dram_clk_disable(RW) 00000_010 bstlen(RW) 000000_00 zq_req(WR) 000000_01 zq_on_sref_exit(RW) 
MC1_CTL_770: .double 0x0101010202020203
//0000_0001 axi2_priority2_relative_priority(RW) 0000_0001 axi2_priority1_relative_priority(RW) 0000_0001 axi2_priority0_relative_priority(RW) 0000_0010 axi1_priority3_relative_priority(RW) 0000_0010 axi1_priority2_relative_priority(RW) 0000_0010 axi1_priority1_relative_priority(RW) 0000_0010 axi1_priority0_relative_priority(RW) 0000_0011 axi0_priority3_relative_priority(RW) 
MC1_CTL_780: .double 0x0102020400040001
//0000_0001 tdfi_dram_clk_enable(RW) 0000_0010 tdfi_ctrl_delay(RW) 0000_0010 rdlvl_gate_dq_zero_count(RW) 0000_0100 rdlvl_dq_zero_count(RW) 0000_0000 lowpower_refresh_enable(RW) 0000_0110 dram_class(RW) 0000_1100 burst_on_fly_bit(RW) 0000_0001 axi2_priority3_relative_priority(RW) 
MC1_CTL_790: .double 0x281900000f000303
//00_101000 wlmrd(RW) 00_011001 wldqsen(RW) 000_00000 lowpower_control(RW) 000_00000 lowpower_auto_enable(RW) 0000_1111 zqcs_chip(RW) 0000_0000 wrr_param_value_err(RD) 0000_0011 tdfi_wrlvl_dll(RW) 0000_0011 tdfi_rdlvl_dll(RW) 
MC1_CTL_7a0: .double 0x00000000000000ff
MC1_CTL_7b0: .double 0x0000000000000000
MC1_CTL_7c0: .double 0x0000000000000000
MC1_CTL_7d0: .double 0x0000000000000000
MC1_CTL_7e0: .double 0x0000000000000000
//00000000 rdlvl_gate_delay_2(RD) 00000000 rdlvl_gate_delay_1(RD) 00000000 rdlvl_gate_delay_0(RD) 00000000 rdlvl_gate_clk_adjust_8(RW) 00000000 rdlvl_gate_clk_adjust_7(RW) 00000000 rdlvl_gate_clk_adjust_6(RW) 00000000 rdlvl_gate_clk_adjust_5(RW) 00000000 rdlvl_gate_clk_adjust_4(RW) 
MC1_CTL_7f0: .double 0xff08000000000000
//11111111 rdlvl_max_delay(RW) 00001000 rdlvl_gate_max_delay(RW) 00000000 rdlvl_gate_delay_8(RD) 00000000 rdlvl_gate_delay_7(RD) 00000000 rdlvl_gate_delay_6(RD) 00000000 rdlvl_gate_delay_5(RD) 00000000 rdlvl_gate_delay_4(RD) 00000000 rdlvl_gate_delay_3(RD) 
MC1_CTL_800: .double 0x0000000000000000
//00000000 rdlvl_midpoint_delay_7(RD) 00000000 rdlvl_midpoint_delay_6(RD) 00000000 rdlvl_midpoint_delay_5(RD) 00000000 rdlvl_midpoint_delay_4(RD) 00000000 rdlvl_midpoint_delay_3(RD) 00000000 rdlvl_midpoint_delay_2(RD) 00000000 rdlvl_midpoint_delay_1(RD) 00000000 rdlvl_midpoint_delay_0(RD) 
MC1_CTL_810: .double 0x000000000000000e
//00000000 rdlvl_offset_delay_6(RW) 00000000 rdlvl_offset_delay_5(RW) 00000000 rdlvl_offset_delay_4(RW) 00000000 rdlvl_offset_delay_3(RW) 00000000 rdlvl_offset_delay_2(RW) 00000000 rdlvl_offset_delay_1(RW) 00000000 rdlvl_offset_delay_0(RW) 00000000 rdlvl_midpoint_delay_8(RD) 
MC1_CTL_820: .double 0x0420000c20400000
//00000100 tdfi_wrlvl_resplat(RW) 00000000 tdfi_wrlvl_resp(RD) 00000000 tdfi_rdlvl_rr(RW) 00001100 tdfi_rdlvl_resplat(RW) 00000000 tdfi_rdlvl_resp(RD) 01000000 refresh_per_zq(RW) 00000000 rdlvl_offset_delay_8(RW) 00000000 rdlvl_offset_delay_7(RW) 
//MC1_CTL_830: .double 0x0000000000000c0a
MC1_CTL_830: .double 0x0f29292929290c0a
//0x1313131313130c0a mxl modified
//00000000 wrlvl_delay_5(RD) 00000000 wrlvl_delay_4(RD) 00000000 wrlvl_delay_3(RD) 00000000 wrlvl_delay_2(RD) 00000000 wrlvl_delay_1(RD) 00000000 wrlvl_delay_0(RD) 00000010 tmod(RW) 00001010 tdfi_wrlvl_ww(RW) 
MC1_CTL_840: .double 0x0000640064001515
// 0x0000640064001313 mxl modified
//00000000000000_0001100100 axi1_priority_relax(RW) 000000_0001100100 axi0_priority_relax(RW) 00000000 wrlvl_delay_8(RD) 00000000 wrlvl_delay_7(RD) 00000000 wrlvl_delay_6(RD) 
MC1_CTL_850: .double 0x0000000000000064
MC1_CTL_860: .double 0x0200004000000000
MC1_CTL_870: .double 0x0000000000000046
//0x0044004400440046  mxl modified
//0_000000000000010 emrs1_data_3(RW) 0_000000000000010 emrs1_data_2(RW) 0_000000000000010 emrs1_data_1(RW) 0_000000000000010 emrs1_data_0(RW) 
MC1_CTL_880: .double 0x0000000000000000
MC1_CTL_890: .double 0x0a520a520a520a5a
//0x0a520a520a520a52  mxl modified 
//0_000010000010000 mrs_data_3(RW) 0_000010000010000 mrs_data_2(RW) 0_000010000010000 mrs_data_1(RW) 0_000010000010000 mrs_data_0(RW) 
MC1_CTL_8a0: .double 0x00000000001c001c
MC1_CTL_8b0: .double 0x0000000000000000
MC1_CTL_8c0: .double 0x0004000000000000
MC1_CTL_8d0: .double 0x00000000c8000000
MC1_CTL_8e0: .double 0x0000000000000050
//MC1_CTL_8f0: .double 0x0000000020202080
MC1_CTL_8f0: .double 0x0000000010141380 //clk skew cleared
//0x000000000a100080  mxl modified
//MC1_CTL_8f0: .double 0x000000002b352180
//MC1_CTL_8f0: .double 0x0000000040404080
//0000000000000000000000000111100_000000000000000000000000001111000 dll_ctrl_reg_2(RW) 
MC1_CTL_900: .double 0x0000000000000000
MC1_CTL_910: .double 0x0000000000000000
MC1_CTL_920: .double 0x0000000000000000
MC1_CTL_930: .double 0x0000000000000000
MC1_CTL_940: .double 0x0300000000050500
MC1_CTL_950: .double 0x0000000000000a03
MC1_CTL_960: .double 0x0604000100000000
//000_00101 rdlat_adj(RW) 0000_0100 wrlat_adj(RW) 0000000_0 swlvl_start(WR) 0000000_0 swlvl_load(WR) 0000000_0 swlvl_exit(WR) 000000_000000000000000000 int_status(RD) 
MC1_CTL_970: .double 0x000000000003e805
MC1_CTL_980: .double 0x0001000001000000
//0000000_0 zq_in_progress(RD) 0000000_1 zqcs_rotate(RW) 0000000_0 wrlvl_reg_en(RW) 0000000_0 wrlvl_en(RW) 0000000_1 resync_dll_per_aref_en(RW) 0000000_0 resync_dll(WR) 0000000_0 rdlvl_reg_en(RW) 0000000_0 rdlvl_gate_reg_en(RW) 
MC1_CTL_990: .double 0x0001020202000100
//00000_000 w2w_samecs_dly(RW) 00000_001 w2w_diffcs_dly(RW) 00000_010 tbst_int_interval(RW) 00000_010 r2w_samecs_dly(RW) 00000_010 r2w_diffcs_dly(RW) 00000_000 r2r_samecs_dly(RW) 00000_001 r2r_diffcs_dly(RW) 00000_000 axi_aligned_strobe_disable(RW) 
MC1_CTL_9a0: .double 0x0707030200060100
//00000111 tdfi_wrlvl_load(RW) 00000111 tdfi_rdlvl_load(RW) 000_00011 tckesr(RW) 000_00010 tccd(RW) 000_00000 add_odt_clk_difftype_diffcs(RW) 0000_0110 trp_ab(RW) 0000_0001 add_odt_clk_sametype_diffcs(RW) 0000_0000 add_odt_clk_difftype_samecs(RW) 
MC1_CTL_9b0: .double 0x02000100000a000f
//0000_001000000000 zqinit(RW) 0000_000100000000 zqcl(RW) 000000_0000001010 tdfi_wrlvl_ww(RW) 000000_0000001111 tdfi_rdlvl_rr(RW) 
MC1_CTL_9c0: .double 0x0a520c2d0c2d0c2d
//MC1_CTL_9c0: .double 0x04100c2d0c2d0c2d
//0_000101001010010 mr0_data_0(RW) 00_00110000101101 tdfi_phyupd_type3(RW) 00_00110000101101 tdfi_phyupd_type2(RW) 00_00110000101101 tdfi_phyupd_type1(RW) 
//MC1_CTL_9d0: .double 0x0004041004100410
MC1_CTL_9d0: .double 0x00440a520a520a52
//0_000000000000100 mr1_data_0(RW) 0_000101001010010 mr0_data_3(RW) 0_000101001010010 mr0_data_2(RW) 0_000101001010010 mr0_data_1(RW) 
MC1_CTL_9e0: .double 0x0000004400440044
//0_000000000000000 mr2_data_0(RW) 0_000000000000100 mr1_data_3(RW) 0_000000000000100 mr1_data_2(RW) 0_000000000000100 mr1_data_1(RW) 
MC1_CTL_9f0: .double 0x0000000000000000
//0_000000000000000 mr3_data_0(RW) 0_000000000000000 mr2_data_3(RW) 0_000000000000000 mr2_data_2(RW) 0_000000000000000 mr2_data_1(RW) 
MC1_CTL_a00: .double 0x00ff000000000000
//0000000011111111 dfi_wrlvl_max_delay(RW) 0_000000000000000 mr3_data_3(RW) 0_000000000000000 mr3_data_2(RW) 0_000000000000000 mr3_data_1(RW) 
MC1_CTL_a10: .double 0x0000000000000000
//0000000000000000 rdlvl_begin_delay_3(RD) 0000000000000000 rdlvl_begin_delay_2(RD) 0000000000000000 rdlvl_begin_delay_1(RD) 0000000000000000 rdlvl_begin_delay_0(RD) 
MC1_CTL_a20: .double 0x0000000000000000
//0000000000000000 rdlvl_begin_delay_7(RD) 0000000000000000 rdlvl_begin_delay_6(RD) 0000000000000000 rdlvl_begin_delay_5(RD) 0000000000000000 rdlvl_begin_delay_4(RD) 
MC1_CTL_a30: .double 0x0e0e0e0e0e0e0000
//0000111000001110 rdlvl_delay_2(RW) 0000111000001110 rdlvl_delay_1(RW) 0000111000001110 rdlvl_delay_0(RW) 0000000000000000 rdlvl_begin_delay_8(RD) 
MC1_CTL_a40: .double 0x0e0e0e0e0e0e0e0e
//0000111000001110 rdlvl_delay_6(RW) 0000111000001110 rdlvl_delay_5(RW) 0000111000001110 rdlvl_delay_4(RW) 0000111000001110 rdlvl_delay_3(RW) 
MC1_CTL_a50: .double 0x000000000e0e0e0e
//0000000000000000 rdlvl_end_delay_1(RD) 0000000000000000 rdlvl_end_delay_0(RD) 0000111000001110 rdlvl_delay_8(RW) 0000111000001110 rdlvl_delay_7(RW) 
MC1_CTL_a60: .double 0x0000000000000000
//0000000000000000 rdlvl_end_delay_5(RD) 0000000000000000 rdlvl_end_delay_4(RD) 0000000000000000 rdlvl_end_delay_3(RD) 0000000000000000 rdlvl_end_delay_2(RD) 
MC1_CTL_a70: .double 0x0000000000000000
//0000000000000000 rdlvl_gate_delay_0(RW+) 0000000000000000 rdlvl_end_delay_8(RD) 0000000000000000 rdlvl_end_delay_7(RD) 0000000000000000 rdlvl_end_delay_6(RD) 
MC1_CTL_a80: .double 0x0000000000000000
//0000000000000000 rdlvl_gate_delay_4(RW+) 0000000000000000 rdlvl_gate_delay_3(RW+) 0000000000000000 rdlvl_gate_delay_2(RW+) 0000000000000000 rdlvl_gate_delay_1(RW+) 
MC1_CTL_a90: .double 0x0000000000000000
//0000000000000000 rdlvl_gate_delay_8(RW+) 0000000000000000 rdlvl_gate_delay_7(RW+) 0000000000000000 rdlvl_gate_delay_6(RW+) 0000000000000000 rdlvl_gate_delay_5(RW+) 
MC1_CTL_aa0: .double 0x0000ffff00000000
//0000000000000000 rdlvl_midpoint_delay_0(RD) 1111111111111111 rdlvl_max_delay(RW) 0000000000000000 rdlvl_gate_refresh_interval(RW) 0000000000010000 rdlvl_gate_max_delay(RW) 
MC1_CTL_ab0: .double 0x0000000000000000
//0000000000000000 rdlvl_midpoint_delay_4(RD) 0000000000000000 rdlvl_midpoint_delay_3(RD) 0000000000000000 rdlvl_midpoint_delay_2(RD) 0000000000000000 rdlvl_midpoint_delay_1(RD) 
MC1_CTL_ac0: .double 0x0000000000000000
//0000000000000000 rdlvl_midpoint_delay_8(RD) 0000000000000000 rdlvl_midpoint_delay_7(RD) 0000000000000000 rdlvl_midpoint_delay_6(RD) 0000000000000000 rdlvl_midpoint_delay_5(RD) 
MC1_CTL_ad0: .double 0x0000000000000000
//0000000000000000 rdlvl_offset_delay_3(RW) 0000000000000000 rdlvl_offset_delay_2(RW) 0000000000000000 rdlvl_offset_delay_1(RW) 0000000000000000 rdlvl_offset_delay_0(RW) 
MC1_CTL_ae0: .double 0x0000000000000000
//0000000000000000 rdlvl_offset_delay_7(RW) 0000000000000000 rdlvl_offset_delay_6(RW) 0000000000000000 rdlvl_offset_delay_5(RW) 0000000000000000 rdlvl_offset_delay_4(RW) 
MC1_CTL_af0: .double 0x0020002000000000
//0000000000000000 wrlvl_delay_1(RW+) 0000000000000000 wrlvl_delay_0(RW+) 0000000000000000 rdlvl_refresh_interval(RW) 0000000000000000 rdlvl_offset_delay_8(RW) 
MC1_CTL_b00: .double 0x0020002000200020
//0000000000000000 wrlvl_delay_5(RW+) 0000000000000000 wrlvl_delay_4(RW+) 0000000000000000 wrlvl_delay_3(RW+) 0000000000000000 wrlvl_delay_2(RW+) 
MC1_CTL_b10: .double 0x0000000000200020
//0000000000000000 wrlvl_refresh_interval(RW) 0000000000000000 wrlvl_delay_8(RW+) 0000000000000000 wrlvl_delay_7(RW+) 0000000000000000 wrlvl_delay_6(RW+) 
MC1_CTL_b20: .double 0x00000c2d00000c2d
//00000000000000000000110000101101 tdfi_rdlvl_resp(RW) 00000000000000000000110000101101 tdfi_rdlvl_max(RW) 
MC1_CTL_b30: .double 0x00000c2d00000000
//00000000000000000000110000101101 tdfi_wrlvl_resp(RW) 00000000000000000000000000000000 tdfi_wrlvl_max(RW) 
MC1_CTL_start_DATA_LO: .word  0x00000000
//0000000_1 rw_same_en 0000000_0 reg_dimm_enable 0000000_0 reduc 0000000_0 pwrup_srefresh_exit
MC1_CTL_start_DATA_HI: .word  0x01010100
//0000000_1 swap_port_rw_same_en 0000000_1 swap_en 0000000_0 start 0000000_0 srefresh

#endif

#if 0
MC1_CTL_000: .double 0x0000000000000101
//000000000000000_0 concurrentap(RW) 0000000_1 bank_split_en(RW) 0000000_0 auto_refresh_mode(RW) 0000000_0 arefresh(WR) 0000000_0 ap(RW) 0000000_1 addr_cmp_en(RW) 0000000_1 active_aging(RW) 
MC1_CTL_010: .double 0x0001000100010000
//0000000_0 fwc(WR) 0000000_0 fast_write(RW) 0000000_1 enable_quick_srefresh(RW) 0000000_1 eight_bank_mode(RW) 0000000_0 ecc_disable_w_uc_err(RW) 0000000_1 dqs_n_en(RW) 0000000_0 dll_bypass_mode(RW) 0000000_0 dlllockreg(RD) 
MC1_CTL_020: .double 0x0100010101000000
//0000000_1 priority_en(RW) 0000000_0 power_down(RW) 0000000_1 placement_en(RW) 0000000_1 odt_add_turn_clk_en(RW) 0000000_0 no_cmd_init(RW) 0000000_0 intrptwritea(RW) 0000000_0 intrptreada(RW) 0000000_0 intrptapburst(RW) 
MC1_CTL_030: .double 0x0101000001000000
//0000000_1 swap_port_rw_same_en(RW) 0000000_1 swap_en(RW) 0000000_0 start(RW) 0000000_0 srefresh(RW+) 0000000_1 rw_same_en(RW) 0000000_0 reg_dimm_enable(RW) 0000000_0 reduc(RW) 0000000_0 pwrup_srefresh_exit(RW) 
MC1_CTL_040: .double 0x0100010200010101
//000000_01 rtt_0(RW) 000000_11 ctrl_raw(RW) 000000_01 axi0_w_priority(RW) 000000_01 axi0_r_priority(RW) 0000000_0 write_modereg(WR) 0000000_1 writeinterp(RW) 0000000_1 tref_enable(RW) 0000000_1 tras_lockout(RW) 
MC1_CTL_050: .double 0x0000000404050100
//00000_000 q_fullness(RW) 00000_000 port_data_error_type(RD) 000_00000 out_of_range_type(RD) 00000_000 max_cs_reg(RD) 00000_100 column_size(RW) 0000_0101 caslat(RW) 00000_010 addr_pins(RW) 00000000
MC1_CTL_060: .double 0x0a04040603040003
//0000_1010 aprebit(RW) 0000_0101 wrlat(RW) 0000_0100 twtr(RW) 000_00110 twr_int(RW) 00000_011 trtp(RW) 00000_100 trrd(RW) 0000000000000_011 tcke(RW) 
MC1_CTL_070: .double 0x0f0e020000010a08
//0000_0000 max_row_reg(RD) 0000_0000 max_col_reg(RD) 0000_0000 initaref(RW) 00000000000000000000_1111 cs_map(RW) 000_01010 caslat_lin_gate(RW) 000_01010 caslat_lin(RW) 
MC1_CTL_080: .double 0x0104040101000400
//0000_0001 odt_wr_map_cs3(RW) 0000_0010 odt_wr_map_cs2(RW) 0000_0100 odt_wr_map_cs1(RW) 0000_1000 odt_wr_map_cs0(RW) 0000_0001 odt_rd_map_cs3(RW) 0000_0010 odt_rd_map_cs2(RW) 0000_0100 odt_rd_map_cs1(RW) 0000_1000 odt_rd_map_cs0(RW) 
MC1_CTL_090: .double 0x0000050b00000000
//000_00000 ocd_adjust_pup_cs_0(RW) 000_00000 ocd_adjust_pdn_cs_0(RW) 0000_0101 trp(RW) 000_01011 tdal(RW) 000000000000_0000 port_cmd_error_type(RD) 0000000000000000
MC1_CTL_0a0: .double 0x0000003f3f140612
//00000000000000000000000000_111111 command_age_count(RW) 00_111111 age_count(RW) 000_10100 trc(RW) 000_00110 tmrd(RW) 000_10010 tfaw(RW) 
MC1_CTL_0b0: .double 0x0000000000000000
MC1_CTL_0c0: .double 0x00002c050f000000
//000000000000000000101100 trfc(RW) 00000101 trcd_int(RW) 00001111 tras_min(RW) 00000000 out_of_range_length(RD) 00000000 ecc_u_synd(RD) 00000000 ecc_c_synd(RD) 
MC1_CTL_0d0: .double 0x0000000000000000
MC1_CTL_0e0: .double 0x0000000000000000
MC1_CTL_0f0: .double 0x0000000000000000
MC1_CTL_100: .double 0x0000000000000000
MC1_CTL_110: .double 0x0000000000001c2d
//0_000000000000000 emrs2_data_1(RW) 0_000000000000000 emrs2_data_0(RW) 000000000000000000_00110000101101 tref(RW) 
MC1_CTL_120: .double 0xffff000000000000
//0000000000011100 axi0_en_size_lt_width_instr(RW) 00000000000000000_000000000000000 emrs2_data_3(RW) 0_000000000000000 emrs2_data_2(RW) 
MC1_CTL_130: .double 0x0d56000302000000
//0110110101010110 tras_max(RW) 0000000000000011 tpdex(RW) 0000001000000000 tdll(RW) 0000000000000000 tcpd(RW) 
MC1_CTL_140: .double 0x0000204002000030
//0000000000000000 xor_check_bits(RW) 0000000000000000 version(RD) 0000001000000000 txsr(RW) 0000000000110000 txsnr(RW) 
MC1_CTL_150: .double 0x0000000011000004
//000_0000000000000000000000000000000000000 ecc_c_addr(RD) 000000000000000000011011 tinit(RW) 
MC1_CTL_160: .double 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 ecc_u_addr(RD) 
MC1_CTL_170: .double 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 out_of_range_addr(RD) 
MC1_CTL_180: .double 0x0000000000000000
//000000000000000000000000000_0000000000000000000000000000000000000 port_cmd_error_addr(RD) 
MC1_CTL_190: .double 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000000 ecc_c_data(RD) 
MC1_CTL_1a0: .double 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000000 ecc_u_data(RD) 
MC1_CTL_1b0: .double 0x0000000000000000
//0000000000000000000000000000000000000000000000000000000000000_000 cke_delay(RW) 
MC1_CTL_1c0: .double 0x0000000000000000
MC1_CTL_1d0: .double 0x0203070400000101
//0000_0010 tdfi_phy_wrlat_base(RW) 0000_0000 tdfi_phy_wrlat(RD) 000_00111 tdfi_phy_rdlat(RW) 0000_0000 tdfi_ctrlupd_min(RD) 000000000000_0000 dram_clk_disable(RW) 0000000_1 odt_alt_en(RW) 0000000_1 drive_dq_dqs(RW) 
MC1_CTL_1e0: .double 0x0c2d0c2d0c2d0205
//00_00000000000000 tdfi_phyupd_type0(RD) 00_00000000000000 tdfi_phyupd_resp(RD) 00_00000000000000 tdfi_ctrlupd_max(RD) 000_00000 tdfi_rddata_en_base(RW) 000_00000 tdfi_rddata_en(RD) 
#ifdef DDR_DEBUG
//MC1_CTL_1f0: .double 0x00120e8000000000
//MC1_CTL_200: .double 0x00160e8000160e80
//MC1_CTL_210: .double 0x00100e8000130e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC1_CTL_220: .double 0x00130e8000010e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC1_CTL_230: .double 0x00200e8000100e80

//MC1_CTL_1f0: .double 0x00000e8000000000
//MC1_CTL_200: .double 0x00200e8000200e80
//MC1_CTL_210: .double 0x00200e8000200e80
////00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC1_CTL_220: .double 0x00200e8000200e80
////00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC1_CTL_230: .double 0x00200e8000200e80

#if 1
MC1_CTL_1f0: .double 0x000a0e8000000000
MC1_CTL_200: .double 0x000a0e80000a0e80
MC1_CTL_210: .double 0x000a0e80000a0e80
//00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
MC1_CTL_220: .double 0x000a0e80000a0e80
//00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
MC1_CTL_230: .double 0x000a0e80000a0e80
#else
MC1_CTL_1f0: .double 0x00300e8000000000
MC1_CTL_200: .double 0x00300e8000300e80
MC1_CTL_210: .double 0x00300e8000300e80
//00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
MC1_CTL_220: .double 0x00300e8000300e80
//00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
MC1_CTL_230: .double 0x00300e8000300e80

#endif
//#debug write
//MC1_CTL_1f0: .double 0x00000e8000000000
//MC1_CTL_200: .double 0x00000e8000000e80
//MC1_CTL_210: .double 0x00000e8000000e80
//00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC1_CTL_220: .double 0x00000e8000000e80
//00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC1_CTL_230: .double 0x00200e8000000e80

//#debug read
//MC1_CTL_1f0: .double 0x00120e8000000000
//MC1_CTL_200: .double 0x00160e8000160e80
//MC1_CTL_210: .double 0x00100e8000140e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC1_CTL_220: .double 0x00100e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC1_CTL_230: .double 0x00200e8000140e80

#else
MC1_CTL_1f0: .double 0x000a0e8000000000
MC1_CTL_200: .double 0x000a0e80000a0e80
MC1_CTL_210: .double 0x000a0e80000a0e80
//00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
MC1_CTL_220: .double 0x000a0e80000f1080
//00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
MC1_CTL_230: .double 0x000a0e80000a0e80

//MC1_CTL_1f0: .double 0x00000e8000000000
//MC1_CTL_200: .double 0x00000e8000000e80
//MC1_CTL_210: .double 0x00000e8000000e80
////00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC1_CTL_220: .double 0x00000e8000000e80
////00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC1_CTL_230: .double 0x00000e8000000e80

//MC1_CTL_1f0: .double 0x00230eff00000000
//MC1_CTL_200: .double 0x00230eff00230eff
//MC1_CTL_210: .double 0x00230eff00230eff
////00000000001000000000111510000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC1_CTL_220: .double 0x00200eff00230eff
////00000000001000000000111510000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC1_CTL_230: .double 0x00150eff00200eff

//MC1_CTL_1f0: .double 0x00120e8000000000
//MC1_CTL_200: .double 0x00160e8000160e80
//MC1_CTL_210: .double 0x00100e8000180e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC1_CTL_220: .double 0x000c0e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC1_CTL_230: .double 0x00200e8000100e80

//150M MC1
//MC1_CTL_1f0: .double 0x001a0e8000000000
//MC1_CTL_200: .double 0x00120e8000120e80
//MC1_CTL_210: .double 0x00100e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC1_CTL_220: .double 0x00100e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC1_CTL_230: .double 0x00200e8000120e80

//150M MC1
//MC1_CTL_1f0: .double 0x00160e8000000000
//MC1_CTL_200: .double 0x00100e8000100e80
//MC1_CTL_210: .double 0x00100e8000160e80
////00000000001000000000111010000000 dll_ctrl_reg_0_4(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_3(RW) 
//MC1_CTL_220: .double 0x00160e8000120e80
////00000000001000000000111010000000 dll_ctrl_reg_0_6(RW) 00000000001000000000111010000000 dll_ctrl_reg_0_5(RW) 
//MC1_CTL_230: .double 0x00200e8000100e80
#endif
MC1_CTL_240: .double 0x00000e0000000e00
//00000000000000000000111000000000 dll_ctrl_reg_1_1(RW) 00000000000000000000111000000000 dll_ctrl_reg_1_0(RW) 
MC1_CTL_250: .double 0x00000e0000000e00
MC1_CTL_260: .double 0x0000100000000e00
MC1_CTL_270: .double 0x00000e0000000e00
MC1_CTL_280: .double 0x0000000000000e00
MC1_CTL_290: .double 0x0000000000000000
MC1_CTL_2a0: .double 0x0000000000000000
MC1_CTL_2b0: .double 0x0000000000000000
MC1_CTL_2c0: .double 0x0000000000000000
//MC1_CTL_2d0: .double 0xf300494603c0019d
MC1_CTL_2d0: .double 0xf3004946003c099c
//11110100000000000011101100100111 phy_ctrl_reg_0_0(RD) 000000_00000000000000000110011101 pad_ctrl_reg_0(RW) 
MC1_CTL_2e0: .double 0xf3004946f3004946
MC1_CTL_2f0: .double 0xf3004946f3004946
MC1_CTL_300: .double 0xf3004946f3004946
MC1_CTL_310: .double 0xf3004946f3004946
//MC1_CTL_2d0: .double 0xf3005a470000019d
////11110100000000000011101100100111 phy_ctrl_reg_0_0(RD) 000000_00000000000000000110011101 pad_ctrl_reg_0(RW) 
//MC1_CTL_2e0: .double 0xf3005a47f3005a47
//MC1_CTL_2f0: .double 0xf3005a47f3005a47
//MC1_CTL_300: .double 0xf3005a47f3005a47
//MC1_CTL_310: .double 0xf3005a47f3005a47
MC1_CTL_320: .double 0x0680000006800000
MC1_CTL_330: .double 0x0680000006800000
MC1_CTL_340: .double 0x0680000006800000
MC1_CTL_350: .double 0x0680000006800000
MC1_CTL_360: .double 0x0800c00506800000
//00000000000000001100000000000101 phy_ctrl_reg_2(RW) 00000111110000000000001100000001 phy_ctrl_reg_1_8(RD) 
MC1_CTL_370: .double 0x0000000000000000
MC1_CTL_380: .double 0x0000000000000000
MC1_CTL_390: .double 0x0000000000000000
MC1_CTL_3a0: .double 0x0000000000000000
MC1_CTL_3b0: .double 0x0000000000000000
MC1_CTL_3c0: .double 0x0000000000000000
MC1_CTL_3d0: .double 0x0000000000000000
MC1_CTL_3e0: .double 0x0000000000000000
MC1_CTL_3f0: .double 0x0000000000000000
MC1_CTL_400: .double 0x0000000000000000
MC1_CTL_410: .double 0x0000000000000000
MC1_CTL_420: .double 0x0000000000000000
MC1_CTL_430: .double 0x0000000000000000
MC1_CTL_440: .double 0x0000000000000000
MC1_CTL_450: .double 0x0000000000000000
MC1_CTL_460: .double 0x0000000000000000
MC1_CTL_470: .double 0x0000000000000000
MC1_CTL_480: .double 0x0000000000000000
MC1_CTL_490: .double 0x0000000000000000
MC1_CTL_4a0: .double 0x0000000000000000
MC1_CTL_4b0: .double 0x0000000000000000
MC1_CTL_4c0: .double 0x0000000000000000
MC1_CTL_4d0: .double 0x0000000000000000
MC1_CTL_4e0: .double 0x0000000000000000
MC1_CTL_4f0: .double 0x0000000000000000
MC1_CTL_500: .double 0x0000000000000000
MC1_CTL_510: .double 0x0000000000000000
MC1_CTL_520: .double 0x0000000000000000
MC1_CTL_530: .double 0x0000000000000000
MC1_CTL_540: .double 0x0000000000000000
MC1_CTL_550: .double 0x0000000000000000
MC1_CTL_560: .double 0x0000000000000000
MC1_CTL_570: .double 0x0000000000000000
MC1_CTL_580: .double 0x0000000000000000
MC1_CTL_590: .double 0x0000000000000000
MC1_CTL_5a0: .double 0x0000000000000000
MC1_CTL_5b0: .double 0x0000000000000000
MC1_CTL_5c0: .double 0x0000000000000000
MC1_CTL_5d0: .double 0x0000000000000000
MC1_CTL_5e0: .double 0x0000000000000000
MC1_CTL_5f0: .double 0x0000000000000000
MC1_CTL_600: .double 0x0000000000000000
MC1_CTL_610: .double 0x0000000000000000
MC1_CTL_620: .double 0x0000000000000000
MC1_CTL_630: .double 0x0000000000000000
MC1_CTL_640: .double 0x0000000000000000
MC1_CTL_650: .double 0x0000000000000000
MC1_CTL_660: .double 0x0000000000000000
MC1_CTL_670: .double 0x0000000000000000
MC1_CTL_680: .double 0x0000000000000000
MC1_CTL_690: .double 0x0000000000000000
MC1_CTL_6a0: .double 0x0000000000000000
MC1_CTL_6b0: .double 0x0000000000000000
MC1_CTL_6c0: .double 0x0000000000000000
MC1_CTL_6d0: .double 0x0000000000000000
MC1_CTL_6e0: .double 0x0000000000000000
MC1_CTL_6f0: .double 0x0000000000000000
MC1_CTL_700: .double 0x0000000000000000
MC1_CTL_710: .double 0x0000000000000000
MC1_CTL_720: .double 0x0000000000000000
MC1_CTL_730: .double 0x0000000000000000
MC1_CTL_740: .double 0x0100000000000000
//MC1_CTL_750: .double 0x0100000101020101
MC1_CTL_750: .double 0x0101000101020101
//000000_01 wrlvl_cs(RW) 000000_00 sw_leveling_mode(RW) 000000_00 rdlvl_cs(RW) 000000_01 axi2_w_priority(RW) 000000_01 axi2_r_priority(RW) 000000_10 axi2_port_ordering(RW) 000000_01 axi1_w_priority(RW) 000000_01 axi1_r_priority(RW) 
MC1_CTL_760: .double 0x0303030000020001
//0000_0011 axi0_priority2_relative_priority(RW) 0000_0011 axi0_priority1_relative_priority(RW) 0000_0011 axi0_priority0_relative_priority(RW) 0000_0000 address_mirroring(RW) 00000_000 tdfi_dram_clk_disable(RW) 00000_010 bstlen(RW) 000000_00 zq_req(WR) 000000_01 zq_on_sref_exit(RW) 
MC1_CTL_770: .double 0x0101010202020203
//0000_0001 axi2_priority2_relative_priority(RW) 0000_0001 axi2_priority1_relative_priority(RW) 0000_0001 axi2_priority0_relative_priority(RW) 0000_0010 axi1_priority3_relative_priority(RW) 0000_0010 axi1_priority2_relative_priority(RW) 0000_0010 axi1_priority1_relative_priority(RW) 0000_0010 axi1_priority0_relative_priority(RW) 0000_0011 axi0_priority3_relative_priority(RW) 
MC1_CTL_780: .double 0x0102020400040001
//0000_0001 tdfi_dram_clk_enable(RW) 0000_0010 tdfi_ctrl_delay(RW) 0000_0010 rdlvl_gate_dq_zero_count(RW) 0000_0100 rdlvl_dq_zero_count(RW) 0000_0000 lowpower_refresh_enable(RW) 0000_0110 dram_class(RW) 0000_1100 burst_on_fly_bit(RW) 0000_0001 axi2_priority3_relative_priority(RW) 
MC1_CTL_790: .double 0x281900000f000303
//00_101000 wlmrd(RW) 00_011001 wldqsen(RW) 000_00000 lowpower_control(RW) 000_00000 lowpower_auto_enable(RW) 0000_1111 zqcs_chip(RW) 0000_0000 wrr_param_value_err(RD) 0000_0011 tdfi_wrlvl_dll(RW) 0000_0011 tdfi_rdlvl_dll(RW) 
MC1_CTL_7a0: .double 0x00000000000000ff
MC1_CTL_7b0: .double 0x0000000000000000
MC1_CTL_7c0: .double 0x0000000000000000
MC1_CTL_7d0: .double 0x0000000000000000
MC1_CTL_7e0: .double 0x0000000000000000
//00000000 rdlvl_gate_delay_2(RD) 00000000 rdlvl_gate_delay_1(RD) 00000000 rdlvl_gate_delay_0(RD) 00000000 rdlvl_gate_clk_adjust_8(RW) 00000000 rdlvl_gate_clk_adjust_7(RW) 00000000 rdlvl_gate_clk_adjust_6(RW) 00000000 rdlvl_gate_clk_adjust_5(RW) 00000000 rdlvl_gate_clk_adjust_4(RW) 
MC1_CTL_7f0: .double 0xff08000000000000
//11111111 rdlvl_max_delay(RW) 00001000 rdlvl_gate_max_delay(RW) 00000000 rdlvl_gate_delay_8(RD) 00000000 rdlvl_gate_delay_7(RD) 00000000 rdlvl_gate_delay_6(RD) 00000000 rdlvl_gate_delay_5(RD) 00000000 rdlvl_gate_delay_4(RD) 00000000 rdlvl_gate_delay_3(RD) 
MC1_CTL_800: .double 0x0000000000000000
//00000000 rdlvl_midpoint_delay_7(RD) 00000000 rdlvl_midpoint_delay_6(RD) 00000000 rdlvl_midpoint_delay_5(RD) 00000000 rdlvl_midpoint_delay_4(RD) 00000000 rdlvl_midpoint_delay_3(RD) 00000000 rdlvl_midpoint_delay_2(RD) 00000000 rdlvl_midpoint_delay_1(RD) 00000000 rdlvl_midpoint_delay_0(RD) 
MC1_CTL_810: .double 0x000000000000000e
//00000000 rdlvl_offset_delay_6(RW) 00000000 rdlvl_offset_delay_5(RW) 00000000 rdlvl_offset_delay_4(RW) 00000000 rdlvl_offset_delay_3(RW) 00000000 rdlvl_offset_delay_2(RW) 00000000 rdlvl_offset_delay_1(RW) 00000000 rdlvl_offset_delay_0(RW) 00000000 rdlvl_midpoint_delay_8(RD) 
MC1_CTL_820: .double 0x0420000c20400000
//00000100 tdfi_wrlvl_resplat(RW) 00000000 tdfi_wrlvl_resp(RD) 00000000 tdfi_rdlvl_rr(RW) 00001100 tdfi_rdlvl_resplat(RW) 00000000 tdfi_rdlvl_resp(RD) 01000000 refresh_per_zq(RW) 00000000 rdlvl_offset_delay_8(RW) 00000000 rdlvl_offset_delay_7(RW) 
//MC1_CTL_830: .double 0x0000000000000c0a
//MC1_CTL_830: .double 0x1313131313130c0a
MC1_CTL_830: .double 0x0f29292929290c0a
//00000000 wrlvl_delay_5(RD) 00000000 wrlvl_delay_4(RD) 00000000 wrlvl_delay_3(RD) 00000000 wrlvl_delay_2(RD) 00000000 wrlvl_delay_1(RD) 00000000 wrlvl_delay_0(RD) 00000010 tmod(RW) 00001010 tdfi_wrlvl_ww(RW) 
//MC1_CTL_840: .double 0x0000640064001313
MC1_CTL_840: .double 0x0000640064001515 # 3A2
//00000000000000_0001100100 axi1_priority_relax(RW) 000000_0001100100 axi0_priority_relax(RW) 00000000 wrlvl_delay_8(RD) 00000000 wrlvl_delay_7(RD) 00000000 wrlvl_delay_6(RD) 
MC1_CTL_850: .double 0x0000000000000064
MC1_CTL_860: .double 0x0200004000000000
MC1_CTL_870: .double 0x0000000000000046
//0_000000000000010 emrs1_data_3(RW) 0_000000000000010 emrs1_data_2(RW) 0_000000000000010 emrs1_data_1(RW) 0_000000000000010 emrs1_data_0(RW) 
MC1_CTL_880: .double 0x0000000000000000
//MC1_CTL_890 : .double 0x0a520a520a520a52
MC1_CTL_890: .double 0x0a520a520a520a5a
//0_000010000010000 mrs_data_3(RW) 0_000010000010000 mrs_data_2(RW) 0_000010000010000 mrs_data_1(RW) 0_000010000010000 mrs_data_0(RW) 
MC1_CTL_8a0: .double 0x00000000001c001c
MC1_CTL_8b0: .double 0x0000000000000000
MC1_CTL_8c0: .double 0x0004000000000000
MC1_CTL_8d0: .double 0x00000000c8000000
MC1_CTL_8e0: .double 0x0000000000000050
//MC1_CTL_8f0: .double 0x000000000a150080 //clk skew cleared
//MC1_CTL_8f0: .double 0x000000000a100080 //clk skew cleared
//MC1_CTL_8f0: .double 0x000000002b352180
//MC1_CTL_8f0: .double 0x000000001b251180
//MC1_CTL_8f0: .double 0x0000000040404080
MC1_CTL_8f0: .double 0x0000000010141380 //clk skew of 3A2 0.4
//0000000000000000000000000111100_000000000000000000000000001111000 dll_ctrl_reg_2(RW) 
MC1_CTL_900: .double 0x0000000000000000
MC1_CTL_910: .double 0x0000000000000000
MC1_CTL_920: .double 0x0000000000000000
MC1_CTL_930: .double 0x0000000000000000
MC1_CTL_940: .double 0x0300000000050500
MC1_CTL_950: .double 0x0000000000000a03
MC1_CTL_960: .double 0x0604000100000000
//000_00101 rdlat_adj(RW) 0000_0100 wrlat_adj(RW) 0000000_0 swlvl_start(WR) 0000000_0 swlvl_load(WR) 0000000_0 swlvl_exit(WR) 000000_000000000000000000 int_status(RD) 
MC1_CTL_970: .double 0x000000000003e805
MC1_CTL_980: .double 0x0001000001000000
//0000000_0 zq_in_progress(RD) 0000000_1 zqcs_rotate(RW) 0000000_0 wrlvl_reg_en(RW) 0000000_0 wrlvl_en(RW) 0000000_1 resync_dll_per_aref_en(RW) 0000000_0 resync_dll(WR) 0000000_0 rdlvl_reg_en(RW) 0000000_0 rdlvl_gate_reg_en(RW) 
MC1_CTL_990: .double 0x0001020202000100
//00000_000 w2w_samecs_dly(RW) 00000_001 w2w_diffcs_dly(RW) 00000_010 tbst_int_interval(RW) 00000_010 r2w_samecs_dly(RW) 00000_010 r2w_diffcs_dly(RW) 00000_000 r2r_samecs_dly(RW) 00000_001 r2r_diffcs_dly(RW) 00000_000 axi_aligned_strobe_disable(RW) 
MC1_CTL_9a0: .double 0x0707030200060100
//00000111 tdfi_wrlvl_load(RW) 00000111 tdfi_rdlvl_load(RW) 000_00011 tckesr(RW) 000_00010 tccd(RW) 000_00000 add_odt_clk_difftype_diffcs(RW) 0000_0110 trp_ab(RW) 0000_0001 add_odt_clk_sametype_diffcs(RW) 0000_0000 add_odt_clk_difftype_samecs(RW) 
MC1_CTL_9b0: .double 0x02000100000a000f
//0000_001000000000 zqinit(RW) 0000_000100000000 zqcl(RW) 000000_0000001010 tdfi_wrlvl_ww(RW) 000000_0000001111 tdfi_rdlvl_rr(RW) 
MC1_CTL_9c0: .double 0x0a520c2d0c2d0c2d
//MC1_CTL_9c0: .double 0x04100c2d0c2d0c2d
//0_000101001010010 mr0_data_0(RW) 00_00110000101101 tdfi_phyupd_type3(RW) 00_00110000101101 tdfi_phyupd_type2(RW) 00_00110000101101 tdfi_phyupd_type1(RW) 
//MC1_CTL_9d0: .double 0x0004041004100410
MC1_CTL_9d0: .double 0x00440a520a520a52
//0_000000000000100 mr1_data_0(RW) 0_000101001010010 mr0_data_3(RW) 0_000101001010010 mr0_data_2(RW) 0_000101001010010 mr0_data_1(RW) 
MC1_CTL_9e0: .double 0x0000004400440044
//0_000000000000000 mr2_data_0(RW) 0_000000000000100 mr1_data_3(RW) 0_000000000000100 mr1_data_2(RW) 0_000000000000100 mr1_data_1(RW) 
MC1_CTL_9f0: .double 0x0000000000000000
//0_000000000000000 mr3_data_0(RW) 0_000000000000000 mr2_data_3(RW) 0_000000000000000 mr2_data_2(RW) 0_000000000000000 mr2_data_1(RW) 
MC1_CTL_a00: .double 0x00ff000000000000
//0000000011111111 dfi_wrlvl_max_delay(RW) 0_000000000000000 mr3_data_3(RW) 0_000000000000000 mr3_data_2(RW) 0_000000000000000 mr3_data_1(RW) 
MC1_CTL_a10: .double 0x0000000000000000
//0000000000000000 rdlvl_begin_delay_3(RD) 0000000000000000 rdlvl_begin_delay_2(RD) 0000000000000000 rdlvl_begin_delay_1(RD) 0000000000000000 rdlvl_begin_delay_0(RD) 
MC1_CTL_a20: .double 0x0000000000000000
//0000000000000000 rdlvl_begin_delay_7(RD) 0000000000000000 rdlvl_begin_delay_6(RD) 0000000000000000 rdlvl_begin_delay_5(RD) 0000000000000000 rdlvl_begin_delay_4(RD) 
MC1_CTL_a30: .double 0x0e0e0e0e0e0e0000
//0000111000001110 rdlvl_delay_2(RW) 0000111000001110 rdlvl_delay_1(RW) 0000111000001110 rdlvl_delay_0(RW) 0000000000000000 rdlvl_begin_delay_8(RD) 
MC1_CTL_a40: .double 0x0e0e0e0e0e0e0e0e
//0000111000001110 rdlvl_delay_6(RW) 0000111000001110 rdlvl_delay_5(RW) 0000111000001110 rdlvl_delay_4(RW) 0000111000001110 rdlvl_delay_3(RW) 
MC1_CTL_a50: .double 0x000000000e0e0e0e
//0000000000000000 rdlvl_end_delay_1(RD) 0000000000000000 rdlvl_end_delay_0(RD) 0000111000001110 rdlvl_delay_8(RW) 0000111000001110 rdlvl_delay_7(RW) 
MC1_CTL_a60: .double 0x0000000000000000
//0000000000000000 rdlvl_end_delay_5(RD) 0000000000000000 rdlvl_end_delay_4(RD) 0000000000000000 rdlvl_end_delay_3(RD) 0000000000000000 rdlvl_end_delay_2(RD) 
MC1_CTL_a70: .double 0x0000000000000000
//0000000000000000 rdlvl_gate_delay_0(RW+) 0000000000000000 rdlvl_end_delay_8(RD) 0000000000000000 rdlvl_end_delay_7(RD) 0000000000000000 rdlvl_end_delay_6(RD) 
MC1_CTL_a80: .double 0x0000000000000000
//0000000000000000 rdlvl_gate_delay_4(RW+) 0000000000000000 rdlvl_gate_delay_3(RW+) 0000000000000000 rdlvl_gate_delay_2(RW+) 0000000000000000 rdlvl_gate_delay_1(RW+) 
MC1_CTL_a90: .double 0x0000000000000000
//0000000000000000 rdlvl_gate_delay_8(RW+) 0000000000000000 rdlvl_gate_delay_7(RW+) 0000000000000000 rdlvl_gate_delay_6(RW+) 0000000000000000 rdlvl_gate_delay_5(RW+) 
MC1_CTL_aa0: .double 0x0000ffff00000000
//0000000000000000 rdlvl_midpoint_delay_0(RD) 1111111111111111 rdlvl_max_delay(RW) 0000000000000000 rdlvl_gate_refresh_interval(RW) 0000000000010000 rdlvl_gate_max_delay(RW) 
MC1_CTL_ab0: .double 0x0000000000000000
//0000000000000000 rdlvl_midpoint_delay_4(RD) 0000000000000000 rdlvl_midpoint_delay_3(RD) 0000000000000000 rdlvl_midpoint_delay_2(RD) 0000000000000000 rdlvl_midpoint_delay_1(RD) 
MC1_CTL_ac0: .double 0x0000000000000000
//0000000000000000 rdlvl_midpoint_delay_8(RD) 0000000000000000 rdlvl_midpoint_delay_7(RD) 0000000000000000 rdlvl_midpoint_delay_6(RD) 0000000000000000 rdlvl_midpoint_delay_5(RD) 
MC1_CTL_ad0: .double 0x0000000000000000
//0000000000000000 rdlvl_offset_delay_3(RW) 0000000000000000 rdlvl_offset_delay_2(RW) 0000000000000000 rdlvl_offset_delay_1(RW) 0000000000000000 rdlvl_offset_delay_0(RW) 
MC1_CTL_ae0: .double 0x0000000000000000
//0000000000000000 rdlvl_offset_delay_7(RW) 0000000000000000 rdlvl_offset_delay_6(RW) 0000000000000000 rdlvl_offset_delay_5(RW) 0000000000000000 rdlvl_offset_delay_4(RW) 
MC1_CTL_af0: .double 0x0020002000000000
//0000000000000000 wrlvl_delay_1(RW+) 0000000000000000 wrlvl_delay_0(RW+) 0000000000000000 rdlvl_refresh_interval(RW) 0000000000000000 rdlvl_offset_delay_8(RW) 
MC1_CTL_b00: .double 0x0020002000200020
//0000000000000000 wrlvl_delay_5(RW+) 0000000000000000 wrlvl_delay_4(RW+) 0000000000000000 wrlvl_delay_3(RW+) 0000000000000000 wrlvl_delay_2(RW+) 
MC1_CTL_b10: .double 0x0000000000200020
//0000000000000000 wrlvl_refresh_interval(RW) 0000000000000000 wrlvl_delay_8(RW+) 0000000000000000 wrlvl_delay_7(RW+) 0000000000000000 wrlvl_delay_6(RW+) 
MC1_CTL_b20: .double 0x00000c2d00000c2d
//00000000000000000000110000101101 tdfi_rdlvl_resp(RW) 00000000000000000000110000101101 tdfi_rdlvl_max(RW) 
MC1_CTL_b30: .double 0x00000c2d00000000
//00000000000000000000110000101101 tdfi_wrlvl_resp(RW) 00000000000000000000000000000000 tdfi_wrlvl_max(RW) 
MC1_CTL_start_DATA_LO: .word  0x00000000
//0000000_1 rw_same_en 0000000_0 reg_dimm_enable 0000000_0 reduc 0000000_0 pwrup_srefresh_exit
MC1_CTL_start_DATA_HI: .word  0x01010100
//0000000_1 swap_port_rw_same_en 0000000_1 swap_en 0000000_0 start 0000000_0 srefresh
#endif


