#ifndef hrMatrixDam_h_20111117
#define hrMatrixDma_h_20111117
#ifdef __cplusplus
extern "C"{
#endif
#define MATRIX_DMA_USE_CACHE_TEMP

#define MASK_IQ_64K  0xffff0000
#define MASK_IQ_16K  0xffffc000
#define MASK_IQ_4K   0xfffff000
#define MASK_IQ_1K   0xfffffc00
#define MASK_IQ_256  0xffffff00
#define MASK_IQ_64   0xffffffc0
#define MASK_IQ_16   0xfffffff0
#define DATA_IQ_64K  0x00010000
#define DATA_IQ_16K  0x00004000
#define DATA_IQ_4K   0x00001000
#define DATA_IQ_1K   0x00000400
#define DATA_IQ_256  0x00000100
#define DATA_IQ_64   0x00000040
#define DATA_IQ_16   0x00000010
#define DATA_RC_64K  0x00000100
#define DATA_RC_16K  0x00000080
#define DATA_RC_4K   0x00000040
#define DATA_RC_1K   0x00000020
#define DATA_RC_256  0x00000010
#define DATA_RC_64   0x00000008
#define DATA_RC_16   0x00000004

#define MATRIX_DMA_MAX_64Bit  DATA_IQ_64K    /*64K64bit, 512KB*/
#define MATRIX_DMA_MIN_64Bit  DATA_IQ_1K    /*256x64bit, 2KB*/

#define  DMA1_SRC_ADDR      0x3ff00700  /*源矩阵起始地址*/
#define  DMA1_DST_ADDR      0x3ff00708  /*目标矩阵起始地址*/
#define  DMA1_SRC_ROW       0x3ff00710  /*源矩阵的一行元素个数*/
#define  DMA1_DST_COL       0x3ff00718  /*源矩阵的一列元素个数*/
#define  DMA1_SRC_LENGTH    0x3ff00720  /*源矩阵所在大矩阵的行跨度(字节)*/
#define  DMA1_DST_LENGTH    0x3ff00728  /*目标矩阵所大大矩阵的行跨度*/
#define  DMA1_TRANS_CTRL    0x3ff00730  /*转置控制寄存器*/
#define  DMA1_TRANS_STATUS  0x3ff00738  /*转置状态寄存器*/

#define  DMA0_SRC_ADDR      0x3ff00600  /*源矩阵起始地址*/
#define  DMA0_DST_ADDR      0x3ff00608  /*目标矩阵起始地址*/
#define  DMA0_SRC_ROW       0x3ff00610  /*源矩阵的一行元素个数*/
#define  DMA0_DST_COL       0x3ff00618  /*源矩阵的一列元素个数*/
#define  DMA0_SRC_LENGTH    0x3ff00620  /*源矩阵所在大矩阵的行跨度(字节)*/
#define  DMA0_DST_LENGTH    0x3ff00628  /*目标矩阵所大大矩阵的行跨度*/
#define  DMA0_TRANS_CTRL    0x3ff00630  /*转置控制寄存器*/
#define  DMA0_TRANS_STATUS  0x3ff00638  /*转置状态寄存器*/

#define  DMA_ENABLE         1            /*1--enable*/
#define  DMA_DISABLE      0xfffffffc
#define  DMA_START          0x2          /*是否允许写目标矩阵,我理解为是否开始传输*/
#define  DMA_SRC_INT_VALID  0x4          /*源矩阵读取完毕后，是否有效中断*/
#define  DMA_DST_INT_VALID  0x8          /*目标矩阵写入完毕后，是否有效中断*/
#define  DMA_ARCMD          0xc0         /*读命令内部控制位,DMA_ARCHE为0xf00时有效*/
#define  DMA_ARCHE          0xf00        /*读命令内部控制位，读使用cache通道*/
#define  DMA_ARUNCHE        0x0          /*读命令内部控制位，读使用uncache通道*/
#define  DMA_AWCMD          0xb000       /*写命令内部控制位*/
#define  DMA_AWCHE          0xf0000      /*写命令内部控制位，写使用cache通道*/
#define  DMA_AWUNCHE        0x0          /*写命令内部控制位，写使用uncache通道*/
#define  DMA_TRANS_1BITS    0x0          /*矩阵元素大小为1个字节*/
#define  DMA_TRANS_2BITS    0x100000     /*矩阵元素大小为2个字节*/
#define  DMA_TRANS_4BITS    0x200000     /*矩阵元素大小为4个字节*/
#define  DMA_TRANS_8BITS    0x300000     /*矩阵元素大小为8个字节*/
#define  DMA_TRANS_TURN     0x400000

#define  DMA_READ_FINISHED  1             /*目标矩阵读取完毕*/
#define  DMA_TRANS_FINISHED 2             /*目标矩阵写入完毕*/

	
/*add by wangzx 20111116*/
//#ifndef BOOTROM
#define HRMATRIXDMA0_DEVICE
#define HRMATRIXDMA1_DEVICE
//#endif

#define HRMATRIXDMA0_VXBNAME "hrmatrixdma0dev"
#define HRMATRIXDMA0_PIN 8
#define HRMATRIXDMA0_CORE_PIN 2  /* 2*/
#define HRMATRIXDMA0_CORE_NUM 0
#define HRMATRIXDMA0_DEVICE_DESC \
{ HRMATRIXDMA0_VXBNAME,0,VXB_BUSID_PLB,0,hrMatrixDma0Num,hrMatrixDma0Resources}
#define HRMATRIXDMA0_INT_DESC {HRMATRIXDMA0_PIN,HRMATRIXDMA0_VXBNAME,0,0}
#define HRMATRIXDMA0_XBAR_DESC {HRMATRIXDMA0_PIN,HRMATRIXDMA0_CORE_PIN}
#define HRMATRIXDMA0_ROUTE_DESC {HRMATRIXDMA0_PIN,HRMATRIXDMA0_CORE_NUM}
	
#define HRMATRIXDMA1_VXBNAME "hrmatrixdma1dev"
#define HRMATRIXDMA1_PIN 9
#define HRMATRIXDMA1_CORE_PIN 1
#define HRMATRIXDMA1_CORE_NUM 0
#define HRMATRIXDMA1_DEVICE_DESC \
{ HRMATRIXDMA1_VXBNAME,0,VXB_BUSID_PLB,0,hrMatrixDma1Num,hrMatrixDma1Resources}
#define HRMATRIXDMA1_INT_DESC {HRMATRIXDMA1_PIN,HRMATRIXDMA1_VXBNAME,0,0}
#define HRMATRIXDMA1_XBAR_DESC {HRMATRIXDMA1_PIN,HRMATRIXDMA1_CORE_PIN}
#define HRMATRIXDMA1_ROUTE_DESC {HRMATRIXDMA1_PIN,HRMATRIXDMA1_CORE_NUM}
	

/*this lines copy to sysLib.c*/
/*
#ifdef HRMATRIXDMA0_DEVICE
	hrMatrixdma0Register();
#endif
#ifdef HRMATRIXDMA1_DEVICE
	hrMatrixdma1Register();
#endif
*/
/*add by wangzx 20111116 end*/
	
	
#ifdef __cplusplus
}
#endif	
	
#endif/*zrMatrixDma_h_20111117*/

