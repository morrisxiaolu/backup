#ifndef SIMPLEPRINTF
#define SIMPLEPRINTF

#define DebugPrint

#ifdef DebugPrint
#define PORT3f8 /*0xbff003f8*/0xbfe001e0
#define PORT2f8 /*0xbff002f8*/0xbfd002f8

/*
 * Macros to access the system control coprocessor
 */
#if  1  /*added by pdm. 2010.4.12*/
__asm int getNodeNumberZR ()
{
! "$2"						
	mfc0	$2, $15, 1
        andi    $2, $2,  0x0c
        srl     $2, $2,  2	
}
#endif  /*pdm*/

 
 
__asm int read_status ()
{
! "$2"						/* v0 clobbered */
	mfc0	$2, $12				/* count leading zeros */
}	
__asm int read_wire ()
{
! "$2"						/* v0 clobbered */
	mfc0	$2, $6				/* count leading zeros */
}	
__asm int read_once ()
{
! "$2"						/* v0 clobbered */
	dli	  $2,0x900000003ff01424
	lw	  $2,0($2)               /* count leading zeros */
}	
__asm int read_sp ()
{
! "$2"						/* v0 clobbered */
	move	  $2,sp
}

__asm int read_ra ()
{
! "$2"						/* v0 clobbered */
	move	  $2,ra
}

__asm int read_realra ()
{
! "$2"						/* v0 clobbered */
	lw	$2,1008(sp)
}

#if 0
__asm volatile BOOL vxCas_inline (atomic_t * address, atomicVal_t old_value,
				  atomicVal_t new_value)
{
% reg address; reg old_value; reg new_value; \
  lab vxCas1_inline, vxCas2_inline
vxCas1_inline:
	.set noreorder
	ll	t0, 0(address)
	bne	t0, old_value, vxCas2_inline
	li	v0, FALSE
	move	t1, new_value
	sc	t1, 0(address)
	beqz	t1, vxCas1_inline
	li	v0, FALSE
	li	v0, TRUE
	.set reorder
vxCas2_inline:
}
#endif
/********************* added by liuw for HT_PCI config********************/
__asm volatile void MIPS_SW64_HT1LO_PCICFG_BASE_TP1(unsigned int address, unsigned int value) 
{
% reg address; reg value;                       
	.set noreorder 
	dli	t0,0x90000efdff000000
	or  t0,t0,address
	sw	value, 0(t0)
	sync
	.set reorder
}

__asm volatile void MIPS_SW64_HT1LO_PCICFG_BASE(unsigned int address, unsigned int value) 
{
% reg address; reg value;                       
	.set noreorder 
	dli	t0,0x90000efdfe000000
	or  t0,t0,address
	sw	value, 0(t0)
	sync
	.set reorder
}

__asm volatile unsigned int MIPS_LW64_HT1LO_PCICFG_BASE(unsigned int address) 
{
% reg address;
! "$2"
	.set noreorder 
	dli	t0,0x90000efdfe000000
	or  t0,t0,address
	lw	$2, 0(t0)
	.set reorder
}
__asm volatile unsigned int MIPS_LW64_HT1LO_PCICFG_BASE_TP1(unsigned int address) 
{
% reg address;
! "$2"
	.set noreorder 
	dli	t0,0x90000efdff000000
	or  t0,t0,address
	lw	$2, 0(t0)
	.set reorder
}
__asm volatile unsigned int TEST_LW64(unsigned int address) 
{
% reg address;
! "$2"
	.set noreorder 
	dli	t0,0x900000003ff00000
	or  t0,t0,address
	lw	$2, 0(t0)
	.set reorder
}

__asm volatile void MIPS_SW64_IO_base_regs_addr(unsigned int address, unsigned int value) 
{
% reg address; reg value;                       
	.set noreorder 
	dli	t0,0x900000003ff00000
	or  t0,t0,address
	sw	value, 0(t0)
/*	sync*/
	.set reorder
}


__asm volatile void MIPS_SB64_IO_base_regs_addr(unsigned int address, unsigned char value) 
{
% reg address; reg value;                       
	.set noreorder 
	dli	t0,0x900000003ff00000
	or  t0,t0,address
	sb	value, 0(t0)
/*	sync*/
	.set reorder
}
__asm volatile void MIPS_SB64(unsigned int address, unsigned char value) /*zxj,20091208*/
{
% reg address; reg value;                       
	.set noreorder 
	dli	t0,0x9000000000000000
	or  t0,t0,address
	sb	value, 0(t0)
	/*sync*/
	.set reorder
}
__asm volatile void MIPS_SW64(unsigned int address, unsigned int value) /*zxj,20091208*/
{
% reg address; reg value;                       
	.set noreorder 
	dli	t0,0x9000000000000000
	or  t0,t0,address
	sw	value, 0(t0)
	/*sync*/
	.set reorder
}

__asm volatile void MIPS_SD64(unsigned int address, unsigned long value) /*lw,20100818*/
{
% reg address; reg value;                       
	.set noreorder 
	dli	t0,0x9000000000000000	 
    or  t0,t0,address        
	sd	value, 0(t0)
	/*sync*/
	.set reorder
}

__asm volatile void MIPS_SD64_a(unsigned int address, unsigned long value) /*lw,20100818*/
{
% reg address; reg value;                       
	.set noreorder 
	dli	t0,0x9000000000000000
	dli t1,0x9000000000000000
    or  t0,t0,address    
    or  t1,t1,value
	sd	t1, 0(t0)
	/*sync*/
	.set reorder
}

__asm volatile void MIPS_WRITE64(unsigned int address, unsigned int datah, unsigned int datal)
{
% reg address; reg datah; reg datal;                       
	.set noreorder 
	dli t0,0x9000000000000000
	or  t0,t0,address
	dsll32 t1,datah,0
	or t1,t1,datal
	sd  t1,0(t0)
/*	sync*/
	.set reorder
}


 
__asm volatile void CPU_WRITE32(unsigned int address,unsigned int cpunum, unsigned int data)
{
% reg address; reg cpunum; reg data;                     
	.set noreorder 
	dli t0,0x9000000000000000
	or  t0,t0,address
	dsll32 t1,cpunum, 12
	or	t0,t0,t1
	sw  data,0(t0)
	/*sync*/
	.set reorder
}

__asm volatile void CPU_WRITE16(unsigned int address,unsigned int cpunum, unsigned short data)
{
% reg address; reg cpunum; reg data;                      
	.set noreorder 
	dli t0,0x9000000000000000
	or  t0,t0,address
	dsll32 t1,cpunum, 12
	or	t0,t0,t1
	sh  data,0(t0)
/*	sync*/
	.set reorder
}

__asm volatile void CPU_WRITE8(unsigned int address,unsigned int cpunum, unsigned char data)
{
% reg address; reg cpunum; reg data;                      
	.set noreorder 
	dli t0,0x9000000000000000
	or  t0,t0,address
	dsll32 t1,cpunum, 12
	or	t0,t0,t1
	sb  data,0(t0)
	/*sync*/
	.set reorder
}

__asm volatile unsigned char CPU_READ8(unsigned int address,unsigned int cpunum) /*zxj,20091208*/
{
% reg address;reg cpunum; 
! "$2"
	.set noreorder 
	dli	t0,0x9000000000000000
	or  t0,t0,address
	dsll32 t1,cpunum, 12
	or	t0,t0,t1
	lb	$2, 0(t0)
	.set reorder
}

__asm volatile unsigned short CPU_READ16(unsigned int address,unsigned int cpunum) /*zxj,20091208*/
{
% reg address;reg cpunum; 
! "$2"
	.set noreorder 
	dli	t0,0x9000000000000000
	or  t0,t0,address
	dsll32 t1,cpunum, 12
	or	t0,t0,t1
	lh	$2, 0(t0)
	.set reorder
}

__asm volatile unsigned int CPU_READ32(unsigned int address,unsigned int cpunum) /*zxj,20091208*/
{
% reg address;reg cpunum; 
! "$2"
	.set noreorder 
	dli	t0,0x9000000000000000
	or  t0,t0,address
	dsll32 t1,cpunum, 12
	or	t0,t0,t1
	lw	$2, 0(t0)
	.set reorder
}


__asm volatile void CPU_WRITE32_C(unsigned int address,unsigned int cpunum, unsigned int data)
{
% reg address; reg cpunum; reg data;                     
	.set noreorder 
	dli t0,0x9800000000000000
	or  t0,t0,address
	dsll32 t1,cpunum, 12
	or	t0,t0,t1
	sw  data,0(t0)
	/*sync*/
	.set reorder
}
__asm volatile unsigned int CPU_READ32_C(unsigned int address,unsigned int cpunum) /*liuw,20110714*/
{
% reg address;reg cpunum; 
! "$2"
	.set noreorder 
	dli	t0,0x9800000000000000
	or  t0,t0,address
	dsll32 t1,cpunum, 12
	or	t0,t0,t1
	lw	$2, 0(t0)
	.set reorder
}

__asm volatile void HT0L_CONF_WRITE(unsigned int address,unsigned int data)
{
% reg address; reg data;                     
	.set noreorder 
	dli t0,0x90000CFDFB000000
	or  t0,t0,address
	sw  data,0(t0)
	/*sync*/
	.set reorder
}
__asm volatile void HT0H_CONF_WRITE(unsigned int address,unsigned int data)
{
% reg address; reg data;                     
	.set noreorder 
	dli t0,0x90000DFDFB000000
	or  t0,t0,address
	sw  data,0(t0)
	/*sync*/
	.set reorder
}
__asm volatile void HT1L_CONF_WRITE(unsigned int address,unsigned int data)
{
% reg address; reg data;                     
	.set noreorder 
	dli t0,0x90000EFDFB000000
	or  t0,t0,address
	sw  data,0(t0)
	/*sync*/
	.set reorder
}
__asm volatile void HT1H_CONF_WRITE(unsigned int address,unsigned int data)
{
% reg address; reg data;                     
	.set noreorder 
	dli t0,0x90000FFDFB000000
	or  t0,t0,address
	sw  data,0(t0)
	/*sync*/
	.set reorder
}


__asm volatile unsigned int  HT0L_CONF_READ(unsigned int address)
{
% reg address;
! "$2"
	.set noreorder 
	dli	t0,0x90000CFDFB000000
	or  t0,t0,address
	lw	$2, 0(t0)
	.set reorder
}

__asm volatile unsigned int  HT0H_CONF_READ(unsigned int address)
{
% reg address;
! "$2"
	.set noreorder 
	dli	t0,0x90000DFDFB000000
	or  t0,t0,address
	lw	$2, 0(t0)
	.set reorder
}
__asm volatile unsigned int  HT1L_CONF_READ(unsigned int address)
{
% reg address;
! "$2"
	.set noreorder 
	dli	t0,0x90000EFDFB000000
	or  t0,t0,address
	lw	$2, 0(t0)
	.set reorder
}

__asm volatile unsigned int  HT1H_CONF_READ(unsigned int address)
{
% reg address;
! "$2"
	.set noreorder 
	dli	t0,0x90000FFDFB000000
	or  t0,t0,address
	lw	$2, 0(t0)
	.set reorder
}

__asm volatile void MIPS_SW64_SM(unsigned int address, unsigned int cpunum, unsigned int value) /*yinwx, 20100203*/
{
% reg address; reg cpunum; reg value;                       
	.set noreorder 
	dli	t0,0x9000000000000000
	or  t0,t0,address
	dsll32 t1,cpunum, 12
	or	t0,t0,t1
	sw	value, 0(t0)
/*	sync*/
	.set reorder
}

__asm volatile unsigned int MIPS_LW64(unsigned int address) /*zxj,20091208*/
{
% reg address;
! "$2"
	.set noreorder 
	dli	t0,0x9000000000000000
	or  t0,t0,address
	lw	$2, 0(t0)
	.set reorder
}

__asm volatile void  MIPS_LD64(unsigned int address, unsigned int* datah, unsigned int* datal)
{
% reg address; reg datah; reg datal;                       
	.set noreorder 
	dli t0,0x9000000000000000
	or  t0,t0,address
	ld  t1,0(t0)
	sw  t1,0(datal)
	dsrl32 t1,t1,0
	sw  t1,0(datah)
	/*sync*/
	.set reorder
}

__asm volatile void SET_XBAR_CHIPDMA(unsigned int winbase, unsigned int nodenum)
{
% reg winbase; reg nodenum;                       
	.set noreorder 
	dli	t0,0x9000000000000000
	or  t0,t0,winbase
	dsll32 t1,nodenum,12
	or	t0,t0,t1
	dli t1,0x0
	sd	t1,0x80(t0)
	dli t1,0x0000000007ff0000
	sd	t1,0(t0)
	dli	t1,0xffffffffffff0000
	sd	t1,0x40(t0)
	dli t1,0x0000000007ffffff
	sd	t1,0x80(t0)
/*	sync*/
	nop	
	.set reorder
}
__asm volatile void UNSET_XBAR_CHIPDMA(unsigned int winbase, unsigned int nodenum)
{
% reg winbase; reg nodenum;                       
	.set noreorder 
	dli	t0,0x9000000000000000
	or  t0,t0,winbase
	dsll32 t1,nodenum,12
	or	t0,t0,t1
	dli t1,0x0
	sd	t1,0x80(t0)
	dli t1,0x0
	sd	t1,0(t0)
	dli	t1,0xffffffff00000c00
	sd	t1,0x40(t0)
	dli t1,0xf0
	sd	t1,0x80(t0)
/*	sync*/
	nop	
	.set reorder
}



__asm volatile void SET_XBAR_SM(unsigned int winbase, unsigned int nodenum)
{
% reg winbase; reg nodenum;                       
	.set noreorder 
	dli	t0,0x9000000000000000
	or  t0,t0,winbase
	dsll32 t1,nodenum,12
	or	t0,t0,t1
	dli t1,0x0
	sd	t1,0x80(t0)
	dli t1,0x000000003ff00000
	sd	t1,0(t0)
	dli	t1,0xfffffffffff00000
	sd	t1,0x40(t0)
	dli t1,0x000000003ff00086
	sd	t1,0x80(t0)
	/*sync*/
	nop	
	.set reorder
}

__asm volatile void UNSET_XBAR_SM(unsigned int winbase, unsigned int nodenum)
{
% reg winbase; reg nodenum;                       
	.set noreorder 
	dli	t0,0x9000000000000000
	or  t0,t0,winbase
	dsll32 t1,nodenum,12
	or	t0,t0,t1
	dli t1,0x0
	sd	t1,0x80(t0)
	dli t1,0x0
	sd	t1,0(t0)
		dli	t1,0x0 /*0xffffffff00000c00*//*zk 20110126 for 16k cache*/
	sd	t1,0x40(t0)
		dli t1,0x0 /*0xf0*//*zk 20110126 for 16k cache*/
	sd	t1,0x80(t0)
	sync
	nop	
	.set reorder
}

/*
 * Macros to access the system control coprocessor
 */
#if 1 
#define __read_32bit_c0_register(source, sel)               \
({ int __res;                               \
    if (sel == 0)                           \
        __asm__ __volatile__(                   \
            "mfc0\t%0, " #source "\n\t"         \
            : "=r" (__res));                \
    else                                \
        __asm__ __volatile__(                   \
            ".set\tmips32\n\t"              \
            "mfc0\t%0, " #source ", " #sel "\n\t"       \
            ".set\tmips0\n\t"               \
            : "=r" (__res));                \
    __res;                              \
})
 
#define __read_64bit_c0_register(source, sel)               \
({ unsigned long long __res;                        \
    if (sizeof(unsigned long) == 4)                 \
        __res = __read_64bit_c0_split(source, sel);     \
    else if (sel == 0)                      \
        __asm__ __volatile__(                   \
            ".set\tmips3\n\t"               \
            "dmfc0\t%0, " #source "\n\t"            \
            ".set\tmips0"                   \
            : "=r" (__res));                \
    else                                \
        __asm__ __volatile__(                   \
            ".set\tmips64\n\t"              \
            "dmfc0\t%0, " #source ", " #sel "\n\t"      \
            ".set\tmips0"                   \
            : "=r" (__res));                \
    __res;                              \
})

#define __write_32bit_c0_register(register, sel, value)         \
do {                                    \
    if (sel == 0)                           \
        __asm__ __volatile__(                   \
            "mtc0\t%z0, " #register "\n\t"          \
            : : "Jr" ((unsigned int)(value)));      \
    else                                \
        __asm__ __volatile__(                   \
            ".set\tmips32\n\t"              \
            "mtc0\t%z0, " #register ", " #sel "\n\t"    \
            ".set\tmips0"                   \
            : : "Jr" ((unsigned int)(value)));      \
} while (0)
 
#define __write_64bit_c0_register(register, sel, value)         \
do {                                    \
    if (sizeof(unsigned long) == 4)                 \
        __write_64bit_c0_split(register, sel, value);       \
    else if (sel == 0)                      \
        __asm__ __volatile__(                   \
            ".set\tmips3\n\t"               \
            "dmtc0\t%z0, " #register "\n\t"         \
            ".set\tmips0"                   \
            : : "Jr" (value));              \
    else                                \
        __asm__ __volatile__(                   \
            ".set\tmips64\n\t"              \
            "dmtc0\t%z0, " #register ", " #sel "\n\t"   \
            ".set\tmips0"                   \
            : : "Jr" (value));              \
} while (0)

#define read_c0(x)   __read_32bit_c0_register(x, 0)
#define read_c0_status __read_32bit_c0_register($12, 0)
#define read_c0_cause __read_32bit_c0_register($13, 0)
#define write_c0(x, val)  __write_32bit_c0_register(x, 0, val)
#define write_c0_status(val)	write_c0($12, val)
#define write_c0_cause(val)		write_c0($13, val)
#endif

#endif

#endif
