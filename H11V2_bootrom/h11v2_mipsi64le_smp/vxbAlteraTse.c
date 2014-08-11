#include <stdio.h>
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
#include <wdLib.h>
#include <etherMultiLib.h>
#include <end.h>

#define END_MACROS
#include <endLib.h>

#include <miiLib.h>

#include <hwif/vxbus/vxBus.h>
#include <hwif/vxbus/hwConf.h>
#include <hwif/vxbus/vxbPlbLib.h>
#include <hwif/util/vxbParamSys.h>
#include <../src/hwif/h/vxbus/vxbAccess.h>
#include <../src/hwif/h/mii/miiBus.h>
#include <../src/hwif/h/hEnd/hEnd.h>

#include "simpleprintf.h"

#include "vxbAlteraTse.h"

#define ALT_DEBUG      


typedef struct
{
    END_OBJ     endObj;             /*vxWorks指定其为第一项*/
    VXB_DEVICE_ID pDev;             /* vxbus 要求的项目*/

    void *  muxDevCookie;           /*小段信息,是由muxDevLoad带来的,需要给muxDevStart/Stop使用*/
    unsigned char *  macAddrPtr;             /*mac地址,这个地址应该从EEROM中读取,很多时候是直接写下去的mib2Init函数需要用*/

    int     nVxUnit;                /*设备序号,0,1,2,3…EndObjInit函数要用*/

    SEM_ID  atseDevSem;              /*配置锁,在start,stop,ioctl,restart和unload函数保证不冲突*/
    SEM_ID  semRecv;                /*接收锁,不一定要用*/
    SEM_ID  semSend;                /*发送锁,一定要用*/

    UINT32  pRecvBufferAddr;        /*接收缓存区netPoolInit函数间接调用 */
    UINT32  bRecvBufAlloced;        /*接收缓存区被分配的标志*/

    END_ERR VxEndLastError;         /*最后一次出错信息的结构,可以被muxError调用*/
    CACHE_FUNCS cacheFuncs;         /*cache 的函数组,如果CPU不能保证cache一致性,需要通过这个函数组来调用cache相关的操作*/
  
    struct 
    {
        NET_POOL    NetPool;        /**< VxWorks Rx Network Pool */
        M_CL_CONFIG ClConfig;       /**< VxWorks Rx Cluster Config */
        unsigned    ClSize;         /**< Rx Cluster size */
        int         BufCnt;         /**< Total number of Rx buffers allocated */
    } RxBuf;                        /*接收缓存,netPoolInit调用其中的NetPool和ClConfig等*/
    
    struct {
        unsigned char  *BaseAddr;   /**< 1st Tx buffer address */
        unsigned char  *HighAddr;   /**< Last Tx buffer address */
        unsigned char  *NextBuf;    /**< Address of next available Tx buffer */
        unsigned int  BufSize;      /**< Tx buffer size */
        int      BufsAvailable;     /**< Number of Tx buffers left */
        int      BufCnt;            /**< Total number of Tx buffers allocated */
    } TxBuf;
    
    UINT32    atseMacMaxMtu;        /*最大包长,间接被NetPool调用*/
    INT32     nSpeed;               /*连接速度,10/100/1000M*/
    UINT32    nIntID;               /*中断向量 */
    ATSEMAC   macObj;               /*本地驱动对象*/
    
    /*下面添加和硬件直接相关的部分*/
    np_tse_mac *  tseMac;            /* mac芯片register地址指针 */

    void * pSgdma0TxReg;
    void * pSgdma1RxReg;
  
    ATSE_SGDMA_DESCRIPTOR * pDMASendDesc;    /*DMA发送缓冲区,原来的BD*/
    ATSE_SGDMA_DESCRIPTOR * pDMARecvDesc;    /*DMA接收缓冲区,原来的BD*/

    JOB_QUEUE_ID     emacJobQueue;      /* 网卡任务处理队列 ID */

    QJOB             emacIntJob;
    BOOL    emacIntPending;
    QJOB             emacRxJob;
    BOOL    emacRxPending;
    QJOB             emacTxJob;
    BOOL    emacTxPending;

    M_BLK_ID         emacRxMblk[ALTERA_SGDMA_RX_RING_NUM];
    M_BLK_ID         emacTxMblk[ALTERA_SGDMA_TX_RING_NUM];

    BOOL             emacPolling;

    M_BLK_ID         emacPollBuf;
    END_CAPABILITIES emacCaps;
    END_MEDIALIST *  emacMediaList;
    VXB_DEVICE_ID    emacMiiBus;

    END_ERR          emacLastError;
    END_IFDRVCONF    emacEndStatsConf;
    END_IFCOUNTERS   emacEndStatsCounters;
    UINT32           emacCurMedia;
    UINT32           emacCurStatus; 

    volatile BOOL    emacTxStall;
    volatile BOOL    emacRxStall;
    
    UINT32           emacTxProd;
    UINT32           emacTxCons;
    UINT32           emacTxFree;
/*    int              emacTxQCount;  */
    UINT32           emacTxPend;
    UINT32           emacTxCnt;
    
    UINT32           emacRxIdx;

    
} ATSEMACEND;


#define SGDMA_0_BASE   0xb8001440
#define SGDMA_1_BASE   0xb8001400

#define ALTERA_TSE_SGDMA_BUSY_TIME_OUT_CNT      1000000 


#define ATSEMAC_PROMISC_OPTION               0x00000001
#define ATSEMAC_JUMBO_OPTION                 0x00000002
#define ATSEMAC_VLAN_OPTION                  0x00000004
#define ATSEMAC_FLOW_CONTROL_OPTION          0x00000008
#define ATSEMAC_FCS_STRIP_OPTION             0x00000010
#define ATSEMAC_FCS_INSERT_OPTION            0x00000020
#define ATSEMAC_LENTYPE_ERR_OPTION           0x00000040
#define ATSEMAC_TRANSMITTER_ENABLE_OPTION    0x00000080
#define ATSEMAC_RECEIVER_ENABLE_OPTION       0x00000100
#define ATSEMAC_BROADCAST_OPTION             0x00000200
#define ATSEMAC_MULTICAST_OPTION             0x00000400
#define ATSEMAC_DEFAULT_OPTIONS                     \
                                (ATSEMAC_FLOW_CONTROL_OPTION |                  \
                                 ATSEMAC_BROADCAST_OPTION |                     \
                                 ATSEMAC_FCS_INSERT_OPTION |                    \
                                 ATSEMAC_FCS_STRIP_OPTION |                     \
                                 ATSEMAC_LENTYPE_ERR_OPTION |                   \
                                 ATSEMAC_TRANSMITTER_ENABLE_OPTION |            \
                                 ATSEMAC_RECEIVER_ENABLE_OPTION )

     

/*MAC设备*/


static unsigned char atsemacMacAddrDft0[6] = { 0x00, 0x0A, 0x35, 0x01, 0x02, 0x03 };
static unsigned char atsemacMacAddrDft1[6] = { 0x00, 0x0A, 0x35, 0x01, 0x02, 0x04 };


LOCAL void atsemacInstInit (struct vxbDev * pDev);
LOCAL void atsemacInstInit2(struct vxbDev * pDev);
LOCAL void atsemacInstConnect(struct vxbDev * pDev);


LOCAL STATUS atsemacInstUnlink (struct vxbDev * pDev, void *pArg);
LOCAL STATUS atsemacPhyRead(VXB_DEVICE_ID pDev, UINT8 phyAddr, UINT8 regAddr, UINT16 *dataVal);
LOCAL STATUS atsemacPhyWrite(VXB_DEVICE_ID pDev, UINT8 phyAddr, UINT8 regAddr, UINT16 dataVal);
LOCAL STATUS atsemacLinkUpdate( VXB_DEVICE_ID pDev );


STATUS   sysMacMemoryInit(ATSEMACEND * pDrvCtrl);




END_OBJ *atsemacEndLoad (  char * ,  void *   );
LOCAL STATUS atsemacEndStart(END_OBJ *DriverPtr);
LOCAL STATUS atsemacEndStop(END_OBJ *DriverPtr);
LOCAL STATUS atsemacEndUnload(END_OBJ *DriverPtr);
LOCAL STATUS atsemacEndIoctl(END_OBJ *DriverPtr, int Cmd, caddr_t Data);
LOCAL STATUS atsemacEndSend(END_OBJ *DriverPtr, M_BLK_ID MblkPtr);
LOCAL void   atsemacEndInt(ATSEMACEND *);
LOCAL void   atsemacEndRxHandle(void *);
LOCAL void   atsemacEndTxHandle(void *);
LOCAL void   atsemacEndIntHandle(void *);

 
LOCAL STATUS atsemacEndPollSend(END_OBJ *DriverPtr, M_BLK_ID MblkPtr);
LOCAL STATUS atsemacEndPollRecv(END_OBJ *DriverPtr, M_BLK_ID MblkPtr);
LOCAL STATUS atsemacEndMCastAddrGet(END_OBJ *EndPtr, MULTI_TABLE *MultiTablePtr);
LOCAL STATUS atsemacEndMCastAddrAdd(END_OBJ *EndPtr, char *AddressPtr);
LOCAL STATUS atsemacEndMCastAddrDel(END_OBJ *EndPtr, char *AddressPtr);
/* LOCAL int    atsemacEndMCastAddrFind(END_OBJ *DriverPtr, char *AddressPtr); */

IMPORT void cacheLsn2eSync(void *vAddr, UINT pageSize);   /* flush */
IMPORT void cacheLsn2eSync2 (void *vAddr, UINT pageSize); /* invalidate */

void sysSgdmaInit (void *base);


void atsemacMuxConnect(struct vxbDev * pDev, void * unused);

LOCAL NET_FUNCS atsemacENDFuncs =
{
    (FUNCPTR)atsemacEndStart,
    (FUNCPTR)atsemacEndStop,
    (FUNCPTR)atsemacEndUnload,
    (FUNCPTR)atsemacEndIoctl,
    (FUNCPTR)atsemacEndSend,
    (FUNCPTR)atsemacEndMCastAddrAdd,
    (FUNCPTR)atsemacEndMCastAddrDel,
    (FUNCPTR)atsemacEndMCastAddrGet,
    (FUNCPTR)atsemacEndPollSend,
    (FUNCPTR)atsemacEndPollRecv,
    endEtherAddressForm,
    endEtherPacketDataGet,
    endEtherPacketAddrGet
};

LOCAL struct drvBusFuncs atsemacFuncs =
{
    atsemacInstInit,     /* devInstanceInit */
    atsemacInstInit2,    /* devInstanceInit2 */
    atsemacInstConnect   /* devConnect */
};

LOCAL device_method_t atsemacMethods[] =
{
    DEVMETHOD(miiRead,           atsemacPhyRead),
    DEVMETHOD(miiWrite,          atsemacPhyWrite),
    DEVMETHOD(miiMediaUpdate,    atsemacLinkUpdate),

    DEVMETHOD(muxDevConnect,     atsemacMuxConnect),
    DEVMETHOD(vxbDrvUnlink,      atsemacInstUnlink),
    {0, 0}
};

/* default queue parameters */

LOCAL HEND_RX_QUEUE_PARAM atsemacRxQueueDefault =
{
    NULL,                       /* jobQueId */
    0,                          /* priority */
    0,                          /* rbdNum */
    0,                          /* rbdTupleRatio */
    0,                          /* rxBufSize */
    NULL,                       /* pBufMemBase */
    0,                          /* rxBufMemSize */
    0,                          /* rxBufMemAttributes */
    NULL,                       /* rxBufMemFreeMethod */
    NULL,                       /* pRxBdBase */
    0,                          /* rxBdMemSize */
    0,                          /* rxBdMemAttributes */
    NULL                        /* rxBdMemFreeMethod */
};

LOCAL HEND_TX_QUEUE_PARAM atsemacTxQueueDefault =
{
    NULL,                       /* jobQueId */
    0,                          /* priority */
    0,                          /* tbdNum */
    0,                          /* allowedFrags */
    NULL,                       /* pTxBdBase */
    0,                          /* txBdMemSize */
    0,                          /* txBdMemAttributes */
    NULL                        /* txBdMemFreeMethod */
};

LOCAL VXB_PARAMETERS atsemacParamDefaults[] =
{
    {"rxQueue00", VXB_PARAM_POINTER,     {(void *)&atsemacRxQueueDefault}},
    {"txQueue00", VXB_PARAM_POINTER,     {(void *)&atsemacTxQueueDefault}},
    {NULL,        VXB_PARAM_END_OF_LIST, {NULL}}
};

#define ATSEMAC_VXBNAME     "atse"
#define ATSEMAC_NAME        "atse"

LOCAL struct vxbPlbRegister atsemacRegistration =
{
    {
        NULL,                  
        VXB_DEVID_DEVICE,      
        VXB_BUSID_PLB,         
        VXBUS_VERSION_4,          /* 版本号,6.7为版本4 */
        ATSEMAC_NAME,     /*设备名称,必须和hwconfig.c中统一*/
        &atsemacFuncs,               
        atsemacMethods,          
        NULL,              
        atsemacParamDefaults
    }
};

UINT32 atsemac_debug_flag = 1;
char * g_pAlignedMem[EMAC_TX_DESC_CNT] = {NULL};

extern UINT32 bsp_printf(const char *  fmt, ...);

INT32 sysMacSwReset(np_tse_mac *pmac) 
{
    INT32 timeout;
    INT32 cc;
        
    cc = IORD_ALTERA_TSEMAC_CMD_CONFIG(pmac);  

    IOWR_ALTERA_TSEMAC_CMD_CONFIG(pmac, (ALTERA_TSEMAC_CMD_SW_RESET_MSK | ALTERA_TSEMAC_CMD_ETH_SPEED_MSK));

    timeout = 0;
    while( (0 != (IORD_ALTERA_TSEMAC_CMD_CONFIG(pmac) & ALTERA_TSEMAC_CMD_SW_RESET_MSK)) 
        && (timeout < ALTERA_TSE_SW_RESET_TIME_OUT_CNT) )
    {
      timeout++;
    }

    IOWR_ALTERA_TSEMAC_CMD_CONFIG(pmac, cc);   
    
    if(ALTERA_TSE_SW_RESET_TIME_OUT_CNT == timeout)
    {
        return (-1);
    }
    
    return SUCCESS;
}


STATUS atsemacReset ( VXB_DEVICE_ID pDev )
{
    ATSEMACEND *pDrvCtrl;

    pDrvCtrl = pDev->pDrvCtrl;
    sysMacSwReset(pDrvCtrl->tseMac);  
    
    return (OK);
}

INT32 sysPhyWrMdioReg(np_tse_mac *pmacAddr, UINT8 reg_num, UINT8 lsb_num, UINT8 bit_length, UINT16 data)
{
    
    UINT16 temp_data;
    UINT16 bit_mask;
    INT32 i;
    np_tse_mac *pmac = (np_tse_mac *) pmacAddr;
    
    bit_mask = 0x00;
    /* generate mask consist of bit_length number of 1
     * eg: bit_length = 3, bit_mask = 0b0000 0000 0000 0111
     */
    for(i = 0; i < bit_length; i++)
    {
        bit_mask <<= 1;
        bit_mask |= 0x01;        
    }
    
    /* shifting mask to left by bit_num */
    bit_mask <<= lsb_num;

    /* read register data */
    temp_data = IORD(&pmac->mdio0, reg_num);
    
    /* clear bits to be written */
    temp_data &= ~bit_mask;
    
    /* OR-ed together corresponding bits data */
    temp_data |= ((data << lsb_num) & bit_mask);    
    
    /* write data to MDIO register */
    IOWR(&pmac->mdio1, reg_num, temp_data);
    
    return SUCCESS;
    
}


UINT32 sysPhyRdMdioReg(np_tse_mac *pmacAddr, UINT8 reg_num, UINT8 lsb_num, UINT8 bit_length)
{
    UINT16 temp_data;
    UINT32 bit_mask;
    INT32 i;
    np_tse_mac *pmac = (np_tse_mac *) pmacAddr;
    
    bit_mask = 0x00;
    /* generate mask consist of bit_length number of 1
     * eg: bit_length = 3, bit_mask = 0b0000 0000 0000 0111
     */
    for(i = 0; i < bit_length; i++)
    {
        bit_mask <<= 1;
        bit_mask |= 0x01;        
    }
    
    /* read register data */
    temp_data = IORD(&pmac->mdio0, reg_num);
    
    /* shifting read data */
    temp_data >>= lsb_num;
    
    return (temp_data & bit_mask);
}

LOCAL int sysPhyAutoNegPHYSpeed(ATSEMACEND *pDrvCtrl)
{
    INT32 timeout = 0;
    if(!sysPhyRdMdioReg(pDrvCtrl->tseMac, TSE_PHY_MDIO_STATUS, TSE_PHY_MDIO_STATUS_AN_ABILITY, 1))
    {
		return TSE_PHY_AN_NOT_CAPABLE;
    }
    
    /* enable Auto-Negotiation */    
    sysPhyWrMdioReg(pDrvCtrl->tseMac, TSE_PHY_MDIO_CONTROL, TSE_PHY_MDIO_CONTROL_AN_ENA, 1, 1);
    
    /* send PHY reset command */
    sysPhyWrMdioReg(pDrvCtrl->tseMac, TSE_PHY_MDIO_CONTROL, TSE_PHY_MDIO_CONTROL_RESTART_AN, 1, 1);
    
    while(0 == sysPhyRdMdioReg(pDrvCtrl->tseMac, TSE_PHY_MDIO_STATUS, TSE_PHY_MDIO_STATUS_AN_COMPLETE, 1))
    { 
        if(timeout++ > ALTERA_AUTONEG_TIMEOUT_THRESHOLD) 
        {
		   return TSE_PHY_AN_NOT_COMPLETE;
        }
    }

    return TSE_PHY_AN_COMPLETE;
}



static unsigned char * sysMacGetMacAddr(ATSEMACEND * pDrvCtrl)
{
	int nChannel;
	nChannel = pDrvCtrl->nVxUnit;

	if(nChannel == 0)
	{
		return &atsemacMacAddrDft0[0];
	}
	else if(nChannel == 1)
	{
		return &atsemacMacAddrDft1[0];
	}
	return &atsemacMacAddrDft0[0];;
}

static void sysMacSetMacAddr(ATSEMACEND * pDrvCtrl,unsigned char * macaddr)
{
	unsigned int MacAddr;
	unsigned char *Aptr = (unsigned char *) macaddr;

	MacAddr = Aptr[0];
	MacAddr |= Aptr[1] << 8;
	MacAddr |= Aptr[2] << 16;
	MacAddr |= Aptr[3] << 24;
    IOWR_ALTERA_TSEMAC_MAC_0(pDrvCtrl->tseMac, MacAddr);
    
    MacAddr =0;
    MacAddr |= Aptr[4];
	MacAddr |= Aptr[5] << 8;
    IOWR_ALTERA_TSEMAC_MAC_1(pDrvCtrl->tseMac, MacAddr);
                             
}

LOCAL int sysMacSetSpeed(ATSEMACEND * pDrvCtrl, int speed)
{
	INT32 helpvar;
    helpvar = IORD_ALTERA_TSEMAC_CMD_CONFIG(pDrvCtrl->tseMac);

    if(speed == 0)
    {
        sysPhyAutoNegPHYSpeed(pDrvCtrl);
    }
    /* 1000 Mbps */
    else if(speed == TSE_PHY_SPEED_1000) 
    {
        helpvar |= ALTERA_TSEMAC_CMD_ETH_SPEED_MSK;
        helpvar &= ~ALTERA_TSEMAC_CMD_ENA_10_MSK;
    }
    /* 100 Mbps */
    else if(speed == TSE_PHY_SPEED_100) 
    {
        helpvar &= ~ALTERA_TSEMAC_CMD_ETH_SPEED_MSK;
        helpvar &= ~ALTERA_TSEMAC_CMD_ENA_10_MSK;
    }
    /* 10 Mbps */
    else if(speed == TSE_PHY_SPEED_10) 
    {
        helpvar &= ~ALTERA_TSEMAC_CMD_ETH_SPEED_MSK;
        helpvar |= ALTERA_TSEMAC_CMD_ENA_10_MSK;
    }  
    else 
    {
        helpvar |= ALTERA_TSEMAC_CMD_ETH_SPEED_MSK;
        helpvar &= ~ALTERA_TSEMAC_CMD_ENA_10_MSK;
    }

    IOWR_ALTERA_TSEMAC_CMD_CONFIG(pDrvCtrl->tseMac, helpvar);
    
    /* PHY无需配置 */    
    atsemacReset(pDrvCtrl->pDev);
    
    return speed;
}

static int sysMacGetMTU(ATSEMACEND * pDrvCtrl)
{
	/* return 1500; */
	return ETHERMTU;
}



void sysMacTransIntEnable(ATSEMACEND * pDrvCtrl )
{
    UINT32 control;
    control = IORD_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma0TxReg);
    control |= ALTERA_AVALON_SGDMA_CONTROL_IE_GLOBAL_MSK;

    IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma0TxReg, control);
}

void sysMacTransIntDisable(ATSEMACEND * pDrvCtrl)
{
    UINT32 control;
    control = IORD_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma0TxReg);
    control &= ~ALTERA_AVALON_SGDMA_CONTROL_IE_GLOBAL_MSK;

    IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma0TxReg, control);
}

void sysMacTransIntClear(ATSEMACEND * pDrvCtrl)
{
    UINT32 control;
    control = IORD_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma0TxReg);
    control |= ALTERA_AVALON_SGDMA_CONTROL_CLEAR_INTERRUPT_MSK;

    IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma0TxReg, control);
}

void sysMacRcvIntEnable(ATSEMACEND * pDrvCtrl)
{
    UINT32 control;
    control = IORD_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg);
    control |= ALTERA_AVALON_SGDMA_CONTROL_IE_GLOBAL_MSK;

    IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg, control);
}

void sysMacRcvIntDisable(ATSEMACEND * pDrvCtrl)
{
    UINT32 control;
    control = IORD_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg);
    control &= ~ALTERA_AVALON_SGDMA_CONTROL_IE_GLOBAL_MSK;

    IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg, control);

}

void sysMacRcvIntClear(ATSEMACEND * pDrvCtrl)
{
    UINT32 control;
    control = IORD_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg);
    control |= ALTERA_AVALON_SGDMA_CONTROL_CLEAR_INTERRUPT_MSK;

    IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg, control);
}

void sysMacIntEnable(ATSEMACEND * pDrvCtrl)
{
    //sysMacRcvIntEnable( pDrvCtrl);
    //sysMacTransIntEnable(pDrvCtrl);
}

void sysMacIntDisable(ATSEMACEND *  pDrvCtrl)
{
    //sysMacRcvIntDisable( pDrvCtrl);
    //sysMacTransIntDisable( pDrvCtrl);
}

void sysMacIntClear(ATSEMACEND * pDrvCtrl)
{
    //sysMacRcvIntClear( pDrvCtrl);
    //sysMacTransIntClear( pDrvCtrl);
}

STATUS sysMacMemoryInit(ATSEMACEND *pDrvCtrl)
{
    int nMTU_Size, loop;   

    /* allocate a buffer pool */
    nMTU_Size = sysMacGetMTU(pDrvCtrl);       
    pDrvCtrl->atseMacMaxMtu = nMTU_Size ;    /* nMTU_Size + ETH_HDR_SIZE + ETH_TRL_SIZE  1518*/


    for(loop = 0; loop < ALTERA_SGDMA_RX_RING_NUM ; loop++)
    {
        pDrvCtrl->emacRxMblk[loop] = NULL;
    }

    for(loop = 0; loop < ALTERA_SGDMA_TX_RING_NUM ; loop++)
    {
        pDrvCtrl->emacTxMblk[loop] = NULL;
    }

    return(OK);

    
}

LOCAL void sysMacInitTse(ATSEMACEND * pDrvCtrl)
{
    int status = SUCCESS;

    /* Set desc mem base addr */
    pDrvCtrl->pDMARecvDesc = (ATSE_SGDMA_DESCRIPTOR *) ALTERA_ON_CHIP_MEM_BASE_ADDRESS;
    pDrvCtrl->pDMASendDesc =  (ATSE_SGDMA_DESCRIPTOR *)(ALTERA_ON_CHIP_MEM_BASE_ADDRESS + 
                            ALTERA_SGDMA_RX_RING_NUM * sizeof(ATSE_SGDMA_DESCRIPTOR));
    
	/* Init SGDMA0 TX & RX*/
    sysSgdmaInit(pDrvCtrl->pSgdma0TxReg);
    sysSgdmaInit(pDrvCtrl->pSgdma1RxReg);
	
    /* Reset RX-side SGDMA */
    IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg, ALTERA_AVALON_SGDMA_CONTROL_SOFTWARERESET_MSK);
    IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg, 0x0);

    /* reset the mac */ 
    /* 去掉中断只能，在end start函数中使能 mmac_cc_TX_ENA_mask | mmac_cc_RX_ENA_mask  */ 
    if(SUCCESS != sysMacSwReset(pDrvCtrl->tseMac)) 
    {
        printf("sysMacInitTse :TSEMAC SW reset bit never cleared!\n");
    }
    
    /* NO shared fifo Initialize MAC registers */
    IOWR_ALTERA_TSEMAC_FRM_LENGTH(pDrvCtrl->tseMac, ALTERA_TSE_MAC_MAX_FRAME_LENGTH); 
    IOWR_ALTERA_TSEMAC_RX_ALMOST_EMPTY(pDrvCtrl->tseMac, 8);
    IOWR_ALTERA_TSEMAC_RX_ALMOST_FULL(pDrvCtrl->tseMac, 8);
    IOWR_ALTERA_TSEMAC_TX_ALMOST_EMPTY(pDrvCtrl->tseMac, 8);
    IOWR_ALTERA_TSEMAC_TX_ALMOST_FULL(pDrvCtrl->tseMac,  3);
    IOWR_ALTERA_TSEMAC_TX_SECTION_EMPTY(pDrvCtrl->tseMac, TRIPLE_SPEED_ETHERNET_0_RECEIVE_FIFO_DEPTH - 16);   
    IOWR_ALTERA_TSEMAC_TX_SECTION_FULL(pDrvCtrl->tseMac,  0);  
    IOWR_ALTERA_TSEMAC_RX_SECTION_EMPTY(pDrvCtrl->tseMac, TRIPLE_SPEED_ETHERNET_0_TRANSMIT_FIFO_DEPTH - 16);   
    IOWR_ALTERA_TSEMAC_RX_SECTION_FULL(pDrvCtrl->tseMac,  0);

    return ;
	
}

void sysSgdmaInit (void *base)
{
    /* 
    * Halt any current transactions (reset the device)
    * SW reset is written twice per SGDMA documentation 
    */
    IOWR_ALTERA_AVALON_SGDMA_CONTROL(base,
    ALTERA_AVALON_SGDMA_CONTROL_SOFTWARERESET_MSK);
    IOWR_ALTERA_AVALON_SGDMA_CONTROL(base,
    ALTERA_AVALON_SGDMA_CONTROL_SOFTWARERESET_MSK);

    /*
    * Disable interrupts, halt future descriptor processing,
    * and clear status register content
    */
    IOWR_ALTERA_AVALON_SGDMA_CONTROL(base, 0x0);
    IOWR_ALTERA_AVALON_SGDMA_STATUS(base, 0xFF);

    return ;
}

int atseCount = 0;

void atsemacRegister(void)
{
    vxbDevRegister((struct vxbDevRegInfo *)&(atsemacRegistration));
}

LOCAL void atsemacInstInit (struct vxbDev * pDev)
{
    const struct hcfDevice * pHcf;

    /* always use the unit number allocated in the hwconf.c file */
    pHcf = hcfDeviceGet(pDev);
    vxbInstUnitSet(pDev, pHcf->devUnit);
    
    return;
}


LOCAL void atsemacInstInit2(struct vxbDev * pDev)
{
    ATSEMACEND *pDrvCtrl;
    pDrvCtrl = malloc(sizeof(ATSEMACEND));

    /* bsp_printf("atsemacInstInit2 : enter   \r\n"); */
    if (pDrvCtrl == NULL) 
    {
        return;
    }
    
    bzero ((char *)pDrvCtrl, sizeof(ATSEMACEND));
    
    pDev->pDrvCtrl = pDrvCtrl;
    pDrvCtrl->pDev = pDev;
    
    return;
}

LOCAL void atsemacInstConnect(struct vxbDev * pDev)
{
    return;
}

LOCAL STATUS atsemacInstUnlink(VXB_DEVICE_ID pDev, void * unused)
{
    ATSEMACEND * pDrvCtrl;
    pDrvCtrl = pDev->pDrvCtrl;

    if (pDrvCtrl->muxDevCookie != NULL)
    {
        if (muxDevStop(pDrvCtrl->muxDevCookie) != OK)
        {
            return (ERROR);
        }
        
        /* Detach from the MUX. */
        if (muxDevUnload (ATSEMAC_NAME, pDev->unitNumber) != OK)
        {
            return (ERROR);
        }
    }

    /* Disconnect the ISR. */
    vxbIntDisconnect (pDev, 0, atsemacEndInt, pDrvCtrl);
    
    /* Destroy our MII bus and child PHYs. */
    miiBusDelete (pDrvCtrl->emacMiiBus);
    semDelete (pDrvCtrl->atseDevSem);

    /* Destroy the adapter context. */
    free (pDrvCtrl);
    pDev->pDrvCtrl = NULL;
    
    return (OK);
}


LOCAL STATUS atsemacPhyRead(VXB_DEVICE_ID pDev, UINT8 phyAddr, UINT8 regAddr, UINT16 *dataVal)
{
    ATSEMACEND * pDrvCtrl;

    /* only one PHY chip is connected */
    if(phyAddr != 0)
        return(ERROR);
 
    pDrvCtrl = pDev->pDrvCtrl;

    semTake (pDrvCtrl->atseDevSem, WAIT_FOREVER);
    
    if(regAddr == 1)
    {
         *dataVal = (MII_SR_LINK_STATUS | MII_SR_AUTO_NEG);
    }
    else
    {
        *dataVal =0xffff;
    }

    semGive (pDrvCtrl->atseDevSem);
    
    return (OK);
}


LOCAL STATUS atsemacPhyWrite(VXB_DEVICE_ID pDev, UINT8 phyAddr, UINT8 regAddr, UINT16 dataVal)
{
    return (OK);
}


LOCAL STATUS atsemacLinkUpdate( VXB_DEVICE_ID pDev )
{
    ATSEMACEND * pDrvCtrl;
    UINT32 oldStatus;
    
    if (pDev->pDrvCtrl == NULL)
        return (ERROR);

    pDrvCtrl = (ATSEMACEND *)pDev->pDrvCtrl;
    
    semTake (pDrvCtrl->atseDevSem, WAIT_FOREVER);

    if (pDrvCtrl->emacMiiBus == NULL)
    {
        semGive (pDrvCtrl->atseDevSem);
        return (ERROR);
    }

    oldStatus = pDrvCtrl->emacCurStatus;
    
    if ( miiBusModeGet (pDrvCtrl->emacMiiBus,
            &pDrvCtrl->emacCurMedia, &pDrvCtrl->emacCurStatus) == ERROR)
    {
        semGive (pDrvCtrl->atseDevSem);
        return (ERROR);
    }

    /* No matter atseMacCurMedia, set it 1G  */
    pDrvCtrl->endObj.mib2Tbl.ifSpeed = 1000000000;

    if (pDrvCtrl->endObj.pMib2Tbl != NULL)
    {
        pDrvCtrl->endObj.pMib2Tbl->m2Data.mibIfTbl.ifSpeed =
                                            pDrvCtrl->endObj.mib2Tbl.ifSpeed;
    }
    
    if (!(pDrvCtrl->endObj.flags & IFF_UP))
    {
        semGive (pDrvCtrl->atseDevSem);
        return (OK);
    }

    /* If status went from down to up, announce link up. */

    if (pDrvCtrl->emacCurStatus & IFM_ACTIVE && !(oldStatus & IFM_ACTIVE))
    {
        jobQueueStdPost (pDrvCtrl->emacJobQueue, NET_TASK_QJOB_PRI,
                         muxLinkUpNotify, &pDrvCtrl->endObj, 
                         NULL, NULL, NULL, NULL);    
    }
    /* If status went from up to down, announce link down. */
    else if (!(pDrvCtrl->emacCurStatus & IFM_ACTIVE) && oldStatus & IFM_ACTIVE)
    {
         jobQueueStdPost (pDrvCtrl->emacJobQueue, NET_TASK_QJOB_PRI,
                              muxLinkDownNotify, &pDrvCtrl->endObj,
                              NULL, NULL, NULL, NULL);
    }
    
    semGive (pDrvCtrl->atseDevSem);
    
    return (OK);
}



void atsemacMuxConnect(struct vxbDev * pDev, void * unused)
{
    ATSEMACEND *pDrvCtrl;    

    pDrvCtrl = pDev->pDrvCtrl;

    /* Create our MII bus. */
    pDrvCtrl->atseDevSem = semMCreate (SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE);

    miiBusCreate( pDev, &pDrvCtrl->emacMiiBus );
    miiBusMediaListGet( pDrvCtrl->emacMiiBus, &pDrvCtrl->emacMediaList );
    miiBusModeSet( pDrvCtrl->emacMiiBus,
                   pDrvCtrl->emacMediaList->endMediaListDefault );

    /* Attach our ISR */
    vxbIntConnect (pDev, 0, atsemacEndInt, pDev->pDrvCtrl);
    
    pDrvCtrl->muxDevCookie = muxDevLoad (pDev->unitNumber, atsemacEndLoad, "", TRUE, pDev);
    
    if (pDrvCtrl->muxDevCookie != NULL)
    {
        muxDevStart (pDrvCtrl->muxDevCookie);
    }
    
    return;
}


END_OBJ *atsemacEndLoad (  char * loadStr,  void * pArg  )
{
    ATSEMACEND *pDrvCtrl;
    VXB_DEVICE_ID pDev;

    int loop;
    /* make the MUX happy */
    if (NULL == loadStr)
    {
        return (NULL);
    }
    
    if (loadStr[0] == 0)
    {
        bcopy (ATSEMAC_NAME, loadStr, sizeof(ATSEMAC_NAME));
        return (NULL);
    }

    pDev = (VXB_DEVICE_ID)pArg; 
    pDrvCtrl = pDev->pDrvCtrl;   

    pDrvCtrl->nVxUnit = pDev->unitNumber;     
    pDrvCtrl->tseMac = (np_tse_mac*)ALTERA_TSE_MAC_BASE_ADDRESS;   
    pDrvCtrl->pSgdma0TxReg = (void*)ALTERA_SGDMA0_TX_BASE_ADDRESS;
    pDrvCtrl->pSgdma1RxReg = (void*)ALTERA_SGDMA1_RX_BASE_ADDRESS;

    sysMacInitTse(pDrvCtrl);     
    
    pDrvCtrl->macAddrPtr = sysMacGetMacAddr(pDrvCtrl);  
    pDrvCtrl->nSpeed = sysMacSetSpeed(pDrvCtrl, TSE_PHY_SPEED_100);    /* sysMacSetSpeed(pDrvCtrl, 0); */  
    
    sysMacSetMacAddr(pDrvCtrl,pDrvCtrl->macAddrPtr);   

    pDrvCtrl->atseMacMaxMtu = sysMacGetMTU(pDrvCtrl);     

    for(loop = 0; loop < ALTERA_SGDMA_RX_RING_NUM ; loop++)
    {
        pDrvCtrl->emacRxMblk[loop] = NULL;
    }

    for(loop = 0; loop < ALTERA_SGDMA_TX_RING_NUM ; loop++)
    {
        pDrvCtrl->emacTxMblk[loop] = NULL;
    }
    
    if (ERROR == END_OBJ_INIT(&pDrvCtrl->endObj,
                               NULL,
                               ATSEMAC_NAME,        /* pDev->pName */
                               pDev->unitNumber,
                               &atsemacENDFuncs,
                               "Altera TSE Vxbus End Driver") )
    {
        logMsg("%s%d: END_OBJ_INIT failed\n", (int)ATSEMAC_NAME, pDev->unitNumber, 0, 0, 0, 0);
        return (NULL);
    }

    endM2Init (&pDrvCtrl->endObj, M2_ifType_ethernet_csmacd,
                pDrvCtrl->macAddrPtr, ETHER_ADDR_LEN, ETHERMTU, 100000000,
                IFF_NOTRAILERS | IFF_SIMPLEX | IFF_MULTICAST | IFF_BROADCAST);

    pDrvCtrl->atseMacMaxMtu = ETHERMTU ;   

    if (endPoolCreate(ATSEMAC_TUPLE_CNT, &pDrvCtrl->endObj.pNetPool) == ERROR)
    {
        logMsg("%s%d: pool creation failed\n", (int)ATSEMAC_NAME, pDev->unitNumber, 0, 0, 0, 0);
        return (NULL);
    }

    pDrvCtrl->emacPollBuf = endPoolTupleGet(pDrvCtrl->endObj.pNetPool);

    /* set up polling stats */

    pDrvCtrl->emacEndStatsConf.ifPollInterval = sysClkRateGet();
    pDrvCtrl->emacEndStatsConf.ifEndObj = &pDrvCtrl->endObj;
    pDrvCtrl->emacEndStatsConf.ifWatchdog = NULL;
    pDrvCtrl->emacEndStatsConf.ifValidCounters = (END_IFINUCASTPKTS_VALID |
                            END_IFINMULTICASTPKTS_VALID | END_IFINBROADCASTPKTS_VALID |
                            END_IFINOCTETS_VALID | END_IFINERRORS_VALID | END_IFINDISCARDS_VALID |
                            END_IFOUTUCASTPKTS_VALID | END_IFOUTMULTICASTPKTS_VALID |
                            END_IFOUTBROADCASTPKTS_VALID | END_IFOUTOCTETS_VALID |
                            END_IFOUTERRORS_VALID);

    /* set up capabilities */
    pDrvCtrl->emacCaps.cap_available = IFCAP_VLAN_MTU;
    pDrvCtrl->emacCaps.cap_enabled = IFCAP_VLAN_MTU;

    return(&pDrvCtrl->endObj);
        
}


LOCAL STATUS atsemacEndUnload ( END_OBJ * pEnd )
{
    ATSEMACEND * pDrvCtrl;
    VXB_DEVICE_ID pDev;

    if(pEnd == NULL) return ERROR;
    
    /* we must be stopped before we can be unloaded */
    if (pEnd->flags & IFF_UP)
        return (ERROR);

    pDrvCtrl = (ATSEMACEND *)pEnd;
    pDev = pDrvCtrl->pDev;

    endPoolTupleFree(pDrvCtrl->emacPollBuf);

    /* relase our buffer pool */
    endPoolDestroy(pDrvCtrl->endObj.pNetPool);

    endM2Free(&pDrvCtrl->endObj);

    END_OBJECT_UNLOAD(&pDrvCtrl->endObj);

    return (EALREADY);
}


LOCAL STATUS atsemacEndMCastAddrAdd ( END_OBJ * pEnd, char * pAddr)
{
    int retVal;
    ATSEMACEND * pDrvCtrl;

    pDrvCtrl = (ATSEMACEND *)pEnd;

    semTake (pDrvCtrl->atseDevSem, WAIT_FOREVER);

    if (!(pDrvCtrl->endObj.flags & IFF_UP))
    {
        semGive (pDrvCtrl->atseDevSem);
        return (OK);
    }

    retVal = etherMultiAdd(&pEnd->multiList, pAddr);
    
    if (retVal == ENETRESET)
    {
        pEnd->nMulti++;
        /*atsemacEndHashTblPopulate((ATSEMACEND *)pEnd); */
    }

    semGive (pDrvCtrl->atseDevSem);
    
    return (OK);
}


LOCAL STATUS atsemacEndMCastAddrDel ( END_OBJ *pEnd, char * pAddr )
{
    
    int retVal;
    ATSEMACEND * pDrvCtrl;

    pDrvCtrl = (ATSEMACEND *)pEnd;

    semTake (pDrvCtrl->atseDevSem, WAIT_FOREVER);

    if (!(pDrvCtrl->endObj.flags & IFF_UP))
    {
        semGive (pDrvCtrl->atseDevSem);
        return (OK);
    }
        
    retVal = etherMultiDel(&pEnd->multiList, pAddr);

    if (retVal == ENETRESET)
    {
        pEnd->nMulti--;
        /*atsemacEndHashTblPopulate((ATSEMACEND *)pEnd); */
    }
    
    semGive (pDrvCtrl->atseDevSem);
    return (OK);
}


LOCAL STATUS atsemacEndMCastAddrGet ( END_OBJ *  pEnd, MULTI_TABLE * pTable )
{
    STATUS rval;
    ATSEMACEND * pDrvCtrl;

    pDrvCtrl = (ATSEMACEND *)pEnd;

    semTake (pDrvCtrl->atseDevSem, WAIT_FOREVER);

    if (!(pDrvCtrl->endObj.flags & IFF_UP))
    {
        semGive (pDrvCtrl->atseDevSem);
        return (OK);
    }

    rval = etherMultiGet (&pEnd->multiList, pTable);

    semGive (pDrvCtrl->atseDevSem);

    return (rval);
}


LOCAL int atsemacEndIoctl( END_OBJ * pEnd, int  cmd, caddr_t data )
{
    ATSEMACEND * pDrvCtrl;
    END_MEDIALIST * mediaList;
    END_CAPABILITIES * hwCaps;
    
    END_MEDIA * pMedia;
    END_RCVJOBQ_INFO * qinfo;
    UINT32 nQs;
    
    VXB_DEVICE_ID pDev;
    INT32 value;
    int error = OK;
    
    pDrvCtrl = (ATSEMACEND *)pEnd;
    pDev = pDrvCtrl->pDev;

    switch (cmd)
        {
        case EIOCSADDR:
            if (data == NULL)
                error = EINVAL;
            else
                bcopy((char *)data, (char *)pDrvCtrl->macAddrPtr, ETHER_ADDR_LEN);

            /* atsemacEndRxConfig(pDrvCtrl); */            
            break;

        case EIOCGADDR:
            if (data == NULL)
                error = EINVAL;
            else
                bcopy((char *)pDrvCtrl->macAddrPtr, (char *)data, ETHER_ADDR_LEN);

            break;

        case EIOCSFLAGS:
            value = (INT32)data;
            if (value < 0)
            {
                value = -value;
                value--;
                END_FLAGS_CLR(pEnd, value);
            }
            else
                END_FLAGS_SET(pEnd, value);

            /* atsemacEndRxConfig(pDrvCtrl); */

            break;

        case EIOCGFLAGS:
            if (data == NULL)
                error = EINVAL;
            else
                *(long *)data = END_FLAGS_GET(pEnd);

            break;

        case EIOCMULTIADD:
            error = atsemacEndMCastAddrAdd(pEnd, (char *)data);
            break;

        case EIOCMULTIDEL:
            error = atsemacEndMCastAddrDel(pEnd, (char *)data);
            break;

        case EIOCMULTIGET:
            error = atsemacEndMCastAddrGet(pEnd, (MULTI_TABLE *)data);
            break;

        case EIOCPOLLSTART:
            #if 0
            pDrvCtrl->emacPolling = TRUE;
            pDrvCtrl->dmIntMask = dmMacRead(pDev, DM_IMR);
            dmMacWrite(pDev, DM_IMR, 0x80);

            /*
             * We may have been asked to enter polled mode while
             * there are transmissions pending. This is a problem,
             * because the polled transmit routine expects that
             * the TX ring will be empty when it's called. In
             * order to guarantee this, we have to drain the TX
             * ring here. We could also just plain reset and
             * reinitialize the transmitter, but this is faster.
             */

            while (pDrvCtrl->dmTxFree < DM_TX_CNT)
                {
                M_BLK_ID pMblk;

                pMblk = pDrvCtrl->dmTxMblk[pDrvCtrl->dmTxCons];

                if (pMblk != NULL)
                    {
                    endPoolTupleFree (pMblk);
                    pDrvCtrl->dmTxMblk[pDrvCtrl->dmTxCons] = NULL;
                    }

                pDrvCtrl->dmTxFree++;
                DM_TX_INC (pDrvCtrl->dmTxCons, DM_TX_CNT);
                }
            #endif
            break;

        case EIOCPOLLSTOP:
            pDrvCtrl->emacPolling = FALSE;
            /* dmReConfigure(pDev); */
            break;  

        case EIOCGMIB2233:
        case EIOCGMIB2:
            error = endM2Ioctl(&pDrvCtrl->endObj, cmd, data);
            break;

        case EIOCGMEDIALIST:
            if (data == NULL)
            {
                error = EINVAL;
                break;
            }
            if (pDrvCtrl->emacMediaList->endMediaListLen == 0)
            {
                error = ENOTSUP;
                break;
            }

            mediaList = (END_MEDIALIST *)data;
            if (mediaList->endMediaListLen < pDrvCtrl->emacMediaList->endMediaListLen)
            {
                mediaList->endMediaListLen = pDrvCtrl->emacMediaList->endMediaListLen;
                error = ENOSPC;
                
                break;
            }

            bcopy((char *)pDrvCtrl->emacMediaList, (char *)mediaList,
                  sizeof(END_MEDIALIST) + (sizeof(UINT32) *  pDrvCtrl->emacMediaList->endMediaListLen));

            break;

        case EIOCGIFMEDIA:
            if (data == NULL)
                error = EINVAL;
            else
            {
                pMedia = (END_MEDIA *)data;
                pMedia->endMediaActive = pDrvCtrl->emacCurMedia;
                pMedia->endMediaStatus = pDrvCtrl->emacCurStatus;
            }

            break;

        case EIOCSIFMEDIA:
            if (data == NULL)
                error = EINVAL;
            else
            {
                pMedia = (END_MEDIA *)data;
                miiBusModeSet(pDrvCtrl->emacMiiBus, pMedia->endMediaActive);
                atsemacLinkUpdate(pDrvCtrl->pDev);
                error = OK;
            }

            break;
        
        case EIOCGIFCAP:
            hwCaps = (END_CAPABILITIES *)data;
            if (hwCaps == NULL)
            {
                error = EINVAL;
                break;
            }
            hwCaps->cap_available = pDrvCtrl->emacCaps.cap_available;
            hwCaps->cap_enabled = pDrvCtrl->emacCaps.cap_enabled;

            break;
 
        case EIOCSIFCAP:
            error = ENOTSUP;
            break;

        case EIOCGIFMTU:
            if (data == NULL)
                error = EINVAL;
            else
                *(INT32 *)data = pEnd->mib2Tbl.ifMtu;

            break;

        case EIOCSIFMTU:
            value = (INT32)data;
            
            if (value <= 0 || value > pDrvCtrl->atseMacMaxMtu)
            {
                error = EINVAL;
                break;
            }
            
            pEnd->mib2Tbl.ifMtu = value;
            if (pEnd->pMib2Tbl != NULL)
                pEnd->pMib2Tbl->m2Data.mibIfTbl.ifMtu = value;

            break;

        case EIOCGRCVJOBQ:
            if (data == NULL)
                {
                error = EINVAL;
                break;
                }

            qinfo = (END_RCVJOBQ_INFO *)data;
            nQs = qinfo->numRcvJobQs;
            qinfo->numRcvJobQs = 1;
            
            if (nQs < 1)
                error = ENOSPC;
            else
                qinfo->qIds[0] = pDrvCtrl->emacJobQueue;

            break;

        default:
            error = EINVAL;
            break;
        }

    return (error);
}


LOCAL void atsemacEndRxConfig  ( ATSEMACEND * pDrvCtrl )
{
    VXB_DEVICE_ID pDev;

    pDev = pDrvCtrl->pDev;

    #if 0
    mac = dmMacRead(pDev, DM_RCR);    

    /* Set station address */

    dmMacWrite (pDev, DM_MAC_ADDR0, pDrvCtrl->dmAddr[0]);
    dmMacWrite (pDev, DM_MAC_ADDR1, pDrvCtrl->dmAddr[1]);
    dmMacWrite (pDev, DM_MAC_ADDR2, pDrvCtrl->dmAddr[2]);
    dmMacWrite (pDev, DM_MAC_ADDR3, pDrvCtrl->dmAddr[3]);
    dmMacWrite (pDev, DM_MAC_ADDR4, pDrvCtrl->dmAddr[4]);
    dmMacWrite (pDev, DM_MAC_ADDR5, pDrvCtrl->dmAddr[5]);
    
    /* Enable promisc mode, if specified. */

    if (pDrvCtrl->endObj.flags & IFF_PROMISC)
        mac |= DM_RCR_PRMSC;

    dmMacWrite (pDev, DM_RCR, mac);    

    /* Program the multicast filter. */

    dmEndHashTblPopulate (pDrvCtrl);
    #endif
    
    return;
}



LOCAL STATUS atsemacEndStart( END_OBJ * pEnd )
{
    ATSEMACEND *   pDrvCtrl;
    VXB_DEVICE_ID  pDev;
    UINT32 data , i;

    M_BLK_ID pMblk;
    ATSE_SGDMA_DESCRIPTOR * pDesc = NULL , * pDescNext= NULL;
    
    pDrvCtrl = (ATSEMACEND *)pEnd;
    pDev = pDrvCtrl->pDev;
    semTake (pDrvCtrl->atseDevSem, WAIT_FOREVER);
    END_TX_SEM_TAKE (pEnd, WAIT_FOREVER);

    if (pEnd->flags & IFF_UP)
    {
        END_TX_SEM_GIVE (pEnd);
        semGive (pDrvCtrl->atseDevSem);
        return (OK);
    }

    for(i = 0 ; i< EMAC_TX_DESC_CNT; i++)
        g_pAlignedMem[i] = (char *)memalign(4, 1560);
    
    /* initialize job queues */
    pDrvCtrl->emacJobQueue = netJobQueueId;

    QJOB_SET_PRI(&pDrvCtrl->emacTxJob, NET_TASK_QJOB_PRI);
    pDrvCtrl->emacTxJob.func = atsemacEndTxHandle;
    
    QJOB_SET_PRI(&pDrvCtrl->emacRxJob, NET_TASK_QJOB_PRI);
    pDrvCtrl->emacRxJob.func = atsemacEndRxHandle;
    
    QJOB_SET_PRI(&pDrvCtrl->emacIntJob, NET_TASK_QJOB_PRI);
    pDrvCtrl->emacIntJob.func = atsemacEndIntHandle;

    vxAtomicSet (&pDrvCtrl->emacRxPending,  FALSE);
    vxAtomicSet (&pDrvCtrl->emacTxPending,  FALSE);
    vxAtomicSet (&pDrvCtrl->emacIntPending, FALSE);

    /* reset controller to known state */
    atsemacReset(pDev);

    /* Program the RX filter. */
    atsemacEndRxConfig (pDrvCtrl);

    /* set up the TX descriptor ring , descriptor initlized in endSend function */
    bzero((char *)pDrvCtrl->pDMASendDesc,  ALTERA_SGDMA_TX_RING_NUM * sizeof(ATSE_SGDMA_DESCRIPTOR));
    
    /* set up the RX descriptor ring */
    /* The last one is clear for terminate list with non-hardware owned description */
    bzero((char *)pDrvCtrl->pDMARecvDesc,  ALTERA_SGDMA_RX_RING_NUM * sizeof(ATSE_SGDMA_DESCRIPTOR));
    
    for (i = 0; i < EMAC_RX_DESC_CNT; i++)
    {
        pMblk = endPoolTupleGet(pDrvCtrl->endObj.pNetPool);
        if (pMblk == NULL)
            return (ERROR);

        pMblk->m_next = NULL;
        pDrvCtrl->emacRxMblk[i] = pMblk;

        pDesc = &pDrvCtrl->pDMARecvDesc[i];
        pDescNext = &pDrvCtrl->pDMARecvDesc[i + 1];        

        pDesc->read_addr                = 0;
        pDesc->write_addr               = (UINT32 *)((mtod(pMblk, UINT32)) & 0x0fffffff);
        pDesc->next                     = (UINT32 *)((UINT32) pDescNext & 0x0fffffff);
        pDesc->read_addr_pad            = 0x0;
        pDesc->write_addr_pad           = 0x0;
        pDesc->next_pad                 = 0x0;
        pDesc->bytes_to_transfer        = 0;
        pDesc->actual_bytes_transferred = 0;
        pDesc->status                   = 0x0;
        pDesc->read_burst               = 0;
        pDesc->write_burst              = 0;

        /*pDesc->control = ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_OWNED_BY_HW_MSK; */ /* mxl : added */
        
        IOWR_32DIRECT(&pDesc->actual_bytes_transferred,  0 , 
                      (UINT32) ((ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_OWNED_BY_HW_MSK    
                          |ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_GENERATE_EOP_MSK)  <<  24) );
                                 
        /* cacheFlush(DATA_CACHE,(void*) pDesc,  sizeof(ATSE_SGDMA_DESCRIPTOR)); uncache address , NO need to flush*/        
        
    }
    
    if(NULL != pDesc)
    	pDesc->next   = (UINT32 *)((UINT32) &pDrvCtrl->pDMARecvDesc[0] & 0x0fffffff);  /* mxl : debug, last one should point first */

    /* Clear any (previous) status register information
    * that might occlude our error checking later. */    
    IOWR_ALTERA_AVALON_SGDMA_STATUS(pDrvCtrl->pSgdma0TxReg, 0xFF);
    IOWR_ALTERA_AVALON_SGDMA_STATUS(pDrvCtrl->pSgdma1RxReg, 0xFF);

    #if 0 /* mxl: init at rx & tx handle */
    /* Point the controller at the descriptor */
    IOWR_ALTERA_AVALON_SGDMA_NEXT_DESC_POINTER(pDrvCtrl->pSgdma0TxReg, ((UINT32)(&pDrvCtrl->pDMASendDesc[0]) & 0x0fffffff));
    #endif
    IOWR_ALTERA_AVALON_SGDMA_NEXT_DESC_POINTER(pDrvCtrl->pSgdma1RxReg, ((UINT32)(&pDrvCtrl->pDMARecvDesc[0]) & 0x0fffffff));

    /* initialize state */
    pDrvCtrl->emacRxIdx   = 0;
    pDrvCtrl->emacTxStall = FALSE;
    pDrvCtrl->emacTxProd  = 0;
    pDrvCtrl->emacTxCons  = 0;
    pDrvCtrl->emacTxCnt   = 0;
    pDrvCtrl->emacTxPend  = EMAC_TX_DESC_CNT - 1;
    pDrvCtrl->emacTxFree  = EMAC_TX_DESC_CNT;
    
    #if 1
    /* Enable interrupts */
    vxbIntEnable (pDev, 0, atsemacEndInt, pDrvCtrl);  

    /* Enable SGDMA transmit interrupt */
    data = IORD_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma0TxReg);

    data |= (ALTERA_AVALON_SGDMA_CONTROL_IE_CHAIN_COMPLETED_MSK     | 
             ALTERA_AVALON_SGDMA_CONTROL_IE_GLOBAL_MSK              |
             ALTERA_AVALON_SGDMA_CONTROL_STOP_DMA_ER_MSK  );
    data &= ~ALTERA_AVALON_SGDMA_CONTROL_STOP_DMA_ER_MSK;  /* added by mxl for bug fixed 20120927 */
    //data &= ~ALTERA_AVALON_SGDMA_CONTROL_IE_CHAIN_COMPLETED_MSK;  /* added by mxl for bug fixed 201201015 */
    IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma0TxReg, data);

    /* Enable SGDMA receive interrupt , RUN  */
    data = IORD_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg);

    data |= (ALTERA_AVALON_SGDMA_CONTROL_IE_DESC_COMPLETED_MSK  |
             ALTERA_AVALON_SGDMA_CONTROL_IE_CHAIN_COMPLETED_MSK | 
             ALTERA_AVALON_SGDMA_CONTROL_IE_GLOBAL_MSK          | 
             ALTERA_AVALON_SGDMA_CONTROL_RUN_MSK                |
             ALTERA_AVALON_SGDMA_CONTROL_STOP_DMA_ER_MSK |
             ALTERA_AVALON_SGDMA_CONTROL_PARK_MSK);         /* added by mxl for park added  */
             
    data &= ~ALTERA_AVALON_SGDMA_CONTROL_STOP_DMA_ER_MSK;   /* added by mxl for bug fixed 20120927 */
    data &= ~ALTERA_AVALON_SGDMA_CONTROL_PARK_MSK;   /* added by mxl for bug fixed 201201015 */
    //data |= ALTERA_AVALON_SGDMA_CONTROL_IE_EOP_ENCOUNTERED_MSK;   /* added by mxl for bug fixed 201201015 */
    //data &= ~ALTERA_AVALON_SGDMA_CONTROL_IE_CHAIN_COMPLETED_MSK;  /* added by mxl for bug fixed 201201015 */
    //data &= ~ALTERA_AVALON_SGDMA_CONTROL_IE_DESC_COMPLETED_MSK;  /* added by mxl for bug fixed 201201015 */
    IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg, data);

    /* enable MAC  global interrupt */
    data = IORD_ALTERA_TSEMAC_CMD_CONFIG(pDrvCtrl->tseMac);
    
    data |= ALTERA_TSEMAC_CMD_TX_ENA_MSK       |
            ALTERA_TSEMAC_CMD_RX_ENA_MSK       |
            mmac_cc_RX_ERR_DISCARD_mask        |
            ALTERA_TSEMAC_CMD_TX_ADDR_INS_MSK  |
            ALTERA_TSEMAC_CMD_RX_ERR_DISC_MSK;   

    IOWR_ALTERA_TSEMAC_CMD_CONFIG(pDrvCtrl->tseMac, data); 
    
    #else
	printf("Spawn Daemon Thread\n");
	
	if(ERROR == taskSpawn("tAtseNet", 80 , 0, 0x20000, tAtseNetD, (int)pDrvCtrl,1,2,3,4,5,6,7,8,9))
	{
	    return (ERROR);
	}
    #endif
    
    /* Set initial link state */
    pDrvCtrl->emacCurMedia  = IFM_ETHER | IFM_NONE;
    pDrvCtrl->emacCurStatus = IFM_AVALID;

    END_FLAGS_SET (pEnd, (IFF_UP | IFF_RUNNING));

    END_TX_SEM_GIVE (pEnd);
    semGive (pDrvCtrl->atseDevSem);

    return (OK);
}


LOCAL STATUS atsemacEndStop ( END_OBJ * pEnd)
{
    ATSEMACEND * pDrvCtrl;
    VXB_DEVICE_ID pDev;
    int i, state;

    pDrvCtrl = (ATSEMACEND *)pEnd;

    if (!(pEnd->flags & IFF_UP))
    {
        semGive (pDrvCtrl->atseDevSem);
        return (OK);
    }

    END_FLAGS_CLR (pEnd, (IFF_UP | IFF_RUNNING));

    pDev = pDrvCtrl->pDev;

    /* disable interrupt */
    vxbIntDisable(pDev, 0, atsemacEndInt, pDrvCtrl);
    
    for (i = 0; i < ATSE_TIMEOUT; i++)
    {
        if ( vxAtomicGet (&pDrvCtrl->emacRxPending) == FALSE &&
             vxAtomicGet (&pDrvCtrl->emacTxPending)  == FALSE &&
             vxAtomicGet (&pDrvCtrl->emacIntPending) == FALSE )
            break;
            
        taskDelay(1);
    }

    if (i == ATSE_TIMEOUT)
    {
        logMsg("%s%d: timed out waiting for job to complete\n", (int)ATSEMAC_NAME, pDev->unitNumber, 0, 0, 0, 0);
    }

    for(i = 0 ; i< EMAC_TX_DESC_CNT; i++)
        free(g_pAlignedMem[i]);

    /* disable the interrupt of MAC */
    state = IORD_ALTERA_TSEMAC_CMD_CONFIG(pDrvCtrl->tseMac);
    
    IOWR_ALTERA_TSEMAC_CMD_CONFIG( pDrvCtrl->tseMac, 
                          state & ~(ALTERA_TSEMAC_CMD_RX_ENA_MSK | ALTERA_TSEMAC_CMD_TX_ENA_MSK)); 

    sysMacIntDisable(pDrvCtrl);
    
    /* Release resources */

    for (i = 0; i < ALTERA_SGDMA_RX_RING_NUM; i++)
    {
        if (pDrvCtrl->emacRxMblk[i] != NULL)
        {
            netMblkClChainFree(pDrvCtrl->emacRxMblk[i]);
            pDrvCtrl->emacRxMblk[i] = NULL;
        }
    }
    endMcacheFlush ();
    
    END_TX_SEM_TAKE (pEnd, WAIT_FOREVER);
    
    for (i = 0; i < ALTERA_SGDMA_TX_RING_NUM; i++)
    {
        if (pDrvCtrl->emacTxMblk[i] != NULL)
        {
            netMblkClChainFree(pDrvCtrl->emacTxMblk[i]);
            pDrvCtrl->emacTxMblk[i] = NULL;
        }
    }


    END_TX_SEM_GIVE (pEnd); 

    return (OK);
}
UINT8 g_atseDebug = 0;
ULONG g_packNum = 0;

LOCAL void atsemacEndIntHandle( void * pArg )
{
    QJOB *pJob;
    ATSEMACEND *pDrvCtrl;
    VXB_DEVICE_ID pDev;
    UINT32 rxStatus, txStatus;
    
    pJob = pArg;
    pDrvCtrl = member_to_object(pJob, ATSEMACEND, emacIntJob);
    pDev = pDrvCtrl->pDev;    

        /* read and clear interrupts here */
    rxStatus = IORD_ALTERA_AVALON_SGDMA_STATUS(pDrvCtrl->pSgdma1RxReg);
    txStatus = IORD_ALTERA_AVALON_SGDMA_STATUS(pDrvCtrl->pSgdma0TxReg);    

    if (rxStatus & (ALTERA_AVALON_SGDMA_STATUS_DESC_COMPLETED_MSK  
                     |ALTERA_AVALON_SGDMA_STATUS_CHAIN_COMPLETED_MSK))
    {
        IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg, 
                        IORD_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg) | 0x80000000);
        
        vxAtomicSet (&pDrvCtrl->emacRxPending, TRUE);
        jobQueuePost (pDrvCtrl->emacJobQueue, &pDrvCtrl->emacRxJob);

    }
    
    else if(rxStatus & ALTERA_AVALON_SGDMA_STATUS_EOP_ENCOUNTERED_MSK)
    {
        IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg, 
                        IORD_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg) | 0x80000000);
        vxAtomicSet (&pDrvCtrl->emacRxPending, TRUE);
        jobQueuePost (pDrvCtrl->emacJobQueue, &pDrvCtrl->emacRxJob);

    }
 
    if (txStatus & (ALTERA_AVALON_SGDMA_STATUS_CHAIN_COMPLETED_MSK | 
                     ALTERA_AVALON_SGDMA_STATUS_DESC_COMPLETED_MSK))
    {

        vxAtomicSet (&pDrvCtrl->emacTxPending, TRUE);   
        jobQueuePost (pDrvCtrl->emacJobQueue, &pDrvCtrl->emacTxJob);
        IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma0TxReg, 
                         IORD_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma0TxReg) | 0x80000000);

    }
    
     //s = intCpuLock();  

    IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg, 
                         IORD_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg) | 0x80000000);

    IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma0TxReg, 
                         IORD_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma0TxReg) | 0x80000000);

    /* Dummy read to ensure IRQ is negated before the ISR returns */
    IORD_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg);
    IORD_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma0TxReg);
    
    //intCpuUnlock(s);
    
    /* Unmask interrupts here */
   // s = intCpuLock();
    sysMacIntEnable(pDrvCtrl);
    //intCpuUnlock(s);
    
    
    return;
}


LOCAL void atsemacEndInt ( ATSEMACEND * pDrvCtrl )
{
    VXB_DEVICE_ID pDev;
    int s;

    pDev = pDrvCtrl->pDev;
    /* mask interrupts here */
    s = intCpuLock();
    sysMacIntDisable(pDrvCtrl);
    
    atsemacEndIntHandle(&pDrvCtrl->emacIntJob);
    intCpuUnlock(s);
    return;
}

LOCAL void atsemacEndRxHandle ( void * pArg )
{
    QJOB *pJob;
    ATSEMACEND *pDrvCtrl;
    VXB_DEVICE_ID pDev;
    
    M_BLK_ID pMblk;
    M_BLK_ID pNewMblk;
    
    ATSE_SGDMA_DESCRIPTOR * pDesc , * pDescNext;
    int loopCounter = ATSEMAC_MAX_RX;
    int desc_status;

    pJob = pArg;
    pDrvCtrl = member_to_object(pJob, ATSEMACEND, emacRxJob);
    pDev = pDrvCtrl->pDev;
    
    sysMacIntDisable(pDrvCtrl);

    pDesc = &pDrvCtrl->pDMARecvDesc[pDrvCtrl->emacRxIdx];

    desc_status = IORD_ALTERA_TSE_SGDMA_DESC_STATUS(pDesc);

    #if 0
    if(0)// IORD_ALTERA_AVALON_SGDMA_STATUS(pDrvCtrl->pSgdma1RxReg) & ALTERA_AVALON_SGDMA_STATUS_CHAIN_COMPLETED_MSK )
    {
        //bsp_printf("\r\nc2");
        #if 0
         /* Clear Run */
        IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg,   0 );

        /* Get & clear status register contents */
        IOWR_ALTERA_AVALON_SGDMA_STATUS(pDrvCtrl->pSgdma1RxReg, 0xFF);

        IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg, 0x0003c );
        #else
        /* Clear Run */
        IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg, 
                                       (IORD_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg) &
                                       ~ALTERA_AVALON_SGDMA_CONTROL_RUN_MSK) );
        
        /* Get & clear status register contents */
        IOWR_ALTERA_AVALON_SGDMA_STATUS(pDrvCtrl->pSgdma1RxReg, 0xFF);

        IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg, 
                                       (IORD_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg) |
                                       ALTERA_AVALON_SGDMA_CONTROL_RUN_MSK) );
        #endif
    }
    #endif
    /* mxl: from TSE eth code reference, different: ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_TERMINATED_BY_EOP_MSK */
    while (loopCounter && 
           ( desc_status & ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_TERMINATED_BY_EOP_MSK ) )
    {

        if( (desc_status & 
            ( ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_CRC_MSK | 
              ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_PARITY_MSK | 
              ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_OVERFLOW_MSK |
              ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_SYNC_MSK | 
              ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_UEOP_MSK | 
              ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_MEOP_MSK | 
              ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_MSOP_MSK ) ) != 0 )
        {
            logMsg("%s%d: bad packet, flag: %x (%p %d)\n", (int)ATSEMAC_NAME,
                    pDev->unitNumber, desc_status, (int)pDesc,
                    pDrvCtrl->emacRxIdx, 0);
                    
            goto skip;
        }

        /* Try to conjure up a new mBlk tuple to hold the incoming packet.
         * If this fails, we have to skip the packet and move to the next one. */         

        pNewMblk = endPoolTupleGet(pDrvCtrl->endObj.pNetPool);
        if (pNewMblk == NULL)
        {
            logMsg("%s%d: out of mBlks at %d\n", (int)ATSEMAC_NAME,
                    pDev->unitNumber, pDrvCtrl->emacRxIdx,0,0,0);
                    
            pDrvCtrl->emacLastError.errCode = END_ERR_NO_BUF;
            muxError(&pDrvCtrl->endObj, &pDrvCtrl->emacLastError);

        skip:
            bsp_printf("get skip packet \r\n");
            
            IOWR_32DIRECT(&pDesc->actual_bytes_transferred,  0 , 
                      (UINT32) ((ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_OWNED_BY_HW_MSK 
                          |ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_GENERATE_EOP_MSK)  <<  24) );
                          

            EMAC_INC_DESC(pDrvCtrl->emacRxIdx, EMAC_RX_DESC_CNT);
            loopCounter--;
            pDesc = &pDrvCtrl->pDMARecvDesc[pDrvCtrl->emacRxIdx];

            /* restart the rx process if we reach to the end of queue */
            if (pDrvCtrl->emacRxIdx == 0)
            {
                loopCounter = 0;
                pDrvCtrl->emacRxStall = TRUE;
                break;
            }
            continue;
        }
             
        /* swap the mBlks */
        pMblk = pDrvCtrl->emacRxMblk[pDrvCtrl->emacRxIdx];
        pDrvCtrl->emacRxMblk[pDrvCtrl->emacRxIdx] = pNewMblk;
        
        pNewMblk->m_next = NULL;

        #if 0
        /* pre-invalidate the new buffer */
        cacheLsn2eSync2( pNewMblk->m_data, pNewMblk->m_len);
        #endif
        
        /* set the mBlk header up with the frame length */
        pMblk->m_len = pMblk->m_pkthdr.len = IORD_16DIRECT(&(pDesc->actual_bytes_transferred), 0) & 0xFFFF;
        pMblk->m_flags = M_PKTHDR | M_EXT;

        if(g_atseDebug )
        printf("\r\nRX len = %d", pMblk->m_len);
        /* sync the buffer with the cache */
        cacheLsn2eSync2((void*)pMblk->m_data, pMblk->m_len);

        pDesc = &pDrvCtrl->pDMARecvDesc[pDrvCtrl->emacRxIdx];

        //logMsg("\r\n status = %#x  control = %#x\n", (UINT8)(pDesc->status),(UINT8)(pDesc->control),0,0,0,0);

        pDescNext = &pDrvCtrl->pDMARecvDesc[(pDrvCtrl->emacRxIdx + 1) % EMAC_RX_DESC_CNT];        

        
//        pDesc->read_addr                = 0;
        pDesc->write_addr               = (UINT32 *)((mtod(pNewMblk, UINT32)) & 0x0fffffff);
        pDesc->next                     = (UINT32 *)((UINT32) pDescNext & 0x0fffffff);
//        pDesc->read_addr_pad            = 0x0;
//        pDesc->write_addr_pad           = 0x0;
//        pDesc->next_pad                 = 0x0;
//        pDesc->bytes_to_transfer        = 0;
//        pDesc->actual_bytes_transferred = 0;
//        pDesc->status                   = 0x0;
//        pDesc->read_burst               = 0;
//        pDesc->write_burst              = 0;

        /* pDesc->control = ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_OWNED_BY_HW_MSK; */
        
        IOWR_32DIRECT(&pDesc->actual_bytes_transferred,  0 , 
                      (UINT32) ((ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_OWNED_BY_HW_MSK  
                           |ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_GENERATE_EOP_MSK)  <<  24) );

                                       
        /* advance to the next descriptor */
        EMAC_INC_DESC(pDrvCtrl->emacRxIdx, EMAC_RX_DESC_CNT);
        loopCounter--;

        /* give the frame to the stack */
        END_RCV_RTN_CALL(&pDrvCtrl->endObj, pMblk);
        g_packNum++;
        
        pDesc = &pDrvCtrl->pDMARecvDesc[pDrvCtrl->emacRxIdx];
        desc_status = IORD_ALTERA_TSE_SGDMA_DESC_STATUS(pDesc);

        #if 0  
        /* restart the rx process if we reach to the end of queue */
        if (0) //pDrvCtrl->emacRxIdx == 0)
        {
            pDrvCtrl->emacRxStall = TRUE;
            loopCounter = 0;
            break;
        }
        #endif
    }

    if(!(IORD_ALTERA_AVALON_SGDMA_STATUS(pDrvCtrl->pSgdma1RxReg) & ALTERA_AVALON_SGDMA_STATUS_BUSY_MSK))
    {
        pDesc = &pDrvCtrl->pDMARecvDesc[pDrvCtrl->emacRxIdx];

        desc_status = IORD_ALTERA_TSE_SGDMA_DESC_STATUS(pDesc);
        while ( desc_status & ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_TERMINATED_BY_EOP_MSK  )
        {

            if( (desc_status & 
                ( ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_CRC_MSK | 
                  ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_PARITY_MSK | 
                  ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_OVERFLOW_MSK |
                  ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_SYNC_MSK | 
                  ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_UEOP_MSK | 
                  ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_MEOP_MSK | 
                  ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_MSOP_MSK ) ) != 0 )
            {
                logMsg("%s%d: bad packet, flag: %x (%p %d)\n", (int)ATSEMAC_NAME,
                        pDev->unitNumber, desc_status, (int)pDesc,
                        pDrvCtrl->emacRxIdx, 0);
                        
                goto skip2;
            }

            /* Try to conjure up a new mBlk tuple to hold the incoming packet.
             * If this fails, we have to skip the packet and move to the next one. */         

            pNewMblk = endPoolTupleGet(pDrvCtrl->endObj.pNetPool);
            if (pNewMblk == NULL)
            {
                logMsg("%s%d: out of mBlks at %d\n", (int)ATSEMAC_NAME,
                        pDev->unitNumber, pDrvCtrl->emacRxIdx,0,0,0);
                        
                pDrvCtrl->emacLastError.errCode = END_ERR_NO_BUF;
                muxError(&pDrvCtrl->endObj, &pDrvCtrl->emacLastError);

            skip2:
                bsp_printf("get skip packet \r\n");
                
                IOWR_32DIRECT(&pDesc->actual_bytes_transferred,  0 , 
                          (UINT32) ((ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_OWNED_BY_HW_MSK 
                              |ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_GENERATE_EOP_MSK)  <<  24) );
                              

                EMAC_INC_DESC(pDrvCtrl->emacRxIdx, EMAC_RX_DESC_CNT);
                loopCounter--;
                pDesc = &pDrvCtrl->pDMARecvDesc[pDrvCtrl->emacRxIdx];

                /* restart the rx process if we reach to the end of queue */
                if (pDrvCtrl->emacRxIdx == 0)
                {
                    loopCounter = 0;
                    pDrvCtrl->emacRxStall = TRUE;
                    break;
                }
                continue;
            }
                 
            /* swap the mBlks */
            pMblk = pDrvCtrl->emacRxMblk[pDrvCtrl->emacRxIdx];
            pDrvCtrl->emacRxMblk[pDrvCtrl->emacRxIdx] = pNewMblk;
            
            pNewMblk->m_next = NULL;

            #if 0
            /* pre-invalidate the new buffer */
            cacheLsn2eSync2( pNewMblk->m_data, pNewMblk->m_len);
            #endif
            
            /* set the mBlk header up with the frame length */
            pMblk->m_len = pMblk->m_pkthdr.len = IORD_16DIRECT(&(pDesc->actual_bytes_transferred), 0) & 0xFFFF;
            pMblk->m_flags = M_PKTHDR | M_EXT;

            if(g_atseDebug )
            printf("\r\nRX len = %d", pMblk->m_len);
            /* sync the buffer with the cache */
            cacheLsn2eSync2((void*)pMblk->m_data, pMblk->m_len);

            pDesc = &pDrvCtrl->pDMARecvDesc[pDrvCtrl->emacRxIdx];

            //logMsg("\r\n status = %#x  control = %#x\n", (UINT8)(pDesc->status),(UINT8)(pDesc->control),0,0,0,0);

            pDescNext = &pDrvCtrl->pDMARecvDesc[(pDrvCtrl->emacRxIdx + 1) % EMAC_RX_DESC_CNT];        

            
    //        pDesc->read_addr                = 0;
            pDesc->write_addr               = (UINT32 *)((mtod(pNewMblk, UINT32)) & 0x0fffffff);
            pDesc->next                     = (UINT32 *)((UINT32) pDescNext & 0x0fffffff);
    //        pDesc->read_addr_pad            = 0x0;
    //        pDesc->write_addr_pad           = 0x0;
    //        pDesc->next_pad                 = 0x0;
    //        pDesc->bytes_to_transfer        = 0;
    //        pDesc->actual_bytes_transferred = 0;
    //        pDesc->status                   = 0x0;
    //        pDesc->read_burst               = 0;
    //        pDesc->write_burst              = 0;

            /* pDesc->control = ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_OWNED_BY_HW_MSK; */
            
            IOWR_32DIRECT(&pDesc->actual_bytes_transferred,  0 , 
                          (UINT32) ((ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_OWNED_BY_HW_MSK  
                               |ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_GENERATE_EOP_MSK)  <<  24) );

                                           
            /* advance to the next descriptor */
            EMAC_INC_DESC(pDrvCtrl->emacRxIdx, EMAC_RX_DESC_CNT);
            loopCounter--;

            /* give the frame to the stack */
            END_RCV_RTN_CALL(&pDrvCtrl->endObj, pMblk);
            g_packNum++;
            
            pDesc = &pDrvCtrl->pDMARecvDesc[pDrvCtrl->emacRxIdx];
            desc_status = IORD_ALTERA_TSE_SGDMA_DESC_STATUS(pDesc);

        }


        //if(IORD_ALTERA_AVALON_SGDMA_STATUS(pDrvCtrl->pSgdma1RxReg) & ALTERA_AVALON_SGDMA_STATUS_CHAIN_COMPLETED_MSK )
        {
            //bsp_printf("\r\nc2");

            #if 0
             /* Clear Run */
            IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg,   0 );

            /* Get & clear status register contents */
            IOWR_ALTERA_AVALON_SGDMA_STATUS(pDrvCtrl->pSgdma1RxReg, 0xFF);

            IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg, 0x0003c );
            #else
            /* Clear Run */
            IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg, 
                                           (IORD_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg) &
                                           ~ALTERA_AVALON_SGDMA_CONTROL_RUN_MSK) );
            
            /* Get & clear status register contents */
            IOWR_ALTERA_AVALON_SGDMA_STATUS(pDrvCtrl->pSgdma1RxReg, 0xFF);

            IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg, 
                                           (IORD_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg) |
                                           ALTERA_AVALON_SGDMA_CONTROL_RUN_MSK) );
            #endif
        }
    }
    
    #if 0
    if (0) //loopCounter == 0)
    {
        jobQueuePost(pDrvCtrl->emacJobQueue, &pDrvCtrl->emacRxJob);
        sysMacIntEnable(pDrvCtrl);
        return;
    }
    #endif  
    
    vxAtomicSet (&pDrvCtrl->emacRxPending, FALSE);
    
    /* restart the RX channel, if needed */
    if (pDrvCtrl->emacRxStall == TRUE)
    {
        UINT32 data;
        pDrvCtrl->emacRxStall = FALSE;

                
        IOWR_ALTERA_AVALON_SGDMA_NEXT_DESC_POINTER(pDrvCtrl->pSgdma1RxReg, 
                                       ((UINT32)(&pDrvCtrl->pDMARecvDesc[0]) & 0x0fffffff));


        if( IORD_ALTERA_AVALON_SGDMA_STATUS(pDrvCtrl->pSgdma1RxReg) & ALTERA_AVALON_SGDMA_STATUS_CHAIN_COMPLETED_MSK )
        {
            //bsp_printf("\r\nc2");
             /* Clear Run */
            //IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg,   0 );

            /* Get & clear status register contents */
            //IOWR_ALTERA_AVALON_SGDMA_STATUS(pDrvCtrl->pSgdma1RxReg, 0xFF);

            //IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg, 0x0003e );
            #if 1
            /* Clear Run */
            IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg, 
                                           (IORD_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg) &
                                           ~ALTERA_AVALON_SGDMA_CONTROL_RUN_MSK) );
            
            /* Get & clear status register contents */
            IOWR_ALTERA_AVALON_SGDMA_STATUS(pDrvCtrl->pSgdma1RxReg, 0xFF);

            IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg, 
                                           (IORD_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg) |
                                           ALTERA_AVALON_SGDMA_CONTROL_RUN_MSK) );
            #endif
        }

        /* Clear any (previous) status register information
        * that might occlude our error checking later. */    
        //IOWR_ALTERA_AVALON_SGDMA_STATUS(pDrvCtrl->pSgdma1RxReg, 0xFF);
        
        /* Enable SGDMA receive interrupt , RUN  */
        data = IORD_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg);

        data |= (ALTERA_AVALON_SGDMA_CONTROL_IE_DESC_COMPLETED_MSK  |
                 ALTERA_AVALON_SGDMA_CONTROL_IE_CHAIN_COMPLETED_MSK | 
                 ALTERA_AVALON_SGDMA_CONTROL_IE_GLOBAL_MSK          | 
                 ALTERA_AVALON_SGDMA_CONTROL_RUN_MSK                |
                 ALTERA_AVALON_SGDMA_CONTROL_STOP_DMA_ER_MSK  );
        data &= ~ALTERA_AVALON_SGDMA_CONTROL_STOP_DMA_ER_MSK;   
        //data &= ~ALTERA_AVALON_SGDMA_CONTROL_IE_CHAIN_COMPLETED_MSK;  /* added by mxl for bug fixed 201201015 */
        //data &= ~ALTERA_AVALON_SGDMA_CONTROL_RUN_MSK; 

        //IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg, data);   
        //data |=ALTERA_AVALON_SGDMA_CONTROL_RUN_MSK; 
        //IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma1RxReg, data);  

    }
    sysMacIntEnable(pDrvCtrl);
    return;
}


LOCAL void atsemacEndTxHandle( void * pArg )
{
    QJOB *pJob;
    ATSEMACEND *pDrvCtrl;
    VXB_DEVICE_ID pDev;
    
    ATSE_SGDMA_DESCRIPTOR * pDesc;
    M_BLK_ID pMblk;    
    BOOL restart = FALSE;

    UINT8 sgdma_status;

    pJob = pArg;
    pDrvCtrl = member_to_object (pJob, ATSEMACEND, emacTxJob);
    pDev = pDrvCtrl->pDev;
    END_TX_SEM_TAKE(&pDrvCtrl->endObj, WAIT_FOREVER);
    
    /* get first BD to process */
    while (pDrvCtrl->emacTxFree < EMAC_TX_DESC_CNT)
    {
        pDesc = &pDrvCtrl->pDMASendDesc[pDrvCtrl->emacTxCons];


        #if 0
        if (IORD_ALTERA_TSE_SGDMA_DESC_STATUS(pDesc) & ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_OWNED_BY_HW_MSK)
            break;
        #endif
        sgdma_status = IORD_ALTERA_AVALON_SGDMA_STATUS(pDrvCtrl->pSgdma0TxReg);

        if (sgdma_status & ALTERA_AVALON_SGDMA_STATUS_CHAIN_COMPLETED_MSK)
        {
            if (pDesc->next)
            {
                pDrvCtrl->emacTxCnt = 0;
                IOWR_ALTERA_AVALON_SGDMA_NEXT_DESC_POINTER(pDrvCtrl->pSgdma0TxReg, ((UINT32)pDesc->next & 0x0fffffff));
            }
        }

        /* return the mblk */
        pMblk = pDrvCtrl->emacTxMblk[pDrvCtrl->emacTxCons];

        if (pMblk != NULL)
        {
            endPoolTupleFree (pMblk);
            pDrvCtrl->emacTxMblk[pDrvCtrl->emacTxCons] = NULL;
        }

        #if 0
        if(g_pAlignedMem[pDrvCtrl->emacTxProd])
        {
            free((void*)g_pAlignedMem[pDrvCtrl->emacTxProd]);
            g_pAlignedMem[pDrvCtrl->emacTxProd] = NULL;
        }
        #endif

        pDrvCtrl->emacTxFree++;

        EMAC_INC_DESC(pDrvCtrl->emacTxCons, EMAC_TX_DESC_CNT);

        /*
         * We released at least one descriptor: if the transmit
         * channel is stalled, unstall it.
         */

        if (pDrvCtrl->emacTxStall == TRUE)
        {
            pDrvCtrl->emacTxStall = FALSE;
            restart = TRUE;
        }
    }

    /*
     * If we reclaimed all descriptors, then the channel is
     * implicitly idle. Make sure to reset the TX count here.
     * This handles the case where a bunch of traffic has
     * completed with no new traffic queued.
     */

    if (pDrvCtrl->emacTxFree == EMAC_TX_DESC_CNT)
        pDrvCtrl->emacTxCnt = 0;

    END_TX_SEM_GIVE(&pDrvCtrl->endObj);

    pDrvCtrl->emacTxPending = FALSE;

    if (restart == TRUE)
        muxTxRestart(pDrvCtrl);

    return;
}


LOCAL int atsemacEndEncap ( ATSEMACEND * pDrvCtrl,  M_BLK_ID  pMblk )
{
    ATSE_SGDMA_DESCRIPTOR * pDesc = NULL, * pFirst = NULL;    
    M_BLK_ID pCurr;
    UINT32 firstIdx=0, lastIdx=0;
    int used = 0, i;
    ATSE_SGDMA_DESCRIPTOR  * pPrev , *pTemp;

    INT32  control;
    char  *pm_data;
   
    UINT8   result = 0;


    control = IORD_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma0TxReg);
    control &= ~ALTERA_AVALON_SGDMA_CONTROL_RUN_MSK;
    IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma0TxReg, control);

    firstIdx = pDrvCtrl->emacTxProd;
    
    pFirst = &pDrvCtrl->pDMASendDesc[pDrvCtrl->emacTxProd];
    
    for (pCurr = pMblk; pCurr != NULL; pCurr = pCurr->m_next)
    {
        if (pCurr->m_len != 0)
        {
            if (used == EMAC_MAXFRAG || pDrvCtrl->emacTxFree == 0)
                break;

            pDesc = &pDrvCtrl->pDMASendDesc[pDrvCtrl->emacTxProd];

            #if 0
            if (IORD_ALTERA_TSE_SGDMA_DESC_STATUS(pDesc) & ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_OWNED_BY_HW_MSK)
                break;
            #endif
                
            pPrev = pDesc;
            
            pm_data = (char*)(pCurr->m_data);
            i = ((UINT32)(pm_data) & 0x03);
            
            if(0 != i)
            {
                //bsp_printf("not alignd\r\n"); 
                //g_pAlignedMem[pDrvCtrl->emacTxProd] = (char *)memalign(4, 1560);
                //bzero(g_pAlignedMem , 1560);
                 
                /*printf("g_pAlignedMem[pDrvCtrl->emacTxProd]= %#x\r\n" ,g_pAlignedMem[pDrvCtrl->emacTxProd]);*/
                //printf("g_pAlignedMem %#x\r\n", g_pAlignedMem[pDrvCtrl->emacTxProd]); 
                #if 0  /* 20120904 mxl */
                for(cnt = 0 ; cnt < pCurr->m_len; cnt++ )
                {
                    g_pAlignedMem[pDrvCtrl->emacTxProd][cnt] = pm_data[cnt];
                }
                #else
                bcopy((char *)pm_data,(char *)g_pAlignedMem[pDrvCtrl->emacTxProd] , pCurr->m_len);
                #endif

                pDesc->read_addr = (UINT32*)((UINT32)(g_pAlignedMem[pDrvCtrl->emacTxProd]) & 0x0fffffff );
                
                /* sync the data buffer */
                cacheLsn2eSync((void*)g_pAlignedMem[pDrvCtrl->emacTxProd] , pCurr->m_len);
            }
            else
            {
                pDesc->read_addr = (UINT32*)(((UINT32)(pCurr->m_data)) & 0x0fffffff );
                
                /* sync the data buffer */
                cacheLsn2eSync((void*)pCurr->m_data, pCurr->m_len);

            }

            pDesc->write_addr               = 0;
            /* pDesc->next                     = (UINT32 *)((UINT32) pDescNext & 0x0fffffff); */
            pDesc->read_addr_pad            = 0x0;
            pDesc->write_addr_pad           = 0x0;
            pDesc->next_pad                 = 0x0;
            pDesc->bytes_to_transfer        = (pCurr->m_len);
            pDesc->actual_bytes_transferred = 0;
            pDesc->status                   = 0x0;
            pDesc->read_burst               = 0;
            pDesc->write_burst              = 0;

            if (used == 0)
            {
                pDesc->control |= ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_WRITE_FIXED_ADDRESS_MSK;  /* SOP */
            }
            else
            {
                pPrev->next = (UINT32 *)((UINT32) pDesc & 0x0fffffff);;
                pDesc->control |=  ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_OWNED_BY_HW_MSK ;                        
            }

            #if 0
            /* sync the data buffer */
            cacheLsn2eSync((void*)pCurr->m_data, pCurr->m_len);
            printf("send: pCurr->m_data = %x, pCurr->m_len = %x \r\n", pCurr->m_data, pCurr->m_len );
            #endif
            /* printf("send: pDesc->read_addr = %x, pCurr->m_len = %x \r\n", pDesc->read_addr, pCurr->m_len );*/

            lastIdx = pDrvCtrl->emacTxProd;
            EMAC_INC_DESC(pDrvCtrl->emacTxProd, EMAC_TX_DESC_CNT);
            used++;
            pDrvCtrl->emacTxFree--;
        }
    }

    if (pCurr != NULL)
        goto noDescs;

    pTemp = &pDrvCtrl->pDMASendDesc[pDrvCtrl->emacTxProd];
    #if 0
    pTemp->control |= ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_OWNED_BY_HW_MSK;   /* mxl: 不加这个会不断产生 chain complete */
    #endif
    pDesc->next = (UINT32 *)((UINT32) pTemp & 0x0fffffff); 
    pDesc->control |= ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_GENERATE_EOP_MSK; 

    if (pMblk->m_pkthdr.len < ETHERSMALL)
    {
        pDesc->bytes_to_transfer += (ETHERSMALL - pMblk->m_pkthdr.len);
    }

    /* save the mBlk for later */
    pDrvCtrl->emacTxMblk[lastIdx] = pMblk;

    pFirst->control |= ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_OWNED_BY_HW_MSK;

    pPrev = &pDrvCtrl->pDMASendDesc[pDrvCtrl->emacTxPend];
    EMAC_INC_DESC(pDrvCtrl->emacTxPend, EMAC_TX_DESC_CNT);

    /* 20120904 mxl */
    if (pDrvCtrl->emacTxCnt == 0 || IORD_ALTERA_AVALON_SGDMA_NEXT_DESC_POINTER(pDrvCtrl->pSgdma0TxReg) == 0)  
    {

        /* Clear Run */
        IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma0TxReg,
                                       (IORD_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma0TxReg) &
                                       ~ALTERA_AVALON_SGDMA_CONTROL_RUN_MSK) );

        /*
        * Clear any (previous) status register information
        * that might occlude our error checking later.
        */
        IOWR_ALTERA_AVALON_SGDMA_STATUS(pDrvCtrl->pSgdma0TxReg, 0xFF);
        IOWR_ALTERA_AVALON_SGDMA_NEXT_DESC_POINTER(pDrvCtrl->pSgdma0TxReg, ((UINT32)pFirst & 0x0fffffff));
        
        
    }
    else
    {
        pDrvCtrl->emacTxCnt++;
        pPrev->next = (UINT32 *)((UINT32) pFirst & 0x0fffffff);  
    }

    IOWR_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma0TxReg,
                                        (ALTERA_AVALON_SGDMA_CONTROL_IE_DESC_COMPLETED_MSK  |
                                        ALTERA_AVALON_SGDMA_CONTROL_IE_CHAIN_COMPLETED_MSK |
                                        ALTERA_AVALON_SGDMA_CONTROL_IE_GLOBAL_MSK |
                                        ALTERA_AVALON_SGDMA_CONTROL_RUN_MSK |
                                        ALTERA_AVALON_SGDMA_CONTROL_STOP_DMA_ER_MSK | 
                                        IORD_ALTERA_AVALON_SGDMA_CONTROL(pDrvCtrl->pSgdma0TxReg)) );
    return (OK);

noDescs:

    /*
     * Ran out of descriptors: undo all relevant changes
     * and fall back to copying.
     */

    pDrvCtrl->emacTxProd = firstIdx;

    for (i = 0; i < used; i++)
    {
        pDrvCtrl->emacTxFree++;
        pDesc = &pDrvCtrl->pDMASendDesc[pDrvCtrl->emacTxProd];
        pDesc->next = 0;
        pDesc->control = 0;
        pDesc->read_addr = 0;
        pDesc->bytes_to_transfer = 0;
        EMAC_INC_DESC(pDrvCtrl->emacTxProd, EMAC_TX_DESC_CNT);
    }

    pDrvCtrl->emacTxProd = firstIdx;

    return (ENOSPC);
    
}

/*******************************************************************************
*
* atsemacEndSend - transmit a packet
*
* This function transmits the packet specified in <pMblk>.
*
* RETURNS: OK, ERROR, or END_ERR_BLOCK.
*
* ERRNO: N/A
*/

LOCAL int atsemacEndSend( END_OBJ *   pEnd,  M_BLK_ID    pMblk )
{
    ATSEMACEND * pDrvCtrl;
    VXB_DEVICE_ID pDev;
    M_BLK_ID pTmp;
    int rval;


    pDrvCtrl = (ATSEMACEND *)pEnd;

    if (pDrvCtrl->emacPolling == TRUE)
    {
        endPoolTupleFree(pMblk);
        return (ERROR);
    }    

    pDev = pDrvCtrl->pDev;
    END_TX_SEM_TAKE(pEnd, WAIT_FOREVER);

    if (!pDrvCtrl->emacTxFree || !(pDrvCtrl->emacCurStatus & IFM_ACTIVE))
        goto blocked;    

    if( pMblk->m_next != NULL)
    {
        bsp_printf("atsemacEndSend pMblk->m_next != NULL \r\n");
        if ((pTmp = endPoolTupleGet (pDrvCtrl->endObj.pNetPool)) == NULL)
                goto blocked;

        pTmp->m_len = pTmp->m_pkthdr.len =
            netMblkToBufCopy (pMblk, mtod(pTmp, char *), NULL);
        pTmp->m_flags = pMblk->m_flags;
        endPoolTupleFree (pMblk);
    }
    else
    {
        pTmp = pMblk;
    }

    rval = atsemacEndEncap(pDrvCtrl, pTmp);

    if (rval != OK)
        goto blocked;
    

    if (rval != OK)
        goto blocked;
    
    END_TX_SEM_GIVE(pEnd);
    
    return (OK);

blocked:

    pDrvCtrl->emacTxStall = TRUE;
    END_TX_SEM_GIVE(pEnd);

    return (END_ERR_BLOCK);

}


LOCAL STATUS atsemacEndPollSend
    (
    END_OBJ *   pEnd,
    M_BLK_ID    pMblk
    )
    {

    bsp_printf("atsemacEndPollSend : enter 1 \r\n" );
    #if 0
    ATSEMACEND * pDrvCtrl;
    VXB_DEVICE_ID pDev;
    EMAC_DESC * pDesc;
    M_BLK_ID pTmp;
    int len, i;

    pDrvCtrl = (ATSEMACEND *)pEnd;

    if (pDrvCtrl->emacPolling == FALSE)
        return (ERROR);

    pDev = pDrvCtrl->pDev;
    pTmp = pDrvCtrl->emacPollBuf;

    len = netMblkToBufCopy(pMblk, mtod(pTmp, char *), NULL);
    pTmp->m_len = pTmp->m_pkthdr.len = len;
    pTmp->m_flags = pMblk->m_flags;

    if (atsemacEndEncap(pDrvCtrl, pTmp) != OK)
        return (EAGAIN);

    /* poll the status to see if the transmission is done */

    for (i = 0; i < EMAC_TIMEOUT; i++)
        if (CSR2_READ_4(pDrvCtrl->emacDev, EMAC_TX0_HDP) == 0)
            break;

    if (i == EMAC_TIMEOUT)
        return (ERROR);

    pDrvCtrl->emacTxMblk[pDrvCtrl->emacTxCons] = NULL;

    pDesc = &pDrvCtrl->emacTxDescMem[pDrvCtrl->emacTxCons];

    /* ack the tx bd */

    CSR2_WRITE_4(pDrvCtrl->emacDev, EMAC_TX0_CP, (UINT32)pDesc);

    pDesc->next = 0;
    pDesc->buffer = 0;
    pDesc->buflen_off = 0;
    pDesc->pktlen_flags = 0;
    pDrvCtrl->emacTxFree++;
    EMAC_INC_DESC(pDrvCtrl->emacTxCons, EMAC_TX_DESC_CNT);
    #endif
    return (OK);
    }

/*******************************************************************************
*
* atsemacEndPollReceive - polled mode receive routine
*
* This function receive a packet in polled mode, with interrupts disabled.
* It's similar in operation to the atsemacEndRxHandle() routine, except it
* doesn't process more than one packet at a time and does not load out buffers.
* Instead, the caller supplied an mBlk tuple into which this function will
* place the received packet.
*
* If no packet is available, this routine will return EAGAIN.
* If the supplied mBlk is too small to contain the received frame,
* the routine will return ERROR.
*
* RETURNS: OK, EAGAIN, or ERROR
*
* ERRNO: N/A
*/

LOCAL int atsemacEndPollRecv
    (
    END_OBJ *   pEnd,
    M_BLK_ID    pMblk
    )
    {
    #if 0
    ATSEMACEND * pDrvCtrl;
    VXB_DEVICE_ID pDev;
    EMAC_DESC * pDesc;
    M_BLK_ID pPkt;
    int rval = EAGAIN;

    pDrvCtrl = (ATSEMACEND *)pEnd;

    if (pDrvCtrl->emacPolling == FALSE)
        return (ERROR);

    if (!(pMblk->m_flags & M_EXT))
        return (ERROR);

    pDev = pDrvCtrl->pDev;

    pDesc = &pDrvCtrl->emacRxDescMem[pDrvCtrl->emacRxIdx];
    pPkt = pDrvCtrl->emacRxMblk[pDrvCtrl->emacRxIdx];

    /* restart the RX channel, if needed */

    if (pDrvCtrl->emacRxIdx == 0 && pDrvCtrl->emacRxStall == FALSE)
        {
        pDrvCtrl->emacRxStall = TRUE;
        CSR2_WRITE_4(pDrvCtrl->emacDev, EMAC_RX0_HDP, (UINT32)pDesc);
        }

    if (pDesc->pktlen_flags & EMAC_RBD_OWNER)
        return (EAGAIN);

    if (pMblk->m_len < (pDesc->pktlen_flags & 0xFFFF))
        return (ERROR);

    if (pDesc->pktlen_flags & EMAC_RBD_ERR)
        rval = ERROR;
    else
        {
        pMblk->m_flags |= M_PKTHDR;
        pMblk->m_len = pMblk->m_pkthdr.len = pDesc->pktlen_flags & 0xFFFF;
        EMAC_ADJ(pMblk);

        /* sync the DMA buffer */

        cacheInvalidate(DATA_CACHE,
            pPkt->m_data + EMAC_ETHER_ALIGN, pMblk->m_len);

        bcopy(mtod(pPkt, char *) + EMAC_ETHER_ALIGN,
            mtod(pMblk, char *), pMblk->m_len);

        cacheInvalidate(DATA_CACHE, pPkt->m_data, pPkt->m_len);

        rval = OK;
        }

    /* ack the rx bd */

    CSR2_WRITE_4(pDrvCtrl->emacDev, EMAC_RX0_CP, (UINT32)pDesc);

    /* reset the rx descriptor */

    pDesc->buflen_off = EMAC_CLSIZE;
    pDesc->pktlen_flags = EMAC_RBD_OWNER;

    EMAC_INC_DESC(pDrvCtrl->emacRxIdx, EMAC_RX_DESC_CNT);

    pDrvCtrl->emacRxStall = FALSE;

    return (rval);
    #endif
    return (OK);
}

#if 0  /* 后面再实现 */
/*******************************************************************************
*
* atsemacMacHashClac - calculate a hash checksum
*
* This routine performs the Davinci EMAC hash calculation over MAC addresses.
* The EMAC implement multicast filtering using a hash table where a hash
* checksum of the multicast group address is used as the table index.
*
* RETURNS: the 32-bit checksum of the supplied buffer
*
* ERRNO: N/A
*/

UINT32 atsemacMacHashClac(const UINT8 * pBuf )
{
    UINT32 hash = 0;
    UINT8 bit[6];
    int i;

    bit[0] = ((pBuf[0] >> 2) ^ (pBuf[1] >> 4) ^ (pBuf[2] >> 6) ^ (pBuf[2] >> 0)
            ^ (pBuf[3] >> 2) ^ (pBuf[4] >> 4) ^ (pBuf[5] >> 6) ^ (pBuf[5] >> 0))
            & 1;
    bit[1] = ((pBuf[0] >> 3) ^ (pBuf[1] >> 5) ^ (pBuf[2] >> 7) ^ (pBuf[2] >> 1)
            ^ (pBuf[3] >> 3) ^ (pBuf[4] >> 5) ^ (pBuf[5] >> 7) ^ (pBuf[5] >> 1))
            & 1;
    bit[2] = ((pBuf[0] >> 4) ^ (pBuf[1] >> 6) ^ (pBuf[1] >> 0) ^ (pBuf[2] >> 2)
            ^ (pBuf[3] >> 4) ^ (pBuf[4] >> 6) ^ (pBuf[4] >> 0) ^ (pBuf[5] >> 2))
            & 1;
    bit[3] = ((pBuf[0] >> 5) ^ (pBuf[1] >> 7) ^ (pBuf[1] >> 1) ^ (pBuf[2] >> 3)
            ^ (pBuf[3] >> 5) ^ (pBuf[4] >> 7) ^ (pBuf[4] >> 1) ^ (pBuf[5] >> 3))
            & 1;
    bit[4] = ((pBuf[0] >> 6) ^ (pBuf[0] >> 0) ^ (pBuf[1] >> 2) ^ (pBuf[2] >> 4)
            ^ (pBuf[3] >> 6) ^ (pBuf[3] >> 0) ^ (pBuf[4] >> 2) ^ (pBuf[5] >> 4))
            & 1;
    bit[5] = ((pBuf[0] >> 7) ^ (pBuf[0] >> 1) ^ (pBuf[1] >> 3) ^ (pBuf[2] >> 5)
            ^ (pBuf[3] >> 7) ^ (pBuf[3] >> 1) ^ (pBuf[4] >> 3) ^ (pBuf[5] >> 5))
            & 1;

    for (i = 0; i < 6; i++)
        if (bit[i])
            hash += (1 << i);

    return hash;
}

/*******************************************************************************
*
* atsemacEndHashTblPopulate - populate the multicast hash filter
*
* This function programs the EMAC controller's multicast hash filter to receive
* frames sent to the multicast groups specified in the multicast address list
* attached to the END object.  If the interface is in IFF_ALLMULTI mode, the
* filter will be programmed to receive all multicast packets by setting all the
* bits in the hash table to one.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

LOCAL void atsemacEndHashTblPopulate ( ATSEMACEND * pDrvCtrl )
{
    VXB_DEVICE_ID pDev;
    UINT32 h;
    UINT32 hashes[2] = { 0, 0 };
    ETHER_MULTI * mCastNode = NULL;

    pDev = pDrvCtrl->pDev;

    if (pDrvCtrl->endObj.flags & IFF_ALLMULTI)
        {
        /* set all multicast mode */

        CSR2_WRITE_4(pDev, EMAC_MAC_HASH1, 0xFFFFFFFF);
        CSR2_WRITE_4(pDev, EMAC_MAC_HASH2, 0xFFFFFFFF);
        return;
        }

    /* first, clear out the original filter */

    CSR2_WRITE_4(pDev, EMAC_MAC_HASH1, 0);
    CSR2_WRITE_4(pDev, EMAC_MAC_HASH2, 0);

    /* now repopulate it */

    for (mCastNode = (ETHER_MULTI *)lstFirst(&pDrvCtrl->emacEndObj.multiList);
         mCastNode != NULL;
         mCastNode = (ETHER_MULTI *)lstNext(&mCastNode->node))
        {
        h = atsemacMacHashClac((const UINT8 *)mCastNode->addr);

        if (h < 32)
            hashes[0] |= (1 << h);
        else
            hashes[1] |= (1 << (h - 32));
        }

    /* reload filter */

    CSR2_WRITE_4(pDev, EMAC_MAC_HASH1, hashes[0]);
    CSR2_WRITE_4(pDev, EMAC_MAC_HASH2, hashes[1]);
}
#endif


/*******************************These code add into hwconf.c in your bsp*******************
#include "vxbAlteraTse.h"

.....
const struct hcfResource atsemac0Resources[] = {
	    { "deviceId", HCF_RES_INT, { (void *)(ATSEMAC_UNIT0_DEVICE_ID) } },
	    { "regBase", HCF_RES_INT, { (void *)(0) } },
	    { "intr0", HCF_RES_INT, { (void *)(0) } },
	    { "intr0Level", HCF_RES_INT, { (void *)(0) } },        
	    { "cacheLineSize", HCF_RES_INT, { (void *)(_CACHE_ALIGN_SIZE) } },
	    { "mtuSize", HCF_RES_INT, { (void *)(1500) } },
	    { "bufferAddr", HCF_RES_INT, { (void *)(0) } },
	    { "bufferSize", HCF_RES_INT, { (void *)(0) } },
	    { "bdAddr", HCF_RES_INT, { (void *)(0) } },
	    { "bdSize", HCF_RES_INT, { (void *)(0) } },
	    { "rxBdCount", HCF_RES_INT, { (void *)(0) } },
	    { "txBdCount", HCF_RES_INT, { (void *)(0) } },
};

#define atsemac0Num NELEMENTS(atsemac0Resources)
....

in const struct hcfDevice hcfDeviceList[] = {
		....
		{ATSEMAC_CARD_VXBNAME,ATSEMAC_UNIT0_DEVICE_ID,VXB_BUSID_PLB,0,atsemac0Num,atsemac0Resources},
		....
		}

******************************************************************************************/

/****************************These code add into syslib.c in your bsp*********************
....
IMPORT void atsemacRegister(void);
....
in sysHwInit()
{
.....
	atsemacRegister();
.....
}
******************************************************************************************/


