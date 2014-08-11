/* mmuMipsLib.c - Memory Management Unit Library for MIPS R4k */

/*
 * Copyright (c) 1999-2001, 2003-2009 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

#define	ENABLE_DYNAMIC_SIZE  
#undef  ENABLE_DYNAMIC_SIZE  /* mxl del */
/*
modification history
--------------------
04b,07oct09,rlg  fix for WIND00182273 - problems with vmPageOtimize test
04a,03apr09,pch  enable dynamic TLB sizing
03z,31mar09,slk  add (disabled) support for dynamic TLB sizing
03y,26feb09,pgh  Remove _WRS_MIPS_SDA_SUPPORTED #if
03x,15feb09,slk  added initialization of globals moved to -1 page for SMP
03w,03sep08,jpb  Renamed VSB config file.
03v,27aug08,slk  enable SDA support functions
03u,18jun08,jpb  Renamed _WRS_VX_SMP to _WRS_CONFIG_SMP.  Added include path
		 for kernel configurations options set in vsb.
03t,09nov07,pes  Disable page locking for SMP builds
03s,12jul07,dcc  added SDA support in the kernel, although not yet enabled.
03r,01aug07,pes  Eliminate boot-time warnings about undefined symbol for
		 mmuMipsEnableHandler
03q,21jun07,pes  Rename .kseg0data to .kseg0bss
03p,16feb07,pgh  Fix WIND00087298
03o,09feb07,pes  Modify mmuMipsInitialMemoryMap to not wire data section into
		 TLB.
03n,01feb07,pes  Change -1 page references to refer to .kseg0data-resident
		 values. Remove debug code. Code review cleanups. Update to
		 new HLD APIs.
03m,10may06,pes  Eliminate tests against mmuUncacheable when initializing
		 mmuStateCacheTransArrayLocal.
03l,19jan06,pes  Change reference to _mipsTlbSizeGet to mipsTlbSizeGet.
03k,29sep05,rlg  SPR 113079 - remove comiler warnings
03j,11jul05,rlg  SPR 104080 fix for mapped kernel
03i,23mar05,mig  added extra parameter (pointer to region table entry) to
		 mmuMipsPteSet and mmuMipsPteStateGet.
03h,23feb05,d_c  SPR 103891, 104353: Consider virtual address when calculating
		 user mode protection states. SPR 104399: Remove VALID
		 attribute from protection translation table and protection
		 mask to allow vmStateSet to set protection while clearing
		 VALID. Corrected copyright.
03g,09dec04,pes  Added initialization of the 'isr' field in mmuMipsPteInit.
03f,01nov04,slk  Set OSM handler data in exception scratch page to NULL.
03e,05oct04,pes  Removed #if 0 code.
03d,01oct04,agf  Replaced calls to sysWired[G,S]et with _mipsWired[G,S]et
03c,29sep04,pes  Removed use of mmuMipsEnabled. It is no longer needed.
03b,29sep04,sru  Minor whitespace cleanup.
03a,27sep04,sru  Changed alias of sysTextProtect from KSEG1 to KSEG0.
02z,22sep04,pes  Add mmuMaxPhysBitsGet function and initialize its pointer
		 in the mmuArchLibFuncs array.
02y,21sep04,sru  Removed unused 'regionCnt' variable.
02x,17sep04,sru  Update mmuMipsInitialMemoryMap to set the 'writable'
		 attribute correctly for the text segment TLBs, and
		 'unwire' the data segment TLB mappings.
02w,15sep04,???  remove debug code for release
02v,31aug04,dtr  Pass number of locked entries available up to AIM.
02u,19aug04,pes  Add support for TLB locking.
02t,09jun04,dtr  Add in CurrentGet func.
02s,01jun04,dtr  Modify PageWrite
02r,02jun04,mem  get init parameters from palDesc instead of arguments
02q,26may04,agf  change exc vector references to use the palDesc struct
02p,17may04,mem  cleanups, remove spurious KM_TO_K0 translations
02o,04may04,sru  add support for query of TLB size
02n,24mar04,slk  Cleanup mmuMipsInstallTempTlbHandler
02m,03mar04,pes  Merge virtualSegmentInfo entries. Added mmuMipsTransTblGet().
02l,12jan04,pes  Add mmuMipsVirtSegInfoGet support.
02k,11dec03,jmt  Incorporate more Code Review changes
02j,08dec03,jmt  Incorporate Code Review changes
02i,02dec03,jmt  Continued Testing of MIPS AIM AD-MMU code
02h,17nov03,jmt  Continued Development of MIPS AIM AD-MMU code
02g,30oct03,jmt  rewrite as architecture-dependent portion of AIM MMU library
02f,26sep03,jmt  Continued MMU Testing
02e,18sep03,jmt  Modify TLB Handling for Base 6
02d,10sep03,jmt  Merge code from AE to Base 6
02c,18apr01,mem  SPR #65789: change MMU_REGION_SIZE back to 4M in all cases.
02b,28feb01,tlc  Add palification of page size.
02a,29nov00,mem  Work around SL test failure.
01z,29nov00,mem  Fix minor problem with last checkin.
01y,29nov00,mem  added VM_CTX_MASK_L2_FREE functionality (SPR #62572)
01x,07nov00,mem  Fix compiler warning.
01w,19sep00,dra  Updated mmuMipsGetNewContext.
01v,08sep00,dra  Fixed default values for PTE.
01u,30aug00,dra  Fix ASID assignment algorithum.
01t,21jun00,dra  MMU support routines now translated during setup.
		 Move context switch funcs to windALib.s
01s,14jun00,dra  generalize the mmuMipsUnMap flush of virtual page address to
		 all Mips archs
01r,19apr00,dra  Write protect page and region tables, clear P-cache lines
		 on page unmap, implement phys->virt address translation
01q,12apr00,dra  Fix compilation warnings.  Add support for XTLB exception vector.
01p,06apr00,dra  Clear TLB after removing virtual-to-physical mappings.
01o,06apr00,gls  added NULL for mmuTransTblCopy entry in mmuLibFuncsLocal
01n,30mar00,dra  Remove semTake/semGive from mmuMipsBufferWrite.
01m,24mar00,dra  Change default state of newly mapped page to CACHE_OFF,
		 VALID, and DIRTY (SUP/USR RWX).  Fix minor bug in vmBufferWrite
		 implementation.  Call cacheTextUpdate() before deleting
		 translation tables.  Return error if null PTE found in mmuMipsetPte.
01l,09mar00,dra  Don't xlate TransTbls from kseg2 to kseg0.  Translate page
		 tables to kseg1 instead of kseg0.  Additional cache clearing.
		 Fix some bugs in mmuMipsTransTblMask.
01k,25jan00,dra  Use mmuMipsConfig.ptrContextSetRtn instead of
		 mmuMipsConfig.ptrWriteEntryHiRtn.  Clean up old MMU_STATE use
		 and replace with proper generic pointers.
01j,22dec99,dra  mmuMipsTransTableCreate now allocates memory before use
01i,10dec99,dra  Changed parameter in mmuMipsEnableH from 'asid' to 'tlbVector'
		 so to specify location of TLB Exception Vector for any
		 MIPS device.  'asid' is assumed to zero; only the Kernel
		 can enable the mmu.
01h,22nov99,dra  Added specialized implementation for mmuMipsBufferWrite
		 for the case that the physical address is in the lower
		 512M of memory.
01g,19nov99,dra  Added macros for references to device-specific
		 mips functions: MMU_MIPS_EXTRACT_PHYS_ADDR,
		 MMU_MIPS_INVALIDATE, MMU_MIPS_CLEAR_TLB, MMU_MIPS_CREATE_PTE
01f,17nov99,dra  Added mmuMipsBufferWrite();
		 Added mask-out capability to mmuMipsTransTblMask();
		 Removed references to mmuKernelTransTbl and mmuPageBlockSize.
		 Added mmuMipsPageUnProtect();
		 Added mmuMipsPageSizeGet();
01e,16nov99,dra  Added standardized call to mmuMipsClearTlb();.
01f,25oct99,dra  Removed protection, cache, and mask table declarations.
		 These tables are now declared in the device-dependent
		 mmu libraries and accessed using pointers in mmuMipsConfig.
		 +
		 Removed calls from mmuMipsALib.s.  These functions are
		 now accessed using the mmuMipsConfig pointers.
		 +
		 Added check for mmuMipsConfig.initFlag in mmuMipsLibInit.
01e,22oct99,dra  Removed device-dependent C functions mmuMipsCreatePte
		 and mmuMipsExtractPhysAddr.  These functions are called
		 using pointers in the interface data structure
		 mmuMipsConfig.
01d,04oct99,dra  Added PHYS_TO_VIRT translation for mmuMipsTranslate.
01c,15sep99,dra  Updated mmuStateCacheTransArrayLocal[] for MIPS.
		 Updated mmuStateProtTransArrayLocal[] for MIPS.
01b,14sep99,dra  Updated mmuLibFuncsLocal for vmLib.c.  Updated global
		 initializations for vmLib.c.
01a,29jul99,dra  Created.
*/


/*
DESCRIPTION:

The MIPS family of devices range between many different manufacturers
resulting in a wide range of implementations of memory management units.
This library contains the functions that support the generic R4k version
of these devices.  It provides routines that are called by the architecture
independent manager (AIM). There are two layers of architecture independent
routines: the lower of these is the Architecture-Independant MMU which
provides page-table management, and the upper is vmLib.c or vmBaseLib.c.

The MIPS mmu library is based on a page-table architecture created to handle
TLB exceptions.  This page-table architecture is a three level heirarchy.
Level-0, the Context Table, is comprised of 256 4-byte pointers to the next
level of page tables.  The Level-1 page table, the Region Table, is pointed
to by Level-0 and is comprised of 1024 4-byte pointers to the next level of
page tables.  It is indexed by the top 10 bits of the BadVaddr register.
Level-2, the Page Table, pointed to by Level-1, contains the page-table
entries used to fill the TLB.  It is indexed by the second 10 bits of the
BadVAddr register.

The sizes of the three-level page-table architecture restrict some of the
characteristics of the system.  The size of the region table--limited to the
number of available contexts given by the Address Space ID (if available)
field of the TLB--restricts the system to only 256 separate execution
contexts.  The size of the L1 and L2 page tables restricts the system to
a minimum page size of 4KB and a maximum page size of 4MB.
*/

/* includes */

#include <vxWorks.h>
#include <vsbConfig.h>
#include <errno.h>
#include <cacheLib.h>
#include <vmLib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <intLib.h>
#include <taskLib.h>
#include <memPartLib.h>
#include <bmpLib.h>
#include <private/vmLibP.h>
#include <aimMmuLib.h>
#include <arch/mips/mmuMipsLib.h>
#include <arch/mips/palMipsLib.h>
#ifdef _WRS_CONFIG_SMP
#include <vxCpuLib.h>
#endif /* _WRS_CONFIG_SMP */

#define K0_TO_KM(x)	PHYS_TO_KM(K0_TO_PHYS(x))

/*
 * The form follows the attributes in mmuAttr.h.  These two tables must be
 * constructed as to hold the proper architecture dependent state given
 * the archictecture independent state.
 *
 * The cache translation table is shifted by 4 bits.
 *
 * The VM_STATE_SET function uses this table to translate requested
 * arch-independent state protection bits to arch-dependent state
 * protection bits. The arch-independent bits form an index into the
 * table. The contents of the table represent a MIPS-dependent version
 * of what the calling program is requesting. This not necessarily
 * what will actually get implemented.  Read permission always implies
 * execute permission. Execute without read can not be implemented.
 * The MMU_STATE_USER_ACCESS bit will be ignored. Instead, the virtual
 * address will be used to determine user access. However, the
 * MMU_STATE_USER_ACCESS bit has a role when this table is used by
 * VM_STATE_GET.
 *
 * The VM_STATE_GET function uses this table to translate arch-dependent
 * protection bits to arch-independent protection bits. When the
 * arch-specific state word is retrieved (by mmuMipsPteStateGet),
 * the MMU_STATE_USER_ACCESS bit will be set iff the virtual address
 * corresponding to the PTE entry is in kuseg. (This information was saved
 * in the PTE by mmuMipsPteSet). VM_STATE_GET will OR together the arch
 * independent bits for all entries in the table matching the retrieved
 * arch-dependent protection bits. This will produce arch-independent
 * protection bits that accurately represent the actual protection
 * implemented by hardware.
 */

    /* Arch-dependent */			/* Arch-independent */

LOCAL const UINT mmuStateProtTransArrayLocal [] =
    {
     (MMU_STATE_USER_ACCESS_NOT
      | MMU_R4K_STATE_WRITABLE_NOT),		/*	SUP R--		*/
     (MMU_STATE_USER_ACCESS_NOT
      | MMU_R4K_STATE_WRITABLE),		/*	SUP RW-		*/
     (MMU_STATE_USER_ACCESS_NOT
      | MMU_R4K_STATE_WRITABLE_NOT),		/*	SUP R-X		*/
     (MMU_STATE_USER_ACCESS_NOT
      | MMU_R4K_STATE_WRITABLE),		/*	SUP RWX		*/
     (MMU_STATE_USER_ACCESS
      | MMU_R4K_STATE_WRITABLE_NOT),		/*	SUP R--,USR R-- */
     (MMU_STATE_USER_ACCESS
      | MMU_R4K_STATE_WRITABLE),		/*	SUP RW-,USR R-- */
     (MMU_STATE_USER_ACCESS
      | MMU_R4K_STATE_WRITABLE_NOT),		/*	SUP R-X,USR R-- */
     (MMU_STATE_USER_ACCESS
      | MMU_R4K_STATE_WRITABLE),		/*	SUP RWX,USR R-- */
     MMU_R4K_STATE_INVALID_STATE,		/* INV: SUP R--,USR -W- */
     MMU_R4K_STATE_INVALID_STATE,		/* INV: SUP RW-,USR -W- */
     MMU_R4K_STATE_INVALID_STATE,		/* INV: SUP R-X,USR -W- */
     MMU_R4K_STATE_INVALID_STATE,		/* INV: SUP RWX,USR -W- */
     MMU_R4K_STATE_INVALID_STATE,		/* INV: SUP R--,USR RW- */
     (MMU_STATE_USER_ACCESS
      | MMU_R4K_STATE_WRITABLE),		/*	SUP RW-,USR RW- */
     MMU_R4K_STATE_INVALID_STATE,		/* INV: SUP R-X,USR RW- */
     (MMU_STATE_USER_ACCESS
      | MMU_R4K_STATE_WRITABLE),		/*	SUP RWX,USR RW- */
     MMU_R4K_STATE_INVALID_STATE,		/* INV: SUP R--,USR --X */
     MMU_R4K_STATE_INVALID_STATE,		/* INV: SUP RW-,USR --X */
     MMU_R4K_STATE_INVALID_STATE,		/* INV: SUP R-X,USR --X */
     MMU_R4K_STATE_INVALID_STATE,		/* INV: SUP RWX,USR --X */
     MMU_R4K_STATE_INVALID_STATE,		/* INV: SUP R--,USR R-X */
     MMU_R4K_STATE_INVALID_STATE,		/* INV: SUP RW-,USR R-X */
     (MMU_STATE_USER_ACCESS
      | MMU_R4K_STATE_WRITABLE_NOT),		/*	SUP R-X,USR R-X */
     (MMU_STATE_USER_ACCESS
      | MMU_R4K_STATE_WRITABLE),		/*	SUP RWX,USR R-X */
     MMU_R4K_STATE_INVALID_STATE,		/* INV: SUP R--,USR -WX */
     MMU_R4K_STATE_INVALID_STATE,		/* INV: SUP RW-,USR -WX */
     MMU_R4K_STATE_INVALID_STATE,		/* INV: SUP R-X,USR -WX */
     MMU_R4K_STATE_INVALID_STATE,		/* INV: SUP RWX,USR -WX */
     MMU_R4K_STATE_INVALID_STATE,		/* INV: SUP R--,USR RWX */
     MMU_R4K_STATE_INVALID_STATE,		/* INV: SUP RW-,USR RWX */
     MMU_R4K_STATE_INVALID_STATE,		/* INV: SUP R-X,USR RWX */
     (MMU_STATE_USER_ACCESS
      | MMU_R4K_STATE_WRITABLE)			/*	SUP RWX,USR RWX */
    };

/*
 * This entire table is initialized as invalid.  The mmuMipsR4kLibInit
 * re-initializes entries 1-4 using configuration parameters.
 */

LOCAL UINT mmuStateCacheTransArrayLocal [] =
    {
     MMU_R4K_STATE_INVALID_STATE,	/* Invalid			  */
     MMU_R4K_STATE_INVALID_STATE,	/* Cache Off			  */
     MMU_R4K_STATE_INVALID_STATE,	/* Cache Copyback		  */
     MMU_R4K_STATE_INVALID_STATE,	/* Cache Writethrough		  */
     MMU_R4K_STATE_INVALID_STATE,	/* Mem. Coherency (MC)		  */
     MMU_R4K_STATE_INVALID_STATE,	/* MC and Cache Off		  */
     MMU_R4K_STATE_INVALID_STATE,	/* MC and Cache Copyback	  */
     MMU_R4K_STATE_INVALID_STATE,	/* MC and Writethrough		  */
     MMU_R4K_STATE_INVALID_STATE,	/* Cache I/O			  */
     MMU_R4K_STATE_INVALID_STATE,	/* Cache I/O and Cache Off	  */
     MMU_R4K_STATE_INVALID_STATE,	/* Cache I/0 and Copyback	  */
     MMU_R4K_STATE_INVALID_STATE,	/* Cache I/0 and Writethrough	  */
     MMU_R4K_STATE_INVALID_STATE,	/* Cache I/0 and MC		  */
     MMU_R4K_STATE_INVALID_STATE,	/* Cache I/0, MC and cache off	  */
     MMU_R4K_STATE_INVALID_STATE,	/* Cache I/O, MC and Copyback	  */
     MMU_R4K_STATE_INVALID_STATE,	/* Cache I/O, MC and Writethrough */
     MMU_R4K_STATE_INVALID_STATE	/* Default, filled in at init.	  */
    };

/*
 * This table defines the values for invalid and valid states
 */

LOCAL const UINT mmuStateValidTransArrayLocal [] =
    {
    /* First is NOT_VALID */

    MMU_R4K_STATE_VALID_NOT,

    /* Second is VALID */

    MMU_R4K_STATE_VALID
    };

/*
 * The mask translation structure uses only 3 bits for the index
 * This gives a total of 8 entries.  Entry [0] is set to NULL to
 * insure that if an improper mask is used, it will not access at
 * an invalid index.
 */

LOCAL const UINT mmuMaskTransArrayLocal [] =
    {
    (0),						/* error	*/

    (MMU_R4K_STATE_MASK_WRITABLE |
     MMU_STATE_MASK_USER_ACCESS),			/* prot. mask */

    (MMU_R4K_STATE_MASK_VALID),				/* valid mask */

    (MMU_R4K_STATE_MASK_VALID |
     MMU_R4K_STATE_MASK_WRITABLE |
     MMU_STATE_MASK_USER_ACCESS),			/* prot & valid mask */

    (MMU_R4K_STATE_MASK_CACHEABLE),			/* cache mask */

    (MMU_R4K_STATE_MASK_WRITABLE |
     MMU_R4K_STATE_MASK_CACHEABLE |
     MMU_STATE_MASK_USER_ACCESS),			/* prot & cache mask */

    (MMU_R4K_STATE_MASK_VALID |
     MMU_R4K_STATE_MASK_CACHEABLE),			/* valid & cache mask */

    (MMU_R4K_STATE_MASK_VALID |
     MMU_R4K_STATE_MASK_WRITABLE |
     MMU_R4K_STATE_MASK_CACHEABLE |
     MMU_STATE_MASK_USER_ACCESS)			/* p/c/v mask */
    };

/*
 * This provides settings for all sizes included in MMU_PAGE_SIZES_ALLOWED,
 * but only the smallest entry is actually used.
 */

const MIPS_TLB_SETUP_STRUCT mipsTlbSetupInfo[] =
    {
    /* 8 KB Page size */
    { TLB_8K_VADDR_MASK,
      TLB_8K_PAGE_SIZE_MASK,
      (TLB_8K_PAGE_SIZE >> (MMU_R4K_VPN2_SHIFT - MMU_R4K_PFN_SHIFT)),
      TLB_8K_TABLE_SIZE,
      TLB_8K_VADDR_SHIFT },
    /* 32 KB Page size */
    { TLB_32K_VADDR_MASK,
      TLB_32K_PAGE_SIZE_MASK,
      (TLB_32K_PAGE_SIZE >> (MMU_R4K_VPN2_SHIFT - MMU_R4K_PFN_SHIFT)),
      TLB_32K_TABLE_SIZE,
      TLB_32K_VADDR_SHIFT },
    /* 128 KB Page size */
    { TLB_128K_VADDR_MASK,
      TLB_128K_PAGE_SIZE_MASK,
      (TLB_128K_PAGE_SIZE >> (MMU_R4K_VPN2_SHIFT - MMU_R4K_PFN_SHIFT)),
      TLB_128K_TABLE_SIZE,
      TLB_128K_VADDR_SHIFT },
    /* 512 KB Page size */
    { TLB_512K_VADDR_MASK,
      TLB_512K_PAGE_SIZE_MASK,
      (TLB_512K_PAGE_SIZE >> (MMU_R4K_VPN2_SHIFT - MMU_R4K_PFN_SHIFT)),
      TLB_512K_TABLE_SIZE,
      TLB_512K_VADDR_SHIFT },
    /* 2 MB Page size */
    { TLB_2M_VADDR_MASK,
      TLB_2M_PAGE_SIZE_MASK,
      (TLB_2M_PAGE_SIZE >> (MMU_R4K_VPN2_SHIFT - MMU_R4K_PFN_SHIFT)),
      TLB_2M_TABLE_SIZE,
      TLB_2M_VADDR_SHIFT }
    };

/*
 * The organization of virtual memory segments need to be conveyed to the
 * address space allocator.
 */

LOCAL VIRT_SEG_INFO virtualSegmentInfo [] =
    {
    {
	/* KUSEG */
    (VIRT_ADDR)0x0,			/* don't allocate in bottom 4M */
    0x400000,
    VIRT_SEG_TYPE_NO_MAP		/* un-mappeable */
    },
    {
	/* KUSEG */
    (VIRT_ADDR)0x400000,
    (K0BASE-0x400000),			/* effectively, KUSIZE */
    VIRT_SEG_TYPE_U_MAP			/* user */
    },
    {
	/* KSEG0, KSEG1 */
    (VIRT_ADDR)K0BASE,
    K0SIZE + K1SIZE,
    VIRT_SEG_TYPE_NO_MAP		/* un-mappable */
    },
    {
	/* KSEG2, KSEG3 */
    (VIRT_ADDR)K2BASE,
    K2SIZE + K3SIZE,
    VIRT_SEG_TYPE_K_MAP			/* kernel only */
    }
    };

int mmuMipsWired = -1;			/* MUST be in data segment, not bss */
int mmuMipsWired2 = -1;			/* MUST be in data segment, not bss */

/* Page table sizes */

#define MMU_REGION_TBL_SIZE	1024	/* number of region table entries */

/* -1 page init function */

IMPORT STATUS mmuMipsExcpageInit(void);

/* Globals for windALib.s' context switch */

IMPORT FUNCPTR _func_mmuMipsContextSet;

/* used by OSM handler */

IMPORT __attribute__ ((__section__ (".kseg0bss"))) VIRT_ADDR *_func_excOsmHandler;

/*
 * Global Variables declared by vmLib.c
 */

/* import from vmLib */

IMPORT UINT *		mmuProtStateTransTbl;
IMPORT UINT *		mmuCacheStateTransTbl;
IMPORT UINT *		mmuValidStateTransTbl;
IMPORT UINT *		mmuMaskTransTbl;

IMPORT int mmuInvalidState;

/* import from mmuAim */

IMPORT MMU_ARCH_LIB_ATTRS mmuArchLibAttrs;
IMPORT MMU_ARCH_LIB_FUNCS mmuArchLibFuncs;
IMPORT MMU_CONTEXT_TBL * aimMmuContextTbl;

/* .kseg0bss storage variables */

IMPORT __attribute__ ((__section__ (".kseg0bss"))) FUNCPTR _func_mmuMipsTlbRefillHandler;
IMPORT __attribute__ ((__section__ (".kseg0bss"))) MMU_CONTEXT_TBL * pMmuCntxtTbl;
IMPORT __attribute__ ((__section__ (".kseg0bss"))) UINT32 mmuVaddrMask;
IMPORT __attribute__ ((__section__ (".kseg0bss")))  UINT32 mmuVaddrShift;

/* import from MIPS AIM AD-MMU */

IMPORT void mmuMipsTlbDump(UINT start, UINT count, UINT64 * table);
IMPORT void __mmuMipsClearTlb (UINT nTlbEntries);
IMPORT void __mmuMipsInvTlbEntry (UINT nTlbEntries, UINT asid,
				  VIRT_ADDR virtAddr);
IMPORT void mmuMipsContextSet (UINT asid);
IMPORT UINT mmuMipsContextGet ();
IMPORT UINT mmuMipsWiredGet();
IMPORT void mmuMipsWiredSet(UINT entry);
IMPORT void mmuMipsTlbRefillHandler (void);
IMPORT STATUS mmuMipsTlbWiredEntrySet (UINT32 vAddr, PTE *pPte);
IMPORT void mmuMipsTlbEntryMove (UINT dst, UINT src);
/* SPR 113079 */
IMPORT UINT32 _mipsWiredGet(void);
IMPORT int _mipsWiredSet (UINT32);
IMPORT int palKernelCCAGet (void);
/* end SPR 113079 */

/* Global variables */

IMPORT void vxSdaInitKseg0 (void);

/*
 * This is the structure used to configure the mmu library
 * for the varying MIPS architectures.
 */

typedef struct mmuMipsConfigStruct
    {
    int  nTlbEntries;   /* Number of Entries in TLB */

    UINT minPageSize;	/* minimum size of pages, 8k or greater	*/

    UINT tlbVector;	/* Value of TLB vector for device */
    UINT xtlbVector;	/* Value of XTLB vector for device */

    /* Pointers to subroutines for MIPS device-specific needs */

    MMUMIPSTLBCLRFUNC	   ptrMmuInvalidateRtn;    /* Invalidate Entry */
    MMUMIPSTLBCLRALLFUNC   ptrMmuInvalidateAllRtn; /* Invalidate All   */
    } MMU_MIPS_CONFIG;

MMU_MIPS_CONFIG mmuMipsConfig;

/* MMU Basic API Prototypes */

LOCAL void		mmuMipsTlbInvalidateAll ();
LOCAL void		mmuMipsTlbInvalidate (int context, VIRT_ADDR vAddr);
LOCAL VIRT_ADDR		mmuMipsKmemPtrToPageTablePtr(VIRT_ADDR kmem);
LOCAL VIRT_ADDR		mmuMipsPageTablePtrToKmemPtr(VIRT_ADDR page);
LOCAL STATUS		mmuMipsEnable (int enable);
LOCAL void		mmuMipsCurrentSet (MMU_CONTEXT_TBL * pTransTbl,
					   int		   context);
LOCAL void		mmuMipsPageWrite (int	    context,
					  void *    pPteSrc,
					  VIRT_ADDR virtAddrSrc,
					  void *    pPteDst,
					  VIRT_ADDR virtAddrDst,
					  UINT	    numBytes);
LOCAL void		mmuMipsPteSet (void *	    ptrPte,
				       PHYS_ADDR    physicalAddress,
				       VIRT_ADDR    virtualAddress,
				       UINT	    state,
				       BOOL	    global,
				       UINT	    pageSize,
				       void *	    ptrRte);
LOCAL void		mmuMipsPteInit(void *pPte);
LOCAL UINT		mmuMipsPteStateGet(void *pPte, void *pRte);
LOCAL BOOL		mmuMipsPteValidGet(void *pPte);
LOCAL BOOL		mmuMipsPteGlobalGet(void *pPte);
LOCAL BOOL 		mmuMipsPteIsrCallableGet(void * pPte);
LOCAL UINT		mmuMipsPtePageSizeGet(void *pPte);
LOCAL PHYS_ADDR		mmuMipsPtePhysAddrGet(void *ptrPte);
LOCAL BOOL		mmuMipsStateValidGet(UINT state);
LOCAL UINT		mmuMipsStateCacheModeGet(UINT state);
LOCAL BOOL		mmuMipsStateWriteEnabledGet(UINT state);
LOCAL VIRT_SEG_INFO *	mmuMipsVirtSegInfoGet (UINT *numElems);
LOCAL UINT *		mmuMipsTransTblGet(MMU_CONTEXT_TBL *transTbl,
					   int context);
LOCAL STATUS		mmuMipsAttrTranslate(UINT srcState,
					     UINT * pDestState,
					     UINT srcMask,
					     UINT * pDestMask,
					     BOOL convertToArchDepState);
#ifdef ENABLE_DYNAMIC_SIZE
LOCAL STATUS		mmuMipsPteRangeSet (void *	pTargetPte,
					    void *	pRegionPte,
					    UINT	actualLength,
					    PHYS_ADDR	physAddr,
					    BOOL	on,
					    BOOL	pageTableProtect,
					    VIRT_ADDR	virtAddr);
IMPORT STATUS		mmuMipsPteLockForUpdate (int	   localContext,
						 void *	   pPte,
						 VIRT_ADDR virtAddr,
						 UINT	   actualPageSize,
						 BOOL	   lockSet);
#endif /* ENABLE_DYNAMIC_SIZE */

#ifndef _WRS_CONFIG_SMP
#define LOCK_API
#endif

#ifdef LOCK_API
LOCAL STATUS		mmuMipsLock(void * pVoidPte,
				    VIRT_ADDR virtAddr,
				    UINT size);
LOCAL STATUS 		mmuMipsUnlock(VIRT_ADDR virtAddr);
LOCAL BOOL		mmuMipsPteLockGet(void *pPte);
LOCAL STATUS		mmuMipsPteLockSet(void *pVoidPte,
					  UINT length,
					  BOOL lock,
					  BOOL pageTableProtect);
LOCAL STATUS		mmuMipsLockImport(int index,
					  VIRT_ADDR * pVirtAddr,
					  UINT32 * pSize, BOOL *pWritable);

#endif /* LOCK_API */

LOCAL UINT		mmuMipsMaxPhysBitsGet(void);



/***************************************************************************
 *
 * mmuMipsLibInit() - Initialize the MIPS MMU library.
 *
 * This routine performs the necessary initialization for the MIPS MMU
 * library.  Initialization consists mainly of initializing processing
 * variables, setting up the processing variables for the AIM and vmLib
 * and calling the AIM init function.
 *
 * RETURNS: OK or ERROR if unsuccessful.
 */

STATUS mmuMipsLibInit (void)
    {
    LOCAL MMU_ARCH_LIB_FUNCS  funcs;
    int   bitPos;
    int   index;

    /* Initialize library globals */

    /*
     * Re-initialize the LOCAL UINT mmuStateCacheTransArrayLocal [] table
     * entries for uncacheable, copyback, writethrough, and coherency
     */

    mmuStateCacheTransArrayLocal[MMU_CACHEOFF] =
      (UINT) (MMU_BASE_STATE |
	      (palDesc.tlb.mmuUncacheable << MMU_R4K_CACHE_START));

    /*
     * Reinitialize remaining entries only if there's an applicable init
     * value
     */

    mmuStateCacheTransArrayLocal[MMU_CPYBACK] =
      (UINT) (MMU_BASE_STATE |
	      (palDesc.tlb.mmuCacheCopyback << MMU_R4K_CACHE_START));

    mmuStateCacheTransArrayLocal[MMU_WRTHRU] =
      (UINT) (MMU_BASE_STATE |
	      (palDesc.tlb.mmuCacheWritethrough << MMU_R4K_CACHE_START));


    mmuStateCacheTransArrayLocal[MMU_COHRNT] =
      (UINT) (MMU_BASE_STATE |
	      (palDesc.tlb.mmuCacheCoherency << MMU_R4K_CACHE_START));

    /* Pointers to tables for protection, cache attributes, masks */

    mmuProtStateTransTbl  = (UINT *)mmuStateProtTransArrayLocal;
    mmuCacheStateTransTbl = mmuStateCacheTransArrayLocal;
    mmuValidStateTransTbl = (UINT *)mmuStateValidTransArrayLocal;
    mmuMaskTransTbl	  = (UINT *)mmuMaskTransArrayLocal;

    mmuInvalidState = mmuCacheStateTransTbl[MMU_INVALID];

    /* Initialize the mmuAim specific data structures */

    mmuArchLibAttrs.regionTblNumEntries = MMU_REGION_TBL_SIZE;
    mmuArchLibAttrs.pteSize = MMU_BYTES_PER_ENTRY;

    /*
     * palDesc.tlb.supportedPageSizes identifies the page sizes supported
     * by the hardware based on probing C0_PAGEMASK, however page size
     * optimization is limited to sizes <= 2MB since we don't allow an
     * optimized page to span L1 page-table entries.  (An L1 PTE covers
     * 4MB which is not a MIPS page size; the closest are 2MB and 8MB.)
     * Locking uses a different interface and can use any hardware-supported
     * size.  FIXME - TBD - can locking support > 2MB?
     */

    mmuArchLibAttrs.lockSizeMask = MMU_PAGE_SIZES_ALLOWED; /*alDesc.tlb.supportedPageSizes;according to v6.7*/
    mmuArchLibAttrs.pageSizeMask = MMU_PAGE_MASK_128K; /*32K for 4 chip;palDesc.tlb.supportedPageSizes & MMU_PAGE_SIZES_ALLOWED; modified by liuw for lsn3a according to v6.7 by yinwx*/

    /*
     * Check supported page sizes.
     *
     * Search for the smallest allowed page size, which will determine
     * the required size of an L2 page table.  (An L2 page table must
     * provide space for enough PTE's to map 4MB of address space.)
     *
     * The loop is not really needed since any compliant MIPS processor
     * must support the 8KB page size and this handler won't support
     * anything smaller than that even if the hardware does.
     */

    mmuArchLibAttrs.pageTblNumEntries = 0;
    for (bitPos = MMU_PAGE_MASK_8K, index = 0;
	 bitPos <= MMU_PAGE_MASK_2M;
	 bitPos <<= 2, index++)
	{
	if ((mmuArchLibAttrs.pageSizeMask & bitPos) == bitPos)
	    {
	    /* save the page table size (number of entries) */

	    mmuArchLibAttrs.pageTblNumEntries =
	      mipsTlbSetupInfo[index].tableSize;

	    /* setup TLB Miss Page Table mask and shift values */

	    mmuVaddrMask = mipsTlbSetupInfo[index].vAddrMask;
	    mmuVaddrShift = mipsTlbSetupInfo[index].vAddrShift;

	    /*
	     * we found the smallest page size supported,
	     * exit out of the loop
	     */

	    break;
	    }
	}

    /* check that a supported page size was found */

    if (mmuArchLibAttrs.pageTblNumEntries == 0)
	{
	return (ERROR);
	}

    /* ASID information */

    mmuArchLibAttrs.contextMin  = 0;			/* Min ASID number */
    mmuArchLibAttrs.contextMax  = MMU_NUM_ASID-1;	/* Max ASID number */
    mmuArchLibAttrs.contextTblAlign = MMU_BYTES_PER_ENTRY; /* Align to entry size */

    /* misc. information */

    mmuArchLibAttrs.tlbNumEntries = palDesc.tlb.nEntries;
    mmuArchLibAttrs.tlbLockNumEntries
    = mmuArchLibAttrs.tlbNumEntries - (mmuArchLibAttrs.tlbNumEntries >> 2);

    /* Initialize the mmuAim specific lib function pointers */

    memset(&funcs, 0, sizeof(funcs));
    funcs.mmuTlbInvalidateAllDynamic =	mmuMipsTlbInvalidateAll;
    funcs.mmuTlbInvalidate =		mmuMipsTlbInvalidate;
    funcs.kmemPtrToPageTablePtr =	mmuMipsKmemPtrToPageTablePtr;
    funcs.pageTablePtrToKmemPtr =	mmuMipsPageTablePtrToKmemPtr;
    funcs.mmuEnable =			mmuMipsEnable;
    funcs.mmuCurrentSet =		mmuMipsCurrentSet;
    funcs.mmuCurrentGet =		mmuMipsContextGet;
    funcs.mmuPageWrite =		mmuMipsPageWrite;
    funcs.mmuAttrTranslate =		mmuMipsAttrTranslate;
    funcs.mmuTransTblGet =		mmuMipsTransTblGet;
    funcs.mmuPteSet =			mmuMipsPteSet;
    funcs.mmuPteInit =			mmuMipsPteInit;
    funcs.mmuPteStateGet =		mmuMipsPteStateGet;
    funcs.mmuPteValidGet =		mmuMipsPteValidGet;
    funcs.mmuPteGlobalGet =		mmuMipsPteGlobalGet;
    funcs.mmuPteIsrCallableGet =	mmuMipsPteIsrCallableGet;
    funcs.mmuPtePageSizeGet =		mmuMipsPtePageSizeGet;
    funcs.mmuPtePhysAddrGet =		mmuMipsPtePhysAddrGet;
    funcs.mmuStateValidGet =		mmuMipsStateValidGet;
    funcs.mmuStateCacheModeGet =	mmuMipsStateCacheModeGet;
    funcs.mmuStateWriteEnabledGet =	mmuMipsStateWriteEnabledGet;

#ifdef ENABLE_DYNAMIC_SIZE
    funcs.mmuPteSetForDynamic =		mmuMipsPteRangeSet;
    funcs.mmuLockPteUpdate =		mmuMipsPteLockForUpdate;
#endif /* ENABLE_DYNAMIC_SIZE */

#ifdef LOCK_API
    funcs.mmuLock =			mmuMipsLock;
    funcs.mmuUnlock =			mmuMipsUnlock;
    funcs.mmuPteLockGet =		mmuMipsPteLockGet;
    funcs.mmuPteSetLockState =		mmuMipsPteLockSet;
    funcs.mmuLockImport = 		mmuMipsLockImport;
#endif  /* LOCK_API */

    funcs.mmuPteMaxPhysBitsGet =	mmuMipsMaxPhysBitsGet;

    mmuArchLibFuncs = funcs;

    /* MIPS specific variable setup */

    mmuMipsConfig.nTlbEntries = palDesc.tlb.nEntries;

    /* Pointers to subroutines for MIPS device-specific needs */

    mmuMipsConfig.ptrMmuInvalidateAllRtn =
      (MMUMIPSTLBCLRALLFUNC) KM_TO_K0(__mmuMipsClearTlb);
    mmuMipsConfig.ptrMmuInvalidateRtn =
      (MMUMIPSTLBCLRFUNC) KM_TO_K0(__mmuMipsInvTlbEntry);

    /* TLB Vector Location */

    mmuMipsConfig.tlbVector	= (UINT) palDesc.tVec.vectorAddr;

    /* Allow for 64-bit addressing if regs are 64-bit in size */

#if (_WRS_INT_REGISTER_SIZE == 8)
    mmuMipsConfig.xtlbVector	= (UINT) palDesc.xVec.vectorAddr;
#else
    mmuMipsConfig.xtlbVector	= 0;
#endif /* _WRS_INT_REGISTER_SIZE */

    /* call AIM init */

    if (aimMmuLibInit() != OK)
	{
	return (ERROR);
	}

    /* init pointer to virtSegInfoGet function */
    _func_virtSegInfoGet   = (FUNCPTR) mmuMipsVirtSegInfoGet;

    return (OK);
    }


/*************************************************************************
 *
 * mmuMipsTlbInvalidateAll - Invalidates the entire TLB
 *
 * Makes sure that the entire TLB is invalidated

 * void mmuMipsTlbInvalidateAll ()

 * RETURNS: None

 */
LOCAL void mmuMipsTlbInvalidateAll ()
    {
    /* invalidate the tlb entry */

    mmuMipsConfig.ptrMmuInvalidateAllRtn(mmuMipsConfig.nTlbEntries);
    }


/***************************************************************************
 *
 * mmuMipsTlbInvalidate - Invalidates a particular TLB entry
 *
 * Makes sure that the particular TLB entry is invalidated

 * void mmuMipsTlbInvalidate (int context, VIRT_ADDR kmem)

 * RETURNS: None

 */
LOCAL void mmuMipsTlbInvalidate
    (
    int context,
    VIRT_ADDR vAddr
    )
    {
    /* invalidate the tlb entry */

    mmuMipsConfig.ptrMmuInvalidateRtn(mmuMipsConfig.nTlbEntries,
				      context,
				      vAddr);
    }


/*************************************************************************
 *
 * mmuMipsKmemPtrToPageTablePtr - Convert Kmem Addr to a Page Table Addr
 *
 * The MIPS TLB Miss handler cannot use a Virtual Address to access the
 * Page Tables.  This function converts the KMEM returned address to a
 * useable address.

 * VIRT_ADDR mmuMipsKmemPtrToPageTablePtr (VIRT_ADDR kmem)

 * RETURNS: Page Table address useable by the Tlb Miss Handler

 */
LOCAL VIRT_ADDR mmuMipsKmemPtrToPageTablePtr
    (
    VIRT_ADDR kmem
    )
    {
    return (KM_TO_K0(kmem));
    }


/*************************************************************************
 *
 * mmuMipsPageTablePtrToKmemPtr - Convert Page Table Addr to a Kmem Addr
 *
 * The MIPS TLB Miss handler cannot use a Virtual Address to access the
 * Page Tables.  This function converts the Page Table address used by the
 * Miss handler to a Virtual address.

 * VIRT_ADDR mmuMipsPageTablePtrToKmemPtr (VIRT_ADDR page)

 * RETURNS: Page Table address useable by the Tlb Miss Handler

 */
LOCAL VIRT_ADDR mmuMipsPageTablePtrToKmemPtr
    (
    VIRT_ADDR page
    )
    {
    return (K0_TO_KM(page));
    }


/*************************************************************************
 *
 * mmuMipsEnable - Enable the MMU
 *
 * The MIPS MMU cannot be physically turned on and off.  Rather, this
 * function is used to invalidate all previous address mappings
 * performed by the temporary Refill Exception Handler and to install the
 * permanent Refill Exception Handler.

 * void mmuMipsEnable (int enable)

 * RETURNS: OK or ERROR if unsuccessful.

 */

LOCAL STATUS mmuMipsEnable
    (
    int enable
    )
    {
    int key;

    key = intCpuLock ();

    /* save address of context table */

    pMmuCntxtTbl = aimMmuContextTbl;

    /* Set up task switches to change the CPU Address Space ID */

    _func_mmuMipsContextSet = (FUNCPTR) KM_TO_K0(mmuMipsContextSet);

    bcopy ((const char *) palDesc.tVec.excHandler,
	   (char *)palDesc.tVec.vectorAddr,
	   palDesc.tVec.excSize);

    bcopy ((const char *) palDesc.xVec.excHandler,
	   (char *)palDesc.xVec.vectorAddr,
	   palDesc.xVec.excSize);

    /* Clear out the data to save any dirty cache lines */
    cacheTextUpdate(0, ENTIRE_CACHE);

    /* Switch to the new handler */
    _func_mmuMipsTlbRefillHandler =
      (FUNCPTR) KM_TO_K0(mmuMipsTlbRefillHandler);

    /* init -1 page values */
    mmuMipsExcpageInit();

    /* Release temporary wired entries and flush from TLB */
    _mipsWiredSet (mmuMipsWired);
    mmuMipsConfig.ptrMmuInvalidateAllRtn(mmuMipsConfig.nTlbEntries);

    intCpuUnlock (key);
    *(volatile unsigned char *)0xbfe001e0 = 'a'; /*added for debug by lw according to v6.7 on 20110215*/
    return (OK);
    }


/*************************************************************************
 *
 * mmuMipsCurrentSet - change active translation table

 * RETURNS: N/A

 */

LOCAL void mmuMipsCurrentSet
    (
    MMU_CONTEXT_TBL * pTransTbl,	/* new active translation table */
    int		      context
    )
    {
    int		      lockKey;			/* intCpuLock lock key */

    /* Set new context */

    lockKey = intCpuLock ();
    mmuMipsContextSet(context);
    intCpuUnlock (lockKey);
    }


/*************************************************************************
 *
 * mmuMipsPteSet - Create a Page-Table Entry (PTE)
 *
 * This routine uses the parameters passed to it to create an
 * appropriate page-table entry.  This entry is based on the format of
 * the EntryLo0 and EntryLo1 registers of the device.
 *
 * The result is stored in the memory pointed to by <pPte>.
 *
 * RETURNS: OK if created successfully, otherwise ERROR.
 *
 */

LOCAL void mmuMipsPteSet
    (
    void    * ptrPte,
    PHYS_ADDR	physicalAddress,
    VIRT_ADDR	virtualAddress,
    UINT	state,
    BOOL	global,
    UINT	pageSize,
    void *	ptrRte
    )
    {
    UINT pfnMask;
    PTE *pPte = (PTE *) ptrPte;

    /* Calculate the pfnMask from the pageSize */

    switch (pageSize)
	{
	case TLB_8K_PAGE_SIZE:
	    pfnMask = TLB_8K_PFN_MASK;
	    pPte->pageMask = TLB_8K_PAGE_SIZE_MASK >> TLB_PAGE_MASK_SHIFT;
	    break;
	case TLB_32K_PAGE_SIZE:
	    pfnMask = TLB_32K_PFN_MASK;
	    pPte->pageMask = TLB_32K_PAGE_SIZE_MASK >> TLB_PAGE_MASK_SHIFT;
	    break;
	case TLB_128K_PAGE_SIZE:
	    pfnMask = TLB_128K_PFN_MASK;
	    pPte->pageMask = TLB_128K_PAGE_SIZE_MASK >> TLB_PAGE_MASK_SHIFT;
	    break;
	case TLB_512K_PAGE_SIZE:
	    pfnMask = TLB_512K_PFN_MASK;
	    pPte->pageMask = TLB_512K_PAGE_SIZE_MASK >> TLB_PAGE_MASK_SHIFT;
	    break;
	case TLB_2M_PAGE_SIZE:
	    pfnMask = TLB_2M_PFN_MASK;
	    pPte->pageMask = TLB_2M_PAGE_SIZE_MASK >> TLB_PAGE_MASK_SHIFT;
	    break;
	case TLB_8M_PAGE_SIZE:
	    pfnMask = TLB_8M_PFN_MASK;
	    pPte->pageMask = TLB_8M_PAGE_SIZE_MASK >> TLB_PAGE_MASK_SHIFT;
	    break;
	case TLB_32M_PAGE_SIZE:
	    pfnMask = TLB_32M_PFN_MASK;
	    pPte->pageMask = TLB_32M_PAGE_SIZE_MASK >> TLB_PAGE_MASK_SHIFT;
	    break;
	default:
	    /* could not decode page size, use default */

	    pfnMask = TLB_8K_PFN_MASK;
	    pPte->pageMask = TLB_8K_PAGE_SIZE_MASK >> TLB_PAGE_MASK_SHIFT;
	    break;
	}

    /*
     * The isr-callable state is controlled here.  The locked state
     * has its own lock-set/lock-clear API.
     */

    pPte->isr = (state & MMU_STATE_ISR_CALLABLE) ? 1 : 0;
    pPte->locked = 0;

    /*
     * Remember whether this PTE corresponds to a virtual address in
     * kuseg. This information will be used by mmuMipsPteStateGet to
     * determine whether user mode has access to the memory.
     */
    pPte->isKuseg = IS_KUSEG(virtualAddress) ? 1 : 0;

    /* Extract PFN from physical address			     */
    /* Use mmuMipsConfig struct to get value which to shift physAddr */

    pPte->entryLo0 =
      ((UINT32) ((UINT64) physicalAddress >> (MMU_R4K_VPN2_SHIFT -      \
					     (MMU_R4K_PFN_SHIFT + 1)))) \
      & pfnMask;

    pPte->entryLo0 |= (state & MMU_R4K_TLBLO_MODE_MASK) |
      ((global << MMU_R4K_GLOBAL_START) & MMU_R4K_STATE_GLOBAL);

    /*
     * map second half of the page
     *
     * This bit will be used to create entryLo1 from entryLo0.
     */

    pPte->entryLo1OrValue = (pageSize >> (MMU_R4K_VPN2_SHIFT - \
					   MMU_R4K_PFN_SHIFT));
    }


#ifdef ENABLE_DYNAMIC_SIZE
/*************************************************************************
 *
 * mmuMipsPteRangeSet - set up PTE page size
 *
 * This function sets up optimized page sizes
 * or resets them back to the default page size.
 *
 * RETURNS: OK, or ERROR if actualLength does not
 * correspond to a supported page size.
 *
 * ERRNO
 *
 * NOMANUAL
 *
 * INTERNAL
 *
 * Parameters
 *
 *  whose meaning is the same in both optimize and deoptimize operations
 *
 *   pRegionPte        the lowest-addressed PTE that will be affected.
 *
 *   actualLength      the length of the region being optimized or deoptimized.
 *
 *   physAddr	       the lowest physical address in the region.
 *
 *   pageTableProtect  ?
 *
 *
 *  whose meaning differs depending on the operation
 *
 *                when optimizing           when deoptimizing
 *
 *   pTargetPte   same as pRegionPte        the PTE for the page that needs
 *                                          to be removed from the region
 *
 *   on           TRUE                      FALSE
 *
 *   virtAddr     lowest virtual address    virtual address of the page that
 *                in the region             needs to be removed from the region
 *
 * This initial implementation does not use pTargetPte because, when
 * deoptimizing, it breaks the entire region down to minimum-sized (8KB)
 * pages rather than attempting to retain larger pages where possible.
 * It uses virtAddr only to determine whether the region is part of kuseg,
 * assuming the answer to be the same for all pages in the region, and thus
 * does not need to distinguish between the different meanings of virtAddr.
 */

LOCAL STATUS mmuMipsPteRangeSet
    (
    void	*pTargetPte,		/* see above (not used)		*/
    void	*pRegionPte,		/* lowest affected PTE pointer	*/
    UINT	actualLength,		/* optimized page size		*/
    PHYS_ADDR	physAddr,		/* lowest physical addr		*/
    BOOL	on,			/* optimized enable flag	*/
    BOOL	pageTableProtect,	/* (not used)			*/
    VIRT_ADDR	virtAddr		/* target virtual address	*/
    )
    {
    UINT pageSize, pageMask, pfnMask, physInc;
    int numPages;
    PTE *pPte = (PTE *) pRegionPte;
    int physShift = MMU_R4K_VPN2_SHIFT - MMU_R4K_PFN_SHIFT;

    /* region size must be in MMU_PAGE_SIZES_ALLOWED */

    switch (actualLength)
	{
	case TLB_8K_PAGE_SIZE:
	    pageMask = TLB_8K_PAGE_SIZE_MASK >> TLB_PAGE_MASK_SHIFT;
	    pfnMask = TLB_8K_PFN_MASK;
	    break;
	case TLB_32K_PAGE_SIZE:
	    pageMask = TLB_32K_PAGE_SIZE_MASK >> TLB_PAGE_MASK_SHIFT;
	    pfnMask = TLB_32K_PFN_MASK;
	    break;
	case TLB_128K_PAGE_SIZE:
	    pageMask = TLB_128K_PAGE_SIZE_MASK >> TLB_PAGE_MASK_SHIFT;
	    pfnMask = TLB_128K_PFN_MASK;
	    break;
	case TLB_512K_PAGE_SIZE:
	    pageMask = TLB_512K_PAGE_SIZE_MASK >> TLB_PAGE_MASK_SHIFT;
	    pfnMask = TLB_512K_PFN_MASK;
	    break;
	case TLB_2M_PAGE_SIZE:
	    pageMask = TLB_2M_PAGE_SIZE_MASK >> TLB_PAGE_MASK_SHIFT;
	    pfnMask = TLB_2M_PFN_MASK;
	    break;
	default:
	    return (ERROR);
	}

    if (on)
	{
	/* set an optimized page size */
	pageSize = actualLength;
	physInc = 0;
	}
    else
	{
	/*
	 * Set size to the smallest supported page size,
	 * and overwrite the mask values selected above.
	 */
	physInc = pageSize = TLB_8K_PAGE_SIZE;
	pageMask = TLB_8K_PAGE_SIZE_MASK >> TLB_PAGE_MASK_SHIFT;
	pfnMask = TLB_8K_PFN_MASK;
	}

    /*
     * Set the affected PTEs.
     *
     * It is the responsibility of mmuMipsPteLockForUpdate(), which is called
     * by the AIM before this function (and again afterwards, to clean up) to
     * ensure that no TLB misses involving the region being updated will occur
     * during this loop.
     */

    for ( numPages = actualLength >> TLB_PAGE_MASK_SHIFT ;
	  numPages-- ;
	  ++pPte, physAddr += physInc )
	{
	pPte->pageMask = pageMask;

	/*
	 * Remember whether this PTE corresponds to a virtual address in
	 * kuseg. This information will be used by mmuMipsPteStateGet to
	 * determine whether user mode has access to the memory.
	 * XXX - Since virtAddr does not change within the loop, this assumes
	 * XXX - that the setting of isKuseg should be the same for the entire
	 * XXX - region.
	 */

	pPte->isKuseg = IS_KUSEG(virtAddr) ? 1 : 0;

	/* Extract PFN from physical address				 */
	/* Use mmuMipsConfig struct to get value which to shift physAddr */

	pPte->entryLo0 =
	    (((UINT32) ((UINT64) physAddr >> (physShift - 1))) & pfnMask)
	  | (pPte->entryLo0 & (MMU_R4K_TLBLO_MODE_MASK | MMU_R4K_STATE_GLOBAL));

	/*
	 * map second half of the page
	 *
	 * This bit will be used to create entryLo1 from entryLo0.
	 */

	pPte->entryLo1OrValue = (pageSize >> physShift);
	}

    return (OK);
    }
#endif /* ENABLE_DYNAMIC_SIZE */


/*************************************************************************
 *
 * mmuMipsPteInit - Initialize a Page-Table Entry (PTE)
 *
 * This routine initializes a PTE to invalid.
 * The result is stored in the memory pointed to by <pPte>.
 *
 * RETURNS: None
 *
 */

LOCAL void mmuMipsPteInit
    (
    void *pPte
    )
    {
    ((PTE *) pPte)->pageMask = TLB_8K_PAGE_SIZE_MASK >> TLB_PAGE_MASK_SHIFT;
    ((PTE *) pPte)->reserved = 0;
    ((PTE *) pPte)->isr	     = 0;
    ((PTE *) pPte)->locked   = 0;
    ((PTE *) pPte)->entryLo1OrValue =
      (TLB_8K_PAGE_SIZE >> (MMU_R4K_VPN2_SHIFT - MMU_R4K_PFN_SHIFT));
    ((PTE *) pPte)->entryLo0 = MMU_PTE_INIT_STATE | MMU_R4K_STATE_GLOBAL;
    }


/*************************************************************************
 *
 * mmuMipsPteStateGet - Get State from PTE
 *
 * RETURNS: State
 *
 */

LOCAL UINT mmuMipsPteStateGet
    (
    void *pPte,
    void *pRte
    )
    {
    UINT state = (((PTE *) pPte)->entryLo0) & MMU_R4K_TLBLO_MODE_MASK;
    if (((PTE *) pPte)->isKuseg)
	{
	state |= MMU_STATE_USER_ACCESS;
	}
    /*
    * WIND00182273
    *
    * add the missing state attributes to return value
    * attributes were in the PTE but not in entryLo0
    */
    if (((PTE *) pPte)->isr)
        {
        state |= MMU_STATE_ISR_CALLABLE;
        }
    if (((PTE *) pPte)->locked)
        {
        state |= MMU_STATE_LOCKED;
        }

    return state;
    }


/*************************************************************************
 *
 * mmuMipsPteValidGet - Get Valid from PTE
 *
 * RETURNS: Valid
 *
 */

LOCAL BOOL mmuMipsPteValidGet
    (
    void *pPte
    )
    {
    return (((((PTE *) pPte)->entryLo0) & MMU_R4K_STATE_VALID) ==
	    MMU_R4K_STATE_VALID);
    }


/*************************************************************************
 *
 * mmuMipsPteGlobalGet - Get Global Flag from PTE
 *
 * RETURNS: Global bit
 *
 */

LOCAL BOOL mmuMipsPteGlobalGet
    (
    void *pPte
    )
    {
    return (((((PTE *) pPte)->entryLo0) & MMU_R4K_STATE_GLOBAL) ==
	    MMU_R4K_STATE_GLOBAL);
    }


/***************************************************************************
*
* mmuMipsPteIsrCallableGet - return PTE isr bit
*
* RETURNS: TRUE if PTE is marked as IsrCallable; FALSE if not
*/

LOCAL BOOL mmuMipsPteIsrCallableGet
    (
    void * pVoidPte
    )
    {
    PTE *pPte = (PTE *) pVoidPte;

    return pPte->isr ? TRUE : FALSE;
    }


/*************************************************************************
 *
 * mmuMipsPtePageSizeGet - Get Page Size from PTE
 *
 * RETURNS: Page Size
 *
 */

LOCAL UINT mmuMipsPtePageSizeGet
    (
    void *ptrPte
    )
    {
    UINT pageSize;
    PTE *pPte = (PTE *) ptrPte;

    if (pPte->pageMask == TLB_8K_PAGE_SIZE_MASK >> TLB_PAGE_MASK_SHIFT)
	{
	pageSize = TLB_8K_PAGE_SIZE;
	}
    else if (pPte->pageMask == TLB_32K_PAGE_SIZE_MASK >> TLB_PAGE_MASK_SHIFT)
	{
	pageSize = TLB_32K_PAGE_SIZE;
	}
    else if (pPte->pageMask == TLB_128K_PAGE_SIZE_MASK >> TLB_PAGE_MASK_SHIFT)
	{
	pageSize = TLB_128K_PAGE_SIZE;
	}
    else if (pPte->pageMask == TLB_512K_PAGE_SIZE_MASK >> TLB_PAGE_MASK_SHIFT)
	{
	pageSize = TLB_512K_PAGE_SIZE;
	}
    else if (pPte->pageMask == TLB_2M_PAGE_SIZE_MASK >> TLB_PAGE_MASK_SHIFT)
	{
	pageSize = TLB_2M_PAGE_SIZE;
	}
    else
	{
	/*
	 * this is an error.
	 * entryLo1 is not formatted with a supported page size
	 * return default page size
	 */

	pageSize = TLB_8K_PAGE_SIZE;
	}

    return (pageSize);
    }


/*************************************************************************
 *
 * mmuMipsPtePhysAddrGet - Get Phys Address from PTE
 *
 * RETURNS: Phys Addr
 *
 */

LOCAL PHYS_ADDR mmuMipsPtePhysAddrGet
    (
    void *pPte
    )
    {
    PHYS_ADDR  address;
    PHYS_ADDR  bitMask;

    /* extract entryLo0 Physical Address */

    address = (PHYS_ADDR) ((((PTE *) pPte)->entryLo0 & MMU_R4K_4K_PFN_MASK) <<
			   (MMU_R4K_VPN2_SHIFT - (MMU_R4K_PFN_SHIFT + 1)));

    return (address);
    }


/*************************************************************************
 *
 * mmuMipsStateValidGet - Get Valid from Status
 *
 * RETURNS: Valid
 *
 */

LOCAL BOOL mmuMipsStateValidGet
    (
    UINT state
    )
    {
    return ((state & MMU_R4K_STATE_VALID) ==
	    MMU_R4K_STATE_VALID);
    }


/*************************************************************************
 *
 * mmuMipsStateCacheModeGet - Get Cache Mode from Status
 *
 * RETURNS: Cache Mode
 *
 */

LOCAL UINT mmuMipsStateCacheModeGet
    (
    UINT state
    )
    {
    UINT localCacheMode = (state & MMU_R4K_STATE_MASK_CACHEABLE);

    if (localCacheMode == mmuStateCacheTransArrayLocal[MMU_CPYBACK])
	return (MMU_ATTR_CACHE_COPYBACK);
    else if (localCacheMode == mmuStateCacheTransArrayLocal[MMU_WRTHRU])
	return (MMU_ATTR_CACHE_WRITETHRU);
    else if (localCacheMode == mmuStateCacheTransArrayLocal[MMU_COHRNT])
	return (MMU_ATTR_CACHE_COHERENCY);
    else if (localCacheMode == mmuStateCacheTransArrayLocal[MMU_CACHEOFF])
	return (MMU_ATTR_CACHE_OFF);
    else
	return 0;
    }


/***************************************************************************
*
* mmuMipsStateWriteEnabledGet - Get Write Enable from Status
*
* RETURNS: Write Enable
*
*/

LOCAL BOOL mmuMipsStateWriteEnabledGet
    (
    UINT state
    )
    {
    return ((state & MMU_R4K_STATE_WRITABLE) ==
	    MMU_R4K_STATE_WRITABLE);
    }


/***************************************************************************
*
* mmuMipsInitialMemoryMap - Install initial memory map in MMU TLB
*
* RETURNS: OK always.
*
*/

STATUS mmuMipsInitialMemoryMap
    (
    UINT	nTlbEntries,
    ULONG	excTlbPageBaseAddress,
    UINT32	localMemLocalAdrs,		/* LOCAL_MEM_LOCAL_ADRS */
    UINT32	memTop				/* top of memory */
    )
    {
    extern int 	sysTextProtect;
    extern char _wrs_kernel_text_end[];
    extern char _wrs_kernel_data_start[];
    extern char _wrs_kernel_bss_end[];
	extern char _wrs_kernel_data_end[];   /*added by lw according to  v6.7 on 20110215*/
	extern char end[];	

    PTE		pte;
    int		wired = 0;	/* stack or register temp */
    UINT32	pageSize;
    UINT32	attributes;
    UINT32	vaddr, paddr, eaddr;
    UINT32	ccaBits;
    int *	pTextProtect = (int *) KM_TO_K0 (&sysTextProtect);
	int i;      /*added by lw according to v6.7 on 20110215*/
	unsigned short ttemp=0;

	int tmp;
    /*
     * Don't use VIRT_ADDR/PHYS_ADDR for the vaddr/paddr variables.
     * The values they hold are 32-bit, and we want to avoid compiler
     * intrinsics on MIPS32 as much as possible.
     */
  
    ccaBits = palKernelCCAGet() << MMU_R4K_CACHE_START;

    /* Setup TLB entry for Exc Page */
	tmp = _mipsWiredGet ();           /*added by lw according to v6.7 on 20110215*/
    if (_mipsWiredGet () >= nTlbEntries)
	return (ERROR);
    mmuMipsPteSet(&pte,
		  excTlbPageBaseAddress,
		  (ULONG) EXCPAGE_VIRT_BASE_ADRS,
		  (MMU_R4K_STATE_VALID | MMU_R4K_STATE_WRITABLE | ccaBits),
		  TRUE,    /* global */
		  TLB_8K_PAGE_SIZE,
		  NULL);
    mmuMipsTlbWiredEntrySet ((UINT32) EXCPAGE_VIRT_BASE_ADRS, &pte);

#if 1
	/* This is for SM anchor's TLB!!, added by lw according to yinwx, 20110215 */
	sysSMSetTlbEntry(1,0x40000000,0x110000000,TLB_4K_PAGE_SIZE|(MMU_R4K_STATE_VALID | MMU_R4K_STATE_WRITABLE |MMU_R4K_STATE_GLOBAL| (0x2<<3)));/* 0x110000000 for 4 chips,modified by lw according to v6.7 on 20110215*/
	for(i=0; i<4; i++)
	{ 
	/* This is for SM TLB!!, added by lw according to V6.7 by yinwx, 20110215 */
    	vaddr = 0x42000000 + 0x2000000 * i;
    	paddr = 0x112000000 + 0x2000000 * i;/* 0x112000000 for 4 chips,modified by zk for memory remap*/
    	sysSMSetTlbEntry(2+i,vaddr,paddr,TLB_16M_PAGE_SIZE|(MMU_R4K_STATE_VALID | MMU_R4K_STATE_WRITABLE |MMU_R4K_STATE_GLOBAL| (0x2<<3)));	
	}
#else /*modified by cfg 2011.10.8*/
	/* This is for SM anchor's TLB!!, added by lw according to yinwx, 20110215 */
	sysSMSetTlbEntry(1,0x40000000,0x60000000,TLB_4K_PAGE_SIZE|(MMU_R4K_STATE_VALID | MMU_R4K_STATE_WRITABLE |MMU_R4K_STATE_GLOBAL| (0x2<<3)));/* 0x110000000 for 4 chips,modified by lw according to v6.7 on 20110215*/
	for(i=0; i<4; i++)
	{ 
	/* This is for SM TLB!!*/
    	vaddr = 0x42000000 + 0x2000000 * i;
    	paddr = 0x62000000 + 0x2000000 * i;
    	sysSMSetTlbEntry(2+i,vaddr,paddr,TLB_16M_PAGE_SIZE|(MMU_R4K_STATE_VALID | MMU_R4K_STATE_WRITABLE |MMU_R4K_STATE_GLOBAL| (0x2<<3)));	
	}
#endif
    /* make sure the OSM handler is initialized to NULL */

#ifdef _WRS_CONFIG_SMP
    /* only do this for boot processor */
    if (vxCpuIndexGet() == 0)
#endif /* _WRS_CONFIG_SMP */
    _func_excOsmHandler = (VIRT_ADDR *)NULL;

#ifdef _WRS_CONFIG_SMP    
     /* init -1 page globals as soon as possible after page is setup */
    _WRS_WIND_VARS_ARCH_SET(0, _mipsSrImask, SR_IMASK);
    _WRS_WIND_VARS_ARCH_SET(0, areWeNested, 0);
    _WRS_WIND_VARS_ARCH_SET(0, _mipsEsfTail, 0);
    _WRS_WIND_VARS_ARCH_SET(0, _mipsIpiMailbox, 0);  
 
#endif /* _WRS_CONFIG_SMP */

    /* If we're asked to start past the end of text, just error */

    if (localMemLocalAdrs > (UINT32) _wrs_kernel_text_end)
	return (ERROR);

    /* start of memory has to be at least minimally aligned */

    if (localMemLocalAdrs & (TLB_8K_PAGE_SIZE-1))
	return (ERROR);

    /*
     * Want to wire down kernel text (RO), data & bss (RW)
     * For the moment, just do kernel text, and leave it RW.
     * pageSize should probably be an argument.
     */

    vaddr = localMemLocalAdrs;
    paddr = vaddr & 0x1fffffff;
#if 0
    eaddr = ROUND_DOWN(((UINT32) _wrs_kernel_data_start),
		       TLB_8K_PAGE_SIZE);
#else
   /*added by lw according to v6.7 on 20110215*/
    eaddr = ROUND_DOWN(((UINT32) _wrs_kernel_data_start+TLB_32M_PAGE_SIZE),
		       TLB_32M_PAGE_SIZE/*TLB_8K_PAGE_SIZE*/);
#endif
    /*
     * Wire down kernel text with read-write or read-only permission,
     * depending on the state of sysTextProtect.   We access this global
     * variable through a KSEG0 alias since we don't have a KSEG2 map
     * for the data segment yet.
     */

    attributes = (MMU_R4K_STATE_VALID | ccaBits);
    if (!*pTextProtect)
	attributes |= MMU_R4K_STATE_WRITABLE;

    /*
     * Try to map from vaddr, up to but not including eaddr.
     * When eaddr is reached, switch attributes & handle other
     * bookkeeping as necessary.
     */

    while (vaddr < memTop)
	{
	/* If we're out of entries, we've failed */

	if (_mipsWiredGet () >= nTlbEntries)
	    return (ERROR);

	/*
	 * Find the largest page we can use, depending on alignment
	 * and how close to the region end we are (eaddr).
	 * If mis-aligned, or too big, shrink the page size.
	 */

	pageSize = TLB_32M_PAGE_SIZE;
	/*while ((pageSize >= TLB_8K_PAGE_SIZE)
	       && ((vaddr & (pageSize-1)) || ((vaddr+pageSize) > eaddr)))
	    pageSize >>= 2;*/
	if (pageSize < TLB_8K_PAGE_SIZE)
	    return (ERROR);

	/* set up static page for region [vaddr, vaddr+pageSize] */

	mmuMipsPteSet(&pte,
		      (PHYS_ADDR) paddr,
		      (VIRT_ADDR) vaddr,
		      attributes,
		      TRUE,    /* global */
		      pageSize,
		      NULL);
	mmuMipsTlbWiredEntrySet ((UINT32) vaddr, &pte);
	vaddr += pageSize;
	paddr += pageSize;

	/* If we hit the end of this region, take the necessary steps */

	if (vaddr == eaddr)
	    {
#ifdef _WRS_CONFIG_SMP
	    /*
	     * When we've reached the end of the text section,
	     * we need to stop mapping pages for non-boot processors.
	     */
	    if (vxCpuIndexGet() != 0)
		return (OK);
#endif /* _WRS_CONFIG_SMP */

	    /* record TLB usage at boundary between text and data */

	    if (wired == 0)
		wired = _mipsWiredGet ();

	    /* reset end address to top of memory */

	    eaddr = memTop;

	    /* enable writability for the remaining PTEs */

	    attributes |= MMU_R4K_STATE_WRITABLE;
	    }
	}

    /*
     * We can now can access memory from localMemLocalAdrs through memTop.
     * Note the position of the wired register at this point.  The TLBs
     * are loaded, so we can safely copy local data to the data segment.
     */

    mmuMipsWired = wired;

    /*
     * Record the total TLB usage of this routine.  We will discard the
     * entries between wired and _mipsWiredGet () during the initialization
     * of the MMU library.
     */

    mmuMipsWired2 = _mipsWiredGet ();
 /*added below by lw for debug according to V6.7 on 20110215*/  
printstr("\r\n##############cfg:mmuMipsInitialMemoryMap#########################\r\n"); 
printstr("\r\nwired is 0x"); printnum(mmuMipsWired); printstr("!!\r\n");
printstr("eaddr is 0x"); printnum(ROUND_DOWN(((UINT32) _wrs_kernel_data_start+TLB_32M_PAGE_SIZE),
		       TLB_32M_PAGE_SIZE/*TLB_8K_PAGE_SIZE*/)); printstr("\r\n");
printstr("_wrs_kernel_text_end is 0x"); printnum(_wrs_kernel_text_end); printstr("\r\n");
    return (OK);
    }


/*************************************************************************
*
* mmuMipsInitialMemoryMapSet - Install initial memory map in MMU TLB
*
* mmuMipsInitialMemoryMapSet is called _very_ early in the boot process
* in order to set up the initial memory map for a mapped kernel.  Unlike
* mmuMipsInitialMemoryMap, this function uses a utility function to
* determine the size of the TLB empirically, rather than requiring the
* user to provide a precise TLB size.
*
* RETURNS: OK, or ERROR if the memory map could not be set.
*/

STATUS mmuMipsInitialMemoryMapSet
    (
    ULONG	excTlbPageBaseAddress,
    UINT32	localMemLocalAdrs,		/* LOCAL_MEM_LOCAL_ADRS */
    UINT32	memTop				/* top of memory */
    )
    {
    /* coerce sp and gp into kseg0 */

    vxSdaInitKseg0 ();

    return mmuMipsInitialMemoryMap (mipsTlbSizeGet(),
				    excTlbPageBaseAddress,
				    localMemLocalAdrs,
				    memTop);
    }


/***************************************************************************
 *
 * mmuMipsPageWrite - copy a vm page
 */

LOCAL void mmuMipsPageWrite
    (
    int		context,	/* AIM MMU context number (PID) */
    void *	pPteSrc,	/* Describes data source page */
    VIRT_ADDR	virtAddrSrc,	/* data source virtual address */
    void *	pPteDst,	/* describes data destination page */
    VIRT_ADDR	virtAddrDst,	/* data destination virtual address */
    UINT	numBytes	/* number of bytes to copy */
    )
    {
    UINT	destState;
    UINT	newState;

    /* Write Enabled? */

    /* copy the bytes; this causes a TLB miss which loads the PTE(s) */

    bcopy ((void *)virtAddrSrc, (void *)virtAddrDst, numBytes);
    }


/*************************************************************************
 *
 * mmuMipsTlbBufferShow - Display the TLB entries from a buffer
 *
 * RETURNS: N/A
 *
 */

void mmuMipsTlbBufferShow
    (
    int		mx,
    UINT64	*data64
    )
    {
    int entry;
    char entryWireCode;
    MIPS_TLB_ENTRY *pTlbEntry = (MIPS_TLB_ENTRY *) data64;

    /* display the entries */

#if (_WRS_INT_REGISTER_SIZE == 4)
    printf("Entry  EntryHi   EntryLo0  EntryLo1  PageMask\n");
#elif (_WRS_INT_REGISTER_SIZE == 8)
    printf("Entry  EntryHi           EntryLo0  EntryLo1  PageMask\n");
#endif
    if (!mx)
	mx = mmuMipsConfig.nTlbEntries;

    for (entry = 0; entry < mx; entry++, pTlbEntry++)
	{
	entryWireCode = ((entry < mmuMipsWired) ? '*' :
			 (entry < _mipsWiredGet () ? '+' : ' '));

#if (_WRS_INT_REGISTER_SIZE == 4)
	printf("%2.2d%c    %8.8x  %8.8x  %8.8x  %8.8x\n",
		entry, entryWireCode,
		pTlbEntry->entryHi,
		pTlbEntry->entryLo0,
		pTlbEntry->entryLo1,
		pTlbEntry->pageMask
		);
#elif (_WRS_INT_REGISTER_SIZE == 8)
	printf("%2.2d%c    %8.8x%8.8x  %8.8x  %8.8x  %8.8x\n",
		entry, entryWireCode,
		(UINT32) (pTlbEntry->entryHi >> 32),
		(UINT32) (pTlbEntry->entryHi),
		pTlbEntry->entryLo0,
		pTlbEntry->entryLo1,
		pTlbEntry->pageMask
		);
#else
#error "_WRS_INT_REGISTER_SIZE must be 4 or 8"
#endif
	}
    }


/*************************************************************************
 *
 * mmuMipsTlbShow - Display the TLB entries
 *
 * RETURNS: N/A
 */

void mmuMipsTlbShow
    (
    int	mx
    )
    {
    UINT64 *data;
    int entry;

    /* allocate room for the info */

    data = memalign (0x1000, mmuMipsConfig.nTlbEntries * 24);

    if (data != NULL)
	{
	data = (UINT64 *) KM_TO_K0(data);

	/* get the TLB entries */
	mmuMipsTlbDump (0, mmuMipsConfig.nTlbEntries, data);

	/* display the entries */
	mmuMipsTlbBufferShow (mx, data);

	/* free the space */
	free((void *) K0_TO_KM(data));
	}
    }


/******************************************************************************
*
* mmuMipsVirtSegInfoGet - get virtual memory segment info
*
* Returns a pointer to a VIRT_SEG_INFO table. The table lists that virtual
* memory segments. The elementCount is passed back in the pointer provided.
*
* NOMANUAL
*/

LOCAL VIRT_SEG_INFO * mmuMipsVirtSegInfoGet
    (
    UINT *pNumElems
    )
    {
    *pNumElems = NELEMENTS (virtualSegmentInfo);

    return (VIRT_SEG_INFO *) &virtualSegmentInfo[0];
    }


/******************************************************************************
*
* mmuMipsTransTblGet - Return a pointer to the translation table of
*		       the specified context
*
* RETURNS: The ASID of the translation table
*
* INTERNAL
*
*          VM_TRANS_TBL_GET (vmLibP.h)
*                 |
*    vmLibInfo.pVmTransTblGetRtn (funcBind)
*                 |
*            vmTransTblGet (vmLib, callable from interrupt level)
*                 |
*          MMU_TRANS_TBL_GET (vmLib)
*                 |
*      mmuLibFuncs.mmuTransTblGet (vmLib)
*                 |
*           mmuMipsTransTblGet (mmuMipsLib)
*
* NOMANUAL
*/

LOCAL UINT *mmuMipsTransTblGet
    (
    MMU_CONTEXT_TBL *contextTbl,
    int context
    )
    {
    return (UINT *)context;		/* XXX return ASID field for now */
    }


/******************************************************************************
*
* mmuAttrTranslate - Translate special purpose attributes
*
* mmuAttrTranslate is used to translate to and from the architecture
* independent page attribute to the bit value in the actual MMU page
* table entry.  The cache and protection bits are handled by the
* standard mechanism in vmLib, not by this routine.  The only flags
* translated here are the MMU_ATTR_SPL_* flags.  The supported special
* attributes are:
*
* MMU_ATTR_SPL_7 	MMU_STATE_ISR_CALLABLE
*
* The only allowed mask is:
*
* MMU_ATTR_SPL_MSK	MMU_STATE_MASK_ISR_CALLABLE
*
* RETURNS: OK
*/

LOCAL STATUS mmuMipsAttrTranslate
    (
    UINT srcState,		/* source state */
    UINT * pDestState,		/* destination state */
    UINT srcMask,		/* source mask */
    UINT * pDestMask,		/* destination mask */
    BOOL convertToArchDepState	/* If TRUE VM state arch-dependent state */
    )
    {
    if (convertToArchDepState)
	{
	/*
	 * Return OK if mask doesn't include MMU_ATTR_SPL_MASK because
	 * the attributes have already been translated by the standard way.
	 */

	if ((srcMask & MMU_ATTR_SPL_MSK) == 0)
	    return (OK);

	/*
	 * MMU_ATTR_SPL_7 corresponds to arch-dependent MMU_STATE_ISR_CALLABLE.
	 */

	*pDestMask |= MMU_STATE_MASK_ISR_CALLABLE;	/* set */
	if (srcState & MMU_ATTR_SPL_7)
	    *pDestState |= MMU_STATE_ISR_CALLABLE;	/* set */
	else
	    *pDestState &= ~MMU_STATE_ISR_CALLABLE;	/* cleared */
	}
    else
	{
	/*
	 * The input state is architecture-dependent.  Translate the *
	 * extra bits (in this case just the global bit) to the
	 * architecture-independent value (in this case SPL_7).
	 */

	if (srcState & MMU_STATE_ISR_CALLABLE)
	   *pDestState |= MMU_ATTR_SPL_7;
	else
	   *pDestState &= ~MMU_ATTR_SPL_7;
	}

    return OK;
    }


#ifdef LOCK_API
/***************************************************************************
*
* mmuMipsPteLockGet - return PTE lock bit
*
* mmuMipsPteLockGet is called when the AIM needs to know the state of
* the lock bit for an individual PTE.  This routine extras the lock
* bit from the provided PTE, and returns the lock state.
*
* RETURNS: TRUE if PTE is marked as locked; FALSE if not
*/

LOCAL BOOL mmuMipsPteLockGet
    (
    void *	pteAdrs 	/* PTE to return fields from */
    )
    {
    PTE *pPte = (PTE *) pteAdrs;

    return pPte->locked ? TRUE : FALSE;
    }


/*************************************************************************
*
* mmuMipsPteLockSet - Set an array of PTEs as locked or unlocked.
*
* mmuMipsPteLockSet is called by the AIM to change the lock state of
* a contiguous sequence of PTEs.
*
*  RETURNS: OK
*/

LOCAL STATUS mmuMipsPteLockSet
    (
    void *pVoidPte,		/* pointer to first PTE to modify */
    UINT length,		/* size in bytes of lock/unlock region */
    BOOL locked,		/* TRUE if locking, FALSE otherwise */
    BOOL pageTableProtect	/* (ignored for MIPS) */
    )
    {
    PTE *pPte = (PTE *) pVoidPte;
    int pteCount = length / mmuArchLibAttrs.pageSize;
    int i;

    for (i = 0; i< pteCount; i++, pPte++)
	pPte->locked = locked;

    return(OK);
    }


/***************************************************************************
*
* mmuMipsLock - Lock a range of address space into the TLB.
*
* mmuMipsLock is called by the MMU AIM to lock a portion of the address
* space into the hardware TLB.  This routine uses the wired register to
* maintain the set of locked vs. unlocked TLB entries.  Up to 75% of the
* TLB registers may be locked.
*
* RETURNS: OK if lock successful, ERROR otherwise.
*/

LOCAL STATUS mmuMipsLock
    (
    void *	pVoidPte,	/* pointer to PTE describing locked TLB */
    VIRT_ADDR	virtAddr,	/* base address of locked region */
    UINT	size		/* size in bytes of locked region */
    )
    {
    int key;
    UINT maxWired;
    PTE pte = *(PTE *) pVoidPte;

    /*
     * The wired register is used to globally track the locked
     * regions.  To protect against race conditions on this global
     * register, this entire routine is wrapped in intCpuLock/intCpuUnlock.
     */

    key = intCpuLock ();

    /*
     * confirm that we have space for this locked entry.  To protect
     * against disaster, this routine only allows 75% of the TLB to
     * be locked .
     */

    maxWired = mmuMipsConfig.nTlbEntries - (mmuMipsConfig.nTlbEntries >> 2);
    if (_mipsWiredGet () >= maxWired)
	{
	intCpuUnlock (key);
	return ERROR;
	}

    /*
     * Flush any entries that conflict with the lock region.  We could
     * do this in a loop, but it's simpler just to flush all entries
     * that aren't wired.  Since the lock function isn't performance
     * critical, this inefficiency shouldn't be noticable.
     */

    mmuMipsTlbInvalidateAll ();

    /*
     * The provided TLB entry has the correct physical address and
     * state information, but but it doesn't necessarily have the
     * correct page size or global attribute.
     */

    mmuMipsPteSet (&pte,
		   mmuMipsPtePhysAddrGet (&pte),
		   virtAddr,
		   mmuMipsPteStateGet (&pte, NULL),
		   TRUE, /* global */
		   size,
		   NULL);

    /*
     * Write the new entry.  This will increment the wired register
     * after the PTE has been written to the TLB.
     */

    mmuMipsTlbWiredEntrySet ((UINT32) virtAddr, &pte);

    /* we're done */

    intCpuUnlock (key);
    return OK;
    }


/***************************************************************************
*
* mmuMipsUnlock - Unlocks a region from the TLB.
*
* mmuMipsUnlock is called by the MMU AIM to unlock a locked TLB entry.
* The routine inspects the wired entries in the TLB to find a wired
* entry that has a virtual address that matches the requested address.
* If an entry is found, there are two possibilities.  If the entry is
* the last wired entry in the TLB, this routine simply decrements the
* wired register and flushes the TLB.  If the entry is not the last
* wired entry, this routine shifts the last wired entry downward into
* the slot containing the entry that is being invalidated.  In either
* case, the wired register is decremented.
*
* RETURNS: OK, or ERROR if no TLB entry was found to unlock.
*/

LOCAL STATUS mmuMipsUnlock
    (
    VIRT_ADDR virtAddr		/* starting virtual address of region */
    )
    {
    static MIPS_TLB_ENTRY *pTlbArray;
    int i;
    int key;
    int wired;
    VIRT_ADDR tlbVirtAddr;

    /* allocate miss-safe buffer for reading TLB contents */

    if (pTlbArray == NULL)
	{
	int size = sizeof(MIPS_TLB_ENTRY) * mmuMipsConfig.nTlbEntries;
	pTlbArray =  (MIPS_TLB_ENTRY *) KMEM_ALIGNED_ALLOC (size, 0x1000);
	if (pTlbArray == NULL)
	    {
	    return ERROR;
	    }
	pTlbArray = (MIPS_TLB_ENTRY *) KM_TO_K0 (pTlbArray);
	}

    /*
     * The wired register is used to globally track the locked
     * regions.  To protect against race conditions on this global
     * register, this entire routine is wrapped in intCpuLock/intCpuUnlock.
     */

    key = intCpuLock ();

    /* Copy all wired TLB contents into buffer */

    mmuMipsTlbDump (0, wired = _mipsWiredGet (), (UINT64 *) pTlbArray);

    /*
     * iterate over locked entries, searching for one that has the
     * requested virtual address.  The permanently wired entries at
     * the start of the TLB are ignored during this search.
     */

    for (i = mmuMipsWired; i < wired; i++)
	{
	tlbVirtAddr = (VIRT_ADDR) pTlbArray[i].entryHi & MMU_R4K_VPN2_MASK;
	if (tlbVirtAddr == virtAddr)
	    break;
	}

    /* if we exited loop without a match, return an error */

    if (i == wired)
	{
	intCpuUnlock (key);
	return ERROR;
	}



    if (i < wired - 1)
	{
	/*
	 * If we're unlocking an entry that isn't last wired entry,
	 * we move the last wired entry into the slot that is being
	 * vacated by the entry being invalidated.
	 */
	mmuMipsTlbEntryMove (i, wired - 1);
	}

    /*
     * Decrement the count of wired TLB entries, and invalidate
     * all non-wired entries.
     */


    _mipsWiredSet (wired - 1);
    mmuMipsConfig.ptrMmuInvalidateAllRtn (mmuMipsConfig.nTlbEntries);

    /* we're done */

    intCpuUnlock (key);
    return OK;
    }


/***************************************************************************
*
* mmuMipsLockImport - Import address and size of locked memory region(s).
*
* mmuMipsLockImport is called by the MMU AIM at system startup time
* to discover any locked memory regions that have been set up by the
* operating system.  The MMU AIM calls this function repeatedly, with
* an increasing index number.  If the index references a valid locked
* region, this routine returns the base address and size to the caller.
*
* RETURNS: OK, or ERROR if the requested region is not locked.
*/

LOCAL STATUS mmuMipsLockImport
    (
    int		index,		/* index of locked region to describe */
    VIRT_ADDR * pVirtAddr,	/* virtual address of the region */
    UINT32 * 	pRegionSize,	/* size in bytes of the region */
    BOOL *	pWritable	/* TRUE iff TLB describes writable region */
    )
    {
    MIPS_TLB_ENTRY tlb;
    UINT32 size;

    /*
     * The "-1" page is wired into TLB entry #0, but that page of virtual
     * memory isn't part of the global memory map.  So, we just ignore
     * this entry, since the only reason that "LockImport" is being
     * called is to allow the AIM to set the lock bits on the PTEs for
     * the global memory map.
     */

    index++;

    /* if we run out of wired entries, we're done. */

    if (index >= mmuMipsWired)
	{
	return ERROR;
	}

    /* read this entry. */

    mmuMipsTlbDump (index, 1, (UINT64 *) &tlb);

    /* return virtual address */

    *pVirtAddr = (VIRT_ADDR) (tlb.entryHi & MMU_R4K_VPN2_MASK);

    /* convert page mask into page size */

    switch (tlb.pageMask)
	{
	case TLB_4K_PAGE_SIZE_MASK:   size =   TLB_4K_PAGE_SIZE; break;
	case TLB_16K_PAGE_SIZE_MASK:  size =  TLB_16K_PAGE_SIZE; break;
	case TLB_64K_PAGE_SIZE_MASK:  size =  TLB_64K_PAGE_SIZE; break;
	case TLB_256K_PAGE_SIZE_MASK: size = TLB_256K_PAGE_SIZE; break;
	case TLB_1M_PAGE_SIZE_MASK:   size =   TLB_1M_PAGE_SIZE; break;
	case TLB_4M_PAGE_SIZE_MASK:   size =   TLB_4M_PAGE_SIZE; break;
	case TLB_16M_PAGE_SIZE_MASK:  size =  TLB_16M_PAGE_SIZE; break;
	default:						 return ERROR;
	}

    /* double page size, to account for entrylo0/entrylo1 pairing */

    *pRegionSize = 2 * size;

    /* report writabilty */

    *pWritable = !!(tlb.entryLo0 & MMU_R4K_STATE_MASK_WRITABLE);

    return OK;
    }
#endif /* LOCK_API */


/***************************************************************************
*
* mmuMipsMaxPhysBitsGet - Identify number of bits in PHYS_ADDR
*
* RETURNS: 64, the number of bits in a PHYS_ADDR for MIPS
*/

LOCAL UINT mmuMipsMaxPhysBitsGet (void)
    {
    return (_WRS_NO_BITS_PHYS_ADDR);
    }
