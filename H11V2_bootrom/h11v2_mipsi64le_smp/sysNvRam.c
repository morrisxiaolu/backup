/* sysNvRam.c - CFE interface to serial nvram */

/* Copyright (c) 2001-2002,2004,2006,2007 Wind River Systems, Inc.
*
* The right to copy, distribute, modify or otherwise make use
* of this software may be licensed only pursuant to the terms
* of an applicable Wind River license agreement.
*/

/*
**********************************************************************
*
*  Copyright 2000,2001
*  Broadcom Corporation. All rights reserved.
*
*  This software is furnished under license to Wind River Systems, Inc.
*  and may be used only in accordance with the terms and conditions
*  of this license.  No title or ownership is transferred hereby.
***********************************************************************
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
01v,12nov07,pgh  Fix WIND00110399. Target lock up on NVRAM write with GNU.
01u,13aug07,slk  remove deprecated APIs
01t,13mar07,rlg  Defect WIND00090615 Fix WIND00090617
01s,22dec06,jmt  Fix a merge problem that dropped a fix
01r,21nov06,rlg  NV ram write problems.
01t,19oct06,jmt  Fix problem with partial string writes with sysNvRamSet
01s,19oct06,pes  Fix compiler warnings.
01r,25sep06,pes  Fix compilation warnings.
01q,18jul06,jmt  Merge in changes from bcm1250
01p,03jul06,jmt  Replace malloc and free to make a Layer 1 function
01o,19jun06,rlg  conversion to CFE and bootApp
01n,17may06,pes  Use cfe_getenv/cfe_setenv_p in sysNvRamGet/sysNvRamSet.
01m,24apr06,slk  SPR 117773: fix bug writing strings to NVRam
01l,14mar06,pes  Use sibyte.h to determine board constants.
01k,08nov04,mdo  Documentation fixes for apigen
01j,05may04,agf  fix compiler warnings
01i,18dec02,agf  modify sysNvRamSet to pass nvramEnvSet return value
01h,18dec02,agf  set M24LV128_EEPROM define to 1
01g,18dec02,agf  correct env driver logic and variable type errors
01f,04oct02,agf  fix call for writing to M24LV128; disable DEBUG msgs
01e,03oct02,agf  changes for shared sentosa support
01d,20jun02,pgh  Change path to bcm1250Lib.h.
01c,13jun02,pgh  Fix SPR 76014 and SPR 76016.
01b,26mar02,tlc  Clean up compiler warnings.
01a,08jan01,agf  written
*/

/*
DESCRIPTION

INCLUDE FILES:
*/

/* includes */

#include <vxWorks.h>
#include <memPartLib.h>
#include <semLib.h>
#include "config.h"

#include "sibyte.h"
#include "sb1.h"
#if (BCM_FAMILY == BCM_SB1)
#include <drv/multi/sb1Lib.h>
#elif (BCM_FAMILY == BCM_SB1A)
#include <drv/multi/bcm1480Lib.h>
#else
#error "Unknown BCM_FAMILY"
#endif
#include "sysNvRam.h"
#include "x1240RtcEeprom.h"
#include "m24lv128Eeprom.h"
#include "sibyte.h"

#ifdef INCLUDE_CFE_SUPPORT
#include <../src/hwif/fw/cfe/cfe_api.h>
#endif /* INCLUDE_CFE_SUPPORT */

/* defines */

#define ENV_EXIST        1
#define ENV_NONEXIST     0

#define X1240_EEPROM     0
#define M24LV128_EEPROM  1

#define DEBUG_SMBUS_IO   0
#define DEBUG_NV_RAM     1

typedef struct
    {
    int        flag;
    uint8_t *  ptr;
    int        len;
    } env_t;

#define  LPC_FLASH_SECSIZE           0x1000
#define  LPC_FLASH_OFFS              0x0007e000

#define  PFLASH_MAX_TIMEOUT 16000

/* externals */
#ifdef INCLUDE_CFE_SUPPORT
IMPORT int	cfe_setenv_p (char *name, char *val);
#endif /* INCLUDE_CFE_SUPPORT */

/* forward declarations */

extern int lpc_flash_isbusy_sst(int what, int offset, int erase);
extern int lpc_flash_erase_sector_sst(int offset);
extern int lpc_flash_erase_chip_sst(void);
extern int lpc_flash_program_sst(int pa, unsigned char *pd);
extern int lpc_flash_reset_sst(void);
extern int lpc_flash_write_protect_unlock(int offset);
extern int lpc_flash_write_protect_lock(int offset);


/* globals */
static unsigned long lpc_flash_map_base=0xbfc00000 ;


/* locals */

/* This local buffer is used by the read/write routines. */
LOCAL char *    buf = NULL;

LOCAL int loopNvram = 0;

#define BIG_LOOP_NVRAM     for( loopNvram=500000000;loopNvram>0;loopNvram--);
#define SMALL_LOOP_NVRAM   for( loopNvram=1000;loopNvram>0;loopNvram--);

/******************************************************************************
*
* strnchr - search a string
*
* This routine searches a string until, at most N characters or the first
* occurrence of a given character.
*
* RETURNS: pointer to string or NULL
*
* ERRNO
*/

static char * strnchr
    (
    const char *    dest,
    char            c,
    unsigned int    cnt
    )
    {
    while (*dest && (cnt > 0))
        {
        if (*dest == c)
            return (char *) dest;
        dest++;
        cnt--;
        }
    return (char *)NULL;
    }


/******************************************************************************
*
* nvramEnvSearch - search a character array
*
* This routine searches the given character array of environment variables
* for a given name.
*
* RETURNS: OK - env->flag: ENV_EXIST, or ERROR - env->flag: ENV_NONEXIST
*
* ERRNO
*/

static STATUS nvramEnvSearch
    (
    char *     name,
    uint8_t *  buf,
    int        size,
    env_t *    env
    )
    {
    uint8_t * ptr;
    unsigned char * envval;
    unsigned int    offset;
    unsigned int    flg;
    unsigned int    reclen;
    unsigned int    rectype;

    ptr = buf;
    offset = 0;

    while ((*ptr != ENV_TLV_TYPE_END)  && (size > 1))
        {
        /* Adjust pointer for TLV type */
        rectype = *ptr++;
        offset++;
        size--;

        /*
         * Read the length.  It can be either 1 or 2 bytes
         * depending on the code
         */
        if (rectype & ENV_LENGTH_8BITS)
            {
            /* Read the record type and length - 8 bits */
            reclen = *ptr++;
            size--;
            offset++;
            }
        else {
            /* Read the record type and length - 16 bits, MSB first */
            reclen = ((unsigned int) *(ptr) << 8) + *(ptr + 1);
            ptr += 2;
            size -= 2;
            offset += 2;
            }

        if (reclen > size)
            return ERROR;       /* should not happen, bad NVRAM */

        switch (rectype)
            {
            case ENV_TLV_TYPE_ENV:
                /* Read the TLV data */
                flg = *ptr++;
                envval = (unsigned char *) strnchr ((char *)ptr, '=', (reclen - 1));
                if (envval && !strncmp (name, (char *)ptr, strlen (name)) &&
                    (int)(ptr + strlen (name)) == (int)envval )
                    {
                    env->ptr  = ptr + strlen(name) + 1;
                    env->len = reclen - strlen(name) - 2;
                    env->flag = ENV_EXIST;
                    return OK;
                    }
                break;

            default:
                /* Unknown TLV type, skip it. */
                break;
            }

        /*
         * Advance to next TLV
         */

        size -= (int)reclen;
        offset += reclen;
        ptr = buf + offset;
        }

    env->flag = ENV_NONEXIST;
    env->ptr = (uint8_t *)ptr;
    env->len = size;

    return OK;
    }


/******************************************************************************
*
* nvramEnvGet - get the value associated to the name in non-volatile RAM
*
* This routine copies the value that associated to the specified name in CFE
* environment stored in non-volatile memory into a specified string. The string
* will be terminated with an EOS.
*
* RETURNS: OK, or ERROR if access is outside the non-volatile RAM range.
*
* ERRNO
*/

STATUS nvramEnvGet
    (
    char *  name,
    char *  string,
    int     strLen
    )
    {
    int         res, retlen;
    uint8_t *   ptr;
    env_t       env;
#if DEBUG_SMBUS_IO
    int         boardRev;
#endif
#if DEBUG_NV_RAM
    int         idx;
#endif

#if DEBUG_SMBUS_IO
    boardRev = G_SYS_CONFIG(SBREADCSR(A_SCD_SYSTEM_CFG)) & 0x3;
    printf("board rev: %x\n", boardRev);
#endif

    /* It is also possible to determine which EEPROM to use based on
     * the board revision number; however, the relationship is different
     * between the Swarm and Sentosa, and for maintenance reasons this 
     * module is being used by both of them
     */

    if (x1240RtcEepromOpen (X1240_SMBUS_CHAN) == OK)
        {
#if DEBUG_SMBUS_IO
        printf("found x1240 EEPROM\n");
#endif
        if (buf == NULL)
            {
            buf = KMEM_ALLOC (X1241_EEPROM_SIZE);
            }

        res = x1240RtcEepromRead (X1240_SMBUS_CHAN, 0, (unsigned char *)buf,
                                  X1241_EEPROM_SIZE, &retlen);

        x1240RtcEepromClose ();
        }
    else if (m24lv128EepromOpen (M24LV128_SMBUS_CHAN) == OK)
        {
#if DEBUG_SMBUS_IO
        printf("found m24lv128 EEPROM\n");
#endif
        if (buf == NULL)
            {
            buf = KMEM_ALLOC (M24LV128_EEPROM_SIZE);
            }

        res = m24lv128EepromRead (M24LV128_SMBUS_CHAN, 0, (unsigned char *)buf,
                                  M24LV128_EEPROM_SIZE, &retlen);

        m24lv128EepromClose ();
        }
    else
        {
#if DEBUG_SMBUS_IO
        printf ("could not open EEPROM for envGet\n");
#endif
        return ERROR;
        }

#if DEBUG_NV_RAM
    printf ("Offset %d Result %d\n", 0, res);
    for (idx = 0; idx < 512; idx++)
        {
        if ((idx % 16) == 0)
            printf ("\n%03x: ", idx);
        printf ("%02X ", buf[idx]);
        }
    printf ("\n");
#endif

    if ((nvramEnvSearch (name, (unsigned char *)buf, retlen, &env) == ERROR) ||
        (env.flag == ENV_NONEXIST) ||
        (strLen >= retlen))
        {
#ifndef INCLUDE_MEM_ALLOT
        KMEM_FREE (buf);
        buf = NULL;
#endif  /* ifndef INCLUDE_MEM_ALLOT */
        return ERROR;
        }
    else
        {
        ptr = env.ptr;
        memcpy (string, ptr, strLen);
        }

#ifndef INCLUDE_MEM_ALLOT
    KMEM_FREE (buf);
    buf = NULL;
#endif  /* ifndef INCLUDE_MEM_ALLOT */
    return OK;
    }

/******************************************************************************
*
* nvramEnvSet - set the value and its associated name in non-volatile RAM
*
* This routine sets the value and its associated name to CFE environment
* stored in non-volatile memory.  The value will be appended with 0xff, if
* string length of value is smaller than specified envLen.
*
* RETURNS: OK, or ERROR if access is outside the non-volatile RAM range
*
* ERRNO
*/

STATUS nvramEnvSet
    (
    char *  name,
    char *  string,
    int     envLen
    )
    {
    int         res, retlen, smbusDev;
    uint8_t *   ptr;
    env_t       env;
#if DEBUG_SMBUS_IO
    int         boardRev;
#endif
#if DEBUG_NV_RAM
    int         idx;
#endif

#if DEBUG_SMBUS_IO
    boardRev = G_SYS_CONFIG(SBREADCSR(A_SCD_SYSTEM_CFG)) & 0x3;
    printf("board rev: %x\n", boardRev);
#endif

    /* It is also possible to determine which EEPROM to use based on
     * the board revision number; however, the relationship is different
     * between the Swarm and Sentosa, and for maintenance reasons this 
     * module is being used by both of them
     */

    if (x1240RtcEepromOpen (X1240_SMBUS_CHAN) == OK)
        {
        if (buf == NULL)
            {
            buf = KMEM_ALLOC (X1241_EEPROM_SIZE);
            }

        res = x1240RtcEepromRead (X1240_SMBUS_CHAN, 0, (unsigned char *)buf,
                                  X1241_EEPROM_SIZE, &retlen);

        smbusDev = X1240_EEPROM;    /* save for EepromWrite () call */
        }
    else if (m24lv128EepromOpen (M24LV128_SMBUS_CHAN) == OK)
        {
        if (buf == NULL)
            {
            buf = KMEM_ALLOC (M24LV128_EEPROM_SIZE);
            }

        res = m24lv128EepromRead (M24LV128_SMBUS_CHAN, 0, (unsigned char *)buf,
                                  M24LV128_EEPROM_SIZE, &retlen);

        smbusDev = M24LV128_EEPROM; /* save for EepromWrite () call */
        }
    else
        {
#if DEBUG_SMBUS_IO
        printf ("could not open EEPROM for envSet");
#endif
        return ERROR;
        }


    if (nvramEnvSearch (name, (unsigned char *)&buf[0], retlen, &env) == ERROR)
        {
#ifndef INCLUDE_MEM_ALLOT
        KMEM_FREE (buf);
        buf = NULL;
#endif  /* ifndef INCLUDE_MEM_ALLOT */
        x1240RtcEepromClose ();
        m24lv128EepromClose ();

        return ERROR;
        }

    if (env.flag == ENV_NONEXIST)
        {
        /* type + len + flg + name + '=' + value + end */
        if ((1 + 1 + 1 + strlen (name) + 1 + envLen + 1) > env.len)
            {
#ifndef INCLUDE_MEM_ALLOT
            KMEM_FREE (buf);
            buf = NULL;
#endif  /* ifndef INCLUDE_MEM_ALLOT */
            x1240RtcEepromClose ();
            m24lv128EepromClose ();
            return ERROR;
            }

        ptr = env.ptr;

        *ptr++ = ENV_TLV_TYPE_ENV;                 /* type */
        *ptr++ = strlen (name) + envLen + 1 + 1;   /* length */
        *ptr++ = ENV_FLG_NORMAL;                   /* flag */
        memcpy (ptr, name, strlen (name));         /* name */
        ptr += strlen (name);
        *ptr++ = '=';                              /* '=' */
        memcpy (ptr, string, envLen);              /* value */
        ptr += envLen;
        *ptr = ENV_TLV_TYPE_END;                   /* type_end */

        /* increase 'envLen' by environment overhead so call to
         * EepromWrite routine has proper size field
         */

        envLen += 5 + strlen (name);
        }
    else
        {
#if DEBUG_NV_RAM
        printf ("strlen %d env.len %d\n", envLen, env.len);
#endif
        if (envLen > NV_RAM_SIZE)
            {
#ifndef INCLUDE_MEM_ALLOT
            KMEM_FREE (buf);
            buf = NULL;
#endif  /* ifndef INCLUDE_MEM_ALLOT */
            x1240RtcEepromClose ();
            m24lv128EepromClose ();
            return ERROR;
            }
        ptr = env.ptr;
        memcpy (env.ptr, string, envLen);       /* copy value */
        ptr += envLen;

        /* fill in 0xff to space left */

        while (envLen < env.len)
            {
            *ptr++ = 0xFF;
            envLen++;
            }
        }

#if DEBUG_NV_RAM
    for (idx = 0; idx < 512; idx++)
        {
        if ((idx % 16) == 0)
            printf ("\n%03x: ", idx);
        printf ("%02X ", buf[idx]);
        }
    printf("\n");
#endif

    switch (smbusDev)
        {
        case X1240_EEPROM:
            res = x1240RtcEepromWrite (X1240_SMBUS_CHAN, 
                                       ((int)env.ptr - (int)buf), 
                                       env.ptr,
                                       envLen, 
                                       &retlen);
           break;

        case M24LV128_EEPROM:
            res = m24lv128EepromWrite (M24LV128_SMBUS_CHAN, 
                                       ((int)env.ptr - (int)buf), 
                                       env.ptr,
                                       envLen, 
                                       &retlen);
           break;

        }

#if DEBUG_SMBUS_IO
    printf ("eepromWrite() Result %d, retlen = %d\n", res, retlen);
#endif

#ifndef INCLUDE_MEM_ALLOT
    KMEM_FREE (buf);
    buf = NULL;
#endif  /* ifndef INCLUDE_MEM_ALLOT */
    x1240RtcEepromClose ();
    m24lv128EepromClose ();
    return OK;
    }


/*
 *  Erase the flash device(s) addressed.
 */
int lpc_flash_erase_device(void *base, int size)
{
	int boffs, ok;
	int timeout;
	unsigned long offset;
	unsigned long  ulLoop  = 0;
	

	offset = (unsigned long)base - lpc_flash_map_base;
	
	lpc_flash_write_protect_unlock(0);
	
	boffs = (int)offset;

	/*
	 * Not entire flash or no BULK erase feature. We
	 * use sector/block erase.
	 */

	if(lpc_flash_erase_sector_sst(boffs) != 0) 
	{
		printf("\nError: Failed to enter erase mode\n");

		lpc_flash_reset_sst();
		lpc_flash_write_protect_lock(0);
		return(-4);
	}

	BIG_LOOP_NVRAM;

	for(timeout = 0 ; ((ok = lpc_flash_isbusy_sst(-1 , boffs, TRUE)) == 1) && (timeout < PFLASH_MAX_TIMEOUT); timeout++) 
	{
        BIG_LOOP_NVRAM;
	}

    BIG_LOOP_NVRAM
    
	if(timeout >= PFLASH_MAX_TIMEOUT)
	{
		printf("\nError: erase fail\n");
	}
	
	lpc_flash_reset_sst( );
	lpc_flash_write_protect_lock(0);

    BIG_LOOP_NVRAM
	return(ok);
}

/*
 *  Program a flash device. Assumed that the area is erased already.
 */
int lpc_flash_program_device(void *lpc_flash_base, void *data_base, int data_size)
{
	int ok;
	int i, off;

	off = (int)((unsigned long)lpc_flash_base - lpc_flash_map_base);

	lpc_flash_write_protect_unlock(0);
	ok =0;

	for(i = 0; i < data_size; i++) 
	{
		//printf("offset : %x , data_base :  %c \n",off,*(unsigned char *)data_base);
		ok = lpc_flash_program_sst(off, data_base);

		off++;
		data_base = (void *)((unsigned long)data_base + 1);
	}

	lpc_flash_reset_sst( );	
	lpc_flash_write_protect_lock(0);

	BIG_LOOP_NVRAM
	
	return(ok);
}

void lpcFlashReadParam(char * param, int  strLen, int  offset)
{
	char * buf;
	char *base;
	int i;

	buf = (char *)(param + offset);
	base = (char *)0xbfc7e000;

	//printstr("read param addr: "); printnum((unsigned int)buf); printstr("\r\n");
	for(i = 0; i<strLen; i++)
	{
		 *(buf + i) = *(base + i);
		 SMALL_LOOP_NVRAM;
	}
}

STATUS lpcFlashWriteParam(char * param, int  strLen, int  offset)
{
	char *buf;
	char *base;
/*	unsigned int base0;
	unsigned int addr;
	unsigned int secNum;
	unsigned int i;*/

	buf = (char *)(param + offset);
	base = (char *)0xbfc7e000;
	
        if(lpc_flash_erase_device(base, LPC_FLASH_SECSIZE)) 
        {
                printf("Error! LPC Flash erase failed!\n");
                return(ERROR);
        }

        if(lpc_flash_program_device(base, buf, strLen)) 
        {
                printf("Error! LPC Flash program failed!\n");
                return(ERROR);
        }

        return(OK);
}


#define	FLASH_BASE_ADRS		        0xb0000000	/* Flash memory base address   */
#define	FLASH_SIZE		            0x04000000	/* Flash memory size     */
#define S29GL_MTD_SECTOR_SIZE       (0x20000)/* 128KB */   

#define S29GL_UNLOCK_ADDR1          0x555
#define S29GL_UNLOCK_ADDR2          0x2aa
#define S29GL_UNLOCK_DATA1          ((UINT16)0xaa)
#define S29GL_UNLOCK_DATA2          ((UINT16)0x55)

extern  int s29GlWrite1   ( UINT16 * addr,   UINT16     value   );
extern int s29glSectorRangeErase1   ( int sectorNum,    int sectorCount    ) ;
#define FLASH_REGISTER_WR(addr,data)      *(volatile UINT16 *)((volatile UINT16 *)(FLASH_BASE_ADRS+(UINT32)(addr)))=(UINT16)(data)   
#define FLASH_REGISTER_RD(addr)         (*(volatile UINT16 *)((volatile UINT16 *)(FLASH_BASE_ADRS+(UINT32)(addr))))   

#define S29GL_ENTRY_BYPASS \
    do  \
    {   \
        FLASH_REGISTER_WR(0x00, 0xf0);   \
        FLASH_REGISTER_WR(S29GL_UNLOCK_ADDR1 * 2,S29GL_UNLOCK_DATA1);      \
        FLASH_REGISTER_WR(S29GL_UNLOCK_ADDR2 * 2,S29GL_UNLOCK_DATA2);      \
        FLASH_REGISTER_WR(S29GL_UNLOCK_ADDR1 * 2,0x20);      \
    } while((0));

 #define S29GL_RESET_BYPASS \
    do  \
    {   \
        FLASH_REGISTER_WR(0x00, 0xf0);   \
        FLASH_REGISTER_WR(S29GL_UNLOCK_ADDR1 * 2,0x90);      \
        FLASH_REGISTER_WR(S29GL_UNLOCK_ADDR1 * 2,0x00);      \
    } while((0));
   

    
void s29glFlashReadParam(char * param, int  strLen, int  offset)
{
	char * buf;
	char *base;
	int i;

	buf = (char *)(param + offset);
	base = (char *)(FLASH_BASE_ADRS + FLASH_SIZE - S29GL_MTD_SECTOR_SIZE);  /* ×îºóµÄsector */

	//printstr("read param addr: "); printnum((unsigned int)buf); printstr("\r\n");
	for(i = 0; i<strLen; i++)
	{
		 *(buf + i) = *(base + i);
		 SMALL_LOOP_NVRAM;
	}
}


STATUS s29glFlashWriteParam(char * param, int  strLen, int  offset)
{
	char *buf;
	char *base;
/*	unsigned int base0;
	unsigned int addr;
	unsigned int secNum;
	unsigned int i;*/
	int temp=0;

    char *bufferTmp  ;

	buf = (char *)(param + offset);
	base = (char *)(FLASH_BASE_ADRS + FLASH_SIZE - S29GL_MTD_SECTOR_SIZE);
	
        if(s29glSectorRangeErase1((((UINT32)base-FLASH_BASE_ADRS)/S29GL_MTD_SECTOR_SIZE), 1)) 
        {
                printf("Error! LPC Flash erase failed!\n");
                return(ERROR);
        }

        #if 1
        /* start write data to fpga */
        bufferTmp = buf ;
        temp = strLen;
         
        S29GL_ENTRY_BYPASS;
        while(temp > 0)
        {
            s29GlWrite1((UINT16 *)(base+(strLen-temp)), *((UINT16*)bufferTmp) );
            
            temp -=2;
            bufferTmp +=2;
        }
        S29GL_RESET_BYPASS;
        #endif
        return(OK);
}


/******************************************************************************
*
* sysNvRamGet - get the contents of non-volatile RAM
*
* This routine copies the contents of non-volatile memory into a specified
* location.  The data will be terminated with an EOS.
*
* RETURNS: OK, or ERROR if access is outside the non-volatile RAM range.
*
* ERRNO
*
* SEE ALSO: sysNvRamSet()
*/

STATUS sysNvRamGet
    (
    char *string,    /* where to copy non-volatile RAM           */
    int  strLen,     /* maximum number of bytes to copy          */
    int  offset      /* byte offset into non-volatile RAM        */
    )
    {
    char s[NV_RAM_SIZE+1];

#if  DEBUG_NV_RAM 
    printf("string @ 0x%x, length = %d, offset = %d\n", (int)string, strLen, offset);
    printf("BOOT_LINE_ADRS @ 0x%x \n", BOOT_LINE_ADRS);
#endif
    if ((strLen < 0) || (offset < 0) || ((offset + strLen) > NV_RAM_SIZE))
        return (ERROR);

    bzero (s, NV_RAM_SIZE+1);

#ifdef INCLUDE_LPC_FLASH
	lpcFlashReadParam(s, strLen, offset);

#else

    s29glFlashReadParam(s, strLen, offset);
	//sprintf(s, "atse(0,0)host:vxWorks h=192.168.0.116   e=192.168.0.115 u=mxl pw=mxl");

#endif    

    #if 0
    key = intCpuLock ();

    
#ifdef INCLUDE_CFE_SUPPORT
    zz = cfe_getenv ("vxwbootline", s, NV_RAM_SIZE);
#else /* INCLUDE_CFE_SUPPORT */
    zz = nvramEnvGet ("vxwbootline", s, NV_RAM_SIZE);
#endif /* INCLUDE_CFE_SUPPORT */
    intCpuUnlock (key);


    if (zz != OK)
        {
#if DEBUG_SMBUS_IO
        printf ("could not find vxwBootline in nvRAM\n");
#endif
        return (ERROR);
        }
    #endif
    
    bcopy ((s + offset), string, strLen);
    string[strLen+offset] = EOS;

#if DEBUG_NV_RAM
    printf("vxwbootline: %s\n", string);
#endif

    return (OK);
    }


/*******************************************************************************
*
* sysNvRamSet - write to non-volatile RAM
*
* This routine copies a specified data into non-volatile RAM.
*
* RETURNS: OK if able to write to non-volatile RAM.
*          ERROR if access is outside the non-volatile RAM range.
*          ERROR if unable to create a semaphore.
*
* ERRNO
*
* SEE ALSO: sysNvRamGet()
*/

STATUS sysNvRamSet
    (
    char *  string,     /* string to be copied into non-volatile RAM */
    int     strLen,       /* maximum number of bytes to copy           */
    int     offset        /* byte offset into non-volatile RAM         */
    )
    {
    char    s[NV_RAM_SIZE+1];

    if ((strLen < 0) || (offset < 0) || ((offset + strLen) > NV_RAM_SIZE))
        return (ERROR);


    bzero (s, NV_RAM_SIZE+1);
    strncpy (s+offset, string, strLen);
    

#ifdef INCLUDE_LPC_FLASH
   /*  printf("enter my sysNvRamSet\n"); */

    if(lpcFlashWriteParam(s, (strLen+1), offset) != OK)
    {
	    return (ERROR);
	}
#else
    if(s29glFlashWriteParam(s, (strLen+1), offset) != OK)
    {
	    return (ERROR);
	}
     
#endif

#if 0
#ifdef INCLUDE_CFE_SUPPORT
    cfe_getenv ("vxwbootline", s, NV_RAM_SIZE);
#else /* INCLUDE_CFE_SUPPORT */
    nvramEnvGet ("vxwbootline", s, NV_RAM_SIZE);
#endif /* INCLUDE_CFE_SUPPORT */
    strncpy (s+offset, string, strLen);

#ifdef INCLUDE_CFE_SUPPORT
    zz = cfe_setenv_p("vxwbootline", s);
#else /* INCLUDE_CFE_SUPPORT */
    zz = nvramEnvSet ("vxwbootline", s, NV_RAM_SIZE);
#endif /* INCLUDE_CFE_SUPPORT */
#endif
    /* Give semaphore to allow other processes from updating NVRAM */


    return (OK);
    }



