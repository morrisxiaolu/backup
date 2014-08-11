# Makefile - make rules for bsp/config/sibyte
#
# Copyright (c) 2006-2007 Wind River Systems, Inc.
#
# The right to copy, distribute or otherwise make use of this software
# may be licensed only pursuant to the terms of an applicable Wind River
# license agreement.
#
# This file has been developed or significantly modified by the
# MIPS Center of Excellence Dedicated Engineering Staff.
# This notice is as per the MIPS Center of Excellence Master Partner
# Agreement, do not remove this notice without checking first with
# WR/Platforms MIPS Center of Excellence engineering management.
#
# modification history
# --------------------
# 01p,11dec07,jmt  Remove sb1SmBus.c from BOOT_EXTRA
# 01o,11aug07,rlg  Mips Library Restructure - fix cpu variant
# 01n,11aug07,rlg  MIPS Library restructure - fix variant type
# 01m,09aug07,rlg  Changes for the MIPS Library restructuring
# 01l,27jul07,ami  cmdLine referecne removed
# 01k,13apr07,slk  remove sb1IntLib.c, sb1DuartSio.c and sb1Timer.c from
#                  MACH_EXTRA
# 01j,22feb07,pes  Update CPU for new lib structure.
# 01k,13mar07,rlg  Defect WIND00090615 Fix WIND00090617
# 01j,06mar07,rec  WIND00089420 - remove ipnet makefile workaround
# 01i,01mar07,rlg  update for latest code drop from Broadcom
# 01i,01dec06,d_c  Add dependency on sysLib.c to create cmdLine.c
# 01h,24oct06,jmt  Add bootrom.hex to release
# 01g,09sep06,mil  Correct network driver file being used. (sb1SmBus.o instead
#                  of bcm1250SmBus.o)
# 01f,05sep06,rlg  update to add CPU number to extra defines - for removal of
#                  sb1CpuNum.h
# 01e,27jul06,rlg  remove INCLUDE_SM_COMMON define
# 01d,08jul06,wap  Switch to new VxBus SB1 ethernet driver
# 01c,08may06,pes  Correct network driver file being used. (sb1SmBus.o instead
#                  of bcm1250SmBus.o)
# 01b,10apr06,pes  Remove bootrom from 'make release'.
# 01a,14mar06,pes  Created.
#
# DESCRIPTION
# This file contains rules for building VxWorks for the Broadcom SB1-based
# targets.
#
# INCLUDES
#     makeTarget
#*/

CPU	= MIPSI64
TOOL	= diable
CPU_VARIANT	= _sb1

TGT_DIR=$(WIND_BASE)/target
include $(TGT_DIR)/h/make/defs.bsp

## Only redefine make definitions below this point, or your definitions will
## be overwritten by the makefile stubs above.

TARGET_DIR	= h11v2_mipsi64le_smp
VENDOR		= Loongson.Ict.Cas #Broadcom
BOARD		= Huirui1-SMP #Broadcom SB1-based target
EXTRA_DEFINE	+= -D_WRS_MIPS_SB1480

#
# The constants ROM_TEXT_ADRS, ROM_SIZE, RAM_LOW_ADRS and
# RAM_HIGH_ADRS are defined in config.h and MakeSkel.
# All definitions for these constants must be identical.
#

ROM_TEXT_ADRS	= bfc00000	# ROM entry address
ROM_SIZE	= 00200000	# 2MB ROM space

ifeq ($(MAPPED),yes)
LOCAL_MEM_LOCAL_ADRS  = c0000000
RAM_LOW_ADRS	= c0200000	# RAM text/data address
RAM_HIGH_ADRS	= c1000000	# RAM text/data address
EXTRA_DEFINE	+= -DINCLUDE_MAPPED_KERNEL
DATA_SEG_ALIGN	= 00080000
ADJUST_VMA	= 1
else
LOCAL_MEM_LOCAL_ADRS  = 80000000
RAM_LOW_ADRS	= 80200000	#80020000	 RAM text/data address
RAM_HIGH_ADRS	= 81000000	# RAM text/data address
DATA_SEG_ALIGN		= 1
endif

# mxl:  add build bootrom define
#ifeq ($(BUILD_SPEC), bootloader.bin)
EXTRA_DEFINE += -DBOOTROM
#endif

#USRCONFIG = usrConfig.c

MACH_EXTRA	= \
	hs_rdwr.o		\
	bcm1250JTAGSio.o	\
	bcm1250PciLib.o		\
	sb1SmBus.o	\
	htConfig.o	\
	simpleprintf.o	\
	vxbNs16550Sio.o \
	s29GlMtd.o  \
	vxbAlteraTse.o  \
	hrMatrixDma.o  \
	usrNetBootConfig.o \
	vxbMipsZrIntCtlr.o \
	vxbMipsZrIntCtlrIsr.o \
	obcint.o 
	

##	ftllite.o 	mmuMipsLib.o \ vxbDmaBufLib.o \ vxbMipsZrIntCtlr.o	\	vxbMipsZrIntCtlrIsr.o	\

BOOT_EXTRA	= \
	hs_rdwr.o 

RELEASE_CMD	= vxWorks vxWorks.st
RELEASE		= $(RELEASE_PRE) $(RELEASE_CMD) $(RELEASE_PRJ) $(RELEASE_POST)

## Only redefine make definitions above this point, or the expansion of
## makefile target dependencies may be incorrect.

include $(TGT_DIR)/h/make/rules.bsp


