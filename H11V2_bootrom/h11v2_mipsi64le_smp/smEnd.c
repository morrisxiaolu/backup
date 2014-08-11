/* smEnd.c - END shared memory (SM) network interface driver */

/* 
 * Copyright (c) 1999-2006 Wind River Systems, Inc. 
 *
 * The right to copy, distribute, modify or otherwise make use 
 * of this software may be licensed only pursuant to the terms 
 * of an applicable Wind River license agreement. 
 */

/*
modification history
--------------------
02c,10aug07,dtr  Fix call to smIsAlive.
02b,02jul07,bwa  fixed defects 40943 and 71660.
02a,05sep06,kch  Removed unnecessary coreip header includes.
01z,29aug06,bwa  WIND00061961: smEndLoad was using unit# as array index before
                 it was initialized.
01y,10jul06,dtr  Fix smEndIsr to always ack even if no data ready.
01x,24may06,dgp  doc: fix routine formatting in lib description
01w,06oct04,jln  fix compiler warnings (spr#102310)
01v,31aug04,mdo  Documentation fixes for apigen
01u,12may04,hch  fixed diab compiler warning about illegal lvalue
01t,06feb04,jln  ported to T221 and Base6; add smEndLoad; fix polling mode
01s,20sep01,mas  fixed SM packet and mBlk freeing bugs (SPR 70171, 70181)
01r,15jun01,mas  fixed packet, cluster, and MTU size calculations (SPR 68195)
01q,23apr01,mas  pSmPktHdr->reserved1 set in usrNetSmEndSecBoot.c (SPR 66532)
01p,09apr01,mas  made IP address string conversion endian agnostic (SPR 65709)
01o,30mar01,mas  added recording/publishing of netmask (SPR 64957)
01n,15mar01,mas  made driver IP address independent (SPR 34766)
01m,11aug00,mas  added comments for SM_MEM_OFFSET (SPR 32092)
01l,10aug00,hbh  fixed an alignment problem (SPR 32094).
01k,26jun00,mas  fixed packet and mBlk addressing (SPR 32085)
01j,21jun00,mas  fixed packet cpu descriptor indexing
01i,17jun00,mas  fixed ISR NULL reference
01h,11jun00,ham  removed reference to etherLib.
01g,27apr00,mas  slave now waits for master IP address to be set (SPR 30955)
01f,05apr00,mas  added boot dev support for sequential and gateway IP addresses
01e,25feb00,mas  SM master node local memory prep moved to configlette to
		 support compile time determination of VM support in Wind 6.0;
		 added support for address space selection, BSP-specific
		 pointer option, and a bus boot address; put int connect and
		 smPktAttach back in smEndStart().
01d,30nov99,mas  no Mblk freeing by polled send routine (SPR 28492)
01c,04oct99,mas  componentization: ipAttach and netIfConfig now done by boot
		 code.  Int connect and smPktAttach done on first call to
		 smEndIoctl() with code of EIOCGMIB2.
01b,16sep99,mas  modified to reflect H/W independence and multiple SM subnets;
		 added hook routine IOCTL support; removed freeing of driver
		 object (see SPR 28772); code released (SPR 25573)
01a,20jan99,mas  written based on if_sm.c version 01k but with no Unix.
*/

/*
DESCRIPTION
This module implements the VxWorks shared memory (SM) Enhanced Network Driver
(END).

This driver is designed to be moderately generic, operating unmodified
across most targets supported by VxWorks.  To achieve this, the driver must
be given several target-specific parameters, and some external support
routines must be provided.  These parameters are detailed below.

There are no user-callable routines.

This driver is layered between the shared memory packet library and the MUX
modules.  The SM END gives CPUs sharing common memory the ability to
communicate using Internet Protocol (IP).

Sending of multiple frames (mBlk chains) is supported but only single frames
can be received as there is not yet a netBufLib support routine to do so.

I/O CONTROL CODES
The standard END commands implemented are:

\ts
Command        | Data              | Function
---------------|-------------------|-----------------------------------------
EIOCSADDR      | char *            | set SM device address
EIOCGADDR      | char *            | get SM device address
EIOCSFLAGS     | int               | set SM device flags
EIOCGFLAGS     | int               | get SM device flags
EIOCGMWIDTH    | int *             | get memory width (always 0)
EIOCMULTIADD   | --                | [not supported]
EIOCMULTIDEL   | --                | [not supported]
EIOCMULTIGET   | --                | [not supported]
EIOCPOLLSTART  | N/A               | start polled operation
EIOCPOLLSTOP   | N/A               | stop polled operation
EIOCGMIB2      | M2_INTERFACETBL * | return MIB2 information
EIOCGFBUF      | int               | return minimum First Buffer for chaining
EIOCGHDRLEN    | int *             | get ether header length
\te

The driver-specific commands implemented are:

\ts
Command        | Data              | Function
---------------|-------------------|-----------------------------------------
SMIOCGMCPYRTN  | FUNCPTR *         | get mblk copy routine pointer
SMIOCSMCPYRTN  | FUNCPTR           | set mblk copy routine pointer
SMIOCGCCPYRTN  | FUNCPTR *         | get chained mblk copy routine pointer
SMIOCSCCPYRTN  | FUNCPTR           | set chained mblk copy routine pointer
\te

INCLUDES
smEnd.h

\INTERNAL
Data flows through the following layers of code:

\bs
		 -------
		 | MUX |		| multiplexer: interface to
		 -------		| network drivers/protocols
		    |			|
		    |    		|
	      -------------		|
	      |   smEnd	  |		| shared memory Enhanced
	      -------------		| Network Driver
		    |			|
		    |			|
	      -------------		|
	      | smPktLib  |		| shared memory data
	      -------------		| packetization layer
		    |			|
		    |			|
	      -------------		|
	      |   smLib   |		| shared memory functional
	      -------------		| abstraction layer
		    |			|
		    |			|
	      -------------		|
	      | smUtilLib |		| shared memory H/W
	      -------------		| abstraction layer
		    |			|
		    |			|
	===========================	| shared physical memory
\be

\is
\i MUX
Implements the interface between the protocol layer and the data link
layer which contains the END driver.

\i smEnd
Contains the network interface code that is common to all END based
systems.  It is the interface between the network modules and the shared
memory.  It does packet oriented I/O.

\i smPktLib
Reformats data between IP packets and shared memory buffer
packets.

\i smLib
Presents a uniform functional API to varied types of shared memory.

\i smUtilLib
Contains routines to abstract the shared memory hardware.
Presents a uniform interface to any type of shared memory.
\ie

MUX INTERFACE
The interfaces into this module from the MUX module follow.  

\is
\i smEndLoad
Called by the MUX, the routine initializes and attaches this
shared memory network interface driver to the MUX.  It is the only globally
accessible entry into this driver.  This routine typically gets called twice
per SM interface and accepts a pointer to a string of initialization
parameters.  The first call to this routine will be made with an empty string.
This action signals the routine to return a device name, not to load
and initialize the driver.  The second call will be with a valid parameter
string, signalling that the driver is to be loaded and initialized with the
parameter values in the string.  The shared memory region must have been
setup and initialized (via smPktSetup) prior to calling smEndLoad().
Although initialized, no devices will become active until smEndStart() is
called.
\ie

The following routines are all local to this driver but are listed in the
driver entry function table:

\is
\i smEndUnload()
Called by the MUX, this routine stops all associated devices,
frees driver resources, and prepares this driver to be unloaded.  If required,
calls to smEndStop() will be made to all active devices.

\i smEndStart()
Called by the MUX, the routine starts this driver and device(s).
The routine activates this driver and its device(s).  The activities performed
are dependent upon the selected mode of operation, interrupt or polled.

\i smEndStop()
Called by the MUX, the routine stops this driver by inactivating
the driver and its associated device(s).  Upon completion of this routine,
this driver is left in the same state it was just after smEndLoad() execution.

\i smEndRecv()
This routine is not called from the MUX.  It gets called from
this drivers interrupt service routine (ISR) to process input shared memory
packets.  It then passes them on to the MUX.

\i smEndSend()
Called by the MUX, this routine sends a packet via shared memory.

\i smEndPollRec()
Called by the MUX, this routine polls the shared memory region
designated for this CPU to determine if any new packet buffers are available
to be read.  If so, it reads the packet into the supplied mBlk and returns OK
to the MUX.  If the packet is too big for the mBlk or if no packets are
available, EAGAIN is returned.  If the device is not in polled mode, EIO is
returned.

\i smEndPollSend()
Called by the MUX, this routine does a polled send of one
packet to shared memory.  Because shared memory buffers act as a message queue,
this routine will attempt to put the polled mode packet at the head of the list
of buffers.  If no free buffers are available, the buffer currently appearing
first in the list is overwritten with the packet.  This routine returns OK or
an error code directly, not through errno.  It does not free the Mblk it is
passed under any circumstances, that being the responsibility of the caller.

\i smEndIoctl()
Called by the MUX, the routine accesses the control routines for
this driver.

\i smEndMCastAddrAdd()
Called by the MUX, this routine adds an address to a
device's multicast address list.

\i smEndMCastAddrDel()
Called by the MUX, this routine deletes an address from a
device's multicast address list.

\i smEndMCastAddrGet()
Called by the MUX, this routine gets the multicast address
list maintained for a specified device.
\ie

The following routines do not require shared memory specific logic so the
default END library routines are referenced in the function table:

\is
\i endEtherAddressForm()
Called by the MUX, this routine forms an address by
adding appropriate link-level (shared memory) information to a specified mBlk
in preparation for transmission.

\i endEtherPacketDataGet()
Called by the MUX, this routine derives the protocol
specific data within a specified mBlk by stripping the link-level (shared
memory) information from it.  The resulting data are copied to another mBlk.

\i endEtherPacketAddrGet()
Called by the MUX, this routine extracts address
information from one mBlk, ignoring all other data.  Each source and
destination address is written to its own mBlk.  For ethernet packets, this
routine produces two output mBlks (an address pair).  However, for non-ethernet
packets, up to four mBlks (two address pairs) may be produced; two for an
intermediate address pair and two more for the terminal address pair.
\ie

OPTIONAL EXTERNAL SUPPORT
The following routine(s) may be optionally provided for this module at run
time via the associated IOCTL codes:

\is
\i smEndCopyRtn()
\cs
    int smEndCopyRtn (void* source, void* destination, UINT numBytes);
\ce
A function hook to allow the BSP to specify how data are copied between mBlks
and SM packets.  The default is bcopy().  Any function specified must have the
same type, number, and order of input and output arguments.  The following
IOCTL codes apply:

\cs
    SMIOCGMCPYRTN	- get mblk copy routine pointer
    SMIOCSMCPYRTN	- set mblk copy routine pointer
\ce

For example:

\cs
void   myDmaCopyFunc (u_char *, u_char *, unsigned);
int    smFd;	/@ SM file descriptor @/
STATUS result;

    ...
    result = ioctl (smFd, SMIOCSMCPYRTN, (int)myDmaCopyFunc);
    ...
\ce

\i smEndMblkCopyRtn()
\cs
    int smEndMblkCopyRtn (M_BLK_ID, char *, FUNCPTR);
\ce
A function hook to allow the BSP to specify how frames (mblk chains) are
copied to and from SM packets.  The default is netMblkToBufCopy(), a
unidirectional copy.  Any function specified must have the same type, number,
and order of input and output arguments.  The following IOCTL codes apply:

\cs
    SMIOCGCCPYRTN	- get chained mblk copy routine pointer
    SMIOCSCCPYRTN	- set chained mblk copy routine pointer
\ce

For example:

\cs
int    myDmaMblkCopyFunc (M_BLK_ID pFrame, char * pBuf, UINT copyDirection);
int    smFd;	/@ SM file descriptor @/
STATUS result;

    ...
    result = ioctl (smFd, SMIOCSCCPYRTN, (int)myDmaMblkCopyFunc);
    ...
\ce
\ie

TARGET-SPECIFIC PARAMETERS
These parameters are input to this driver in an ASCII string format, using
colon delimited values, via the smEndLoad() routine. Each parameter has a 
preselected radix in which it is expected to be read as shown below.

\ts
Parameter        | Radix| Use
-----------------|------|------------------------------------------------------
SM_UNIT          | 10   | Unit number assigned to shared memory device
SM_NET_DEV_NAME  | --   | String literal name of shared memory device
SM_ANCHOR_ADRS   | 16   | SM anchor region address within SM address space
SM_MEM_ADRS      | 16   | Shared memory address
SM_MEM_MEM_SIZE  | 16   | Shared memory network size in bytes.
                 |      | Used by the master CPU when building SM.
SM_TAS_TYPE      | 10   | Test-and-set type (SM_TAS_HARD or SM_TAS_SOFT)
SM_CPUS_MAX      | 10   | Maximum number of CPUs supported in SM
                 |      | (0 = default number)
SM_MASTER_CPU    | 10   | Master CPU#
SM_LOCAL_CPU     | 10   | This board's CPU number (NONE = use sysProcNumGet)
SM_PKTS_SIZE     | 10   | Max number of data bytes per shared memory packet
                 |      | (0 = default)
SM_MAX_INPUT_PKTS| 10   | Max number of queued receive packets for this CPU
                 |      | (0 = default)
SM_INT_TYPE      | 10   | Interrupt method (SM_INT_MAILBOX/_BUS/_NONE)
SM_INT_ARG1      | 16   | 1st interrupt argument
SM_INT_ARG2      | 16   | 2nd interrupt argument
SM_INT_ARG3      | 16   | 3rd interrupt argument
SM_NUM_MBLKS     | 16   | Number of mBlks in driver memory pool (if < 16,
                 |      | a default value is used)
SM_NUM_CBLKS     | 16   | Number of clBlks in driver memory pool (if < 16,
                 |      | a default value is used)
\te

ISR LIMITATIONS
Because this driver may be used in systems without chaining of interrupts,
and there can be two or more SM subnets using the same type of SM interrupt,
all shared memory subnets are serviced each time there is an interrupt by the
SM interrupt service routine (ISR) smEndIsr().  This is NOT optimal and does
waste some time but is required due to the lack of guaranteed SM interrupt
chaining.

When and if interrupt chaining becomes a guaranteed feature for all SM
interrupt types, the ISR can be optimized.

MESSAGE LIMITATIONS
This driver does not support multicast messages or multicast operations.

SEE ALSO: muxLib, endLib 

\INTERNAL
There are currently certain values stored in what are listed as reserved
locations within various SM structures.  These must be treated with extreme
care so as not to overwrite them:

\ts
Structure           | Reserved field usage
--------------------|------------------------------------------------------
SM_HDR              | reserved1 - Shared memory size
SM_PKT_MEM_HDR      | reserved1 - Master node's IP address
                    | reserved2 - SM heartbeat WD timer ID
SM_CPU_DESC         | reserved1 - Each CPU stores it's IP address here.
                    |             There is one SM_CPU_DESC per CPU in SM.
                    | reserved2 - Each CPU stores its IP netmask here.
\te

Debug Support:
This module has a define to enable compilation of debug instrumentation code
called SM_DBG.  There is also a conditionally compiled global called smEndDebug
which is used as a bit array for enabling/disabling debug reporting in various
routines in this module.  This enables selective debugging with little or no
extraneous information from other code sections.
*/

/* includes */

#include "vxWorks.h"
#include "stdlib.h"
#include "stdio.h"
#include "cacheLib.h"
#include "intLib.h"
#include "end.h"			/* Common END structures. */
#include "endLib.h"
#include "lstLib.h"			/* Needed to maintain protocol list. */
#include "arpLib.h"
#include "wdLib.h"
#include "iv.h"
#include "semLib.h"
#include "taskLib.h"
#include "logLib.h"
#include "netLib.h"
#include "sysLib.h"
#include "errno.h"
#include "errnoLib.h"
#include "memLib.h"
#include "muxLib.h"
#include "etherMultiLib.h"		/* multicast stuff. */
#include "m2Lib.h"
#include "inetLib.h"

#include "net/mbuf.h"
#include "netinet/if_ether.h"
#include "sys/ioctl.h"
#include "sys/times.h"

#include "drv/end/smEnd.h"
#include "smLib.h"
#include "smPktLib.h"
#include "smUtilLib.h"

/* externals */

IMPORT	int    endMultiLstCnt (END_OBJ * pEnd);
IMPORT  int sysReadSlotNum();

/*add by wangzx to decrease warning*/
extern void printstr(char *s);
extern void printnum(unsigned long long n);
/*end wangzx*/

/* defines */

/* shared memory END compound states */

#define S_ACTIVE         (S_LOADED | S_CPU_ATTACHED | S_RUNNING)
#define S_POLLED_SM_RDY  (S_ACTIVE | S_POLLED_SM)
#define S_POLLED_END_RDY (S_ACTIVE | S_POLLED_END)
#define S_POLLED_RDY     (S_ACTIVE | S_POLLED_SM | S_POLLED_END)
#define S_INTR_RDY       (S_ACTIVE)

#define MODE_INTR_MASK   (S_ACTIVE | S_POLLED_RDY | S_RCV_TASK_ACTIVE)

/* macros */

#define SM_ALIGN_OFFSET	2	/* offset to adjust alignment */

/* temp definition */
 
#ifndef SM_NUM_INT_TYPES
#define SM_NUM_INT_TYPES 10
#endif /* SM_NUM_INT_TYPES */

#ifndef NSM
#define NSM 2
#endif /* NSM */

#ifndef SM_POLL_TASK_NAME
#define SM_POLL_TASK_NAME       "tsmPollTask"
#endif /* SM_POLL_TASK_NAME */

#ifndef SM_NET_DEV_NAME
#define SM_NET_DEV_NAME      "sm"
#endif /* SM_NET_DEV_NAME */

/*
 * Default macro definitions for smUtilLib interface.
 * These macros must be redefined when architecture-neutral functional calls
 * exist for: intConnect(), intDisconnect(), intEnable(), intDisable(),
 * sysMailboxDisconnect(), and sysMailboxDisable().
 */

/*
 * temporary; someday will have to make a real one based on a
 * functional intDisconnect()!
 */

#define smUtilIntDisconnect(pri,pIsr,isrArg,iType,iArg1,iArg2,iArg3)  OK

/* ditto: smUtilLib will someday have real versions of these: */

/* Macro to enable the appropriate interrupt level */

#ifndef   smUtilIntEnable
#define smUtilIntEnable(pSmEndDev) \
    do \
        { \
        if (pSmEndDev->intType == SM_INT_BUS) \
            sysIntEnable (pSmEndDev->intArg1); \
        else if (pSmEndDev->intType != SM_INT_NONE) \
            sysMailboxEnable ((char *)pSmEndDev->intArg2); \
        else \
            taskResume (pollTaskId); \
        } while ((0))
#endif /* smUtilIntEnable */

/* Macro to disable the appropriate interrupt level */

#ifndef   smUtilIntDisable
#define smUtilIntDisable(pSmEndDev) \
    do \
        { \
        if (pSmEndDev->intType == SM_INT_BUS) \
            sysIntDisable (pSmEndDev->intArg1); \
        else if (pSmEndDev->intType == SM_INT_NONE) \
            taskSuspend (pollTaskId); \
        } while ((0))
#endif /* smUtilIntDisable */

/* A shortcut for getting the hardware address from the MIB II stuff. */

#define END_HW_ADDR(pEnd)	\
		((pEnd)->mib2Tbl.ifPhysAddress.phyAddress)

#define END_HW_ADDR_LEN(pEnd) \
		((pEnd)->mib2Tbl.ifPhysAddress.addrLength)

#define SM_END_PKT_LEN_GET(pPkt)  ((pPkt)->header.nBytes)

#define SM_END_OLD_MASTER \
    ((pSmEndDev->flags & S_MSTR_PRE_AE_COMPAT) != 0)

#define SM_END_IS_RUNNING  \
    ((END_FLAGS_GET (&pSmEndDev->end) & IFF_RUNNING) != 0)

#define SM_END_IS_LOADED  \
    ((pSmEndDev->flags & S_LOADED) != 0)

#define SM_END_IS_ACTIVE  \
    ((pSmEndDev->flags & S_ACTIVE) == S_ACTIVE)

#define SM_END_IS_ARP_PUB(flags)  \
    ((flags & S_ARP_PUB) == S_ARP_PUB)

#define SM_END_IS_STARTED  \
    ((pSmEndDev->flags & (S_RUNNING | S_CPU_ATTACHED)) == \
     (S_RUNNING | S_CPU_ATTACHED))

#define SM_END_POLL_SM_RDY  \
    ((pSmEndDev->flags & S_POLLED_SM_RDY) == S_POLLED_SM_RDY)

#define MUX_IS_POLLING  \
    ((pSmEndDev->flags & S_POLLED_END_RDY) == S_POLLED_END_RDY)

#define SM_END_INTR_RDY  \
    ((pSmEndDev->flags & MODE_INTR_MASK) == S_INTR_RDY)

#define SM_END_STOP  \
    {pSmEndDev->flags &= ~(S_RUNNING | S_CPU_ATTACHED); \
     END_FLAGS_CLR (&pSmEndDev->end, IFF_UP | IFF_RUNNING);}

#define SM_END_START  \
    {pSmEndDev->flags |= (S_RUNNING | S_CPU_ATTACHED); \
     END_FLAGS_SET (&pSmEndDev->end, IFF_UP | IFF_RUNNING);}

#define SM_RCV_TASK_ACTIVE  \
    (pSmEndDev->flags |= S_RCV_TASK_ACTIVE)

#define SM_RCV_TASK_INACTIVE  \
    (pSmEndDev->flags &= ~S_RCV_TASK_ACTIVE)


/* DEBUG MACROS */

#undef	SM_DBG   
#ifdef	SM_DBG
#  define SM_DBG_OFF		0x0000
#  define SM_DBG_RX		0x0001
#  define SM_DBG_TX		0x0002
#  define SM_DBG_INT		0x0004
#  define SM_DBG_POLL		(SM_DBG_POLL_RX | SM_DBG_POLL_TX)
#  define SM_DBG_POLL_RX	0x0008
#  define SM_DBG_POLL_TX	0x0010
#  define SM_DBG_LOAD		0x0020
#  define SM_DBG_MEM_INIT	0x0040
#  define SM_DBG_UNLOAD		0x0080
#  define SM_DBG_IOCTL		0x0100
#  define SM_DBG_START		0x0200
#  define SM_DBG_STOP		0x0400
#  define SM_DBG_CFG		0x0800
#  define SM_DBG_RSLV		0x1000
#  define SM_DBG_LOAD2		0x2000

#  define SM_LOG(FLG, X0) \
	if (smEndDebug & FLG)                    \
            {printf X0; taskDelay(3);}

int        smEndDebug  = 0;  	/* section debug enable switch */
void       smShowDev (SM_END_DEV * pSmEndDev); 
LOCAL char * smDbgEaddrSprintf ( UCHAR * p);
static char smDigits[] = "0123456789abcdef";
static char smDbgbuf[36];

#else /*SM_DBG*/

#  define SM_LOG(FLG, X0)

#endif /*SM_DBG*/

/* typedefs */

/* Allowed State Changes */

typedef enum {SC_NONE, SC_INIT, SC_POLL2INT, SC_INT2POLL} STATE_CHANGE;

/* globals */

/* unit -> SM_END_DEV * Table */

SM_END_DEV ** unitTbl = NULL;

/* LOCALS */

/* forward static functions */

LOCAL STATUS smEndParse (SM_END_DEV * pSmEndDev, char * pParamStr);
LOCAL STATUS smEndMemInit (SM_END_DEV * pSmEndDev);
LOCAL void   smEndDevFree (SM_END_DEV * pSmEndDev);
LOCAL STATUS smEndConfig (SM_END_DEV * pSmEndDev, STATE_CHANGE sDelta);
LOCAL void   smEndHwAddrSet (SM_END_DEV * pSmEndDev);
LOCAL void   smEndIsr (SM_END_DEV * pSmEndDev);

LOCAL void   smEndSrvcRcvInt (SM_END_DEV * pSmEndDev);
LOCAL STATUS smEndRecv (SM_END_DEV * pSmEndDev, SM_PKT * pPkt, M_BLK_ID pMblk);
LOCAL int    smEndPollQGet (SM_END_DEV * pSmEndDev, M_BLK_ID mBlkId);
LOCAL void   smEndPollQFree (SM_END_DEV * pSmEndDev);
LOCAL void   smEndPulse (SM_PKT_MEM_HDR * pSmPktHdr);
LOCAL STATUS smEndMblkWalk (ULONG maxPktSize, M_BLK * pMblk, UINT * pFragNum,
			    UINT16 * pPktType, UINT * pMaxSize);

/* END Specific interfaces */

/* This is the externally visible interface. */

STATUS smEndDevInit (SM_END_DEV * pSmEndDev);
STATUS smEndUnload (void *);

/* This is the set of internal END routines */

LOCAL STATUS smEndStart  (void *);
LOCAL STATUS smEndStop   (void *);
LOCAL int    smEndIoctl  (void *, UINT32, caddr_t);
LOCAL STATUS smEndSend   (void *, M_BLK_ID);
			  
LOCAL STATUS smEndMCastAddrAdd  (void *, char *);
LOCAL STATUS smEndMCastAddrDel  (void *, char *);
LOCAL STATUS smEndMCastAddrGet  (void *, MULTI_TABLE *);

LOCAL int    smEndPollSend  (void *, M_BLK_ID);
LOCAL int    smEndPollRecv  (void *, M_BLK_ID);

LOCAL STATUS smEndPollStart (void *);
LOCAL STATUS smEndPollStop  (void *);

#if FALSE	/* future enhancement */
LOCAL void smEndAddrFilterSet (void *);
#endif


/*
 * function hook to allow BSP to specify how data are copied between mBlks
 * and SM packets.  The default (within netBufLib) is bcopy().  Any function
 * specified must have the same type, number and order of input arguments!
 */

LOCAL FUNCPTR smEndCopyRtn = (FUNCPTR)bcopy;

/*
 * function hook to allow BSP to specify how frames are copied between chained
 * mBlks and SM packets.  The default (within netBufLib) is netMblkToBufCopy().
 * Any function specified must have the same type, number and order of input
 * arguments!
 */

LOCAL FUNCPTR smEndMblkCopyRtn = (FUNCPTR)netMblkToBufCopy;

/*
 * Declare our function entry table.  This is static across all END driver
 * instances.
 */

LOCAL NET_FUNCS smEndFuncTable =
    {
    (FUNCPTR)smEndStart,	/* Function to start the device */
    (FUNCPTR)smEndStop,		/* Function to stop the device */
    (FUNCPTR)smEndUnload,	/* Unloading function of the driver */
    (FUNCPTR)smEndIoctl,	/* Ioctl function of the driver */
    (FUNCPTR)smEndSend,		/* Send function of the driver */
    (FUNCPTR)smEndMCastAddrAdd,	/* Multicast add function of the driver */
    (FUNCPTR)smEndMCastAddrDel,	/* Multicast delete function of the driver */
    (FUNCPTR)smEndMCastAddrGet,	/* Multicast retrieve function of the driver */
    (FUNCPTR)smEndPollSend,	/* Polling send function */
    (FUNCPTR)smEndPollRecv,	/* Polling receive function */
    endEtherAddressForm,	/* put address info into a NET_BUFFER */
    endEtherPacketDataGet, 	/* get pointer to data in NET_BUFFER */
    endEtherPacketAddrGet 	/* Get packet addresses */
    };

LOCAL int  pollTaskId = NONE;

/*
 *
 * This array will only be needed until there are functional intConnect(),
 * intDisconnect(), intEnable() and intDisable() routines for all BSPs.
 */

LOCAL BOOL connected[SM_NUM_INT_TYPES] =
    {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};

#define DBG_PAUSE(i) \
printf("%d :",i);getchar();printf("\n");

/******************************************************************************
*
* smEndLoad - attach the SM interface to the MUX, initialize driver and device
*
* This routine attaches an SM Ethernet interface to the network MUX.  This
* routine makes the interface available by allocating and filling in an END_OBJ
* structure, a driver entry table, and a MIB2 interface table.
*
* Calls to this routine evoke different results depending upon the parameter
* string it receives.  If the string is empty, the MUX is requesting that the
* device name be returned, not an initialized END_OBJ pointer.  If the string
* is not empty, a load operation is being requested with initialization being
* done with the parameters parsed from the string.
*
* Upon successful completion of a load operation by this routine, the driver
* will be ready to be started, not active.  The system will start the driver
* when it is ready to accept packets.
*
* The shared memory region will be initialized, via smPktSetup(), during the
* call to this routine if it is executing on the designated master CPU.
* The smEndLoad() routine can be called to load only one device unit at a time.
*
* Input parameters are specified in the form of an ASCII string of colon (:)
* delimited values of the form:
*
* "<unit>:<pAnchor>:<smAddr>:<memSize>:<tasType>:
*  <maxCpus>:<masterCpu>:<localCpu>:<maxPktBytes>:<maxInputPkts>:
*  <intType>:<intArg1>:<intArg2>:<intArg3>:<mbNum>:<cbNum>:
*  <configFlg>:<pBootParams>"
*
* The <unit> parameter denotes the logical device unit number assigned by the
* operating system.  Specified using radix 10.
*
* The <pAnchor> parameter is the address of the SM anchor in the given
* <adrsSpace>.  If <adrsSpace> is SM_M_LOCAL, this is the local virtual address
* on the SM master node by which the local CPU may access the shared memory
* anchor.  Specified using radix 16.
*
* The <smAddr> parameter specify the shared memory address; It could be in the
* master node, or in the off-board memory. The address is the local address of the 
* master CPU. If smAddr is NONE, the driver may allocate a cache-safe memory 
* region from the system memory in the master node as the shared memory region; and
* Currently, it is users' responsibility to make sure slave nodes can access this 
* memory, and maintain atomic operations on this region.
*
* The <memSize> parameter is the size, in bytes, of the shared memory region.
* Specified using radix 16.
*
* The <tasType> parameter specifies the test-and-set operation to be used to
* obtain exclusive access to the shared data structures.  It is preferable
* to use a genuine test-and-set instruction, if the hardware permits it.  In
* this case, <tasType> should be SM_TAS_HARD.  If any of the CPUs on the
* SM network do not support the test-and-set instruction, <tasType> should
* be SM_TAS_SOFT.  Specified using radix 10.
*
* The <maxCpus> parameter specifies the maximum number of CPUs that may
* use the shared memory region.  Specified using radix 10.
*
* The <masterCpu> parameter indicates the shared memory master CPU number.
* Specified in radix 10.
*
* The <localCpu> parameter specifies this CPU's number in the SM subnet.
*
* The <maxPktBytes> parameter specifies the size, in bytes, of the data
* buffer in shared memory packets.  This is the largest amount of data
* that may be sent in a single packet.  If this value is not an exact
* multiple of 4 bytes, it will be rounded up to the next multiple of 4.
* If zero, the default size specified in DEFAULT_PKT_SIZE is used.
* Specified using radix 10.
*
* The <maxInputPkts> parameter specifies the maximum number of incoming shared
* memory packets which may be queued to this CPU at one time.  If zero, the
* default value is used.  Specified using radix 10.
*
* The <intType> parameter allows a CPU to announce the method by which it is to
* be notified of input packets which have been queued to it.  Specified using
* radix 10.
*
* The <intArg1>, <intArg2>, and <intArg3> parameters are arguments chosen based
* on, and required by, the interrupt method specified.  They are used to
* generate an interrupt of type <intType>.  Specified using radix 16.
*
* If <mbNum> is non-zero, it specifies the number of mBlks to allocate in the
* driver memory pool.  If <mbNum> is less than 0x10, a default value is used.
* Specified using radix 16.
*
* If <cbNum> is non-zero, it specifies the number of clBlks and, therefore, the
* number of clusters, to allocate in the driver memory pool.  If <cbNum> is
* less than 0x10, a default value is used.  Specified using radix 16.
*
* The number of clBlks is also the number of clusters which will be allocated.
* The clusters allocated in the driver memory pool all have a size of
* <maxPktBytes> bytes.
*
* The <configFlg> parameter indicate some configuration flags for smEnd. The flag
* includes, but not limited to, SMEND_PROXY_SERVER, SMEND_PROXY_CLIENT, 
* SMEND_PROXY_DEFAULT_ADDR, and  SMEND_INCLUDE_SEQ_ADDR.
*
* The <pBootParams> parameter is the address of a BOOT_PARAMS. The smEnd will use 
* this structure to get the backplane IP address, and/or anchor address.
*
* RETURNS: return values are dependent upon the context implied by the input
* parameter string length as shown below.
*
* \ts
* Length  | Return Value
* --------|---------------------------------------------------------------
* 0       | OK and device name copied to input string pointer or ERROR if
*         | NULL string pointer.
* non-0   | END_OBJ * to initialized object or NULL if bogus string or an
*         | internal error occurs.
* \te
*
* \INTERNAL: multicasting to be added later
*/

END_OBJ * smEndLoad
    (
    char * pParamStr	/* ptr to initialization parameter string */
    )
    {
    char *          cp                       = NULL;
    char *          pAnchor                  = NULL;
    END_OBJ *       pEnd                     = NULL;
    SM_END_DEV *    pSmEndDev                = NULL;
    BOOL            isBootDev                = FALSE;
    BOOL            nodeIsMaster             = FALSE;
    struct in_addr  localAddr;             /* local IP address */
    char            bpAddr [BOOT_DEV_LEN]; /* backplane IP address */
    char *          bpAnchor;
	/*int			nodeNum;*/ /* added by yinwx, 20100303,comment by wangzx avoid warning */

    SM_LOG (SM_DBG_LOAD, ("smEndLoad: ptr = %p, len = %d, *ptr = %s\n",
            pParamStr, strlen(pParamStr), pParamStr));

    /* sanity check */

    if (pParamStr == NULL)
        return (pEnd);

    /* On the first pass, return name of shared memory device to MUX. */

    if (strlen (pParamStr) == 0)
        {
        strncpy (pParamStr, SMEND_DEV_NAME, (strlen(SMEND_DEV_NAME) + 1));
        SM_LOG (SM_DBG_LOAD, ("smEndLoad: 1st pass: dev name = %s\n",
                pParamStr));
        return (pEnd);
        }

    /* allocate the SM_END_DEV structure */

    SM_LOG (SM_DBG_LOAD, ("smEndLoad: 2nd pass\n"));
    if ((pSmEndDev = (SM_END_DEV *)calloc (1, sizeof (SM_END_DEV))) == NULL)
        {
        SM_LOG (SM_DBG_LOAD, ("smEndLoad: can't Allocate SM_END_DEV\n"));
        return (pEnd);
        }

    /* If not already, allocate unit table */

    if (unitTbl == NULL)
        {
        if ((unitTbl = (SM_END_DEV **)calloc (NSM, sizeof (SM_END_DEV *)))
                     == NULL)
            {
            cfree ((char *)pSmEndDev);
            return (pEnd);
            }
        }

    /* store the device name */

    strncpy (pSmEndDev->devName, SMEND_DEV_NAME, (sizeof(SMEND_DEV_NAME) + 1));

    /* Parse input string and set unit#, device name, and any overrides */

    SM_LOG (SM_DBG_LOAD, ("Parsing string...\n"));
    if (smEndParse (pSmEndDev, pParamStr) == ERROR)
        {
        cfree ((char *)pSmEndDev);
        return (pEnd);
        }

    /* now that we have obtained our unit#, store our END object in table  */

    unitTbl [pSmEndDev->unit] = pSmEndDev;

    /* If this is the master CPU, determine addressing mode */

    SM_LOG (SM_DBG_LOAD, ("Master CPU = %#x, Local CPU = %#x\n",
            pSmEndDev->masterCpu, pSmEndDev->localCpu));

    if (pSmEndDev->masterCpu == pSmEndDev->localCpu)
        {
        pSmEndDev->isMaster = TRUE;
        nodeIsMaster = TRUE;
        SM_LOG (SM_DBG_LOAD, ("MASTER node\n"));
        }

    /* Get the Anchor and backplane IP if they are specified in the bootline */

    if (pSmEndDev->pBootParams == NULL)
        {
        SM_LOG(SM_DBG_LOAD, ("smEndLoad: boot params is not available"));
        cfree ((char *)pSmEndDev);
        return (pEnd);
        }

    /* determine anchor address if "sm=" is specified */
    if ((strncmp (pSmEndDev->pBootParams->bootDev, "sm", 2) == 0))
        {
	/* sanity check */

        if (pSmEndDev->isMaster) 
	    {
            printf ("SM master can not boot from SM\n");
            cfree ((char *)pSmEndDev);
            return (pEnd);
            }

	/* slave node with "sm" boot */ 

         pSmEndDev->isBoot = TRUE;	/* we are boot device */
         isBootDev = TRUE;
            
         SM_LOG (SM_DBG_LOAD, ("smEndLoad: we are boot dev\n"));

         /*
          * If explicit anchor address, use it.  The SM boot device
          * parameter string is expected to be of the form:
          *
          *   sm=<anchorAdrs>
          */

         if ((strncmp (pSmEndDev->pBootParams->bootDev, "sm=", 3) == 0))
	     {
             if (bootBpAnchorExtract (pSmEndDev->pBootParams->bootDev, &bpAnchor) < 0)
                 {
                 printf ("Invalid anchor address specified: \"%s\"\n", 
                                                pSmEndDev->pBootParams->bootDev);
                 cfree ((char *)pSmEndDev);
                 return (pEnd);
                 }

             /* override the ANCHOR address specified in the loadstring */

             pSmEndDev->pAnchor = (void *) bpAnchor;
             }
        }

    /* check other field */

    else if ((strncmp (pSmEndDev->pBootParams->other, "sm=", 3) == 0))
        {
        if (bootBpAnchorExtract (pSmEndDev->pBootParams->other, &bpAnchor) < 0)
            {
            printf ("Invalid anchor address specified: \"%s\"\n", 
                            pSmEndDev->pBootParams->bootDev);
            cfree ((char *)pSmEndDev);
            return (pEnd);
            }

         /* override the ANCHOR address specified in the loadstring */

         pSmEndDev->pAnchor = (void *) bpAnchor;
        }

    /* sanity check */
    
    if (pSmEndDev->pAnchor == NULL)
        {
        printf ("ERROR: NULL SM anchor address!\n");
        cfree ((char *)pSmEndDev);
        return (pEnd);
        }

    /* verify anchor address is properly aligned */

     if (((UINT)pSmEndDev->pAnchor & (SM_ALIGN_BOUNDARY - 1)) != 0)
         {
         printf ("smEndLoad: Anchor address %p not properly aligned!\n",
                  pSmEndDev->pAnchor);
         cfree ((char *)pSmEndDev);
         return (pEnd);
         }

     /* MASTER */

     if (pSmEndDev->isMaster)
         {
         if ((pSmEndDev->memSize == 0) || (pSmEndDev->pMem == NULL))
              {
              printf ("smEndLoad: no SM net memory area!\n");
              cfree ((char *)pSmEndDev);
              return (pEnd);
              }           

         if (pSmEndDev->pMem == (char *) NONE)          /* allocate the shared memory */
             {
             if (!CACHE_DMA_IS_WRITE_COHERENT () || !CACHE_DMA_IS_READ_COHERENT ())
                 {
                 logMsg ("smNetInit - cache coherent buffer not available\n",
                           0, 0, 0, 0, 0, 0);
                 cfree ((char *)pSmEndDev);
                 return (pEnd);
                 }

              if ((pSmEndDev->pMemAlloc = (char *) cacheDmaMalloc 
                                           (pSmEndDev->memSize + SM_ALIGN_BOUNDARY - 1)) == NULL)
		  {
                  cfree ((char *)pSmEndDev);
                  return (pEnd);
                  }
       
              pSmEndDev->pMem = (char *) ((ULONG)(pSmEndDev->pMemAlloc) & ~(SM_ALIGN_BOUNDARY - 1));
              }
          else
              {
              if (pSmEndDev->pMem == (char *) pAnchor)
                  {
                  int ruOffset = ROUND_UP (sizeof(SM_ANCHOR), SM_ALIGN_BOUNDARY);

                  pSmEndDev->pMem    +=  ruOffset;
                  pSmEndDev->memSize -=  ruOffset;
                  }
             
              /* ensure properly aligned address */

              if (((UINT)pSmEndDev->pMem & (SM_ALIGN_BOUNDARY - 1)) != 0)
                  {
                  printf ("smEndLoad: SM region address %p not aligned!\n",
                           pSmEndDev->pMem);
                  cfree ((char *)pSmEndDev);
                  return (pEnd);
                  }
	       }
	 }
    
    SM_LOG (SM_DBG_LOAD,
            ("MASTER: anchor = %p, memory @ %p, size = %#lx\n",
            pSmEndDev->pAnchor, pSmEndDev->pMem, pSmEndDev->memSize));

    /* announce anchor address */

    printf ("Attaching to SM net with memory anchor at %p, memory at %p...\n",
            (void *)pSmEndDev->pAnchor, (void *)pSmEndDev->pMem);

#ifdef SM_DBG
    smShowDev (pSmEndDev);
#endif /* SM_DBG */
    
    /*
     * At this point, any node may have an IP address assigned manually
     * in the boot parameters.  A boot param IP address will always be
     * used, regardless of the addressing mode.  If an IP address is not
     * set at this point, it must be because 1) this is a slave node that
     * will be assigned an address later via CPU sequence or 2) this is 
     * a master node that will be assigned an address via proxy server with 
     * the proxy default address. If one is available, record IP address for
     * information only.
     */

    /* strip any netmask off of IP address */

    bpAddr[0] = EOS;
    strcpy (bpAddr, pSmEndDev->pBootParams->bad);
    if ((cp = strchr (bpAddr, (int)':')) != NULL)
        {
        *cp = EOS;
        }
    
    if (pSmEndDev->isMaster)
        {
	/*  get the IP address from the bootParams */

        if (*bpAddr == EOS)
	   {

	    /* no IP address set up for backplane in boot parameter */
 
            if ((pSmEndDev->configFlag & 
                 (SMEND_PROXY_SERVER_FLAG | SMEND_PROXY_DEFAULT_ADDR_FLAG)) != 
                 (SMEND_PROXY_SERVER_FLAG | SMEND_PROXY_DEFAULT_ADDR_FLAG))
	        {
                printf ("Error: smEndLoad() needs to specify master IP address!\n");
                if (pSmEndDev->pMemAlloc != NULL)
                    cacheDmaFree (pSmEndDev->pMemAlloc);
                cfree ((char *)pSmEndDev);
                return (pEnd);
                }
             else
	        {
                char eadBuf [80];

                /* strip off netmask from the end */
                eadBuf[0] = EOS;
                strcpy (eadBuf, pSmEndDev->pBootParams->ead);
                if ((cp = strchr (eadBuf, (int)':')) != NULL)
                    {
                    *cp = EOS;
                    }

                 /* configure an SM IP address for proxy ARP server */

                  pSmEndDev->ipAddr = ntohl (inet_addr(eadBuf)) + 1;
                  pSmEndDev->startAddr  = pSmEndDev->ipAddr;
                  pSmEndDev->netmask    = 0xffffffff;
		}
	   }
        else
            {
	    /* convert ASCII to IP address */ 

            if (inet_aton (bpAddr, &localAddr) != OK)
                {
                printf ("smEndLoad: SM IP address assignment failed: %s\n",
                        bpAddr);
                if (pSmEndDev->pMemAlloc != NULL)
                    cacheDmaFree (pSmEndDev->pMemAlloc);
                cfree ((char *)pSmEndDev);
                return (pEnd);
                }
            pSmEndDev->ipAddr     = ntohl (localAddr.s_addr);
            pSmEndDev->startAddr  = pSmEndDev->ipAddr;
            pSmEndDev->masterAddr = pSmEndDev->ipAddr;
            }
        }
    else if (*bpAddr == EOS)   /* slave without backplane IP */
        {
        /* determine IP address later */

        if (pSmEndDev->configFlag & SMEND_SM_SEQ_ADDR)      /* if INCLUDE_SM_SEQ_ADDR */ 
            pSmEndDev->startAddr = 0xfaceface;
        else
	    { 
            pSmEndDev->startAddr = 0;
            printf ("Error: smEndLoad: slave IP address required!\n");
            if (pSmEndDev->pMemAlloc != NULL)
                    cacheDmaFree (pSmEndDev->pMemAlloc);
            cfree ((char *)pSmEndDev);
            return (pEnd);
	    }
        }
    else        /* slave with IP backplane setup */
        {
        /*
         * An IP address assigned in the boot parameters overrides any
         * sequential address.
         */
		
printf("In bootline bpAddr is %s\n", bpAddr);
        if (inet_aton (bpAddr, &localAddr) != OK)
            {
            printf ("smEndLoad: SM IP address assignment failed: %s\n",
                    bpAddr);
            if (pSmEndDev->pMemAlloc != NULL)
                    cacheDmaFree (pSmEndDev->pMemAlloc);
            cfree ((char *)pSmEndDev);
            return (pEnd);
            }
		
		/* Here added by yinwx, to differ slave IP, 20100303 */
#if 0        
	 	nodeNum = (sysPridGet() & 0xc)>>2;
		localAddr.s_addr += (nodeNum << 24); /* for little endian, yinwx */
	 	strcpy(sysBootParams.bad, inet_ntoa(localAddr)); 
printf("~~ smEndLoad: localAddr.s_addr is 0x%08x ~~\n", localAddr.s_addr);
#endif
        pSmEndDev->ipAddr    = ntohl (localAddr.s_addr);
printf("~~ smEndLoad: pSmEndDev->ipAddr is 0x%08x ~~\n", pSmEndDev->ipAddr);

        pSmEndDev->startAddr = pSmEndDev->ipAddr;
        }
    
        /* finish driver load and device setup */

    if (smEndDevInit (pSmEndDev) != ERROR)
        {
        SM_ANCHOR      * pAnchor;
        SM_CPU_DESC    * pCpuDesc;
        SM_HDR         * pSmHdr;
        SM_PKT_MEM_HDR * pSmPktHdr;
       
        if (pSmEndDev->startAddr != 0)
            {
            localAddr.s_addr = htonl (pSmEndDev->startAddr);
            }
        else if (pSmEndDev->ipAddr != 0)
            {
            localAddr.s_addr = htonl (pSmEndDev->ipAddr);
            }
        inet_ntoa_b (localAddr, bpAddr);
        printf ("SM address: %s\n", bpAddr);

        /* at this pointer, we should have IP/netmask for this node */

        pAnchor  = (SM_ANCHOR *)pSmEndDev->pAnchor;
        pSmHdr   = SM_OFFSET_TO_LOCAL (ntohl (pAnchor->smHeader),
                                               (unsigned)pAnchor,
                                               SM_HDR *);
        pCpuDesc = SM_OFFSET_TO_LOCAL (ntohl (pSmHdr->cpuTable),
                                               (unsigned)pAnchor,
                                               SM_CPU_DESC *);
        pCpuDesc = &(pCpuDesc[pSmEndDev->localCpu]);

        pCpuDesc->reserved1 =(unsigned) localAddr.s_addr;
        pCpuDesc->reserved2 = htonl (((unsigned)pSmEndDev->netmask));

        /* if master, store its IP */

        if (pSmEndDev->isMaster)
            {
            pSmPktHdr = SM_OFFSET_TO_LOCAL(ntohl(pAnchor->smPktHeader),
                                           (unsigned)pAnchor,
                                            SM_PKT_MEM_HDR*);
			printf ("NOTE master: pSmPktHdr address is 0x%x\n", pSmPktHdr);
            pSmPktHdr->reserved1 = localAddr.s_addr;
            }              

        pEnd = (END_OBJ *)pSmEndDev;

#ifdef SM_DBG
        smShowDev (pSmEndDev);
#endif /* SM_DBG */
	}
     else
        {
        pEnd = NULL;

        if (isBootDev)
            printf ("SM boot device: %s; Error: init failed\n",
                    pSmEndDev->devName);
        else
            printf ("SM secondary device: %s; Error: init failed\n",
                    pSmEndDev->devName);

#ifdef SM_DBG
        smShowDev (pSmEndDev);
#endif /* SM_DBG */

        if (pSmEndDev->pMemAlloc != NULL)
            cacheDmaFree (pSmEndDev->pMemAlloc);

        cfree ((char*)pSmEndDev);
        }

    SM_LOG (SM_DBG_LOAD, ("smEndLoad: pEnd = %p\n", pEnd));

    return (pEnd);
    }

/******************************************************************************
* smEndInetAddrGet - get the IP address of a SM interface
*
* This routine returns the IP address in <pSmIp> for the CPU
* specified by <cpuNum> on the SM network specified by <pSmName> and 
* <smUnit>. If <cpuNum> is NONE (-1), this routine returns information about 
* the local (calling) CPU.
*
* This routine can only be called after SM is loaded.  It will
* block if the shared memory region has not yet been initialized.
*
* RETURNS: OK, or ERROR if the Internet address cannot be found.
*
* \NOMANUAL
*/

STATUS smEndInetAddrGet
     (
     char * pSmName,     /* SM device name */
     int    smUnit,      /* SM unit number */
     int    cpuNum,      /* CPU number     */
     char * pSmIp        /* return IP address */
     )
     { 
     struct in_addr   localAddr;      /* local IP address */
     SM_ANCHOR      * pAnchor;        /* pointer to ANCHOR */
     SM_CPU_DESC    * pCpuDesc;       /* pointer to cpuDesc Table */
     SM_HDR         * pSmHdr;         /* pointer to SM header */       
     SM_END_DEV     * pSmEndDev;      /* driver structure */
     
     pSmEndDev = (SM_END_DEV *) endFindByName (pSmName, smUnit);
  
     if (pSmEndDev == NULL)
         {
         printf ("Error: smEndInetAddrGet - can not find the valid END driver" 
                  "for %s%d\n", pSmName, smUnit);
         *pSmIp = '\0';
         return (ERROR);
         }

     /* get the SM structure */

     pAnchor  = (SM_ANCHOR *)pSmEndDev->pAnchor;
     pSmHdr   = SM_OFFSET_TO_LOCAL (ntohl (pAnchor->smHeader),
                                           (unsigned)pAnchor,
                                           SM_HDR *);
     pCpuDesc = SM_OFFSET_TO_LOCAL (ntohl (pSmHdr->cpuTable),
                                           (unsigned)pAnchor,
                                             SM_CPU_DESC *);
     if (cpuNum == NONE)
         cpuNum = (int)pSmEndDev->localCpu;

     pCpuDesc = &(pCpuDesc[cpuNum]);

     /* reserved1 stores IP of the corresponding <cpuNum> */

     if (pCpuDesc->reserved1 == 0)
         {
         printf ("Error: smEndInetAddrGet - can not find the valid IP address" 
                  "for SM cpu %d", cpuNum);
         *pSmIp = '\0';
         return (ERROR);
         }

     localAddr.s_addr = (unsigned)pCpuDesc->reserved1;
     inet_ntoa_b (localAddr, pSmIp);

     return OK;
     }     

/******************************************************************************
*
* smEndParse - parse input parameter string and derive parameter values
*
* This routine parses the input parameter string, deriving the values of each
* parameter in the assumed order and radix as shown here:
*
* "<unit>:<pAnchor>:<smAddr>:<memSize>:<tasType>:
*  <maxCpus>:<masterCpu>:<localCpu>:<maxPktBytes>:<maxInputPkts>:
*  <intType>:<intArg1>:<intArg2>:<intArg3>:<mbNum>:<cbNum>:
*  <configFlg>:<pBootParams>"
*
* Each is a string literal representing a string or a number in a predetermined
* radix.  All numbers expected to be in radix 16 start with "0x".  All other
* numbers are expected to be radix 10.
*
* When needed, NONE is -1 in radix 10 and 0xffffffff in radix 16.
*
* Parameter	Radix	Use
* -------------	-----	-------------------------------------------------------
* unitNumber	 10	device unit number assigned by operating system
* pAnchor	 16	SM anchor region virtual address on SM master node if
*			on local bus, else, SM anchor bus address
* smAdrs	 16	Local address of SM
* memSize	 16	total shared memory size in bytes
* tasType	 10	test-and-set type (SM_TAS_HARD or SM_TAS_SOFT)
* maxCpus	 10	maximum number of CPUs supported in sm
*			(0 = default)
* masterCpu	 10	master CPU#
* localCpu	 10	this board's CPU number (NONE = use sysProcNumGet)
* maxPktBytes	 10	maximum number of bytes per shared memory packet
*			(0 = default)
* maxInputPkts	 10	maximum number of queued receive packets for this CPU
*			(0 = default)
* intType	 10	interrupt method (SM_INT_MAILBOX/BUS/NONE/USER)
* intArg1	 16	1st interrupt argument
* intArg2	 16	2nd interrupt argument
* intArg3	 16	3rd interrupt argument
* mbNum		 16	number of mBlks in driver memory pool (if < 16, a
*			default value is used)
* cbNum		 16	number of clBlks in driver memory pool (if < 16, a
*			default value is used)
* configFlg      16     configuration flag from BSP
* pBootParams    16     address of BOOT_PARAMS structure
*
* The parameter values are delimited by colons and the EOS.
*
* Typically this string will contain only the unit number which is sufficient
* to set all default parameters.  However, most default parameters (except
* device name and unit number) can be overridden by placing the new value into
* this string in its assumed location, as shown above.
*
* RETURNS: OK or ERROR if bogus string.
*/

LOCAL STATUS smEndParse
    (
    SM_END_DEV * pSmEndDev,	/* pointer to device descriptor */
    char *       pParamStr	/* pointer to parameter string to parse */
    )
    {
    char *       tok     = NULL;	/* parameter value (token) pointer */
    char *       pHolder = NULL;

    /* derive unit number */

    /* parse the initString */

    /* get the unit */

    tok = strtok_r (pParamStr, ":", &pHolder);
    if (tok == NULL)
        return ERROR;
 
    pSmEndDev->unit = atoi (tok);

    if (pSmEndDev->unit  >= NSM)
         {
         SM_LOG (SM_DBG_LOAD, ("illegal unit number: %d\n",
                 pSmEndDev->unit));
         return (ERROR);
          }

     /* get the anchor */ 

     tok = strtok_r (NULL, ":", &pHolder);
     if (tok == NULL)
        return ERROR;

     pSmEndDev->pAnchor = (SM_ANCHOR *) strtoul (tok, NULL, 16);

     /* get the SM memory address */
 
     tok = strtok_r (NULL, ":", &pHolder);
     if (tok == NULL)
        return ERROR;

     pSmEndDev->pMem = (char *) strtoul (tok, NULL, 16);

     /* get the SM memory size */

     tok = strtok_r (NULL, ":", &pHolder);
     if (tok == NULL)
        return ERROR;

     pSmEndDev->memSize = (UINT32) strtoul (tok, NULL, 16);

    /* derive test-and-set type */

    tok = strtok_r (NULL, ":", &pHolder);
    if (tok == NULL)
        return ERROR;
    pSmEndDev->tasType = atoi (tok);

    /* derive maximum number of CPUs supported in shared memory */

    tok = strtok_r (NULL, ":", &pHolder);
    if (tok == NULL)
        return ERROR;
    pSmEndDev->maxCpus = atoi (tok);

    if (pSmEndDev->maxCpus == 0)
        pSmEndDev->maxCpus = DEFAULT_CPUS_MAX;
    else if (pSmEndDev->maxCpus > SM_REGIONS_MAX)
        pSmEndDev->maxCpus = SM_REGIONS_MAX;

    /* derive master CPU number, only support 0 */

    tok = strtok_r (NULL, ":", &pHolder);
    if (tok == NULL)
        return ERROR;
    pSmEndDev->masterCpu = atoi (tok);

    /* derive this board's CPU number */

    tok = strtok_r (NULL, ":", &pHolder);
    if (tok != NULL)
        pSmEndDev->localCpu = atoi (tok);

    if (pSmEndDev->localCpu == NONE)
        pSmEndDev->localCpu = sysProcNumGet ();

    /* derive maximum shared memory packet size */

    tok = strtok_r (NULL, ":", &pHolder);
    if (tok == NULL)
        return ERROR;
    pSmEndDev->maxPktBytes = atoi (tok);

    if (pSmEndDev->maxPktBytes == 0)
        pSmEndDev->maxPktBytes = DEFAULT_PKT_SIZE;

    /* derive maximum #packets that can be received; must be > 0 */

    tok = strtok_r (NULL, ":", &pHolder);
    if (tok == NULL)
        return ERROR;
    pSmEndDev->maxPackets = atoi (tok);

    if (pSmEndDev->maxPackets == 0)
        pSmEndDev->maxPackets = DEFAULT_PKTS_MAX;

    /* derive interrupt method */

    tok = strtok_r (NULL, ":", &pHolder);
    if (tok == NULL)
        return ERROR;
    
    if ((pSmEndDev->intType = atoi (tok)) > SM_INT_USER_2)
        {
        SM_LOG (SM_DBG_LOAD, ("illegal SM interrupt type: %d\n",
                    pSmEndDev->intType));
        return (ERROR);
        }

    /* derive interrupt argument 1 */

     tok = strtok_r (NULL, ":", &pHolder);
     if (tok == NULL)
        return ERROR;

     pSmEndDev->intArg1 = (ULONG) strtoul (tok, NULL, 16);

    /* derive interrupt argument 2 */

     tok = strtok_r (NULL, ":", &pHolder);
     if (tok == NULL)
        return ERROR;

     pSmEndDev->intArg2 = (ULONG) strtoul (tok, NULL, 16);

    /* derive interrupt argument 3 */
     tok = strtok_r (NULL, ":", &pHolder);
     if (tok == NULL)
        return ERROR;

     pSmEndDev->intArg3 = (ULONG) strtoul (tok, NULL, 16);

    /* derive number of mBlks to allocate */

     tok = strtok_r (NULL, ":", &pHolder);
     if (tok == NULL)
        return ERROR;

     pSmEndDev->mbNum = (uint32_t) strtoul (tok, NULL, 16);

    if (pSmEndDev->mbNum < SM_MIN_MBLK_NUM)
        pSmEndDev->mbNum = SM_DFLT_MBLK_NUM;

    /* derive number of clBlks to allocate */
     tok = strtok_r (NULL, ":", &pHolder);
     if (tok == NULL)
        return ERROR;

     pSmEndDev->cbNum = (uint32_t) strtoul (tok, NULL, 16);
     if (pSmEndDev->cbNum < SM_MIN_CLBLK_NUM)
         pSmEndDev->cbNum = SM_DFLT_CLBLK_NUM;

     /* derive the configuration parameter */

     tok = strtok_r (NULL, ":", &pHolder);
     if (tok == NULL)
        return ERROR;

     pSmEndDev->configFlag = (uint32_t) strtoul (tok, NULL, 16);

     /* derive the pointer to the BOOT_PARAMS structure */

     tok = strtok_r (NULL, ":", &pHolder);
     if (tok == NULL)
        return ERROR;

     pSmEndDev->pBootParams = (BOOT_PARAMS *) strtoul (tok, NULL, 16);

     pSmEndDev->pSmFree  = NULL;   /* future */
     pSmEndDev->smAlloc = FALSE;  /* future */

     return (OK);
     }

#ifdef SM_DBG
/******************************************************************************
*
* smShowDev - show the SM device descriptor structure
*
* This routine shows the specified SM device descriptor structure.  This is
* strictly a debug tool.
*
* RETURNS: N/A
*
* \NOMANU
*/
 
void smShowDev
    (
    SM_END_DEV * pSmEndDev	/* pointer to SM device structure to show */
    )
    {
    printf ("\nSM DEVICE:\n");
    printf ("==========================================================\n");
    printf ("pSmEndDev:    %p\n",   (void *)pSmEndDev);
    printf ("cookie:       %#x\n",  pSmEndDev->cookie);
    printf ("unit:         %#x\n",  pSmEndDev->unit);
    printf ("devName:      %s\n",   pSmEndDev->devName);
    printf ("pAnchor:      %p\n",   pSmEndDev->pAnchor);
    printf ("pMem:         %p\n",   pSmEndDev->pMem);
    printf ("memSize:      %#lx\n", pSmEndDev->memSize);
    printf ("tasType:      %#x\n",  pSmEndDev->tasType);
    printf ("maxCpus:      %#x\n",  pSmEndDev->maxCpus);
    printf ("masterCpu:    %#x\n",  pSmEndDev->masterCpu);
    printf ("localCpu:     %#x\n",  pSmEndDev->localCpu);
    printf ("maxPktBytes:  %#lx\n", pSmEndDev->maxPktBytes);
    printf ("ipAddr:       %#lx\n", pSmEndDev->ipAddr);
    printf ("maxPackets:   %#x\n",  pSmEndDev->maxPackets);
    printf ("intType:      %#x\n",  pSmEndDev->intType);
    printf ("intArg1:      %#x\n",  pSmEndDev->intArg1);
    printf ("intArg2:      %#x\n",  pSmEndDev->intArg2);
    printf ("intArg3:      %#x\n",  pSmEndDev->intArg3);
    printf ("ticksPerBeat: %#x\n",  pSmEndDev->ticksPerBeat);
    printf ("mbNum:        %#lx\n", pSmEndDev->mbNum);
    printf ("cbNum:        %#lx\n", pSmEndDev->cbNum);
    printf ("pSmFree:      %p\n",   pSmEndDev->pSmFree);
    printf ("smAlloc:      %#x\n",  pSmEndDev->smAlloc);
    printf ("isMaster:     %#x\n",  pSmEndDev->isMaster);
    printf ("isBoot:       %#x\n",  pSmEndDev->isBoot);
    printf ("flags:        %#lx\n", pSmEndDev->flags);
    printf ("masterAddr:   %#lx\n\n", pSmEndDev->masterAddr);
    taskDelay (12);
    }

/*********************************************************************
* smDbgEaddrSprintf - convert a MAC address to a string
* 
* This function converts an ethernet address to a printable string
*
* RETURNS: buffer that stores the string 
*
* \NOMANUAL
*/

LOCAL char * smDbgEaddrSprintf
    (
    UCHAR * p
    )
    {
    int ix;
    char * np = smDbgbuf;

    for (ix = 0; ix < 6; ix++) 
        {
        *np++ = smDigits[*p >> 4];
        *np++ = smDigits[*p++ & 0xf];
        *np++ = ':';
        }

    *--np = 0;

    return (smDbgbuf);
    }

#endif /* SM_DBG */

/******************************************************************************
*
* smEndDevInit - initialize SM END network device
*
* This routine finishes the SM END load operation begun by smEndLoad() in the
* configlette.
*
* RETURNS: OK or ERROR.
*
* SEE ALSO: smEndLoad()
*
* \NOMANUAL
*/

STATUS smEndDevInit
    (
    SM_END_DEV *	pSmEndDev	/* device control pointer */
    )
    {
    SM_ANCHOR *		pAnchor;		/* addr of anchor */
    FAST SM_HDR *	pSmHdr;
    SM_PKT_MEM_HDR *	pSmPktHdr;		/* packet header */
    int                 addrReadable = ERROR;	/* remote address visible */
    int                 tics;			/* SM probe delay period */
    int			temp;			/* temp for probing/waiting */
    static int		smEndDevAttr = (IFF_NOTRAILERS | IFF_BROADCAST);
#if FALSE	/* future enhancement */
    static int		smEndDevAttr = (IFF_NOTRAILERS | IFF_BROADCAST |
                                        IFF_MULTICAST);
#endif

    SM_LOG (SM_DBG_LOAD, ("smEndDevInit: pSmEndDev = %#X\n",
            (unsigned)pSmEndDev));

    /* If SM device descriptor pointer is NULL, just return ERROR */

    if (pSmEndDev == NULL)
        {
        SM_LOG (SM_DBG_LOAD,
                ("smEndDevInit: ERROR: no SM END device ptr...\n"));
        return (ERROR);
        }

    /*
     * Finish load operation.  At this point, allocation of SM proper is done.
     * Now perform final setup and any driver working buffer allocation.
     */

    /* Perform driver buffer setup and initialization */

    if (smEndMemInit (pSmEndDev) == ERROR)
        {
        goto smEndLoadFail;
        }

    /*
     * The following code is a hack.  The anchor may not yet even be
     * mapped to the bus.  When probing with the next few lines
     * Bus Errors occur on many boards if the slave beats the master
     * to the pool.  The hack here simply avoids the sequential
     * addressing if the slave gets a BERR looking for the anchor.
     * Therefore, reliable use of sequential addressing can only be
     * guaranteed if the slave is stopped during the booting sequence
     * until the master has fully booted.  Towards this end an initial
     * delay in bus probing is introduced based on processor number.
     * If the first probe is unsuccessful, an exponential increase in
     * delay period is used to reduce bus contention on subsequent
     * probes.  This workaround is no worse than receiving BERRs
     * but does reduce bus contention and the number of BERRs.  The
     * master processor does not delay.
     */

    if (!pSmEndDev->isMaster)
        {
        /* First wait for valid anchor region */

        pAnchor = pSmEndDev->pAnchor;
        tics    = pSmEndDev->localCpu;
        for (tics <<= 1; (tics < SM_MAX_WAIT) && (addrReadable == ERROR);
             tics <<= 1)
            {
            smUtilDelay (NULL, tics);
            if ((addrReadable = smUtilMemProbe ((char *)pAnchor, READ, 4,
                                                (char *)&temp)) != OK)
                {
                printf (".");
                continue;
                }
            if (temp == 0xffffffff)
                {
                printf (",");
                continue;
                }
            break;
            }
        printf ("\n");
        if (tics >= SM_MAX_WAIT)
            {
            printf ("smEndDevInit: Error: SM probe 1 time out\n");
            goto smEndLoadFail;
            }

        /* Now wait for master to attach to SM */

        tics = pSmEndDev->localCpu;
        for (tics <<= 1; tics < SM_MAX_WAIT; tics <<= 1)
            {
            smUtilDelay (NULL, tics);
            if (((temp = ntohl (pAnchor->smPktHeader)) != 0) &&
                (temp != 0xffffffff))
                {
                break;
                }
            }
        if (tics >= SM_MAX_WAIT)
            {
            printf ("smEndDevInit: Error: SM probe 2 time out\n");
            goto smEndLoadFail;
            }

        pSmHdr = SM_OFFSET_TO_LOCAL (ntohl (pAnchor->smHeader),
                                     (unsigned)pAnchor, SM_HDR *);
        pSmPktHdr = SM_OFFSET_TO_LOCAL (ntohl (pAnchor->smPktHeader),
                                        (unsigned)pAnchor, SM_PKT_MEM_HDR*);

        /*
         * Wait for stable heartbeat before continuing.  Number of beats
         * to wait is based on CPU number so that slave startup is
         * staggered.
         */
printf ("NOTE slave 0: pAnchor->smPktHeader value is 0x%x\n", ntohl (pAnchor->smPktHeader));
        /* give the master a minute to boot */
        temp = 0;/*(pSmEndDev->localCpu * 10) + 60;  in seconds */
        if (!smIsAlive (pAnchor, (int*)&pAnchor->smPktHeader, (UINT)pAnchor, temp, 0))
            {
            printf ("smEndDevInit Error: Master heartbeat time out\n");
            goto smEndLoadFail;
            }
#if 1		
		pSmHdr = SM_OFFSET_TO_LOCAL (ntohl (pAnchor->smHeader),
                                     (unsigned)pAnchor, SM_HDR *);
        pSmPktHdr = SM_OFFSET_TO_LOCAL (ntohl (pAnchor->smPktHeader),
                                        (unsigned)pAnchor, SM_PKT_MEM_HDR*);
#endif

        /* If we are using the new method, we can check to see if the master
         * has published its IP address as well. The heartbeat still has to be
         * checked however, since the IP address found could be a stale one.
         */
        if (!SM_END_OLD_MASTER)
            {
            /*
             * Wait for master to publish its IP address. 
             */
printf ("NOTE slave: pAnchor->smPktHeader value is 0x%x\n", ntohl (pAnchor->smPktHeader));
printf ("NOTE slave: pAnchor address is 0x%x\n", (unsigned)pAnchor);
printf ("NOTE slave: pSmPktHdr address is 0x%x\n", pSmPktHdr);
            tics = pSmEndDev->localCpu;
            for (tics <<= 1; tics < SM_MAX_WAIT; tics <<= 1)
                {
                if (((temp = ntohl (pSmPktHdr->reserved1)) == 0) ||
                    (temp == 0xffffffff))
                    smUtilDelay (NULL, tics);
                else
                    {
                    break;
                    }
                }

            if (tics >= SM_MAX_WAIT)
                {
                printf ("smEndDevInit Error: Master IP address time out\n");
                goto smEndLoadFail;
                }
            }

        SM_LOG (SM_DBG_LOAD, ("smEndDevInit: SM probe worked, tics = %u\n",
                tics));

        /*
         * Now that the anchor is visible and initialized, set the
         * address of the start of SM proper.
         */

        pSmEndDev->pMem = (char *)pSmHdr;
        }

    /* configure the device */

    smEndConfig (pSmEndDev, SC_INIT);

    /* Initialize the shared memory descriptor.  */

    smPktInit (&pSmEndDev->smPktDesc,   pSmEndDev->pAnchor,
                pSmEndDev->maxPackets,   pSmEndDev->ticksPerBeat,
                pSmEndDev->intType,      pSmEndDev->intArg1,
                pSmEndDev->intArg2,      pSmEndDev->intArg3);

    pSmEndDev->smPktDesc.hdrLocalAdrs = pSmEndDev->pSmPktHdr;

    /* initialize the END and MIB2 parts of the device structure */

    if ((END_OBJ_INIT (&pSmEndDev->end, NULL,
                       pSmEndDev->devName, pSmEndDev->unit,
                       &smEndFuncTable, "Shared Mem END Driver") != ERROR) &&
        (END_MIB_INIT (&pSmEndDev->end, M2_ifType_ethernet_csmacd,
                       pSmEndDev->enPhyAddr, SM_EADDR_LEN,
                       SM_MTU (pSmEndDev), SM_END_MIB2_SPEED) != ERROR))
        {
        /* set the flags to indicate readiness */

        SM_RCV_TASK_INACTIVE;
        END_OBJ_READY (&pSmEndDev->end, smEndDevAttr);
        pSmEndDev->flags |= S_LOADED;
        SM_LOG (SM_DBG_LOAD, ("Done loading smEnd - flags = %#lx\n",
                pSmEndDev->flags));

        return (OK);
        }

    /* Release allocated memory and other cleaning on load failure */

smEndLoadFail:

    smEndDevFree (pSmEndDev);
    return (ERROR);
    }


/*******************************************************************************
*
* smEndUnload - unload SM device from the system
*
* This routine unloads the device pointed to by <pSmEndDev> from the system.
*
* RETURNS: OK or ERROR.
*
* SEE ALSO: smEndLoad()
*
* \NOMANUAL
*/

STATUS smEndUnload
    (
    void * pObj		/* device control pointer */
    )
    {
    ULONG		flags;
    struct in_addr	localAddr;	/* local IP address */
    SM_END_DEV *	pSmEndDev  = (SM_END_DEV *)pObj;
    char		ipAddr [16] = {0,0,0,0,0,0,0,0};

    SM_LOG (SM_DBG_UNLOAD, ("smEndUnload: start\n"));

    if (pSmEndDev == NULL)
        {
        SM_LOG (SM_DBG_UNLOAD, ("smEndUnload error: NULL device pointer\n"));
	return (ERROR);
        }

    /* been there, done that... */

    if (pSmEndDev->cookie != SM_END_COOKIE)
        {
        SM_LOG (SM_DBG_UNLOAD, ("smEndUnload warning: dev already unloaded\n"));
	return (OK);
        }

#ifdef	SM_DBG
    if (!SM_END_IS_LOADED)
        {
        SM_LOG (SM_DBG_UNLOAD, ("smEndUnload warning: device not loaded\n"));
        }
#endif /* SM_DBG */

    /* stop the SM device if it isn't already */

    flags = pSmEndDev->flags;
    smEndStop (pSmEndDev);
    pSmEndDev->flags = 0;

    /* delete any ARP table entry */

    localAddr.s_addr = htonl (pSmEndDev->ipAddr);
    inet_ntoa_b (localAddr, ipAddr);
    if (SM_END_IS_ARP_PUB (flags))
        {
        arpDelete (ipAddr);
        }

    /* invalidate structure */

    pSmEndDev->cookie = 0;

    /* Release allocated memory and do other cleaning on unload */

    smEndDevFree (pSmEndDev);

    /* Finally, free memory for the device control structure */

    cfree ((char*)pSmEndDev);
    SM_LOG (SM_DBG_UNLOAD, ("smEndUnload: Done\n"));

    return (OK);
    }


/******************************************************************************
*
* smEndConfig - configure the SM interface to the device
*
* Configure the SM interface through controlled state changes.
*
* Changes between interrupt and polled modes are performed regardless of the
* state of the SM semaphore.  The semaphore is not needed while in polled
* mode and will remain in what ever state it was when transitioning back to
* interrupt mode so that normal execution can resume.
*
* RETURNS: OK or ERROR.
*/

LOCAL STATUS smEndConfig
    (
    SM_END_DEV * pSmEndDev,	/* device to be configured */
    STATE_CHANGE sDelta		/* type of configuration change */
    )
    {
    STATUS      result          = OK;

    /* Perform requested state change */

    switch (sDelta)
        {
        case SC_INT2POLL:
            break;	/* no config needed */

        case SC_POLL2INT:
            break;	/* no config needed */

        case SC_INIT:    /* set hardware address */
            smEndHwAddrSet (pSmEndDev);
            break;

        case SC_NONE:
            break;

        default:
            SM_LOG (SM_DBG_CFG, ("smEndConfig: unknown state change %x\n",
                    sDelta));
            errno  = EINVAL;
            result = ERROR;
        }

#if FALSE	/* future enhancement */
    /* set up address filter for multicasting */
	
    if (END_MULTI_LST_CNT(&pSmEndDev->end) > 0)
        {
        smEndAddrFilterSet (pSmEndDev);
        }
#endif

    return (result);
    }


#if FALSE	/* future enhancement */
/******************************************************************************
*
* smEndAddrFilterSet - set the address filter for multicast addresses
*
* This routine goes through all of the multicast addresses on the list
* of addresses (added with the endAddrAdd() routine) and sets the
* device's filter correctly.
*
* RETURNS: N/A.
*
* \NOMANUAL
*/

void smEndAddrFilterSet
    (
    VOID * pObj		/* device to be updated */
    )
    {
    SM_END_DEV *  pSmEndDev = (SM_END_DEV *)pObj; /* device to be updated */
    ETHER_MULTI * pCurr;

    pCurr = END_MULTI_LST_FIRST (&pSmEndDev->end);

    while (pCurr != NULL)
	{
        /* TODO - set up the multicast list */
        
	pCurr = END_MULTI_LST_NEXT(pCurr);
	}
    
    /* TODO - update the device filter list */
    }
#endif


/******************************************************************************
*
* smEndStart - start the device
*
* This function performs the actual startup of the device (smPktAttach and
* interrupt connecting/enabling).
*
* RETURNS: OK or ERROR if unable to connect interrupt or attach to packet
* layer.
*
* ERRNO:
* EINTR - unable to connect interrupt
* EIO   - unable to attach to packet layer (smPktAttach() failure)
*
* SEE ALSO: smEndIoctl
*/

LOCAL STATUS smEndStart
    (
    VOID * pObj		/* device ID */
    )
    {
    STATUS                 result    = OK;
    SM_END_DEV *           pSmEndDev = (SM_END_DEV *)pObj; /* dev to start */
    FAST SM_PKT_CPU_DESC * pCpuDesc  = NULL;
    static BOOL            firstPass = TRUE;

    SM_LOG (SM_DBG_START, ("smEndStart(%p)\n", pObj));

    /* First pass through here heralds startup by boot code */
    printstr("== smEndStart ==\r\n");
    if (firstPass)
        {
        firstPass = FALSE;

        /*
         * If not already connected, connect SM interrupt.
         * NOTE: This is necessary until a functional
         * intDisconnect() or smUtilIntDisconnect() routine
         * becomes available.
         */

/*        if (!connected[pSmEndDev->intType])
            {  */
            if (smUtilIntConnect (LOW_PRIORITY, (FUNCPTR)smEndIsr,
                                  (int)pSmEndDev, pSmEndDev->intType,
                                  pSmEndDev->intArg1, pSmEndDev->intArg2,
                                  pSmEndDev->intArg3) != OK)
                {
                SM_LOG (SM_DBG_START,
                        ("smEndStart: ERROR: can't connect interrupt!\n"));
                errno = EINTR;
                return (ERROR);
                }
            else
                {
                connected[pSmEndDev->intType] = TRUE;
				printstr("smEndStart: interrupt connected\n\r");
                SM_LOG (SM_DBG_START, ("smEndStart: interrupt connected\n"));
                if ((pSmEndDev->intType == SM_INT_NONE) &&
                    (pollTaskId == NONE))
                    {
                    pollTaskId = taskNameToId (SM_POLL_TASK_NAME);
                    }

                /* enable interrupt */

                smUtilIntEnable (pSmEndDev);

                SM_LOG (SM_DBG_START, ("smEndStart: interrupt enabled\n"));
                }
/*            } */

        /* start the device: attach to SM */

        if (smPktAttach (&pSmEndDev->smPktDesc) == ERROR)
            {
            SM_LOG (SM_DBG_START,
                    ("smEndStart: ERROR attaching to SM: 0x%x\n", errno));

            /* if attach fails, disable interrupts and disconnect ISR */

            /*
             *  someday this will have to be a call to a real
             * functional smUtilIntDisable() routine.
             */

            smUtilIntDisable (pSmEndDev);

            /*
             *  someday this will have to be a call to a real
             * functional smUtilIntDisconnect() routine.
             */

            result = smUtilIntDisconnect (LOW_PRIORITY, (FUNCPTR)smEndIsr,
                                          (int)pSmEndDev,
                                          pSmEndDev->intType,
                                          pSmEndDev->intArg1,
                                          pSmEndDev->intArg2,
                                          pSmEndDev->intArg3);
            errno = EIO;
            return (ERROR);
            }

        else
            {
            pCpuDesc = pSmEndDev->smPktDesc.cpuLocalAdrs;
	    pCpuDesc = &(pCpuDesc[pSmEndDev->localCpu]);
            pSmEndDev->pRxPktCnt = (int *)&pCpuDesc->inputList.count;

            /* set attached and running flags */

            SM_END_START;
            SM_LOG (SM_DBG_START, ("smEndStart: attached to SM; \
flags = %#lx; CPU is %s\n", pSmEndDev->flags,
                    ((pSmEndDev->smPktDesc.status) ?
                    "ATTACHED" : "NOT Attached!")));
			if(pSmEndDev->smPktDesc.status)
				printstr ("ATTACHED\r\n");
			else
				printstr ("NOT Attached!\n");
            }
        }

    /* free all queued tuples */

    smEndPollQFree (pSmEndDev);

    return (OK);
    }


/******************************************************************************
*
* smEndStop - stop the device
*
* This function calls the system functions to disconnect interrupts and stop
* the device from operating in interrupt mode.
*
* RETURNS: OK or ERROR.
*/

LOCAL STATUS smEndStop
    (
    VOID * pObj		/* device to be stopped */
    )
    {
    SM_END_DEV * pSmEndDev = (SM_END_DEV *)pObj; /* device to be stopped */
    STATUS       result    = OK;

    SM_LOG (SM_DBG_STOP, ("smEndStop...\n"));

    /* just return OK if already stopped */

    if (!SM_END_IS_STARTED)
        {
        return (result);
        }

    /* stop/disable the device. */

    SM_END_STOP;

    /*
     *  someday this will have to be a call to a real
     * functional smUtilIntDisable() routine.
     */

    smUtilIntDisable (pSmEndDev);

    /*
     *  someday this will have to be a call to a real functional
     * smUtilIntDisconnect() routine.
     */

    if ((result = smUtilIntDisconnect (LOW_PRIORITY, (FUNCPTR)smEndIsr,
                                       (int)pSmEndDev, pSmEndDev->intType,
                                       pSmEndDev->intArg1, pSmEndDev->intArg2,
                                       pSmEndDev->intArg3))
                == ERROR)
	{
	SM_LOG (SM_DBG_STOP, ("smEndStop: interrupt disconnect error!\n"));
	}

    if (pSmEndDev->intType == SM_INT_BUS)
        connected[SM_INT_BUS] = FALSE;
    SM_LOG (SM_DBG_STOP, ("smEndStop: interrupt disconnected\n"));

    /* detach from shared mem */

    if (pSmEndDev->smPktDesc.status == SM_CPU_ATTACHED)
        {
        (void) smPktDetach (&pSmEndDev->smPktDesc, SM_FLUSH);
        }

    return (result);
    }


/******************************************************************************
*
* smEndPollStart - start MUX polled mode operation
*
* This function starts MUX polled mode operation for the shared memory END
* interface.
*
* RETURNS: OK or ERROR.
*/

LOCAL STATUS smEndPollStart
    (
    void * pObj
    )
    {
    SM_END_DEV * pSmEndDev = (SM_END_DEV *)pObj; /* device receiving command */
    STATUS       result = ERROR;

    SM_LOG (SM_DBG_POLL, ("smEndPollStart...\n"));

    /* if SM is already in polled mode, just set END polled flag */

    if (SM_END_IS_ACTIVE)
        {
        if (!MUX_IS_POLLING)
            {
            /* reconfigure device as required */

            smEndConfig (pSmEndDev, SC_INT2POLL);
            }

        pSmEndDev->flags |= (S_POLLED_END | S_POLLED_SM);

        result = OK;
        }
    else
        errno = ENODEV;

    return (result);
    }


/******************************************************************************
*
* smEndPollStop - stop MUX polled mode operation
*
* This function stops MUX polled mode operation for this END and prepares it
* to resume normal operation.
*
* RETURNS: OK or ERROR.
*/

LOCAL STATUS smEndPollStop
    (
    void * pObj
    )
    {
    SM_END_DEV * pSmEndDev = (SM_END_DEV *)pObj; /* device receiving command */
    STATUS       result = ERROR;

    SM_LOG (SM_DBG_POLL, ("smEndPollStop...\n"));

    /* if SM is not in polled mode, just clear END polled flag */

    if (SM_END_IS_ACTIVE)
        {
        if (MUX_IS_POLLING)
            {
            /* reconfigure device as required */

            smEndConfig (pSmEndDev, SC_POLL2INT);
            }

        pSmEndDev->flags &= ~(S_POLLED_END | S_POLLED_SM);

        result = OK;
        }
    else
        errno = ENODEV;

    return (result);
    }


/******************************************************************************
*
* smEndIoctl - SM device I/O control routine
*
* This routine is called to perform control operations on the SM device network
* interface.  The implemented commands are:
*
* Command       | Data              | Function
* --------------|-------------------|-----------------------------------------
* EIOCSADDR     | char *            | set SM device ethernet address
* EIOCGADDR     | char *            | get SM device ethernet address
* EIOCSFLAGS    | int               | set SM device flags
* EIOCGFLAGS    | int               | get SM device flags
* EIOCGMWIDTH   | int *             | get memory width (always 0)
* EIOCMULTIADD  | --                | [not supported]
* EIOCMULTIDEL  | --                | [not supported]
* EIOCMULTIGET  | --                | [not supported]
* EIOCPOLLSTART | N/A               | start polled operation
* EIOCPOLLSTOP  | N/A               | stop polled operation
* EIOCGMIB2     | M2_INTERFACETBL * | return MIB2 information
* EIOCGFBUF     | int               | return minimum First Buffer for chaining
* EIOCGHDRLEN   | int *             | get ether header length
* SMIOCGMCPYRTN | FUNCPTR *         | get mblk copy routine pointer
* SMIOCSMCPYRTN | FUNCPTR           | set mblk copy routine pointer
* SMIOCGCCPYRTN | FUNCPTR *         | get chained mblk copy routine pointer
* SMIOCSCCPYRTN | FUNCPTR           | set chained mblk copy routine pointer
*
* RETURNS: A command specific response, usually OK or an error code:
* EINVAL (invalid argument), ENOTSUP (unsupported Ioctl command), or EIO
* (SM int connect or packet attach failed).
*/

LOCAL int smEndIoctl
    (
    VOID *  pObj,	/* device receiving command */
    UINT32  cmd,	/* ioctl command code */
    caddr_t data	/* command argument */
    )
    {
    SM_END_DEV * pSmEndDev = (SM_END_DEV *)pObj; /* device receiving command */
    int          error     = OK;
    long         value;

    switch (cmd)
        {
        case SMIOCGMCPYRTN:	/* get mblk copy routine pointer */
	    if (data == NULL)
	        {
                SM_LOG (SM_DBG_IOCTL,
                        ("smEndIoctl: SMIOCGMCPYRTN: data = NULL\n"));
		return (EINVAL);
	        }
            SM_LOG (SM_DBG_IOCTL, ("smEndIoctl: SMIOCGMCPYRTN\n"));
            *(FUNCPTR *)data = smEndCopyRtn;
            break;

        case SMIOCSMCPYRTN:	/* set mblk copy routine pointer */
	    if (data == NULL)
	        {
                SM_LOG (SM_DBG_IOCTL,
                        ("smEndIoctl: SMIOCSMCPYRTN: data = NULL\n"));
		return (EINVAL);
	        }
            SM_LOG (SM_DBG_IOCTL, ("smEndIoctl: SMIOCSMCPYRTN\n"));
            smEndCopyRtn = (FUNCPTR)data;
            break;

        case SMIOCGCCPYRTN:	/* get chained mblk copy routine pointer */
	    if (data == NULL)
	        {
                SM_LOG (SM_DBG_IOCTL,
                        ("smEndIoctl: SMIOCGCCPYRTN: data = NULL\n"));
		return (EINVAL);
	        }
            SM_LOG (SM_DBG_IOCTL, ("smEndIoctl: SMIOCGCCPYRTN\n"));
            *(FUNCPTR *)data = smEndMblkCopyRtn;
            break;

        case SMIOCSCCPYRTN:	/* set chained mblk copy routine pointer */
	    if (data == NULL)
	        {
                SM_LOG (SM_DBG_IOCTL,
                        ("smEndIoctl: SMIOCSCCPYRTN: data = NULL\n"));
		return (EINVAL);
	        }
            SM_LOG (SM_DBG_IOCTL, ("smEndIoctl: SMIOCSCCPYRTN\n"));
            smEndMblkCopyRtn = (FUNCPTR)data;
            break;

        case EIOCSADDR:		/* set SM interface address */
	    if (data == NULL)
	        {
                SM_LOG (SM_DBG_IOCTL, ("smEndIoctl: EIOCSADDR: data = NULL\n"));
		return (EINVAL);
	        }
            SM_LOG (SM_DBG_IOCTL, ("smEndIoctl: EIOCSADDR\n"));
            bcopy ((char *)data,
                   (char *)SM_END_ETHER_ADDR_GET (&pSmEndDev->end),
		   SM_END_ETHER_ADDR_LEN_GET (&pSmEndDev->end));
            break;

        case EIOCGADDR:		/* get SM ethernet HW address */
	    if (data == NULL)
	        {
                SM_LOG (SM_DBG_IOCTL, ("smEndIoctl: EIOCGADDR: data = NULL\n"));
		return (EINVAL);
	        }
            bcopy ((char *)SM_END_ETHER_ADDR_GET (&pSmEndDev->end),
                   (char *)data,
		   SM_END_ETHER_ADDR_LEN_GET (&pSmEndDev->end));
            SM_LOG (SM_DBG_IOCTL,
                    ("smEndIoctl: EIOCGADDR:  len = %u, from %#x to %#x\n",
                    (unsigned)SM_END_ETHER_ADDR_LEN_GET (&pSmEndDev->end),
                    (unsigned)SM_END_ETHER_ADDR_GET (&pSmEndDev->end),
                    (unsigned)data));
            SM_LOG (SM_DBG_IOCTL,
                    ("smEndIoctl: MIB2 phys adrs = %u.%u.%u.%u.%u.%u\n",
                    (UINT)pSmEndDev->end.mib2Tbl.ifPhysAddress.phyAddress[0],
                    (UINT)pSmEndDev->end.mib2Tbl.ifPhysAddress.phyAddress[1],
                    (UINT)pSmEndDev->end.mib2Tbl.ifPhysAddress.phyAddress[2],
                    (UINT)pSmEndDev->end.mib2Tbl.ifPhysAddress.phyAddress[3],
                    (UINT)pSmEndDev->end.mib2Tbl.ifPhysAddress.phyAddress[4],
                    (UINT)pSmEndDev->end.mib2Tbl.ifPhysAddress.phyAddress[5]));
            break;

        case EIOCSFLAGS:	/* set SM END device flags */
	    value = (long)data;
            SM_LOG (SM_DBG_IOCTL, ("smEndIoctl: EIOCSFLAGS\n"));
	    if (value < 0)
		{
                value = -value;
                value--;
		END_FLAGS_CLR (&pSmEndDev->end, value);
		}
	    else
		{
		value &= ~IFF_MULTICAST;  /* temporary */
		END_FLAGS_SET (&pSmEndDev->end, value);
		}
	    smEndConfig (pSmEndDev, SC_NONE);
            break;

        case EIOCGFLAGS:	/* get SM END device flags */
            if (data == NULL)
	        {
                SM_LOG (SM_DBG_IOCTL,
                        ("smEndIoctl: EIOCGFLAGS: data = NULL\n"));
                return (EINVAL);
	        }
            SM_LOG (SM_DBG_IOCTL, ("smEndIoctl: EIOCGFLAGS\n"));
	    *(int *)data = END_FLAGS_GET(&pSmEndDev->end);
            break;

        case EIOCGMWIDTH:	/* get SM END memory width (#bytes) */
            if (data == NULL)
	        {
                SM_LOG (SM_DBG_IOCTL,
                        ("smEndIoctl: EIOCGMWIDTH: data = NULL\n"));
                return (EINVAL);
	        }
            SM_LOG (SM_DBG_IOCTL, ("smEndIoctl: EIOCGMWIDTH\n"));
	    *(int *)data = NONE;	/* no restrictions on size */
            break;

        case EIOCMULTIADD:	/* add multicast address (future) */
            if (data == NULL)
	        {
                SM_LOG (SM_DBG_IOCTL,
                        ("smEndIoctl: EIOCMULTIADD: data= NULL\n"));
                return (EINVAL);
	        }
            SM_LOG (SM_DBG_IOCTL, ("smEndIoctl: EIOCMULTIADD\n"));
	    return (ENOTSUP);
/*    return (smEndMCastAddrAdd (pSmEndDev, (char *)data)); */
            break;

        case EIOCMULTIDEL:	/* delete multicast address ( future) */
            if (data == NULL)
	        {
                SM_LOG (SM_DBG_IOCTL,
                        ("smEndIoctl: EIOCMULTIDEL: data= NULL\n"));
                return (EINVAL);
	        }
            SM_LOG (SM_DBG_IOCTL, ("smEndIoctl: EIOCMULTIDEL\n"));
	    return (ENOTSUP);
/*    return (smEndMCastAddrDel (pSmEndDev, (char *)data)); */
            break;

        case EIOCMULTIGET:	/* get multicast list ( future) */
            if (data == NULL)
	        {
                SM_LOG (SM_DBG_IOCTL,
                        ("smEndIoctl: EIOCMULTIGET: data= NULL\n"));
                return (EINVAL);
	        }
            SM_LOG (SM_DBG_IOCTL, ("smEndIoctl: EIOCMULTIGET\n"));
	    return (ENOTSUP);
/*    return (smEndMCastAddrGet (pSmEndDev, (MULTI_TABLE *)data)); */
            break;

	case EIOCPOLLSTART:	/* Begin polled operation */
            SM_LOG (SM_DBG_IOCTL, ("smEndIoctl: EIOCPOLLSTART\n"));
	    smEndPollStart (pSmEndDev);
	    break;

	case EIOCPOLLSTOP:	/* End polled operation */
            SM_LOG (SM_DBG_IOCTL, ("smEndIoctl: EIOCPOLLSTOP\n"));
	    smEndPollStop (pSmEndDev);
	    break;

        case EIOCGMIB2:		/* return MIB information */
            if (data == NULL)
	        {
                SM_LOG (SM_DBG_IOCTL, ("smEndIoctl: EIOCGMIB2: data = NULL\n"));
                return (EINVAL);
	        }
            SM_LOG (SM_DBG_IOCTL, ("smEndIoctl: EIOCGMIB2\n"));
            bcopy((char *)&pSmEndDev->end.mib2Tbl, (char *)data,
                  sizeof(pSmEndDev->end.mib2Tbl));
            break;

        case EIOCGFBUF:		/* return minimum First Buffer for chaining */
            if (data == NULL)
	        {
                SM_LOG (SM_DBG_IOCTL, ("smEndIoctl: EIOCGFBUF: data = NULL\n"));
                return (EINVAL);
	        }
            SM_LOG (SM_DBG_IOCTL, ("smEndIoctl: EIOCGFBUF\n"));
            *(int *)data = SM_END_MIN_FBUF;
            break;

        case EIOCGHDRLEN:	/* return standard ether header length */
            if (data == NULL)
	        {
                SM_LOG (SM_DBG_IOCTL,
                        ("smEndIoctl: EIOCGHDRLEN: data = NULL\n"));
                return (EINVAL);
	        }
            SM_LOG (SM_DBG_IOCTL, ("smEndIoctl: EIOCGHDRLEN\n"));
            *(int *)data = SIZEOF_ETHERHEADER;
            break;

        default:
            SM_LOG (SM_DBG_IOCTL, ("smEndIoctl: unsupported command: 0x%x\n",
                    cmd));
            error = ENOTSUP;
        }

    return (error);
    }


/******************************************************************************
*
* smEndHwAddrSet - set ethernet hardware address
*
* This routine sets the ethernet physical address associated with the SM
* device specified by <pSmEndDev>.
*
* The SM device physical address consists of 48 bits under IPv4, the last 24
* bits assigned node-specific values:
*
* 0x00, 0x02, 0xE2, 0x00, unit, cpu
* 
* wangzx: change define to 0x00,0x02,0xe2,slot,unit,cpu
*
* RETURNS: N/A
*/

LOCAL void smEndHwAddrSet
    (
    SM_END_DEV * pSmEndDev 	/* device for which to set address */
    )
    {
    u_char * 		pEaddr;			/* ethernet address */
    SM_PKT_MEM_HDR *	pSmPktHdr;		/* packet header */
    FAST SM_CPU_DESC *	pMCpuDsc;		/* SM master CPU desc. ptr */
    FAST SM_CPU_DESC *	pCpuDesc;		/* SM local CPU desc. ptr */
    FAST SM_ANCHOR *	pAnchor  = pSmEndDev->pAnchor;
    FAST SM_HDR *	pSmHdr;
    struct in_addr	localAddr;		/* local IP address */
    char		bpAddr [BOOT_DEV_LEN];	/* backplane IP address */

    /*
     * Under IPv4, SM ethernet physical addresses (EUI-48) consist of 48 bits:
     * { 0x00, 0x02, 0xE2, 0x00, unit, cpu }
     *
     * Under IPv6 (EUI-64) the 64-bit format is:
     * { 0x00, 0x02, 0xE2, 0xFF, 0xFE, 0x00, unit, cpu }
     *
     *: FUTURE: The most significant 24 bits are the vendor ID.  Wind
     * River's vendor ID is 0x00 0x40 0x47, not 0x00 0x02 0xE2!  Must be
     * changed in next major release.
     */

    pEaddr = pSmEndDev->enPhyAddr;
    pEaddr [SM_ETHER_ID_FIRST]  = 0x00;
    pEaddr [SM_ETHER_ID_SECOND] = 0x02;
    pEaddr [SM_ETHER_ID_THIRD]  = 0xE2;
    pEaddr [SM_ETHER_USER_OPT]  = 0x00; /*modified by cfg 2011.10.8*/
/*    pEaddr [SM_ETHER_USER_OPT]  = sysReadSlotNum()&0xff;*/
/*  pEaddr [SM_ETHER_USER_OPT]  = pSmEndDev->???;   /@ Future feature */
    pEaddr [SM_ETHER_UNIT]      = pSmEndDev->unit;
    pEaddr [SM_ETHER_CPU]       = pSmEndDev->localCpu;

    /* handle IEEE 802.3 conversion from EUI-48 to EUI-64 */

    /*if (SM_EADDR_LEN > 6)
        {
        pEaddr [SM_ETHER_EUI64_FOURTH] = 0xFF;
        pEaddr [SM_ETHER_EUI64_FIFTH]  = 0xFE;
        }*/ /*comment by wangzx to avoid warning,when use ipv6, uncomment it please!*/

    pSmPktHdr = SM_OFFSET_TO_LOCAL (ntohl (pAnchor->smPktHeader),
                                           (unsigned)pAnchor, SM_PKT_MEM_HDR*);
    pSmHdr = SM_OFFSET_TO_LOCAL (ntohl (pAnchor->smHeader), (unsigned)pAnchor,
                                 SM_HDR *);
    pMCpuDsc = SM_OFFSET_TO_LOCAL (ntohl (pSmHdr->cpuTable), (unsigned)pAnchor,
                                   SM_CPU_DESC *);

    pSmEndDev->pSmPktHdr  = pSmPktHdr;
    pSmEndDev->maxPackets = ntohl (pSmPktHdr->freeList.limit);
    pCpuDesc = &(pMCpuDsc[pSmEndDev->localCpu]);
    pMCpuDsc = &(pMCpuDsc[pSmEndDev->masterCpu]);

    /*
     * If slave node and sequential address mode, determine network address
     * and netmask for information only.
     */

    if (pSmEndDev->startAddr == 0xfaceface)
        {
        /* base slave network address/netmask on master's address/netmask */

        pSmEndDev->startAddr = ntohl (pSmPktHdr->reserved1);
	pSmEndDev->startAddr  += (ULONG)pSmEndDev->localCpu;
        pSmEndDev->ipAddr = pSmEndDev->startAddr;
        pSmEndDev->netmask = ntohl (pMCpuDsc->reserved2);

        /*
         * If booting this board over SM, set boot parameter backplane
         * network address and netmask.
         */

        if (pSmEndDev->isBoot)
            {
            localAddr.s_addr = htonl (pSmEndDev->ipAddr);
            inet_ntoa_b (localAddr, bpAddr);
            }
        }

    /* record master network address */

    if (!pSmEndDev->isMaster)
        {
        pSmEndDev->masterAddr = ntohl (pMCpuDsc->reserved1);
printf("smEndHwAddrSet: masterAddr IP adrs = %#lx\n", pSmEndDev->masterAddr);
        SM_LOG (SM_DBG_CFG, ("smEndHwAddrSet: masterAddr IP adrs = %#lx\n",
                pSmEndDev->masterAddr));
        }

    /*
     * If the IP address for this CPU is known at this time, store it in
     * the CPU descriptor for the local CPU in shared memory for all to see.
     * Also store subnet mask if it is known.
     */

    if (pSmEndDev->ipAddr != 0)
        {
        pSmEndDev->startAddr = pSmEndDev->ipAddr;
        pCpuDesc->reserved1 = htonl ((unsigned)pSmEndDev->ipAddr);
        if (pSmEndDev->netmask != 0)
            {
            pCpuDesc->reserved2 = htonl (((unsigned)pSmEndDev->netmask));
            }
        localAddr.s_addr = pCpuDesc->reserved1;  /* flush SM writes */
printf("smEndHwAddrSet: rsvrd1 (IP address) = %#x\n", ntohl (pCpuDesc->reserved1));
printf("smEndHwAddrSet: rsvrd2 (subnet mask) = %#x\n", ntohl (pCpuDesc->reserved2));
        SM_LOG (SM_DBG_CFG, ("smEndHwAddrSet: rsvrd1 (IP address) = %#x\n",
                ntohl (pCpuDesc->reserved1)));
        SM_LOG (SM_DBG_CFG, ("smEndHwAddrSet: rsvrd2 (subnet mask) = %#x\n",
                ntohl (pCpuDesc->reserved2)));
        SM_LOG (SM_DBG_CFG, ("smEndHwAddrSet: ethernet address = %s\n",
                smDbgEaddrSprintf (pEaddr)));
        }
    }


/******************************************************************************
*
* smEndDevFree - free memory allocated for an SM device
*
* This routine frees allocated memory for the specified SM device and detaches
* from the SM region.  The goal is to 'clean the slate' so that the SM END
* device load function can be called again.
*
* RETURNS: N/A
*/

LOCAL void smEndDevFree
    (
    SM_END_DEV * pSmEndDev	/* SM device descriptor */
    )
    {
    unsigned         i;
    unsigned         cnt;
    SM_PKT_MEM_HDR * pSmPktHdr  = NULL;	/* packet header */

    /* free allocated memory as necessary */

    if (pSmEndDev->tupleId != NULL)
        {
        netMblkClChainFree (pSmEndDev->tupleId);
        pSmEndDev->tupleId = NULL;
        }

    if (pSmEndDev->end.pNetPool != NULL)
        {
        netPoolDelete (pSmEndDev->end.pNetPool);
        pSmEndDev->end.pNetPool = NULL;
        }

    if (pSmEndDev->pClustMem != NULL)
        {
        cacheDmaFree (pSmEndDev->pClustMem);
        pSmEndDev->pClustMem = NULL;
        }

    if (pSmEndDev->pMclBlkCfg != NULL)
        {
        free (pSmEndDev->pMclBlkCfg);
        pSmEndDev->pMclBlkCfg = NULL;
        }

    if (pSmEndDev->end.pNetPool != NULL)
        {
        free (pSmEndDev->end.pNetPool);
        pSmEndDev->end.pNetPool = NULL;
        }

    /*
     * If this is the master CPU, delete heartbeat watchdog timer
     * and free shared memory if it was allocated.
     */

    if (pSmEndDev->isMaster)
        {
        pSmPktHdr = SM_OFFSET_TO_LOCAL (ntohl(pSmEndDev->pAnchor->smPktHeader),
                                        (int)pSmEndDev->pAnchor,
                                        SM_PKT_MEM_HDR *);
        if (pSmPktHdr->reserved2 != 0)
            {
            wdDelete ((WDOG_ID) ntohl (pSmPktHdr->reserved2));
            pSmPktHdr->reserved2 = 0;
            }

        if (pSmEndDev->smAlloc && (pSmEndDev->pSmFree != NULL))
            {
            /* use indicated method of freeing SM */

            (pSmEndDev->pSmFree) (pSmEndDev);
            pSmEndDev->smAlloc = FALSE;
            }

        if (pSmEndDev->pMemAlloc != NULL)
	    {
            cacheDmaFree (pSmEndDev->pMemAlloc);
            pSmEndDev->pMemAlloc = NULL;
            }
        }

    /* if this is the last entry in the unit table, free the table */

    if (unitTbl != NULL)
        {
        for (i = 0, cnt = 0; i < NSM; ++i)
            {
            if (unitTbl[i] == pSmEndDev)
                unitTbl[i] = NULL;
            else if (unitTbl[i] != NULL)
                ++cnt;
            }
        if (cnt == 0)
            {
            free (unitTbl);
            unitTbl = NULL;
            }
        }
    }


/******************************************************************************
*
* smEndMemInit - set up and initialize memory for an SM device
*
* This routine sets up and initializes a net pool for the specified SM device.
*
* RETURNS: OK or ERROR if failure.
*/

LOCAL STATUS smEndMemInit
    (
    SM_END_DEV * pSmEndDev	/* SM device descriptor */
    )
    {
    SM_HDR *         pSmHdr      = NULL;	/* SM header */
    SM_PKT_MEM_HDR * pSmPktHdr   = NULL;	/* packet header */
    WDOG_ID          smEndBeatWd = NULL;	/* heartbeat watchdog ID */
    UINT             smPktSize   = (UINT)pSmEndDev->maxPktBytes;
    M_CL_CONFIG      smEndMclBlkConfig = /* network mBlk/clBlk config table */
        {
        /* 
        no. mBlks		no. clBlks	memArea		memSize
        -----------		----------	-------		-------
        */
        0, 			0, 		NULL, 		0
        };
    CL_DESC          smEndClDescTbl [] = /* network cluster pool config table*/
        {
        /* 
        clSize			num		memArea		memSize
        -----------		----		-------		-------
        */
        {0,			0,		NULL,		0}
        }; 
    int              smEndClDescTblNumEnt = (NELEMENTS(smEndClDescTbl));

    /* If this is the master CPU, setup and initialize shared memory */

    if (pSmEndDev->isMaster)
        {
        SM_LOG (SM_DBG_MEM_INIT, ("MASTER: shared memory setup\n"));
        SM_LOG (SM_DBG_MEM_INIT, ("MASTER: SM packet size = 0x%8.8X\n",
                smPktSize));

        if ((smPktSetup (pSmEndDev->pAnchor, pSmEndDev->pMem,
                         pSmEndDev->memSize, pSmEndDev->tasType,
                         pSmEndDev->maxCpus, smPktSize +
                         SM_PACKET_ROUNDUP (smPktSize)) == OK) &&
            ((smEndBeatWd = wdCreate ()) != NULL))
            {
            pSmPktHdr = SM_OFFSET_TO_LOCAL (ntohl (pSmEndDev->pAnchor->
                                                   smPktHeader),
                                                   (int)pSmEndDev->pAnchor,
                                                   SM_PKT_MEM_HDR *);

            /* pSmPktHdr->reserved1 set in usrNetSmEndSecBoot.c */

            pSmPktHdr->reserved2  = htonl ((unsigned)smEndBeatWd);
            pSmEndDev->pSmPktHdr  = pSmPktHdr;
            pSmEndDev->maxPackets = ntohl (pSmPktHdr->freeList.limit);
            pSmHdr = SM_OFFSET_TO_LOCAL (ntohl (pSmEndDev->pAnchor->
                                                smHeader),
                                                (int)pSmEndDev->pAnchor,
                                                SM_HDR *);
            pSmHdr->reserved1     = htonl (pSmEndDev->memSize);
            SM_LOG (SM_DBG_MEM_INIT,
                    ("MASTER: pSmPktHdr = %p; rsvd1 = %#x; rsvd2 = %#x\n",
                    pSmPktHdr, ntohl (pSmPktHdr->reserved1),
                    ntohl (pSmPktHdr->reserved2)));
            SM_LOG (SM_DBG_MEM_INIT,
                    ("MASTER: pSmHdr = %p; rsvd1 = %#x\n",
                    pSmHdr, ntohl (pSmHdr->reserved1)));

            /* start heartbeat */

            SM_LOG (SM_DBG_MEM_INIT, ("MASTER: starting heartbeat\n"));
            smEndPulse (pSmPktHdr);
            }
        else
            {
            return (ERROR);
            }
        }

    /* allocate and set up END netPool using netBufLib() */
    
    if ((pSmEndDev->end.pNetPool = (NET_POOL_ID)calloc (1, sizeof(NET_POOL)))
                                 == NULL)
        {
        SM_LOG (SM_DBG_MEM_INIT, ("smEndMemInit: net pool allocation error\n"));
        return (ERROR);
        }

    smEndMclBlkConfig.mBlkNum  = pSmEndDev->mbNum;
    smEndMclBlkConfig.clBlkNum = pSmEndDev->cbNum;
    smEndClDescTbl[0].clNum    = pSmEndDev->cbNum;

    /* Calculate the total memory for all the MBlks and ClBlks. */

    smEndMclBlkConfig.memSize = (smEndMclBlkConfig.mBlkNum *
                                 (M_BLK_SZ + M_BLK_OVERHEAD)) +
                                 (smEndMclBlkConfig.clBlkNum * 
                                 (CL_BLK_SZ + CL_BLK_OVERHEAD));

    /* allocate aligned memory for MBlks and ClBlks */

    pSmEndDev->pMclBlkCfg = memalign (SM_ALIGN_BOUNDARY,
                                      smEndMclBlkConfig.memSize);
    
    SM_LOG (SM_DBG_MEM_INIT, ("smEndMemInit: mBlk/clBlk mem size = 0x%8.8X=\n",
            (UINT)smEndMclBlkConfig.memSize));
    SM_LOG (SM_DBG_MEM_INIT, ("  0x%8.8X mBlks, each 0x%8.8X bytes\n",
            (UINT)smEndMclBlkConfig.mBlkNum, (M_BLK_SZ + M_BLK_OVERHEAD)));
    SM_LOG (SM_DBG_MEM_INIT, ("  0x%8.8X clBlks, each 0x%8.8X bytes\n",
            (UINT)smEndMclBlkConfig.clBlkNum, (CL_BLK_SZ + CL_BLK_OVERHEAD)));
    SM_LOG (SM_DBG_MEM_INIT, ("smEndMemInit: mBlk/clBlk mem area = 0x%8.8X\n",
            (UINT)pSmEndDev->pMclBlkCfg));

    if ((smEndMclBlkConfig.memArea = (char *)pSmEndDev->pMclBlkCfg) == NULL)
        {
        SM_LOG (SM_DBG_MEM_INIT,
                ("smEndMemInit: mBlk/clBlk allocation error\n"));
        return (ERROR);
        }
    
    /* Calculate the memory size of all the clusters. */

    smEndClDescTbl[0].clSize  = smPktSize;
    smEndClDescTbl[0].memSize = smEndClDescTbl[0].clNum *
                                (smPktSize + CL_OVERHEAD);

    /* allocate the memory for the clusters from cache safe memory. */

    pSmEndDev->pClustMem = cacheDmaMalloc (smEndClDescTbl[0].memSize);

    SM_LOG (SM_DBG_MEM_INIT, ("smEndMemInit: cluster mem size = 0x%8.8X =\n",
            (UINT)smEndClDescTbl[0].memSize));
    SM_LOG (SM_DBG_MEM_INIT, ("  0x%8.8X clusters, each 0x%8.8X bytes\n",
            (UINT)smEndClDescTbl[0].clNum, smPktSize));
    SM_LOG (SM_DBG_MEM_INIT, ("smEndMemInit: cluster mem area = 0x%8.8X\n",
            (UINT)pSmEndDev->pClustMem));

    if ((smEndClDescTbl[0].memArea = (char *)pSmEndDev->pClustMem) == NULL)
        {
        SM_LOG (SM_DBG_MEM_INIT,
                ("smEndMemInit: insufficient system memory\n"));
        return (ERROR);
        }

    /* Initialize the memory pool. */

    if (netPoolInit (pSmEndDev->end.pNetPool, &smEndMclBlkConfig,
                     &smEndClDescTbl[0], smEndClDescTblNumEnt, NULL) == ERROR)
        {
        SM_LOG (SM_DBG_MEM_INIT, ("smEndMemInit: Can't init net pool: 0x%x\n",
                errno));
        return (ERROR);
        }

    /* Get our pool ID for future reference */

    if ((pSmEndDev->clPoolId = netClPoolIdGet (pSmEndDev->end.pNetPool,
                                               smPktSize, FALSE)) == NULL)
        {
        SM_LOG (SM_DBG_MEM_INIT, ("smEndMemInit: Can't get cluster pool ID\n"));
        return (ERROR);
        }

    /*
     * Prime the tuple pump
     * Try to always have one pre-allocated tuple available for fast receiving.
     */

    if ((pSmEndDev->tupleId = netTupleGet (pSmEndDev->end.pNetPool,
                                          smPktSize, M_DONTWAIT,
                                          MT_DATA, FALSE))
                           == NULL)
        {
        SM_LOG (SM_DBG_MEM_INIT, ("smEndMemInit: Tuple get failed: 0x%x\n",
                errno));
        return (ERROR);
        }

    SM_LOG (SM_DBG_MEM_INIT, ("Memory setup complete\n"));

    return (OK);
    }


/******************************************************************************
*
* smEndIsr - shared memory interrupt service routine
*
* This routine is the interrupt service routine for shared memory input
* packets.  It processes the interrupt (if appropriate) then notifies the
* MUX of the incoming packet by calling the MUX receive routine.  It is
* enabled/disabled by setting/clearing the S_RUNNING flag bit.
*
* NOTE: Because this ISR may be used in a system without chaining of
* interrupts, and there can be two or more SM subnets using the same type of
* SM interrupt, all shared memory subnets are serviced here for each
* interrupt received.  This is NOT optimal and does waste some time but is
* required due to the lack of guaranteed interrupt chaining.
*
* In the future, if interrupt chaining becomes a guaranteed feature for
* all SM interrupt types, the for loop and unitTbl reference can be safely
* removed.
*
* RETURNS: N/A
*/

LOCAL void smEndIsr
    (
    SM_END_DEV * pSmEndDev
    )
    {
    unsigned i;
/*logMsg("========> smEndIsr!!\r\n",0,0,0,0,0,0);*/
    for (i = 0; i < NSM; ++i)
        {
        if ((pSmEndDev = unitTbl[i]) != NULL)
	    {
	    if(SM_END_INTR_RDY)
		{
		if (ntohl (*pSmEndDev->pRxPktCnt) > 0)
		    {
		    SM_RCV_TASK_ACTIVE;
		    if (netJobAdd ((FUNCPTR)smEndSrvcRcvInt, (int)pSmEndDev,
				   0, 0, 0, 0) != OK)
			{
			SM_RCV_TASK_INACTIVE;
			}
		    }

		} /* SM_INT_RDY */
            /* if bus interrupt, acknowledge it */

            /* defect 40943: for iteration 0, the ack will be performed in
             * smUtilIntRoutine(). Since smUtilIntRoutine is only aware of one
             * possible bus, the other devices' interrupt must be acknowledged
             * here.
             */
            if (i != 0 && pSmEndDev->intType == SM_INT_BUS)
                {
                sysBusIntAck (pSmEndDev->intArg1);
                }

            ++pSmEndDev->rxInts;
	    } /* sm unit there */
        } /* sm unit loop */

    }


/******************************************************************************
*
* smEndPulse - continually pulse the shared memory heartbeat
*
* This routine maintains the shared memory heart beat by incrementing 
* the heartbeat count and then re-scheduling itself to run again 
* after a "beat" has passed.
*
* RETURNS: N/A
*/

LOCAL void smEndPulse
    (
    SM_PKT_MEM_HDR * pSmPktHdr	/* pointer to heartbeat */
    )
    {
    smPktBeat (pSmPktHdr);
    (void) wdStart ((WDOG_ID)ntohl (pSmPktHdr->reserved2),
                   (int)sysClkRateGet (), (FUNCPTR)smEndPulse, (int)pSmPktHdr);
    }


/******************************************************************************
*
* smEndRecv - process the next incoming packet
*
* Handle one incoming packet.  The packet is checked for errors.
*
* RETURNS: OK or ERROR.
*
* \INTERNAL: This routine can only receive a single frame (chain of mBlks).
* If support for multi-frame receiving is implemented in the future, this
* routine must make use of the as yet undefined routine netMblkFromBufCopy().
*/

LOCAL STATUS smEndRecv
    (
    SM_END_DEV * pSmEndDev,	/* device structure */
    SM_PKT *     pPkt,		/* packet to process */
    M_BLK_ID     pMblk		/* mBlk to hold packet data */
    )
    {
    int                   len;
    struct ether_header * pEh    = (struct ether_header *)(&pPkt->data[0]);
    u_char *              pData;		/* packet data to process */

    /* Packet must be checked for errors. */

    if ((len = SM_END_PKT_LEN_GET (pPkt)) < SIZEOF_ETHERHEADER)
        {
        SM_LOG (SM_DBG_RX, ("smEndRecv: invalid packet len %d\n", len));
        smPktFreePut (&pSmEndDev->smPktDesc, pPkt);
        END_ERR_ADD (&pSmEndDev->end, MIB2_IN_ERRS, +1);
        return (ERROR);		/* invalid packet size */
        }

    /* debugging aid */
    SM_LOG (SM_DBG_RX, ("smEndRecv SM packet: [0x%x] len: %d src:%s\n",
            ntohs (pEh->ether_type), len,
            smDbgEaddrSprintf (etherAdrsPtr (pEh->ether_shost))));
    SM_LOG (SM_DBG_RX, ("  dst:%s\n",
            smDbgEaddrSprintf (etherAdrsPtr (pEh->ether_dhost))));

    /* if packet with no data, just return */

    if (len != 0)
        {
        /* copy packet from SM to cluster */

        pData = (u_char *)pEh;

	/* 
	 * This following line solves alignment problem when the CPU 
	 * does not accept longword unaligned addresses.
	 *
	 * Pb: When the ethernet chip receives a packet from the network,
	 * it needs a longword aligned buffer to copy the data. To 
	 * process the IP packet, MUX layer adds a SIZEOF_ETHERHEADER 
	 * (0x14) offset to the data buffer. So the CPU obtains a longword
	 * unaligned buffer and a fault exception occurs when it reads 
	 * "ip_v" in the IP structure (ipintr() routine).
	 *
	 * The problem is solved adding an offset (+2) to the data.
	 */  

	pMblk->mBlkHdr.mData += SM_ALIGN_OFFSET;
        smEndCopyRtn (pData, pMblk->mBlkHdr.mData, len);

        /* set up mBlk */

        pMblk->mBlkHdr.mLen    = len;
        pMblk->mBlkHdr.mFlags |= M_PKTHDR;
        pMblk->mBlkPktHdr.len  = len;

        /* Add one to transmit packet count. */

        pEh = (struct ether_header *)pMblk->mBlkHdr.mData;
        if (bcmp ((caddr_t) etherAdrsPtr (pEh->ether_dhost),
                  (caddr_t) etherAdrsPtr (etherbroadcastaddr),
                  sizeof (pEh->ether_dhost)) == 0)
            pSmEndDev->end.mib2Tbl.ifInNUcastPkts++;
        else
            END_ERR_ADD (&pSmEndDev->end, MIB2_IN_UCAST, +1);

        /* debugging aid */

        SM_LOG (SM_DBG_RX, ("smEndRecv Mblk: [0x%x] mBlkHdr.mLen: %u src:%s\n",
                ntohs (pEh->ether_type), pMblk->mBlkHdr.mLen,
                smDbgEaddrSprintf (etherAdrsPtr (pEh->ether_shost))));
        SM_LOG (SM_DBG_RX, ("  dst:%s, mBlkPktHdr.len = %u, flags = %#x\n",
                smDbgEaddrSprintf (etherAdrsPtr (pEh->ether_dhost)),
                pMblk->mBlkPktHdr.len, pMblk->mBlkHdr.mFlags));

        /* Call the upper layer's receive routine. */

        SM_LOG (SM_DBG_RX, ("smEndRecv: calling MUX\n"));
        END_RCV_RTN_CALL (&pSmEndDev->end, pMblk);

        }

    /* free the SM packet */

    smPktFreePut (&pSmEndDev->smPktDesc, pPkt);

    return (OK);
    }


/******************************************************************************
*
* smEndSrvcRcvInt - task level interrupt service for receiving SM packets
*
* This routine is called at task level indirectly by the interrupt service
* routine to do any SM receive packet processing.
*
* The double loop is to protect against a race condition where the interrupt
* code sees rxHandling as TRUE, but it is then turned off by task code.
* This race is not fatal, but does cause occasional delays until a second
* packet is received and then triggers the netTask to call this routine again.
*
* RETURNS: N/A
*
* \INTERNAL
* Fix for SPR 70181 involved allowing SM packets to be lost when the stack
* mBlk pool resources are exhausted.  This is how other ethernet drivers
* handle resource starvation and allows the SM interrupt mechanism to re-arm.
* Just letting a SM packet sit in the incoming queue will disable the
* interrupt generation logic in smPktLib, causing all SM communications to
* ultimately halt due to lack of packet consumption.
*/

LOCAL void smEndSrvcRcvInt
    (
    SM_END_DEV * pSmEndDev	/* interrupting device */
    )
    {
    SM_PKT *    pPkt;		/* shared memory packet */
    M_BLK_ID    pMblk;

    while ((pSmEndDev->smPktDesc.status == SM_CPU_ATTACHED) &&
           (ntohl (*pSmEndDev->pRxPktCnt) > 0))
        {
        /* take receipt of next incoming SM packet */

    	if (smPktRecv (&pSmEndDev->smPktDesc, &pPkt) != OK)
	    {
            END_ERR_ADD (&pSmEndDev->end, MIB2_IN_ERRS, +1);
            SM_LOG (SM_DBG_INT, ("smEndSrvcRcvInt: packet receive error\n"));
            pPkt = NULL;
	    }

        /*
         * If there is at least one more free tuple, get it and proceed.
         * If not, consume the SM packet to free up SM resources.
         */

        else if (pSmEndDev->tupleId == NULL)
            {
            if ((pSmEndDev->tupleId = netTupleGet (pSmEndDev->end.pNetPool,
                                                   pSmEndDev->maxPktBytes,
                                                   M_DONTWAIT, MT_DATA, FALSE))
                                    == NULL)
    	        {
    	        /* just consume SM packet if no stack resources */

                smPktFreePut (&pSmEndDev->smPktDesc, pPkt);
                pPkt = NULL;

                pSmEndDev->lastError.errCode = END_ERR_NO_BUF;
                muxError (&pSmEndDev->end, &pSmEndDev->lastError);
                END_ERR_ADD (&pSmEndDev->end, MIB2_IN_ERRS, +1);
                SM_LOG (SM_DBG_INT, ("smEndSrvcRcvInt: no free tuples\n"));
    	        }
            }

        if (pPkt != NULL)
    	    {
            pMblk = pSmEndDev->tupleId;
            pSmEndDev->tupleId = NULL;
            smEndRecv (pSmEndDev, pPkt, pMblk);
    	    }
        }
    SM_RCV_TASK_INACTIVE;

    /* replenish fast tuple */

    if (pSmEndDev->tupleId == NULL)
        {
        pSmEndDev->tupleId = netTupleGet (pSmEndDev->end.pNetPool,
                                          pSmEndDev->maxPktBytes,
                                          M_DONTWAIT, MT_DATA, FALSE);
        }
    }


/******************************************************************************
*
* smEndMblkWalk - walk and gather information on an mBlk assembly
*
* An mBlk assembly is composed of one or more frames.  Each frame is constructed
* from a chain of one or more mBlk-clBlk-cluster (tuple) sets.
*
* This routine walks all tuple chains (frames) headed by <pMblk>, computes the
* number of frames it is made of and the maximum data size of all frames, and
* returns this information to the parameters referenced by <pFramNum> and
* <pMaxSize>.  In addition, it finds out whether the specified mBlk assembly
* is unicast or multicast, and sets <pPktType> accordingly.
*
* No frame size is permitted to exceed the size of a shared memory packet.  If
* a frame is found which exceeds the SM packet size, ERROR is returned.
*
* RETURNS: OK, or ERROR in case of invalid mBlk.
*
* SEE ALSO: if_ether.h
*/

LOCAL STATUS smEndMblkWalk
    (
    ULONG    maxPktSiz,	/* max SM packet size (bytes) */
    M_BLK *  pMblk,	/* pointer to the mBlk */
    UINT  *  pFramNum,	/* number of frames */
    UINT16 * pPktType,	/* packet type */
    UINT *   pMaxSize	/* size of largest frame */
    )
    {
    STATUS	    result = OK;
    M_BLK *         pCurr = pMblk;	/* the current mBlk in a frame */
    M_BLK *         pFram = pMblk;	/* the 1st mBlk of current frame */
    UINT            tuples;

    if (pMblk == NULL)
        {
        SM_LOG (SM_DBG_TX, ("smEndMblkWalk: invalid pMblk\n"));
        return (ERROR);
        }
    SM_LOG (SM_DBG_TX,
            ("smEndMblkWalk: pMblk = %#x; pMblk->mBlkHdr.mNext = %#x\n",
            (unsigned)pMblk, (unsigned)pMblk->mBlkHdr.mNext));
    SM_LOG (SM_DBG_TX,
            ("    pMblk data ptr = %#x; pMblk data len = %#x\n",
            (unsigned)pMblk->mBlkHdr.mData, (unsigned)pMblk->mBlkHdr.mLen));
#ifdef	SM_DBG
    {
    char temp[512] = {0,0,0,0,0,0,0,0};
    char temp2 [8];
    int  i;

    for (i = 0; (i < 255) && (i < pMblk->mBlkHdr.mLen); ++i)
        {
        sprintf (temp2, "%2.2x", (unsigned)pMblk->mBlkHdr.mData[i]);
        strcat (temp, temp2);
        }
    SM_LOG (SM_DBG_TX, ("1st pMblk data = %s\n", temp));
    taskDelay (60);
    }
#endif /* SM_DBG */

    /* walk this mBlk */

    *pMaxSize = 0;
    *pFramNum = 0;
    tuples    = 0;
    while (pFram != NULL)
        {
        *pFramNum += 1;
        for (pCurr = pFram; pCurr != NULL;
             pCurr = ((M_BLK *) pCurr->mBlkHdr.mNext))
            {
            /* keep track of the number of tuples in all frames */

            tuples += 1;
            }

        /* record and validate largest frame size */

        if (pFram->mBlkPktHdr.len > *pMaxSize)
            *pMaxSize = pFram->mBlkPktHdr.len;
        if (*pMaxSize > maxPktSiz)
            {
            result = ERROR;
            SM_LOG (SM_DBG_TX,
                    ("smEndMblkWalk: frame size (%#x) > max allowed (%#lx)\n",
                    (unsigned)*pMaxSize, maxPktSiz));
            }

        pFram = ((M_BLK *) pFram->mBlkHdr.mNextPkt);
	}

    /* set the packet type to broad/multicast or unicast */

    if (pMblk->mBlkHdr.mData[0] & (UINT8) 0x01)
	{
	(*pPktType) = PKT_TYPE_MULTI;
        SM_LOG (SM_DBG_TX, ("smEndMblkWalk: multicast request\n"));
	}
    else
	{
	(*pPktType) = PKT_TYPE_UNI;
        SM_LOG (SM_DBG_TX, ("smEndMblkWalk: unicast request\n"));
	}
    SM_LOG (SM_DBG_TX, ("smEndMblkWalk: walked %d tuple(s) in %d frame(s)\n",
            tuples, (unsigned)*pFramNum));
    SM_LOG (SM_DBG_TX, ("               largest frame = %#x bytes\n",
            (unsigned)*pMaxSize));

    return (result);
    }


/*******************************************************************************
*
* smEndSend - SM END send routine
*
* This routine accepts an M_BLK_ID and sends the data in it to the appropriate
* shared memory area.  The data may be in a chain of mBlks.  In this case, the
* data from all mBlks is concatenated into one SM packet for the destination
* CPU being addressed.  If the specified mBlk assembly has more than one chain
* (frame), one SM packet is used for each.
*
* The buffer must already have the addressing information properly installed
* in it.  This is done by a higher layer.
*
* RETURNS: OK or END_ERROR_BLOCK if we are in polled mode or insufficient
* SM buffer resources are available.
*
* ERRNO
*   EINVAL - Invalid mBlk.
*   ENOSPC - Insufficient SM packet resources for all data.
*/

LOCAL STATUS smEndSend
    (
    void *    pObj,	/* device ptr */
    M_BLK_ID  mBlkId	/* ID of mBlk (data) to send */
    )
    {
    SM_END_DEV *          pSmEndDev = (SM_END_DEV *)pObj; /* device ptr */
    M_BLK *               pMblk     = mBlkId;
    M_BLK *               pNxtMblk;
    UINT                  framNum;		/* number of frames */
    UINT16                pktType;		/* packet type */
    UINT                  framSizeMax;		/* max frame size (bytes) */
    unsigned		  destCPU;		/* destination cpu */
    struct ether_header * pEh      = NULL;	/* enet header */
    u_char *              pEaddr;		/* enet address ptr */
    SM_PKT *              pPkt     = NULL;	/* packet pointer */
    SM_PKT_INFO           smInfo;		/* SM info */
    
    SM_LOG (SM_DBG_TX, ("smEndSend...\n"));

    /*
     * Obtain exclusive access to shared memory.  This is necessary because
     * we might have more than one stack transmitting at once.
     */

    if (!(pSmEndDev->flags & SM_END_POLL_SM_RDY))
        {
        END_TX_SEM_TAKE (&pSmEndDev->end, WAIT_FOREVER);
        }

    else
        {
        /* if operating mode is polling, return error */

        SM_LOG (SM_DBG_TX, ("smEndSend: polled mode enabled!\n"));
        END_ERR_ADD (&pSmEndDev->end, MIB2_OUT_ERRS, +1);
        return (END_ERR_BLOCK);
        }

    /* walk the mBlk assembly */

    if (smEndMblkWalk (pSmEndDev->maxPktBytes, mBlkId, &framNum, &pktType,
                       &framSizeMax) == ERROR)
        {
        END_ERR_ADD (&pSmEndDev->end, MIB2_OUT_ERRS, +1);
        END_TX_SEM_GIVE (&pSmEndDev->end);
        errno = EINVAL;
        return (END_ERR_BLOCK);
        }

    /* ensure sufficient SM packet resources for all data */

    if (smPktInfoGet (&pSmEndDev->smPktDesc, &smInfo) == ERROR)
        {
        SM_LOG (SM_DBG_TX, ("smEndSend: cpu not attached!\n"));
        END_ERR_ADD (&pSmEndDev->end, MIB2_OUT_ERRS, +1);
        END_TX_SEM_GIVE (&pSmEndDev->end);
        return (END_ERR_BLOCK);
        }
    SM_LOG (SM_DBG_TX, ("smEndSend: #free pkts = %d\n", smInfo.freePkts));

    if ((framNum     > smInfo.freePkts) ||
        (framSizeMax > pSmEndDev->smPktDesc.maxPktBytes))
        {
        SM_LOG (SM_DBG_TX, ("smEndSend: SM resource shortage: FRAME   PKT\n"));
        SM_LOG (SM_DBG_TX, ("                        Number:  %u    %d\n",
                framNum, smInfo.freePkts));
        SM_LOG (SM_DBG_TX, ("                          Size:  %u    %d\n",
                framSizeMax, pSmEndDev->smPktDesc.maxPktBytes));
        END_ERR_ADD (&pSmEndDev->end, MIB2_OUT_ERRS, +1);
        END_TX_SEM_GIVE (&pSmEndDev->end);
        errno = ENOSPC;
        return (END_ERR_BLOCK);
        }

    /* point to ether address in local cluster data */

    pEh = (struct ether_header *)pMblk->mBlkHdr.mData;

    /* check for broadcast message */

    if (bcmp ((caddr_t) etherAdrsPtr (pEh->ether_dhost),
              (caddr_t) etherAdrsPtr (etherbroadcastaddr),
              sizeof (pEh->ether_dhost)) == 0)
        {
        destCPU  = SM_BROADCAST;	 /* specify broadcast */
        SM_LOG (SM_DBG_TX, ("smEndSend: broadcast message\n"));
        }

    /* unicast message */

    else
        {
        pEaddr = etherAdrsPtr (pEh->ether_dhost);
        destCPU = (unsigned) pEaddr [SM_ETHER_CPU];

        SM_LOG (SM_DBG_TX, ("smEndSend: dest CPU = %u\n", destCPU));
        }

    /* diagnostics */

    SM_LOG (SM_DBG_TX, ("smEndSend: [%#x] len:%d src:%s\n",
            pEh->ether_type, pMblk->mBlkPktHdr.len,
            smDbgEaddrSprintf (etherAdrsPtr (pEh->ether_shost))));
    SM_LOG (SM_DBG_TX, ("  dst:%s cpu [%#x], #frames:%#x\n",
            smDbgEaddrSprintf (etherAdrsPtr (pEh->ether_dhost)), destCPU,
            framNum));

    /* initiate SM packet transfer */

    for ( ; (framNum > 0) && (pMblk != NULL); --framNum)
        {
        /*
         * Get next free SM packet from pool.  If this fails, must return
         * unused resources to stack's pool, else they are lost.  (SPR 70171)
         */

    	if ((smPktFreeGet (&pSmEndDev->smPktDesc, &pPkt) == ERROR) ||
    	    (pPkt == NULL))
            {
            /* free all remaining stack pool resources */

            do
                {
                pNxtMblk = (M_BLK *)pMblk->mBlkHdr.mNextPkt;
                netMblkClChainFree (pMblk);
                pMblk = pNxtMblk;
                } while (pMblk != NULL);

            SM_LOG (SM_DBG_TX, ("smEndSend: smPktFreeGet error 0x%x\n", errno));
            END_ERR_ADD (&pSmEndDev->end, MIB2_OUT_ERRS, +1);
            continue;
            }

    	/* copy data to pkt */

    	pEh = (struct ether_header *)(&pPkt->data[0]);
    	pPkt->header.nBytes =
            netMblkToBufCopy (pMblk, (char *)pEh, smEndCopyRtn);
        SM_LOG (SM_DBG_TX, ("smEndSend: sending size: pkt = %u, frame = %u\n",
                (unsigned)pPkt->header.nBytes,
                (unsigned)pMblk->mBlkPktHdr.len));

        /* now send (announce) packet to destination cpu */

    	if (smPktSend (&pSmEndDev->smPktDesc, pPkt, destCPU) == ERROR)
    	    {
            SM_LOG (SM_DBG_TX, ("smEndSend: smPktSend error: 0x%x\n", errno));
            END_ERR_ADD (&pSmEndDev->end, MIB2_OUT_ERRS, +1);

	    /*
	     * need to return shared memory packet on error,
	     * unless it's an incomplete broadcast error.
	     */

    	    if (errno != S_smPktLib_INCOMPLETE_BROADCAST)
    		(void)smPktFreePut (&pSmEndDev->smPktDesc, pPkt);
    	    }

        /* Bump the statistic counter. */

    	else
    	    {
    	    if (destCPU == SM_BROADCAST)	/* broadcast is non-unicast */
    	        pSmEndDev->end.mib2Tbl.ifOutNUcastPkts++;
    	    else
                END_ERR_ADD (&pSmEndDev->end, MIB2_OUT_UCAST, +1);
    	    }

        /* get next frame pointer */

        pNxtMblk = (M_BLK *)pMblk->mBlkHdr.mNextPkt;

        /* Cleanup.  The driver must free the current frame. */

        netMblkClChainFree (pMblk);

        pMblk = pNxtMblk;
        }

    /* relinquish ownership of shared memory */

    END_TX_SEM_GIVE (&pSmEndDev->end);
    
    return (OK);
    }


/******************************************************************************
*
* smEndPollQGet - get next queued tuple
*
* This routine gets the next queued tuple, if any, and copies its cluster to
* the cluster of the receiving tuple specified by <mBlkId>.
*
* RETURNS
*   OK     - Upon success.
*   NONE   - If no tuple is available.
*   EAGAIN - If the receiving tuple cluster size is too small.
*
* SEE ALSO: smEndPollQPut() and smEndPollQFree()
*/

LOCAL int smEndPollQGet
    (
    SM_END_DEV * pSmEndDev,	/* device containing queue */
    M_BLK_ID     mBlkId		/* tuple to receive queued tuple */
    )
    {
    M_BLK_ID pMblk = NULL;

    /* if no queued tuples, return NONE */

    if (pSmEndDev->pollQ == NULL)
        return (NONE);

    /*
     * if queued tuple has more data than receiving tuple can hold,
     * return EAGAIN
     */

    pMblk = pSmEndDev->pollQ;
    if (pMblk->mBlkHdr.mLen > mBlkId->pClBlk->clSize)
        return (EAGAIN);

    /* copy data from queued tuple to receiving tuple */

    bcopy (pMblk->mBlkHdr.mData, mBlkId->mBlkHdr.mData, pMblk->mBlkHdr.mLen);
    mBlkId->mBlkHdr.mLen   = pMblk->mBlkHdr.mLen;
    mBlkId->mBlkHdr.mFlags = pMblk->mBlkHdr.mFlags;
    mBlkId->mBlkPktHdr.len = pMblk->mBlkPktHdr.len;

    /* remove tuple from queue and free it */

    pSmEndDev->pollQ = pMblk->mBlkHdr.mNext;
    if (pSmEndDev->pollQ == NULL)
        pSmEndDev->pollQLast = NULL;
    netMblkClFree (pMblk);

    return (OK);
    }


/******************************************************************************
*
* smEndPollQFree - free all queued tuples in the specified device
*
* This routine frees all tuples, if any, queued in the specified device.
*
* RETURNS: N/A
*
* SEE ALSO: smEndPollQGet() and smEndPollQPut()
*/

LOCAL void smEndPollQFree
    (
    SM_END_DEV * pSmEndDev	/* device containing queue */
    )
    {
    M_BLK_ID pMblk    = NULL;
    M_BLK_ID pMblkNxt = NULL;

    pMblk = pSmEndDev->pollQ;
    while (pMblk != NULL)
        {
        pMblkNxt = pMblk->mBlkHdr.mNext;
        netMblkClFree (pMblk);
        pMblk = pMblkNxt;
        }
    pSmEndDev->pollQLast = pSmEndDev->pollQ = NULL;
    }


/******************************************************************************
*
* smEndPollRecv - routine to receive a packet in polled mode.
*
* This routine is called by a user to try and get a packet from the device.
*
* RETURNS
*   OK     - Upon success.
*   EAGAIN - When no packet is available, the cpu is not attached to SM, or
*            no free mBlks.
*   EINVAL - If either input argument is NULL.
*   EACCES - If device is not in polled mode.
*/

LOCAL int smEndPollRecv
    (
    VOID *    pObj,	/* device to be polled */
    M_BLK_ID  pMblk	/* ptr to buffer */
    )
    {
    SM_END_DEV * pSmEndDev = (SM_END_DEV *)pObj; /* device to be polled */
    SM_PKT *     pPkt      = NULL;		/* shared memory packet */
    struct ether_header * pEh;			/* ethernet header */
    u_char *     pData;				/* packet data to process */
    int          qStatus;
    int          len;

    if ((pObj == NULL) || (pMblk == NULL))
        {
        END_ERR_ADD (&pSmEndDev->end, MIB2_IN_ERRS, +1);
        return (EINVAL);
        }

    /* cpu attached and in polled mode? */

    if (!SM_END_POLL_SM_RDY)
        {
        END_ERR_ADD (&pSmEndDev->end, MIB2_IN_ERRS, +1);
        return (EACCES);
        }

    /* return next queued tuple, if any */

    if ((qStatus = smEndPollQGet (pSmEndDev, pMblk)) != NONE)
        {
        if (qStatus == OK)
            {
            /* Add one to our unicast data. */

            END_ERR_ADD (&pSmEndDev->end, MIB2_IN_UCAST, +1);
            }

        return (qStatus);
        }

    /* any SM packets? */

    if (ntohl (*pSmEndDev->pRxPktCnt) < 1)
        {
/*        END_ERR_ADD (&pSmEndDev->end, MIB2_IN_ERRS, +1);*/
        return (EAGAIN);
        }

    /* get packet address */

    if ((smPktRecv (&pSmEndDev->smPktDesc, &pPkt) != OK) ||
        (pPkt == NULL))
        {
        END_ERR_ADD (&pSmEndDev->end, MIB2_IN_ERRS, +1);
        return (EAGAIN);
        }

    /* Get packet length */

    if ((len = SM_END_PKT_LEN_GET (pPkt)) < SIZEOF_ETHERHEADER)
        {
        END_ERR_ADD (&pSmEndDev->end, MIB2_IN_ERRS, +1);
        return (EAGAIN);		/* invalid packet size */
        }

    /* adjust for enet header */

    pEh = (struct ether_header *)(&pPkt->data[0]);	/* ethernet header */
    pData = ((u_char *) pEh);

    /* Upper layer must provide a valid buffer */


    if ((pMblk->mBlkHdr.mLen < len))
	{
	return (EAGAIN);
	}

    /* if packet with no data, just return */

    if (len == 0)
        {
	return (EAGAIN);
        }

    /* set up mBlk */

    pMblk->mBlkHdr.mLen    = len;
    pMblk->mBlkHdr.mFlags |= M_PKTHDR;
    pMblk->mBlkPktHdr.len  = len;

    /* copy packet from SM to tuple cluster and return to MUX */

    smEndCopyRtn (pData, pMblk->mBlkHdr.mData, len);

    /* Add one to our unicast data. */

    END_ERR_ADD (&pSmEndDev->end, MIB2_IN_UCAST, +1);

    smPktFreePut (&pSmEndDev->smPktDesc, pPkt);

    return (OK);
    }


/******************************************************************************
*
* smEndPollSend - routine to send a packet in polled mode.
*
* This routine is called by a user to try and send a packet on the device.  It
* does not free the Mblk it is passed under any circumstances, that being the
* responsibility of the caller.
*
* RETURNS
*   OK     - Upon success.
*   EINVAL - If either input argument is NULL.
*   EACCES - If device is not in polled mode.
*   EAGAIN - In all other cases.
*/

LOCAL int smEndPollSend
    (
    void *   pObj,	/* device to be polled */
    M_BLK_ID pMblk	/* packet to send */
    )
    {
    SM_END_DEV *          pSmEndDev = (SM_END_DEV *)pObj; /* device to poll */
    unsigned              len;
    int                   destCPU;		/* destination cpu */
    struct ether_header * pEh      = NULL;	/* enet header */
    u_char *              pEaddr;		/* enet address ptr */
    SM_PKT *              pPkt     = NULL;	/* packet pointer */
    SM_PKT_INFO           smInfo;		/* SM info */

    if ((pObj == NULL) || (pMblk == NULL))
        {
        END_ERR_ADD (&pSmEndDev->end, MIB2_OUT_ERRS, +1);
        return (EINVAL);
        }

    /* cpu attached and in polled mode? */

    if (!SM_END_POLL_SM_RDY)
        {
        END_ERR_ADD (&pSmEndDev->end, MIB2_OUT_ERRS, +1);
        return (EACCES);
        }

    /* ensure sufficient SM packet resources for one cluster */

    if (smPktInfoGet (&pSmEndDev->smPktDesc, &smInfo) == ERROR)
        {
        END_ERR_ADD (&pSmEndDev->end, MIB2_OUT_ERRS, +1);
        return (EAGAIN);
        }

    if (smInfo.freePkts == 0)
        {
        END_ERR_ADD (&pSmEndDev->end, MIB2_OUT_ERRS, +1);
        return (EAGAIN);
        }

    /* get a free SM packet */

    if ((smPktFreeGet (&pSmEndDev->smPktDesc, &pPkt) == ERROR) ||
        (pPkt == NULL))
        {
        END_ERR_ADD (&pSmEndDev->end, MIB2_OUT_ERRS, +1);
        return (EAGAIN);
        }

    /* point to ether address in local cluster data */

    pEh = (struct ether_header *)pMblk->mBlkHdr.mData;

    /* check for broadcast message */

    if (bcmp ((caddr_t) etherAdrsPtr (pEh->ether_dhost),
              (caddr_t) etherAdrsPtr (etherbroadcastaddr) ,
              sizeof (pEh->ether_dhost)) == 0)
        destCPU  = (int)(SM_BROADCAST);	 /* specify broadcast */

    /* unicast message */

    else
        {
        pEaddr = etherAdrsPtr (pEh->ether_dhost);
        destCPU = pEaddr [SM_ETHER_CPU];
        }

    /* copy data from cluster to SM packet */

    pEh = (struct ether_header *)(&pPkt->data[0]);
    len = max (ETHERSMALL, pMblk->mBlkHdr.mLen);
    smEndCopyRtn (pMblk->mBlkHdr.mData, (char *)pEh, len);
    pPkt->header.nBytes = pMblk->mBlkHdr.mLen;

    /* now send (announce) packet to destination cpu */

    if (smPktSend (&pSmEndDev->smPktDesc, pPkt, destCPU) == ERROR)
        {
        END_ERR_ADD (&pSmEndDev->end, MIB2_OUT_ERRS, +1);

        /*
         * need to return shared memory packet on error,
         * unless it's an incomplete broadcast error.
         */

        if (errno != S_smPktLib_INCOMPLETE_BROADCAST)
            (void)smPktFreePut (&pSmEndDev->smPktDesc, pPkt);

        return (EAGAIN);
        }

    /* Bump the statistic counter. */

    else
        END_ERR_ADD (&pSmEndDev->end, MIB2_OUT_UCAST, +1);

    return (OK);
    }


/******************************************************************************
*
* smEndMCastAddrAdd - add a multicast address to the address list
*
* This routine adds a multicast address to the list of addresses the driver
* is already listening for.
* 
* RETURNS: OK or ERROR.
*
* SEE ALSO: smEndMCastAddrDel() smEndMCastAddrGet()
*
*/

LOCAL STATUS smEndMCastAddrAdd
    (
    void * pObj,	/* cookie pointer */
    char * pAddr	/* address to be added */
    )
    {
    int          retVal    = ERROR;
#if FALSE	/*  multicast is a future enhancement */
    SM_END_DEV * pSmEndDev = (SM_END_DEV *)pObj; /* device ptr */

    SM_LOG (SM_DBG_MCAST,
            ("smEndMCastAddrAdd addr = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
	    (int) (*pAddr + 0), (int) (*pAddr + 1), 
	    (int) (*pAddr + 2), (int) (*pAddr + 3), 
	    (int) (*pAddr + 4), (int) (*pAddr + 5)));

    retVal = etherMultiAdd (&pSmEndDev->end.multiList, pAddr);

    if (retVal == ENETRESET)
	{
        pSmEndDev->end.nMulti++;
 
	return (smEndHashTblAdd (pSmEndDev, pAddr));
	}
#else
    errno = ENOTSUP;
#endif

    return ((retVal == OK) ? OK : ERROR);
    }

/******************************************************************************
*
* smEndMCastAddrDel - delete a multicast address from the address list
*
* This routine deletes a multicast address from the current list of multicast
* addresses.
* 
* RETURNS: OK or ERROR.
*
* SEE ALSO: smEndMCastAddrAdd() smEndMCastAddrGet()
*
*/

LOCAL STATUS smEndMCastAddrDel
    (
    void * pObj,	/* cookie pointer */
    char * pAddr	/* address to be deleted */
    )
    {
    int          retVal    = ERROR;
#if FALSE	/*  multicast is a future enhancement */
    SM_END_DEV * pSmEndDev = (SM_END_DEV *)pObj; /* device ptr */

    SM_LOG (SM_DBG_MCAST,
            ("smEndMCastAddrDel addr = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
	    (int) (*pAddr + 0), (int) (*pAddr + 1), 
	    (int) (*pAddr + 2), (int) (*pAddr + 3), 
	    (int) (*pAddr + 4), (int) (*pAddr + 5)));

    retVal = etherMultiDel (&pSmEndDev->end.multiList, pAddr);

    if (retVal == ENETRESET)
	{
	pSmEndDev->end.nMulti--;

	/* stop the SM device */

	if (smEndStop (pSmEndDev) != OK)
	    return (ERROR);

	/* populate the hash table */

	retVal = smEndHashTblPopulate (pSmEndDev);

	/* restart the SM device */

	if (smEndStart (pSmEndDev) != OK)
	    return (ERROR);
	}
#else
    errno = ENOTSUP;
#endif

    return ((retVal == OK) ? OK : ERROR);
    }

/******************************************************************************
*
* smEndMCastAddrGet - get the current multicast address list
*
* This routine copies the current multicast address list to <pTable>.
*
* RETURNS: OK or ERROR.
*
* SEE ALSO: smEndMCastAddrAdd() smEndMCastAddrDel()
*
*/

LOCAL STATUS smEndMCastAddrGet
    (
    void *        pObj,		/* cookie pointer */
    MULTI_TABLE * pTable	/* table into which to copy addresses */
    )
    {
#if FALSE	/*  multicast is a future enhancement */
    SM_END_DEV * pSmEndDev = (SM_END_DEV *)pObj; /* device ptr */

    SM_LOG (SM_DBG_MCAST, ("smEndMCastAddrGet\n"));

    return (etherMultiGet (&pSmEndDev->end.multiList, pTable));
#else
    errno = ENOTSUP;
    return (ERROR);
#endif
    }


