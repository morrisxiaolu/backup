/* sibyte/config.h - SB1 configuration header */

/*
 * Copyright (c) 2002-2007 Wind River Systems, Inc.
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
03d,07nov07,slk  add define INCLUDE_VXB_LEGACY_INTERRUPTS for legacy
                 interrupt API support.
03c,28aug07,jmg  Moved VX_SMP_NUM_CPUS to cdf files.
03b,09aug07,rlg  Changes for the MIPS Library restructuring
03a,27jul07,ami  macro INCLUDE_VXB_CMDLINE added
02z,15jun07,bwa  added DSHM_INT_VAL.
02y,14jun07,slk  fix shared memory interface
02x,05jun07,slk  defect 86392: fix CFEARGS define for AMP support and
                 add EXC_PAGE_PHYS_ADRS define for AMP mapped
02w,24may07,slk  fix CONSOLE_TTY define for AMP
02v,27apr07,slk  Move MIPS_CPU_NUM_INTERRUPT_INPUTS to config.h
02u,04apr07,slk  add MIPS interrupt controller defines
02t,05mar07,ami  Macros added for vxbus compatible timer drivers
02t,01mar07,rlg  updates for latest broadcom code drop
02s,08feb07,h_k  enabled DRV_SIO_SB1.
02r,05jan07,rlg  fix compiler warning as per phil's instructions
02q,22dec06,jmt  Add INCLUDE_AUX_CLK as a default component
02p,30nov06,rlg  update bsp rev for NOR
02o,09nov06,pes  Fix makedepend warning/error by moving board type decision
                 tree into sibyte.h.
02n,20oct06,jmt  Update version number
02m,17oct06,jmt  convert 1250a/e to default to vxWorks not CFE
02l,01nov06,pes  Add definition of VX_SMP_NUM_CPUS.
02k,25oct06,pes  SMP merge.
02j,19sep06,rlg  Test for CFE romable images
02i,09sep06,jmt  Fix define for BCM1250_SHARED_SOURCE
02h,28aug06,rlg  dual core issues on 1480 - modified SBE defines
02g,18jul06,jmt  Add SYS_MODEL define
02f,07jul06,wap  Add VxBus support, switch to new sbeVxbEnd driver
02e,12aug06,pes  Remove MMU_EXC_TLB_PAGE_BASE.
02d,08aug06,pes  Increase default memory size from 16 to 32 meg.
02f,22sep06,pmr  add support for DRV_SIO_SB1
02c,12may06,pes  Remove override of EXC_PAGE_PHYS_ADRS for CPU1.
02b,10apr06,pes  Change cache modes from CACHE_WRITETHROUGH to CACHE_COPYBACK.
02a,26feb06,mem  Added INCLUDE_CFE_SUPPORT, 1125 support, etc.
01z,07feb06,agf  update revision
01y,12jan06,jmt  Added SYS_MODEL define
01x,28nov05,agf  move defines that were being set after inclusion of prjParams.h
01w,23aug05,jmt  Componentize BSP for scalability
01v,01aug05,h_k  added VXLAYER.
01u,31oct04,pes  General cleanup and removal of unneeded conditional code,
                 made possible by changing MIPS default of SW_MMU_ENABLE.
01t,28oct04,pes  The default setting of SW_MMU_ENABLE has been changed
                 for MIPS from FALSE to TRUE. This requires a change
		 in the logic to set it to FALSE if INCLUDE_MAPPED_KERNEL
		 is defined.
01s,07oct04,agf  addt'l vxWorks 6.0 clean-up
01r,06oct04,pes  Change INCLUDE_MMU_BASIC to INCLUDE_MAPPED_KERNEL. Add
                 suport for MMU-less RTPs.
01q,30aug04,j_b  remove INCLUDE_NETWORK dependencies (SPR #99747)
01q,30jul04,md   Use default PM_RESERVED_MEM 
01p,03aug04,agf  change MMU include from FULL to BASIC
01o,04may04,agf  port to Base6
01n,15nov02,agf  increment BSP_REV
01m,03oct02,agf  add ifdefs for selecting between cpu0 & cpu1 constants
01l,18jul02,agf  correct ROM_SIZE to be 2M
01k,17jul02,agf  update REV number for T2.2 FCS
01j,20jun02,pgh  Make the definition of NUM_TTY not dependent on bcm1250.h.
01i,10may02,tlc  Add C++ header protection.
01h,30apr02,agf  remove WDB_COMM overrides since WDB END has been fixed (SPR
                 73331)
01g,28mar02,tlc  Update revision information.
01f,13mar02,agf  increase SYS_CLK_RATE_MAX to 5000, SPR 74129
                 remove undefs of INCLUDE_SM_OBJ/NET, SPR 74321
                 move SM anchor address into cached space, SPR 74346
01e,12dec01,ros  remove INCLUDE_SM_OBJ and VXMP for shipment
01d,11dec01,agf  fix DEFAULT_BOOT_LINE
01c,10dec01,agf  minor tweaking
01b,06dec01,agf  remove extraneous component INCLUDE's
01a,15nov01,agf  written
*/

/*
This file contains the configuration parameters for the
Broadcom SB1-based evaluation board.
*/

#ifndef	__INCconfigh
#define	__INCconfigh

#ifdef __cplusplus
extern "C" {
#endif

/* BSP version/revision identification, before configAll.h */
#define BSP_VER_1_1     1
#define BSP_VER_1_2     1
#define BSP_VERSION     "2.0"   /* vxWorks 6.0 compatible */
#define BSP_REV         "/10"    /* 0 for first revision */
/* includes */
#include <configAll.h>
#include <malta.h> /* added by yinwx, 20100106 */
#include <arch/mips/mmuMipsLib.h>
#include "sibyte.h"
#include "sb1.h"

/* SYS_MODEL define for card.  
 * This defines the beginning of the sysModel string.
 */
#define SYS_MODEL   "Broadcom BCM"

/* Default Components */
#define INCLUDE_AUX_CLK
/*
 * VxBus driver support. Note that the pool size may need to
 * be increased if additional interfaces are enabled.
 * also INCLUDE_VXBUS must always be defined for this BSP
 */
#define INCLUDE_VXBUS

#ifdef INCLUDE_VXBUS
#define INCLUDE_HWMEM_ALLOC
#define HWMEM_POOL_SIZE 100000
#define INCLUDE_PLB_BUS
#define INCLUDE_MII_BUS
#define INCLUDE_DMA_SYS
#define INCLUDE_PARAM_SYS
#undef  INCLUDE_SBE_VXB_END
#undef  INCLUDE_BCM54XXPHY
#define INCLUDE_GENERICPHY
#define INCLUDE_VXB_CMDLINE 
/* add MIPS R4K timer */
#define INCLUDE_TIMER_SYS
#define DRV_TIMER_MIPSR4K

#ifdef INCLUDE_AUX_CLK
    #define DRV_TIMER_SB1
#endif

/* #define INCLUDE_NET_BOOT_CONFIG   2012.8.28 mxl added */

/* vxBus SB1 Serial Driver */

#define INCLUDE_SIO_UTILS
#ifdef INCLUDE_SIO_UTILS
    #define DRV_SIO_NS16550
#endif	/* INCLUDE_SIO_UTILS */

/* add the sibyte and mips vxbus interrupt controllers */

#define INTCTLR_LIB_DEBUG
#define DRV_INTCTLR_MIPS
#define DRV_INTCTLR_MIPS_SBE    
#undef  DRV_INTCTLR_MIPS_SBE
#define INCLUDE_INTCTLR_LIB
#define INCLUDE_VXB_LEGACY_INTERRUPTS

#endif  /* INCLUDE_VXBUS */

#if defined (SB1_CPU_0) || defined (SB1_CPU_1) || \
	defined (SB1_CPU_2) || 	defined (SB1_CPU_3)
#define BCM1250_SHARED_SOURCE
#endif /* SB1_CPU_x */

#undef  _SIMULATOR_             /* define when running on the bcm1250 simulator */

#if (BOARD_TYPE == BCM91480B) 
/* #define INCLUDE_CFE_SUPPORT  del by mxl */	/* always true for 1480 */
#endif /* BCM91480B */
#if  (BOARD_TYPE == BCM91250A) || (BOARD_TYPE == BCM91250E)
#undef  INCLUDE_CFE_SUPPORT	/* could be turned on if want CFE boot */
#endif /* BCM91250A/E */

#define _WRS_MIPS_HARDWARE_CACHE_COHERENCY

#define SYS_LED_DISPLAY(a,b,c,d)    do { sysLedDsply(((a)<<24)|((b)<<16)|((c)<<8)|(d)); } while ((0))

/* Network driver configuration */

/* optional PCI-based DEC 21x40 END driver */
/* note: this is normally INCLUDE_DC, but that causes problems here */
#undef  INCLUDE_DEC

/* SCSI configuration */

#undef  INCLUDE_SCSI            /* SCSI support */
#undef  INCLUDE_SCSI2           /* SCSI2 support */


/* CPU-specific facilities */

#undef  USER_D_CACHE_MODE
#define USER_D_CACHE_MODE	CACHE_COPYBACK
#undef  USER_I_CACHE_MODE
#define USER_I_CACHE_MODE	CACHE_COPYBACK

#define INCLUDE_CACHE_SUPPORT
#define USER_I_CACHE_ENABLE
#define USER_D_CACHE_ENABLE


/* MMU configuration
 *
 * MIPS_TLB_PAGE_SIZES is a bitmask that defines the supported MMU Page
 * Sizes for this BSP.  The following Page Sizes are supported:
 *   MMU_PAGE_MASK_8K,   MMU_PAGE_MASK_32K, MMU_PAGE_MASK_128K,
 *   MMU_PAGE_MASK_512K, MMU_PAGE_MASK_2M
 * VM_PAGE_SIZE must be set to the minimum page size define in
 * MIPS_TLB_PAGE_SIZES.
 */
#if defined(INCLUDE_MAPPED_KERNEL)
#define MIPS_TLB_PAGE_SIZES		MMU_PAGE_MASK_8K
#undef  VM_PAGE_SIZE
#define VM_PAGE_SIZE			TLB_8K_PAGE_SIZE
#undef  INCLUDE_EXC_SHOW
#define MMU_USE_EXC_TLB_PAGE		TRUE

/* get a multiplier for calculating the physical addr of the -1 page */
#if defined (SB1_CPU_0)
#define SB1_CORE			0
#elif defined (SB1_CPU_1)
#define SB1_CORE			1
#elif defined (SB1_CPU_2)
#define SB1_CORE			2
#elif defined (SB1_CPU_3)
#define SB1_CORE			3
#else
#define SB1_CORE			0 /* for sb1480_mipsi64 BSP use */
#endif

/* set EXC_PAGE_PHYS_ADRS to calculate physical address for -1 page */
#ifdef EXC_PAGE_PHYS_ADRS
#undef EXC_PAGE_PHYS_ADRS
#define EXC_PAGE_PHYS_ADRS		(0x2000 * (1 + SB1_CORE))
#endif /* EXC_PAGE_PHYS_ADRS */

#define MMU_EXC_TLB_VIRT_BASE		0xffffe000

#define R4K_MMU_CACHEABLE		3
#define R4K_MMU_UNCACHEABLE		2
#define R4K_MMU_CACHE_COPYBACK		3
#define R4K_MMU_CACHE_WRITETHROUGH	1
#define R4K_MMU_CACHE_COHERENCY		5
#endif /* INCLUDE_MAPPED_KERNEL */

/* SBE MAC & PCI allocation */

/* start with everything undefined */
#undef INCLUDE_SBE0
#undef INCLUDE_SBE1
#undef INCLUDE_SBE2
#undef INCLUDE_SBE3
#undef INCLUDE_PCI

#if (BOARD_TYPE == BCM91250A) || \
	(BOARD_TYPE == BCM91250E) 
#if defined (SB1_CPU_2)
#error "SB1_CPU_2: No such CPU"
#elif defined (SB1_CPU_3)
#error "SB1_CPU_3: No such CPU"
#endif /* SB1_CPU_2/3 */
#endif /* BCM91250A/E */

#if 0 
#if defined (SB1_CPU_0)
# define INCLUDE_SBE0
# define INCLUDE_PCI
#elif defined (SB1_CPU_1)
# define INCLUDE_SBE1
#elif defined (SB1_CPU_2)
# define INCLUDE_SBE2
#elif defined (SB1_CPU_3)
# define INCLUDE_SBE3
#else /* SB1_CPU_x sb1480_mipsi64 BSP */
# define INCLUDE_SBE0
# define INCLUDE_SBE1
# if (BOARD_TYPE == BCM91480B)
#  define INCLUDE_SBE2
#  define INCLUDE_SBE3
# endif /* BCM91480B */
#endif /* SB1_CPU_x */
#endif 

/* vxBus PCI support not yet implemented */ 

#undef INCLUDE_PCI              /* added by moxiaolu  */


/* the following three drivers are not applicable for a busless board */
#undef  INCLUDE_EGL             /* remove EGL driver */
#undef  INCLUDE_ENP             /* remove ENP driver */

/* Optional timestamp support for WindView - undefined by default */

#undef INCLUDE_TIMESTAMP

/* Algorithmics NVRAM */
#undef ALGCOMPAT


/* miscellaneous definitions */
#if defined (SB1_CPU_0)
#define DEFAULT_BOOT_LINE \
   "sbe(0,0)host:/usr/vw/config/sb1xxx_0_mipsi64/vxWorks h=90.0.0.3 e=90.0.0.50 u=target"
#elif defined (SB1_CPU_1)
#define DEFAULT_BOOT_LINE \
   "sbe(1,1)host:/usr/vw/config/sb1xxx_1_mipsi64/vxWorks h=90.0.0.3 e=90.0.0.51 u=target"
#elif defined (SB1_CPU_2)
#define DEFAULT_BOOT_LINE \
   "sbe(2,2)host:/usr/vw/config/sb1xxx_2_mipsi64/vxWorks h=90.0.0.3 e=90.0.0.52 u=target"
#elif defined (SB1_CPU_3)
#define DEFAULT_BOOT_LINE \
   "sbe(3,3)host:/usr/vw/config/sb1xxx_3_mipsi64/vxWorks h=90.0.0.3 e=90.0.0.53 u=target"
#else /* SB1_CPU_x sb1480_mipsi64 BSP */
#define DEFAULT_BOOT_LINE \
   "atse(0,0)host:vxWorks h=192.168.0.116:ffffff00 e=192.168.0.115:ffffff00 u=mxl pw=mxl"
#endif /* SB1_CPU_x */


#undef  NUM_TTY
#define NUM_TTY     3       /* 2 DUARTs and 1 JTAG */

#undef  CONSOLE_TTY
/* secondary CPU is on tty 1 */
#if defined (SB1_CPU_1)
#define CONSOLE_TTY 1    /* UART B */
#else /* primary on tty 0 */
#define CONSOLE_TTY 0    /* UART A */
#endif /* defined (SB1_CPU_0) */

/* assign UARTs and JTAG */
#undef INCLUDE_SB1_UART_CHAN_A
#undef INCLUDE_SB1_UART_CHAN_B
#undef INCLUDE_SB1_UART_CHAN_C
#undef INCLUDE_SB1_UART_CHAN_D

#if defined (SB1_CPU_0)
   #define INCLUDE_SB1_UART_CHAN_A
   #define SB1_UART_CHAN_A_IDX		0	/* tty 0 */
   #define SIO_POLL_CONSOLE	        SB1_UART_CHAN_A_IDX
#elif defined (SB1_CPU_1)
   #define INCLUDE_SB1_UART_CHAN_B
   #define SB1_UART_CHAN_B_IDX		0	/* tty 0 */
   #define SIO_POLL_CONSOLE		SB1_UART_CHAN_B_IDX
#elif defined (SB1_CPU_2)
   #define INCLUDE_SB1_UART_CHAN_C
   #define SB1_UART_CHAN_C_IDX		0	/* tty 0 */
   #define SIO_POLL_CONSOLE		SB1_UART_CHAN_C_IDX
#elif defined (SB1_CPU_3)
   #define INCLUDE_SB1_UART_CHAN_D
   #define SB1_UART_CHAN_D_IDX		0	/* tty 0 */
   #define SIO_POLL_CONSOLE		SB1_UART_CHAN_D_IDX
#else
   #define INCLUDE_SB1_UART_CHAN_A
   #define SB1_UART_CHAN_A_IDX		0	/* tty 0 */
   #define SIO_POLL_CONSOLE	        SB1_UART_CHAN_A_IDX

   #define INCLUDE_SB1_UART_CHAN_B
   #define SB1_UART_CHAN_B_IDX		1	/* tty 1 */
#if (BCM_FAMILY == BCM_SB1A)
   #define INCLUDE_SB1_UART_CHAN_C
   #define SB1_UART_CHAN_C_IDX		2	/* tty 2 */
   #define INCLUDE_SB1_UART_CHAN_D
   #define SB1_UART_CHAN_D_IDX		3	/* tty 3 */
#endif /* BCM_FAMILY == BCM_SB1A */

   #define SIO_POLL_CONSOLE		SB1_UART_CHAN_A_IDX
#endif /* SB1_CPU_x */

#undef  INCLUDE_SB1_JTAG_CHAN_A

#ifndef SB1_UART_DEFAULT_BAUD
#ifdef INCLUDE_CFE_SUPPORT
#define SB1_UART_DEFAULT_BAUD   115200
#else
#define SB1_UART_DEFAULT_BAUD  115200
#endif /* INCLUDE_CFE_SUPPORT */
#endif
#undef CONSOLE_BAUD_RATE
#define CONSOLE_BAUD_RATE       SB1_UART_DEFAULT_BAUD


#define SYS_CLK_RATE_MIN  1	/* minimum system clock rate */
#define SYS_CLK_RATE_MAX  5000	/* maximum system clock rate */
#define AUX_CLK_RATE_MIN  1	/* minimum auxiliary clock rate */
#define AUX_CLK_RATE_MAX  10000	/* maximum auxiliary clock rate */


/* allocate 240 bytes in NVRAM for the bootline */
// #define INCLUDE_LPC_FLASH   
#define INCLUDE_NVRAM
#undef BOOT_LINE_SIZE
#define	BOOT_LINE_SIZE	240	/* use 240 bytes for bootline */
#define NV_RAM_SIZE BOOT_LINE_SIZE

#ifdef  SB1_CPU_1
/* apply an offset so cpu0 & cpu1 do not overlap */
#undef BOOT_LINE_OFFSET
#undef EXC_MSG_OFFSET
#define BOOT_LINE_OFFSET        0x900
#define EXC_MSG_OFFSET          0xa00
#endif
#ifdef  SB1_CPU_2
/* apply an offset so cpu0 & cpu2 do not overlap */
#undef BOOT_LINE_OFFSET
#undef EXC_MSG_OFFSET
#define BOOT_LINE_OFFSET        0xb00
#define EXC_MSG_OFFSET          0xc00
#endif
#ifdef  SB1_CPU_3
/* apply an offset so cpu0 & cpu3 do not overlap */
#undef BOOT_LINE_OFFSET
#undef EXC_MSG_OFFSET
#define BOOT_LINE_OFFSET        0xd00
#define EXC_MSG_OFFSET          0xe00
#endif

/* memory constants */

#define LOCAL_MEM_SIZE		0x08000000	/* 64MB memory available to run-time kernel  64*4 MB */
#define USER_RESERVED_MEM      	0x01000000	/* mxl: 16MB number of reserved bytes */


/*
 * The constants ROM_TEXT_ADRS, ROM_SIZE, RAM_LOW_ADRS and
 * RAM_HIGH_ADRS are defined in config.h, and MakeSkel.
 * All definitions for these constants must be identical.
 */

//#ifdef INCLUDE_CFE_SUPPORT
//#define ROM_TEXT_ADRS		0xbfe00000      /* where ROM code loads */
//#define ROM_TEXT_ADRS_CFE	0xbfc00000	/* where cfe is at */
//#define CFEARGS			(((LOCAL_MEM_LOCAL_ADRS & ADDRESS_SPACE_MASK)|\
//				 K0BASE) + BOOT_LINE_OFFSET \
//				 + BOOT_LINE_SIZE + 16 )
//
//#else /* INCLUDE_CFE_SUPPORT */
#define CFEARGS			(((LOCAL_MEM_LOCAL_ADRS & ADDRESS_SPACE_MASK)|\
				 K0BASE) + BOOT_LINE_OFFSET \
				 + BOOT_LINE_SIZE + 16 )

#define ROM_TEXT_ADRS		0xbfc00000      /* base address of ROM */
//#endif /* INCLUDE_CFE_SUPPORT */

#define ROM_BASE_ADRS           ROM_TEXT_ADRS
#define ROM_SIZE                0x00200000      /* 2MB ROM space */

/* Shared memory configuration */
#undef  INCLUDE_VXMP_TESTS

#define SM_TAS_TYPE 		SM_TAS_HARD
#define SM_INT_TYPE		SM_INT_USER_1
#define SM_INT_ARG1		0x1   /* bit to use to set/clr mbox 3 int */
#define SM_INT_ARG2		0     /* not used */
#define SM_INT_ARG3		0     /* not used by this BSP */
#define SM_OBJ_MEM_ADRS 	NONE
#define SM_OBJ_MEM_SIZE 	0x80000  /* sh. mem Objects pool size 512K */

#undef	SM_ANCHOR_ADRS
#define SM_ANCHOR_ADRS  ((char *) 0x80000600)   /* on-board anchor adrs */
#define SM_MEM_ADRS     NONE            /* NONE = allocate sh. mem from pool */
#define SM_MEM_SIZE     0x80000                 /* 512K */
#define SM_OFF_BOARD    FALSE

/* distributed shared memory configuration */

#define DSHM_INT_VAL            SM_INT_ARG1

#define IP_MAX_UNITS 4

/* the sibyte interrupt controller has groups of 8 outputs connected
 * to each available cpu.  only outputs 0-4 from these group are used
 * and they map pins 0-4 to cpu input pins 2-6.  to uniquely
 * identify each sibyte controller output, the groups pins are numbered from
 * 0-7, 8-15, 16-23 and 24-31. since only 5 pins from each group are actually
 * used, pins 0-4 connect to cpu 0 pins 2-6, pins 8-12 connect to cpu 1
 * pins 2-6, pins 16-20 connect to cpu 2 pins 2-6 and pins 24-28 connect to
 * cpu 3 pins 2-6.
 */

#define MIPS_CPU_NUM_INTERRUPT_INPUTS 8    /* interrupt inputs into CPU */

/*
 * mapped kernels require that INCLUDE_MMU_BASIC is defined and
 * SW_MMU_ENABLE is set to FALSE.
 */
#if defined (INCLUDE_MAPPED_KERNEL)
#if !defined (INCLUDE_MMU_BASIC)
#define INCLUDE_MMU_BASIC
#endif /* INCLUDE_MMU_BASIC */
#undef SW_MMU_ENABLE
#define SW_MMU_ENABLE FALSE
#endif /* defined INCLUDE_MAPPED_KERNEL */


#ifdef __cplusplus
}
#endif
#endif	/* __INCconfigh */

#if defined(PRJ_BUILD)
#include "prjParams.h"
#endif /* defined PRJ_BUILD */

/* #define INCLUDE_SM_COMMON */
#define INCLUDE_USER_APPL         
#define USER_APPL_INIT            \
    {                             \
        IMPORT int sysHr1PciScan(); \
        sysHr1PciScan();            \
        /* taskSpawn ("userApp", 30, 0, 5120, userAppInit, 0x0, 0x1); */ \
    }

#if 1
#define	INCLUDE_TFFS
#define INCLUDE_TFFS_SHOW
#define INCLUDE_DISK_UTIL           /* ls, cd, mkdir, xcopy, etc. */
#define INCLUDE_DOSFS               /* usrDosFsOld.c wrapper layer */
#define INCLUDE_TL_FTL
#define INCLUDE_IO_SYSTEM
#define INCLUDE_DISK_UTIL

#define INCLUDE_BOOT_TFFS_LOADER
#define INCLUDE_TFFS_MOUNT

#if defined (INCLUDE_DOSFS)
    #define INCLUDE_DOSFS_MAIN      /* dosFsLib (2) */
    #define INCLUDE_DOSFS_FAT       /* dosFs FAT12/16/32 FAT table handler */
    #define INCLUDE_DOSFS_DIR_VFAT  /* Microsoft VFAT dirent handler */
    #define INCLUDE_DOSFS_DIR_FIXED /* 8.3 & VxLongNames directory handler */
    #define INCLUDE_DOSFS_FMT       /* dosFs2 file system formatting module */
    #define INCLUDE_DOSFS_CHKDSK    /* file system integrity checking */
    #define INCLUDE_CBIO            /* CBIO API module */
    #define INCLUDE_DISK_CACHE      /* CBIO API disk caching layer */
    #define INCLUDE_DISK_PART       /* disk partition handling code, fdisk... */
    #define INCLUDE_RAM_DISK        /* CBIO API ram disk driver */
#endif
#define INCLUDE_XBD_BLKDEV 
#endif


/* #define INCLUDE_ATSEMAC */
#define INCLUDE_ATSEMAC 

//#define INCLUDE_HRMATRIXDMA0
//#define INCLUDE_HRMATRIXDMA1
//#define INCLUDE_INT

#define INCLUDE_END
#define INCLUDE_NETWORK
#define INCLUDE_IFCONFIG /*added by cfg 2011.10.11*/
#define INCLUDE_PING



//#define INCLUDE_OBC_INT0
//#define INCLUDE_OBC_INT1
//#define INCLUDE_OBC_INT2
//#define INCLUDE_OBC_INT3
#ifdef BOOTROM

#undef INCLUDE_CFE_SUPPORT

#endif
