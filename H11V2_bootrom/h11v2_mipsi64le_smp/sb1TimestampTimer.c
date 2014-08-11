/* sb1TimestampTimer.c - sibyte timestamp timer using ZBbus timer */

/*
 * Copyright 2007,2008 Wind River Systems, Inc.
 *
 * The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.
 */

/*
modification history
--------------------
01c,01oct08,pgh  Fix timestamp clock rate.
01b,18jan08,pgh  Reduce the period of the timestamp timer.
01a,28feb07,slk  created.
*/

/*
DESCRIPTION

*/

#include <vxWorks.h>
#include <vxbTimerLib.h>
#if (BCM_FAMILY == BCM_SB1)
#include <drv/multi/sb1Lib.h>
#elif (BCM_FAMILY == BCM_SB1A)
#include <drv/multi/bcm1480Lib.h>
#else
#error "Unknown BCM_FAMILY"
#endif


#ifdef  INCLUDE_TIMESTAMP

/* defines */

#define SB1_ZBBUS_ROLLOVER_CNT   0x0fffffff  /* use only 28 bits of counter */

#define ZBBUS_TIMER_CNT_GET  *(UINT64 *)(PHYS_TO_K1(A_ZBBUS_CYCLE_CNT))

/* locals */

LOCAL BOOL sb1TimestampRunning = FALSE;

/* imports */

IMPORT int vxbR4KTimerFrequencyGet (struct vxbDev *);

/* forward declarations */

LOCAL  STATUS  sb1TimestampConnect (FUNCPTR routine, int arg);
LOCAL  STATUS  sb1TimestampEnable (void);
LOCAL  STATUS  sb1TimestampDisable (void);
LOCAL  UINT32  sb1TimestampFreq (void);
LOCAL  UINT32  sb1TimestampPeriod (void);
LOCAL  UINT32  sb1Timestamp (void);
UINT32  sb1TimestampLock (void);



/********************************************************************************
* sb1TimestampInit - init the callbacks
*
* This routine initializes the callback hooks in vxbTimestampLib.c to
* have functions in this file replace the generic functions supplied by
* the library
*
* RETURNS: OK always.
*/

STATUS sb1TimestampInit ()
    {
    sb1TimestampRunning = FALSE;

    _vxb_timestampConnectRtn = sb1TimestampConnect;
    _vxb_timestampEnableRtn = sb1TimestampEnable;
    _vxb_timestampDisableRtn = sb1TimestampDisable;
    _vxb_timestampPeriodRtn  = sb1TimestampPeriod;
    _vxb_timestampFreqRtn    = sb1TimestampFreq;
    _vxb_timestampRtn        = sb1Timestamp;
    _vxb_timestampLockRtn    = sb1TimestampLock;

    return (OK);
    }


/*******************************************************************************
*
* sb1TimestampConnect - connect a user routine to a timestamp timer interrupt
*
* This routine specifies the user interrupt routine to be called at each
* timestamp timer interrupt.  This functions is currently not supported.
*
* RETURNS: ERROR, always.
*/

LOCAL STATUS sb1TimestampConnect
    (
    FUNCPTR routine,    /* routine called at each timestamp timer interrupt */
    int arg             /* argument with which to call routine */
    )
    {
    return (ERROR);
    }

/*******************************************************************************
*
* sb1TimestampEnable - enable a timestamp timer interrupt
*
* This routine enables timestamp timer interrupts
*
* RETURNS: OK, always.
*
* SEE ALSO: sb1TimestampDisable()
*/

LOCAL STATUS sb1TimestampEnable (void)
   {
   sb1TimestampRunning = TRUE;

   return (OK);
   }

/*******************************************************************************
*
* sb1TimestampDisable - disable a timestamp timer interrupt
*
* This routine returns ERROR because the timestamp timer cannot be disabled.
*
* RETURNS: ERROR, always.
*
* SEE ALSO: sb1TimestampEnable()
*/

LOCAL STATUS sb1TimestampDisable (void)
    {
    return (ERROR);
    }


/*******************************************************************************
*
* sb1TimestampPeriod - get the period of a timestamp timer
*
* This routine gets the period of the timestamp timer, in ticks.  The
* period, or terminal count, is the number of ticks to which the timestamp
* timer counts before rolling over and restarting the counting process.
*
* RETURNS: The period of the timestamp timer in counter ticks.
*/

LOCAL UINT32 sb1TimestampPeriod (void)
    {

    /*
     * max. count of timer before rollover
     */

    return (SB1_ZBBUS_ROLLOVER_CNT);
    }

/*******************************************************************************
*
* sb1TimestampFreq - get a timestamp timer clock frequency
*
* This routine gets the frequency of the timer clock, in ticks per
* second.  The rate of the timestamp timer is set explicitly by the
* hardware and cannot be altered.
*
* RETURNS: The timestamp timer clock frequency, in ticks per second.
*/

LOCAL UINT32 sb1TimestampFreq (void)
    {
    /* ZBbus timer runs at 1/2 CPU clock rate */

    return ((vxbR4KTimerFrequencyGet((struct vxbDev *)0) / 2));
    }

/*******************************************************************************
*
* sb1Timestamp - get a timestamp timer tick count
*
* This routine returns the current value of the timestamp timer tick counter.
* The tick count can be converted to seconds by dividing it by the return of
* sysTimestampFreq().
*
* This routine should be called with interrupts locked.  If interrupts are
* not locked, sysTimestampLock() should be used instead.
* note: this function is identical to sb1TimestampLock
*
* RETURNS: The current timestamp timer tick count.
*
* SEE ALSO: sb1TimestampFreq(), sb1TimestampLock()
*/

LOCAL UINT32 sb1Timestamp (void)
    {
    return ((UINT32)ZBBUS_TIMER_CNT_GET & SB1_ZBBUS_ROLLOVER_CNT);
    }

/*******************************************************************************
*
* sb1TimestampLock - lock interrupts and get the timestamp timer tick count
*
* This routine locks interrupts when the tick counter must be stopped
* in order to read it or when two independent counters must be read.
* It then returns the current value of the timestamp timer tick
* counter.
*
* The tick count can be converted to seconds by dividing it by the return of
* sysTimestampFreq().
*
* If interrupts are already locked, sysTimestamp() should be
* used instead.
* note: this function is identical to sb1Timestamp
*
* RETURNS: The current timestamp timer tick count.
*
* SEE ALSO: sb1TimestampFreq(), sb1Timestamp()
*/

UINT32 sb1TimestampLock (void)
    {
    return ((UINT32)ZBBUS_TIMER_CNT_GET & SB1_ZBBUS_ROLLOVER_CNT);
    }

#endif  /* INCLUDE_TIMESTAMP */
