/* 
 * Loongson 3A HT Turbo Speed for VxWorks 6.7
 * author: yinwx@ict.ac.cn
 * date: 20100316
 *
 * for more HT configuration regs details, please reference
 * mannual of Loongson 3A Part 1: Architecture of Multicore
 * in Chapter 9.4
 */

#include <stdio.h>

/* HT Types */
#define HT0_LO	0
#define HT0_HI	1
#define HT1_LO	2
#define HT1_HI	3

extern unsigned short getNodeZR();
/* for Loongson 3A HT Config Reg Read */
__asm volatile unsigned int LS3A_HT_ConfigRead(unsigned int type, unsigned int cpunum, unsigned int offset)
{
% reg type; reg cpunum; reg offset;
! "$2"                       
	.set noreorder 
	dli	t0,0x90000cfdfb000000
	dsll32 t1,type,8
	or	t0,t0,t1
	dsll32 t1,cpunum, 12
	or	t0,t0,t1
	and offset, offset, 0xffffffff
	or  t0,t0,offset
	lw	$2, 0(t0)
	.set reorder
}
/* for Loongson 3A HT Config Reg Write */
__asm volatile void LS3A_HT_ConfigWrite(unsigned int type, unsigned int cpunum, unsigned int offset, unsigned int value)
{
% reg type; reg cpunum; reg offset; reg value;
	.set noreorder 
	dli	t0,0x90000cfdfb000000
	dsll32 t1,type,8
	or	t0,t0,t1
	dsll32 t1,cpunum, 12
	or	t0,t0,t1
	and offset, offset, 0xffffffff
	or  t0,t0,offset
	sw	value, 0(t0)
	sync
	.set reorder
}

__asm volatile void OPEN_L1XBAR_HT0_WIN(void)
{
	.set noreorder 

	/* L1XBAR CPU#0 Core#0 WIN3 */
	dli	t0,0x900000003ff02018
	dli t1,0x0
	sd	t1,0x80(t0)
	/*
	dli t1,0x00000c0000000000
	sd	t1,0(t0)
	dli	t1,0x00000e0000000000
	sd	t1,0x40(t0)
	dli t1,0x00000c00000000f6
	sd	t1,0x80(t0)
	*/
	sync

	/* L1XBAR CPU#0 Core#1 WIN3 */
	dli	t0,0x900000003ff02118
	dli t1,0x0
	sd	t1,0x80(t0)
	/*
	dli t1,0x00000c0000000000
	sd	t1,0(t0)
	dli	t1,0x00000e0000000000
	sd	t1,0x40(t0)
	dli t1,0x00000c00000000f6
	sd	t1,0x80(t0)
	*/
	sync

	/* L1XBAR CPU#0 Core#2 WIN3 */
	dli	t0,0x900000003ff02218
	dli t1,0x0
	sd	t1,0x80(t0)
	/*
	dli t1,0x00000c0000000000
	sd	t1,0(t0)
	dli	t1,0x00000e0000000000
	sd	t1,0x40(t0)
	dli t1,0x00000c00000000f6
	sd	t1,0x80(t0)
	*/
	sync

	/* L1XBAR CPU#0 Core#3 WIN3 */
	dli	t0,0x900000003ff02318
	dli t1,0x0
	sd	t1,0x80(t0)
	/*
	dli t1,0x00000c0000000000
	sd	t1,0(t0)
	dli	t1,0x00000e0000000000
	sd	t1,0x40(t0)
	dli t1,0x00000c00000000f6
	sd	t1,0x80(t0)
	*/
	sync

	/* L1XBAR CPU#0 WEST(HT0) WIN3 */
	dli	t0,0x900000003ff02618
	dli t1,0x0
	sd	t1,0x80(t0)
	/*
	dli t1,0x00000c0000000000
	sd	t1,0(t0)
	dli	t1,0x00000e0000000000
	sd	t1,0x40(t0)
	dli t1,0x00000c00000000f6
	sd	t1,0x80(t0)
	*/
	sync

	/* L1XBAR CPU#1 WEST(HT0) WIN3 */
	dli	t0,0x900010003ff02618
	dli t1,0x0
	sd	t1,0x80(t0)
	/*
	dli t1,0x00000c0000000000
	sd	t1,0(t0)
	dli	t1,0x00000e0000000000
	sd	t1,0x40(t0)
	dli t1,0x00000c00000000f6
	sd	t1,0x80(t0)
	*/
	sync

	/* L1XBAR CPU#2 WEST(HT0) WIN3 */
	dli	t0,0x900020003ff02618
	dli t1,0x0
	sd	t1,0x80(t0)
	/*
	dli t1,0x00000c0000000000
	sd	t1,0(t0)
	dli	t1,0x00000e0000000000
	sd	t1,0x40(t0)
	dli t1,0x00000c00000000f6
	sd	t1,0x80(t0)
	*/
	sync

	/* L1XBAR CPU#3 WEST(HT0) WIN3 */
	dli	t0,0x900030003ff02618
	dli t1,0x0
	sd	t1,0x80(t0)
	/*
	dli t1,0x00000c0000000000
	sd	t1,0(t0)
	dli	t1,0x00000e0000000000
	sd	t1,0x40(t0)
	dli t1,0x00000c00000000f6
	sd	t1,0x80(t0)
	*/
	sync
	
	.set reorder
}

__asm volatile void CLOSE_L1XBAR_HT0_WIN(void)
{
	.set noreorder
	/* L1XBAR CPU#0 Core#0 WIN3 */
	dli	t0,0x900000003ff02018
	dli t1,0x0
	sd	t1,0x80(t0)
	dli t1,0x00000c0000000000
	sd	t1,0(t0)
	dli	t1,0x00000e0000000000
	sd	t1,0x40(t0)
	dli t1,0x00000e00000000f7
	sd	t1,0x80(t0)
	sync

	/* L1XBAR CPU#0 Core#1 WIN3 */
	dli	t0,0x900000003ff02118
	dli t1,0x0
	sd	t1,0x80(t0)
	dli t1,0x00000c0000000000
	sd	t1,0(t0)
	dli	t1,0x00000e0000000000
	sd	t1,0x40(t0)
	dli t1,0x00000e00000000f7
	sd	t1,0x80(t0)
	sync

	/* L1XBAR CPU#0 Core#2 WIN3 */
	dli	t0,0x900000003ff02218
	dli t1,0x0
	sd	t1,0x80(t0)
	dli t1,0x00000c0000000000
	sd	t1,0(t0)
	dli	t1,0x00000e0000000000
	sd	t1,0x40(t0)
	dli t1,0x00000e00000000f7
	sd	t1,0x80(t0)
	sync

	/* L1XBAR CPU#0 Core#3 WIN3 */
	dli	t0,0x900000003ff02318
	dli t1,0x0
	sd	t1,0x80(t0)
	dli t1,0x00000c0000000000
	sd	t1,0(t0)
	dli	t1,0x00000e0000000000
	sd	t1,0x40(t0)
	dli t1,0x00000e00000000f7
	sd	t1,0x80(t0)
	sync


	/* L1XBAR CPU#0 WEST(HT0) WIN3 */
	dli	t0,0x900000003ff02618
	dli t1,0x0
	sd	t1,0x80(t0)
	dli t1,0x00000c0000000000
	sd	t1,0(t0)
	dli	t1,0x00000e0000000000
	sd	t1,0x40(t0)
	dli t1,0x00000e00000000f7
	sd	t1,0x80(t0)
	sync

	/* L1XBAR CPU#1 WEST(HT0) WIN3 */
	dli	t0,0x900010003ff02618
	dli t1,0x0
	sd	t1,0x80(t0)
	dli t1,0x00000c0000000000
	sd	t1,0(t0)
	dli	t1,0x00000e0000000000
	sd	t1,0x40(t0)
	dli t1,0x00000e00000000f7
	sd	t1,0x80(t0)
	sync

	/* L1XBAR CPU#2 WEST(HT0) WIN3 */
	dli	t0,0x900020003ff02618
	dli t1,0x0
	sd	t1,0x80(t0)
	dli t1,0x00000c0000000000
	sd	t1,0(t0)
	dli	t1,0x00000e0000000000
	sd	t1,0x40(t0)
	dli t1,0x00000e00000000f7
	sd	t1,0x80(t0)
	sync

	/* L1XBAR CPU#3 WEST(HT0) WIN3 */
	dli	t0,0x900030003ff02618
	dli t1,0x0
	sd	t1,0x80(t0)
	dli t1,0x00000c0000000000
	sd	t1,0(t0)
	dli	t1,0x00000e0000000000
	sd	t1,0x40(t0)
	dli t1,0x00000e00000000f7
	sd	t1,0x80(t0)
	sync

	.set reorder
	nop

}

__asm volatile unsigned int read_xbar_win_hi32(unsigned int xbar_addr)
{
% reg xbar_addr;
! "$2"                       
	.set noreorder 
	dli	t0,0x9000000000000000
	or t0,t0,xbar_addr
	daddiu t0,t0,0x4
	lw	$2, 0(t0)
	.set reorder
	nop

}

__asm volatile unsigned int read_xbar_win_lo32(unsigned int xbar_addr)
{
% reg xbar_addr;
! "$2"                       
	.set noreorder 
	dli	t0,0x9000000000000000
	or t0,t0,xbar_addr
	lw	$2, 0(t0)
	.set reorder
}

#if 0
void xbarShow(void)
{
	int i;
	unsigned int base, mask, mmap;
	base = 0x3ff02600;
	for(i=0; i<=7; i++)
		{
		mask = base + 0x40;
		mmap = base + 0x80;
		printf("0x%x-", base); 
		printf("0x%08x", read_xbar_win_hi32(base));
		printf("%08x ", read_xbar_win_lo32(base));
		printf("0x%x-", mask); 
		printf("0x%08x", read_xbar_win_hi32(mask));
		printf("%08x ", read_xbar_win_lo32(mask));
		printf("0x%x-", mmap); 
		printf("0x%08x", read_xbar_win_hi32(mmap));
		printf("%08x ", read_xbar_win_lo32(mmap));
		printf("\n");
		base += 0x8;
		}
}
#endif
unsigned int bslReadHT0LConfig(unsigned int reg)
{
	int node_id;
	unsigned int val;
	node_id = getNodeZR();
	val = LS3A_HT_ConfigRead(0,node_id,reg);
	return val;
}
unsigned int bslReadHT0HConfig(unsigned int reg)
{
	int node_id;
	unsigned int val;
	node_id = getNodeZR();
	val = LS3A_HT_ConfigRead(1,node_id,reg);
	return val;
}

unsigned int bslReadHT1LConfig(unsigned int reg)
{
	int node_id;
	unsigned int val;
	node_id = getNodeZR();
	val = LS3A_HT_ConfigRead(2,node_id,reg);
	return val;
}
unsigned int bslReadHT1HConfig(unsigned int reg)
{
	int node_id;
	unsigned int val;
	node_id = getNodeZR();
	val = LS3A_HT_ConfigRead(3,node_id,reg);
	return val;
}


/* Turbo function for changing HT0 freq */
STATUS HT0_Turbo(int freq)
{
	unsigned int freqVal, miscVal;
	
	printf("Changing HT freq to %dMHz...\n", freq);

	/*xbarShow();*/
	
	OPEN_L1XBAR_HT0_WIN(); /* we must open the HT0 win */
	freqVal = LS3A_HT_ConfigRead(HT0_LO, 3, 0x44);
	printf("offset 0x44 is 0x%x\n", freqVal);

	/*xbarShow();*/

	switch(freq)
	{
		case 200:
			/* CPU #3 HT0_HI */
			freqVal = LS3A_HT_ConfigRead(HT0_HI, 3, 0x48);
			printf("CPU#3, old HT0_HI_freq reg is 0x%x\n", freqVal);
			freqVal &= 0xfffff0ff;
			LS3A_HT_ConfigWrite(HT0_HI, 3, 0x48, freqVal | 0x0);
			printf("CPU#3, new HT0_HI_freq reg is 0x%x\n", LS3A_HT_ConfigRead(HT0_HI, 3, 0x48));
			/* CPU #1 HT0_LO */
			freqVal = LS3A_HT_ConfigRead(HT0_LO, 1, 0x48);
			printf("CPU#1, old HT0_LO_freq reg is 0x%x\n", freqVal);
			freqVal &= 0xfffff0ff;
			LS3A_HT_ConfigWrite(HT0_LO, 1, 0x48, freqVal | 0x0);
			printf("CPU#1, new HT0_LO_freq reg is 0x%x\n", LS3A_HT_ConfigRead(HT0_LO, 1, 0x48));
			/* Disconnect CPU#1 HT0_LO Bus, 0->1 */
			miscVal = LS3A_HT_ConfigRead(HT0_LO, 1, 0x50);
			miscVal &= 0xbfffffff;
			LS3A_HT_ConfigWrite(HT0_LO, 1, 0x50, miscVal);
			miscVal |= 0x40000000;
			LS3A_HT_ConfigWrite(HT0_LO, 1, 0x50, miscVal);
			
			/* CPU #0 HT0_HI */
			freqVal = LS3A_HT_ConfigRead(HT0_HI, 0, 0x48);
			printf("CPU#0, old HT0_HI_freq reg is 0x%x\n", freqVal);
			freqVal &= 0xfffff0ff;
			LS3A_HT_ConfigWrite(HT0_HI, 0, 0x48, freqVal | 0x0);
			printf("CPU#0, new HT0_HI_freq reg is 0x%x\n", LS3A_HT_ConfigRead(HT0_HI, 0, 0x48));
			/* CPU #2 HT0_LO */
			freqVal = LS3A_HT_ConfigRead(HT0_LO, 2, 0x48);
			printf("CPU#2, old HT0_LO_freq reg is 0x%x\n", freqVal);
			freqVal &= 0xfffff0ff;
			LS3A_HT_ConfigWrite(HT0_LO, 2, 0x48, freqVal | 0x0);
			printf("CPU#2, new HT0_LO_freq reg is 0x%x\n", LS3A_HT_ConfigRead(HT0_LO, 2, 0x48));
			/* Disconnect CPU#2 HT0_LO Bus, 0->1 */
			miscVal = LS3A_HT_ConfigRead(HT0_LO, 2, 0x50);
			miscVal &= 0xbfffffff;
			LS3A_HT_ConfigWrite(HT0_LO, 2, 0x50, miscVal);
			miscVal |= 0x40000000;
			LS3A_HT_ConfigWrite(HT0_LO, 2, 0x50, miscVal);

			/* CPU #1 HT0_HI */
			freqVal = LS3A_HT_ConfigRead(HT0_HI, 1, 0x48);
			printf("CPU#1, old HT0_HI_freq reg is 0x%x\n", freqVal);
			freqVal &= 0xfffff0ff;
			LS3A_HT_ConfigWrite(HT0_HI, 1, 0x48, freqVal | 0x0);
			printf("CPU#1, new HT0_HI_freq reg is 0x%x\n", LS3A_HT_ConfigRead(HT0_HI, 1, 0x48));
			/* CPU #3 HT0_LO */
			freqVal = LS3A_HT_ConfigRead(HT0_LO, 3, 0x48);
			printf("CPU#3, old HT0_LO_freq reg is 0x%x\n", freqVal);
			freqVal &= 0xfffff0ff;
			LS3A_HT_ConfigWrite(HT0_LO, 3, 0x48, freqVal | 0x0);
			printf("CPU#3, new HT0_LO_freq reg is 0x%x\n", LS3A_HT_ConfigRead(HT0_LO, 3, 0x48));
			/* Disconnect CPU#3 HT0_LO Bus, 0->1 */
			miscVal = LS3A_HT_ConfigRead(HT0_LO, 3, 0x50);
			miscVal &= 0xbfffffff;
			LS3A_HT_ConfigWrite(HT0_LO, 3, 0x50, miscVal);
			miscVal |= 0x40000000;
			LS3A_HT_ConfigWrite(HT0_LO, 3, 0x50, miscVal);

			/* CPU #2 HT0_HI */
			freqVal = LS3A_HT_ConfigRead(HT0_HI, 2, 0x48);
			printf("CPU#2, old HT0_HI_freq reg is 0x%x\n", freqVal);
			freqVal &= 0xfffff0ff;
			LS3A_HT_ConfigWrite(HT0_HI, 2, 0x48, freqVal | 0x0);
			printf("CPU#2, new HT0_HI_freq reg is 0x%x\n", LS3A_HT_ConfigRead(HT0_HI, 2, 0x48));
			/* CPU #0 HT0_LO */
			freqVal = LS3A_HT_ConfigRead(HT0_LO, 0, 0x48);
			printf("CPU#0, old HT0_LO_freq reg is 0x%x\n", freqVal);
			freqVal &= 0xfffff0ff;
			LS3A_HT_ConfigWrite(HT0_LO, 0, 0x48, freqVal | 0x0);
			printf("CPU#0, new HT0_LO_freq reg is 0x%x\n", LS3A_HT_ConfigRead(HT0_LO, 0, 0x48));
			/* Disconnect CPU#2 HT0_LO Bus, 0->1 */
			miscVal = LS3A_HT_ConfigRead(HT0_LO, 0, 0x50);
			miscVal &= 0xbfffffff;
			LS3A_HT_ConfigWrite(HT0_LO, 0, 0x50, miscVal);
			miscVal |= 0x40000000;
			LS3A_HT_ConfigWrite(HT0_LO, 0, 0x50, miscVal);
			
			break;
			
		case 400:
			/* CPU #3 HT0_HI */
			freqVal = LS3A_HT_ConfigRead(HT0_HI, 3, 0x48);
			printf("CPU#3, old HT0_HI_freq reg is 0x%x\n", freqVal);
			freqVal &= 0xfffff0ff;
			LS3A_HT_ConfigWrite(HT0_HI, 3, 0x48, freqVal | 0x200);
			printf("CPU#3, new HT0_HI_freq reg is 0x%x\n", LS3A_HT_ConfigRead(HT0_HI, 3, 0x48));
			/* CPU #1 HT0_LO */
			freqVal = LS3A_HT_ConfigRead(HT0_LO, 1, 0x48);
			printf("CPU#1, old HT0_LO_freq reg is 0x%x\n", freqVal);
			freqVal &= 0xfffff0ff;
			LS3A_HT_ConfigWrite(HT0_LO, 1, 0x48, freqVal | 0x200);
			printf("CPU#1, new HT0_LO_freq reg is 0x%x\n", LS3A_HT_ConfigRead(HT0_LO, 1, 0x48));
			/* Disconnect CPU#1 HT0_LO Bus, 0->1 */
			miscVal = LS3A_HT_ConfigRead(HT0_LO, 1, 0x50);
			miscVal &= 0xbfffffff;
			LS3A_HT_ConfigWrite(HT0_LO, 1, 0x50, miscVal);
			miscVal |= 0x40000000;
			LS3A_HT_ConfigWrite(HT0_LO, 1, 0x50, miscVal);
			
			/* CPU #0 HT0_HI */
			freqVal = LS3A_HT_ConfigRead(HT0_HI, 0, 0x48);
			printf("CPU#0, old HT0_HI_freq reg is 0x%x\n", freqVal);
			freqVal &= 0xfffff0ff;
			LS3A_HT_ConfigWrite(HT0_HI, 0, 0x48, freqVal | 0x200);
			printf("CPU#0, new HT0_HI_freq reg is 0x%x\n", LS3A_HT_ConfigRead(HT0_HI, 0, 0x48));
			/* CPU #2 HT0_LO */
			freqVal = LS3A_HT_ConfigRead(HT0_LO, 2, 0x48);
			printf("CPU#2, old HT0_LO_freq reg is 0x%x\n", freqVal);
			freqVal &= 0xfffff0ff;
			LS3A_HT_ConfigWrite(HT0_LO, 2, 0x48, freqVal | 0x200);
			printf("CPU#2, new HT0_LO_freq reg is 0x%x\n", LS3A_HT_ConfigRead(HT0_LO, 2, 0x48));
			/* Disconnect CPU#2 HT0_LO Bus, 0->1 */
			miscVal = LS3A_HT_ConfigRead(HT0_LO, 2, 0x50);
			miscVal &= 0xbfffffff;
			LS3A_HT_ConfigWrite(HT0_LO, 2, 0x50, miscVal);
			miscVal |= 0x40000000;
			LS3A_HT_ConfigWrite(HT0_LO, 2, 0x50, miscVal);

			/* CPU #1 HT0_HI */
			freqVal = LS3A_HT_ConfigRead(HT0_HI, 1, 0x48);
			printf("CPU#1, old HT0_HI_freq reg is 0x%x\n", freqVal);
			freqVal &= 0xfffff0ff;
			LS3A_HT_ConfigWrite(HT0_HI, 1, 0x48, freqVal | 0x200);
			printf("CPU#1, new HT0_HI_freq reg is 0x%x\n", LS3A_HT_ConfigRead(HT0_HI, 1, 0x48));
			/* CPU #3 HT0_LO */
			freqVal = LS3A_HT_ConfigRead(HT0_LO, 3, 0x48);
			printf("CPU#3, old HT0_LO_freq reg is 0x%x\n", freqVal);
			freqVal &= 0xfffff0ff;
			LS3A_HT_ConfigWrite(HT0_LO, 3, 0x48, freqVal | 0x200);
			printf("CPU#3, new HT0_LO_freq reg is 0x%x\n", LS3A_HT_ConfigRead(HT0_LO, 3, 0x48));
			/* Disconnect CPU#3 HT0_LO Bus, 0->1 */
			miscVal = LS3A_HT_ConfigRead(HT0_LO, 3, 0x50);
			miscVal &= 0xbfffffff;
			LS3A_HT_ConfigWrite(HT0_LO, 3, 0x50, miscVal);
			miscVal |= 0x40000000;
			LS3A_HT_ConfigWrite(HT0_LO, 3, 0x50, miscVal);

			/* CPU #2 HT0_HI */
			freqVal = LS3A_HT_ConfigRead(HT0_HI, 2, 0x48);
			printf("CPU#2, old HT0_HI_freq reg is 0x%x\n", freqVal);
			freqVal &= 0xfffff0ff;
			LS3A_HT_ConfigWrite(HT0_HI, 2, 0x48, freqVal | 0x200);
			printf("CPU#2, new HT0_HI_freq reg is 0x%x\n", LS3A_HT_ConfigRead(HT0_HI, 2, 0x48));
			/* CPU #0 HT0_LO */
			freqVal = LS3A_HT_ConfigRead(HT0_LO, 0, 0x48);
			printf("CPU#0, old HT0_LO_freq reg is 0x%x\n", freqVal);
			freqVal &= 0xfffff0ff;
			LS3A_HT_ConfigWrite(HT0_LO, 0, 0x48, freqVal | 0x200);
			printf("CPU#0, new HT0_LO_freq reg is 0x%x\n", LS3A_HT_ConfigRead(HT0_LO, 0, 0x48));
			/* Disconnect CPU#2 HT0_LO Bus, 0->1 */
			miscVal = LS3A_HT_ConfigRead(HT0_LO, 0, 0x50);
			miscVal &= 0xbfffffff;
			LS3A_HT_ConfigWrite(HT0_LO, 0, 0x50, miscVal);
			miscVal |= 0x40000000;
			LS3A_HT_ConfigWrite(HT0_LO, 0, 0x50, miscVal);
			
			break;
			
		case 800:
			/* CPU #3 HT0_HI */
			freqVal = LS3A_HT_ConfigRead(HT0_HI, 3, 0x48);
			printf("CPU#3, old HT0_HI_freq reg is 0x%x\n", freqVal);
			freqVal &= 0xfffff0ff;
			LS3A_HT_ConfigWrite(HT0_HI, 3, 0x48, freqVal | 0x500);
			printf("CPU#3, new HT0_HI_freq reg is 0x%x\n", LS3A_HT_ConfigRead(HT0_HI, 3, 0x48));
			/* CPU #1 HT0_LO */
			freqVal = LS3A_HT_ConfigRead(HT0_LO, 1, 0x48);
			printf("CPU#1, old HT0_LO_freq reg is 0x%x\n", freqVal);
			freqVal &= 0xfffff0ff;
			LS3A_HT_ConfigWrite(HT0_LO, 1, 0x48, freqVal | 0x500);
			printf("CPU#1, new HT0_LO_freq reg is 0x%x\n", LS3A_HT_ConfigRead(HT0_LO, 1, 0x48));
			/* Disconnect CPU#1 HT0_LO Bus, 0->1 */
			miscVal = LS3A_HT_ConfigRead(HT0_LO, 1, 0x50);
			miscVal &= 0xbfffffff;
			LS3A_HT_ConfigWrite(HT0_LO, 1, 0x50, miscVal);
			miscVal |= 0x40000000;
			LS3A_HT_ConfigWrite(HT0_LO, 1, 0x50, miscVal);
			
			/* CPU #0 HT0_HI */
			freqVal = LS3A_HT_ConfigRead(HT0_HI, 0, 0x48);
			printf("CPU#0, old HT0_HI_freq reg is 0x%x\n", freqVal);
			freqVal &= 0xfffff0ff;
			LS3A_HT_ConfigWrite(HT0_HI, 0, 0x48, freqVal | 0x500);
			printf("CPU#0, new HT0_HI_freq reg is 0x%x\n", LS3A_HT_ConfigRead(HT0_HI, 0, 0x48));
			/* CPU #2 HT0_LO */
			freqVal = LS3A_HT_ConfigRead(HT0_LO, 2, 0x48);
			printf("CPU#2, old HT0_LO_freq reg is 0x%x\n", freqVal);
			freqVal &= 0xfffff0ff;
			LS3A_HT_ConfigWrite(HT0_LO, 2, 0x48, freqVal | 0x500);
			printf("CPU#2, new HT0_LO_freq reg is 0x%x\n", LS3A_HT_ConfigRead(HT0_LO, 2, 0x48));
			/* Disconnect CPU#2 HT0_LO Bus, 0->1 */
			miscVal = LS3A_HT_ConfigRead(HT0_LO, 2, 0x50);
			miscVal &= 0xbfffffff;
			LS3A_HT_ConfigWrite(HT0_LO, 2, 0x50, miscVal);
			miscVal |= 0x40000000;
			LS3A_HT_ConfigWrite(HT0_LO, 2, 0x50, miscVal);

			/* CPU #1 HT0_HI */
			freqVal = LS3A_HT_ConfigRead(HT0_HI, 1, 0x48);
			printf("CPU#1, old HT0_HI_freq reg is 0x%x\n", freqVal);
			freqVal &= 0xfffff0ff;
			LS3A_HT_ConfigWrite(HT0_HI, 1, 0x48, freqVal | 0x500);
			printf("CPU#1, new HT0_HI_freq reg is 0x%x\n", LS3A_HT_ConfigRead(HT0_HI, 1, 0x48));
			/* CPU #3 HT0_LO */
			freqVal = LS3A_HT_ConfigRead(HT0_LO, 3, 0x48);
			printf("CPU#3, old HT0_LO_freq reg is 0x%x\n", freqVal);
			freqVal &= 0xfffff0ff;
			LS3A_HT_ConfigWrite(HT0_LO, 3, 0x48, freqVal | 0x500);
			printf("CPU#3, new HT0_LO_freq reg is 0x%x\n", LS3A_HT_ConfigRead(HT0_LO, 3, 0x48));
			/* Disconnect CPU#3 HT0_LO Bus, 0->1 */
			miscVal = LS3A_HT_ConfigRead(HT0_LO, 3, 0x50);
			miscVal &= 0xbfffffff;
			LS3A_HT_ConfigWrite(HT0_LO, 3, 0x50, miscVal);
			miscVal |= 0x40000000;
			LS3A_HT_ConfigWrite(HT0_LO, 3, 0x50, miscVal);

			/* CPU #2 HT0_HI */
			freqVal = LS3A_HT_ConfigRead(HT0_HI, 2, 0x48);
			printf("CPU#2, old HT0_HI_freq reg is 0x%x\n", freqVal);
			freqVal &= 0xfffff0ff;
			LS3A_HT_ConfigWrite(HT0_HI, 2, 0x48, freqVal | 0x500);
			printf("CPU#2, new HT0_HI_freq reg is 0x%x\n", LS3A_HT_ConfigRead(HT0_HI, 2, 0x48));
			/* CPU #0 HT0_LO */
			freqVal = LS3A_HT_ConfigRead(HT0_LO, 0, 0x48);
			printf("CPU#0, old HT0_LO_freq reg is 0x%x\n", freqVal);
			freqVal &= 0xfffff0ff;
			LS3A_HT_ConfigWrite(HT0_LO, 0, 0x48, freqVal | 0x500);
			printf("CPU#0, new HT0_LO_freq reg is 0x%x\n", LS3A_HT_ConfigRead(HT0_LO, 0, 0x48));
			/* Disconnect CPU#2 HT0_LO Bus, 0->1 */
			miscVal = LS3A_HT_ConfigRead(HT0_LO, 0, 0x50);
			miscVal &= 0xbfffffff;
			LS3A_HT_ConfigWrite(HT0_LO, 0, 0x50, miscVal);
			miscVal |= 0x40000000;
			LS3A_HT_ConfigWrite(HT0_LO, 0, 0x50, miscVal);
			
			break;
			
		default:
			printf("Now we only support 200/400/800MHz HT frequency! %d is illegal!!\n", freq);
			CLOSE_L1XBAR_HT0_WIN();
			return ERROR;	
	}
	CLOSE_L1XBAR_HT0_WIN();
	printf("done!\n");
	/*xbarShow();*/
	return OK;
}
