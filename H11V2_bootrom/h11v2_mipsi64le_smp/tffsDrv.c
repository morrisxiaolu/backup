


/* includes */

#include "tffs\tffsDrv.h"
#include "tffs\fatlite.h"
#include <ioLib.h>

/* defines */

/* externs */
IMPORT int flFileSysSectorStart;
/* globals */

#if     (POLLING_INTERVAL > 0)
SEM_ID    flPollSemId;
#endif  /* (POLLING_INTERVAL > 0) */

/* locals */

LOCAL BOOL tffsDrvInstalled = FALSE;		/* TRUE, if installed */
LOCAL BOOL tffsDrvStatus = ERROR;		/* OK, if succeeded */
LOCAL TFFS_DEV * tffsBlkDevs[DRIVES] = {NULL};	/* FLite block Devices */


/* forward declarations */

LOCAL void   tffsSetFromBPB	(BLK_DEV *pBlkDev, BPB *pBPB);
LOCAL STATUS tffsIoctl		(TFFS_DEV * pTffsDev, int function, int arg);
LOCAL STATUS tffsBlkRd		(TFFS_DEV * pTffsDev, int startBlk, 
				 int numBlks, char * pBuffer);
LOCAL STATUS tffsBlkWrt		(TFFS_DEV * pTffsDev, int startBlk, 
				 int numBlks, char * pBuffer);
#if     (POLLING_INTERVAL > 0)
LOCAL FLStatus flPollSemCreate (void);
#endif /* (POLLING_INTERVAL > 0) */

/*******************************************************************************
*
* tffsDrv - initialize the TrueFFS system
*
* This routine sets up the structures, the global variables, and the mutual 
* exclusion semaphore needed to manage TrueFFS. This call also registers 
* socket component drivers for all the flash devices attached to your target.  
*
* Because tffsDrv() is the call that initializes the TrueFFS system, this
* function must be called (exactly once) before calling any other TrueFFS
* utilities, such as tffsDevFormat() or tffsDevCreate().  Typically, the call
* to tffsDrv() is handled for you automatically.  If you defined INCLUDE_TFFS
* in your BSP's config.h, the call to tffsDrv() is made from usrRoot().  If
* your BSP's config.h defines INCLUDE_PCMCIA, the call to tffsDrv() is made
* from pccardTffsEnabler().
* 
* RETURNS: OK, or ERROR if it fails.
*/

STATUS tffsDrv (void)
    {
    if (!tffsDrvInstalled)
	{
        /* FLite initialization:                                  */
        /*      - register all the components and initialize them */
        /*      - create socket polling task                      */
        /*      - create task for background FLite operations     */

#if     (POLLING_INTERVAL > 0)

        /* Create Synchronisation semaphore */

        if (flPollSemCreate() != flOK)
            return (ERROR);

#endif /* (POLLING_INTERVAL > 0) */

        tffsDrvStatus = (flInit() == flOK) ? OK : ERROR;
	tffsDrvInstalled = TRUE;

#if     (POLLING_INTERVAL > 0)

	if (tffsDrvStatus == flOK)
	    semGive (flPollSemId);

#endif /* (POLLING_INTERVAL > 0) */
	}

    return (tffsDrvStatus);
    }

/*******************************************************************************
*
* tffsDevCreate - create a TrueFFS block device suitable for use with dosFs
*
* This routine creates a TFFS block device on top of a flash device. It takes 
* as arguments a drive number, determined from the order in which the socket
* components were registered, and a flag integer that indicates whether the 
* medium is removable or not. A zero indicates a non removable medium. A one 
* indicates a removable medium.  If you intend to mount dosFs on this block 
* device, you probably do not want to call tffsDevCreate(), but should 
* call usrTffsConfig() instead.  Internally, usrTffsConfig() 
* calls tffsDevCreate() for you.  It then does everything necessary (such as 
* calling the dosFsDevInit() routine) to mount dosFs on the just created 
* block device.  
*
* RETURNS: BLK_DEV pointer, or NULL if it failed.
*/

BLK_DEV * tffsDevCreate 
    (
    int tffsDriveNo,			/* TFFS drive number (0 - DRIVES-1) */
    int removableMediaFlag		/* 0 - nonremovable flash media */
    )
    {
    FAST TFFS_DEV  *pTffsDev;           /* ptr to created TFFS_DEV struct */
    FAST BLK_DEV   *pBlkDev;            /* ptr to BLK_DEV struct          */
    FLStatus        status = flOK;
    BPB             bpb;
    IOreq           ioreq;

    if (tffsDriveNo >= DRIVES)
        return (NULL);

    ioreq.irHandle = tffsDriveNo;

    /* create and initialize BLK_DEV structure */

    pTffsDev = (TFFS_DEV *) malloc (sizeof (TFFS_DEV));

    if (pTffsDev == NULL)
        return (NULL);

    status = flMountVolume(&ioreq);

    if (status == flOK)
        {
        ioreq.irData = &bpb;

        status = flGetBPB(&ioreq);

        if (status == flOK)
            {
            pBlkDev = &pTffsDev->tffsBlkdev;

            tffsSetFromBPB (pBlkDev, &bpb);

            if (removableMediaFlag)
                pBlkDev->bd_removable  = TRUE;    /* removable                */
            else
                pBlkDev->bd_removable  = FALSE;   /* not removable            */

            pBlkDev->bd_retry        = 1;         /* retry count              */
            pBlkDev->bd_mode         = O_RDWR;    /* initial mode for device  */
            pBlkDev->bd_readyChanged = TRUE;      /* new ready status         */
            pBlkDev->bd_blkRd        = tffsBlkRd; /* read block function      */
            pBlkDev->bd_blkWrt       = tffsBlkWrt;/* write block function     */
            pBlkDev->bd_ioctl        = tffsIoctl; /* ioctl function           */
            pBlkDev->bd_reset        = NULL;      /* no reset function        */
            pBlkDev->bd_statusChk    = NULL;      /* no check-status function */
            pTffsDev->tffsDriveNo    = tffsDriveNo;

            if((pTffsDev->tffsSem = semMCreate(SEM_Q_PRIORITY)) == NULL)
              status = flNotEnoughMemory;          

	    /* Turn off FAT monitoring */
	    tffsDevOptionsSet (pTffsDev);
	    }
        else
	    ;
        }

    if (status != flOK)
    {
        free(pTffsDev);
        return (NULL);
    }

    /* remember that we have created FLite device */

    tffsBlkDevs[tffsDriveNo] = pTffsDev;

    return (&pTffsDev->tffsBlkdev);
    }

/*******************************************************************************
*
* tffsDevOptionsSet - set TrueFFS volume options
*
* This routine is intended to set various TrueFFS volume options. At present
* it only disables FAT monitoring. If VxWorks long file names are to be used
* with TrueFFS, FAT monitoring must be turned off.
*
* RETURNS: OK, or ERROR if it failed.
*/

STATUS tffsDevOptionsSet
    (
    TFFS_DEV * pTffsDev                /* pointer to device descriptor */
    )
    {
    FLStatus        status;
    IOreq           ioreq;

    /* Note : it is expected that as more volume option that would 
     *        need to get set via this routine are detected a second
     *        parameter will be added to indicate the option that needs
     *        to get set.
     */

    if ((pTffsDev == NULL) || (pTffsDev->tffsDriveNo >= DRIVES))
        return (ERROR);

    ioreq.irHandle = pTffsDev->tffsDriveNo;

    /* disable FAT monitoring for TrueFFS volumes */

    status = flDontMonitorFAT(&ioreq);

    return ((status == flOK) ? OK : ERROR);
    }

/*******************************************************************************
*
* tffsDrvOptionsSet - set TrueFFS volume options
*
* This routine is intended to set various TrueFFS volume options. At present
* it only disables FAT monitoring. If VxWorks long file names are to be used
* with TrueFFS, FAT monitoring must be turned off. If Datalite's Reliance
* file file system is to be used with TrueFFS, FAT monitoring must be turned
* off.
*
* RETURNS: OK, or ERROR if it failed.
*/

STATUS tffsDrvOptionsSet
    (
    int tffsDriveNo			/* TFFS drive number (0 - DRIVES-1) */
    )
    {
    FLStatus        status;
    IOreq           ioreq;


    if (tffsDriveNo >= DRIVES)
        return (ERROR);

    ioreq.irHandle = tffsDriveNo;

    /* disable FAT monitoring for TrueFFS volumes */

    status = flDontMonitorFAT(&ioreq);

    return ((status == flOK) ? OK : ERROR);
    }

/*******************************************************************************
*
* tffsIoctl - handle IOCTL calls to TFFS driver
*
* This routine handles IOCTL calls to TrueFFS driver. Currently it sets a global
* error flag and exits. The ioctl FIODISKFORMAT should not be used with TrueFFS
* devices. Use tffsDevFormat() to format TrueFFS drives.
*
* RETURNS: ERROR always.
*
* ERRNO: S_ioLib_UNKNOWN_REQUEST
*
*/

LOCAL STATUS tffsIoctl 
    (
    TFFS_DEV * pTffsDev,		/* pointer to device descriptor */
    int function,			/* function code */
    int arg				/* some argument */
    )
    {
        int       status = OK;
        SECTOR_RANGE *pRange;
                                                                                
        switch (function)

            {
                                                                                
        case FIODISCARDGET:
            *(int *)arg = 1;    /* yes, we want XBD_DISCARD ops */
            break;
                                                                                
        case FIODISCARD:       /* file system layer is discarding sectors */
            pRange = (void *)arg;
            if(! tffsRawio(pTffsDev->tffsDriveNo, TFFS_ABS_DELETE,
                           (int)pRange->startSector, (int)pRange->nSectors,0))
            {
               errnoSet(S_ioLib_DEVICE_ERROR);
               status  = ENOSPC;
            }
            break;
                                                                                
        default:
            errnoSet(S_ioLib_UNKNOWN_REQUEST);
            status = ENOTSUP;
            break;
            }
                                                                                
        return status;
    } 


/*******************************************************************************
*
* tffsBlkRd - reads sequence of blocks from TFFS device
*
* This routine reads a sequence of blocks from TrueFFS formatted device.
*
* RETURNS: OK, or ERROR if it failed.
*
* \NOMANUAL
*/

LOCAL STATUS tffsBlkRd 
    (
    FAST TFFS_DEV * pTffsDev,		/* pointer to device descriptor */
    int startBlk,			/* starting block number to read */
    int numBlks,			/* number of blocks to read */
    char * pBuffer			/* pointer to buffer to receive data */
    )
    {
    FLStatus    status = flOK;
    IOreq       ioreq;
    BPB         bpb;

    if ( (NULL == pTffsDev) || (NULL == pBuffer) ||
         (pTffsDev->tffsDriveNo >= DRIVES) )
        {
        return (ERROR);
        }

    if(semTake(pTffsDev->tffsSem, WAIT_FOREVER) == ERROR)
      return (ERROR);

    ioreq.irHandle = pTffsDev->tffsDriveNo;

    status = flCheckVolume(&ioreq);

    if (status == flNotMounted)
        {
        status = flMountVolume(&ioreq);

        if (status == flOK)
            {
            ioreq.irData = &bpb;
            status = flGetBPB(&ioreq);
            }

        if (status != flOK)
            {
            pTffsDev->tffsBlkdev.bd_readyChanged = TRUE;
            semGive(pTffsDev->tffsSem);
            return (ERROR);
            }

        /* Modify BLK_DEV structure */

        tffsSetFromBPB( &(pTffsDev->tffsBlkdev), &bpb);

        pTffsDev->tffsBlkdev.bd_mode = O_RDWR; /* initial mode for device */
        }

    ioreq.irSectorNo    = startBlk;
    ioreq.irSectorCount = numBlks;
    ioreq.irData        = pBuffer;

    status = flAbsRead(&ioreq);

    /* if success cancel dosFs re-mount volume request */

    if (status != flOK)
        pTffsDev->tffsBlkdev.bd_readyChanged = TRUE;

    semGive(pTffsDev->tffsSem);

    return ((status == flOK) ? OK : ERROR);
    }

/*******************************************************************************
*
* tffsBlkWrt - write sequence of blocks to TFFS device
*
* This routine writes a sequence of blocks to TrueFFS device.
*
* RETURNS: OK, or ERROR if it failed.
*
* \NOMANUAL
*/

LOCAL STATUS tffsBlkWrt 
    (
    FAST TFFS_DEV * pTffsDev,		/* pointer to device descriptor */
    int startBlk,			/* starting block number to write */
    int numBlks,			/* number of blocks to write */
    char * pBuffer			/* pointer to buffer containing data */
    )
    {
    FLStatus   status = flOK;
    IOreq      ioreq;
    BPB        bpb;

    if ( (NULL == pTffsDev) || (NULL == pBuffer) ||
         (pTffsDev->tffsDriveNo >= DRIVES) )
        {
        return (ERROR);
        }

    if(semTake(pTffsDev->tffsSem, WAIT_FOREVER) == ERROR)
      return (ERROR);
    
    ioreq.irHandle = pTffsDev->tffsDriveNo;

    status = flCheckVolume(&ioreq);

    if (status == flNotMounted)
        {
        status = flMountVolume(&ioreq);

        if (status == flOK)
            {
            ioreq.irData = &bpb;
            status = flGetBPB(&ioreq);
            }

        if (status != flOK)
            {
            pTffsDev->tffsBlkdev.bd_readyChanged = TRUE;
            semGive(pTffsDev->tffsSem);
            return (ERROR);
            }

        /* Modify BLK_DEV structure */

        tffsSetFromBPB( &(pTffsDev->tffsBlkdev), &bpb);
        pTffsDev->tffsBlkdev.bd_mode = O_RDWR;	/* initial mode for device */
        }

    ioreq.irSectorNo    = startBlk;
    ioreq.irSectorCount = numBlks;
    ioreq.irData        = pBuffer;

    status = flAbsWrite(&ioreq);

    if (status == flWriteProtect)
        {
        flDismountVolume(&ioreq);		/* force a remount */
        pTffsDev->tffsBlkdev.bd_mode = O_RDONLY;
        }

    /* re-mount volume request */

    if (status != flOK)
        pTffsDev->tffsBlkdev.bd_readyChanged = TRUE;

    semGive(pTffsDev->tffsSem);

    return ((status == flOK) ? OK : ERROR);
    }

/*******************************************************************************
*
* tffsSetFromBPB - copy some data from BIOS parameter block
*
* This routine copies BIOS parameter block data describing the flash device
* returned by TrueFFS into a device descriptor.
*
* RETURNS: N/A
*
* \NOMANUAL
*/

LOCAL void tffsSetFromBPB 
    (
    BLK_DEV * pBlkDev,		/* pointer to device descriptor */
    BPB * pBPB			/* pointer to BIOS parameters block */
    )
    {
    if ((NULL == pBlkDev) || (NULL == pBPB))
        return;

    pBlkDev->bd_nBlocks      = 0;
    pBlkDev->bd_bytesPerBlk  = 0;
    pBlkDev->bd_blksPerTrack = 0;
    pBlkDev->bd_nHeads       = 0;     /* clear first */

    if( UNAL2(pBPB->totalSectorsInVolumeDOS3) )
        pBlkDev->bd_nBlocks  = UNAL2(pBPB->totalSectorsInVolumeDOS3);
    else
        pBlkDev->bd_nBlocks  = LE4(pBPB->totalSectorsInVolume);

    pBlkDev->bd_nBlocks -= flFileSysSectorStart;

    pBlkDev->bd_bytesPerBlk  = UNAL2(pBPB->bytesPerSector);
    pBlkDev->bd_blksPerTrack = LE2(pBPB->sectorsPerTrack);
    pBlkDev->bd_nHeads       = LE2(pBPB->noOfHeads);
    pBlkDev->bd_nHeads -= flFileSysSectorStart;
    }

/*******************************************************************************
*
* tffsDiskChangeAnnounce - announce disk change to the file system attached
*
* This routine is called by TFFS to update the the readyChanged field in the
* device descriptor so that the file system can be notified of a disk change.
*
* RETURNS: N/A
*
* \NOMANUAL
*/

void tffsDiskChangeAnnounce
    (
    unsigned volNo		/* FLite drive number (0 - DRIVES-1) */
    )
    {
    if ((volNo < DRIVES) && (tffsBlkDevs[volNo] != NULL))
        tffsBlkDevs[volNo]->tffsBlkdev.bd_readyChanged = TRUE;
    }

/*******************************************************************************
*
* tffsDevFormat - format a flash device for use with TrueFFS
*
* This routine formats a flash device for use with TrueFFS.  It takes two 
* parameters, a drive number and a pointer to a device format structure. 
* This structure describes how the volume should be formatted.  The structure 
* is defined in dosformt.h.  The drive number is assigned in the order that 
* the socket component for the device was registered.
*
* The format process marks each erase unit with an Erase Unit Header (EUH) and
* creates the physical and virtual Block Allocation Maps (BAM) for the device. 
* The erase units reserved for the "boot-image" are skipped and the first
* EUH is placed at number (boot-image length - 1). To write to the boot-image
* region, call tffsBootImagePut(). 
* 
* WARNING: If any of the erase units in the boot-image 
* region contains an erase unit header from a previous format call (this can
* happen if you reformat a flash device specifying a larger boot region) 
* TrueFFS fails to mount the device.  To fix this problem, use tffsRawio() to
* erase the problem erase units (thus removing the outdated EUH).  
*
* The macro TFFS_STD_FORMAT_PARAMS defines the default values used for 
* formatting a flask disk device. If the second argument to this routine 
* is zero, tffsDevFormat() uses these default values.
*
* RETURNS: OK, or ERROR if it failed.
*/

STATUS tffsDevFormat 
    (
    int tffsDriveNo,		/* TrueFFS drive number (0 - DRIVES-1) */
    int arg			/* pointer to tffsDevFormatParams structure */
    )
    {
    tffsDevFormatParams defaultParams = TFFS_STD_FORMAT_PARAMS;
    tffsDevFormatParams *devFormatParams;
    IOreq                ioreq;
    FLStatus             status;

    if (tffsDriveNo >= DRIVES)
        return (ERROR);

    /* tell dosFs to re-mount volume */
  
    if (tffsBlkDevs[tffsDriveNo] != NULL)
        tffsBlkDevs[tffsDriveNo]->tffsBlkdev.bd_readyChanged = TRUE;

    if (arg == 0)
        devFormatParams = &defaultParams;
    else
        devFormatParams = (tffsDevFormatParams *) arg;

    ioreq.irHandle = tffsDriveNo;
    ioreq.irFlags  = devFormatParams->formatFlags;
    ioreq.irData   = &(devFormatParams->formatParams);

    status = flFormatVolume(&ioreq);

    return ((status == flOK) ? OK : ERROR);
    }

/*******************************************************************************
*
* tffsRawio - low level I/O access to flash components
*
* Use the utilities provided by thisroutine with the utmost care. If you use 
* these routines carelessly, you risk data loss as well as  permanent 
* physical damage to the flash device.
*
* This routine is a gateway to a series of utilities (listed below). Functions 
* such as mkbootTffs() and tffsBootImagePut() use these tffsRawio() utilities 
* to write boot sector information. The functions for physical read, write, and 
* erase are made available with the intention that they be used on erase units 
* allocated to the boot-image region by tffsDevFormat(). Using these functions 
* elsewhere could be dangerous.
*
* The <arg0>, <arg1>, and <arg2> parameters to tffsRawio() are interpreted 
* differently depending on the function number you specify for <functionNo>. 
* The drive number is determined by the order in which the socket 
* components were registered. 
*
* \ts
* Function Name | arg0 | arg1 | arg2
* ---------------------------------------
* TFFS_GET_PHYSICAL_INFO | user buffer address | N/A | N/A
* TFFS_PHYSICAL_READ | address to read | byte count | user buffer address
* TFFS_PHYSICAL_WRITE | address to write | byte count | user buffer address
* TFFS_PHYSICAL_ERASE | first unit | number of units | N/A
* TFFS_ABS_READ	| sector number | number of sectors | user buffer address
* TFFS_ABS_WRITE | sector number | number of sectors | user buffer address
* TFFS_ABS_DELETE | sector number | number of sectors | N/A
* TFFS_DEFRAGMENT_VOLUME | number of sectors | user buffer address | N/A
* \te
*
* \is
* \i TFFS_GET_PHYSICAL_INFO 
* writes the flash type, erasable block size, and media
* size to the user buffer specified in <arg0>.
*
* \i TFFS_PHYSICAL_READ 
* reads <arg1> bytes from <arg0> and writes them to 
* the buffer specified by <arg2>.
*
* \i TFFS_PHYSICAL_WRITE 
* copies <arg1> bytes from the <arg2> buffer and writes 
* them to the flash memory location specified by <arg0>.  
* This aborts if the volume is already mounted to prevent the versions of 
* translation data in memory and in flash from going out of synchronization.
*
* \i TFFS_PHYSICAL_ERASE 
* erases <arg1> erase units, starting at the erase unit
* specified in <arg0>.
* This aborts if the volume is already mounted to prevent the versions of 
* translation data in memory and in flash from going out of synchronization.
*
* \i TFFS_ABS_READ 
* reads <arg1> sectors, starting at sector <arg0>, and writes
* them to the user buffer specified in <arg2>.
*
* \i TFFS_ABS_WRITE 
* takes data from the <arg2> user buffer and writes <arg1> 
* sectors of it to the flash location starting at sector <arg0>.
* 
* \i TFFS_ABS_DELETE 
* deletes <arg1> sectors of data starting at sector <arg0>. 
*
* \i TFFS_DEFRAGMENT_VOLUME 
* calls the defragmentation routine with the minimum
* number of sectors to be reclaimed, <arg0>, and writes the actual number 
* reclaimed in the user buffer by <arg1>. Calling this function through some 
* low priority task will make writes more deterministic.
* No validation is done of the user specified address fields, so the functions 
* assume they are writable. If the address is invalid, you could see bus errors
* or segmentation faults.
* \ie
* 
* RETURNS: OK, or ERROR if it failed.
*/

STATUS tffsRawio 
    (
    int tffsDriveNo,		/* TrueFFS drive number (0 - DRIVES-1) */
    int functionNo,		/* TrueFFS function code */
    int arg0,			/* argument 0 */
    int arg1,			/* argument 1 */
    int arg2			/* argument 2 */
    )
    {
    IOreq	ioreq;
    FLStatus	status;
    int		function;

    if (tffsDriveNo >= DRIVES)
        return (ERROR);

    if (tffsBlkDevs[tffsDriveNo] != NULL)
        if(semTake(tffsBlkDevs[tffsDriveNo]->tffsSem, WAIT_FOREVER) == ERROR)
          return (ERROR);
    
    ioreq.irHandle = tffsDriveNo;	/* drive number */

    switch (functionNo)
	{
	case TFFS_GET_PHYSICAL_INFO:
	    function = FL_GET_PHYSICAL_INFO;
	    ioreq.irData = (char *)arg0; /* address of user buffer to store */
	    break;

	case TFFS_PHYSICAL_READ:
	    function = FL_PHYSICAL_READ;
	    ioreq.irAddress = arg0;	/* chip/card address to read/write */
	    ioreq.irByteCount = arg1;	/* number of bytes to read/write */
	    ioreq.irData = (char *)arg2; /* address of user buffer to r/w */
	    break;

	case TFFS_PHYSICAL_WRITE:
	    function = FL_PHYSICAL_WRITE;
	    ioreq.irAddress = arg0;	/* chip/card address to read/write */
	    ioreq.irByteCount = arg1;	/* number of bytes to read/write */
	    ioreq.irData = (char *)arg2; /* address of user buffer to r/w */
	    break;

	case TFFS_PHYSICAL_ERASE:
	    function = FL_PHYSICAL_ERASE;
	    ioreq.irUnitNo = arg0;	/* first unit to erase */
	    ioreq.irUnitCount = arg1;	/* number of units to erase */
	    break;
	
	case TFFS_ABS_READ:
	    function = FL_ABS_READ;
	    ioreq.irSectorNo = arg0;	/* sector number to read/write */
	    ioreq.irSectorCount = arg1;	/* number of sectors to read/write */
	    ioreq.irData = (char *)arg2; /* address of user buffer to r/w */
	    break;
	
	case TFFS_ABS_WRITE:
	    function = FL_ABS_WRITE;
	    ioreq.irSectorNo = arg0;	/* sector number to read/write */
	    ioreq.irSectorCount = arg1;	/* number of sectors to read/write */
	    ioreq.irData = (char *)arg2; /* address of user buffer to r/w */
	    break;
	
	case TFFS_ABS_DELETE:
	    function = FL_ABS_DELETE;
	    ioreq.irSectorNo = arg0;	/* sector number to delete */
	    ioreq.irSectorCount = arg1;	/* number of sectors to delete */
	    break;
	
	case TFFS_DEFRAGMENT_VOLUME:
	    function = FL_DEFRAGMENT_VOLUME;
	    ioreq.irLength = arg0;	/* minimum number of sectors to get */
	    break;
	
	default:
        if (tffsBlkDevs[tffsDriveNo] != NULL)
            semGive(tffsBlkDevs[tffsDriveNo]->tffsSem);
	    return (ERROR);
	}

    status = flCall (function, &ioreq);

    switch (functionNo)
	{
	case TFFS_DEFRAGMENT_VOLUME:
	    *(int *)arg1 = ioreq.irLength; /* min number of sectors gotten */
	    break;
	}

    if (tffsBlkDevs[tffsDriveNo] != NULL)
        semGive(tffsBlkDevs[tffsDriveNo]->tffsSem);

    return ((status == flOK) ? OK : ERROR);
    }
#if     (POLLING_INTERVAL > 0)
/*******************************************************************************
*
* flPollSemCreate - create semaphore for polling task
*
* This routine creates the semaphore used to delay socket polling until
* TrueFFS initialization is complete.
*
* RETURNS: flOK, or flNotEnoughMemory if it fails.
*/

LOCAL FLStatus flPollSemCreate (void)
    {
    if ((flPollSemId = semBCreate (SEM_Q_PRIORITY, SEM_EMPTY)) == NULL)
        return (flNotEnoughMemory);
    else
        return (flOK);
    }
#endif /* (POLLING_INTERVAL > 0) */

