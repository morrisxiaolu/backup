/* huarui ͷ�ļ� */
#define BONITO_REG_BASE 		0x1fe00000
#define BONITO_REG_SIZE 		0x00040000
#define BONITO_REG_TOP			(BONITO_REG_BASE+BONITO_REG_SIZE-1)
#define BONITO_DEV_BASE 		0x1ff00000
#define BONITO_DEV_SIZE 		0x00100000
#define BONITO_DEV_TOP			(BONITO_DEV_BASE+BONITO_DEV_SIZE-1)
#define BONITO_PCILO_BASE		0x10000000
#define BONITO_PCILO_BASE_VA    0xb0000000
#define BONITO_PCILO_SIZE		0x04000000
#define BONITO_PCILO_TOP		(BONITO_PCILO_BASE+BONITO_PCILO_SIZE-1)
#define BONITO_PCILO0_BASE		0x10000000
#define BONITO_PCILO1_BASE		0x14000000
#define BONITO_PCILO2_BASE		0x1c000000
#define BONITO_PCIHI_BASE		0x18000000
#define BONITO_PCIHI_SIZE		0x02000000
#define BONITO_PCIHI_TOP		(BONITO_PCIHI_BASE+BONITO_PCIHI_SIZE-1)
#define BONITO_PCIIO_BASE		0x1fd00000
#define BONITO_PCIIO_BASE_VA		0xbfd00000  //zxj resumed,20091029
//#define BONITO_PCIIO_BASE		0x1a000000
//#define BONITO_PCIIO_BASE_VA    0xba000000
#define BONITO_PCIIO_SIZE		0x01000000
#define BONITO_PCIIO_TOP		(BONITO_PCIIO_BASE+BONITO_PCIIO_SIZE-1)
#define BONITO_PCICFG_BASE		0x1fe80000
#define BONITO_PCICFG_SIZE		0x00080000
#define BONITO_PCICFG_TOP		(BONITO_PCICFG_BASE+BONITO_PCICFG_SIZE-1)
 

/* Bonito Register Bases */

#define BONITO_PCICONFIGBASE		0x00
#define BONITO_REGBASE			0x100


/* PCI Configuration  Registers */
#define BONITO(x)	*(volatile unsigned long *)(0xbfe00000+(x))

#define BONITO_PCI_REG(x)               BONITO(BONITO_PCICONFIGBASE + (x))
#define BONITO_PCIDID			BONITO_PCI_REG(0x00)
#define BONITO_PCICMD			BONITO_PCI_REG(0x04)
#define BONITO_PCICLASS 		BONITO_PCI_REG(0x08)
#define BONITO_PCILTIMER		BONITO_PCI_REG(0x0c)
#define BONITO_PCIBASE0 		BONITO_PCI_REG(0x10)
#define BONITO_PCIBASE1 		BONITO_PCI_REG(0x14)
#define BONITO_PCIBASE2 		BONITO_PCI_REG(0x18)
#define BONITO_PCIEXPRBASE		BONITO_PCI_REG(0x30)
#define BONITO_PCIINT			BONITO_PCI_REG(0x3c)

#define BONITO_PCICMD_PERR_CLR		0x80000000
#define BONITO_PCICMD_SERR_CLR		0x40000000
#define BONITO_PCICMD_MABORT_CLR	0x20000000
#define BONITO_PCICMD_MTABORT_CLR	0x10000000
#define BONITO_PCICMD_TABORT_CLR	0x08000000
#define BONITO_PCICMD_MPERR_CLR 	0x01000000
#define BONITO_PCICMD_PERRRESPEN	0x00000040
#define BONITO_PCICMD_ASTEPEN		0x00000080
#define BONITO_PCICMD_SERREN		0x00000100
#define BONITO_PCILTIMER_BUSLATENCY	0x0000ff00
#define BONITO_PCILTIMER_BUSLATENCY_SHIFT	8




/* 1. Bonito h/w Configuration */
/* Power on register */

#define BONITO_BONPONCFG		BONITO(BONITO_REGBASE + 0x00)

#define BONITO_BONPONCFG_SYSCONTROLLERRD	0x00040000
#define BONITO_BONPONCFG_ROMCS1SAMP	0x00020000
#define BONITO_BONPONCFG_ROMCS0SAMP	0x00010000
#define BONITO_BONPONCFG_CPUBIGEND	0x00004000
#define BONITO_BONPONCFG_CPUPARITY	0x00002000
#define BONITO_BONPONCFG_CPUTYPE	0x00000007
#define BONITO_BONPONCFG_CPUTYPE_SHIFT	0
#define BONITO_BONPONCFG_PCIRESET_OUT	0x00000008
#define BONITO_BONPONCFG_IS_ARBITER	0x00000010
#define BONITO_BONPONCFG_ROMBOOT	0x000000c0
#define BONITO_BONPONCFG_ROMBOOT_SHIFT	6

#define BONITO_BONPONCFG_ROMBOOT_FLASH	(0x0<<BONITO_BONPONCFG_ROMBOOT_SHIFT)
#define BONITO_BONPONCFG_ROMBOOT_SOCKET (0x1<<BONITO_BONPONCFG_ROMBOOT_SHIFT)
#define BONITO_BONPONCFG_ROMBOOT_SDRAM	(0x2<<BONITO_BONPONCFG_ROMBOOT_SHIFT)
#define BONITO_BONPONCFG_ROMBOOT_CPURESET	(0x3<<BONITO_BONPONCFG_ROMBOOT_SHIFT)

#define BONITO_BONPONCFG_ROMCS0WIDTH	0x00000100
#define BONITO_BONPONCFG_ROMCS1WIDTH	0x00000200
#define BONITO_BONPONCFG_ROMCS0FAST	0x00000400
#define BONITO_BONPONCFG_ROMCS1FAST	0x00000800
#define BONITO_BONPONCFG_CONFIG_DIS	0x00000020


/* Other Bonito configuration */

#define BONITO_BONGENCFG_OFFSET         0x4
#define BONITO_BONGENCFG		BONITO(BONITO_REGBASE + BONITO_BONGENCFG_OFFSET)

#define BONITO_BONGENCFG_DEBUGMODE	0x00000001
#define BONITO_BONGENCFG_SNOOPEN	0x00000002
#define BONITO_BONGENCFG_CPUSELFRESET	0x00000004

#define BONITO_BONGENCFG_FORCE_IRQA	0x00000008
#define BONITO_BONGENCFG_IRQA_ISOUT	0x00000010
#define BONITO_BONGENCFG_IRQA_FROM_INT1 0x00000020
#define BONITO_BONGENCFG_BYTESWAP	0x00000040

#define BONITO_BONGENCFG_PREFETCHEN	0x00000100
#define BONITO_BONGENCFG_WBEHINDEN	0x00000200
#define BONITO_BONGENCFG_CACHEALG	0x00000c00
#define BONITO_BONGENCFG_CACHEALG_SHIFT 10
#define BONITO_BONGENCFG_PCIQUEUE	0x00001000
#define BONITO_BONGENCFG_CACHESTOP	0x00002000
#define BONITO_BONGENCFG_MSTRBYTESWAP	0x00004000
#define BONITO_BONGENCFG_BUSERREN	0x00008000
#define BONITO_BONGENCFG_NORETRYTIMEOUT 0x00010000
#define BONITO_BONGENCFG_SHORTCOPYTIMEOUT	0x00020000

/* 2. IO & IDE configuration */

#define BONITO_IODEVCFG 		BONITO(BONITO_REGBASE + 0x08)

/* 3. IO & IDE configuration */

#define BONITO_SDCFG			BONITO(BONITO_REGBASE + 0x0c)

/* 4. PCI address map control */

#define BONITO_PCIMAP			BONITO(BONITO_REGBASE + 0x10)
#define BONITO_PCIMEMBASECFG	BONITO(BONITO_REGBASE + 0x14)
#define BONITO_PCIMAP_CFG		BONITO(BONITO_REGBASE + 0x18)

/*
 * Command and status register.
 */
#define	PCI_COMMAND_STATUS_REG			0x04

#define	PCI_COMMAND_IO_ENABLE			0x00000001
#define	PCI_COMMAND_MEM_ENABLE			0x00000002
#define	PCI_COMMAND_MASTER_ENABLE		0x00000004
#define	PCI_COMMAND_SPECIAL_ENABLE		0x00000008
#define	PCI_COMMAND_INVALIDATE_ENABLE		0x00000010
#define	PCI_COMMAND_PALETTE_ENABLE		0x00000020
#define	PCI_COMMAND_PARITY_ENABLE		0x00000040
#define	PCI_COMMAND_STEPPING_ENABLE		0x00000080
#define	PCI_COMMAND_SERR_ENABLE			0x00000100
#define	PCI_COMMAND_BACKTOBACK_ENABLE		0x00000200

#define	PCI_STATUS_CAPLIST_SUPPORT		0x00100000
#define	PCI_STATUS_66MHZ_SUPPORT		0x00200000
#define	PCI_STATUS_UDF_SUPPORT			0x00400000
#define	PCI_STATUS_BACKTOBACK_SUPPORT		0x00800000
#define	PCI_STATUS_PARITY_ERROR			0x01000000
#define	PCI_STATUS_DEVSEL_FAST			0x00000000
#define	PCI_STATUS_DEVSEL_MEDIUM		0x02000000
#define	PCI_STATUS_DEVSEL_SLOW			0x04000000
#define	PCI_STATUS_DEVSEL_MASK			0x06000000
#define	PCI_STATUS_TARGET_TARGET_ABORT		0x08000000
#define	PCI_STATUS_MASTER_TARGET_ABORT		0x10000000
#define	PCI_STATUS_MASTER_ABORT			0x20000000
#define	PCI_STATUS_SPECIAL_ERROR		0x40000000
#define	PCI_STATUS_PARITY_DETECT		0x80000000


#define	PCI_BHLC_REG			0x0c

#define	PCI_HDRTYPE_SHIFT			16
#define	PCI_HDRTYPE_MASK			0xff
#define	PCI_HDRTYPE(bhlcr)     (((bhlcr) >> PCI_HDRTYPE_SHIFT) & PCI_HDRTYPE_MASK)
#define PCI_HDRTYPE_TYPE(bhlcr)     (PCI_HDRTYPE(bhlcr) & 0x7f)
#define	PCI_HDRTYPE_MULTIFN(bhlcr)   ((PCI_HDRTYPE(bhlcr) & 0x80) != 0)
 //           *(volatile unsigned char *)0xbfe001e0 = 'b'; BIG_LOOP
#define BONITO_PCIMAP_PCIMAP_LO0	0x0000003f
#define BONITO_PCIMAP_PCIMAP_LO0_SHIFT	0
#define BONITO_PCIMAP_PCIMAP_LO1	0x00000fc0
#define BONITO_PCIMAP_PCIMAP_LO1_SHIFT	6
#define BONITO_PCIMAP_PCIMAP_LO2	0x0003f000
#define BONITO_PCIMAP_PCIMAP_LO2_SHIFT	12
#define BONITO_PCIMAP_PCIMAP_2		0x00040000
#define BONITO_PCIMAP_WIN(WIN,ADDR)	((((ADDR)>>26) & BONITO_PCIMAP_PCIMAP_LO0) << ((WIN)*6))
/* base classes */
#define	PCI_CLASS_PREHISTORIC			0x00
#define	PCI_CLASS_MASS_STORAGE			0x01
#define	PCI_CLASS_NETWORK			0x02
#define	PCI_CLASS_DISPLAY			0x03
#define	PCI_CLASS_MULTIMEDIA			0x04
#define	PCI_CLASS_MEMORY			0x05
#define	PCI_CLASS_BRIDGE			0x06
#define	PCI_CLASS_COMMUNICATIONS		0x07
#define	PCI_CLASS_SYSTEM			0x08
#define	PCI_CLASS_INPUT				0x09
#define	PCI_CLASS_DOCK				0x0a
#define	PCI_CLASS_PROCESSOR			0x0b
#define	PCI_CLASS_SERIALBUS			0x0c
#define	PCI_CLASS_WIRELESS			0x0d
#define	PCI_CLASS_I2O				0x0e
#define	PCI_CLASS_SATCOM			0x0f
#define	PCI_CLASS_CRYPTO			0x10
#define	PCI_CLASS_DASP				0x11
#define	PCI_CLASS_UNDEFINED			0xff

#define	PCI_CLASS_SHIFT				24
#define	PCI_CLASS_MASK				0xff
#define	PCI_CLASS(cr) \
	    (((cr) >> PCI_CLASS_SHIFT) & PCI_CLASS_MASK)

#define	PCI_SUBCLASS_SHIFT			16
#define	PCI_SUBCLASS_MASK			0xff
#define	PCI_SUBCLASS(cr) \
	    (((cr) >> PCI_SUBCLASS_SHIFT) & PCI_SUBCLASS_MASK)

#define PCI_ISCLASS(what, class, subclass) \
	(((what) & 0xffff0000) == (class << 24 | subclass << 16))

#define	PCI_INTERFACE_SHIFT			8
#define	PCI_INTERFACE_MASK			0xff
#define	PCI_INTERFACE(cr) \
	    (((cr) >> PCI_INTERFACE_SHIFT) & PCI_INTERFACE_MASK)

#define	PCI_REVISION_SHIFT			0
#define	PCI_REVISION_MASK			0xff
#define	PCI_REVISION(cr) \
	    (((cr) >> PCI_REVISION_SHIFT) & PCI_REVISION_MASK)

/* 0x06 bridge subclasses */
#define	PCI_SUBCLASS_BRIDGE_HOST		0x00
#define	PCI_SUBCLASS_BRIDGE_ISA			0x01
#define	PCI_SUBCLASS_BRIDGE_EISA		0x02
#define	PCI_SUBCLASS_BRIDGE_MC			0x03
#define	PCI_SUBCLASS_BRIDGE_PCI			0x04
#define	PCI_SUBCLASS_BRIDGE_PCMCIA		0x05
#define	PCI_SUBCLASS_BRIDGE_NUBUS		0x06
#define	PCI_SUBCLASS_BRIDGE_CARDBUS		0x07
#define	PCI_SUBCLASS_BRIDGE_RACEWAY		0x08
#define	PCI_SUBCLASS_BRIDGE_STPCI		0x09
#define	PCI_SUBCLASS_BRIDGE_INFINIBAND	0x0a
#define	PCI_SUBCLASS_BRIDGE_MISC		0x80
