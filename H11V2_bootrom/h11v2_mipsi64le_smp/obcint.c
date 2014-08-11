
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
#include <stdio.h>
#include <stdlib.h>
#include <wdLib.h>

#include <hwif/vxbus/vxBus.h>
#include <hwif/vxbus/hwConf.h>
#include <hwif/vxbus/vxbPlbLib.h>
#include <hwif/util/vxbParamSys.h>
#include <../src/hwif/h/vxbus/vxbAccess.h>

extern void printstr(char *s);
extern UINT32 bspTempRead(int times);
extern unsigned int sysXRD32(unsigned int addrh,unsigned int addrl);
unsigned int g_intStatus;
 static void obcInt(VXB_DEVICE_ID pDev);

 SEM_ID semInt0,semInt1,semInt2,semInt3; 
 
typedef struct
{
	VXB_DEVICE_ID pDev;			 	/* vxbus 要求的项目*/
	unsigned int intSource;			/* intSource */
	unsigned int intLine;			/* intline of core*/
	unsigned int coreNum;			/* core Num */
}OBCINT;  


 void obc0InstInit(struct vxbDev * pDev);
 void obc0InstInit2(struct vxbDev * pDev);
 void obc0InstConnect(struct vxbDev * pDev);
 BOOL obc0Probe(struct vxbDev * pDev);
 static void obc0Int(VXB_DEVICE_ID pDev);

 struct drvBusFuncs obc0Funcs =
{
	obc0InstInit,     /* devInstanceInit */
	obc0InstInit2,    /* devInstanceInit2 */
	obc0InstConnect   /* devConnect */
};
 device_method_t obc0Methods[] ={{0, 0}};
 struct vxbPlbRegister obc0Registration =
{
    {
        NULL,                 /* pNext,一般都用NULL */
        VXB_DEVID_DEVICE,     /* devID,固定,表示这是一个设备 */
        VXB_BUSID_PLB,        /* busID = PLB ,表示这个设备挂接在PLB上*/
        VXBUS_VERSION_4,      /* 版本号,6.7为版本4 */
        "obcintdev0",     /*设备名称,必须和hwconfig.c中统一*/
        &obc0Funcs,       		/* pDrvBusFuncs,设备总线函数组在后面定义 */
        obc0Methods,     		/* pMethods ,设备方法组,在后面定义*/
        obc0Probe         		/* devProbe ,设备探测*/
    }
};
 void obc0Register(void){
	vxbDevRegister((struct vxbDevRegInfo *)&(obc0Registration));
   return;
}
 void obc0InstInit (struct vxbDev * pDev)
{
    vxbNextUnitGet(pDev);
    return;
}
 void obc0InstInit2(struct vxbDev * pDev)
{
	OBCINT *pDrvCtrl;
	
	semInt0=semBCreate(SEM_Q_FIFO, SEM_EMPTY);
//	intNumEn(0);
	
    pDrvCtrl = malloc(sizeof(OBCINT));
    if (pDrvCtrl == NULL) return;
    bzero ((char *)pDrvCtrl, sizeof(OBCINT));
    pDev->pDrvCtrl = pDrvCtrl;
    pDrvCtrl->pDev = pDev;
    return;
}
 void obc0InstConnect(struct vxbDev * pDev)
{
	if(vxbIntConnect(pDev,0,obcInt,(void *)pDev) != OK)
	{
		printstr("obc0InstConnect: vxbIntConnect Fail\r\n");
	}
	else
	{
		printstr("obc0InstConnect: vxbIntConnect OK\r\n");
	}
	if(vxbIntEnable(pDev,0,obcInt,(void *)pDev) != OK)
	{
		printstr("obc0InstConnect: vxbIntEnable Fail\r\n");
	}
	else
	{
		printstr("obc0InstConnect: vxbIntEnable OK\r\n");
	}
}

 

 BOOL obc0Probe(struct vxbDev * pDev){return TRUE;}
 static void obc0Int(VXB_DEVICE_ID pDev)
{
//	 *(int *)0xbe0000c0=1;
//	 bsp_printf("obc int0 ok\r\n");
	 semGive(semInt0);
	 
}
 

 void obc1InstInit(struct vxbDev * pDev);
 void obc1InstInit2(struct vxbDev * pDev);
 void obc1InstConnect(struct vxbDev * pDev);
 BOOL obc1Probe(struct vxbDev * pDev);
 static void obc1Int(VXB_DEVICE_ID pDev);

 struct drvBusFuncs obc1Funcs =
{
	obc1InstInit,     /* devInstanceInit */
	obc1InstInit2,    /* devInstanceInit2 */
	obc1InstConnect   /* devConnect */
};
 device_method_t obc1Methods[] ={{0, 0}};
 struct vxbPlbRegister obc1Registration =
{
    {
        NULL,                 /* pNext,一般都用NULL */
        VXB_DEVID_DEVICE,     /* devID,固定,表示这是一个设备 */
        VXB_BUSID_PLB,        /* busID = PLB ,表示这个设备挂接在PLB上*/
        VXBUS_VERSION_4,      /* 版本号,6.7为版本4 */
        "obcintdev1",     /*设备名称,必须和hwconfig.c中统一*/
        &obc1Funcs,       		/* pDrvBusFuncs,设备总线函数组在后面定义 */
        obc1Methods,     		/* pMethods ,设备方法组,在后面定义*/
        obc1Probe         		/* devProbe ,设备探测*/
    }
};
 void obc1Register(void){
	vxbDevRegister((struct vxbDevRegInfo *)&(obc1Registration));
   return;
}

 void obc1InstInit (struct vxbDev * pDev)
{
    vxbNextUnitGet(pDev);
    return;
}
 void obc1InstInit2(struct vxbDev * pDev)
{
	OBCINT *pDrvCtrl;
	
	semInt1=semBCreate(SEM_Q_FIFO, SEM_EMPTY);
	
    pDrvCtrl = malloc(sizeof(OBCINT));
    if (pDrvCtrl == NULL) return;
    bzero ((char *)pDrvCtrl, sizeof(OBCINT));
    pDev->pDrvCtrl = pDrvCtrl;
    pDrvCtrl->pDev = pDev;
    return;
}
 void obc1InstConnect(struct vxbDev * pDev)
{
	if(vxbIntConnect(pDev,0,obcInt,(void *)pDev) != OK)
	{
		printstr("obc1InstConnect: vxbIntConnect Fail\r\n");
	}
	else
	{
		printstr("obc1InstConnect: vxbIntConnect OK\r\n");
	}
	if(vxbIntEnable(pDev,0,obcInt,(void *)pDev) != OK)
	{
		printstr("obc1InstConnect: vxbIntEnable Fail\r\n");
	}
	else
	{
		printstr("obc1InstConnect: vxbIntEnable OK\r\n");
	}
}
 BOOL obc1Probe(struct vxbDev * pDev){return TRUE;}
 
 extern int bsp_printf(const char *  fmt, ...);
 
static void obc1Int(VXB_DEVICE_ID pDev)
{
	bsp_printf("obc int1 ok!\r\n");
	 semGive(semInt1);
}
 
 
  void obc2InstInit(struct vxbDev * pDev);
  void obc2InstInit2(struct vxbDev * pDev);
  void obc2InstConnect(struct vxbDev * pDev);
  BOOL obc2Probe(struct vxbDev * pDev);
  static void obc2Int(VXB_DEVICE_ID pDev);
 
  struct drvBusFuncs obc2Funcs =
 {
	 obc2InstInit,	   /* devInstanceInit */
	 obc2InstInit2,    /* devInstanceInit2 */
	 obc2InstConnect   /* devConnect */
 };
  device_method_t obc2Methods[] ={{0, 0}};
  struct vxbPlbRegister obc2Registration =
 {
	 {
		 NULL,				   /* pNext,一般都用NULL */
		 VXB_DEVID_DEVICE,	   /* devID,固定,表示这是一个设备 */
		 VXB_BUSID_PLB, 	   /* busID = PLB ,表示这个设备挂接在PLB上*/
		 VXBUS_VERSION_4,	   /* 版本号,6.7为版本4 */
		 "obcintdev2",	 /*设备名称,必须和hwconfig.c中统一*/
		 &obc2Funcs,			 /* pDrvBusFuncs,设备总线函数组在后面定义 */
		 NULL,			 /* pMethods ,设备方法组,在后面定义*/
		 obc2Probe				 /* devProbe ,设备探测*/
	 }
 };
  void obc2Register(void){
	 vxbDevRegister((struct vxbDevRegInfo *)&(obc2Registration));
	 printstr("obc2Register \r\n");
	return;
 }
 
  void obc2InstInit (struct vxbDev * pDev)
 {
	 vxbNextUnitGet(pDev);
	 return;
 }
  void obc2InstInit2(struct vxbDev * pDev)
 {
	 OBCINT *pDrvCtrl;
	 
	semInt3=semBCreate(SEM_Q_FIFO, SEM_EMPTY);
		
	 pDrvCtrl = malloc(sizeof(OBCINT));
	 if (pDrvCtrl == NULL) return;
	 bzero ((char *)pDrvCtrl, sizeof(OBCINT));
	 pDev->pDrvCtrl = pDrvCtrl;
	 pDrvCtrl->pDev = pDev;
	 	printstr("obc2InstInit2 \r\n");
	 return;
 }
  void obc2InstConnect(struct vxbDev * pDev)
 {
 	printstr("obc2InstConnect \r\n");
	 if(vxbIntConnect(pDev,0,obcInt,(void *)pDev) != OK)
	 {
		 printstr("obc2InstConnect: vxbIntConnect Fail\r\n");
	 }
	 else
	 {
		 printstr("obc2InstConnect: vxbIntConnect OK\r\n");
	 }
	 if(vxbIntEnable(pDev,0,obcInt,(void *)pDev) != OK)
	 {
		 printstr("obc2InstConnect: vxbIntEnable Fail\r\n");
	 }
	 else
	 {
		 printstr("obc2InstConnect: vxbIntEnable OK\r\n");
	 }
 }
  BOOL obc2Probe(struct vxbDev * pDev){return TRUE;}
 static void obc2Int(VXB_DEVICE_ID pDev)
 {
	 bsp_printf("obc int3 ok!\r\n");
	 semGive(semInt3);
 }
  


 void obc3InstInit(struct vxbDev * pDev);
 void obc3InstInit2(struct vxbDev * pDev);
 void obc3InstConnect(struct vxbDev * pDev);
 BOOL obc3Probe(struct vxbDev * pDev);
 static void obc3Int(VXB_DEVICE_ID pDev);

 struct drvBusFuncs obc3Funcs =
{
	obc3InstInit,     /* devInstanceInit */
	obc3InstInit2,    /* devInstanceInit2 */
	obc3InstConnect   /* devConnect */
};
 device_method_t obc3Methods[] ={{0, 0}};
 struct vxbPlbRegister obc3Registration =
{
    {
        NULL,                 /* pNext,一般都用NULL */
        VXB_DEVID_DEVICE,     /* devID,固定,表示这是一个设备 */
        VXB_BUSID_PLB,        /* busID = PLB ,表示这个设备挂接在PLB上*/
        VXBUS_VERSION_4,      /* 版本号,6.7为版本4 */
        "obcintdev3",     /*设备名称,必须和hwconfig.c中统一*/
        &obc3Funcs,       		/* pDrvBusFuncs,设备总线函数组在后面定义 */
        obc3Methods,     		/* pMethods ,设备方法组,在后面定义*/
        obc3Probe         		/* devProbe ,设备探测*/
    }
};
 void obc3Register(void){
	vxbDevRegister((struct vxbDevRegInfo *)&(obc3Registration));
   return;
}

 void obc3InstInit (struct vxbDev * pDev)
{
    vxbNextUnitGet(pDev);
    return;
}
 void obc3InstInit2(struct vxbDev * pDev)
{
	OBCINT *pDrvCtrl;
	
	semInt2=semBCreate(SEM_Q_FIFO, SEM_EMPTY);
	
    pDrvCtrl = malloc(sizeof(OBCINT));
    if (pDrvCtrl == NULL) return;
    bzero ((char *)pDrvCtrl, sizeof(OBCINT));
    pDev->pDrvCtrl = pDrvCtrl;
    pDrvCtrl->pDev = pDev;
    return;
}
 void obc3InstConnect(struct vxbDev * pDev)
{
	if(vxbIntConnect(pDev,0,obcInt,(void *)pDev) != OK)
	{
		printstr("obc3InstConnect: vxbIntConnect Fail\r\n");
	}
	else
	{
		printstr("obc3InstConnect: vxbIntConnect OK\r\n");
	}
	if(vxbIntEnable(pDev,0,obcInt,(void *)pDev) != OK)
	{
		printstr("obc3InstConnect: vxbIntEnable Fail\r\n");
	}
	else
	{
		printstr("obc3InstConnect: vxbIntEnable OK\r\n");
	}
}
 BOOL obc3Probe(struct vxbDev * pDev){return TRUE;}
static void obc3Int(VXB_DEVICE_ID pDev)
{

	bsp_printf("obc int2 ok!\r\n");
	 semGive(semInt2);
}
 
	

 static void obcInt(VXB_DEVICE_ID pDev)
 {
	 int lockId;

	 lockId=intCpuLock();
     g_intStatus = sysXRD32(0x90000000, 0x3ff01440);
//	 bsp_printf("obc int ok!\r\n");
	 
/*	 for(i=0;i<2000;i++)
	 {
		 ;
	 }*/
	 
	 *(UINT*)0xbe030010 = 1;
	 *(UINT*)0xbe0000c0 = 1;
	 *(UINT*)0xbe0000c0 = 1;
	 *(UINT*)0xbe0000c0 = 1;
	 *(UINT*)0xbe0000c0 = 1;
	 

	 if(g_intStatus & 0x1)
	 {
		//*(int *)0xbe0000c0=1;
	    obc0Int(pDev);
	 }

	 if(g_intStatus & 0x2)
	 {
	    obc1Int(pDev);
	 }

	 if(g_intStatus & 0x4)
	 {
	    obc2Int(pDev);
	 }

	 if(g_intStatus & 0x8)
	 {
	    obc3Int(pDev);
	 }
	 
	 intCpuUnlock(lockId);
	 
 }

#define INT_EN_SET  (0xbe00009c)
int intNumEn(int intNum)
{
	switch(intNum)
	{
		case 0:
			*(unsigned int *)INT_EN_SET = 0xfffffffe;
			break;
		case 1:
			*(unsigned int *)INT_EN_SET = 0xfffffffd;
			break;
		case 2:
			*(unsigned int *)INT_EN_SET = 0xfffffffb;
			break;
		case 3:
			*(unsigned int *)INT_EN_SET = 0xfffffff7;
			break;
		default:
			*(unsigned int *)INT_EN_SET = 0xffffffff;
			break;
	}
	
	return 0;
}


/*****************************	INT SET	*******************************/
int intSet(int intNum)
{
	intNumEn(intNum);
	bspTempRead(1);
	
	return 0;
}


/*add by zy 20111116 end*/
