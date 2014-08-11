/* sysLib.c - BCM1250 system-dependent routines */

/*
 * Copyright (c) 2001-2002, 2004-2008 Wind River Systems, Inc.
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
03i,01oct08,pgh  Fix clock rate difference between default rate and target
                 board.
03h,16sep08,slk  don't include sysBootSec if INCLUDE_NETWORK not defined
03g,29aug08,kab  Change _WRS_VX_SMP to _WRS_CONFIG_SMP
03f,28may08,dlk  Use generic vxbDmaBufLib DMA map load routines.
03e,16oct07,pes  WIND00108155: vxCpuConfiguredGet returns wrong count (!=1) in
                 a UP system.
03d,13aug07,slk  remove deprecated APIs
03c,27jul07,ami  cmdLine.c inclusion removed
03b,28jun07,bwa  added DSHM support.
03a,22jun07,slk  remove sysCpuNumGet and replace with vxCpuIdGet (from arch)
02z,14jun07,slk  shared memory support and move sb1FfsTbl table here from
                 sysLib.c.
02y,24may07,slk  defect 86392 - fix AMP mode support for secondary cpu

02x,03may07,slk  remove include of vxbMipsR4KTimer.c and fix cpunum
                 warning
02w,27apr07,slk  hardcode sysClk driver selection
02v,27apr07,slk  changes for interrupt controller support
02u,17apr07,rlg  fix CACHE_LIB_INIT_FUNC macro
02t,20feb07,ami  Made compatible with vxbus timers
02s,04feb07,wap  Add support for detecting the system revision
02r,26jan07,pes  Arrange for override of assignment of sysCacheLibInit.
02q,18jan07,d_c  Correct merge error - remove extra endif
02p,08jan07,pes  Add detection of existing bootline in RAM to support warm
                 boots.
02o,05jan07,rlg  update from smp via phil
02n,17nov06,rlg  fix mbox names
02m,31oct06,pes  Change reference to vxCpuLib.h to vxCpuLibArch.h.
02l,18oct06,jmt  Defect 69546 - Add missing code to set boottype
02k,04oct06,pes  Set up Mailbox interrupts as highest priority interrupt in
                 the system.
02j,26sep06,pes  Add sysStartType handling from environment variable. Add
                 sysCpuStart and sysCpuStop functions.
02i,12sep06,pes  Use vxCpuLib.h include to get declaration of vxCpuCount.
02h,21aug06,pes  Modify strategy of sysToMonitor to assert system_reset bit in
                 system config register
02g,12aug06,pes  Change references to VX_MAX_SMP_CPUS to SB_MAX_SMP_CPUS, as
                 defined in this BSP.
02f,08aug06,pes  Correct usage of VX_MAX_SMP_CPU to VX_MAX_SMP_CPUS.
02e,01aug06,pes  Add initialization of vxCpuCount global to cfeInit.
02c,22sep06,pmr  add support for DRV_SIO_SB1
02d,19sep06,rlg  clean up for romable images and etc.
02c,07sep06,rlg  modifications for dual core support
02b,20jul06,wap  Update for new vxbDmaBuf API
02a,18jul06,wap  Fix project build
01z,07jul06,wap  Add VxBus support
01y,23may06,pes  Update sysModelStr.
01x,17may06,pes  Temporarily add test code.
01w,15may06,pes  Adjustments to sysToMonitor() for warm booting.
01v,11may06,pes  Add support for starting slave cpu(s).
01u,13apr06,pes  Support boot-time detection of CPU_CLOCK_RATE.
01t,10apr06,pes  Disable debugging code by default.
01s,14mar06,pes  Added INCLUDE_CFE_SUPPORT.
01r,24jan06,rlg  SPR 102678  lockout interrupts before jumping to restart
01q,17jan06,pes  SPR 109178, 109181, 109182: Use common mipsTlbClear function. 
01q,12jan06,jmt  Modified sysModel to use SYS_MODEL
01p,23aug05,jmt  Componentize BSP for scalability
01o,03aug05,dr   Decoupled for scalability.
01n,16nov04,mdo  Documentation fixes for apigen
01m,07oct04,agf  remove use of LOCAL_MEM_LOCAL_ADRS_RUNT
01l,07oct04,agf  addt'l vxWorks 6.0 clean-up
01k,06oct04,pes  Change INCLUDE_MMU_BASIC to INCLUDE_MAPPED_KERNEL
01j,18aug04,md   PM_RESERVED_MEM is dependent on INCLUDE_EDR_PM
01i,03aug04,agf  change MMU include from FULL to BASIC
01h,23jun04,agf  remove exc vector init, handled by arch code
01k,17may04,agf  AIM AD-MMU support (kernel in KSEG2)
01j,03oct02,agf  changes for shared sentosa support
01i,18jul02,pgh  Use R4K library timer.
01h,25jun02,pgh  Add calls to enable L2 cache.
01g,20jun02,pgh  Change path to bcm1250Lib.h.
01f,13mar02,agf  remove cond compiles for obj module loader types,  SPR 73892
                 change SM_OBJ conditional compiles to SM_COMMON, SPR 74321
01e,21jan02,tlc  Remove specialization of sysBusTas() by removing SYS_BUS_TAS
                 macro.
01d,04jan02,agf  add nvRAM support supplied by Z.Chen
01d,17jan02,agf  make sysForceLink vars global so diab will not optimize them
                 out
01c,20dec01,agf  add references to symbols in bcm1250L2Cache.s and
                 bcm1250DramInit.s to make sure they are in the partial link
                 objects when building vxWorks_rom et al
01c,20dec01,tlc  Remove unecessary include files.
01b,07dec01,agf  remove vestigial #if 0 code from sysHwInit2
01a,15nov01,agf  written.
*/

/*
DESCRIPTION

This library provides board-specific routines for the Broadcom BCM1250-
swarm evaluation board.

INCLUDE FILES
*/

/* includes */

#include <vxWorks.h>
#include <vsbConfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <version.h>
#include <ctype.h>
#include <cacheLib.h>
#include <fppLib.h>
#include <ioLib.h>
#include <intLib.h>
#include <sysLib.h>
#include <string.h>
#include <arch/mips/fppMipsLib.h>
#include <arch/mips/vxCpuArchLib.h>
#include "config.h"
#include "sibyte.h"
#include "simpleprintf.h"

#include "hrconfig.h"
#include <ipProto.h>

#if 0
#include <../src/hwif/h/hEnd/hEnd.h>
#include "vxbAlteraTse.h"
#endif

#include "hrMatrixDma.h"

#if (BCM_FAMILY == BCM_SB1)
#include "bcm1250JTAGSio.h"
#include <drv/multi/sb1Lib.h>
#undef MBOX_INT_3
#define MBOX_INT_3      ILVL_SB1_MBOX_INT_3
#elif (BCM_FAMILY == BCM_SB1A)
#include <drv/multi/bcm1480Lib.h>
#undef MBOX_INT_3
#define MBOX_INT_3      59      /* bit number of Mailbox interrupt source */
#else
#error "Unknown BCM_FAMILY"
#endif

#ifdef INCLUDE_PCI
#include "bcm1250PciLib.h"
#endif

#ifdef INCLUDE_PCI
#include <pciAutoConfigLib.h>
#include <drv/pci/pciConfigLib.h>
#include <drv/pci/pciIntLib.h>
#endif /* INCLUDE_PCI */

#ifdef INCLUDE_SM_COMMON
#include <smLib.h>
#include <smUtilLib.h>

#include <bootLib.h>
#include <bootLoadLib.h>
#include <bootElfLib.h>
#include <fioLib.h>
#include <ftpLib.h>
#include <tftpLib.h>
#include <remLib.h>
#endif

#ifdef INCLUDE_VXBUS
#include <hwif/vxbus/vxBus.h>
#   ifdef INCLUDE_SIO_UTILS
IMPORT void    sysSerialConnectAll();
#   endif /* INCLUDE_SIO_UTILS */
IMPORT void    hardWareInterFaceInit();
#ifdef DRV_TIMER_MIPSR4K
int vxbR4KTimerFrequencyGet (struct vxbDev *);
#endif
#include "hwconf.c"

#include "vxbAlteraTse.h"
#ifdef INCLUDE_DMA_SYS
#include <../src/hwif/h/vxbus/vxbAccess.h>
#include <hwif/util/vxbDmaBufLib.h>
#include <hwif/vxbus/vxbPlbLib.h>

IMPORT device_method_t * pSysPlbMethods;
DEVMETHOD_DEF(sb1RevisionGet, "get SB1 silicon rev info");

LOCAL VXB_DMA_MAP_ID sb1PlbDmaMapCreate (VXB_DEVICE_ID, VXB_DEVICE_ID,
    VXB_DMA_TAG_ID, int, VXB_DMA_MAP_ID *);
LOCAL STATUS sb1PlbDmaMapSync (VXB_DEVICE_ID, VXB_DMA_TAG_ID,
    VXB_DMA_MAP_ID, bus_dmasync_op_t);

LOCAL STATUS sb1RevisionGet (VXB_DEVICE_ID, UINT16 *, UINT8 *);

IMPORT char *   pSysClkName;
IMPORT UINT32   sysClkDevUnitNo;
IMPORT UINT32   sysClkTimerNo;

/*
 * Methods which we want to add to the PLB device.
 * Note that we only need to override the map create function.
 * This will allow us to set up the other custom functions
 * that we need. The map destroy and memory allocate/free
 * routines don't need to be overriden: the defaults will work
 * fine for us.
 */

LOCAL struct vxbDeviceMethod sb1PlbMethods[] =
    {
    DEVMETHOD(vxbDmaBufMapCreate, sb1PlbDmaMapCreate),
    DEVMETHOD(sb1RevisionGet, sb1RevisionGet),
    { 0, 0 }
    };   
#endif /* INCLUDE_DMA_SYS */

/* vxbus compatible timestamp timer */

#ifdef INCLUDE_TIMESTAMP
#include "sb1TimestampTimer.c"
#endif

#endif /* INCLUDE_VXBUS */

#ifdef INCLUDE_NVRAM
#include "sysNvRam.h"
#endif  /* INCLUDE_NVRAM */

#if defined(INCLUDE_MAPPED_KERNEL)
#include "vmLib.h"
#endif

#undef INCLUDE_VXMP_TESTS

#ifdef INCLUDE_VXMP_TESTS
#include "semSmLib.h"
#include "smNameLib.h"
#endif /* INCLUDE_VXMP_TESTS */

#ifdef INCLUDE_CFE_SUPPORT
#include <../src/hwif/fw/cfe/cfe_api.h>
#endif /* INCLUDE_CFE_SUPPORT */

/* defines */

#define RSHD            514     /* rshd service */

#if (BCM_FAMILY == BCM_SB1)
#define MAILBOX_SET		R_IMR_MAILBOX_SET_CPU
#define MAILBOX_CLR		R_IMR_MAILBOX_CLR_CPU
#elif (BCM_FAMILY == BCM_SB1A)
#define MAILBOX_SET		R_IMR_MAILBOX_0_SET_CPU
#define MAILBOX_CLR		R_IMR_MAILBOX_0_CLR_CPU
#else
#error "Unknown BCM_FAMILY"
#endif

/* externals */

IMPORT void 	fpIntr ();
IMPORT int  	sysFpaAck ();

IMPORT int      taskSRInit();
IMPORT int      sysCompareSet ();
IMPORT void     sysAltGo ();

IMPORT int	palCoreNumGet ();

IMPORT int	sysPridGet(void);
IMPORT int	vxCpuIdGet(void);

#ifdef INCLUDE_END
IMPORT STATUS    nvramEnvGet (char *name, char *string, int strLen);
#endif /* INCLUDE_END */

#if	(INT_PRIO_MSB == TRUE)
IMPORT UINT8    ffsMsbTbl[];            /* Msb high interrupt hash table */
#else	/* INT_PRIO_MSB == TRUE */
IMPORT UINT8    ffsLsbTbl[];            /* Lsb high interrupt hash table */
#endif	/* INT_PRIO_MSB == TRUE */

IMPORT int sysStartType;
IMPORT char _gp[];
IMPORT UINT32 taskSrDefault;

#ifdef INCLUDE_CFE_SUPPORT
IMPORT int	cfe_unsetenv (char *name);
#endif /* INCLUDE_CFE_SUPPORT */

/* forward declarations */

UINT32 sysCpuAvailableGet(void);

#ifdef INCLUDE_SM_COMMON
/* added by yinwx, 20100202 */
#include <vxIpiLib.h>
STATUS sysSmIntGen
    (
    int arg1,	/* from SM_INT_ARG1 - the mailbox bit to set */
    int arg2,	/* from SM_INT_ARG2 - unused */
    int arv3,	/* from SM_INT_ARG3 - unused */
    int cpuNum
    );
void sysSmInt
    (
    int parameter
    );
LOCAL void sysSbSmRegister (void);
LOCAL void sysSmInstInit (VXB_DEVICE_ID pInst);
#endif

#ifdef INCLUDE_END
LOCAL int parse_xdigit (char str);
LOCAL int parse_hwaddr (char *str, uint8_t *hwaddr);
#endif

extern void vxbMipsZrIntCtlrRegister(void);

#ifdef INCLUDE_ATSEMAC
    IMPORT void atsemacRegister(void);
#endif

//#ifdef HRMATRIXDMA0_DEVICE
#ifdef INCLUDE_HRMATRIXDMA0
    IMPORT void hrMatrixdma0Register(void);
#endif
//#ifdef HRMATRIXDMA1_DEVICE
#ifdef INCLUDE_HRMATRIXDMA1
    IMPORT void hrMatrixdma1Register(void);
#endif

#ifdef INCLUDE_OBC_INT0
    IMPORT void obc0Register(void);
#endif


#ifdef INCLUDE_OBC_INT1
    IMPORT void obc1Register(void);
#endif


#ifdef INCLUDE_OBC_INT2
    IMPORT void obc2Register(void);
#endif


#ifdef INCLUDE_OBC_INT3
    IMPORT void obc3Register(void);
#endif


void ht_set_rx_win( unsigned long v1, unsigned long v2);

/* globals */

/*
 * Since tying interrupt lines to the processor is board dependent sysHashOrder
 * is provided to select the prioritization of interrupt lines.
 *
 * Usually, a sequential prioritization of interrupts from left to right is
 * sufficient. However, since we MUST have the IPI (InterProcessor Interrupt)
 * at the highest priority for SMP, since the timer interrupt is on the most
 * significant interrupt pin (INT5), and finally, since we can't share the
 * IPI interrupt with another interrupt, the standard prioritization is not
 * acceptable.
 *
 * Here, we provide an alternate priority table that ensures that the
 * IPI interrupt always gets acknowledged whenever it is asserted, and
 * also modifies the priorities seen for the other interrupt sources:
 *
 * Mailbox (for Inter-Processor Interrupts)     (INT4, Highest)
 * Timer (System clock)                         (INT5)
 * AuxTimer                                     (INT2)
 * PCI                                          (INT0)
 * UART0,2                                      (INT1)
 * UART1,3                                      (INT3)
 * SWTRAP1 (Software trap 1)                    (SW1)
 * SWTRAP0 (Software trap 0)                    (SW0, Lowest)
 *
*/

UINT8 sb1FfsTbl [256] =
    {
    0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 2, 2, 2, 2,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 2, 2, 2, 2, 3, 3, 3, 3, 2, 2, 2, 2,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    };

UINT8 * sysHashOrder = sb1FfsTbl;

/*
 *  This table is critical to interrupt processing.  Do not alter
 *  its contents until you understand the consequences. Refer to the
 *  MIPS Architecture Supplement for instructions on its use.
 */

typedef struct 
    {
    ULONG	intCause;		/* cause of interrupt	*/
    ULONG	bsrTableOffset; 	/* index to BSR table	*/
    ULONG	statusReg;		/* interrupt level	*/
    ULONG	pad;			/* pad for ease of use	*/
    } PRIO_TABLE;

#define INT_PRIO_ENFORCED
#ifdef INT_PRIO_ENFORCED
/* This prioritization table ensures that the IPI interrupt is 
 * never disabled by any other interrupt, and disables all other 
 * interrupts
 * 
 * There is some flexibility in these masks. Specifically, this
 * table causes each interrupt to mask not only itself, but
 * also all lower-priority interrupts. This is not strictly
 * required. But DO NOT allow any interrupt mask to include the
 * bit to disable the IPI interrupt (except the IPI interrupt
 * itself, of course), or change the IPI mask so that any other
 * interrupt is left enabled during IPI processing.
 */

PRIO_TABLE intPrioTable[8] = 
    {
    {CAUSE_SW1,(ULONG) IV_SWTRAP0_VEC, 0x0100, 0},  /* sw trap 0    */
    {CAUSE_SW2,(ULONG) IV_SWTRAP1_VEC, 0x0300, 0},  /* sw trap 1    */
    {CAUSE_IP3,(ULONG) IV_INT0_VEC, 0x2f00, 0},     /* INT 0, PCI   */
    {CAUSE_IP4,(ULONG) IV_INT1_VEC, 0x2b00, 0},     /* INT 1, UART0,2 */
    {CAUSE_IP5,(ULONG) IV_INT2_VEC, 0x3f00, 0},     /* INT 2  AuxClk  */
    {CAUSE_IP6,(ULONG) IV_INT3_VEC, 0x2300, 0},     /* INT 3  UART1,3 */
    {CAUSE_IP7,(ULONG) IV_INT4_VEC, 0xff00, 0},     /* INT 4, IPI   */
    {CAUSE_IP8,(ULONG) IV_INT5_VEC, 0xbf00, 0}      /* INT 5, Timer */
    };
#else /* INT_PRIO_ENFORCED */
/*
 * This prioritization table also ensures that the IPI interrupt is
 * never disabled by any other interrupt, but it allows all other
 * interrupts to only disable themselves, instead of masking lower
 * priority interrupts, as done above.
 */
PRIO_TABLE intPrioTable[8] = 
    {
    {CAUSE_SW1,(ULONG) IV_SWTRAP0_VEC, 0x0100, 0},  /* sw trap 0    */
    {CAUSE_SW2,(ULONG) IV_SWTRAP1_VEC, 0x0200, 0},  /* sw trap 1    */
    {CAUSE_IP3,(ULONG) IV_INT0_VEC, 0x0400, 0},     /* INT 0, PCI   */
    {CAUSE_IP4,(ULONG) IV_INT1_VEC, 0x0800, 0},     /* INT 1, UART0,2 */
    {CAUSE_IP5,(ULONG) IV_INT2_VEC, 0x1000, 0},     /* INT 2  AuxClk  */
    {CAUSE_IP6,(ULONG) IV_INT3_VEC, 0x2000, 0},     /* INT 3  UART1,3 */
    {CAUSE_IP7,(ULONG) IV_INT4_VEC, 0xff00, 0},     /* INT 4, IPI   */
    {CAUSE_IP8,(ULONG) IV_INT5_VEC, 0x8000, 0}      /* INT 5, Timer */
    };
#endif /* INT_PRIO_ENFORCED */

/*
 * Virtual Memory definitions
 */

#if defined(INCLUDE_MAPPED_KERNEL)

#define VM_STATE_MASK_FOR_ALL \
	VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE
#define VM_STATE_FOR_IO \
	VM_STATE_VALID | VM_STATE_WRITABLE | VM_STATE_CACHEABLE_NOT
#define VM_STATE_FOR_MEM_OS \
	VM_STATE_VALID | VM_STATE_WRITABLE | VM_STATE_CACHEABLE
#define VM_STATE_FOR_MEM_APPLICATION \
	VM_STATE_VALID | VM_STATE_WRITABLE | VM_STATE_CACHEABLE
#define VM_STATE_FOR_PCI \
	VM_STATE_VALID | VM_STATE_WRITABLE | VM_STATE_CACHEABLE_NOT

/*
 * sysPhysMemDesc[] entries:
 * defines the memory space on the card
 */

PHYS_MEM_DESC sysPhysMemDesc [] =
    {

    /* RAM */
    {
		(VIRT_ADDR) LOCAL_MEM_LOCAL_ADRS, /* virtual Addr */
		(PHYS_ADDR) KM_TO_PHYS(LOCAL_MEM_LOCAL_ADRS), /* Physical Addr */  
		0x10000000,             /* LOCAL_MEM_SIZE  length */
		VM_STATE_MASK_FOR_ALL,
		VM_STATE_FOR_MEM_OS
    },
#if 1
    {
		(VIRT_ADDR) (LOCAL_MEM_LOCAL_ADRS+0x10000000), /* virtual Addr */    
		(PHYS_ADDR) 0x20000000, /* Physical Addr */
		0x10000000,             /* length */
		VM_STATE_MASK_FOR_ALL,
		VM_STATE_FOR_MEM_OS
    },
    {
        (VIRT_ADDR) (LOCAL_MEM_LOCAL_ADRS+0x20000000), /* virtual Addr */    
        (PHYS_ADDR) 0x60000000, /* Physical Addr */
        0x20000000,             /* length */
        VM_STATE_MASK_FOR_ALL,
        VM_STATE_FOR_MEM_OS
      },
#endif  
   
    
    #if 0
    {                        
		(VIRT_ADDR) 0x60000000, /* virtual Addr */    
		(PHYS_ADDR) 0x60000000, /* Physical Addr */
		0x20000000,             /* length */
		VM_STATE_MASK_FOR_ALL,
		VM_STATE_FOR_MEM_OS
    }
    #endif
    };

/* number Mmu entries to be mapped */

int sysPhysMemDescNumEnt = NELEMENTS(sysPhysMemDesc); 

#endif /* defined(INCLUDE_MAPPED_KERNEL) */

#ifdef INCLUDE_CFE_SUPPORT

UINT64 *cfeArgs = (UINT64 *)CFEARGS;
int cfeBoot = FALSE;		/* TRUE if we think we booted from CFE */

#define CFE_MEM_MAX 10
struct
	{
	uint64_t	start;
	uint64_t	len;
	uint64_t	type;
	} cfeMem[CFE_MEM_MAX];

int cfeInit(void);
#endif	/* INCLUDE_CFE_SUPPORT */

#ifdef CPU_CLOCK_RATE
int sb1_clock_rate = CPU_CLOCK_RATE;    /* Use rate defined for target board */
#else
  #if (BOARD_TYPE == BCM91250A)
    int sb1_clock_rate = 500000000  ;  //  mxl: 450000000;         /* sb1250a 400 MHz default */
  #elif (BOARD_TYPE == BCM91250E)
    int sb1_clock_rate = 500000000;     //  mxl: 450000000;     /* sb1250e 500 MHz default */
  #else
    int sb1_clock_rate = 500000000;      //  mxl: 450000000;    /* sb1480 700 MHz default */
  #endif
#endif /* CPU_CLOCK_RATE */

/* locals */

char sysModelStr[64] = BOARD_NAME;

IMPORT STATUS sysCacheSb1Init(int, int);
#define CACHE_LIB_INIT_FUNC sysCacheSb1Init

/* Included Generic MIPS Support code */
#include "sysMipsLib.c"

/* Included drivers */

#ifdef INCLUDE_PCI
#include "pci/pciConfigLib.c"	/* standard PCI config space access */
#include "pciAutoConfigLib.c"
#include "pci/pciIntLib.c"
#ifdef INCLUDE_SHOW_ROUTINES
#include "pci/pciConfigShow.c"
#endif
#endif /* INCLUDE_PCI */

/* Additional Components */
#ifdef INCLUDE_NVRAM
#if (NV_RAM_SIZE == NONE)
#  include <mem/nullNvRam.c>
#else
/* nvRAM driver */
#include "x1240RtcEeprom.c"
#include "m24lv128Eeprom.c"
#include "lpcFlashSST.c"
#include "sysNvRam.c"
#endif
#endif  /* INCLUDE_NVRAM */

/*******************************************************************************
*
* sysModel - return the model name of the CPU board
*
* This routine returns the model name of the CPU board.
*
* RETURNS: A pointer to the board name string.
*
* ERRNO
*/

char *sysModel (void)
    {
    return (sysModelStr);
    }


/******************************************************************************
*
* sysBspRev - return the bsp version with the revision eg 1.1/<x>
*
* This function returns a pointer to a BSP version with the revision.
* for eg. 1.1/<x>. BSP_REV is concatenated to BSP_VERSION to form the
* BSP identification string.
*
* RETURNS: A pointer to the BSP version/revision string.
*
* ERRNO
*/

char * sysBspRev (void)
    {
    return (BSP_VERSION BSP_REV);
    }


typedef unsigned long pcitag_t;
typedef unsigned long pcireg_t;	

#define CACHED_MEMORY_ADDR	0x80000000
#define UNCACHED_MEMORY_ADDR	0xa0000000

#define	PHYS_TO_UNCACHED(x) 	((x) | UNCACHED_MEMORY_ADDR)

pcitag_t _pci_make_tag(int bus,	int device,	int function)
{
	pcitag_t tag;

	tag = (bus << 16) | (device << 11) | (function << 8);
	return(tag);
}

/*
 *  Break up a pci tag to bus, device function components.
 */
void _pci_break_tag(pcitag_t tag,	int *busp,	int *devicep,	int *functionp  )
{
	if (busp) 
    {
		*busp = (tag >> 16) & 255;
	}
	if (devicep) 
    {
		*devicep = (tag >> 11) & 31;
	}
	if (functionp)
    {
		*functionp = (tag >> 8) & 7;
	}
}

long _pci_conf_readn(pcitag_t tag, int reg, int width)
{
    unsigned long  addr, type;
    pcireg_t data;
    int bus, device, function;

    if ((reg & (width-1)) || reg < 0 || reg >= 0x100)
    {
        printf ("_pci_conf_read: bad reg 0x%x\n", reg);
	    return -1;
    }

    _pci_break_tag (tag, &bus, &device, &function); 
    if (bus == 0) 
    {
    	/* Type 0 configuration on onboard PCI bus */
    	if (device > 20 || function > 7)
    	    return -1;		 
    	addr = (1 << (device+11)) | (function << 8) | reg;
    	type = 0x00000;
    }
    else 
    {
        /* Type 1 configuration on offboard PCI bus */
        if (bus > 255 || device > 31 || function > 7)
            return -1;	 
        addr = (bus << 16) | (device << 11) | (function << 8) | reg;
        type = 0x10000;
    }
    
    printf ("_pci_conf_read:  addr = %x\n",   addr);
    printf ("_pci_conf_read:  BONITO_PCICMD = %x\n",   BONITO_PCICMD);
    /* clear aborts */
    BONITO_PCICMD |= PCI_STATUS_MASTER_ABORT | PCI_STATUS_MASTER_TARGET_ABORT;

    BONITO_PCIMAP_CFG = (addr >> 16) | type;
    *(volatile unsigned long *)(0xbfe00000+4); /* bflush */
    printf ("_pci_conf_read:  BONITO_PCICMD = %x\n",   BONITO_PCICMD);

    data = *(volatile pcireg_t *)PHYS_TO_UNCACHED(BONITO_PCICFG_BASE | (addr & 0xfffc));

    printf ("_pci_conf_read:  data = %x\n",   data);
    /* move data to correct position */
    data = data >> ((addr & 3) << 3);
    
    printf ("_pci_conf_read:  BONITO_PCICMD = %x\n",   BONITO_PCICMD);
    if (BONITO_PCICMD & PCI_STATUS_MASTER_ABORT) 
    {
    	BONITO_PCICMD |= PCI_STATUS_MASTER_ABORT;
        
	    printf ("_pci_conf_read: reg=%x master abort, data : %x, BONITO_PCICMD = %x\n", reg, data, BONITO_PCICMD);
    	//return -1;
    }

    if (BONITO_PCICMD & PCI_STATUS_MASTER_TARGET_ABORT) 
    {
        BONITO_PCICMD |= PCI_STATUS_MASTER_TARGET_ABORT;

        printf ( "_pci_conf_read: target abort\n");
        return -1;
    }

    return data;
}

pcireg_t _pci_conf_read(pcitag_t tag,int reg)
{
	return _pci_conf_readn(tag,reg,4);
}

void _pci_conf_writen(pcitag_t tag, int reg, pcireg_t data,int width)
{
    unsigned long addr, type;

    int bus, device, function;

    if ((reg &(width-1)) || reg < 0 || reg >= 0x100)
    {
	    printf ("_pci_conf_write: bad reg %x\n", reg);
    	return;
    }

    _pci_break_tag (tag, &bus, &device, &function);

    if (bus == 0) 
    {
        /* Type 0 configuration on onboard PCI bus */
        if (device > 20 || function > 7)
            return;		
        
        addr = (1 << (device+11)) | (function << 8) | reg;
        type = 0x00000;
    }
    else 
    {
        /* Type 1 configuration on offboard PCI bus */
        if (bus > 255 || device > 31 || function > 7)
            return;	
        
        addr = (bus << 16) | (device << 11) | (function << 8) | reg;
        type = 0x10000;
    }



    /* clear aborts */
    BONITO_PCICMD |= PCI_STATUS_MASTER_ABORT | PCI_STATUS_MASTER_TARGET_ABORT;

    BONITO_PCIMAP_CFG = (addr >> 16)|type;

#if 0
    *(volatile pcireg_t *)PHYS_TO_UNCACHED(BONITO_PCICFG_BASE | (addr & 0xfffc)) = data;
#else
    {
        pcireg_t ori = *(volatile pcireg_t *)PHYS_TO_UNCACHED(BONITO_PCICFG_BASE | (addr & 0xfffc));
        pcireg_t mask = 0x0;

        if (width == 2) 
        {
            if (addr & 3)
                mask = 0xffff; 
            else
                mask = 0xffff0000;
        }
        else if (width == 1) 
        {
            if ((addr & 3) == 1) 
            {
                mask = 0xffff00ff;
            }
            else if ((addr & 3) == 2) 
            {
               mask = 0xff00ffff;
            }
            else if ((addr & 3) == 3) 
            {
                mask = 0x00ffffff;
            }
            else
            {
                mask = 0xffffff00;
            }
        }

        data = data << ((addr & 3) << 3);
        data = (ori & mask) | data;
        *(volatile pcireg_t *)PHYS_TO_UNCACHED(BONITO_PCICFG_BASE | (addr & 0xfffc)) = data;
    }
#endif

    if (BONITO_PCICMD & PCI_STATUS_MASTER_ABORT)
    {
    	BONITO_PCICMD |= PCI_STATUS_MASTER_ABORT;
    	printf ( "_pci_conf_write: master abort\n");
    }

    if (BONITO_PCICMD & PCI_STATUS_MASTER_TARGET_ABORT)
    {
        BONITO_PCICMD |= PCI_STATUS_MASTER_TARGET_ABORT;
        printf ("_pci_conf_write: target abort\n");
    }
}


void  _pci_conf_write(pcitag_t tag, int reg, pcireg_t data)
{
	 _pci_conf_writen(tag,reg,data,4);
}

LOCAL int loop = 0;
#define BIG_LOOP for( loop=50000;loop>0;loop--);
#define SMALL_LOOP for( loop=1500;loop>0;loop--);

IMPORT int sysStartType;

void no_printf (char *fmt, ...) {  }

void sysHr1PciCtlr()
{
    int device = 12, tmp = 0;
	pcitag_t tag;
	pcireg_t id;
	pcireg_t misc;
    int i = 0;
    /* initilize PCI control */
    BONITO_PCICLASS = (PCI_CLASS_BRIDGE << PCI_CLASS_SHIFT) | (PCI_SUBCLASS_BRIDGE_HOST << PCI_SUBCLASS_SHIFT);
    BONITO_PCICMD = BONITO_PCICMD_PERR_CLR|BONITO_PCICMD_SERR_CLR|BONITO_PCICMD_MABORT_CLR|BONITO_PCICMD_MTABORT_CLR|BONITO_PCICMD_TABORT_CLR|BONITO_PCICMD_MPERR_CLR;
    BONITO_PCILTIMER=255;
    BONITO_PCIBASE0 = 0;
    BONITO_PCIBASE1 = 0;
    BONITO_PCIBASE2 = 0;
    BONITO_PCIEXPRBASE = 0;
    BONITO_PCIINT = 0;
    
    BONITO_PCI_REG(0x150) = 0x8000000c;
    BONITO_PCI_REG(0x154)= 0xffffffff;

    tmp = BONITO_PCICMD;
    tmp |= BONITO_PCICMD_PERRRESPEN;
    BONITO_PCICMD = tmp;
    
    tmp = BONITO_PCICMD;
    tmp |= PCI_COMMAND_IO_ENABLE|PCI_COMMAND_MEM_ENABLE|PCI_COMMAND_MASTER_ENABLE;
    BONITO_PCICMD = tmp;
    
    tmp = BONITO_BONGENCFG;
    tmp &= ~0x80;
    BONITO_BONGENCFG = tmp;

    tmp = BONITO_BONGENCFG;
    tmp |= BONITO_BONGENCFG_DEBUGMODE;
    BONITO_BONGENCFG = tmp;
    
    
    /* clear aborts */
    
    BONITO_PCICMD |= PCI_STATUS_MASTER_ABORT | PCI_STATUS_MASTER_TARGET_ABORT;
    
    BONITO_PCIMAP =
	    BONITO_PCIMAP_WIN(0, 0+0x00000000) |	
	    BONITO_PCIMAP_WIN(1, 0+0x04000000) |
	    BONITO_PCIMAP_WIN(2, 0+0x08000000) |
	    BONITO_PCIMAP_PCIMAP_2;
    BONITO_PCIBASE0 = 0xa0000000;  /*  */
    
	for(; device < 32; device++) 
    {

    	tag = _pci_make_tag(0, device, 0);

    	id = _pci_conf_read(tag, 0);


    	if (id == 0 || id == 0xffffffff)
        {
    		continue;
    	}

        
    	misc = _pci_conf_read(tag, PCI_BHLC_REG);

    	if (PCI_HDRTYPE_MULTIFN(misc)) 
        {
    		int function;
    		for (function = 0; function < 8; function++)
            {
    			tag = _pci_make_tag(0, device, function);
    			id = _pci_conf_read(tag, 0);
    			if (id == 0 || id == 0xffffffff)
                {
    				continue;
    			}
                
    		}
    	}
    	else 
        {
    	}

	    break;


    }
    

    /* configurate PCI base address */
    *(volatile unsigned  short *)0xbfe80004 = 0x0047;
    *(volatile unsigned  long *)0xbfe80014 = 0x08000000;

    
    /*  *(volatile unsigned  long *)0xbfe80018 = 0x08001000; */


   // printf("sysHr1PciCtlr completet!!");
 //  *(volatile unsigned  long *)0xb8001008 = 0x3;
    *(volatile unsigned  long *)0xb8005000 = 0x80000000;  /* fpga pci address trans */
    *(volatile unsigned  char *)0xb8004050 = 0x80;  /* fpga pci int enable */
}


int sysHr1PciScan()
{

    #if 0 // INCLUDE_ATSEMAC

    UINT8  wTemp1, wTemp2, j;  
    int i;
    unsigned int bpmask = 0;
    
    bpmask = 0xffffff00;
    /* 2a¨º?SGdma */
    
   //sysTffsFormat();

   //I2c_test();
 //  _func_printErr = (FUNCPTR) bsp_printf;

    usrNetIpAttachCommon ("atse", 0,"IPv4",ipAttach);

    usrNetBootConfig ("atse", 0, sysBootParams.ead,
                          bpmask, sysBootParams.gad);
    bsp_printf("usrNetIpAttachCommon attach!");
    #endif
    
    #if 0
    rxDesc = cacheDmaMalloc(2 * sizeof(ATSE_SGDMA_DESCRIPTOR));
    memset((void * )(rxDesc), 0, 2 * sizeof(ATSE_SGDMA_DESCRIPTOR));
    next = &rxDesc[1];
    
    bsp_printf("rxDesc = %d   ", rxDesc);
    bsp_printf("next = %d \r\n", next);

    bsp_printf("temp = %d \r\n", 0x123456);
    alt_avalon_sgdma_construct_stream_to_mem_desc(
            (alt_sgdma_descriptor *) rxDesc,  // descriptor I want to work with
            (alt_sgdma_descriptor *) next, // pointer to "next"
            0x003400,    // tse_ptr->pkt_array[tse_ptr->chain_loop]->nb_buff,           // starting write_address
            0,                          // read until EOP
            0);  

    bsp_printf("tse_mac_aRxRead before \r\n" );
    tse_mac_aRxRead((alt_sgdma_descriptor *)(rxDesc &0x0fffffff));
    #endif

    return 0;

}

/*******************************************************************************
*
* sysHwInit - initialize the CPU board hardware
*
* This routine initializes various features of the SB1/SB1A
* It is called from usrInit() in usrConfig.c.
*
* This routine initializes and turns off the timers.
*
* NOTE:
* This routine should not be called directly by the user.
* 
* RETURNS: N/A
*
* ERRNO
*/
void sysHwInit (void)
    {
    int sr;
    unsigned int cpunum;

#ifdef INCLUDE_CFE_SUPPORT
    /*cfeInit (); 20091225 by yinwx */
#endif	/* INCLUDE_CFE_SUPPORT */
    sr = SB1_SR;

    /* init status register but leave interrupts disabled */
    taskSRInit (sr);
    intSRSet (sr & ~SR_IE);

    /* set the Processor number based on the CPU ID */
    sysProcNumSet (palCoreNumGet());

     /* make sure there is a valid boot string if running a secondary cpu */
    if (palCoreNumGet () != 0)
	{
        strcpy (sysBootLine, DEFAULT_BOOT_LINE);
	}

#ifdef	INCLUDE_HW_FP
    /* initialize floating pt unit */
    if (fppProbe () == OK)
	{
	fppInitialize ();
	intVecSet ((FUNCPTR *)INUM_TO_IVEC (IV_FPE_VEC), (FUNCPTR) fpIntr);
	}
#endif	/* INCLUDE_HW_FP */

    #if  1/*BOOTROM */
	vxbMipsZrIntCtlrRegister();
	#endif

    /* added by moxiaolu for PCI control initial */
     sysHr1PciCtlr();
    #ifdef INCLUDE_ATSEMAC
        atsemacRegister();
    #endif
    
#ifdef INCLUDE_HRMATRIXDMA0
//#ifdef HRMATRIXDMA0_DEVICE
        hrMatrixdma0Register();
#endif
#ifdef INCLUDE_HRMATRIXDMA1
//#ifdef HRMATRIXDMA1_DEVICE
        hrMatrixdma1Register();
#endif

#ifdef INCLUDE_OBC_INT0
       obc0Register();
#endif


#ifdef INCLUDE_OBC_INT1
       obc1Register();
#endif


#ifdef INCLUDE_OBC_INT2
       obc2Register();
#endif


#ifdef INCLUDE_OBC_INT3
       obc3Register();
#endif

	  
#ifdef INCLUDE_VXBUS
#ifdef INCLUDE_DMA_SYS
    pSysPlbMethods = sb1PlbMethods;
#endif
#ifdef INCLUDE_TIMESTAMP
    sb1TimestampInit ();
#endif /* INCLUDE_TIMESTAMP */
    /* hardcode the sysClk driver settings */

    pSysClkName = "r4KTimerDev";
/* secondary CPU on unit 1 */
#if defined(SB1_CPU_1)
    sysClkDevUnitNo = 1;
#else /* primary CPU on unit 0 */
    sysClkDevUnitNo = 0;
#endif /* defined(SB1_CPU_1) */
    sysClkTimerNo = 0;

    hardWareInterFaceInit();
#endif /* INCLUDE_VXBUS */

#if defined (INCLUDE_PCI)
    /* Initialize the PCI and LDT here (following sysLib for sdb4122eagle). */

    sysPciConfig ();
    pciIntLibInit ();

    /* Initialize the host bridges. */

    sysHostBridgeInit ();

#endif /*INCLUDE_PCI*/
    
    /* ensure all interrupts are masked */

#ifdef _WRS_CONFIG_SMP
    for (cpunum = 0; cpunum < sysCpuAvailableGet (); cpunum++)
#else /* UP and AMP only clear mask bits for this cpu */
    cpunum = vxCpuIdGet ();
#endif /* _WRS_CONFIG_SMP */
	{
	/* Mask off all interrupts (they'll be enabled as needed later). */
#if (BCM_FAMILY == BCM_SB1)
	MIPS3_SD(PHYS_TO_K1(A_IMR_REGISTER(cpunum, R_IMR_INTERRUPT_MASK)),
		 0xffffffffffffffffULL);
#else /* BCM_FAMILY == BCM_SB1 */
	
#if 0 /** !!!!! by yinwx, 20091210 !!! **/
	MIPS3_SD(PHYS_TO_K1(A_IMR_REGISTER(cpunum, R_IMR_INTERRUPT_MASK_H)),
		 0xffffffffffffffffULL);
	MIPS3_SD(PHYS_TO_K1(A_IMR_REGISTER(cpunum, R_IMR_INTERRUPT_MASK_L)),
		 0xffffffffffffffffULL);
#endif
#endif /* BCM_FAMILY == BCM_SB1 */
	}
	
    }


/******************************************************************************
*
* sysHwInit2 - additional system configuration and initialization
*
* This routine connects system interrupts and does any additional
* configuration necessary.
*
* RETURNS: N/A
*
* ERRNO
*/
char*  rioHostAdrs = 0;
void sysHwInit2 (void)
    {
#ifdef INCLUDE_VXBUS
    vxbDevInit ();
#   ifdef INCLUDE_SIO_UTILS
    sysSerialConnectAll();
#   endif /* INCLUDE_SIO_UTILS */
    taskSpawn("tDevConn", 11, 0, 10000,
              vxbDevConnect, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
#endif

#ifdef INCLUDE_PCI
    /* Size the LDT fabric; configure PCI and LDT */
    sysPciAutoConfig ();

    /* Device-specific PCI initialization might go here. */
#endif /*INCLUDE_PCI*/

#ifdef INCLUDE_SM_COMMON
	/* ACTUALLY We didnot use the sysSbSmRegister function!!! */
    /*(void) sysSbSmRegister ();*/ /* added by yinwx, 20100202 */
	smUtilUser1Rtn = (FUNCPTR) sysSmIntGen;
	/*vxIpiConnect(4, (IPI_HANDLER_FUNC)sysSmInt, (void *)4);*/ /* vec is 32+4 */
	/*vxIpiEnable(4); */
	
#endif /* INCLUDE_SM_COMMON */
    ht_set_rx_win(0xc0000000, 0x00c0ffc0);
}


/*******************************************************************************
*
* sysPhysMemTop - get the address of the top of memory
*
* This routine returns the address of the first missing byte of memory, which
* indicates the top of memory.
*
* NOTE: Do not adjust LOCAL_MEM_SIZE to reserve memory for application
* use.  See sysMemTop() for more information on reserving memory.
*
* RETURNS: The address of the top of memory.
*
* ERRNO
*/

char *sysPhysMemTop (void)
    {
    static char * memTop = NULL;

    if (memTop == NULL)
	{
	memTop = (char *)(LOCAL_MEM_LOCAL_ADRS + LOCAL_MEM_SIZE);
	}
    return memTop;
    }

/*******************************************************************************
*
* sysMemTop - get the address of the top of logical memory
*
* This routine returns the address of the first unusable byte of memory.
* VxWorks will not use any memory at or above this address.
*
* The user can reserve local memory from the board by declaring the
* macro USER_RESERVED_MEM with the amount of memory to reserve. This
* routine will return a pointer to the first byte of the reserved memory
* area.
*
* RETURNS: The address of the top of usable memory.
*
* ERRNO
*/

char *sysMemTop (void)
    {
    static char * memTop = NULL;

    if (memTop == NULL)
        {
        memTop = sysPhysMemTop () - USER_RESERVED_MEM;

#ifdef INCLUDE_EDR_PM
        /* account for ED&R persistent memory */

        memTop = memTop - PM_RESERVED_MEM;
#endif
        }

    return memTop;
    }

/*******************************************************************************
*
* sysToMonitor - transfer control to the ROM monitor
*
* This routine transfers control to the ROM monitor.  Normally, it is called
* only by reboot()--which services ^X--and bus errors at interrupt level.
* However, in some circumstances, the user may wish to introduce a
* <startType> to enable special boot ROM facilities.
*
* RETURNS: Does not return.
*
* ERRNO
*/

STATUS sysToMonitor
    (
    int startType    /* parameter passed to ROM to tell it how to boot */
    )
    {
#ifdef INCLUDE_CFE_SUPPORT
    int srValue;
    UINT64 *pSysCfg = (UINT64 *)PHYS_TO_K1(A_SCD_SYSTEM_CFG);
    char boot_type[16];
#ifndef _WRS_CONFIG_SMP
#if defined (SB1_CPU_1) || defined (SB1_CPU_2) || defined (SB1_CPU_3)
    int cpunum = vxCpuIdGet ();
#endif /* !SB1_CPU_0 */
#endif /* _WRS_CONFIG_SMP */
#endif

    FUNCPTR pRom = (FUNCPTR) (ROM_TEXT_ADRS + 8);

    /* lock out interrupts while jumping to reboot code */
    intCpuLock();

#ifdef INCLUDE_CFE_SUPPORT
    sprintf(boot_type, "%d", startType);
    cfe_setenv_p("vxwboottype", boot_type);
    cfe_setenv_p("vxwarmbootline",BOOT_LINE_ADRS);

#if !defined (_WRS_CONFIG_SMP)
#if defined (SB1_CPU_1) || defined (SB1_CPU_2) || defined (SB1_CPU_3)
    cfe_cpu_stop(cpunum);
    return(OK);                 /* shouldn't ever happen */
#endif /* !SB1_CPU_0 */
#endif /* _WRS_CONFIG_SMP */


    srValue = intSRGet();
    srValue |= SR_BEV;
    intSRSet(srValue);
    
    if (cfeBoot)
	*pSysCfg |= M_SYS_SYSTEM_RESET;
    else
#endif
	(* pRom) (startType);

    return (OK);    /* in case we ever continue from rom monitor */
    }

/*******************************************************************************
*
* sysAutoAck - acknowledge the R4000 interrupt condition 
*
* This routine acknowledges an R4000 interrupt for a specified interrupt
* vector.
*
* NOTE:
* This routine must be provided on all R4000 board support packages.
* Most interrupts are automatically acknowledged in the interrupt service
* routine.
*
* RETURNS: The result of the interrupt acknowledge cycle.
*
* ERRNO
*/

int sysAutoAck
    (
    int vecNum		/* vector num of interrupt that bugs us */
    )
    {
    int result;

    result = 0;
    switch (vecNum)
	{
    case IV_INT5_VEC:
        sysCompareSet (0);		/* reset count/compare interrupt */
        break;
	case IV_SWTRAP0_VEC:		/* software trap 0 */
	    return(result = sysSw0Ack ());
	    break;

	case IV_SWTRAP1_VEC:		/* software trap 1 */
	    return(result = sysSw1Ack ());
	    break;

	case IV_FPA_UNIMP_VEC:		/* unimplemented FPA oper*/
	case IV_FPA_INV_VEC:		/* invalid FPA operation*/
	case IV_FPA_DIV0_VEC:		/* FPA div by zero */
	case IV_FPA_OVF_VEC:		/* FPA overflow exception */
	case IV_FPA_UFL_VEC:		/* FPA underflow exception */
	case IV_FPA_PREC_VEC:		/* FPA inexact operation */
            return(result = sysFpaAck ());
	    break;

	default:
            return(-1);
        break;
	}
    return(result);
    }

#ifdef	INCLUDE_SHOW_ROUTINES
/*******************************************************************************
*
* sysDisplayMem - display memory
*
* Display contents of memory, starting at adrs.  Memory is displayed in
* words.  The number of words displayed defaults to 64.  If
* nwords is non-zero, that number of words is printed, rounded up to
* the nearest number of full lines.  That number then becomes the default.
*
* RETURNS: OK always
*
* ERRNO
*/

STATUS sysDisplayMem
    (
    FAST char *adrs,	/* address to display */
    int	       nwords	/* number of words to print. */
    )			/* If 0, print 64 or last specified. */
    {
    static char *last_adrs;
    static int dNbytes = 128;
    char ascii [17];
    FAST int nbytes;
    FAST int byte;

    ascii [16] = EOS;			/* put an EOS on the string */

    nbytes = 2 * nwords;

    if (nbytes == 0)
	nbytes = dNbytes;	/* no count specified: use current byte count */
    else
	dNbytes = nbytes;	/* change current byte count */

    if (adrs == 0)
	adrs = last_adrs;	/* no address specified: use last address */

    adrs = (char *) ((int) adrs & ~1);	/* round adrs down to word boundary */


    /* print leading spaces on first line */

    bfill ((char *) ascii, 16, '.');

    printf ("%06x:  ", (int) adrs & ~0xf);

    for (byte = 0; byte < ((int) adrs & 0xf); byte++)
	{
	printf ("  ");
	if (byte & 1)
	    printf (" ");	/* space between words */
	if (byte == 7)
	    printf (" ");	/* extra space between words 3 and 4 */

	ascii[byte] = ' ';
	}

    /* print out all the words */
    while (nbytes-- > 0)
	{
	if (byte == 16)
	    {
	    /* end of line:
	     *   print out ascii format values and address of next line */

	    printf ("  *%16s*\n%06x:  ", ascii, (int) adrs);

	    bfill ((char *) ascii, 16, '.');	/* clear out ascii buffer */
	    byte = 0;				/* reset word count */
	    }

	printf ("%02x", *adrs & 0x000000ff);
	if (byte & 1)
	    printf (" ");	/* space between words */
	if (byte == 7)
	    printf (" ");	/* extra space between words 3 and 4 */

	if ( *adrs == ' ' || 
             ( isascii (*adrs) && isprint ((int)(*adrs)) )
           )
	    ascii[byte] = *adrs;

	adrs++;
	byte++;
	}

    /* print remainder of last line */
    for (; byte < 16; byte++)
	{
	printf ("  ");
	if (byte & 1)
	    printf (" ");	/* space between words */
	if (byte == 7)
	    printf (" ");	/* extra space between words 3 and 4 */

	ascii[byte] = ' ';
	}

    printf ("  *%16s*\n", ascii);	/* print out ascii format values */
    last_adrs = adrs;

    return(OK);
    }

/*******************************************************************************
*
* showBits - debugging function to show bits of a 64-bit quantity
* 
* This routine displays the bits of the supplied 64-bit value. 
*
* RETURNS: N/A
*
* ERRNO
*/

void showBits
    (
    long long *ptr
    )
    {
    unsigned long long mask = ((long long) 1 << 63);
    int count = 0;

    while (mask)
	{
	putchar ((*ptr & mask) ? '1' : '-');
	mask >>= 1;
	count++;
	if (!(count & 3))
	    putchar ('.');
	}
    putchar ('\n');
    }
#endif	/* INCLUDE_SHOW_ROUTINES */


#ifdef INCLUDE_DSHM

/* this is used to adjust value of the address obtained to the K0 segment if
 * using a mapped kernel
 */
#ifdef INCLUDE_MAPPED_KERNEL
BOOL dshmBusCtlrSibyteMappedKernel = TRUE;
#else
BOOL dshmBusCtlrSibyteMappedKernel = FALSE;
#endif

/********************************************************************************
* sysDshmIntGen - send an interrupt to a CPU
*
* This routine is used by the DSHM drivers to interrupt a CPU in the system.
* The CPU interrupted can be itself.
*
* RETURNS: OK, always
*
* ERRNO
*/

STATUS sysDshmIntGen
    (
    int cpu
    )
    {
    unsigned long long *pMboxSet = (unsigned long long *)
      (K1BASE | A_IMR_REGISTER(cpu,  MAILBOX_SET));

    /* set the mailbox interrupt */

    *pMboxSet = DSHM_INT_VAL;
    return OK;
    }

/********************************************************************************
* sysDshmIntClear - called by the DSHM interrupt handler to clear the mailbox
*
* RETURNS: N/A
*
* ERRNO
*/

void sysDshmIntClear
    (
    int cpu
    )
    {
    unsigned long long *pMboxClear = (unsigned long long *)
      (K1BASE | A_IMR_REGISTER(cpu,  MAILBOX_CLR));

    /* clear the mailbox interrupt */

    *pMboxClear = DSHM_INT_VAL;
    }

#endif  /* INCLUDE_DSHM */


    
#ifdef INCLUDE_SM_COMMON
/*******************************************************************************
*
* sysMailboxConnect - specifies interrupt routine fro each mailbox interruption.
*
* This routine specifies the interrupt service routine to be called at each
* mailbox interrupt.
*
* RETURNS: ERROR, since there is no mailbox facility.
*
* ERRNO
*
* SEE ALSO: sysMailboxEnable()
*/

STATUS sysMailboxConnect
    (
    FUNCPTR routine,    /* routine called at each mailbox interrupt */
    int     arg         /* argument with which to call routine      */
    )
    {
    return (ERROR);
    }

/******************************************************************************
*
* sysMailboxEnable - enable the mailbox interrupt
*
* This routine enables the mailbox interrupt.
*
* RETURNS: ERROR, since there is no mailbox facility.
*
* ERRNO
*
* SEE ALSO: sysMailboxConnect()
*/

STATUS sysMailboxEnable
    (
    char *mailboxAdrs           /* mailbox address */
    )
    {
    return (ERROR);
    }

/*******************************************************************************
*
* sysSmIntGen - Called when we should interrupt the other CPU.
* 
* This routine is used by the SM layer to interrupt another CPU
* in the system.
*
* RETURNS: OK, always
*
* ERRNO
*/

STATUS sysSmIntGen
    (
    int arg1,	/* from SM_INT_ARG1 - the mailbox bit to set */
    int arg2,	/* from SM_INT_ARG2 - unused */
    int arv3,	/* from SM_INT_ARG3 - unused */
    int cpuNum
    )
    {
    #if 0
    unsigned long long *pMboxSet = (unsigned long long *)
      (K1BASE | A_IMR_REGISTER(sysProcNum ^ 1,  MAILBOX_SET));

    /* set the mailbox interrupt */

    *pMboxSet = arg1;
	#endif
	UINT32 * pMboxSet, pMboxEnable;
	
	int localCpuNum, localCoreNum;
	int destCpuNum, destCoreNum;
	int tmp;
	UINT32 xBAR1_Win2Base;
	
	tmp = sysPridGet();
	localCpuNum = (tmp & 0xc)>>2;
	localCoreNum = tmp & 0x3;
	xBAR1_Win2Base = 0x3ff02010 | (localCoreNum << 8);

	destCpuNum = (cpuNum & 0xc)>>2;
	destCoreNum = cpuNum & 0x3;
	pMboxSet = A_IMR_REGISTER(destCoreNum,0x1008); /* IPI_Set */
	pMboxEnable = A_IMR_REGISTER(destCoreNum,0x1004); /* IPI_Enable */

/*printstr("INT GENERATE!! local CPU is 0x");
printnum(localCpuNum);
printstr(" dest CPU is 0x");
printnum(cpuNum);
printstr("\r\n");*/
	/* If dest cpuNum is ZERO, we must set XBAR1 windows
	   to route 3ff address to global CPU0 not self low 4G space 
	   yinwx, 20100223 */	
	if(!destCpuNum)
		SET_XBAR_SM(xBAR1_Win2Base, localCpuNum);
	
	MIPS_SW64_SM(pMboxSet, destCpuNum, 1<<arg1);  /* IPI_Set */
	MIPS_SW64_SM(pMboxEnable, destCpuNum, 1<<arg1);	/* IPI_Enable */

	/* unset XBAR1 windows to original configuration in PMON 
	   yinwx, 20100223 */	
	if(!destCpuNum)
		UNSET_XBAR_SM(xBAR1_Win2Base, localCpuNum);
    return OK;
    }

/*******************************************************************************
*
* sysSmInt - Called when we are interrupted by another CPU.
* 
* sysSmInt - Called when we are interrupted by the other CPU.  This
* function simply passes the call along to the SM layer.  The provided
* parameter was registered when we connected to the interrupt, and is
* the bit value to use to clear this mailbox interrupt. 
*
* RETURNS: N/A
*
* ERRNO
*/

void sysSmInt
    (
    int parameter
    )
    {
    /*unsigned long long *pMboxClear = (unsigned long long *)
      (K1BASE | A_IMR_REGISTER(sysProcNum,  MAILBOX_CLR));*/

    /* give the interrupt to the SM layer */
	unsigned long *pMboxClear;
	UINT32 cpuNum = sysPridGet()&0xf;

	pMboxClear = A_IMR_REGISTER(cpuNum&3, MAILBOX_CLR);
	MIPS_SW64(pMboxClear,1<<parameter);
	
    smUtilIntRoutine ();
	
    /* clear the mailbox interrupt */

    /**pMboxClear = parameter;*/
    }

LOCAL struct drvBusFuncs sysSbSmFuncs =
    {
    NULL,                            /* devInstanceInit */
    sysSmInstInit,                   /* devInstanceInit2 */
    NULL                             /* devConnect */
    };

LOCAL struct vxbDevRegInfo sysSbSmDevRegistration =
    {
    NULL,                 /* pNext */
    VXB_DEVID_DEVICE,     /* devID */
    VXB_BUSID_PLB,        /* busID = PLB */
    VXBUS_VERSION_4,      /* busVer 1 */ /* changed by yinwx, VXBUS_VERSION_2, 20100202 */
    "sm",                 /* drvName */
    &sysSbSmFuncs,        /* pDrvBusFuncs */
    NULL,                 /* pMethods */
    NULL                  /* devProbe */
    };

/******************************************************************************
*
* sysSbSmRegister - register the shared memory driver
*
* This routine registers the shared memory driver and device
* recognition data with the vxBus subsystem.
*
* RETURNS: N/A
*
* ERRNO
*/
LOCAL void sysSbSmRegister(void)
    {
    vxbDevRegister((struct vxbDevRegInfo *)&sysSbSmDevRegistration);
    }


/*******************************************************************************
*
* sysSmInstInit - Initialize the SM layer.
*
* This function is called from vxBus, this function sets up the
* BSP for use with SM objects.  We must accomplish two tasks.  First,
* we must hook in our interrupt-generation routine (sysSmObjInterrupt), 
* and second, we must register as a receiver for interrupts from the 
* other CPU.
*
* RETURNS: N/A
*
* ERRNO
*/

LOCAL void sysSmInstInit (VXB_DEVICE_ID pInst)
   {

    /* provide our interrupt-generation code to the SM layer. The SM
     * layer knows to call this function because of the SM_INT_TYPE
     * definition in config.h. 
     */
printstr("=== sysSmInstInit ===\r\n");
    smUtilUser1Rtn = (FUNCPTR) sysSmIntGen;

    /* 
     * register our interrupt receiver - use first bit of the mailbox,
     * so we need to connect to mbox_int_3 
     */

	vxIpiConnect(4, (IPI_HANDLER_FUNC)sysSmInt, (void *)0);
	vxIpiEnable(4);
	/*vxbIntConnect (pInst, 0, sysSmInt,(void *)SM_INT_ARG1);
    vxbIntEnable (pInst, 0, sysSmInt,(void *)SM_INT_ARG1);*/

    }
#endif /* INCLUDE_SM_COMMON */


/* only enable sysBootSec for primary cpu */
#if  defined(SB1_CPU_0) && defined(INCLUDE_BOOT_LINE_INIT) && \
     defined(INCLUDE_NETWORK)

/*******************************************************************************
*
* sysBootSec - this function loads cpu1's runtime kernel and starts it running
*
* This function can be called from the target shell or made to run automatically
* after finishes booting via the INCLUDE_DEMO functionality. It loads the 
* run-time kernel for the secondary cpu from across the network, then passes
* the entry address to the secondary cpu via the mailbox register.
*
* 5 parameters are required to load the kernel image from across the network:
*    hostName - computer which will be serving the kernel image file
*    fileName - file name of the kernel image
*    hostType - protocol the host uses to serve the image file (rsh/TFTP/FTP)
*    usrName  - user name required to login to the host
*    passwd   - password required to login to the host
*
* Except for the fileName, which can optionally be supplied when this function
* is called, all the other parameters are picked up from cpu0's BOOT_LINE.
*
* RETURNS: OK, or ERROR
*
* ERRNO
*/

STATUS sysBootSec
    (
    char * kernelFile
    )
    {
    BOOT_PARAMS params;
    FUNCPTR entry;
    int fd;
    int errFd;          /* for receiving standard error messages from Unix */
    char command [100];
    BOOL bootFtp = FALSE;
    BOOL bootRsh = FALSE;
#ifdef INCLUDE_CFE_SUPPORT
    long coreSp;
#endif


    /* initialize object module loader */

    bootElfInit ();              /* MIPS only uses elf format */


    /* use cpu0's boot string as a starting point */

    bootStringToStruct (BOOT_LINE_ADRS, &params);

    bootFtp = (params.passwd[0] != EOS);

    /*
     * if a kernelFile is specified use it, 
     * otherwise append a '1' to the filename of cpu0's BOOT_LINE
     */

    if (kernelFile != EOS)
        {
        sprintf(params.bootFile, "%s", kernelFile) ;
        }
    else
        {
        sprintf(params.bootFile, "%s1", params.bootFile) ;
        }

    /*
     * begin the kernel image download process
     */

    printf ("Secondary kernel image is %s\n", params.bootFile);
    printf ("Loading kernel... ");

#ifdef INCLUDE_TFTP_CLIENT
    if (params.flags & SYSFLG_TFTP)         /* use tftp to get image */
       {
       if (tftpXfer (params.had, 0, params.bootFile, "get", "binary", 
                      &fd, &errFd) == ERROR)
           return (ERROR);
       }
    else
#endif
       {
       if (bootFtp)
           {

           if (ftpXfer (params.had, params.usr, params.passwd, "", "RETR %s",
                        "", params.bootFile, &errFd, &fd) == ERROR)
               return (ERROR);
           }
       else
           {
           bootRsh = TRUE;
           sprintf (command, "cat %s", params.bootFile);

           fd = rcmd (params.had, RSHD, params.usr, params.usr, 
                      command, &errFd);
           if (fd == ERROR)
               return (ERROR);
           }
       }

    if (bootLoadModule (fd, &entry) != OK)
        goto readErr;

    printf("Starting cpu1 at 0x%x\n\n", (int)entry);


#ifdef INCLUDE_TFTP_CLIENT
    /*
     * Successful TFTP transfers don't need any cleanup. The tftpXfer()
     * routine closes all file descriptors once the data has been
     * retrieved from the remote host.
     */

    if (params.flags & SYSFLG_TFTP)
        {
#ifdef INCLUDE_CFE_SUPPORT
        coreSp = (long)(entry - (4 * _RTypeSize));
        cfe_cpu_start( (int)SECONDARY_CORE,entry,(long)coreSp,
                       (long)_gp,(long)0);
#else 
        sysAltGo (entry);
#endif
        return (OK);
        }
#endif


    if (bootRsh == FALSE)
        {

        /* Empty the Data Socket before close. PC FTP server hangs otherwise */

        while ((read (fd, command, sizeof (command))) > 0);

        if (bootFtp)
            (void) ftpCommand (errFd, "QUIT",0,0,0,0,0,0);
        }

    close (fd);
    close (errFd);
#ifdef INCLUDE_CFE_SUPPORT
        coreSp = (long)(entry - (4 * _RTypeSize));
        cfe_cpu_start( (int)SECONDARY_CORE,entry,(long)coreSp,
                       (long)_gp,(long)0);
#else 
    sysAltGo (entry);
#endif

    return (OK);

readErr:
    /* check standard error on Unix */

    if (bootRsh == FALSE)
        {

        /* Empty the Data Socket before close. PC FTP server hangs otherwise */

        while ((read (fd, command, sizeof (command))) > 0);

        if (bootFtp)
            {
            (void) ftpReplyGet (errFd, FALSE); /* error message on std. err */
            (void) ftpCommand (errFd, "QUIT",0,0,0,0,0,0);
            }
        }
    else
        {
        char buf [100];
        int errBytesRecv = fioRead (errFd, buf, sizeof (buf));

        if (errBytesRecv > 0)
            {
            /* print error message on standard error fd */

            buf [errBytesRecv] = EOS;
            printf ("\n%s:%s: %s\n", params.had, params.bootFile, buf);
            }
        }

    close (fd);
    close (errFd);

    return (ERROR);
    }
#endif
/* defined(SB1_CPU_0) && defined(INCLUDE_BOOT_LINE_INIT) && 
 * defined(INCLUDE_NETWORK)
 */

/*------------------------------------------------------------------------*/

#if defined(INCLUDE_VXMP_TESTS) && defined(INCLUDE_SM_OBJ)

#define SEM_NAME "mySharedSem"
#define DELAY_TICKS 200

/*******************************************************************************
*
* semTask1 - User-callable test function for shared semaphores.
*
* This routine implements the "first CPU" part of the shared semaphore
* test.  It is designed to run in parallel with semTask2.
*
* RETURNS: OK or ERROR.
*
* ERRNO
*
* SEE ALSO: semTask2
*/

STATUS semTask1 (void)
    {
    int thisTick;
    SEM_ID semSmId;

    /* created shared semaphore */

    if ((semSmId = semBSmCreate (SEM_Q_FIFO, SEM_FULL)) == NULL)
	{
	printf( "semTask1: can't create shared semaphore\n");
	return ERROR;
	}

    /* add object to name database */

    if (smNameAdd (SEM_NAME, semSmId, T_SM_SEM_B) == ERROR)
	{
	printf ("semTask1: can't name the semaphore\n");
	return ERROR;
	}

    /* grab shared semaphore and hold it for a while */

    semTake (semSmId, WAIT_FOREVER);

    /* normally do something useful */

    printf( "Task1 has the shared semaphore\n");
    for (thisTick = 0; thisTick < DELAY_TICKS; thisTick++)
	{
	printf( "taskDelay(tick %d of %d)\n", thisTick, DELAY_TICKS);
	taskDelay (1);
	}

    printf( "Task1 has released the shared semaphore\n");
    semGive (semSmId);

    printf( "Task1 killing time\n");
    for (thisTick = 0; thisTick < DELAY_TICKS / 2; thisTick++)
	{
	printf( "taskDelay(tick %d of %d)\n", thisTick, DELAY_TICKS / 2);
	taskDelay (1);
	}


    printf( "Task1 retaking shared semaphore\n");

    /* grab shared semaphore and hold it for a while */

    semTake (semSmId, WAIT_FOREVER);

    printf( "Task1 now has the shared semaphore\n");

    return OK;
    }

/*******************************************************************************
*
* semTask2 - User-callable test function for shared semaphores.
*
* This routine implements the "second CPU" part of the shared semaphore
* test.  It is designed to run in parallel with semTask1.
*
* RETURNS: OK or ERROR.
*
* ERRNO
*
* SEE ALSO: semTask1
*/

STATUS semTask2 (void)
    {
    STATUS s;
    SEM_ID semSmId;
    int objType;
    int thisTick;

    /* find object in name database */
    
    printf( "semTask2: starting\n");

    s = smNameFind (SEM_NAME, (void **) &semSmId, &objType, WAIT_FOREVER);
    if (s == ERROR)
	{
	printf ("semTask2: can't find %s\n", SEM_NAME);
	return ERROR;
	}

    /* take the shared semaphore */

    printf ("semTask2 is now going to take the shared semaphore\n");
    semTake (semSmId, WAIT_FOREVER);

    /* normally do something useful */

    printf ("semTask2 got the shared semaphore\n");

    printf( "semTask2 killing time\n");
    for (thisTick = 0; thisTick < DELAY_TICKS; thisTick++)
	{
	printf( "taskDelay(tick %d of %d)\n", thisTick, DELAY_TICKS);
	taskDelay (1);
	}

    /* release the shared semaphore */

    semGive (semSmId);

    printf ("semTask2 has released the shared semaphore.\n");

    return OK;
    }

#endif /* INCLUDE_VXMP_TESTS && INCLUDE_SM_OBJ */

#ifdef INCLUDE_CFE_SUPPORT
/******************************************************************************
*
* cfeInit - Query CFE for information
*
* This routine queries CFE for information about the board/system,
* and sets up datastructures necessary for the BSP to take advantage
* of CFE capabilities.
*
* RETURNS: OK or ERROR.
*
* ERRNO
*
*/
STATUS cfeInit (void)
    {
    int i;
    uint64_t cfeHandle = 0;
    uint64_t cfeEpt = 0;
    uint32_t cfeEptSeal = 0;
    char ipaddr[24];
    char cpu_type[16];
    char cpu_speed[8];
    char cpu_rev[8];
    char num_cores[4];
    char boardname[16];
    char boot_type[16];
    int cpunum = (sysPridGet () >> 25) & 0x7;

    /* Try to determine if we booted from CFE */
    if (((int)cfeArgs[0]) < 0)
	{
	cfeHandle = cfeArgs[0];
	cfeEpt = cfeArgs[2];
	cfeEptSeal = (uint32_t) cfeArgs[3];
	}
    if (!cfeHandle)
	return (ERROR);

    if (cfeEptSeal != CFE_EPTSEAL)
	return (ERROR);

    cfeBoot = TRUE;
    cfe_init (cfeHandle, cfeEpt);

    /* Find available memory */
    for (i = 0; i < CFE_MEM_MAX; ++i)
	cfe_enummem (i, 0, &cfeMem[i].start, &cfeMem[i].len, &cfeMem[i].type);

    /* determine boot type */
    if (!cfe_getenv ("vxwboottype", boot_type, sizeof(boot_type)))
	{
	sysStartType = atoi(boot_type);
	cfe_unsetenv ("vxwboottype");

	}
    else
	sysStartType = BOOT_CLEAR;

    /* if there is a bootline in RAM, assume it's valid and use it */

    if (strlen(BOOT_LINE_ADRS) == 0)
	{	
	/* Build up a VxWorks bootline from CFE env variables */

	/* First, try to read nvram */

	/* a warm boot will have saved the ram copy of the bootline as the
	 * CFE environment variable 'vxwarmbootline'. If this environment
	 * variable exists, we copy it back into ram and sysBootLine, and
	 * erase the environment variable.
	 */
	if ((cfe_getenv ("vxwarmbootline",sysBootLine,BOOT_LINE_SIZE) == OK) &&
	    (cpunum == 0))
	    cfe_unsetenv ("vxwarmbootline");
	else if (cfe_getenv ("vxwbootline", sysBootLine, BOOT_LINE_SIZE) != OK)
	    {
	    /* fallback strategy: manufacture a bootline */
	    if (cfe_getenv ("NET_IPADDR", ipaddr, sizeof(ipaddr)))
		return (ERROR);
	    sprintf (sysBootLine, "sbe(0,0)host:vxWorks e=%s u=target",
		     ipaddr);
	    }

	/* whatever bootline we settled on, copy it to BOOT_LINE_ADRS */
	bcopy(sysBootLine, BOOT_LINE_ADRS,BOOT_LINE_SIZE);
	}

    if (!cfe_getenv ("CPU_SPEED", cpu_speed, sizeof(cpu_speed)))
	{
	sb1_clock_rate = atoi(cpu_speed) * 1000000;

	if (!cfe_getenv ("CPU_TYPE", cpu_type, sizeof(cpu_type))
	    && !cfe_getenv ("CPU_REVISION", cpu_rev, sizeof(cpu_rev))
	    && !cfe_getenv ("CPU_NUM_CORES", num_cores, sizeof(num_cores))
	    && !cfe_getenv ("CFE_BOARDNAME", boardname, sizeof(boardname)))
	    {
	    sprintf (sysModelStr,
		     "Broadcom BCM%s: %s-%s cores @%sMHz, %s board",
		     cpu_type, num_cores, cpu_rev, cpu_speed, boardname);
	    }
	}

    return (OK);
    }

#ifdef INCLUDE_SHOW_ROUTINES

/******************************************************************************
*
* cfeDisplay - Query CFE for information and display it
*
* This routine queries CFE for information about the board/system
* and displays it using printf.
*
* RETURNS: OK or ERROR.
*
* ERRNO
*
*/
STATUS cfeDisplay (void)
    {
    int i;
    char name[256];
    char val[512];

    for (i = 0; i < CFE_MEM_MAX && cfeMem[i].len; ++i)
	{
	printf ("mem%d: 0x%08x%08x 0x%08x%08x 0x%08x%08x\n",
		i,
		(int)(cfeMem[i].start >> 32), (int) cfeMem[i].start,
		(int)(cfeMem[i].len >> 32), (int) cfeMem[i].len,
		(int)(cfeMem[i].type >> 32), (int) cfeMem[i].type);
	}
    for (i = 0; !cfe_enumenv (i, name, sizeof(name), val, sizeof(val)); ++i)
	{
	printf ("%s=%s\n", name, val);
	}
    return (OK);
    }
#endif	/* INCLUDE_SHOW_ROUTINES */
#endif	/* INCLUDE_CFE_SUPPORT */

#if defined(_WRS_CONFIG_SMP)

/* forward declarations */
void sysCpuInit (int, WIND_CPU_STATE *);

/* globals */

int sysCpuLoopCount[SB_MAX_SMP_CPUS] = {0};

/* statics */

FUNCPTR sysCpuInitTable[SB_MAX_SMP_CPUS] = {NULL};
#endif /* _WRS_CONFIG_SMP */

#if defined (_WRS_CONFIG_SMP)
#if defined (INCLUDE_CFE_SUPPORT)
/***************************************************************************
*
* sysCpuStart - start a SB1 core
*
* This function acts as a wrapper around the cfe_cpu_start function to
* provide the vxWorks SMP equivalent API, sysCpuStart().
*
* N.B.: This function currently assumes that CFE is available.
*
* The cfe_cpu_start() function requires 5 parameters:
*
* int cpu;		/@ core number to start @/
* void (*fn)(void);	/@ pointer to initial PC @/
* long sp;		/@ value of initial stack pointer @/
* long gp;		/@ value of initial global pointer @/
* long a1;		/@ value to be passed to a1 at startup @/
*
* The sysCpuStart() function takes two parameters:
* int cpu;		/@ core number to start @/
* REG_SET *pRegs;	/@ pointer to a REG_SET structure @/
*
* The intent is to implement a function with the basic features of
* vxTaskRegsInit() that sets up the regs structure, then passes the
* cpu number and pRegs pointer to this function, which in turn extracts
* the needed values from the regs structure and calls cfe_cpu_start.
*
* RETURNS: OK
*
* NOMANUAL
*/

#if defined (INCLUDE_MAPPED_KERNEL)
#define SYS_CPU_INIT	KM_TO_K0(sysCpuInit)
#else /* INCLUDE_MAPPED_KERNEL */
#define SYS_CPU_INIT	sysCpuInit
#endif /* INCLUDE_MAPPED_KERNEL */

STATUS sysCpuStart(int cpu, WIND_CPU_STATE *cpuState)
    {
    #if 0 /* added for godson3 yinwx, 20091219 */
    return (cfe_cpu_start(cpu,
			  (void (*)(void))SYS_CPU_INIT,
			  (long)cpuState->regs.spReg,
			  (long)cpuState->regs.gpReg,
			  (long)&cpuState->regs));
	#else
	godson3_cpu_start(cpu,
			  (void (*)(void))SYS_CPU_INIT,
			  (long)cpuState->regs.spReg,
			  (long)cpuState->regs.gpReg,
			  (long)cpuState->regs.a1Reg);
	return OK;
	
	#endif
    }

/***************************************************************************
*
* sysCpuStop - stop a SB1 core
*
* This function acts as a wrapper around the cfe_cpu_stop function to
* provide the vxWorks SMP equivalent API, sysCpuStop().
*
* N.B.: This function currently assumes that CFE is available.
*
* The cfe_cpu_start() function requires 1 parameter:
*
* int cpu;		/@ core number to stop @/
*
* The sysCpuStop() function takes one parameter:
* int cpu;		/@ core number to stop @/
*
* RETURNS: OK
*
* NOMANUAL
*/
STATUS sysCpuStop(int cpu)
    {
    return (cfe_cpu_stop(cpu));
    }
#endif /* INCLUDE_CFE_SUPPORT */

/******************************************************************************
 *
 * sysCpuEnable - enable a multi core CPU
 * 
 * This routine brings a CPU out of reset
 *
 * RETURNS:  0 - Success
 *          -1 - Fail
 * 
 */

STATUS sysCpuEnable(unsigned int cpuNum, WIND_CPU_STATE *cpuState)
    {
    if ((cpuNum < 1) || (cpuNum > (sysCpuAvailableGet ()-1)))
        {
	return -1;
        }

    sysCpuInitTable[cpuNum] = (FUNCPTR) cpuState->regs.pc;
    return sysCpuStart(cpuNum, cpuState);
    }
    
/******************************************************************************
 *
 * sysCpuDisable - disable a multi core CPU
 * 
 * This routine shuts down the specified core.
 *
 * RETURNS:  0 - Success
 *          -1 - Fail
 * 
 */


STATUS sysCpuDisable(int cpuNum)
    {
    return sysCpuStop(cpuNum);
    }

#endif /* if defined(_WRS_CONFIG_SMP) */
    
/******************************************************************************
 *
 * sysCpuAvailableGet - Determine the number of cores present
 *
 * this routine returns the number of cores
 * 
 * RETURNS: CPU number (0-7)
 * 
 */

UINT32 sysCpuAvailableGet(void)
    {
    #if 0
    return ((*(unsigned long long *)PHYS_TO_K1(A_SCD_SYSTEM_REVISION)) >> 24)
      & 0x7;
	#else
	return 4;
	#endif
    }

#ifdef INCLUDE_VXBUS

#ifdef INCLUDE_DMA_SYS

/*******************************************************************************
*
* sb1PlbDmaMapCreate - SB1 custom DMA map creation routine
*
* This function implements a custom DMA map creation method for devices
* on the SB1's processor local bus. The only special thing it really needs
* to do is set up the map with pointers to the other custom map handling
* routines provided by this module. In particular, we need to override
* the map load routines so that we can do address translation.
*
* RETURNS: pointer to new map if creation suceeds, otherwise NULL
*
* ERRNO: N/A
*/

LOCAL VXB_DMA_MAP_ID sb1PlbDmaMapCreate
    (
    VXB_DEVICE_ID pDev,
    VXB_DEVICE_ID pCreator,
    VXB_DMA_TAG_ID dmaTagID,
    int flags,
    VXB_DMA_MAP_ID * ppMap
    )
    {
    VXB_DMA_MAP_ID pMap;

    /*
     * In order to avoid some code duplication here, we
     * re-use the vxbDmaBufMapCreate() routine. When we call
     * it, we pass in our own VxBus device instance. Since
     * we don't have a parent, that routine will just create
     * a map for us and return. We then override the creator
     * device and method pointers that we need to.
     */

    pMap = vxbDmaBufMapCreate (pDev, dmaTagID, flags, ppMap);

    if (pMap == NULL)
        return (NULL);

    pMap->pDev = pCreator;

    /* Insert our custom methods */

    pMap->mapSyncFunc = sb1PlbDmaMapSync;

    return (pMap);
    }

/*******************************************************************************
*
* sb1PlbDmaMapSync - make mapped DMA buffers cache coherent
*
* This routine is used to provide a small optimization for consumers of
* the VxBus DMA API. The Broadcom SB1 platform is cache coherent, so
* no special flush or invalidate operations are needed. Providing this
* dummy no-op sync routine saves a few cycles, since it's shorter than
* the generic one provided in vxbDmaBufLib.
*
* RETURNS: always OK
*
* ERRNO: N/A
*/

LOCAL STATUS sb1PlbDmaMapSync
    (
    VXB_DEVICE_ID       pInst,
    VXB_DMA_TAG_ID      dmaTagID,
    VXB_DMA_MAP_ID      map,
    bus_dmasync_op_t    op
    )
    {
    return (OK);
    }

#endif /* INCLUDE_DMA_SYS */


/*******************************************************************************
*
* sb1RevisionGet - get silicon revision information
*
* Save the silicon part number and revision into partNum and revNum
* respectively
*
* RETURNS: ERROR if partNum or revNum are either NULL pointers else OK
*
* ERRNO: N/A
*/

LOCAL STATUS sb1RevisionGet
    (
    VXB_DEVICE_ID pDev,
    UINT16 * partNum,
    UINT8 * revNum
    )
    {
    UINT64 revInfo;

    if (partNum == NULL || revNum == NULL)
        return (ERROR);

    revInfo = *(UINT64 *)PHYS_TO_K1(A_SCD_SYSTEM_REVISION);

    *partNum = G_SYS_PART(revInfo);
    *revNum = G_SYS_REVISION(revInfo);

    return (OK);
    }

#endif /* INCLUDE_VXBUS */

#ifdef INCLUDE_END

/******************************************************************************
*
* sysSbeMacEnetAddrGet - get the hardware Ethernet address
*
* This routine gets the hardware ethernet address from nvRAM and passes it
* back to the sb1 END.
*
* RETURNS: N/A
*
* ERRNO
*
* SEE ALSO: sbeMacEndLoad()
*/

void sysSbeEnetAddrGet
    (
    int unit,
    uint8_t * enetAdrs
    )
    {
    char hwAddr[18];
    char envName[12];

    /*
     * following strings are field descriptors used by Broadcom's CFE
     * refer to CFE documentation for full details
     */
    sprintf (envName, "ETH%d_HWADDR", unit);

#ifdef INCLUDE_CFE_SUPPORT
    if (!cfe_getenv(envName, hwAddr, sizeof(hwAddr))
	&& !parse_hwaddr(hwAddr, enetAdrs))
	return;
#else
    if (nvramEnvGet(envName, hwAddr, 17) == OK)
	if (parse_hwaddr(hwAddr, enetAdrs) == 0)
	    return;
#endif	/* INCLUDE_CFE_SUPPORT */

    enetAdrs[0] = 0x00;
    enetAdrs[1] = 0x02;
    enetAdrs[2] = 0x4c;
    enetAdrs[3] = 0xff;
    enetAdrs[4] = 0x00;
    enetAdrs[5] = 0x20 + unit;
    }

/**********************************************************************
* parse_xdigit - parse a hex digit
*
* This routine parses a hex digit and returns its value.
*
* RETURNS: hex value, or -1 if invalid
*
* ERRNO
*/
LOCAL int parse_xdigit
    (
    char str
    )
    {
    int digit;

    if ((str >= '0') && (str <= '9'))
        digit = str - '0';
    else if ((str >= 'a') && (str <= 'f'))
        digit = str - 'a' + 10;
    else if ((str >= 'A') && (str <= 'F'))
        digit = str - 'A' + 10;
    else
        return -1;

    return digit;
    }


/**********************************************************************
* parse_hwaddr - convert a string to Ethernet Address
*
* This routine converts a string in the form xx:xx:xx:xx:xx:xx into
* a 6-byte Ethernet address.
*
* RETURNS: 0 if ok, else -1
*
* ERRNO
*/
LOCAL int parse_hwaddr
    (
    char *str,
    uint8_t *hwaddr
    )
    {
    int digit1,digit2;
    int idx = 6;

    while (*str && (idx > 0))
        {
        digit1 = parse_xdigit(*str);
        if (digit1 < 0)
            return -1;
        str++;
        if (!*str)
            return -1;

        if ((*str == ':') || (*str == '-'))
            {
            digit2 = digit1;
            digit1 = 0;
            }
        else
            {
            digit2 = parse_xdigit(*str);
            if (digit2 < 0)
                return -1;
            str++;
            }

        *hwaddr++ = (digit1 << 4) | digit2;
        idx--;

        if (*str == '-')
            str++;
        if (*str == ':')
            str++;
        }
    return 0;
}
#endif	/* INCLUDE_END */

#ifdef INCLUDE_SIO_UTILS   /* changed mxl: DRV_SIO_SB1 */
SIO_CHAN * bspSerialChanGet
    (
    int channel     /* serial channel */
    )
    {
    return ((SIO_CHAN *) ERROR);
    }
    
#endif /* DRV_SIO_SB1 */

#ifdef DRV_TIMER_MIPSR4K
/******************************************************************************
*  vxbR4KTimerFrequencyGet - to get the frequency
*
* The CPU frequency is determined by the function cfeInit() in sysLib.c.
*
* RETURNS: the frequncy of the CPU
*
* ERRNO: none
*/

int vxbR4KTimerFrequencyGet
    (
    struct vxbDev * pInst
    )
    {
    return sb1_clock_rate;
    }

#endif /* DRV_TIMER_MIPSR4K */
int bslProcGetId (void)
{
	int node = getNodeNumberZR() & 0x03;
	return (node);
}

unsigned short  getNodeZR()
{
      unsigned short node = getNodeNumberZR() & 0x03;
/*      printstr("Local Node Number is: "); printnum((unsigned long long)node); printstr("\r\n");*/
      return (node);
}
/*add by wangzx for debug*/
void sysShowMem(unsigned int chip,unsigned int baseaddr)
{
	unsigned int i;
	unsigned int dst;
	unsigned int base0;
	unsigned int addr0;
	unsigned int addr4;
	unsigned int val0;
	unsigned int val4;
	dst = chip&0x3;
	base0 = baseaddr;

	for(i = 0;i<0x32;i++)
	{
		addr0 = base0 + i * 8;
		addr4 = addr0 + 4;
		val0 = CPU_READ32(addr0,dst);
		val4 = CPU_READ32(addr4,dst);
		printf("%08x:  %08x   %08x\n",addr0,val0,val4);
	}
	return;
}

/*added by mxl for debug */
STATUS sysPrintfPoll(char *pString, int nchars, int outarg)
{
    int msgIx;
	for(msgIx = 0; msgIx < nchars; msgIx++)
	{
	    #if 1
	     if (pString[msgIx] == '\n')
            {
                //while(sioPollOutput (pSioChan, '\r') == EAGAIN);
                while (((*(volatile unsigned char*)(0xbfe001e0+5)) & 0x20)==0);
                *(unsigned char*)0xbfe001e0 = pString[msgIx];
                while (((*(volatile unsigned char*)(0xbfe001e0+5)) & 0x20)==0); 
            }
            //while(sioPollOutput (pSioChan, pString[msgIx]) == EAGAIN);
            while (((*(volatile unsigned char*)(0xbfe001e0+5)) & 0x20)==0);
             *(unsigned char*)0xbfe001e0 =pString[msgIx];
             //logMsg("pString[msgIx]=%d \r\n",pString[msgIx] );
             while (((*(volatile unsigned char*)(0xbfe001e0+5)) & 0x20)==0); 
        #else

        printstr(pString);
        #endif
	}
    return (OK);

}

extern int fioFormatV
    (
    FAST const char * fmt,         /* format string */
    va_list           vaList,      /* pointer to varargs list */
    FUNCPTR           outRoutine,  /* handler for args as they're formatted */
    int               outarg       /* argument to routine */
    );
    
int bsp_printf(const char *  fmt, ...)
{
    va_list vaList;	
    int nChars;
    FUNCPTR outRoutine;
    int     outArg;
    
    outRoutine = (FUNCPTR)sysPrintfPoll;
    outArg     = CONSOLE_TTY;

    va_start (vaList, fmt);
    nChars = fioFormatV (fmt, vaList, sysPrintfPoll, 1);
    va_end (vaList);
    return nChars;
}

void testPrintf()
{
    int a =  12;
    bsp_printf("abcd = %d", a);
}

extern unsigned int readHt0_0(unsigned int address);
extern void writeHt0_0(unsigned int address, unsigned int data);


//#define I2C_BASE_ADDRESS  0xbe030000

#define I2C_REGISTER_WR(addr,data)  writeHt0_0((addr | 0x30000), data)   
#define I2C_REGISTER_RD(addr)       readHt0_0((addr | 0x30000))

#define I2C_LOOP   for( loop = 1000; loop > 0; loop--);

void I2cInit()
{
 //   I2C_REGISTER_WR( 0x00 ,0x31 );
 //   I2C_REGISTER_WR( 0x04 ,0x00 );
    I2C_REGISTER_WR( 0x00 ,0x31 );
    I2C_REGISTER_WR( 0x04 ,0x00 );
    I2C_REGISTER_WR( 0x08 ,0xc0 );
    I2C_LOOP;
}



void I2CWrite(UINT8 dev, UINT8 reg, UINT8 data)
{
    
    I2C_REGISTER_WR( 0x0c ,dev );
    I2C_REGISTER_WR( 0x10 ,0x90 );
    I2C_LOOP;

    I2C_REGISTER_WR( 0x0c ,reg );
    I2C_REGISTER_WR( 0x10 ,0x11 );
    I2C_LOOP;

    I2C_REGISTER_WR( 0x0c ,data );
    I2C_REGISTER_WR( 0x10 ,0x59 );
    I2C_LOOP;
}

UINT32 I2CRead(UINT8 dev, UINT8 reg )
{
    I2C_REGISTER_WR( 0x0c ,dev );
    I2C_REGISTER_WR( 0x10 ,0x90 );
    I2C_LOOP

    I2C_REGISTER_WR( 0x0c ,reg );
    I2C_REGISTER_WR( 0x10 ,0x11 );
    I2C_LOOP
    
    I2C_REGISTER_WR( 0x0c ,(dev+1) );
    I2C_REGISTER_WR( 0x10 ,0x91 );    
    I2C_LOOP

    I2C_REGISTER_WR( 0x0c ,0x0 );
    I2C_REGISTER_WR( 0x10 ,0x68 );
    
    for( loop = 15; loop > 0; loop--);
    I2C_LOOP  /* mxl added */
	
    return ( I2C_REGISTER_RD(0x0c));
}

void I2cStart()
{
    I2C_REGISTER_WR( 0x00 ,0x31 );
    I2C_REGISTER_WR( 0x04 ,0x00 );
    I2C_REGISTER_WR( 0x08 ,0xc0 );
    I2C_LOOP;
}


#if 1

UINT32 bspTempRead(int times)
{
	UINT32  tempA,tempB,tempC;
	double obcTemp,fpga1Temp,fpga2Temp,fpga3Temp,cpuTemp,v09Temp,v11Temp;
	int num=1;
	
	if(times==0)times=1;
	
    I2cInit();
    I2CWrite(0x90, 1, 0xf8);
    I2CWrite(0x90, 6, 0x33);
    I2CWrite(0x90, 7, 0x33);
    I2CWrite(0x90, 8, 0x78);
	BIG_LOOP;
	BIG_LOOP;
	BIG_LOOP;
	
	printf("Temp OBC	FPGA1	FPGA2	FPGA3	CPU   V0.9   V1.1\n");
	while(times--)
	{
		tempB = I2CRead(0x90, 0x16);
		tempC = I2CRead(0x90, 0x17);
		tempA = ((tempB & 0x1f)<<8 | tempC) ;
		//fpga1Temp = 0.0625* tempA;
		if((tempB&0x10)==0x00)   
		{
			tempA= ((tempB&0x1f)<<8 |tempC);
			fpga1Temp= tempA * 0.0625;
		}
	else
		{
			
			tempA= ((tempB&0x1f)<<8 | tempC);
			tempA=(~tempA)&0x1fff;
			fpga1Temp= (tempA+1)*(-0.0625); 
	     }
			
		tempB = I2CRead(0x90, 0x12);
		tempC = I2CRead(0x90, 0x13);
		tempA = ((tempB & 0x1f)<<8 | tempC) ;
		//fpga2Temp = 0.0625* tempA;
		if((tempB&0x10)==0x00)   
		{
			tempA= ((tempB&0x1f)<<8 |tempC);
			fpga2Temp= tempA * 0.0625;
		}
	else
		{
			
			tempA= ((tempB&0x1f)<<8 | tempC);
			tempA=(~tempA)&0x1fff;
			fpga2Temp= (tempA+1)*(-0.0625); 
	     }
		
		tempB = I2CRead(0x90, 0xe);
		tempC = I2CRead(0x90, 0xf);
		tempA = ((tempB & 0x1f)<<8 | tempC) ;
		fpga3Temp = 0.0625* tempA;
		if((tempB&0x10)==0x00)   
		{
			tempA= ((tempB&0x1f)<<8 |tempC);
			fpga3Temp= tempA * 0.0625;
		}
	else
		{
			
			tempA= ((tempB&0x1f)<<8 | tempC);
			tempA=(~tempA)&0x1fff;
			fpga3Temp= (tempA+1)*(-0.0625); 
	     }
		
		tempB = I2CRead(0x90, 0xa);
		tempC = I2CRead(0x90, 0xb);
		tempA = ((tempB & 0x1f)<<8 | tempC) ;
		//obcTemp = 0.0625* tempA;
		if((tempB&0x10)==0x00)   
		{
			tempA= ((tempB&0x1f)<<8 |tempC);
			obcTemp= tempA * 0.0625;
		}
	else
		{
			
			tempA= ((tempB&0x1f)<<8 | tempC);
			tempA=(~tempA)&0x1fff;
			obcTemp= (tempA+1)*(-0.0625); 
	     }

		BIG_LOOP;
		BIG_LOOP;
		I2cInit();
    	I2CWrite(0x92, 1, 0x78);
    	I2CWrite(0x92, 6, 0x33);
    	I2CWrite(0x92, 7, 0x33);
    	I2CWrite(0x92, 8, 0x78);
		BIG_LOOP;
		BIG_LOOP;
		BIG_LOOP;
		tempB = I2CRead(0x92, 0xa);
		tempC = I2CRead(0x92, 0xb);
		tempA = ((tempB & 0x1f)<<8 | tempC) ;
		if((tempB&0x10)==0x00)   
		{
			tempA= ((tempB&0x1f)<<8 |tempC);
			cpuTemp= tempA * 0.0625;
		}
	else
		{
			
			tempA= ((tempB&0x1f)<<8 | tempC);
			tempA=(~tempA)&0x1fff;
			cpuTemp= (tempA+1)*(-0.0625); 
	     }

		tempB = I2CRead(0x92, 0xe);
		tempC = I2CRead(0x92, 0xf);
		tempA = ((tempB & 0x1f)<<8 | tempC) ;
		if((tempB&0x10)==0x00)   
		{
			tempA= ((tempB&0x1f)<<8 |tempC);
			v09Temp= tempA * 0.0625;
		}
	else
		{
			
			tempA= ((tempB&0x1f)<<8 | tempC);
			tempA=(~tempA)&0x1fff;
			v09Temp= (tempA+1)*(-0.0625); 
	     }

		tempB = I2CRead(0x92, 0x12);
		tempC = I2CRead(0x92, 0x13);
		tempA = ((tempB & 0x1f)<<8 | tempC) ;
		if((tempB&0x10)==0x00)   
		{
			tempA= ((tempB&0x1f)<<8 |tempC);
			v11Temp= tempA * 0.0625;
		}
	else
		{
			
			tempA= ((tempB&0x1f)<<8 | tempC);
			tempA=(~tempA)&0x1fff;
			v11Temp= (tempA+1)*(-0.0625); 
	     }		
		printf("%d: 	%.2f	%.2f	%.2f	%.2f	%.2f    %.2f   %.2f\n",num++,obcTemp,fpga1Temp,fpga2Temp,fpga3Temp,cpuTemp,v09Temp,v11Temp);
		taskDelay(10);
	}
	return 0;
}
#endif



#if 1

UINT32 bspVolRead(int times)
{
	UINT32 volA,volB,volC;
	int i=1;
	double vol33,vol30,vol25,vol18,vol15,vol12,vol11,vol09;
	
	I2cInit();	
	I2CWrite(0x94,1,0xf0);
	I2CWrite(0x94,6,0x00);
	I2CWrite(0x94,7,0x00);
	BIG_LOOP;
	BIG_LOOP;
	printf("vol  0.9 	1.1 	1.2 	1.5 	1.8 	2.5 	3.0 	3.3\n");
	if(times==0)times=1;
	while(times--)
	{
		volA=I2CRead(0x94,0x18);
		volB=I2CRead(0x94,0x19);
		volC= ((volA & 0x3f)<<8 |volB);
		vol09= 305.18 * volC /1000000;
		
		volA=I2CRead(0x94,0x16);
		volB=I2CRead(0x94,0x17);
		volC= ((volA & 0x3f)<<8 |volB);
		vol11= 305.18 * volC /1000000;
		
		volA=I2CRead(0x94,0x14);
		volB=I2CRead(0x94,0x15);
		volC= ((volA & 0x3f)<<8 |volB);
		vol12= 305.18 * volC /1000000;
		
		volA=I2CRead(0x94,0x12);
		volB=I2CRead(0x94,0x13);
		volC= ((volA & 0x3f)<<8 |volB);
		vol15= 305.18 * volC /1000000;
		
		volA=I2CRead(0x94,0x10);
		volB=I2CRead(0x94,0x11);
		volC= ((volA & 0x3f)<<8 |volB);
		vol18= 305.18 * volC /1000000;
		
		volA=I2CRead(0x94,0xe);
		volB=I2CRead(0x94,0xf);
		volC= ((volA & 0x3f)<<8 |volB);
		vol25= 305.18 * volC /1000000;
		
		volA=I2CRead(0x94,0xc);
		volB=I2CRead(0x94,0xd);
		volC= ((volA & 0x3f)<<8 |volB);
		vol30= 305.18 * volC /1000000;
		
		volA=I2CRead(0x94,0xa);
		volB=I2CRead(0x94,0xb);
		volC= ((volA & 0x3f)<<8 |volB);
		vol33= 305.18 * volC /1000000;
		
printf("%d   %.2f	%.2f	%.2f	%.2f	%.2f	%.2f	%.2f	%.2f\n",i++,vol09,vol11,vol12,vol15,vol18,vol25,vol30,vol33);
	taskDelay(10);
	}
	return 0;
}
#endif


#if 1
UINT32 bspCurRead(int times)
{
	UINT32 curA,curB,curC;
	int i=1;
	double cur33,cur30,cur25,cur18,cur15,cur12,cur11,cur09;
	
	printf("cur  0.9 	1.1 	1.2 	1.5  1.8	2.5	3.0	3.3\n");
	I2cInit();	
	I2CWrite(0x96,1,0xf0);
	I2CWrite(0x96,6,0x11);
	I2CWrite(0x96,7,0x11);
	BIG_LOOP;
	BIG_LOOP;
	
	if(times==0)times=1;
	while(times--)
	{
		curA=I2CRead(0x96,0x0c);
		curB=I2CRead(0x96,0x0d);
		if((curA&0x40)==0x00)   /*MSB'S bit[6] is sign,if sign=0 */
		{
			curC= ((curA&0x3f)<<8 |curB);
			cur09= curC * 19.0735/500;
		}
	else
		{
			
			curC= ((curA&0x3f)<<8 | curB);
			curC=(~curC)&0x3fff;
			cur09= (curC+1)*(-19.0735)/500; 
	     }
		
		
		curA=I2CRead(0x96,0x10);
		curB=I2CRead(0x96,0x11);
		if((curA&0x40)==0x00)   /*MSB'S bit[6] is sign,if sign=0 */
		{
			curC= ((curA&0x3f)<<8 |curB);
			cur11= curC * 19.0735/500;
		}
	else
		{
			
			curC= ((curA&0x3f)<<8 | curB);
			curC=(~curC)&0x3fff;
			cur11= (curC+1)*(-19.0735)/500; 
	     }
		
		curA=I2CRead(0x96,0x14);
		curB=I2CRead(0x96,0x15);
		if((curA&0x40)==0x00)   /*MSB'S bit[6] is sign,if sign=0 */
		{
			curC= ((curA&0x3f)<<8 |curB);
			cur12= curC * 19.0735/10000;
		}
	else
		{
			
			curC= ((curA&0x3f)<<8 | curB);
			curC=(~curC)&0x3fff;
			cur12= (curC+1)*(-19.0735)/10000; 
	     }
		
		curA=I2CRead(0x96,0x18);
		curB=I2CRead(0x96,0x19);
		if((curA&0x40)==0x00)   /*MSB'S bit[6] is sign,if sign=0 */
		{
			curC= ((curA&0x3f)<<8 |curB);
			cur15= curC * 19.0735/10000;
		}
	else
		{
			
			curC= ((curA&0x3f)<<8 | curB);
			curC=(~curC)&0x3fff;
			cur15= (curC+1)*(-19.0735)/10000; 
	     }
		
		
		BIG_LOOP;
		BIG_LOOP;
		I2cInit();	                    
    I2CWrite(0x98,1,0xf0);          
    I2CWrite(0x98,6,0x11);          
    I2CWrite(0x98,7,0x11);          
    BIG_LOOP;                       
    BIG_LOOP;                       
  

		
    curA=I2CRead(0x98,0x0c);
		curB=I2CRead(0x98,0x0d);
		if((curA&0x40)==0x00)   /*MSB'S bit[6] is sign,if sign=0 */
		{
			curC= ((curA&0x3f)<<8 |curB);
			cur18= curC * 19.0735/10000;
		}
	else
		{
			
			curC= ((curA&0x3f)<<8 | curB);
			curC=(~curC)&0x3fff;
			cur18= (curC+1)*(-19.0735)/10000; 
	     }
		
		curA=I2CRead(0x98,0x10);
		curB=I2CRead(0x98,0x11);
		if((curA&0x40)==0x00)   /*MSB'S bit[6] is sign,if sign=0 */
		{
			curC= ((curA&0x3f)<<8 |curB);
			cur25= curC * 19.0735/500;
		}
	else
		{
			
			curC= ((curA&0x3f)<<8 | curB);
			curC=(~curC)&0x3fff;
			cur25= (curC+1)*(-19.0735)/500; 
	     }
		
		curA=I2CRead(0x98,0x14);
		curB=I2CRead(0x98,0x15);
		if((curA&0x40)==0x00)   /*MSB'S bit[6] is sign,if sign=0 */
		{
			curC= ((curA&0x3f)<<8 |curB);
			cur30= curC * 19.0735/10000;
		}
	else
		{
			
			curC= ((curA&0x3f)<<8 | curB);
			curC=(~curC)&0x3fff;
			cur30= (curC+1)*(-19.0735)/10000; 
	     }
		
		curA=I2CRead(0x98,0x18);
		curB=I2CRead(0x98,0x19);
		if((curA&0x40)==0x00)   /*MSB'S bit[6] is sign,if sign=0 */
		{
			curC= ((curA&0x3f)<<8 |curB);
			cur33= curC * 19.0735/10000;
		}
	else
		{
			
			curC= ((curA&0x3f)<<8 | curB);
			curC=(~curC)&0x3fff;
			cur33= (curC+1)*(-19.0735)/10000; 
	     }
		
printf("%d   %.2f	%.2f	%.2f	%.2f	%.2f	%.2f	%.2f	%.2f\n",i++,cur09,cur11,cur12,cur15,cur18,cur25,cur30,cur33);
	taskDelay(10);
	}
	return 0;
}
#endif

UINT32 I2cReadCur2(UINT8 addr)
{   
    I2cStart();
    I2C_REGISTER_WR( 0x0c ,0x98 );
    I2C_REGISTER_WR( 0x10 ,0x90 );
    I2C_LOOP

    I2C_REGISTER_WR( 0x0c ,addr );
    I2C_REGISTER_WR( 0x10 ,0x11 );
    I2C_LOOP
    
    I2C_REGISTER_WR( 0x0c ,0x99 );
    I2C_REGISTER_WR( 0x10 ,0x91 );    
    I2C_LOOP

    I2C_REGISTER_WR( 0x0c ,0x0 );
    I2C_REGISTER_WR( 0x10 ,0x68 );
    
    I2C_LOOP
    return ( I2C_REGISTER_RD(0x0c));
}

void I2cWriteCur2(UINT8 addr, UINT8 data)
{
    I2cStart();
    
    I2C_REGISTER_WR( 0x0c ,0x98 );
    I2C_REGISTER_WR( 0x10 ,0x90 );
    I2C_LOOP;

    I2C_REGISTER_WR( 0x0c ,addr );
    I2C_REGISTER_WR( 0x10 ,0x11 );
    I2C_LOOP;

    I2C_REGISTER_WR( 0x0c ,data );
    I2C_REGISTER_WR( 0x10 ,0x59 );
    I2C_LOOP;
}

UINT32 I2cReadCur1(UINT8 addr)
{   
    I2cStart();
    I2C_REGISTER_WR( 0x0c ,0x96 );
    I2C_REGISTER_WR( 0x10 ,0x90 );
    I2C_LOOP

    I2C_REGISTER_WR( 0x0c ,addr );
    I2C_REGISTER_WR( 0x10 ,0x11 );
    I2C_LOOP
    
    I2C_REGISTER_WR( 0x0c ,0x97 );
    I2C_REGISTER_WR( 0x10 ,0x91 );    
    I2C_LOOP

    I2C_REGISTER_WR( 0x0c ,0x0 );
    I2C_REGISTER_WR( 0x10 ,0x68 );
    
    I2C_LOOP
    return ( I2C_REGISTER_RD(0x0c));
}

void I2cWriteCur1(UINT8 addr, UINT8 data)
{
    I2cStart();
    
    I2C_REGISTER_WR( 0x0c ,0x96 );
    I2C_REGISTER_WR( 0x10 ,0x90 );
    I2C_LOOP;

    I2C_REGISTER_WR( 0x0c ,addr );
    I2C_REGISTER_WR( 0x10 ,0x11 );
    I2C_LOOP;

    I2C_REGISTER_WR( 0x0c ,data );
    I2C_REGISTER_WR( 0x10 ,0x59 );
    I2C_LOOP;
}

void readCurrent()
{
    //I2cRead(0xa);
}


void I2c_test()
{

    I2cInit();
    //I2cWrite(0x01, 0xf8);
    
    BIG_LOOP
    //I2cWrite(0x06, 0x38);

    BIG_LOOP
    I2cInit();

   // ulRead = I2cRead(0x01);
  //  printf("I2C read 0x01  :  %#x", ulRead);
}


extern unsigned int readHt0_0(unsigned int address);
extern void writeHt0_0(unsigned int address, unsigned int data);
extern int fioRead
    (
    int    fd,       /* file descriptor of file to read */
    char * buffer,   /* buffer to receive input */
    int    maxbytes  /* maximum number of bytes to read */
    );

#define  FPGA_MODE   0x0
#define  FPGA_STATUS 0x4
#define  FPGA_DATA   0x8
#define  FPGA_CONFIG 0xc

/* base address: 0x40000, 0x48000, 0x50000  filename: "host:*.rbf" */
int fpgaLoad(int baseAddr, char * fileName)
{
    int fd;
	int len = 0 ;
	size_t	bytesRead;
	int nbytes;
	
    unsigned int temp=0;

    unsigned int count = 1000000;
    unsigned char * buffer = (unsigned char *)0xa9000000;
    unsigned char *bufferTmp = buffer ;

    //memset(buffer , 0 , 1024*1024*100);

    if ((fd = open (fileName, O_RDONLY , 0)) == ERROR)
	{
		printf("fpgaLoad: Cannot open \"%s\".\n", fileName);
		return (ERROR);
	}

	if( ioctl (fd, FIONREAD, (int) &bytesRead) == ERROR)
	{
	    printf("fpgaLoad : ioctl bytesRead = %d\n", bytesRead);
	}
	len = 0;

	while ((nbytes = fioRead (fd, (char *)bufferTmp, 1024)) > 0)
	{
	    bufferTmp +=nbytes;
	    len += nbytes;
	} 

	printf("\r\nfpgaLoad :  read over,   bytes Read = %d\n", len);
	close(fd);
	
    
    /*  model sel */
    writeHt0_0(baseAddr | FPGA_MODE, 0x108);

    /* start config  */
    writeHt0_0(baseAddr | FPGA_CONFIG, 0x01);

    while(count)
    {
        count--;
        temp = readHt0_0(baseAddr|FPGA_STATUS);
        if( 0 == (temp & 0x110))
        {
            break;
        }
    }

    if( 0 == count)
    {
        printf("fpgaLoad: start config not ready %x \r\n" , temp);
        return -1;
    }

    /* close config  */
    writeHt0_0(baseAddr | FPGA_CONFIG, 0x0);

    printf("fpgaLoad: start write to fpga !\r\n" );
    
    count = 1000000;
    while(count)
    {
        count--;
        temp = readHt0_0(baseAddr|FPGA_STATUS);
        if(  (temp & 0x1) 
          && (temp & 0x10)
          &&!(temp & 0x100))
        {
            break;
        }
    }

    if( 0 == count)
    {
        printf("fpgaLoad: close config  not ready %x \r\n" , temp);
        return -1;
    }

    /* start write data to fpga */
    bufferTmp = buffer ;
    temp = len;
    while(temp >= 4)
    {
        writeHt0_0(baseAddr | FPGA_DATA, (*((unsigned int*)bufferTmp) ));
        //for( loop=10;loop>0;loop--);

        if(0 == (temp % 1000000))
        {
            //printf("bufferTmp = %x, *bufferTmp = %x. \r\n",bufferTmp , *bufferTmp);
            printf(".");
        }
        
        temp -=4;
        bufferTmp +=4;
    }

    if(3 == temp)
    {
        writeHt0_0(baseAddr | FPGA_DATA, (*((unsigned int*)bufferTmp) ) | 0xff000000);
    }
    else if(2 == temp)
    {
        writeHt0_0(baseAddr | FPGA_DATA, (*((unsigned int*)bufferTmp) ) | 0xffff0000);
    }
    else if(1 == temp)
    {
        writeHt0_0(baseAddr | FPGA_DATA, (*((unsigned int*)bufferTmp) ) | 0xffffff00);
    }

    printf("\r\n");

    /* config done? */
    count = 1000000;
    while(count)
    {
        count--;
        temp = readHt0_0(baseAddr|FPGA_STATUS);
        if(  temp & 0x100)
        {
            break;
        }
    }
    
    if( 0 == count)
    {
        printf("fpgaLoad:   config done not ready %x \r\n" , temp);
        return -1;
    }
    
    /* init done? */
    count = 1000000;
    while(count)
    {
        count--;
        temp = readHt0_0(baseAddr|FPGA_STATUS);
        if(  temp & 0x1000)
        {
            break;
        }

    }

	if( 0 == count)
	{
		printf("fpgaLoad:  init done  not ready %x \r\n" , temp);
		return -1;
	}

    writeHt0_0(baseAddr | FPGA_MODE , 0x0);
    return 0;
}

int sysSpendTimeGet(UINT32 startTimeH,UINT32 startTimeL,UINT32 endTimeH,UINT32 endTimeL)
{
	float spendTime;
	UINT32 clkFreq=100;         /*	us	*/
//	clkFreq = sysFpgaTimerClkFreqGet();
	//printf("startTimeH=0x%x,startTimeL=0x%x,endTimeH=0x%x,endTimeL=0x%x,clkFreq=%d\n",startTimeH,startTimeL,endTimeH,endTimeL,clkFreq);
	if(endTimeL<startTimeL) 
		spendTime = (endTimeL+0xffffffff-startTimeL) + (endTimeH-1-startTimeH)*4.0*1024*1024*1024;
	else  spendTime = (endTimeL-startTimeL) + (endTimeH-startTimeH)*4.0*1024*1024*1024;
	
	spendTime = spendTime/clkFreq;
	
	//printf("startTimeH=0x%x,startTimeL=0x%x,endTimeH=0x%x,endTimeL=0x%x,clkFreq=%d  %f\n",startTimeH,startTimeL,endTimeH,endTimeL,clkFreq,spendTime);
	return spendTime;
	
}

#define FPGA_COUNTER_L 0xbe0000a0		/*FPGA Counter for low,  1=10ns*/
#define FPGA_COUNTER_H 0xbe0000a4		/*FPGA Counter for high*/

int sysGetTime(UINT32 *timeH,UINT32 *timeL)
{


	*timeH=readHt0_0(0xa4);   //*(int *)FPGA_COUNTER_H;
	*timeL=readHt0_0(0xa0);   //*(int *)FPGA_COUNTER_L;
	return 0;
}



#define  DMA1_SRC_ADDR      0x3ff00700   
#define  DMA1_DST_ADDR      0x3ff00708   
#define  DMA1_SRC_ROW       0x3ff00710   
#define  DMA1_DST_COL       0x3ff00718   
#define  DMA1_SRC_LENGTH    0x3ff00720   
#define  DMA1_DST_LENGTH    0x3ff00728   
#define  DMA1_TRANS_CTRL    0x3ff00730  
#define  DMA1_TRANS_STATUS  0x3ff00738  

#define  DMA0_SRC_ADDR      0x3ff00600   
#define  DMA0_DST_ADDR      0x3ff00608  
#define  DMA0_SRC_ROW       0x3ff00610   
#define  DMA0_DST_COL       0x3ff00618   
#define  DMA0_SRC_LENGTH    0x3ff00620   
#define  DMA0_DST_LENGTH    0x3ff00628  
#define  DMA0_TRANS_CTRL    0x3ff00630   
#define  DMA0_TRANS_STATUS  0x3ff00638   

#define  DMA_ENABLE         1            
#define  DMA_DISABLE      0xfffffffc
#define  DMA_START          0x2           
#define  DMA_SRC_INT_VALID  0x4           
#define  DMA_DST_INT_VALID  0x8           
#define  DMA_ARCMD          0xf0          
#define  DMA_ARCHE          0xf00        
#define  DMA_ARUNCHE        0x0          
#define  DMA_AWCMD          0xb000        
#define  DMA_AWCHE          0xf0000       
#define  DMA_AWUNCHE        0x0          
#define  DMA_TRANS_1BITS    0x0          
#define  DMA_TRANS_2BITS    0x100000     
#define  DMA_TRANS_4BITS    0x200000     
#define  DMA_TRANS_8BITS    0x300000     
#define  DMA_TRANS_TURN     0x400000

#define  DMA_READ_FINISHED  1              
#define  DMA_TRANS_FINISHED 2             

__asm volatile void _asmWrite64Bits(unsigned int addrh, unsigned int addrl,unsigned int valh,unsigned int vall)
{
% reg addrh; reg addrl; reg valh; reg vall;                       
	.set noreorder 
	move t0,addrh
	dsll32 t0,t0,0
    or  t0,t0,addrl
	dsll32 t1,valh,0
	or t1,t1,vall
	sd  t1,0(t0)
	sync
	.set reorder
}

__asm volatile void _asmRead64Bits(unsigned int addrh,unsigned int addrl,unsigned int * valh,unsigned int * vall)
{
% reg addrh; reg addrl; reg valh; reg vall;
	.set noreorder 
	move t0,addrh
	dsll32 t0,t0,0
	or  t0,t0,addrl
	ld  t1,0(t0)
	sw  t1,0(vall)
	dsrl32 t1,t1,0
	sw  t1,0(valh)
	sync
	.set reorder
}

__asm volatile void _asmWrite32Bits(unsigned int addrh, unsigned int addrl,unsigned int val)
{
% reg addrh; reg addrl; reg val;                       
	.set noreorder 
	move t0,addrh
	dsll32 t0,t0,0
    or  t0,t0,addrl
    move t1,val;
	sw  t1,0(t0)
	sync
	.set reorder
}

__asm volatile unsigned int _asmRead32Bits(unsigned int addrh,unsigned int addrl)
{
% reg addrh; reg addrl;
! "$2"
	.set noreorder
	move t0,addrh
	dsll32 t0,t0,0
	or  t0,t0,addrl
	lw $2,0(t0);
	.set reorder	
}

void sysMatrixDma0Trans(unsigned int raddrh,unsigned int raddrl, 
						unsigned int waddrh,unsigned int waddrl, 
						unsigned int row,unsigned int col,unsigned int size,int turn)
{
	int bRCache;
	int bWCache;
	unsigned int mode;
	int shift;
	
	if(raddrl == 0) return;
	if(waddrl == 0) return;
	
	if(row == 0) return;
	if(col == 0) return;
	
	if((size != 1)&&(size != 2)&&(size != 4)&&(size !=8)) return;
	
	bRCache = ((raddrh & 0x98000000) == 0x98000000);
	bWCache = ((waddrh & 0x98000000) == 0x98000000);
	
	mode =0;
	
	if(bRCache) mode |= DMA_ARCMD|DMA_ARCHE;  /*cacheable address*/
	if(bWCache) mode |= DMA_AWCMD|DMA_AWCHE;  /*cacheable address*/
	
	mode |= DMA_DST_INT_VALID;
	
	switch(size)
	{
		case 1:
			mode |= DMA_TRANS_1BITS;
			shift = 0;
			break;
			
		case 2:
			mode |= DMA_TRANS_2BITS;
			shift = 1;
			break;
			
		case 3:
			mode |= DMA_TRANS_4BITS;
			shift = 2;
			break;
			
		default:
			mode |= DMA_TRANS_8BITS;
			shift = 3;
			break;
	}
	
	_asmWrite64Bits(0x90000000,DMA0_SRC_ADDR,raddrh,raddrl);
	_asmWrite64Bits(0x90000000,DMA0_DST_ADDR,waddrh,waddrl);
	
	if(turn)
	{
		_asmWrite32Bits(0x90000000,DMA0_SRC_ROW , row);
		_asmWrite32Bits(0x90000000,DMA0_DST_COL , col);
		_asmWrite32Bits(0x90000000,DMA0_SRC_LENGTH ,(row<<shift));
		_asmWrite32Bits(0x90000000,DMA0_DST_LENGTH, (col<<shift));
		
		mode |= DMA_TRANS_TURN;
	}
	else
	{
		_asmWrite32Bits(0x90000000,DMA0_SRC_ROW , row<<shift);
		_asmWrite32Bits(0x90000000,DMA0_DST_COL , col);
		_asmWrite32Bits(0x90000000,DMA0_SRC_LENGTH ,(row<<shift));
		_asmWrite32Bits(0x90000000,DMA0_DST_LENGTH, (row<<shift));
		
		mode &= (~DMA_TRANS_8BITS);/*alwayse 0*/
	} 
	
	_asmWrite32Bits(0x90000000,DMA0_TRANS_CTRL,DMA_ENABLE|DMA_START|mode); /*start dma*/
	 
}

unsigned long long testFlashT1,testFlashT2 , countFlash;
extern unsigned long long getcount( );
double testFlashTime;
void testFlash()
{
    UINT8  wTemp1, wTemp2, j;  
    int i;

#if 1
    printf("start FLASH_REGISTER_RD(0x00)  \r\n" );
    FLASH_REGISTER_WR(0x00, 0xf0);/* RESET ????*/  

    FLASH_REGISTER_WR(0xaaa,0xaa);  
    FLASH_REGISTER_WR(0x555,0x55);  
    FLASH_REGISTER_WR(0xaaa,0x90);  
    //FLASH_REGISTER_WR(0x00, 0x01);  
    printf("FLASH_REGISTER_RD(0x00) = 0x%x \r\n", FLASH_REGISTER_RD(0x00));  

    printf("start FLASH_REGISTER_RD(0x01)  \r\n" );
    FLASH_REGISTER_WR(0x00, 0xf0);/* RESET ????*/  
    
    FLASH_REGISTER_WR(0xaaa,0xaa);  
    FLASH_REGISTER_WR(0x555,0x55);  
    FLASH_REGISTER_WR(0xaaa,0x90);  
    //FLASH_REGISTER_WR(0x001,0x227e);
    printf("FLASH_REGISTER_RD(0x01) = 0x%x \r\n", FLASH_REGISTER_RD(0x02));  

    FLASH_REGISTER_WR(0x00, 0xf0);/* RESET ?????*/  


    FLASH_REGISTER_WR(0xaaa,0xaa);  
    FLASH_REGISTER_WR(0x555,0x55);  
    FLASH_REGISTER_WR(0xaaa,0xa0);  
    FLASH_REGISTER_WR(0x0, 0x12);

    while (1) 
    {
        wTemp1 = FLASH_REGISTER_RD(0);
        wTemp2 = FLASH_REGISTER_RD(1);
        printf("1111 wTemp1 = %x   wTemp2 = %x\n", wTemp1, wTemp2);
        if ((wTemp1 ^ wTemp2) & 0x40)  /* toggle */ 
        {   
            printf("2222\n");
            if (wTemp2 & 0x20)  /* DQ5 = 1 */
            {     
                printf("333\n");
                /* read twice */
                wTemp1 = FLASH_REGISTER_RD(0);
                wTemp2 = FLASH_REGISTER_RD(1);
                if ((wTemp1 ^ wTemp2) & 0x40)  /* toggle */
                { 
                    break; /* program not completed, need reset*/
                }
                else 
                {
                    break;   /* program completed */
                }
            }
            else 
            {
                printf("4444\n");
                wTemp1 = wTemp2;
                continue;
            }
        }
        else 
        { 
            break;   /* program completed */
        }
    }
#endif
#if 1

    FLASH_REGISTER_WR(0x00, 0xf0);/* RESET ????*/ 
      
    FLASH_REGISTER_WR(0xaaa,0xaa);  
    FLASH_REGISTER_WR(0x555,0x55);  
    FLASH_REGISTER_WR(0xaaa,0x80);  
    FLASH_REGISTER_WR(0xaaa,0xaa);  
    FLASH_REGISTER_WR(0x555,0x55);  
    FLASH_REGISTER_WR(0,0x30);  
      

    while (1) 
    {
        wTemp1 = FLASH_REGISTER_RD(0);
        wTemp2 = FLASH_REGISTER_RD(1);
        if ((wTemp1 ^ wTemp2) & 0x40)  /* toggle */ 
        {   
            if (wTemp2 & 0x20)  /* DQ5 = 1 */
            {     
                /* read twice */
                wTemp1 = FLASH_REGISTER_RD(0);
                wTemp2 = FLASH_REGISTER_RD(1);
                if ((wTemp1 ^ wTemp2) & 0x40)  /* toggle */
                { 
                    break; /* program not completed, need reset*/
                }
                else 
                {
                    break;   /* program completed */
                }
            }
            else 
            {
                wTemp1 = wTemp2;
                continue;
            }
        }
        else 
        { 
            break;   /* program completed */
        }
    }
	
      
    FLASH_REGISTER_WR(0x00, 0xf0);/* RESET ????*/ 
    FLASH_REGISTER_WR(0xaaa,0xaa);  
    FLASH_REGISTER_WR(0x555,0x55);  
    FLASH_REGISTER_WR(0xaaa,0x20);
    testFlashT1 = getcount();
    for ( i = 0; i < 0x100000; i++ )  
    {
        FLASH_REGISTER_WR(0x00, 0xf0);/* RESET ????*/
        #if 0
        FLASH_REGISTER_WR(0xaaa,0xaa);  
        FLASH_REGISTER_WR(0x555,0x55);  
        FLASH_REGISTER_WR(0xaaa,0xa0);  
        #else
        FLASH_REGISTER_WR(0xaaa,0xa0);  
        #endif
        j= 255-(UINT8)i;
        //printf("j=%d\n",j);
        FLASH_REGISTER_WR(i, j);

        while (1) 
        {
            wTemp1 = FLASH_REGISTER_RD(i);
            //printf("1111 wTemp1 = %x   wTemp2 = %x\n", wTemp1, wTemp2);
            if ((wTemp1 ^ j) & 0x80)  /* toggle */ 
            {   
               // printf("2222\n");
                if (wTemp1 & 0x20)  /* DQ5 = 1 */
                {     
                   // printf("333\n");
                    /* read twice */
                    wTemp1 = FLASH_REGISTER_RD(i);
                    
                    if ((wTemp1 ^ j) & 0x80)  /* toggle */
                    { 
                        break; /* program not completed, need reset*/
                    }
                    else 
                    {
                        break;   /* program completed */
                    }
                }
                else 
                {
                   // printf("4444\n");
                    
                    continue;
                }
            }
            else 
            { 
                break;   /* program completed */
            }
        }
        
    
    }
    FLASH_REGISTER_WR(0x00, 0xf0);/* RESET ????*/ 
    FLASH_REGISTER_WR(0xaaa,0x90);  
    FLASH_REGISTER_WR(0xaaa,0x00);
    testFlashT2 = getcount();

    if(testFlashT2 > testFlashT1)
	{
    	countFlash = testFlashT2-testFlashT1;
    	testFlashTime = countFlash *2.00000/sb1_clock_rate;
    	printf("transmission time is %f s\r\n",testFlashTime);
	}
	else
		printf("cal transmission time error!\r\n");
#endif

}

#define FLASH_REGISTER_WRX16(addr,data)      *((volatile UINT16 *)0xb0000000+addr)=(UINT16)data   
#define FLASH_REGISTER_RDX16(addr)         (*((volatile UINT16 *)0xb0000000+addr))   

void eraseFlashx16()
{   
    UINT8  wTemp1, wTemp2, j;  
    int i;
    FLASH_REGISTER_WRX16(0x00, 0xf0);/* RESET ????*/  

    FLASH_REGISTER_WRX16(0x555,0xaa);  
    FLASH_REGISTER_WRX16(0x2aa,0x55);  
    FLASH_REGISTER_WRX16(0x555,0x80);  
    FLASH_REGISTER_WRX16(0x555,0xaa);  
    FLASH_REGISTER_WRX16(0x2aa,0x55);  
    FLASH_REGISTER_WRX16(0x555, 0x10);

    while (1) 
    {
        wTemp1 = FLASH_REGISTER_RDX16(0);
        wTemp2 = FLASH_REGISTER_RDX16(1);
        printf("1111 wTemp1 = %x   wTemp2 = %x\n", wTemp1, wTemp2);
        if ((wTemp1 ^ wTemp2) & 0x40)  /* toggle */ 
        {   
            printf("2222\n");
            if (wTemp2 & 0x20)  /* DQ5 = 1 */
            {     
                printf("333\n");
                /* read twice */
                wTemp1 = FLASH_REGISTER_RDX16(0);
                wTemp2 = FLASH_REGISTER_RDX16(1);
                if ((wTemp1 ^ wTemp2) & 0x40)  /* toggle */
                { 
                    break; /* program not completed, need reset*/
                }
                else 
                {
                    break;   /* program completed */
                }
            }
            else 
            {
                printf("4444\n");
                wTemp1 = wTemp2;
                continue;
            }
        }
        else 
        { 
            break;   /* program completed */
        }
    }
}

void eraseFlashTest(UINT32 sector)
{ 
    UINT8  wTemp1, wTemp2, j;  
    int i;
    FLASH_REGISTER_WRX16(0x00, 0xf0);/* RESET ????*/ 
    FLASH_REGISTER_WRX16(0x555,0xaa);  
    FLASH_REGISTER_WRX16(0x2aa,0x55);  
    FLASH_REGISTER_WRX16(0x555,0x20);

    FLASH_REGISTER_WRX16(0x2aa,0x80);  
    FLASH_REGISTER_WRX16((sector*0x20000),0x30);

    while (1) 
    {
        wTemp1 = FLASH_REGISTER_RDX16(0);
        wTemp2 = FLASH_REGISTER_RDX16(1);
        //printf("1111 wTemp1 = %x   wTemp2 = %x\n", wTemp1, wTemp2);
        if ((wTemp1 ^ wTemp2) & 0x40)  /* toggle */ 
        {   
           // printf("2222\n");
            if (wTemp2 & 0x20)  /* DQ5 = 1 */
            {     
               // printf("333\n");
                /* read twice */
                wTemp1 = FLASH_REGISTER_RDX16(0);
                wTemp2 = FLASH_REGISTER_RDX16(1);
                if ((wTemp1 ^ wTemp2) & 0x40)  /* toggle */
                { 
                    break; /* program not completed, need reset*/
                }
                else 
                {
                    break;   /* program completed */
                }
            }
            else 
            {
               // printf("4444\n");
                wTemp1 = wTemp2;
                continue;
            }
        }
        else 
        { 
            break;   /* program completed */
        }
    }


    FLASH_REGISTER_WRX16(0x00, 0xf0);/* RESET ????*/ 
    FLASH_REGISTER_WRX16(0xaaa,0x90);  
    FLASH_REGISTER_WRX16(0xaaa,0x00);
}


void testFlashx16()
{
    UINT8  wTemp1, wTemp2, j;  
    int i;

#if 1
	#if 0

    printf("start FLASH_REGISTER_RD(0x00)  \r\n" );
    FLASH_REGISTER_WRX16(0x00, 0xf0);/* RESET ????*/  

    FLASH_REGISTER_WRX16(0x555,0xaa);  
    FLASH_REGISTER_WRX16(0x2aa,0x55);  
    FLASH_REGISTER_WRX16(0x555,0x90);  
    //FLASH_REGISTER_WR(0x00, 0x01);  
    printf("FLASH_REGISTER_RD(0x00) = 0x%x \r\n", FLASH_REGISTER_RDX16(0x00));  

    printf("start FLASH_REGISTER_RD(0x01)  \r\n" );
    FLASH_REGISTER_WRX16(0x00, 0xf0);/* RESET ????*/  
    
    FLASH_REGISTER_WRX16(0x555,0xaa);  
    FLASH_REGISTER_WRX16(0x2aa,0x55);  
    FLASH_REGISTER_WRX16(0x555,0x90);  
    //FLASH_REGISTER_WR(0x001,0x227e);
    printf("FLASH_REGISTER_RD(0x01) = 0x%x \r\n", FLASH_REGISTER_RDX16(0x01));  

    FLASH_REGISTER_WRX16(0x00, 0xf0);/* RESET ?????*/  


    FLASH_REGISTER_WRX16(0x555,0xaa);  
    FLASH_REGISTER_WRX16(0x2aa,0x55);  
    FLASH_REGISTER_WRX16(0x555,0xA0);  
    FLASH_REGISTER_WRX16(0x0, 0xAA55);

    while (1) 
    {
        wTemp1 = FLASH_REGISTER_RDX16(0);
        wTemp2 = FLASH_REGISTER_RDX16(1);
        printf("1111 wTemp1 = %x   wTemp2 = %x\n", wTemp1, wTemp2);
        if ((wTemp1 ^ wTemp2) & 0x40)  /* toggle */ 
        {   
            printf("2222\n");
            if (wTemp2 & 0x20)  /* DQ5 = 1 */
            {     
                printf("333\n");
                /* read twice */
                wTemp1 = FLASH_REGISTER_RDX16(0);
                wTemp2 = FLASH_REGISTER_RDX16(1);
                if ((wTemp1 ^ wTemp2) & 0x40)  /* toggle */
                { 
                    break; /* program not completed, need reset*/
                }
                else 
                {
                    break;   /* program completed */
                }
            }
            else 
            {
                printf("4444\n");
                wTemp1 = wTemp2;
                continue;
            }
        }
        else 
        { 
            break;   /* program completed */
        }
    }
    if( FLASH_REGISTER_RDX16(0x0)!=0xaa55)
    printf("write error %#x",FLASH_REGISTER_RDX16(0x0));

   #endif
      
    FLASH_REGISTER_WRX16(0x00, 0xf0);/* RESET ????*/ 
    FLASH_REGISTER_WRX16(0x555,0xaa);  
    FLASH_REGISTER_WRX16(0x2aa,0x55);  
    FLASH_REGISTER_WRX16(0x555,0x20);
    testFlashT1 = getcount();
    for ( i = 0; i < 0x100000; i++ )  
    {
        FLASH_REGISTER_WRX16(0x00, 0xf0);/* RESET ????*/
        #if 0
        FLASH_REGISTER_WR(0xaaa,0xaa);  
        FLASH_REGISTER_WR(0x555,0x55);  
        FLASH_REGISTER_WR(0xaaa,0xa0);  
        #else
        FLASH_REGISTER_WRX16(0xaaa,0xa0);  
        #endif
        j= (UINT8)i;
        //printf("j=%d\n",j);
        FLASH_REGISTER_WRX16(i, j);

        while (1) 
        {
            wTemp1 = FLASH_REGISTER_RDX16(0);
            wTemp2 = FLASH_REGISTER_RDX16(1);
            //printf("1111 wTemp1 = %x   wTemp2 = %x\n", wTemp1, wTemp2);
            if ((wTemp1 ^ wTemp2) & 0x40)  /* toggle */ 
            {   
               // printf("2222\n");
                if (wTemp2 & 0x20)  /* DQ5 = 1 */
                {     
                   // printf("333\n");
                    /* read twice */
                    wTemp1 = FLASH_REGISTER_RDX16(0);
                    wTemp2 = FLASH_REGISTER_RDX16(1);
                    if ((wTemp1 ^ wTemp2) & 0x40)  /* toggle */
                    { 
                        break; /* program not completed, need reset*/
                    }
                    else 
                    {
                        break;   /* program completed */
                    }
                }
                else 
                {
                   // printf("4444\n");
                    wTemp1 = wTemp2;
                    continue;
                }
            }
            else 
            { 
                break;   /* program completed */
            }
        }

    
    }
    FLASH_REGISTER_WRX16(0x00, 0xf0);/* RESET ????*/ 
    FLASH_REGISTER_WRX16(0xaaa,0x90);  
    FLASH_REGISTER_WRX16(0xaaa,0x00);
    testFlashT2 = getcount();

    if(testFlashT2 > testFlashT1)
	{
    	countFlash = testFlashT2-testFlashT1;
    	testFlashTime = countFlash *2.00000/sb1_clock_rate;
    	printf("transmission time is %f s\r\n",testFlashTime);
	}
	else
		printf("cal transmission time error!\r\n");

    
    
#endif

}

#define  HT_REG_BASE   0x90000cfdfb000000
#define  HT_BUS_BASE   0x90000efdfe000000

#define  PHY_HT_REG_BASE(node, i)    (HT_REG_BASE + node * 0x100000000000 + i * 0x10000000000)
#define  PHY_HT_BUS_BASE(node )			 (HT_BUS_BASE + node * 0x100000000000)


__asm volatile void writeHt0_fdfb(unsigned int address, unsigned int data)
{
% reg address; reg data;                     
	.set noreorder 
	dli t0,0x90000cFDFB000000
	or  t0,t0,address
	sw  data,0(t0)
	/*sync*/
	.set reorder
}


void writefdfb(unsigned int address,unsigned int value)
{
    //printf("\r\naddrh = %#x, addrh = %#x, value= %#x ",addrh, addrl, value);
	writeHt0_fdfb(address,value); 
}

#define  ht_set_reg( reg, val)     writefdfb(  reg, val)
void ht_set_rx_win( unsigned long v1, unsigned long v2)
{
		//0x80000000
		ht_set_reg( 0x60, v1);
		//0x0000fff8   //32M
		ht_set_reg( 0x64, v2);
	
}

