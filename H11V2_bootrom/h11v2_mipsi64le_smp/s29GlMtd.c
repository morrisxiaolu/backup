/* s29glMtd.c - TrueFFS MTD for   s29glxxxN devices */  
   
  
/* includes */  
  
#include <vxWorks.h>   
#include <taskLib.h>   
#include <logLib.h>   
#include <stdio.h>   
#include <cacheLib.h>   
  
#include "tffs/flflash.h"   
#include "tffs/backgrnd.h"   
  
/* defines */  
  
#define S29GL_MTD_SECTOR_SIZE           (0x20000)/* 128KB */   
#define S29GL_128_CHIP_SIZE             (0x1000000) /* 16MB */   
#define S29GL_128_LAST_SECTOR_NUM       (S29GL_128_CHIP_SIZE / S29GL_MTD_SECTOR_SIZE - 1) /* 127 */   
  
#define S29GL_256_CHIP_SIZE             (0x2000000) /* 32MB */   
#define S29GL_256_LAST_SECTOR_NUM       (S29GL_256_CHIP_SIZE / S29GL_MTD_SECTOR_SIZE - 1) /* 255 */   
  
#define S29GL_512_CHIP_SIZE             (0x4000000) /*mxl: 64MB  32MB */   
#define S29GL_512_LAST_SECTOR_NUM       (S29GL_512_CHIP_SIZE / S29GL_MTD_SECTOR_SIZE - 1) /* 512 */   
  
#define S29GL_RSV_FOR_BOOTROM           0x100000   /*mxl del (0x100000)  1MB */   
#define S29GL_FIRST_SECTOR_NUM          (S29GL_RSV_FOR_BOOTROM / S29GL_MTD_SECTOR_SIZE ) /* 8 */   
  
#define S29GL_MTD_CHIP_CNT              (1)   
#define S29GL_MTD_INTERLEAVE            (1)   


#define	FLASH_BASE_ADRS		0xb0000000	/* Flash memory base address  mxl modified*/
#define	FLASH_SIZE		    0x04000000	/* Flash memory size   mxl modified */

#define FLASH_REGISTER_WR(addr,data)      *(volatile UINT16 *)((volatile UINT16 *)(FLASH_BASE_ADRS+(UINT32)(addr)))=(UINT16)(data)   
#define FLASH_REGISTER_RD(addr)         (*(volatile UINT16 *)((volatile UINT16 *)(FLASH_BASE_ADRS+(UINT32)(addr))))   

/* 8 bits write & read */

//#define FLASH_REGISTER_WR(addr,data)    FLASH_REGISTER_WRX16(addr,data)   //*((volatile UINT8 *)(FLASH_BASE_ADRS+addr))=(UINT8)data   
//#define FLASH_REGISTER_RD(addr)         FLASH_REGISTER_RDX16(addr)   //(*((volatile UINT8 *)(FLASH_BASE_ADRS+addr)))   

#define S29GL_FLASH_BIT             1  /* 16-bit */
#define S29GL_FLASH_WIDTH           2

#if (S29GL_FLASH_WIDTH == 1)
//typedef volatile UINT8 *            S29GL_FLASH_ADRS;
//typedef volatile UINT8              S29GL_FLASH_DATA;
//#define S29GL_WRITE_BYTE_ONCE       1
//#define S29GL_WRITE_ADDR_MASK       0
#endif

#if (S29GL_FLASH_WIDTH == 2)
typedef volatile UINT16 *           S29GL_FLASH_ADRS ;
typedef volatile UINT16             S29GL_FLASH_DATA ;
#define S29GL_WRITE_BYTE_ONCE       2
#define S29GL_WRITE_ADDR_MASK       1
#endif


#if   (S29GL_FLASH_BIT == 0)        /* 8-bit */
//#define S29GL_UNLOCK_ADDR1          0xaaa
//#define S29GL_UNLOCK_ADDR2          0x555
#elif (S29GL_FLASH_BIT == 1)        /* 16-bit */
#define S29GL_UNLOCK_ADDR1          0x555
#define S29GL_UNLOCK_ADDR2          0x2aa
#endif

#define S29GL_UNLOCK_DATA1          ((S29GL_FLASH_DATA)0xaa)
#define S29GL_UNLOCK_DATA2          ((S29GL_FLASH_DATA)0x55)

#define S29GL_CMD_RESET             ((S29GL_FLASH_DATA)0xF0)
#define S29GL_CMD_READ_ID           ((S29GL_FLASH_DATA)0x90)
#define S29GL_CMD_ERASE_SECTOR1     ((S29GL_FLASH_DATA)0x80)
#define S29GL_CMD_ERASE_SECTOR2     ((S29GL_FLASH_DATA)0x30)
#define S29GL_CMD_PROGRAM           ((S29GL_FLASH_DATA)0xA0)

#define S29GL_DQ6_MASK              ((S29GL_FLASH_DATA)0x40)

#define S29GL_OP_TIMEOUT            5000
#define S29GL_OP_DELAY              10

#define S29GL_SECTOR_MASK           (~(S29GL_MTD_SECTOR_SIZE - 1))

//#define S29GL_DEBUG_ON

#ifdef S29GL_DEBUG_ON
#ifndef S29GL_DBG_MSG
#define S29GL_DBG_MSG(...)      printf(__VA_ARGS__)
#endif
#else
#define S29GL_DBG_MSG(...)
#endif

/* flash ops */

#define S29GL_RESET(addr)   \
    do  \
    {   \
        *(volatile UINT16 *)(addr) = S29GL_CMD_RESET;    \
    } while((0));

#define S29GL_ERASE(addr)   \
    do  \
    {   \
        FLASH_REGISTER_WR(S29GL_UNLOCK_ADDR1 * 2, S29GL_UNLOCK_DATA1);      \
        FLASH_REGISTER_WR(S29GL_UNLOCK_ADDR2 * 2, S29GL_UNLOCK_DATA2) ;      \
        FLASH_REGISTER_WR(S29GL_UNLOCK_ADDR1 * 2, S29GL_CMD_ERASE_SECTOR1) ; \
        FLASH_REGISTER_WR(S29GL_UNLOCK_ADDR1 * 2, S29GL_UNLOCK_DATA1) ;      \
        FLASH_REGISTER_WR(S29GL_UNLOCK_ADDR2 * 2, S29GL_UNLOCK_DATA2) ;      \
        *(volatile UINT16 *)(addr) = S29GL_CMD_ERASE_SECTOR2;    \
    } while((0));

#define S29GL_PROGRAM( addr2, value)  \
    do  \
    {   \
        FLASH_REGISTER_WR(S29GL_UNLOCK_ADDR1 * 2,S29GL_UNLOCK_DATA1);  \
        FLASH_REGISTER_WR(S29GL_UNLOCK_ADDR2 * 2,S29GL_UNLOCK_DATA2);   \
        FLASH_REGISTER_WR(S29GL_UNLOCK_ADDR1 * 2,S29GL_CMD_PROGRAM);    \
        *(volatile UINT16 *)(addr2) = (value); \
    } while((0));


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
   
#define S29GL_ERASE_BYPASS(addr)   \
    do  \
    {   \
        FLASH_REGISTER_WR(S29GL_UNLOCK_ADDR2 * 2,S29GL_CMD_ERASE_SECTOR1);      \
        FLASH_REGISTER_WR((addr*0x20000),0x30);   \
    } while((0));

#define S29GL_PROGRAM_BYPASS(  addr2, value)  \
    do  \
    {   \
        FLASH_REGISTER_WR(S29GL_UNLOCK_ADDR1 * 2,S29GL_CMD_PROGRAM);    \
        *(volatile UINT16 *)addr2 = value; \
    } while((0));
    

UINT16 awBuffForOneSector[S29GL_MTD_SECTOR_SIZE];  
  
/* local routines */  
  
 FLStatus s29glSectorRangeErase(FLFlash* pVol, int, int);  
  
 FLStatus s29glProgram(FLFlash*, CardAddress, const void FAR1*, int, FLBoolean);  
  
 void FAR0* s29glMap(FLFlash*, CardAddress, int);  
  
 void flashIdGet(FLFlash*, UINT16*, UINT16*);  

extern UINT32 bsp_printf(const char *  fmt, ...);

char * g_s29Buff =NULL;


#if 1
int s29Erase (int sectorNum )  
{  

    UINT16 data1;
    S29GL_FLASH_ADRS  sector;
    UINT32 timeout = 0, status;
    int i=0;
    sector = (S29GL_FLASH_ADRS)(sectorNum * S29GL_MTD_SECTOR_SIZE);
    S29GL_ERASE((S29GL_FLASH_ADRS )(FLASH_BASE_ADRS+(UINT32)sector ));
    #if 0
        FLASH_REGISTER_WR(S29GL_UNLOCK_ADDR1,S29GL_UNLOCK_DATA1);      
        FLASH_REGISTER_WR(S29GL_UNLOCK_ADDR2,S29GL_UNLOCK_DATA2) ;       
        FLASH_REGISTER_WR(S29GL_UNLOCK_ADDR1,S29GL_CMD_ERASE_SECTOR1) ;  
        FLASH_REGISTER_WR(S29GL_UNLOCK_ADDR1,S29GL_UNLOCK_DATA1) ;       
        FLASH_REGISTER_WR(S29GL_UNLOCK_ADDR2,S29GL_UNLOCK_DATA2) ; 
        FLASH_REGISTER_WR(i,S29GL_CMD_ERASE_SECTOR2) ; 
        //*((S29GL_FLASH_ADRS *)(FLASH_BASE_ADRS+(UINT32)i )) = S29GL_CMD_ERASE_SECTOR2;    
    #endif
     bsp_printf("\r\n s29Erase");

    while (TRUE) 
    {
        status = *(volatile UINT16 *)(FLASH_BASE_ADRS+(sectorNum * S29GL_MTD_SECTOR_SIZE)); 
        
        if (0 == (status & 0x80)) /* data DQ7 */ 
        {   
            if (status & 0x20)  /* DQ5 = 1 */
            {     
                /* read twice */
                status = *(volatile UINT16 *)(FLASH_BASE_ADRS+(sectorNum * S29GL_MTD_SECTOR_SIZE));  

                if (0 == (status & 0x80))  /* toggle */
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
                //wTemp1 = wTemp2;
                continue;
            }
        }
        else 
        { 
            break;   /* program completed */
        }
    }

        
    while((data1 = *(volatile UINT16*)((FLASH_BASE_ADRS + (sectorNum * S29GL_MTD_SECTOR_SIZE)) ) ) !=0xffff )
    bsp_printf("         data1 = %x    \r\n", data1   );


    FLASH_REGISTER_WR(0x00, 0xf0);/* RESET ????*/ 
    return 0;
}
#endif

/****************************************************************************** 
* 
*  - MTD identify routine (see TrueFFS Programmer's Guide) 
* 
* RETURNS: FLStatus 
* 
*/  
  
FLStatus s29glMTDIdentify  
    (  
    FLFlash* pVol  
    )  
{  
    UINT16 manCode;  
    UINT16 devCode;  

    flashIdGet ( pVol, &manCode, &devCode );  

  #if 0
    if (( manCode & 0xff ) != 0x0001 )  
    {  
        return ( flUnknownMedia );  
    }  
  #endif
    if(( 0x227E == devCode )&& ( manCode == 0x01 ))
    {
        printf("s29glMTDIdentify succeed! \r\n");
        pVol->type = 0x017E;  
        pVol->erasableBlockSize = S29GL_MTD_SECTOR_SIZE;  
        pVol->chipSize = S29GL_512_CHIP_SIZE - S29GL_RSV_FOR_BOOTROM;/* Not sure  ?????????*/  
        pVol->noOfChips = S29GL_MTD_CHIP_CNT;  
        pVol->interleaving = S29GL_MTD_INTERLEAVE;  
        pVol->write = s29glProgram;  
        pVol->erase = s29glSectorRangeErase;  
        pVol->map = s29glMap;  

        g_s29Buff = (char *)memalign(4, S29GL_MTD_SECTOR_SIZE); 
        return(flOK);  
    }
    else  
    {  
        printf("s29glMTDIdentify failed!   devCode %#x, manCode %#x\r\n", devCode, manCode);

        return ( flUnknownMedia );  
    }      
      
} 

/*******************************************************************************
*
* s29GlWrite - low level byte programming routine
*
* This routine is called to program byte to the flash part.
*
* RETURNS: flOK or flTimedOut if timeout.
*
* ERRNO: N/A
*/

  __inline__ FLStatus s29GlWrite
    (
    S29GL_FLASH_ADRS    addr,
    S29GL_FLASH_DATA    value
    )
{
    UINT16 *  sector;
    UINT16 secNum;
    UINT32  timeout = 0, status;

    //bsp_printf("\r\n  addr=%#x, oldvalue = %#x, value= %#x", addr, *(UINT16*)addr,value);
    sector = (UINT16 * )((UINT32)addr & S29GL_SECTOR_MASK);

    //FLASH_REGISTER_WR(0x00, 0xf0);

    #if 0 
    S29GL_PROGRAM( addr, value);
    #else
    S29GL_PROGRAM_BYPASS(  addr, value) 
    #endif
    secNum = ((UINT32)sector - FLASH_BASE_ADRS )/ S29GL_MTD_SECTOR_SIZE ;
    //printf("\r\n addr =%#x, value = %x", addr,value);

    /* set timeout = 5s */

   // timeout = flMsecCounter + S29GL_OP_TIMEOUT;

    while (TRUE) 
    {
        timeout++;
        if(timeout > 1000)
            break;
        status = *(volatile UINT16 *)addr; 

        if ((status ^ value) & 0x80 ) /* toggle */ 
        {   
            if (status & 0x20)  /* DQ5 = 1 */
            {     
                /* read twice */
                status = *(volatile UINT16 *)addr; 
                if ((status ^ value) & 0x80 )  /* toggle */
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
                continue;
            }
        }
        else 
        { 
            break;   /* program completed */
        }
    }
    
    
    FLASH_REGISTER_WR(0x00, 0xf0);/* RESET ????*/ 
    //while(value != *addr);

    return flOK;
}
  
/****************************************************************************** 
* 
* s29glProgram - MTD write routine (see TrueFFS Programmer's Guide) 
* 
* RETURNS: FLStatus 
* 
*/  
  
FLStatus s29glProgram  
    (  
    FLFlash*          pVol,  
    CardAddress       address,  
    const void FAR1*  buffer,  
    int               length,  
    FLBoolean         overwrite  
    )  
{  

    UINT8  *unaligned;
    UINT8  *buf = (UINT8 *)buffer;
    UINT32  left = length;
    S29GL_FLASH_ADRS aligned;
    S29GL_FLASH_DATA data;
    UINT32  num;
    int     i;
    STATUS rc = OK;
    volatile UINT8* pFlash =NULL, *pBuffer =NULL;
    BOOL doFree = FALSE;
    
    if (flWriteProtected(vol.socket))
        return flWriteProtect;

    /* calculate the program addr, make sure it's aligned */
    if (overwrite && length == 2)
    {
        int sector;
        int offset;
        
        pFlash = (volatile UINT8*) pVol->map(pVol, address, length);

        buf = (UINT8*) g_s29Buff;
        
        if (buf == 0)
        {
            S29GL_DBG_MSG( "S29GLvProgram: No memory\n");
            return(flBadParameter);
        }
        pBuffer = buf;
        /* Determine sector and offset */

        sector = address / S29GL_MTD_SECTOR_SIZE;
        offset = address % S29GL_MTD_SECTOR_SIZE;

        /* Get a pointer to the flash sector */
        pFlash = (volatile UINT8*) pVol->map(pVol,
                                              sector * S29GL_MTD_SECTOR_SIZE,
                                              S29GL_MTD_SECTOR_SIZE);


        /* Copy the sector from flash to memory */
        memcpy(buf, (void*) pFlash, S29GL_MTD_SECTOR_SIZE);

        /* Overwrite the sector in memory */
        memcpy(((UINT8*) buf) + offset, buffer, length);

        /* Erase sector */
        rc = s29glSectorRangeErase(pVol, sector, 1);
        if (rc != flOK)
        {
            return(rc);
        }

        length = S29GL_MTD_SECTOR_SIZE;     
        
        unaligned = (UINT8 *)vol.map (&vol, sector * S29GL_MTD_SECTOR_SIZE, 0);
        num = (UINT32)unaligned & S29GL_WRITE_ADDR_MASK;
        aligned = (S29GL_FLASH_ADRS)((UINT32)unaligned - num);

        doFree = TRUE;
    }
    else
    {
        unaligned = (UINT8 *)vol.map (&vol, address, 0);
        num = (UINT32)unaligned & S29GL_WRITE_ADDR_MASK;
        aligned = (S29GL_FLASH_ADRS)((UINT32)unaligned - num);
    }

    left = length;
    S29GL_ENTRY_BYPASS;
    
    if (num != 0)
    {
        data = *aligned;

        for (i = num ; i < S29GL_WRITE_BYTE_ONCE; i++)
        {
            data &= ~(0xFF << (((S29GL_WRITE_BYTE_ONCE - 1) - i) * 8));
            data |= ((*(buf + i - num)) << (((S29GL_WRITE_BYTE_ONCE - 1) - i) * 8));
        }

        if (s29GlWrite(aligned, data) != flOK)
        {
            S29GL_RESET_BYPASS;
            goto WriteFault;
        }
        
        buf  += (S29GL_WRITE_BYTE_ONCE - num);
        left -= (S29GL_WRITE_BYTE_ONCE - num);
        aligned++;
    }

    while (left >= S29GL_WRITE_BYTE_ONCE)
    {
        data = *(S29GL_FLASH_ADRS)buf;

        if (s29GlWrite(aligned, data) != flOK)
        {
            S29GL_RESET_BYPASS;
            goto WriteFault;
        }

        buf  += S29GL_WRITE_BYTE_ONCE;
        left -= S29GL_WRITE_BYTE_ONCE;
        aligned++;
    }

    if (left > 0)
    {
        data = *aligned;

        for (i = 0 ; i < left; i++)
            {
            data &= ~(0xFF << (((S29GL_WRITE_BYTE_ONCE - 1) - i) * 8));
            data |= ((*(buf + i)) << (((S29GL_WRITE_BYTE_ONCE - 1) - i) * 8));
            }

        if (s29GlWrite(aligned, data) != flOK)
        {
            S29GL_RESET_BYPASS;
            goto WriteFault;
        }
    }

    #if 1
    if (doFree)
    {
        if (tffscmp((void FAR0 *)pFlash, (void FAR0 *)pBuffer, length))
        {
            {
                S29GL_RESET_BYPASS;
                goto WriteFault;
            }
        }
    }
    else
    {
        if (tffscmp((void FAR0 *)unaligned, (void FAR0 *)buffer, length))
        {
            {
                S29GL_RESET_BYPASS;
                goto WriteFault;
            }
        }
    }
    #endif


    S29GL_RESET_BYPASS;
    return flOK;

    
 WriteFault:

    return flWriteFault;
 
}  
  
/****************************************************************************** 
* 
* s29glSectorRangeErase - MTD erase routine (see TrueFFS Programmer's Guide) 
* 
* RETURNS: FLStatus 
* 
*/  
  
 FLStatus s29glSectorRangeErase  
    (  
    FLFlash* pVol,  
    int sectorNum,  
    int sectorCount  
    )  
{  
    S29GL_FLASH_ADRS sector = NULL;
    S29GL_FLASH_DATA data1;
    UINT32 timeout = 0, status;
    int i;
    
    //   if (flWriteProtected(vol.socket))
   //     return flWriteProtect;
    //S29GL_ENTRY_BYPASS;
    for (i = sectorNum;
         i < sectorNum + sectorCount;
         i++)
        {
        //bsp_printf(".");
        //sector = (S29GL_FLASH_ADRS)vol.map(&vol, i * vol.erasableBlockSize, 0);
        
        sector = (S29GL_FLASH_ADRS)(sectorNum * S29GL_MTD_SECTOR_SIZE);
        S29GL_ERASE((S29GL_FLASH_ADRS )(FLASH_BASE_ADRS+(UINT32)sector ));

        while (TRUE) 
        {
            status = *(volatile UINT16 *)(FLASH_BASE_ADRS+(i * S29GL_MTD_SECTOR_SIZE)); 
            
            if (0 == (status & 0x80)) /* data DQ7 */ 
            {   
                if (status & 0x20)  /* DQ5 = 1 */
                {     
                    /* read twice */
                    status = *(volatile UINT16 *)(FLASH_BASE_ADRS+(i * S29GL_MTD_SECTOR_SIZE));  

                    if (0 == (status & 0x80))  /* toggle */
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
                    continue;
                }
            }
            else 
            { 
                break;   /* program completed */
            }
        }
        
    }

    while((data1 = *(volatile UINT16*)((FLASH_BASE_ADRS + (i-1) * 0x20000) ) ) !=0xffff )
        bsp_printf(" while       data1 = %x    \r\n", data1   );
   
    FLASH_REGISTER_WR(0x00, 0xf0); 

    return flOK;   
}  
  
/****************************************************************************** 
* 
* s29glMap - MTD map routine (see TrueFFS Programmer's Guide) 
* 
* RETURNS: FLStatus 
* 
*/  
void FAR0* s29glMap  
    (  
    FLFlash* pVol,  
    CardAddress address,  
    int length  
    )  
{  
    UINT32 flashBaseAddr = (pVol->socket->window.baseAddress << 12);  
    void FAR0* pFlash = (void FAR0*) (flashBaseAddr + address );  
    
    return(pFlash);  
}  
      
/****************************************************************************** 
* 
* flashIdGet - Get flash man. and device codes. 
* 
* RETURNS: N/A 
* 
*/  
  
void flashIdGet  
    (  
    FLFlash* pVol,  
    UINT16* manCode,  
    UINT16* devCode  
    )  
{  
    FLASH_REGISTER_WR(0x00, 0xf0);/* RESET ????*/  

    FLASH_REGISTER_WR(0x555 * 2,0xaa);  
    FLASH_REGISTER_WR(0x2aa * 2,0x55);  
    FLASH_REGISTER_WR(0x555 * 2,0x90);  
    *manCode = FLASH_REGISTER_RD(0x00);  
    /*printf("FLASH_REGISTER_RD(0x00) = 0x%x \r\n",*manCode);  */

    FLASH_REGISTER_WR(0x00, 0xf0);/* RESET ????*/  
    
    FLASH_REGISTER_WR(0x555 * 2,0xaa);  
    FLASH_REGISTER_WR(0x2aa * 2,0x55);  
    FLASH_REGISTER_WR(0x555 * 2,0x90);  
    *devCode=FLASH_REGISTER_RD(0x02);
    /*printf("FLASH_REGISTER_RD(0x01) = 0x%x \r\n", *devCode);  */

    FLASH_REGISTER_WR(0x00, 0xf0);/* RESET ????*/  
 //   printf("Manufacture ID(0x0001)=%4x, Device ID(0x22xx)=%4x\n",*manCode, *devCode);  
}



int s29GlWrite1
    (
    S29GL_FLASH_ADRS    addr,
    S29GL_FLASH_DATA    value
    )
{
    UINT16 *  sector;
    UINT16 secNum;
    UINT32  timeout = 0, status;

    //bsp_printf("\r\n  addr=%#x, oldvalue = %#x, value= %#x", addr, *(UINT16*)addr,value);
    sector = ( UINT16 * )((UINT32)addr & S29GL_SECTOR_MASK);

    //FLASH_REGISTER_WR(0x00, 0xf0);

    #if 0 
    S29GL_PROGRAM( addr, value);
    #else
    S29GL_PROGRAM_BYPASS(  addr, value) 
    #endif
    secNum = ((UINT32)sector - FLASH_BASE_ADRS )/ S29GL_MTD_SECTOR_SIZE ;
    //printf("\r\n addr =%#x, value = %x", addr,value);

    /* set timeout = 5s */

   // timeout = flMsecCounter + S29GL_OP_TIMEOUT;

    while (TRUE) 
    {
        timeout++;
        if(timeout > 1000)
            break;
        status = *(volatile UINT16 *)addr; 

        if ((status ^ value) & 0x80 ) /* toggle */ 
        {   
            if (status & 0x20)  /* DQ5 = 1 */
            {     
                /* read twice */
                status = *(volatile UINT16 *)addr; 
                if ((status ^ value) & 0x80 )  /* toggle */
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
                continue;
            }
        }
        else 
        { 
            break;   /* program completed */
        }
    }
    
    
    FLASH_REGISTER_WR(0x00, 0xf0);/* RESET ????*/ 
    //while(value != *addr);

    return flOK;
}

int s29glSectorRangeErase1  
    (  
    int sectorNum,  
    int sectorCount  
    )  
{  
    S29GL_FLASH_ADRS sector = NULL;
    S29GL_FLASH_DATA data1;
    UINT32 timeout = 0, status;
    int i;
    //bsp_printf("\r\n s29glSectorRangeErase enter " );
    
    //FLASH_REGISTER_WR(0x00, 0xf0);/* RESET ????*/ 
 //   if (flWriteProtected(vol.socket))
   //     return flWriteProtect;
    //S29GL_ENTRY_BYPASS;
    for (i = sectorNum;
         i < sectorNum + sectorCount;
         i++)
        {
        bsp_printf(".");
        //sector = (S29GL_FLASH_ADRS)vol.map(&vol, i * vol.erasableBlockSize, 0);
        
        //S29GL_DBG_MSG("Erasing sector#%03d @ 0x%08x ...\r", i, (UINT32)sector);
        //bsp_printf("\r\n erase sector = %#x,sectorNum %d, sectorCount %d,", (UINT32)sector, sectorNum, sectorCount);
        //bsp_printf("\r\n erase 0xb4000008 = %#x,0xb400000c %#x ", *(UINT16*)0xb4000008, *(UINT16*)0xb400000c );
        sector = (S29GL_FLASH_ADRS)(sectorNum * S29GL_MTD_SECTOR_SIZE);
        S29GL_ERASE((S29GL_FLASH_ADRS )(FLASH_BASE_ADRS+(UINT32)sector ));

       // 
        //S29GL_ERASE(sector);
        while (TRUE) 
        {
            status = *(volatile UINT16 *)(FLASH_BASE_ADRS+(i * S29GL_MTD_SECTOR_SIZE)); 
            
            if (0 == (status & 0x80)) /* data DQ7 */ 
            {   
                //printf("2222\n");
                if (status & 0x20)  /* DQ5 = 1 */
                {     
                   // printf("333\n");
                    /* read twice */
                    status = *(volatile UINT16 *)(FLASH_BASE_ADRS+(i * S29GL_MTD_SECTOR_SIZE));  

                    if (0 == (status & 0x80))  /* toggle */
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
                    //wTemp1 = wTemp2;
                    continue;
                }
            }
            else 
            { 
                break;   /* program completed */
            }
        }
        


        
    }

     while((data1 = *(volatile UINT16*)((FLASH_BASE_ADRS + (i-1) * 0x20000) ) ) !=0xffff )
     bsp_printf(" while       data1 = %x    \r\n", data1   );
   
    FLASH_REGISTER_WR(0x00, 0xf0); 
    //S29GL_RESET_BYPASS;
    return flOK;   
}

void resetBypass()
{
    S29GL_RESET_BYPASS;
}

void s29Flashx16()
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
      
    FLASH_REGISTER_WR(0x00, 0xf0);/* RESET ????*/ 
    //FLASH_REGISTER_WRX16(0x555,0xaa);  
    //FLASH_REGISTER_WRX16(0x2aa,0x55);  
    //FLASH_REGISTER_WRX16(0x555,0x20);
    for ( i = 0; i < 0x1000; i++ )  
    {
        FLASH_REGISTER_WR(0x00, 0xf0);/* RESET ????*/
        j= (UINT8)i;
        S29GL_PROGRAM( ((volatile UINT16 *)FLASH_BASE_ADRS) + i * 2, j);
        #if 1
        //FLASH_REGISTER_WR(0xaaa,0xaa);  
        //FLASH_REGISTER_WR(0x555,0x55);  
        //FLASH_REGISTER_WR(0xaaa,0xa0);  
        #else
        FLASH_REGISTER_WRX16(0xaaa,0xa0);  
        #endif
        
        //printf("j=%d\n",j);
        //FLASH_REGISTER_WRX16(i, j);

        while (TRUE) 
        {
            wTemp1 = FLASH_REGISTER_RD(0);
            wTemp2 = FLASH_REGISTER_RD(1);
            bsp_printf("\r\n1111  i = %x wTemp1 = %x   wTemp2 = %x", i, wTemp1, wTemp2);
            if ((wTemp1 ^ wTemp2) & 0x40)  /* toggle */ 
            {   
                //printf("2222\n");
                if (wTemp2 & 0x20)  /* DQ5 = 1 */
                {     
                   // printf("333\n");
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
    FLASH_REGISTER_WR(0x00, 0xf0);/* RESET ????*/ 

    
    
#endif

}

#if 1

#if 1
#define	FLASH_BASE_ADRS2		0xb4000000 	/* Flash memory base address  mxl modified*/
//#define	FLASH_BASE_ADRS2		0xbc000000 	/* Flash memory base address  mxl modified*/

#define	FLASH_SIZE2		    0x04000000	/* Flash memory size   mxl modified */

#define FLASH_REGISTER_WR2(addr,data)      *(volatile UINT16 *)((volatile UINT16 *)(0xb4000000+(UINT32)(addr)))=(UINT16)(data)   
#define FLASH_REGISTER_RD2(addr)         (*(volatile UINT16 *)((volatile UINT16 *)(0xb4000000+(UINT32)(addr))))   


#else   /* 0xbfc00000 从pci启动时用来烧写bootrom */
#define	FLASH_BASE_ADRS2		0xbc000000 	/* Flash memory base address  mxl modified*/

#define	FLASH_SIZE2		    0x04000000	/* Flash memory size   mxl modified */

#define FLASH_REGISTER_WR2(addr,data)      *(volatile UINT16 *)((volatile UINT16 *)(0xbfc00000+(UINT32)(addr)))=(UINT16)(data)   
#define FLASH_REGISTER_RD2(addr)         (*(volatile UINT16 *)((volatile UINT16 *)(0xbfc00000+(UINT32)(addr))))   

#endif

#define S29GL_ENTRY_BYPASS2 \
     do  \
     {   \
         FLASH_REGISTER_WR2(0x00, 0xf0);   \
         FLASH_REGISTER_WR2(S29GL_UNLOCK_ADDR1 * 2,S29GL_UNLOCK_DATA1);      \
         FLASH_REGISTER_WR2(S29GL_UNLOCK_ADDR2 * 2,S29GL_UNLOCK_DATA2);      \
         FLASH_REGISTER_WR2(S29GL_UNLOCK_ADDR1 * 2,0x20);      \
     } while((0));
 
 #define S29GL_RESET_BYPASS2 \
     do  \
     {   \
         FLASH_REGISTER_WR2(0x00, 0xf0);   \
         FLASH_REGISTER_WR2(S29GL_UNLOCK_ADDR1 * 2,0x90);      \
         FLASH_REGISTER_WR2(S29GL_UNLOCK_ADDR1 * 2,0x00);      \
     } while((0));
     
#define S29GL_PROGRAM_BYPASS2(  addr2, value)  \
         do  \
         {   \
             FLASH_REGISTER_WR2(S29GL_UNLOCK_ADDR1 * 2,S29GL_CMD_PROGRAM);    \
             *(volatile UINT16 *)addr2 = value; \
         } while((0));
		 
#define S29GL_PROGRAM2( addr2, value)  \
			 do  \
			 {	 \
				 FLASH_REGISTER_WR2(S29GL_UNLOCK_ADDR1 * 2,S29GL_UNLOCK_DATA1);	\
				 FLASH_REGISTER_WR2(S29GL_UNLOCK_ADDR2 * 2,S29GL_UNLOCK_DATA2);	 \
				 FLASH_REGISTER_WR2(S29GL_UNLOCK_ADDR1 * 2,S29GL_CMD_PROGRAM);	 \
				 *(volatile UINT16 *)(addr2) = (value); \
			 } while((0));


#define S29GL_ERASE2(addr)   \
	 do  \
	 {	 \
		 FLASH_REGISTER_WR2(S29GL_UNLOCK_ADDR1 * 2, S29GL_UNLOCK_DATA1); 	 \
		 FLASH_REGISTER_WR2(S29GL_UNLOCK_ADDR2 * 2, S29GL_UNLOCK_DATA2) ;	  \
		 FLASH_REGISTER_WR2(S29GL_UNLOCK_ADDR1 * 2, S29GL_CMD_ERASE_SECTOR1) ; \
		 FLASH_REGISTER_WR2(S29GL_UNLOCK_ADDR1 * 2, S29GL_UNLOCK_DATA1) ;	  \
		 FLASH_REGISTER_WR2(S29GL_UNLOCK_ADDR2 * 2, S29GL_UNLOCK_DATA2) ;	  \
		 *(volatile UINT16 *)(addr) = S29GL_CMD_ERASE_SECTOR2;	  \
	 } while((0));

 int s29GlWrite2
    (
    S29GL_FLASH_ADRS    addr,
    S29GL_FLASH_DATA    value
    )
{
    UINT16 *  sector;
    UINT16  secNum;
    UINT32  timeout = 0, status;

    //bsp_printf("\r\n  addr=%#x, oldvalue = %#x, value= %#x", addr, *(UINT16*)addr,value);
    sector = (UINT16 * )((UINT32)addr & S29GL_SECTOR_MASK);

    //FLASH_REGISTER_WR(0x00, 0xf0);

    #if 1 
    S29GL_PROGRAM2( addr, value);
    #else
    S29GL_PROGRAM_BYPASS2(  addr, value) 
    #endif
    secNum = ((UINT32)sector - FLASH_BASE_ADRS2 )/ S29GL_MTD_SECTOR_SIZE ;
    //printf("\r\n addr =%#x, value = %x", addr,value);

    /* set timeout = 5s */

   // timeout = flMsecCounter + S29GL_OP_TIMEOUT;

    while (TRUE) 
    {
        timeout++;
        if(timeout > 1000)
            break;
        status = *(volatile UINT16 *)addr; 

        //bsp_printf("\r\naddr = %x value = %x status = %x  ", addr,value, status );
        if ((status ^ value) & 0x80 ) /* toggle */ 
        {   
            //printf("2222\n");
            if (status & 0x20)  /* DQ5 = 1 */
            {     
               // printf("333\n");
                /* read twice */
                status = *(volatile UINT16 *)addr; 
                if ((status ^ value) & 0x80 )  /* toggle */
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
                //wTemp1 = wTemp2;
                //FLASH_REGISTER_WR(0x00, 0xf0);                
                continue;
            }
        }
        else 
        { 
            break;   /* program completed */
        }
    }
    
    
    //bsp_printf("\r\n   Newvalue = %#x ",  *(UINT16*)addr );
    FLASH_REGISTER_WR2(0x00, 0xf0);/* RESET ????*/ 
    //while(value != *addr);

    //bsp_printf("\r\ns29GlWrite exit", addr, *(UINT16)addr,value);
    return flOK;
}

int s29glSectorRangeErase2  
    (  
    int sectorNum,  
    int sectorCount  
    )  
{  
    S29GL_FLASH_ADRS sector = NULL;
    S29GL_FLASH_DATA data1;
    UINT32 timeout = 0, status;
    int i;
    //bsp_printf("\r\n s29glSectorRangeErase enter " );
    
    //FLASH_REGISTER_WR(0x00, 0xf0);/* RESET ????*/ 
 //   if (flWriteProtected(vol.socket))
   //     return flWriteProtect;
    //S29GL_ENTRY_BYPASS;
    for (i = sectorNum;
         i < sectorNum + sectorCount;
         i++)
        {
        bsp_printf(".");
        //sector = (S29GL_FLASH_ADRS)vol.map(&vol, i * vol.erasableBlockSize, 0);
        
        //S29GL_DBG_MSG("Erasing sector#%03d @ 0x%08x ...\r", i, (UINT32)sector);
        //bsp_printf("\r\n erase sector = %#x,sectorNum %d, sectorCount %d,", (UINT32)sector, sectorNum, sectorCount);
        //bsp_printf("\r\n erase 0xb4000008 = %#x,0xb400000c %#x ", *(UINT16*)0xb4000008, *(UINT16*)0xb400000c );
        sector = (S29GL_FLASH_ADRS)(sectorNum * S29GL_MTD_SECTOR_SIZE);
        S29GL_ERASE2((S29GL_FLASH_ADRS )(FLASH_BASE_ADRS2+(UINT32)sector ));

       // 
        //S29GL_ERASE(sector);
        while (TRUE) 
        {
            status = *(volatile UINT16 *)(FLASH_BASE_ADRS2+(i * S29GL_MTD_SECTOR_SIZE)); 
            
            if (0 == (status & 0x80)) /* data DQ7 */ 
            {   
                //printf("2222\n");
                if (status & 0x20)  /* DQ5 = 1 */
                {     
                   // printf("333\n");
                    /* read twice */
                    status = *(volatile UINT16 *)(FLASH_BASE_ADRS2+(i * S29GL_MTD_SECTOR_SIZE));  

                    if (0 == (status & 0x80))  /* toggle */
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
                    //wTemp1 = wTemp2;
                    continue;
                }
            }
            else 
            { 
                break;   /* program completed */
            }
        }
        


        
    }

    while((data1 = *(volatile UINT16*)((FLASH_BASE_ADRS2 + (i-1) * 0x20000) ) ) !=0xffff )
        bsp_printf(" while       data1 = %x    \r\n", data1   );
   
    FLASH_REGISTER_WR2(0x00, 0xf0); 
    //S29GL_RESET_BYPASS;
    return flOK;   
}

extern int fioRead
    ( 
    int    fd,       /* file descriptor of file to read */
    char * buffer,   /* buffer to receive input */
    int    maxbytes  /* maximum number of bytes to read */
    );

void writeBootrom()
{
    int fd;
	int len = 0 ;
	size_t	bytesRead;
	int nbytes;

	char * fileName="bootrom.bin";
    unsigned int temp=0;

    unsigned int count = 1000000;
    unsigned char * buffer = (unsigned char *)0xd9000000;
    unsigned char *bufferTmp = buffer ;

    //memset(buffer , 0 , 1024*1024*100);

    if ((fd = open (fileName, O_RDONLY , 0)) == ERROR)
	{
		printf("writeBootrom: Cannot open \"%s\".\n", fileName);
	}

    printf("\r\nwriteBootrom :  1\n", len);
    
	if( ioctl (fd, FIONREAD, (int) &bytesRead) == ERROR)
	{
	    printf("writeBootrom : ioctl bytesRead = %d\n", bytesRead);
	}
	len = 0;

    printf("\r\nwriteBootrom :  2\n", len);
    
	while ((nbytes = fioRead (fd, (char*)bufferTmp, 1024)) > 0)
	{
	    bufferTmp +=nbytes;
	    len += nbytes;
	} 

	printf("\r\nwriteBootrom :  read over,   bytes Read = %d\n", len);
	close(fd);

    s29glSectorRangeErase2(  (0xb7c00000-0xb4000000)/S29GL_MTD_SECTOR_SIZE, 1);
    s29glSectorRangeErase2(  (0xb7c20000-0xb4000000)/S29GL_MTD_SECTOR_SIZE, 1);
    s29glSectorRangeErase2(  (0xb7c40000-0xb4000000)/S29GL_MTD_SECTOR_SIZE, 1);
    s29glSectorRangeErase2(  (0xb7c60000-0xb4000000)/S29GL_MTD_SECTOR_SIZE, 1);
    
    /* start write data to fpga */
    bufferTmp = buffer ;
    temp = len;
    printf("\r\nwriteBootrom :  bufferTmp= %#x\n", bufferTmp);
    //S29GL_ENTRY_BYPASS2;
    while(temp >= 2)
    {
    	
        s29GlWrite2((S29GL_FLASH_ADRS)(0xb7c00000+(len-temp)), *(UINT16*)bufferTmp );
        //for( loop=10;loop>0;loop--);

        if(0 == (temp % 1000000))
        {
            //printf("bufferTmp = %x, *bufferTmp = %x. \r\n",bufferTmp , *bufferTmp);
            printf(".");
        }
        
        temp -=2;
        bufferTmp +=2;
    }
    //S29GL_RESET_BYPASS2;
    
}

#if 0
void writeBootrom_pci()
{
    int fd;
	int len = 0 ;
	size_t	bytesRead;
	int nbytes;

	char * fileName="bootrom.bin";
    unsigned int temp=0;

    unsigned int count = 1000000;
    unsigned char * buffer = (char *)0xd9000000;
    unsigned char *bufferTmp = buffer ;

    //memset(buffer , 0 , 1024*1024*100);

    if ((fd = open (fileName, O_RDONLY , 0)) == ERROR)
	{
		printf("writeBootrom: Cannot open \"%s\".\n", fileName);
		return (ERROR);
	}

    printf("\r\nwriteBootrom :  1\n", len);
    
	if( ioctl (fd, FIONREAD, (int) &bytesRead) == ERROR)
	{
	    printf("writeBootrom : ioctl bytesRead = %d\n", bytesRead);
	}
	len = 0;

    printf("\r\nwriteBootrom :  2\n", len);
    
	while ((nbytes = fioRead (fd, bufferTmp, 1024)) > 0)
	{
	    bufferTmp +=nbytes;
	    len += nbytes;
	} 

	printf("\r\nwriteBootrom :  read over,   bytes Read = %d\n", len);
	close(fd);

    s29glSectorRangeErase2(  (0xbfc00000-0xbc000000)/S29GL_MTD_SECTOR_SIZE, 1);
    s29glSectorRangeErase2(  (0xbfc20000-0xbc000000)/S29GL_MTD_SECTOR_SIZE, 1);
    s29glSectorRangeErase2(  (0xbfc40000-0xbc000000)/S29GL_MTD_SECTOR_SIZE, 1);
    s29glSectorRangeErase2(  (0xbfc60000-0xbc000000)/S29GL_MTD_SECTOR_SIZE, 1);
    
    /* start write data to fpga */
    bufferTmp = buffer ;
    temp = len;
    printf("\r\nwriteBootrom :  bufferTmp= %#x\n", bufferTmp);
    //S29GL_ENTRY_BYPASS2;
    while(temp >= 2)
    {
    	
        s29GlWrite2(0xbfc00000+(len-temp), *(UINT16*)bufferTmp );
        //for( loop=10;loop>0;loop--);

        if(0 == (temp % 1000000))
        {
            //printf("bufferTmp = %x, *bufferTmp = %x. \r\n",bufferTmp , *bufferTmp);
            printf(".");
        }
        
        temp -=2;
        bufferTmp +=2;
    }
    //S29GL_RESET_BYPASS2;
    
}
#endif

   /* 0xbfc00000 从pci启动时用来烧写bootrom */
#define	FLASH_BASE_ADRS3		0xbc000000 	/* Flash memory base address  mxl modified*/

#define	FLASH_SIZE2		    0x04000000	/* Flash memory size   mxl modified */

#define FLASH_REGISTER_WR3(addr,data)      *(volatile UINT16 *)((volatile UINT16 *)(0xbfc00000+(UINT32)(addr)))=(UINT16)(data)   
#define FLASH_REGISTER_RD3(addr)         (*(volatile UINT16 *)((volatile UINT16 *)(0xbfc00000+(UINT32)(addr))))   



#define S29GL_ENTRY_BYPASS3 \
     do  \
     {   \
         FLASH_REGISTER_WR3(0x00, 0xf0);   \
         FLASH_REGISTER_WR3(S29GL_UNLOCK_ADDR1 * 2,S29GL_UNLOCK_DATA1);      \
         FLASH_REGISTER_WR3(S29GL_UNLOCK_ADDR2 * 2,S29GL_UNLOCK_DATA2);      \
         FLASH_REGISTER_WR3(S29GL_UNLOCK_ADDR1 * 2,0x20);      \
     } while((0));
 
 #define S29GL_RESET_BYPASS3 \
     do  \
     {   \
         FLASH_REGISTER_WR3(0x00, 0xf0);   \
         FLASH_REGISTER_WR3(S29GL_UNLOCK_ADDR1 * 2,0x90);      \
         FLASH_REGISTER_WR3(S29GL_UNLOCK_ADDR1 * 2,0x00);      \
     } while((0));
     
#define S29GL_PROGRAM_BYPASS3(  addr2, value)  \
         do  \
         {   \
             FLASH_REGISTER_WR3(S29GL_UNLOCK_ADDR1 * 2,S29GL_CMD_PROGRAM);    \
             *(volatile UINT16 *)addr2 = value; \
         } while((0));
		 
#define S29GL_PROGRAM3( addr2, value)  \
			 do  \
			 {	 \
				 FLASH_REGISTER_WR3(S29GL_UNLOCK_ADDR1 * 2,S29GL_UNLOCK_DATA1);	\
				 FLASH_REGISTER_WR3(S29GL_UNLOCK_ADDR2 * 2,S29GL_UNLOCK_DATA2);	 \
				 FLASH_REGISTER_WR3(S29GL_UNLOCK_ADDR1 * 2,S29GL_CMD_PROGRAM);	 \
				 *(volatile UINT16 *)(addr2) = (value); \
			 } while((0));


#define S29GL_ERASE3(addr)   \
	 do  \
	 {	 \
		 FLASH_REGISTER_WR3(S29GL_UNLOCK_ADDR1 * 2, S29GL_UNLOCK_DATA1); 	 \
		 FLASH_REGISTER_WR3(S29GL_UNLOCK_ADDR2 * 2, S29GL_UNLOCK_DATA2) ;	  \
		 FLASH_REGISTER_WR3(S29GL_UNLOCK_ADDR1 * 2, S29GL_CMD_ERASE_SECTOR1) ; \
		 FLASH_REGISTER_WR3(S29GL_UNLOCK_ADDR1 * 2, S29GL_UNLOCK_DATA1) ;	  \
		 FLASH_REGISTER_WR3(S29GL_UNLOCK_ADDR2 * 2, S29GL_UNLOCK_DATA2) ;	  \
		 *(volatile UINT16 *)(addr) = S29GL_CMD_ERASE_SECTOR2;	  \
	 } while((0));

 int s29GlWrite3
    (
    S29GL_FLASH_ADRS    addr,
    S29GL_FLASH_DATA    value
    )
{
    UINT16 *  sector;
    UINT16  secNum;
    UINT32  timeout = 0, status;

    //bsp_printf("\r\n  addr=%#x, oldvalue = %#x, value= %#x", addr, *(UINT16*)addr,value);
    sector = (UINT16 * )((UINT32)addr & S29GL_SECTOR_MASK);

    //FLASH_REGISTER_WR(0x00, 0xf0);
    //SMALL_LOOP;

    #if 1 
    S29GL_PROGRAM3( addr, value);
    #else
    S29GL_PROGRAM_BYPASS2(  addr, value) 
    #endif
    secNum = ((UINT32)sector - FLASH_BASE_ADRS3 )/ S29GL_MTD_SECTOR_SIZE ;
    //printf("\r\n addr =%#x, value = %x", addr,value);

    /* set timeout = 5s */

   // timeout = flMsecCounter + S29GL_OP_TIMEOUT;

    while (TRUE) 
    {
        timeout++;
        if(timeout > 1000)
            break;
        status = *(volatile UINT16 *)addr; 

        //bsp_printf("\r\naddr = %x value = %x status = %x  ", addr,value, status );
        if ((status ^ value) & 0x80 ) /* toggle */ 
        {   
            //printf("2222\n");
            if (status & 0x20)  /* DQ5 = 1 */
            {     
               // printf("333\n");
                /* read twice */
                status = *(volatile UINT16 *)addr; 
                if ((status ^ value) & 0x80 )  /* toggle */
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
                //wTemp1 = wTemp2;
                //FLASH_REGISTER_WR(0x00, 0xf0);                
                continue;
            }
        }
        else 
        { 
            break;   /* program completed */
        }
    }
    
    
    //bsp_printf("\r\n   Newvalue = %#x ",  *(UINT16*)addr );
    FLASH_REGISTER_WR3(0x00, 0xf0);/* RESET ????*/ 
    //while(value != *addr);

    //bsp_printf("\r\ns29GlWrite exit", addr, *(UINT16)addr,value);
    return flOK;
}

int s29glSectorRangeErase3  
    (  
    int sectorNum,  
    int sectorCount  
    )  
{  
    S29GL_FLASH_ADRS sector = NULL;
    S29GL_FLASH_DATA data1 ;
    UINT32 timeout = 0, status;
    int i;
    //bsp_printf("\r\n s29glSectorRangeErase enter " );
    
    //FLASH_REGISTER_WR(0x00, 0xf0);/* RESET ????*/ 
 //   if (flWriteProtected(vol.socket))
   //     return flWriteProtect;
    //S29GL_ENTRY_BYPASS;
    for (i = sectorNum;
         i < sectorNum + sectorCount;
         i++)
        {
        bsp_printf(".");
        //sector = (S29GL_FLASH_ADRS)vol.map(&vol, i * vol.erasableBlockSize, 0);
        
        //S29GL_DBG_MSG("Erasing sector#%03d @ 0x%08x ...\r", i, (UINT32)sector);
        //bsp_printf("\r\n erase sector = %#x,sectorNum %d, sectorCount %d,", (UINT32)sector, sectorNum, sectorCount);
        //bsp_printf("\r\n erase 0xb4000008 = %#x,0xb400000c %#x ", *(UINT16*)0xb4000008, *(UINT16*)0xb400000c );
        sector = (S29GL_FLASH_ADRS)(sectorNum * S29GL_MTD_SECTOR_SIZE);
        S29GL_ERASE3((S29GL_FLASH_ADRS )(FLASH_BASE_ADRS3+(UINT32)sector ));

       // 
        //S29GL_ERASE(sector);
        while (TRUE) 
        {
            status = *(volatile UINT16 *)(FLASH_BASE_ADRS3+(i * S29GL_MTD_SECTOR_SIZE)); 
            
            if (0 == (status & 0x80)) /* data DQ7 */ 
            {   
                //printf("2222\n");
                if (status & 0x20)  /* DQ5 = 1 */
                {     
                   // printf("333\n");
                    /* read twice */
                    status = *(volatile UINT16 *)(FLASH_BASE_ADRS3+(i * S29GL_MTD_SECTOR_SIZE));  

                    if (0 == (status & 0x80))  /* toggle */
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
                    //wTemp1 = wTemp2;
                    continue;
                }
            }
            else 
            { 
                break;   /* program completed */
            }
        }
        


        
    }

    while((data1 = *(volatile UINT16*)((FLASH_BASE_ADRS3 + (i-1) * 0x20000) ) ) !=0xffff )
        bsp_printf(" while       data1 = %x    \r\n", data1   );
   
    FLASH_REGISTER_WR3(0x00, 0xf0); 
    //S29GL_RESET_BYPASS;
    return flOK;   
}
void writeBootrom_pci()
{
    int fd;
	int len = 0 ;
	size_t	bytesRead;
	int nbytes;

	char * fileName="bootrom.bin";
    unsigned int temp=0;

    unsigned int count = 1000000;
    unsigned char * buffer = (unsigned char *)0xd9000000;
    unsigned char *bufferTmp = buffer ;

    //memset(buffer , 0 , 1024*1024*100);

    if ((fd = open (fileName, O_RDONLY , 0)) == ERROR)
	{
		printf("writeBootrom: Cannot open \"%s\".\n", fileName);
		return ;
	}

    printf("\r\nwriteBootrom :  1\n", len);
    
	if( ioctl (fd, FIONREAD, (int) &bytesRead) == ERROR)
	{
	    printf("writeBootrom : ioctl bytesRead = %d\n", bytesRead);
	}
	len = 0;

    printf("\r\nwriteBootrom :  2\n", len);
    
	while ((nbytes = fioRead (fd, (char*)bufferTmp, 1024)) > 0)
	{
	    bufferTmp +=nbytes;
	    len += nbytes;
	} 

	printf("\r\nwriteBootrom :  read over,   bytes Read = %d\n", len);
	close(fd);

    s29glSectorRangeErase3(  (0xbfc00000-0xbc000000)/S29GL_MTD_SECTOR_SIZE, 1);
    s29glSectorRangeErase3(  (0xbfc20000-0xbc000000)/S29GL_MTD_SECTOR_SIZE, 1);
    s29glSectorRangeErase3(  (0xbfc40000-0xbc000000)/S29GL_MTD_SECTOR_SIZE, 1);
    s29glSectorRangeErase3(  (0xbfc60000-0xbc000000)/S29GL_MTD_SECTOR_SIZE, 1);
    
    /* start write data to fpga */
    bufferTmp = buffer ;
    temp = len;
    printf("\r\nwriteBootrom :  bufferTmp= %#x\n", bufferTmp);
    //S29GL_ENTRY_BYPASS2;
    while(temp >= 2)
    {
    	
        s29GlWrite3((S29GL_FLASH_ADRS)(0xbfc00000+(len-temp)), *(UINT16*)bufferTmp );
        //for( loop=10;loop>0;loop--);

        if(0 == (temp % 1000000))
        {
            //printf("bufferTmp = %x, *bufferTmp = %x. \r\n",bufferTmp , *bufferTmp);
            printf(".");
        }
        
        temp -=2;
        bufferTmp +=2;
    }
    //S29GL_RESET_BYPASS2;
    
}

#endif






