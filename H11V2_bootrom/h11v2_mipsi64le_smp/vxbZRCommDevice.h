/*************************************************************
 * vxbZRCommDevice.h device for ZhongRui 4Chip Board communication
 * every chip each other
 * 
 * 20100830 create this file first --wangzx
 */

#ifndef vxbZRCommDevice_h_20100830
#define vxbZRCommDevice_h_20100830
#include "zrBsp.h"
/*wangzx 20111116*/
#include "zrMatrixDma.h"
/*add by wangzx 20110920 add extend memory space*/
 
#ifdef MEMCONTROL_2CH
#define MC0_PART_SIZE  (128*M)
#define MC1_PART_SIZE  (128*M)
#define MC0_PART_BASE_ADDR (MC0_BASE_ADDR + RIO_RECV_LEVEL*2*M)
#define MC1_PART_BASE_ADDR (MC1_BASE_ADDR + RIO_RECV_LEVEL*2*M)
#define MC0_XCHIP_SENDBUF_SIZE (100*M)
#define MC1_XCHIP_SENDBUF_SIZE (100*M)
#else
#define MC0_PART_SIZE  (64*M)
#define MC1_PART_SIZE  (64*M)
#define MC0_PART_BASE_ADDR (MC0_BASE_ADDR + RIO_RECV_LEVEL*2*M)
#define MC1_PART_BASE_ADDR (MC0_BASE_ADDR + RIO_RECV_LEVEL*2*M + MC0_PART_SIZE)
#define MC0_XCHIP_SENDBUF_SIZE (50*M)
#define MC1_XCHIP_SENDBUF_SIZE (50*M)
#endif
#define XCHIP_SENDBUF_ALIGN (0x1000)  /*4K Byte align*/
#define FPGA_INT_PIN 1
#define FPGA_INT_CORE_PIN 1
#define FPGA_INT_CORE_NUM 1 

#ifndef BOOTROM
#define ZRCOMMDEVICE_CARD_DEVICE
#endif

#define ZRCOMMDEVICE_CARD_VXBNAME "zrcommdevicedev"

/* these line copy to hw_conf.c
LOCAL const struct hcfResource zrCommdevcieResources[] = { 
    { "regBase", HCF_RES_INT, { (void *)(0)} },
};
#define zrCommdeviceNum NELEMENTS(zrCommdevcieResources)
*/

#define ZRCOMMDEVICE_CARD_DEVICE_DESC \
	{ ZRCOMMDEVICE_CARD_VXBNAME, 0, VXB_BUSID_PLB, 0, zrCommdeviceNum, zrCommdevcieResources}

#define ZRCOMMDEVICE_CARD_INT_DESC \
	{FPGA_INT_PIN,ZRCOMMDEVICE_CARD_VXBNAME,0,0}

#define ZRCOMMDEVICE_CARD_XBAR_DESC \
	{FPGA_INT_PIN,FPGA_INT_CORE_PIN} 

#define ZRCOMMDEVICE_CARD_CPU_ROUTE_DESC \
	{FPGA_INT_PIN, FPGA_INT_CORE_NUM}

#define IPI_RingBufferSize 32

int zrIpiPushMessage(unsigned int dstId,unsigned int message);
int zrIpiPopMessage(unsigned int * srcId,unsigned int * message);

/*add by wangzx : Create 2 Channel Memory Ctrl BD Ring Buffer, 
 * 
 * Each Channel 128MByte,We alloc 376M memory every MC channel,Send BD alloc 100MByte by user,
 * RecvBD alloc 100MX2,other we use it to do manage,No xChannel used!!*/

int zrForwardNodeID();
int zrBackwardNodeID();
int zrDmaSetSendMode(int mode);
void zrDmaWaitData(int nCh);
void zrSendDmaHT2(int peer,int ch/*0 to 4*/,unsigned int * pSrc,unsigned int len32);
BOOL zrGetDmaData(int ch,unsigned int * from,unsigned int * nLen32,unsigned int **pBuf);

void * AllocMC0SendBuf(int len8);
void * AllocMC1SendBuf(int len8);
#endif/*vxbZRCommDevice_h_20100830*/
