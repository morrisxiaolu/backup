/*	$Id: flash_sst.c,v 1.1.1.1 2006/09/14 01:59:08 root Exp $ */

/*
 * Copyright (c) 2001 ipUnplugged AB   (www.ipunplugged.com)
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by
 *	ipUnplugged AB, Sweden.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <vxWorks.h>
#include <stdio.h>

#define FL_PROTO_SST    2
#define FL_CAP_DE       0x01    /* Device have entire device Erase */
#define FL_CAP_A7       0x02    /* Device uses a7 for Bulk Erase */

#define FL_BUS_8        0x01    /* Byte wide bus */
#define FL_BUS_16       0x02    /* Short wide bus */
#define FL_BUS_32       0x03    /* Word wide bus */
#define FL_BUS_64       0x04    /* Quad wide bus */
#define FL_BUS_8_ON_64  0x05    /* Byte wide on quad wide bus */
#define FL_BUS_16_ON_64 0x06    /* 16-bit wide flash on quad wide bus */

/* 
 *  Flash Commands
 */
#define FL_AUTOSEL      0x90    /* Device identification */
#define FL_RESET        0xf0    /* Return to DATA mode */
#define FL_ERASE        0x80
#define FL_ERASE_CHIP   0x10
#define FL_CHIP         0x10
#define FL_SECT         0x30
#define FL_PGM          0xa0
#define FL_SUSPEND      0xb0    /* Erase Suspend */
#define FL_RESUME       0x30    /* Erase Resume */

#define SST_CMDOFFS  0x5555
#define SST_CMDOFFS1 0x5555
#define SST_CMDOFFS2 0x2aaa

#define LPC_FLASH_MAP_BASE 0xbfc00000

#define lpc_outb(addr,val) do {*(volatile unsigned char *)(addr) = val ; } while(FALSE)
#define lpc_inb(addr)      *(volatile unsigned char *)(addr)
#define lpc_outw(addr,val) do {*(volatile unsigned int *)(addr) = val ; } while(FALSE)
#define lpc_inw(addr)      *(volatile unsigned int *)(addr)

LOCAL int loopSst = 0;
#define BIG_LOOP_LPC for( loopSst=10000;loopSst>0;loopSst--);

/*
 *  Function to poll flash BUSY if available.
 *  returns 1 if busy, 0 if OK, -1 if error.
 */
int lpc_flash_isbusy_sst(int what, int offset, int erase)
{
	int busy;

	/* Data polling 
	 *  algorithm is in Figure 6
	 */

	while (TRUE) 
	{
		unsigned char poll1, poll2;
		poll1 = lpc_inb(LPC_FLASH_MAP_BASE + offset);
		poll2 = lpc_inb(LPC_FLASH_MAP_BASE + offset);
		if ((poll1 ^ poll2) & 0x40) 
		{   /* toggle */ 
			busy=1;
			break;
		}
		else 
		{ 
			poll1 = lpc_inb(LPC_FLASH_MAP_BASE + offset);
			poll2 = lpc_inb(LPC_FLASH_MAP_BASE + offset);
			if ((poll1 ^ poll2) & 0x40) 
				busy=1;
			else 
				busy = 0;
			break;   /* program completed */
		}
	}

	return(busy);
}

/*
 *  Function to erase sector
 */
int lpc_flash_erase_sector_sst(int offset)
{
/*	int stat;*/

	lpc_outb((LPC_FLASH_MAP_BASE + SST_CMDOFFS1), 0xAA);
	lpc_outb((LPC_FLASH_MAP_BASE + SST_CMDOFFS2), 0x55);
	lpc_outb((LPC_FLASH_MAP_BASE + SST_CMDOFFS1), FL_ERASE);
	lpc_outb((LPC_FLASH_MAP_BASE + SST_CMDOFFS1), 0xAA);
	lpc_outb((LPC_FLASH_MAP_BASE + SST_CMDOFFS2), 0x55);
	lpc_outb((LPC_FLASH_MAP_BASE + offset), FL_SECT);

	BIG_LOOP_LPC
	return(0);
}

/*
 *  Function to erase chip
 */
int lpc_flash_erase_chip_sst(void)
{
	
	lpc_outb((LPC_FLASH_MAP_BASE + SST_CMDOFFS1), 0xaa);
	lpc_outb((LPC_FLASH_MAP_BASE + SST_CMDOFFS2), 0x55);
	lpc_outb((LPC_FLASH_MAP_BASE + SST_CMDOFFS1), FL_ERASE);
	lpc_outb((LPC_FLASH_MAP_BASE + SST_CMDOFFS1), 0xaa);
	lpc_outb((LPC_FLASH_MAP_BASE + SST_CMDOFFS2), 0x55);
	lpc_outb((LPC_FLASH_MAP_BASE + SST_CMDOFFS1), FL_ERASE_CHIP);

	BIG_LOOP_LPC
    return(0);
}

/*
 *  Function to Program
 */
int lpc_flash_program_sst(int pa, unsigned char *pd)
{
	int stat;

	lpc_outb((LPC_FLASH_MAP_BASE + SST_CMDOFFS1), 0xAA);
	lpc_outb((LPC_FLASH_MAP_BASE + SST_CMDOFFS2), 0x55);
	lpc_outb((LPC_FLASH_MAP_BASE + SST_CMDOFFS1), 0xA0);
	lpc_outb((LPC_FLASH_MAP_BASE + pa), *pd);
    
	do 
	{
		stat = lpc_flash_isbusy_sst(*pd, pa, FALSE);
	} while(stat == 1);  
	return(0);
}

/*
 *  Function to "reset" flash, eg return to read mode.
 */
int lpc_flash_reset_sst(void)
{
	lpc_outb((LPC_FLASH_MAP_BASE), FL_RESET);
	BIG_LOOP_LPC
	return(0);
}

int lpc_flash_write_protect_unlock(int offset)
{
    unsigned int trans_unlock_value;

    //printf("Disable all space write protection of 49LF040B. \r\n");
    /* Open translation of 0xbc000000 - 0xbd00000 */
    trans_unlock_value = lpc_inw(0xbfe00200);
    lpc_outw(0xbfe00200, (0x00ff0000 | trans_unlock_value));

    /* Disable all space write protection */
    lpc_outb(0xbdbf0002, 0x0);
    lpc_outb(0xbdbe0002, 0x0);
    lpc_outb(0xbdbd0002, 0x0);
    lpc_outb(0xbdbc0002, 0x0);
    lpc_outb(0xbdbb0002, 0x0);
    lpc_outb(0xbdba0002, 0x0);
    lpc_outb(0xbdb90002, 0x0);
    lpc_outb(0xbdb80002, 0x0);
    
    lpc_outw(0xbfe00200, trans_unlock_value);

    BIG_LOOP_LPC
    return(1);
}

int lpc_flash_write_protect_lock(int offset)
{
    unsigned int trans_unlock_value;

    //printf("Enable all space write protection of 49LF040B. \r\n");
    /* Open translation of 0xbc000000 - 0xbd00000 */
    trans_unlock_value = lpc_inw(0xbfe00200);
    lpc_outw(0xbfe00200, (0x00ff0000 | trans_unlock_value));

    /* Enable all space write protection */
    lpc_outb(0xbdbf0002, 0x1);
    lpc_outb(0xbdbe0002, 0x1);
    lpc_outb(0xbdbd0002, 0x1);
    lpc_outb(0xbdbc0002, 0x1);
    lpc_outb(0xbdbb0002, 0x1);
    lpc_outb(0xbdba0002, 0x1);
    lpc_outb(0xbdb90002, 0x1);
    lpc_outb(0xbdb80002, 0x1);

    lpc_outw(0xbfe00200, trans_unlock_value);

    BIG_LOOP_LPC
    return(1);
}
