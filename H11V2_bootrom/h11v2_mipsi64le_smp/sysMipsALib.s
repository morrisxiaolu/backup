/* sysMipsALib.s - MIPS system-dependent routines */

/* Copyright 2001 Wind River Systems, Inc. */
#include "copyright_wrs.h"

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
01j,01mar07,rlg  addition of sysGpGet function - derived from latest bsp code
                 drop from Broadcom.
01i,08apr04,pes  Use M{TF]C0 instead of m{tf}c0 when accessing C0_TLBHI.
01h,01dec03,pes  Correct spelling in conditional compilation of sysWiredSet.
01g,05aug03,agf  add routines for TLB related operations
01f,07jun02,jmt  Fix typo with SYS_CONFIG_SET and SYS_PRID_GET macros
01e,29nov01,agf  fix sysConfigSet to load from a0 instead of v0
01e,29nov01,pes  Correct parameter passed to sysConfigSet()
01d,16jul01,tlc  Add CofE copyright.
01c,27jun01,tlc  General cleanup.
01b,21jun01,agf  fix typos in comments
01a,15jun01,tlc  Use HAZARD_VR5400 macro.
*/

/*
DESCRIPTION
This library provides board-specific routines that are shared by all MIPS-based
BSPs.  MIPS BSPs utilize this file by creating a symbolic link from their
directory to target/config/mipsCommon/sysMipsALib.s and include the file at the 
*bottom* of sysALib.s using

	#include "sysMipsALib.s"
	
A list of provided routines follows.  If a BSP requires a specialized routine,
then #define the appropriate MACRO corresponding to the routine to be
specialized in the BSPs sysALib.s file.

	 ROUTINE		  MACRO
	------------------------------------------------------
	sysGpInit		SYS_GP_INIT
	sysGpGet		SYS_GP_GET
	sysCompareSet		SYS_COMPARE_SET
	sysCompareGet		SYS_COMPARE_GET
	sysCountSet		SYS_COUNT_SET
	sysCountGet		SYS_COUNT_GET
	sysPridGet		SYS_PRID_GET
	sysConfigGet		SYS_CONFIG_GET
	sysConfigSet		SYS_CONFIG_SET
	sysIndexSet		SYS_INDEX_SET
	sysIndexGet		SYS_INDEX_GET
	sysRandomSet		SYS_RANDOM_SET
	sysRandomGet		SYS_RANDOM_GET
	sysEntryLo0Set		SYS_ENTRYLO0_SET
	sysEntryLo0Get		SYS_ENTRYLO0_GET
	sysEntryLo1Set		SYS_ENTRYLO1_SET
	sysEntryLo1Get		SYS_ENTRYLO1_GET
	sysPageMaskSet		SYS_PAGEMASK_SET
	sysPageMaskGet		SYS_PAGEMASK_GET
	sysWiredSet		SYS_WIRED_SET
	sysWiredGet		SYS_WIRED_GET
	sysBadVaddrGet		SYS_BADVADDR_GET
	sysEntryHiSet		SYS_ENTRYHI_SET
	sysEntryHiGet		SYS_ENTRYHI_GET
	sysTlbProbe		SYS_TLB_PROBE
	sysTlbRead		SYS_TLB_READ
	sysTlbWriteIndex	SYS_TLB_WRITE_INDEX
	sysTlbWrteRandom	SYS_TLB_WRITE_RANDOM
*/
	
	.globl	sysGpInit
	.globl  sysCompareSet
	.globl  sysCompareGet
	.globl  sysCountSet
	.globl	sysCountGet
	.globl	sysPridGet
	.globl	sysConfigGet
	.globl	sysConfigSet
	.globl	sysIndexSet
	.globl	sysIndexGet
	.globl	sysRandomSet
	.globl	sysRandomGet
	.globl	sysEntryLo0Set
	.globl	sysEntryLo0Get
	.globl	sysEntryLo1Set
	.globl	sysEntryLo1Get
	.globl	sysPageMaskSet
	.globl	sysPageMaskGet
	.globl	sysWiredSet
	.globl	sysWiredGet
	.globl	sysBadVaddrGet
	.globl	sysEntryHiSet
	.globl	sysEntryHiGet
	.globl	sysTlbProbe
	.globl	sysTlbRead
	.globl	sysTlbWriteIndex
	.globl	sysTlbWrteRandom
	
	.globl	GTEXT(hrTlbDump)	
	
	.text

#ifndef SYS_GP_INIT
/*******************************************************************************
*
* sysGpInit - initialize the MIPS global pointer
*
* The purpose of this routine is to initialize the global pointer (gp).
* It is required in order support compressed ROMs.
*
* RETURNS: N/A
*
* NOMANUAL
*/

	.ent	sysGpInit
sysGpInit:
	la	gp, _gp			/* set global pointer from compiler */
	j	ra
	.end	sysGpInit

#endif

#ifndef SYS_GP_GET
/*******************************************************************************
*
* sysGpGet - return the MIPS global pointer
*
* RETURNS: The current value of gp
*
* NOMANUAL
*/

        .globl  sysGpGet
        .ent    sysGpGet
sysGpGet:
        la      v0, _gp                 /* set global pointer from compiler */
        j       ra
        .end    sysGpGet

#endif /* SYS_GP_GET */

#ifndef SYS_COMPARE_SET	
/******************************************************************************
*
* sysCompareSet - set the MIPS timer compare register
*
* RETURNS: N/A

* void sysCompareSet
*     (
*     int compareValue
*     )

* NOMANUAL
*/

	.ent	sysCompareSet
sysCompareSet:
	HAZARD_VR5400
	mtc0	a0,C0_COMPARE
	j	ra
	.end	sysCompareSet
#endif

#ifndef SYS_COMPARE_GET	
/******************************************************************************
*
* sysCompareGet - get the MIPS timer compare register
*
* RETURNS: The MIPS timer compare register value

* int sysCompareGet (void)

* NOMANUAL	
*/

	.ent	sysCompareGet
sysCompareGet:
	HAZARD_VR5400
	mfc0	v0,C0_COMPARE
	j	ra
	.end	sysCompareGet
#endif

#ifndef SYS_COUNT_SET		
/******************************************************************************
*
* sysCountSet - set the MIPS timer count register
*
* RETURNS: N/A

* void sysCountSet
*     (
*     int countValue
*     )

* NOMANUAL	
*/

	.ent	sysCountSet
sysCountSet:
	HAZARD_VR5400
	mtc0	a0,C0_COUNT
	j	ra
	.end	sysCountSet
#endif

#ifndef SYS_COUNT_GET
/******************************************************************************
*
* sysCountGet - get the MIPS timer count register
*
* RETURNS: The MIPS timer count register value
*
* int sysCountGet (void)
*
* NOMANUAL	
*/

	.ent	sysCountGet
sysCountGet:
	HAZARD_VR5400
	mfc0	v0,C0_COUNT
	j	ra
	.end	sysCountGet
#endif
	
#ifndef SYS_PRID_GET
/******************************************************************************
*
* sysPridGet - get the MIPS processor ID register
*
* RETURNS: N/A

* int sysPridGet (void)

*/
	.ent	sysPridGet
sysPridGet:
	HAZARD_VR5400
	#if 0
	mfc0	v0,C0_PRID
	#else
	mfc0	v0, $15, 1
	sync
	#endif
	j	ra
	.end	sysPridGet
#endif

#ifndef SYS_CONFIG_GET				
/******************************************************************************
*
* sysConfigGet - get the MIPS processor CONFIG register
*
* RETURNS: N/A

* int sysConfigGet (void)

*/
	.ent	sysConfigGet
sysConfigGet:
	HAZARD_VR5400
	mfc0	v0,C0_CONFIG
	j	ra
	.end	sysConfigGet
#endif

#ifndef SYS_CONFIG_SET
/******************************************************************************
*
* sysConfigSet - set the MIPS processor CONFIG register
*
* RETURNS: N/A

* int sysConfigSet (void)

*/
	.ent	sysConfigSet
sysConfigSet:
	HAZARD_VR5400
	mtc0	a0,C0_CONFIG
	j	ra
	.end	sysConfigSet
#endif

#ifndef SYS_INDEX_SET
/******************************************************************************
*
* sysIndexSet - set the MIPS processor INDEX register
*
* RETURNS: N/A
*
* void sysIndexSet
*     (
*     UINT32 index
*     )
*
*/
        .ent        sysIndexSet
sysIndexSet:
        mtc0        a0,C0_INX
        HAZARD_TLB
        j        ra
        .end        sysIndexSet
#endif

#ifndef SYS_INDEX_GET
/******************************************************************************
*
* sysIndexGet - get the MIPS processor INDEX register
*
* RETURNS: N/A
*
* UINT32 sysIndexGet (void)
*
*/
        .ent        sysIndexGet
sysIndexGet:
        mfc0        v0,C0_INX
        HAZARD_CP_READ
        j        ra
        .end        sysIndexGet
#endif

#ifndef SYS_RANDOM_SET
/******************************************************************************
*
* sysRandomSet - set the MIPS processor RANDOM register
*
* RETURNS: N/A
*
* void sysRandomSet
*     (
*     UINT32 random
*     )
*
*/
        .ent        sysRandomSet
sysRandomSet:
        mtc0        a0,C0_RAND
        HAZARD_TLB
        .end        sysRandomSet
#endif

#ifndef SYS_RANDOM_GET
/******************************************************************************
*
* sysRandomGet - get the MIPS processor RANDOM register
*
* RETURNS: N/A
*
* UINT32 sysRandomGet (void)
*
*/
        .ent        sysRandomGet
sysRandomGet:
        mfc0        v0,C0_RAND
        HAZARD_CP_READ
        j        ra
        .end        sysRandomGet
#endif

#ifndef SYS_ENTRYLO0_SET
/******************************************************************************
*
* sysEntryLo0Set - set the MIPS processor ENTRYLO0 register
*
* RETURNS: N/A
*
* void sysEntryLo0Set
*     (
*     UINT32 entrylo
*     )
*
*/
        .ent        sysEntryLo0Set
sysEntryLo0Set:
        mtc0        a0,C0_TLBLO0
        HAZARD_TLB
        j        ra
        .end        sysEntryLo0Set
#endif

#ifndef SYS_ENTRYLO0_GET
/******************************************************************************
*
* sysEntryLo0Get - get the MIPS processor ENTRYLO0 register
*
* RETURNS: N/A
*
* UINT32 sysEntryLo0Get (void)
*
*/
        .ent        sysEntryLo0Get
sysEntryLo0Get:
        mfc0        v0,C0_TLBLO0
        HAZARD_CP_READ
        j        ra
        .end        sysEntryLo0Get
#endif

#ifndef SYS_ENTRYLO1_SET
/******************************************************************************
*
* sysEntryLo0Set - set the MIPS processor ENTRYLO1 register
*
* RETURNS: N/A
*
* void sysEntryLo1Set
*     (
*     UINT32 entrylo
*     )
*
*/
        .ent        sysEntryLo1Set
sysEntryLo1Set:
        mtc0        a0,C0_TLBLO1
        HAZARD_TLB
        j        ra
        .end        sysEntryLo1Set
#endif

#ifndef SYS_ENTRYLO1_GET
/******************************************************************************
*
* sysEntryLo1Get - get the MIPS processor ENTRYLO1 register
*
* RETURNS: N/A
*
* UINT32 sysEntryLo1Get (void)
*
*/
        .ent        sysEntryLo1Get
sysEntryLo1Get:
        mfc0        v0,C0_TLBLO1
        HAZARD_CP_READ
        j        ra
        .end        sysEntryLo1Get
#endif

#ifndef SYS_PAGEMASK_SET
/******************************************************************************
*
* sysPageMaskSet - set the MIPS processor PAGEMASK register
*
* RETURNS: N/A
*
* void sysPageMaskSet
*     (
*     UINT32 pagemask
*     )
*
*/
        .ent        sysPageMaskSet
sysPageMaskSet:
        mtc0        a0,C0_PAGEMASK
        HAZARD_TLB
        j        ra
        .end        sysPageMaskSet
#endif

#ifndef SYS_PAGEMASK_GET
/******************************************************************************
*
* sysPageMaskGet - get the MIPS processor PAGEMASK register
*
* RETURNS: N/A
*
* UINT32 sysPageMaskGet (void)
*
*/
        .ent        sysPageMaskGet
sysPageMaskGet:
        mfc0        v0,C0_PAGEMASK
        HAZARD_CP_READ
        j        ra
        .end        sysPageMaskGet
#endif

#ifndef SYS_WIRED_SET
/******************************************************************************
*
* sysWiredSet - set the MIPS processor WRIRED register
*
* RETURNS: N/A
*
* void sysWiredSet
*     (
*     UINT32 wired
*     )
*
*/
        .ent        sysWiredSet
sysWiredSet:
        mtc0        a0,C0_WIRED
        HAZARD_TLB
        j        ra
        .end        sysWiredSet
#endif

#ifndef SYS_WIRED_GET
/******************************************************************************
*
* sysWiredGet - get the MIPS processor WIRED register
*
* RETURNS: N/A
*
* UINT32 sysWiredGet (void)
*
*/
        .ent        sysWiredGet
sysWiredGet:
        mfc0        v0,C0_WIRED
        HAZARD_CP_READ
        j        ra
        .end        sysWiredGet
#endif

#ifndef SYS_BADVADDR_GET
/******************************************************************************
*
* sysBadVaddrGet - get the MIPS processor BADVADDR register
*
* RETURNS: N/A
*
* UINT32 sysBadVaddrGet (void)
*
*/
        .ent        sysBadVaddrGet
sysBadVaddrGet:
        mfc0        v0,C0_BADVADDR
        HAZARD_CP_READ
        j        ra
        .end        sysBadVaddrGet
#endif

#ifndef SYS_ENTRYHI_SET
/******************************************************************************
*
* sysEntryHiSet - set the MIPS processor ENTRYHI register
*
* RETURNS: N/A
*
* void sysEntryHiSet
*     (
*     UINT32 entryhi
*     )
*
*/
        .ent        sysEntryHiSet
sysEntryHiSet:
        MTC0        a0,C0_TLBHI
        HAZARD_TLB
        j        ra
        .end        sysEntryHiSet
#endif

#ifndef SYS_ENTRYHI_GET
/******************************************************************************
*
* sysEntryHiGet - get the MIPS processor ENTRYHI register
*
* RETURNS: N/A
*
* UINT32 sysEntryHiGet (void)
*
*/
        .ent        sysEntryHiGet
sysEntryHiGet:
        MFC0        v0,C0_TLBHI
        HAZARD_CP_READ
        j        ra
        .end        sysEntryHiGet
#endif

#ifndef SYS_TLBPROBE
/******************************************************************************
*
* sysTlbProbe - TLBP instruction
*
* RETURNS: N/A
*
* void sysTlbProbe (void)
*
*/
        .ent        sysTlbProbe
sysTlbProbe:
        tlbp
        HAZARD_CP_READ
        j        ra
        .end        sysTlbProbe
#endif

#ifndef SYS_TLBREAD
/******************************************************************************
*
* sysTlbRead - TLBR instruction
*
* RETURNS: N/A
*
* void sysTlbRead (void)
*
*/
        .ent        sysTlbRead
sysTlbRead:
        tlbr
        HAZARD_CP_READ
        j        ra
        .end        sysTlbRead
#endif

#ifndef SYS_TLB_WRITE_INDEX
/******************************************************************************
*
* sysTlbWriteIndex - TLBWI instruction
*
* RETURNS: N/A
*
* void sysTlbWriteIndex (void)
*
*/
        .ent        sysTlbWriteIndex
sysTlbWriteIndex:
        tlbwi
        HAZARD_CP_WRITE
        /* This for fixing Loongson3A's TLB bug, by yinwx, 20100511 */
		mtc0 zero, C0_PAGEMASK
        j        ra
        .end        sysTlbWriteIndex
#endif

#ifndef SYS_TLB_WRITE_RANDOM
/******************************************************************************
*
* sysTlbWrteRandom - TLBWR instruction
*
* RETURNS: N/A
*
* void sysTlbWrteRandom (void)
*
*/
        .ent        sysTlbWrteRandom
sysTlbWrteRandom:
        tlbwr
        HAZARD_CP_WRITE
        /* This for fixing Loongson3A's TLB bug, by yinwx, 20100511 */
		mtc0 zero, C0_PAGEMASK
        j        ra
        .end        sysTlbWrteRandom
#endif

/******************************************************************************
*
* _MIPS_INT_DISABLE - disable interrupts by masking off SR_IE
*
* When built for R1, this performs the read/modify/write of C0_SR to turn off
* the SR_IE bit.  The mask is usually specified as an immediate operand, but
* may instead be specified as a register (which must already contain ~SR_IE).
*
* When built for R2 it generates a di instruction and uses only the first
* and last parameters.
*
* _MIPS_INT_DISABLE(oldSR, scratch, mask, hazard)
*
* oldSR    Destination register for "mfc0  <oldSR>, C0_SR"
* scratch  Register used to construct and write the new SR value, may be
*          the same as oldSR if the caller does not need the old value.
* mask     Usually the immediate value ~SR_IE, but may
*          instead be a register containing that value.
* hazard   Hazard macro to emit following "mtc0  <scratch>, C0_SR",
*          usually HAZARD_CP_WRITE but occasionally HAZARD_INTERRUPT.
*          May be HAZARD_NULL if the caller handles the hazard.
*/

# ifdef	_WRS_MIPS_ENABLE_R2_ISA
#define	_MIPS_INT_DISABLE(oldSR, scratch, mask, hazard)  \
	.set	noreorder				;\
	di	oldSR					;\
	hazard						;\
	.set	reorder
# else	/* _WRS_MIPS_ENABLE_R2_ISA */
#define	_MIPS_INT_DISABLE(oldSR, scratch, mask, hazard)  \
	.set	noreorder				;\
	mfc0	oldSR, C0_SR				;\
	HAZARD_CP_READ					;\
	and	scratch, oldSR, mask			;\
	mtc0	scratch, C0_SR				;\
	hazard						;\
	.set	reorder
# endif	/* _WRS_MIPS_ENABLE_R2_ISA */

/******************************************************************************
*
* _MIPS_INT_ENABLE - enable interrupts by turning on SR_IE
*
* When built for R1, this performs the read/modify/write of C0_SR to turn on
* the SR_IE bit.  The mask is usually specified as an immediate operand, but
* may instead be specified as a register (which must already contain SR_IE).
*
* When built for R2 it generates an ei instruction and uses only the first
* and last parameters.
*
* _MIPS_INT_ENABLE(oldSR, scratch, mask, hazard)
*
* oldSR    Destination register for "mfc0  <oldSR>, C0_SR"
* scratch  Register used to construct and write the new SR value, may be
*          the same as oldSR if the caller does not need the old value.
* mask     Usually the immediate value SR_IE, but may
*          instead be a register containing that value.
* hazard   Hazard macro to emit following "mtc0  <scratch>, C0_SR",
*          usually HAZARD_CP_WRITE but occasionally HAZARD_INTERRUPT.
*          May be HAZARD_NULL if the caller handles the hazard.
*/

# ifdef	_WRS_MIPS_ENABLE_R2_ISA
#define	_MIPS_INT_ENABLE(oldSR, scratch, mask, hazard)  \
	.set	noreorder				;\
	ei	oldSR					;\
	hazard						;\
	.set	reorder
# else	/* _WRS_MIPS_ENABLE_R2_ISA */
#define	_MIPS_INT_ENABLE(oldSR, scratch, mask, hazard)   \
	.set	noreorder				;\
	mfc0	oldSR, C0_SR				;\
	HAZARD_CP_READ					;\
	or	scratch, oldSR, mask			;\
	mtc0	scratch, C0_SR				;\
	hazard						;\
	.set	reorder
# endif	/* _WRS_MIPS_ENABLE_R2_ISA */



#define FUNC_BEGIN(func)        FUNC_LABEL(func)
#define FUNC_END(func)          .size   FUNC(func), . - FUNC(func)

/**********************************************************************
*
* hrTlbDump
*
* RETURNS: N/A
*
*void hrTlbDump(unsigned int * pBuf);
*
*a0	buffer to write to
*
*/

	.ent	hrTlbDump
FUNC_LABEL(hrTlbDump)

	.set noreorder
	_MIPS_INT_DISABLE(t7, t3, ~SR_INT_ENABLE, HAZARD_INTERRUPT)
	
	li a1,0
	li a2,64
	MFC0	t4, C0_TLBHI  /*back tlbhi*/
	HAZARD_CP_READ
1:
	mtc0	a1,C0_INX		/* start from 0*/
	HAZARD_TLB
	tlbr
	HAZARD_TLB
	MFC0	t0,C0_TLBHI
	MFC0	t1,C0_TLBLO0
	MFC0	t2,C0_TLBLO1
	MFC0	t3,C0_PAGEMASK
	HAZARD_CP_READ
	SW	t0,0(a0)		/* Entry HI */
	SW	t1,8(a0)		/* Entry Lo0 */
	SW	t2,16(a0)		/* Entry Lo1 */
	SW	t3,24(a0)		/* pagemask */
	add	a0,32			/* increment buffer pointer */
	add	a1,1			/* increment index */
	sub	a2,1			/* decrement count */
	bnez	a2,1b			/* continue if count > 0 */
	
	MTC0	t4, C0_TLBHI  /*restore tlbhi*/
	HAZARD_TLB
	
	mtc0	t7,C0_SR		/* UNLOCK INTERRUPTS */
	
	.set	reorder
	j	ra
FUNC_END(hrTlbDump)
	.end	hrTlbDump
