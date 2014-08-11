#include <stdio.h>
#include <stdlib.h>
#include <vxWorks.h>
#include <cacheLib.h>
#include <intLib.h>
#include <logLib.h>
#include <muxLib.h>
#include <netLib.h>
#include <netBufLib.h>
#include <semLib.h>
#include <sysLib.h>
#include <vxBusLib.h>
#include <vxbTimerLib.h>
#include <tickLib.h>
#include <taskLib.h>

#include <wdLib.h>

#include <hwif/vxbus/vxBus.h>
#include <hwif/vxbus/hwConf.h>
#include <hwif/vxbus/vxbPlbLib.h>
#include <hwif/util/vxbParamSys.h>
#include <../src/hwif/h/vxbus/vxbAccess.h>

#include "hrMatrixDma.h"

#define SECONDS 10

WDOG_ID mywatchdogId ;



static SEM_ID semMatrixDma0;
static SEM_ID semMatrixDma1;

extern void printstr(char *s);
extern int sb1_clock_rate; /*clock rate, in this project it's 500000000Hz*/
unsigned long long starttime0,overtime0,countnum0; /*64bit, test dma transmission time*/
unsigned long long starttime1,overtime1,countnum1;



/*struct timespec
	{
	double ticknum;
	double countnum;
	double time;
};*/

//struct timespec starttime;
//struct timespec overtime;
	
typedef struct
{
	VXB_DEVICE_ID pDev;			 	/* vxbus 要求的项目*/
	unsigned int intSource;			/* intSource */
	unsigned int intLine;			/* intline of core*/
	unsigned int coreNum;			/* core Num */
}HRMATRIXDMA;  /*2 matrix dma use this same struct*/

/*Matrix Dma 0*/
 void hrMatrixDma0InstInit(struct vxbDev * pDev);
 void hrMatrixDma0InstInit2(struct vxbDev * pDev);
 void hrMatrixDma0InstConnect(struct vxbDev * pDev);
 BOOL hrMatrixDma0Probe(struct vxbDev * pDev);
 static void hrMatrixDma0Int(VXB_DEVICE_ID pDev);

 extern int bsp_printf(const char *  fmt, ...);

 //LOCAL UINT32 microtime(struct timespec *tvp);
 unsigned long long getcount( );

__asm volatile void XNOP16()
{
	.set noreorder
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	.set reorder	
}

void sysXNop16()
{
	XNOP16();
}


unsigned int hrToPhys(unsigned int addr)
{
	unsigned int c;
	unsigned int m,n;
	c = (unsigned int)addr;
	m = (c & 0x0fffffff);
	c >>= 28;
	c &= 0xf;
	n = 0;
	switch(c)
	{
	case 0x0:  /*this only can be phys address*/
		n = (m|0x00000000);
		break;
	case 0x1:  /*this only can be phys address*/
		n = (m|0x10000000);
		break;
	case 0x2:  /*this only can be phys address*/
		n = (m|0x20000000);
		break;
	case 0x3:  /*this only can be phys address*/
		n = (m|0x30000000);
		break;
	case 0x4:  /*may be mapped address of phys address, 
	when it is a mapped address, return may be  0x20000000,
	but we ignore it, because it only used in smnet*/
		n = (m|0x40000000);
		break;
	case 0x5:   /*this only can be phys address*/
		n = (m|0x50000000);
		break;
	case 0x6:   /*this can be phys address or mapped address, but they are same!*/
		n = (m|0x60000000);
		break;
	case 0x7:  /*this can be phys address or mapped address, but they are same!*/
		n = (m|0x70000000);
		break;
	case 0x8:  /*this only can be kseg 0 address*/
		n = (m|0x00000000);
		break;
	case 0x9:  /*this only can be kseg 0 address,io space*/
		n = (m|0x10000000);
		break;
	case 0xa:  /*this only can be ksge1 address*/
		n = (m|0x00000000);
		break;
	case 0xb:  /*this only can be kseg1 address*/
		n = (m|0x10000000);
		break;
	case 0xc:  /*this only can be mapped address*/
		n = (m|0x00000000);
		break;  
	case 0xd: /*this only can be mapped address*/
		n = (m|0x20000000);
		break;
	case 0xe: /*this only can be mapped address*/
		n = (m|0x60000000);
		break;
	case 0xf: /*this only can be mapped address*/
		n = (m|0x70000000);
		break;	
	default:  /*other address not defined*/
	//	bslDebug("hrToPhys: error address 0x08x format!!\r\n",addr);
		printf("hrToPhys: error address 0x08x format!!\r\n",addr);
		break;
	}
	return n;		
}

__asm volatile void XWR64(unsigned int addrh, unsigned int addrl,unsigned int valh,unsigned int vall)
{
% reg addrh; reg addrl; reg valh; reg vall;                       
	.set noreorder 
	move t0,addrh
	dsll32 t0,t0,0
    or  t0,t0,addrl
	dsll32 t1,valh,0
	or t1,t1,vall
	sd  t1,0(t0)
	sync
	.set reorder
}
__asm volatile void XRD64(unsigned int addrh,unsigned int addrl,unsigned int * valh,unsigned int * vall)
{
% reg addrh; reg addrl; reg valh; reg vall;
	.set noreorder 
	move t0,addrh
	dsll32 t0,t0,0
	or  t0,t0,addrl
	ld  t1,0(t0)
	sw  t1,0(vall)
	dsrl32 t1,t1,0
	sw  t1,0(valh)
	sync
	.set reorder
}
__asm volatile void XWR32(unsigned int addrh, unsigned int addrl,unsigned int val)
{
% reg addrh; reg addrl; reg val;                       
	.set noreorder 
	move t0,addrh
	dsll32 t0,t0,0
    or  t0,t0,addrl
    move t1,val;
	sw  t1,0(t0)
	sync
	.set reorder
}
__asm volatile unsigned int XRD32(unsigned int addrh,unsigned int addrl)
{
% reg addrh; reg addrl;
! "$2"
	.set noreorder
	move t0,addrh
	dsll32 t0,t0,0
	or  t0,t0,addrl
	lw $2,0(t0);
	.set reorder	
}
__asm volatile void XWR16(unsigned int addrh, unsigned int addrl,unsigned short val)
{
% reg addrh; reg addrl; reg val;                       
	.set noreorder 
	move t0,addrh
	dsll32 t0,t0,0
    or  t0,t0,addrl
    move t1,val;
	sh  t1,0(t0)
	sync
	.set reorder
}

__asm volatile unsigned short XRD16(unsigned int addrh,unsigned int addrl)
{
% reg addrh; reg addrl;
! "$2"
	.set noreorder
	move t0,addrh
	dsll32 t0,t0,0
	or  t0,t0,addrl
	lh $2,0(t0);
	.set reorder	
}
__asm volatile void XWR8(unsigned int addrh,unsigned int addrl,unsigned char val)
{
% reg addrh; reg addrl; reg val;                       
	.set noreorder 
	move t0,addrh
	dsll32 t0,t0,0
	or  t0,t0,addrl
	move t1,val;
	sb  t1,0(t0)
	sync
	.set reorder
}

__asm volatile unsigned char XRD8(unsigned int addrh,unsigned int addrl)
{
% reg addrh; reg addrl;
! "$2"
	.set noreorder
	move t0,addrh
	dsll32 t0,t0,0
	or  t0,t0,addrl
	lb $2,0(t0);
	.set reorder	
}

__asm volatile void XDMACpy(unsigned int dsth,unsigned int dstl,unsigned int srch,unsigned int srcl,unsigned int size)
{
% reg dsth; reg dstl; reg srch; reg srcl; reg size;
.set noreorder
	move t0,srch
	dsll32 t0,t0,0
	or t0,t0,srcl
	move t1,dsth
	dsll32 t1,t1,0
	or t1,t1,dstl
loopcpyXDMA:
	addu size, size, -32
	ld t2, 0(t0)
	ld t3, 8(t0)
	ld t4, 16(t0)
	ld t5, 24(t0)
	daddiu t0, t0, 32
	sd t2, 0(t1)
	sd t3, 8(t1)
	sd t4, 16(t1)
	sd t5, 24(t1)
	bnez size, loopcpyXDMA
	daddiu t1, t1, 32
	.set reorder	
}



__asm volatile void SYNC()
{
	.set noreorder
	sync
	.set reorder	
}


void sysXRD64(unsigned int addrh,unsigned int addrl,unsigned int * valh,unsigned int * vall)
{
		
	XRD64(addrh,addrl,valh,vall);
}


unsigned int sysXRD32(unsigned int addrh,unsigned int addrl)
{
	unsigned int addrl0;
	addrl0 = hrToPhys(addrl);
	return XRD32(addrh,addrl0);
}
unsigned short sysXRD16(unsigned int addrh,unsigned int addrl)
{
	unsigned int addrl0;
	addrl0 = hrToPhys(addrl);
	return XRD16(addrh,addrl0);
}
unsigned char sysXRD8(unsigned int addrh,unsigned int addrl)
{
	unsigned int addrl0;
	addrl0 = hrToPhys(addrl);
	return XRD8(addrh,addrl0);
}


void sysXWR64(unsigned int addrh,unsigned int addrl,unsigned int valh,unsigned int vall)
{
	unsigned int val0;
	
	val0 = hrToPhys(vall);

//	printf("addrl = %#x \r\n", val0);
	XWR64(addrh,addrl,valh,val0);
	
}

/*used to write reg,the addrl is reg address,write the value to addrl directly*/
void sysXWR32(unsigned int addrh,unsigned int addrl,unsigned int value)
{
	unsigned int addrl0;
	addrl0 = hrToPhys(addrl);
	XWR32(addrh,addrl0,value); 
}

void sysXWR16(unsigned int addrh,unsigned int addrl,unsigned short value)
{
	unsigned int addrl0;
	addrl0 = hrToPhys(addrl);
	XWR16(addrh,addrl0,value);
}

void sysXWR8(unsigned int addrh,unsigned int addrl,unsigned char value)
{
	unsigned int addrl0;
	addrl0 = hrToPhys(addrl);
	XWR8(addrh,addrl0,value);
}


 struct drvBusFuncs hrMatrixDma0Funcs =
{
	hrMatrixDma0InstInit,     /* devInstanceInit */
	hrMatrixDma0InstInit2,    /* devInstanceInit2 */
	hrMatrixDma0InstConnect   /* devConnect */
};
 device_method_t hrMatrixDma0Methods[] ={{0, 0}};
 struct vxbPlbRegister hrMatrixDma0Registration =
{
    {
        NULL,                 /* pNext,一般都用NULL */
        VXB_DEVID_DEVICE,     /* devID,固定,表示这是一个设备 */
        VXB_BUSID_PLB,        /* busID = PLB ,表示这个设备挂接在PLB上*/
        VXBUS_VERSION_4,      /* 版本号,6.7为版本4 */
        HRMATRIXDMA0_VXBNAME,     /*设备名称,必须和hwconfig.c中统一*/
        &hrMatrixDma0Funcs,       		/* pDrvBusFuncs,设备总线函数组在后面定义 */
        hrMatrixDma0Methods,     		/* pMethods ,设备方法组,在后面定义*/
        hrMatrixDma0Probe         		/* devProbe ,设备探测*/
    }
};
 void hrMatrixdma0Register(void){
	vxbDevRegister((struct vxbDevRegInfo *)&(hrMatrixDma0Registration));
   return;
}
 void hrMatrixDma0InstInit (struct vxbDev * pDev)
{
    vxbNextUnitGet(pDev);
    return;
}
 void hrMatrixDma0InstInit2(struct vxbDev * pDev)
{
	HRMATRIXDMA *pDrvCtrl;
	semMatrixDma0 = semBCreate(SEM_Q_PRIORITY,SEM_EMPTY);
    pDrvCtrl = malloc(sizeof(HRMATRIXDMA));
    if (pDrvCtrl == NULL) return;
    bzero ((char *)pDrvCtrl, sizeof(HRMATRIXDMA));
    pDev->pDrvCtrl = pDrvCtrl;
    pDrvCtrl->pDev = pDev;
    return;
}
 void hrMatrixDma0InstConnect(struct vxbDev * pDev)
{
	if(vxbIntConnect(pDev,0,hrMatrixDma0Int,(void *)pDev) != OK)
	{
		printstr("hrMatrixDma0InstConnect: vxbIntConnect Fail\r\n");
	}
	else
	{
		printstr("hrMatrixDma0InstConnect: vxbIntConnect OK\r\n");
	}
	if(vxbIntEnable(pDev,0,hrMatrixDma0Int,(void *)pDev) != OK)
	{
		printstr("hrMatrixDma0InstConnect: vxbIntEnable Fail\r\n");
	}
	else
	{
		printstr("hrMatrixDma0InstConnect: vxbIntEnable OK\r\n");
	}
}

 

 BOOL hrMatrixDma0Probe(struct vxbDev * pDev){return TRUE;}
 static void hrMatrixDma0Int(VXB_DEVICE_ID pDev)
{
	int lockId;
	unsigned int n;
	lockId=intCpuLock();
//	printstr("catch dma0 int \r\n");
	n = sysXRD32(0x90000000,DMA0_TRANS_STATUS);
	//printstr("catch dma0 int 1\r\n");
	n &= DMA_TRANS_FINISHED;
	if(n != 0) /*stop dma*/
	{
	   // printstr("catch dma0 int 2\r\n");
//		overtime0=getcount();
		sysXWR32(0x90000000,DMA0_TRANS_CTRL,0);
		semGive(semMatrixDma0);
//		printstr("matrixdma0 int ok\r\n");
	}
	intCpuUnlock(lockId);
}


void sysMatrixTurn0(unsigned int * pSrc,unsigned int * pDst,unsigned int row64,unsigned int col64)
{
	unsigned int srch,dsth,srcl,dstl,srcaddh,dstaddh;
	unsigned int n;
	unsigned int mode;
	
	srcl = (unsigned int)pSrc;
	dstl = (unsigned int)pDst;
	/*verify src address*/
	n = (srcl>>28)&0xf;
	switch(n)
	{
	case 0xa:
		mode = 0;
		srcaddh = 0x90000000;
		break;
	case 0xc:
	case 0xd:
	case 0xe:
	case 0xf:
		//mode = (DMA_ARCMD|DMA_ARCHE);
		mode = 0;
		srcaddh = 0x90000000;
		break;
	default:
		printstr("sysMatrixTurn0 invalid src address\r\n");
		return;
	}
	/*verify dst address*/
	n = (dstl>>28)&0xf;
	switch(n)
	{
	case 0xa:
		mode |= 0;
		dstaddh = 0x90000000;
		break;
	case 0xc:
	case 0xd:
	case 0xe:
		
	default:
	//	printstr("sysMatrixTurn0: invalid dest address format\r\n");
		mode |= (DMA_AWCMD|DMA_AWCHE);
		dstaddh = 0x98000000;
		//mode = 0;
		//return;
	}
	mode |= (DMA_ENABLE|DMA_START|DMA_TRANS_1BITS) ;//|DMA_DST_INT_VALID);
	srch = 0x98000000;
	dsth = 0x98000000;
	srcl = hrToPhys(srcl);
	dstl = hrToPhys(dstl);


	printf("src address 0x%08x:%08x\n",srch,srcl);
	printf("dst address 0x%08x:%08x\n",dsth,dstl);
	printf("row and col %d*%d\n",row64,col64);
	printf("row and col size %d * %d\n",row64<<3,col64<<3);
	printf("src chip address high %08x\n",srcaddh);
	printf("dst chip address high %08x\n",dstaddh);

	taskDelay(60);
	bsp_printf("1");
	sysXWR64(0x90000000,DMA0_SRC_ADDR,srch,srcl);
	bsp_printf("2");
	sysXWR64(0x90000000,DMA0_DST_ADDR,dsth,dstl);
	bsp_printf("3");
	
	sysXWR32(0x90000000,DMA0_SRC_ROW , row64);
	bsp_printf("4");
	sysXWR32(0x90000000,DMA0_DST_COL , col64);
	bsp_printf("5");
	sysXWR32(0x90000000,DMA0_SRC_LENGTH ,(row64<<3));
	bsp_printf("6");
	sysXWR32(0x90000000,DMA0_DST_LENGTH, (col64<<3));
	bsp_printf("7");
	sysXWR32(0x90000000,DMA0_TRANS_CTRL,mode); /*start dma*/
    bsp_printf("8");
	n = sysXRD32(0x90000000,DMA0_TRANS_STATUS);
	bsp_printf("9");
	
	while((n & DMA_TRANS_FINISHED) == 0)
	{
		sysXNop16();
		n = sysXRD32(0x90000000,DMA0_TRANS_STATUS);
		sysXNop16();
		bsp_printf("9");
		taskDelay(10);
	}
	sysXWR32(0x90000000,DMA0_TRANS_CTRL,(mode & DMA_DISABLE));
}

 void hrMatrixDma0Send(unsigned int raddrh,unsigned int raddrl,
					  unsigned int waddrh,unsigned int waddrl,
					  unsigned int row,unsigned int col,unsigned int size,int turn)
{
	BOOL bRCache ,bWCache;

	unsigned int mode;
	int shift;
 
	if(raddrl == 0) return;
	if(waddrl == 0) return;
	if(row == 0) return;
	if(col == 0) return;
	if((size != 1)&&(size != 2)&&(size != 4)&&(size !=8)) return;

	//starttime0=getcount();
	bRCache = ((raddrh & 0x98000000) == 0x98000000);
	bWCache = ((waddrh & 0x98000000) == 0x98000000);
	mode =0;
	if(bRCache) mode |= DMA_ARCMD|DMA_ARCHE;  /*cacheable address*/
	if(bWCache) mode |= DMA_AWCMD|DMA_AWCHE;  /*cacheable address*/
	mode |= DMA_DST_INT_VALID;
	switch(size)
	{
	case 1:
		mode |= DMA_TRANS_1BITS;
		shift = 0;
		break;
	case 2:
		mode |= DMA_TRANS_2BITS;
		shift = 1;
		break;
	case 4:
		mode |= DMA_TRANS_4BITS;
		shift = 2;
		break;
	default:
		mode |= DMA_TRANS_8BITS;
		shift = 3;
		break;
	}
	
	/*write the 64bit address to reg,need to map the raddrl*/	
	sysXWR64(0x90000000,DMA0_SRC_ADDR,raddrh,raddrl);	
	sysXWR64(0x90000000,DMA0_DST_ADDR,waddrh,waddrl);

   /* config the ROW,COL,SRC_LENGTH,DST_LENGTH*/
	if(turn){
	sysXWR32(0x90000000,DMA0_SRC_ROW , row);
	sysXWR32(0x90000000,DMA0_DST_COL , col);
	sysXWR32(0x90000000,DMA0_SRC_LENGTH ,(row<<shift));
	sysXWR32(0x90000000,DMA0_DST_LENGTH, (col<<shift));
	mode |= DMA_TRANS_TURN;
		  
	}else{
	sysXWR32(0x90000000,DMA0_SRC_ROW , row<<shift);
	sysXWR32(0x90000000,DMA0_DST_COL , col<<shift);
	sysXWR32(0x90000000,DMA0_SRC_LENGTH ,(row<<shift));
	sysXWR32(0x90000000,DMA0_DST_LENGTH, (col<<shift));
	mode &= (~DMA_TRANS_8BITS);/*alwayse 0*/
		    
	}/*turn*/

//	starttime0=getcount();
	sysXWR32(0x90000000,DMA0_TRANS_CTRL,DMA_ENABLE|DMA_START|mode); /*start dma*/
	
//	bsp_printf("wating for sem ... \r\n");
	semTake(semMatrixDma0,WAIT_FOREVER);
//	taskDelay(30);
//	bsp_printf("got sem ... \r\n");
	#if 0
	do
	{
	n = sysXRD32(0x90000000,DMA0_TRANS_STATUS);
	n &= DMA_TRANS_FINISHED;
	taskDelay(1);
	}while(n==0);
	overtime0=getcount();
    #endif
	
//	sysXWR32(0x90000000,DMA0_TRANS_CTRL,0);
    sysXWR32(0x90000000,DMA0_TRANS_CTRL,0);
		
//    if(overtime0 > starttime0)
//	{
//	countnum0 = overtime0-starttime0;
//	testtime0 = countnum0 *2.00000/sb1_clock_rate;
//	printf("transmission time is %f s\r\n",testtime0);
  //  	}
//	else
	//	printf("cal transmission time error!\r\n");
}
 
 int dmaTestCyh(int addrSrc,int addrDst)
 {
	 int n;
	 
	 hrMatrixDma0Send(0x98000000,addrSrc,0x98000000,addrDst,8,8,8,0);
	 while(TRUE)
	 {
		 n=sysXRD32(0x90000000,DMA0_TRANS_STATUS);
		 if(n!=0)
			 break;
	 }
	 sysXWR32(0x90000000,DMA0_TRANS_CTRL,0);
	 printf("dmaTransEnd\n");
	 return 0;
 }
 

/*Matrix Dma 1*/
 void hrMatrixDma1InstInit(struct vxbDev * pDev);
 void hrMatrixDma1InstInit2(struct vxbDev * pDev);
 void hrMatrixDma1InstConnect(struct vxbDev * pDev);
 BOOL hrMatrixDma1Probe(struct vxbDev * pDev);
 static void hrMatrixDma1Int(VXB_DEVICE_ID pDev);

 struct drvBusFuncs hrMatrixDma1Funcs =
{
	hrMatrixDma1InstInit,     /* devInstanceInit */
	hrMatrixDma1InstInit2,    /* devInstanceInit2 */
	hrMatrixDma1InstConnect   /* devConnect */
};
 device_method_t hrMatrixDma1Methods[] ={{0, 0}};
 struct vxbPlbRegister hrMatrixDma1Registration =
{
    {
        NULL,                 /* pNext,一般都用NULL */
        VXB_DEVID_DEVICE,     /* devID,固定,表示这是一个设备 */
        VXB_BUSID_PLB,        /* busID = PLB ,表示这个设备挂接在PLB上*/
        VXBUS_VERSION_4,      /* 版本号,6.7为版本4 */
        HRMATRIXDMA1_VXBNAME,     /*设备名称,必须和hwconfig.c中统一*/
        &hrMatrixDma1Funcs,       		/* pDrvBusFuncs,设备总线函数组在后面定义 */
        hrMatrixDma1Methods,     		/* pMethods ,设备方法组,在后面定义*/
        hrMatrixDma1Probe         		/* devProbe ,设备探测*/
    }
};
 void hrMatrixdma1Register(void){
	vxbDevRegister((struct vxbDevRegInfo *)&(hrMatrixDma1Registration));
   return;
}

 void hrMatrixDma1InstInit (struct vxbDev * pDev)
{
    vxbNextUnitGet(pDev);
    return;
}
 void hrMatrixDma1InstInit2(struct vxbDev * pDev)
{
	HRMATRIXDMA *pDrvCtrl;
	semMatrixDma1 = semBCreate(SEM_Q_PRIORITY,SEM_EMPTY);
    pDrvCtrl = malloc(sizeof(HRMATRIXDMA));
    if (pDrvCtrl == NULL) return;
    bzero ((char *)pDrvCtrl, sizeof(HRMATRIXDMA));
    pDev->pDrvCtrl = pDrvCtrl;
    pDrvCtrl->pDev = pDev;
    return;
}
 void hrMatrixDma1InstConnect(struct vxbDev * pDev)
{
	if(vxbIntConnect(pDev,0,hrMatrixDma1Int,(void *)pDev) != OK)
	{
		printstr("hrMatrixDmaInstConnect: vxbIntConnect Fail\r\n");
	}
	else
	{
		printstr("hrMatrixDmaInstConnect: vxbIntConnect OK\r\n");
	}
	if(vxbIntEnable(pDev,0,hrMatrixDma1Int,(void *)pDev) != OK)
	{
		printstr("hrMatrixDmaInstConnect: vxbIntEnable Fail\r\n");
	}
	else
	{
		printstr("hrMatrixDmaInstConnect: vxbIntEnable OK\r\n");
	}
}
 BOOL hrMatrixDma1Probe(struct vxbDev * pDev){return TRUE;}
static void hrMatrixDma1Int(VXB_DEVICE_ID pDev)
{
	unsigned int n;
//	lockId=intCpuLock();
	printstr("catch dma1 int \r\n");
	n = sysXRD32(0x90000000,DMA1_TRANS_STATUS);
	n &= DMA_TRANS_FINISHED;
	if(n != 0) /*stop dma*/
	{
		overtime1=getcount();
		sysXWR32(0x90000000,DMA1_TRANS_CTRL,0);
		semGive(semMatrixDma1);
		bsp_printf("matrixdma1 int ok!\r\n");
	}
//	intCpuUnlock(lockId);
}
 void hrMatrixDma1Send(unsigned int raddrh,unsigned int raddrl,\
					  unsigned int waddrh,unsigned int waddrl,\
					  unsigned int row,unsigned int col,unsigned int size,int turn)
{
	BOOL bRCache;
	BOOL bWCache;
	unsigned int mode;
	int shift;
	double testtime1;
	if(raddrl == 0) return;
	if(waddrl == 0) return;
	if(row == 0) return;
	if(col == 0) return;
	if((size != 1)&&(size != 2)&&(size != 4)&&(size !=8)) return;

	starttime1=getcount();
	
	bRCache = ((raddrh & 0x98000000) == 0x98000000);
	bWCache = ((waddrh & 0x98000000) == 0x98000000);
	mode =0;
	if(bRCache) mode |= DMA_ARCMD|DMA_ARCHE;  /*cacheable address*/
	if(bWCache) mode |= DMA_AWCMD|DMA_AWCHE;  /*cacheable address*/
	mode |= DMA_DST_INT_VALID;
	switch(size)
	{
	case 1:
		mode |= DMA_TRANS_1BITS;
		shift = 0;
		break;
	case 2:
		mode |= DMA_TRANS_2BITS;
		shift = 1;
		break;
	case 4:
		mode |= DMA_TRANS_4BITS;
		shift = 2;
		break;
	default:
		mode |= DMA_TRANS_8BITS;
		shift = 3;
		break;
	}

	sysXWR64(0x90000000,DMA0_SRC_ADDR,raddrh,raddrl);	
	sysXWR64(0x90000000,DMA0_DST_ADDR,waddrh,waddrl);

	
	if(turn){
	sysXWR32(0x90000000,DMA1_SRC_ROW , row);
	sysXWR32(0x90000000,DMA1_DST_COL , col);
	sysXWR32(0x90000000,DMA1_SRC_LENGTH ,(row<<shift));
	sysXWR32(0x90000000,DMA1_DST_LENGTH, (col<<shift));
	mode |= DMA_TRANS_TURN;
	}else{
	sysXWR32(0x90000000,DMA1_SRC_ROW , row<<shift);
	sysXWR32(0x90000000,DMA1_DST_COL , col<<shift);
	sysXWR32(0x90000000,DMA1_SRC_LENGTH ,(row<<shift));
	sysXWR32(0x90000000,DMA1_DST_LENGTH, (row<<shift));
	mode &= (~DMA_TRANS_8BITS);
	}/*turn*/
	sysXWR32(0x90000000,DMA1_TRANS_CTRL,DMA_ENABLE|DMA_START|mode); /*start dma*/
	semTake(semMatrixDma1,WAIT_FOREVER);

	 if(overtime1 > starttime1)
	{
	countnum1 = overtime1-starttime1;
	testtime1 = countnum1 *2.00000/sb1_clock_rate;
	printf("transmission time is %f s\r\n",testtime1);
    	}
	else
		printf("cal transmission time error!\r\n");
}

void getMatrixDma1()
{
	semTake(semMatrixDma1,WAIT_FOREVER);
}

void getMatrixDma2()
{
	semTake(semMatrixDma0,WAIT_FOREVER);
}



#if 0
#define MAX_TIMESTAMP_COUNT   0x0fffffff  /* use only 28 bits of counter */

extern int sb1_clock_rate;
LOCAL UINT32 microtime(struct timespec *tvp)
{
	

	/* tick num * tick time,unit is ms*/
	tvp->ticknum = (tickGet() *1/sysClkRateGet())*1000;
	/*count reg: 2 sysclk T to add 1,unit is ms*/
	tvp->countnum = ((sysCountGet()& MAX_TIMESTAMP_COUNT) * (2/sb1_clock_rate)) *1000 ; 
	tvp->time = tvp->ticknum + tvp->countnum;
}
#else
IMPORT int	sysCompareGet(void);
IMPORT int	sysCountGet(void);
 unsigned long long getcount( )

{
	/*tickGet() return the ticks, sysCompareGet() return the compare value, sysCountGet() return the count value*/
	unsigned long long a, b,c;
	a =tickGet();
	b = sb1_clock_rate/(2*sysClkRateGet());
	c = b-(sysCompareGet()-sysCountGet());
	/*printf("a = %#x, b = %#x, c = %#x", a, b, c);*/
	return(a * b + c);
}
#endif
unsigned long long timecount0;
unsigned long long timecount1;


 int  testtime()
{

timecount1= getcount();
printf("now the test time is: %ld\r\n",timecount1);
return 0;
}

int task(void)
{
	timecount0= getcount();
	printf("now the test time is: %ld\r\n",timecount0);

    mywatchdogId = wdCreate();
    
	if(NULL == mywatchdogId)
	    return 0;
	
	wdStart(mywatchdogId,sysClkRateGet() * SECONDS,testtime,0);
	return 0;
}


typedef struct
{
	VXB_DEVICE_ID pDev;			 	/* vxbus 要求的项目*/
	unsigned int intSource;			/* intSource */
	unsigned int intLine;			/* intline of core*/
	unsigned int coreNum;			/* core Num */
}HRINT;  /*2 matrix dma use this same struct*/

 device_method_t hrIntMethods[] ={{0, 0}};

/*INTn 0*/
 void hrIntn0InstInit(struct vxbDev * pDev);
 void hrIntn0InstInit2(struct vxbDev * pDev);
 void hrIntn0InstConnect(struct vxbDev * pDev);
 BOOL hrIntn0Probe(struct vxbDev * pDev);
 static void hrIntn0Int(VXB_DEVICE_ID pDev);


 struct drvBusFuncs hrIntn0Funcs =
{
	hrIntn0InstInit,     /* devInstanceInit */
	hrIntn0InstInit2,    /* devInstanceInit2 */
	hrIntn0InstConnect   /* devConnect */
};
 struct vxbPlbRegister hrIntn0Registration =
{
    {
        NULL,                 /* pNext,一般都用NULL */
        VXB_DEVID_DEVICE,     /* devID,固定,表示这是一个设备 */
        VXB_BUSID_PLB,        /* busID = PLB ,表示这个设备挂接在PLB上*/
        VXBUS_VERSION_4,      /* 版本号,6.7为版本4 */
        "hrint0dev",     /*设备名称,必须和hwconfig.c中统一*/
        &hrIntn0Funcs,       		/* pDrvBusFuncs,设备总线函数组在后面定义 */
        hrIntMethods,     		/* pMethods ,设备方法组,在后面定义*/
        hrIntn0Probe ,        		/* devProbe ,设备探测*/
    }
};
 void hrIntn0Register(void){
	vxbDevRegister((struct vxbDevRegInfo *)&(hrIntn0Registration));
   return;
}
 void hrIntn0InstInit (struct vxbDev * pDev)
{
    vxbNextUnitGet(pDev);
    return;
}
 void hrIntn0InstInit2(struct vxbDev * pDev)
{
	HRINT *pDrvCtrl;

    pDrvCtrl = malloc(sizeof(HRINT));
    if (pDrvCtrl == NULL) return;
    bzero ((char *)pDrvCtrl, sizeof(HRINT));
    pDev->pDrvCtrl = pDrvCtrl;
    pDrvCtrl->pDev = pDev;
    return;
}
 void hrIntn0InstConnect(struct vxbDev * pDev)
{
	if(vxbIntConnect(pDev,0,hrIntn0Int,(void *)pDev) != OK)
	{
		printstr("hrIntn0InstConnect: vxbIntConnect Fail\r\n");
	}
	else
	{
		printstr("hrIntn0InstConnect: vxbIntConnect OK\r\n");
	}
	if(vxbIntEnable(pDev,0,hrIntn0Int,(void *)pDev) != OK)
	{
		printstr("hrIntn0InstConnect: vxbIntEnable Fail\r\n");
	}
	else
	{
		printstr("hrIntn0InstConnect: vxbIntEnable OK\r\n");
	}
}

 

 BOOL hrIntn0Probe(struct vxbDev * pDev){return TRUE;}
 
 static void hrIntn0Int(VXB_DEVICE_ID pDev)
{
	int lockId;

	lockId=intCpuLock();
	printstr("Catch Intn 0 int \r\n");
	intCpuUnlock(lockId);
}


/*INTn 1*/
 void hrIntn1InstInit(struct vxbDev * pDev);
 void hrIntn1InstInit2(struct vxbDev * pDev);
 void hrIntn1InstConnect(struct vxbDev * pDev);
 BOOL hrIntn1Probe(struct vxbDev * pDev);
 static void hrIntn1Int(VXB_DEVICE_ID pDev);


 struct drvBusFuncs hrIntn1Funcs =
{
	hrIntn1InstInit,     /* devInstanceInit */
	hrIntn1InstInit2,    /* devInstanceInit2 */
	hrIntn1InstConnect   /* devConnect */
};
 struct vxbPlbRegister hrIntn1Registration =
{
    {
        NULL,                 /* pNext,一般都用NULL */
        VXB_DEVID_DEVICE,     /* devID,固定,表示这是一个设备 */
        VXB_BUSID_PLB,        /* busID = PLB ,表示这个设备挂接在PLB上*/
        VXBUS_VERSION_4,      /* 版本号,6.7为版本4 */
        "hrint1dev",     /*设备名称,必须和hwconfig.c中统一*/
        &hrIntn1Funcs,       		/* pDrvBusFuncs,设备总线函数组在后面定义 */
        hrIntMethods,     		/* pMethods ,设备方法组,在后面定义*/
        NULL ,        		/* devProbe ,设备探测*/
    }
};
 void hrIntn1Register(void){
	vxbDevRegister((struct vxbDevRegInfo *)&(hrIntn1Registration));
   return;
}
 void hrIntn1InstInit (struct vxbDev * pDev)
{
    const struct hcfDevice * pHcf;

    /* always use the unit number allocated in the hwconf.c file */
    pHcf = hcfDeviceGet(pDev);
    vxbInstUnitSet(pDev, pHcf->devUnit);
    return;
}
 void hrIntn1InstInit2(struct vxbDev * pDev)
{
	HRINT *pDrvCtrl;

    pDrvCtrl = malloc(sizeof(HRINT));
    if (pDrvCtrl == NULL) return;
    bzero ((char *)pDrvCtrl, sizeof(HRINT));
    pDev->pDrvCtrl = pDrvCtrl;
    pDrvCtrl->pDev = pDev;
    return;
}
 void hrIntn1InstConnect(struct vxbDev * pDev)
{
	if(vxbIntConnect(pDev,0,hrIntn1Int,(void *)pDev) != OK)
	{
		printstr("hrIntn1InstConnect: vxbIntConnect Fail\r\n");
	}
	else
	{
		printstr("hrIntn1InstConnect: vxbIntConnect OK\r\n");
	}
	if(vxbIntEnable(pDev,0,hrIntn1Int,(void *)pDev) != OK)
	{
		printstr("hrIntn1InstConnect: vxbIntEnable Fail\r\n");
	}
	else
	{
		printstr("hrIntn1InstConnect: vxbIntEnable OK\r\n");
	}
}

 

 BOOL hrIntn1Probe(struct vxbDev * pDev){return TRUE;}
 
 static void hrIntn1Int(VXB_DEVICE_ID pDev)
{
	int lockId;

	lockId=intCpuLock();
	printstr("Catch Intn 1 int \r\n");
	intCpuUnlock(lockId);
}


/*INTn 0*/
 void hrIntn2InstInit(struct vxbDev * pDev);
 void hrIntn2InstInit2(struct vxbDev * pDev);
 void hrIntn2InstConnect(struct vxbDev * pDev);
 BOOL hrIntn2Probe(struct vxbDev * pDev);
 static void hrIntn2Int(VXB_DEVICE_ID pDev);


 struct drvBusFuncs hrIntn2Funcs =
{
	hrIntn2InstInit,     /* devInstanceInit */
	hrIntn2InstInit2,    /* devInstanceInit2 */
	hrIntn2InstConnect   /* devConnect */
};
 struct vxbPlbRegister hrIntn2Registration =
{
    {
        NULL,                 /* pNext,一般都用NULL */
        VXB_DEVID_DEVICE,     /* devID,固定,表示这是一个设备 */
        VXB_BUSID_PLB,        /* busID = PLB ,表示这个设备挂接在PLB上*/
        VXBUS_VERSION_4,      /* 版本号,6.7为版本4 */
        "hrint2dev",     /*设备名称,必须和hwconfig.c中统一*/
        &hrIntn2Funcs,       		/* pDrvBusFuncs,设备总线函数组在后面定义 */
        hrIntMethods,     		/* pMethods ,设备方法组,在后面定义*/
        hrIntn2Probe  ,       		/* devProbe ,设备探测*/
    }
};
 void hrIntn2Register(void){
	vxbDevRegister((struct vxbDevRegInfo *)&(hrIntn2Registration));
   return;
}
 void hrIntn2InstInit (struct vxbDev * pDev)
{
    vxbNextUnitGet(pDev);
    return;
}
 void hrIntn2InstInit2(struct vxbDev * pDev)
{
	HRINT *pDrvCtrl;

    pDrvCtrl = malloc(sizeof(HRINT));
    if (pDrvCtrl == NULL) return;
    bzero ((char *)pDrvCtrl, sizeof(HRINT));
    pDev->pDrvCtrl = pDrvCtrl;
    pDrvCtrl->pDev = pDev;
    return;
}
 void hrIntn2InstConnect(struct vxbDev * pDev)
{
	if(vxbIntConnect(pDev,0,hrIntn2Int,(void *)pDev) != OK)
	{
		printstr("hrIntn2InstConnect: vxbIntConnect Fail\r\n");
	}
	else
	{
		printstr("hrIntn2InstConnect: vxbIntConnect OK\r\n");
	}
	if(vxbIntEnable(pDev,0,hrIntn2Int,(void *)pDev) != OK)
	{
		printstr("hrIntn2InstConnect: vxbIntEnable Fail\r\n");
	}
	else
	{
		printstr("hrIntn2InstConnect: vxbIntEnable OK\r\n");
	}
}

 

 BOOL hrIntn2Probe(struct vxbDev * pDev){return TRUE;}
 
 static void hrIntn2Int(VXB_DEVICE_ID pDev)
{
	int lockId;

	lockId=intCpuLock();
	printstr("Catch Intn 2 int \r\n");
	intCpuUnlock(lockId);
}


/*INTn 0*/
 void hrIntn3InstInit(struct vxbDev * pDev);
 void hrIntn3InstInit2(struct vxbDev * pDev);
 void hrIntn3InstConnect(struct vxbDev * pDev);
 BOOL hrIntn3Probe(struct vxbDev * pDev);
 static void hrIntn3Int(VXB_DEVICE_ID pDev);


 struct drvBusFuncs hrIntn3Funcs =
{
	hrIntn3InstInit,     /* devInstanceInit */
	hrIntn3InstInit2,    /* devInstanceInit2 */
	hrIntn3InstConnect   /* devConnect */
};
 struct vxbPlbRegister hrIntn3Registration =
{
    {
        NULL,                 /* pNext,一般都用NULL */
        VXB_DEVID_DEVICE,     /* devID,固定,表示这是一个设备 */
        VXB_BUSID_PLB,        /* busID = PLB ,表示这个设备挂接在PLB上*/
        VXBUS_VERSION_4,      /* 版本号,6.7为版本4 */
        "hrint3dev",     /*设备名称,必须和hwconfig.c中统一*/
        &hrIntn3Funcs,       		/* pDrvBusFuncs,设备总线函数组在后面定义 */
        hrIntMethods,     		/* pMethods ,设备方法组,在后面定义*/
        hrIntn3Probe,         		/* devProbe ,设备探测*/
    }
};
 void hrIntn3Register(void){
	vxbDevRegister((struct vxbDevRegInfo *)&(hrIntn3Registration));
   return;
}
 void hrIntn3InstInit (struct vxbDev * pDev)
{
    vxbNextUnitGet(pDev);
    return;
}
 void hrIntn3InstInit2(struct vxbDev * pDev)
{
	HRINT *pDrvCtrl;

    pDrvCtrl = malloc(sizeof(HRINT));
    if (pDrvCtrl == NULL) return;
    bzero ((char *)pDrvCtrl, sizeof(HRINT));
    pDev->pDrvCtrl = pDrvCtrl;
    pDrvCtrl->pDev = pDev;
    return;
}
 void hrIntn3InstConnect(struct vxbDev * pDev)
{
	if(vxbIntConnect(pDev,0,hrIntn3Int,(void *)pDev) != OK)
	{
		printstr("hrIntn3InstConnect: vxbIntConnect Fail\r\n");
	}
	else
	{
		printstr("hrIntn3InstConnect: vxbIntConnect OK\r\n");
	}

	#if 1
	if(vxbIntEnable(pDev,0,hrIntn3Int,(void *)pDev) != OK)
	{
		printstr("hrIntn3InstConnect: vxbIntEnable Fail\r\n");
	}
	else
	{
		printstr("hrIntn3InstConnect: vxbIntEnable OK\r\n");
	}
	#endif
}


 

 BOOL hrIntn3Probe(struct vxbDev * pDev){return TRUE;}
 
 static void hrIntn3Int(VXB_DEVICE_ID pDev)
{
	int lockId;

	lockId=intCpuLock();
	printstr("Catch Intn 3 int \r\n");
	intCpuUnlock(lockId);
}

