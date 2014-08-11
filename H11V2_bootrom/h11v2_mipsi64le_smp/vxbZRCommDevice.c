/******************************************************************
 * vxbZRCommDevice.c 
 * 20100926 Add Chip Dma access
 * 20100830 Create this file first ---wangzx
 */

/******** includes *****************************/
#include "config.h" 

#include <string.h>
#include <cacheLib.h>
#include <stdlib.h>
#include <stdio.h>
#include <intLib.h>
#include <logLib.h>
#include <taskLib.h>
#include <sysLib.h>
/*added for vxBus*/
#include <hwif/vxbus/vxBus.h>
#include <hwif/vxbus/vxbPlbLib.h>
#include <vxBusLib.h>
#include <hwif/vxbus/hwConf.h>
#include <net/utils/netstat.h>
#include <vxWorks.h>
#include "vxbZRCommDevice.h"
#include "simpleprintf.h"


#include "bsl.h"
#include "ZRRapidIO.h"

#define USE_DMA_LOCAL_MEM_RING
#undef INCLUDE_HT1_DMA_ENG
#undef CHIP_TRANS_USE_MT


unsigned int g_nIPISendCount = 0;
unsigned int g_nIPIRecvCount = 0;
static int s_nMC0WriteP = 0;
static int s_nMC1WriteP = 0;
static void * s_pLockSendMC0;
static void * s_pLockSendMC1;
static char * s_pXChipMC0SendBuf = 0;
static char * s_pXChipMC1SendBuf = 0;
IMPORT void zrAccessDmaInInt2();

/*static int sysHTDmaSend(void *srcbuf, void *dstbuf, unsigned int dmasize, unsigned short srcnode,unsigned short dstnode);
static void sysHTDmaIntClear();*/

unsigned int LOCAL_NODE_ID = 0;

#include "zrIpi_Local.c"
#ifdef CHIP_TRANS_USE_MT
#include "zrHT_copy.c"
#else
#include "zrHT_copy_v.c"
#endif

/*add by wangzx 20111116*/
#include "zrMatrixDma.c"

/*�豸���� data struct*/
typedef struct
{
	VXB_DEVICE_ID pDev;			 	/* vxbus Ҫ�����Ŀ*/
	unsigned int intSource;			/* intSource */
	unsigned int intLine;			/* intline of core*/
	unsigned int coreNum;			/* core Num */
} ZRCOMMDEVICE;


LOCAL void zrCommDeviceInstInit (struct vxbDev * pDev);
LOCAL void zrCommDeviceInstInit2(struct vxbDev * pDev);
LOCAL void zrCommDeviceInstConnect(struct vxbDev * pDev);
/*�������� (��)*/
/*̽Ѱ����*/
LOCAL BOOL zrCommDeviceProbe(struct vxbDev * pDev);
/*�ж���Ӧ����*/
LOCAL void zrCommDeviceInt(VXB_DEVICE_ID pDev);

/*vxBus ע�����߼��غ���*/
LOCAL struct drvBusFuncs zrCommDeviceFuncs =
{
    zrCommDeviceInstInit,     /* devInstanceInit */
    zrCommDeviceInstInit2,    /* devInstanceInit2 */
    zrCommDeviceInstConnect   /* devConnect */
};
/*vxBus �豸��������*/
LOCAL device_method_t zrCommDeviceMethods[] =
{
	{0, 0}
};
/*vxBus �豸ע�����*/
LOCAL struct vxbPlbRegister zrCommDeviceRegistration =
{
    {
        NULL,                 /* pNext,һ�㶼��NULL */
        VXB_DEVID_DEVICE,     /* devID,�̶�,��ʾ����һ���豸 */
        VXB_BUSID_PLB,        /* busID = PLB ,��ʾ����豸�ҽ���PLB��*/
        VXBUS_VERSION_4,      /* �汾��,6.7Ϊ�汾4 */
        ZRCOMMDEVICE_CARD_VXBNAME,     /*�豸����,�����hwconfig.c��ͳһ*/
        &zrCommDeviceFuncs,       		/* pDrvBusFuncs,�豸���ߺ������ں��涨�� */
        zrCommDeviceMethods,     		/* pMethods ,�豸������,�ں��涨��*/
        zrCommDeviceProbe         		/* devProbe ,�豸̽��*/
    }
};

/**************************************�����***********************************/
void zrCommDeviceRegister(void)
{ 
	vxbDevRegister((struct vxbDevRegInfo *)&(zrCommDeviceRegistration));
   return;
}


/*************************************���߽ӿ�************************************/
LOCAL void zrCommDeviceInstInit (struct vxbDev * pDev)
{
    vxbNextUnitGet(pDev);
    return;
}

LOCAL void zrCommDeviceInstInit2(struct vxbDev * pDev)
{

	ZRCOMMDEVICE *pDrvCtrl;
	PART_ID  idMC0;
	PART_ID  idMC1;
    pDrvCtrl = malloc(sizeof(ZRCOMMDEVICE));
    if (pDrvCtrl == NULL) return;
    bzero ((char *)pDrvCtrl, sizeof(ZRCOMMDEVICE));
    pDev->pDrvCtrl = pDrvCtrl;
    pDrvCtrl->pDev = pDev;
/*�������Ӳ��Ҫ��Ĵ���*/
    LOCAL_NODE_ID = getNodeZR()&0x3;
/*add by wangzx 20110920*/
/*step 1: malloc xchip send buf,application use this to store calc result*/
    idMC0 = (PART_ID)NULL;
    idMC1 = (PART_ID)NULL;
    
    idMC0 = memPartCreate((char *)MC0_PART_BASE_ADDR,MC0_PART_SIZE);
    idMC1 = memPartCreate((char *)MC1_PART_BASE_ADDR,MC1_PART_SIZE);
    if((idMC0 == NULL)||(idMC1 == NULL))
    {
    	printstr("zrCommDeviceInstInit2::extend memory part create error\r\n");
    	return;    	 
    }
    s_pXChipMC0SendBuf = memPartAlignedAlloc(idMC0,MC0_XCHIP_SENDBUF_SIZE,XCHIP_SENDBUF_ALIGN);
    s_pXChipMC1SendBuf = memPartAlignedAlloc(idMC1,MC1_XCHIP_SENDBUF_SIZE,XCHIP_SENDBUF_ALIGN);    
    if((s_pXChipMC0SendBuf == NULL)||(s_pXChipMC1SendBuf == NULL))
    {
    	printstr("zrCommDeviceInstInit2::x-chip send buf alloc failed\r\n");
    	return;     
    }
 /*step 2: create mc0 sendbuf block descript*/
    s_nMC0WriteP = 0;
    s_nMC1WriteP = 0;
 /*step 3: create spinlock*/
    s_pLockSendMC0 = SpinLockInit();
    s_pLockSendMC1 = SpinLockInit();
    if((s_pLockSendMC0 == NULL)||(s_pLockSendMC1 == NULL))
    {
    	printstr("zrCommDeviceInstInit2:: spinlock create failed\r\m");
    	return;    
    }
 /*end*/    
    zrIpiRingBufferInit(); /*defined in zrIpi_Local.c*/
    zrDmaInit2();/*defined zrHT_copy.c*/
    return;
}
LOCAL void zrCommDeviceInstConnect(struct vxbDev * pDev)
{
	/*add by wangzx 20100828*/
	unsigned int data;
	unsigned int val;
	int interruptIndex;
	int cpuId;

	cpuId = bslProcGetId()&0x3;

	printf("Clear All Isr\n");
	FPGA_WRITE(FPGA_IntrIrq,0);
	data = FPGA_READ(FPGA_IntrIrq);
	printf("Init FPGA Irq is 0x%x\n",data);
	
	printf("disable FPGA interrupt\n");
	data = FPGA_READ(FPGA_IntrMask);
	data &= ~(0xff<<(cpuId*8));
	printf("enable FPGA interrupt\n");
	val =0xff<<(cpuId*8);
	data |= val;
    FPGA_WRITE(FPGA_IntrMask,data);
    val =FPGA_READ(FPGA_IntrMask);
    printf("Intr Mask is %x\n",val);

    interruptIndex = 0;

    if(vxbIntConnect(pDev,0,zrCommDeviceInt,(void *)pDev) != OK)
    {
    	printf("vxbIntConnect Fail\n");
    }
    else
    {
    	printf("vxbIntConnect OK\n");
    }
   
    if(vxbIntEnable(pDev,0,zrCommDeviceInt,(void *)pDev) != OK)
    {
    	printf("vxbIntEnable Fail\n");
    }
    else
    {
    	printf("vxbIntEnable OK\n");
    }

    /*add by wangzx 20100828 end*/
}
LOCAL BOOL zrCommDeviceProbe(struct vxbDev * pDev)
{
	return TRUE;
}

/*HT1 DMA MEMORY SPACE,ADDED BY LW*/
#define HT_DMA_SRCADDR_REG     0
#define HT_DMA_DESTADDR_REG    8
#define HT_DMA_START_REG       0x10
#define HT_DMA_LENGTH_REG      0x18   /*in byte*/
#define HT_DMA_INTR_ENABLE_REG 0x20
#define HT_DMA_INTR_CFG_REG    0x28   /*read only*/
#define HT_DMA_INTR_CLR_REG    0x40



#if INCLUDE_HT1_DMA_ENG
static void sysHTDmaIntClear()
{
	sysXWR32(0x90000e00,HT_DMA_INTR_CLR_REG,0);
}
#endif
static void zrCommDeviceInt(VXB_DEVICE_ID pDev)
{
	int lockId;
	unsigned short cpuId;
	unsigned int flag;
	int intr_type=0; //added by lw

	lockId=intCpuLock();
	cpuId = bslProcGetId()&0x3; 
#ifdef INCLUDE_HT1_DMA_ENG   
	intr_type = sysFpgaRD32(0,HT_DMA_INTR_CFG_REG);
/*	intr_type =RIO_READ32(HT_DMA_INTR_CFG_REG,0);*/
	if(intr_type)
	{
		sysHTDmaIntClear();
		zrAccessDmaInInt2(); 
	} /*add by lw for ht1 dma*/
	else /*ipi interrupt*/
#endif
	{
		/*clear my interrupt*/
		flag = FPGA_READ(FPGA_IntrStatus);
		flag >>= (cpuId * 8);
		flag &= 0xff;
		flag <<= (cpuId * 8);
		FPGA_WRITE(FPGA_IntrStatus,flag);
		zrDispatchIPI();
		g_nIPIRecvCount ++;
	}
	intCpuUnlock(lockId);
}
/*add by wangzx, 20110920*/
void * AllocMC0SendBuf(int len8)
{
	unsigned int len;
	unsigned int wptr;
	if(len8 < 0) return NULL;
	if(len8 >= MC0_XCHIP_SENDBUF_SIZE) return NULL;
	len = bslAlign(XCHIP_SENDBUF_ALIGN,len8);
	SpinLockTake(s_pLockSendMC0);
	wptr = s_nMC0WriteP;
	if((wptr +len) >MC0_XCHIP_SENDBUF_SIZE)
	{
		wptr =0;
		s_nMC0WriteP = len;
	}
	else
	{
		s_nMC0WriteP = wptr + len;
	}
	SpinLockGive(s_pLockSendMC0);
	return (void *)(s_pXChipMC0SendBuf + wptr);
}
void * AllocMC1SendBuf(int len8)
{
	unsigned int len;
	unsigned int wptr;
	if(len8 < 0) return NULL;
	if(len8 >= MC1_XCHIP_SENDBUF_SIZE) return NULL;
	len = bslAlign(XCHIP_SENDBUF_ALIGN,len8);
	SpinLockTake(s_pLockSendMC1);
	wptr = s_nMC1WriteP;
	if((wptr +len) >MC1_XCHIP_SENDBUF_SIZE)
	{
		wptr =0;
		s_nMC1WriteP = len;
	}
	else
	{
		s_nMC1WriteP = wptr + len;
	}
	SpinLockGive(s_pLockSendMC1);
	return (void *)(s_pXChipMC1SendBuf + wptr);
}
/*end*/
