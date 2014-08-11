
#ifndef __INCVxbAlteraTseh
#define __INCVxbAlteraTseh

#ifdef __cplusplus
extern "C" {
#endif


#define ALTERA_ON_CHIP_MEM_BASE_ADDRESS     (0xb8000000)
#define ALTERA_TSE_MAC_BASE_ADDRESS         (0xb8001000)
#define ALTERA_SGDMA0_TX_BASE_ADDRESS       (0xb8001440)
#define ALTERA_SGDMA1_RX_BASE_ADDRESS       (0xb8001400)
#define ALTERA_PCI_CONTROL_BASE_ADDRESS     (0xb8004000)

#define ALTERA_SGDMA_RX_RING_NUM           60
#define ALTERA_SGDMA_TX_RING_NUM           10

#define EMAC_RX_DESC_CNT        (ALTERA_SGDMA_RX_RING_NUM - 1)
#define EMAC_TX_DESC_CNT        (ALTERA_SGDMA_TX_RING_NUM - 1)


#define __IO_CALC_ADDRESS_DYNAMIC(BASE, OFFSET)   ((void *)(((UINT8*)BASE) + (OFFSET)))
#define __IO_CALC_ADDRESS_NATIVE(BASE, REGNUM)    ((void *)(((UINT8*)BASE) + (REGNUM * 4)))

#define IORD_32DIRECT(base,offset)  (*((volatile UINT32 *)((UINT32)base+offset)))
#define IORD_16DIRECT(base,offset)  (*((volatile UINT16 *)((UINT32)base+offset)))
#define IORD_8DIRECT(base,offset)   (*((volatile UINT8 *)((UINT32)base+offset)))

#define IOWR_32DIRECT(base,offset,data)  *((volatile UINT32 *)((UINT32)base+offset))=(UINT32)data ;
#define IOWR_16DIRECT(base,offset,data)  *((volatile UINT16 *)((UINT32)base+offset))=(UINT16)data ;
#define IOWR_8DIRECT(base,offset,data)   *((volatile UINT8 *)((UINT32)base+offset))=(UINT8)data ;

#define ALTERA_REGISTER_WR(addr,data)     *((volatile UINT32 *)( addr))=(UINT32)data   
#define ALTERA_REGISTER_RD(addr)         (*((volatile UINT32 *)( addr)))   

#define IORD(BASE, REGNUM)       ALTERA_REGISTER_RD(__IO_CALC_ADDRESS_NATIVE ((BASE), (REGNUM)))
#define IOWR(BASE, REGNUM, DATA) ALTERA_REGISTER_WR(__IO_CALC_ADDRESS_NATIVE ((BASE), (REGNUM)), (DATA))


#define IOADDR_ALTERA_AVALON_SGDMA_STATUS(base)       __IO_CALC_ADDRESS_DYNAMIC(base, 0)
#define IORD_ALTERA_AVALON_SGDMA_STATUS(base)         IORD(base, 0)
#define IOWR_ALTERA_AVALON_SGDMA_STATUS(base, data)   IOWR(base, 0, data)

#define ALTERA_AVALON_SGDMA_STATUS_ERROR_MSK           (0x1)
#define ALTERA_AVALON_SGDMA_STATUS_ERROR_OFST          (0)
#define ALTERA_AVALON_SGDMA_STATUS_EOP_ENCOUNTERED_MSK           (0x2)
#define ALTERA_AVALON_SGDMA_STATUS_EOP_ENCOUNTERED_OFST          (1)
#define ALTERA_AVALON_SGDMA_STATUS_DESC_COMPLETED_MSK           (0x4)
#define ALTERA_AVALON_SGDMA_STATUS_DESC_COMPLETED_OFST          (2)
#define ALTERA_AVALON_SGDMA_STATUS_CHAIN_COMPLETED_MSK           (0x8)
#define ALTERA_AVALON_SGDMA_STATUS_CHAIN_COMPLETED_OFST          (3)
#define ALTERA_AVALON_SGDMA_STATUS_BUSY_MSK            (0x10)
#define ALTERA_AVALON_SGDMA_STATUS_BUSY_OFST           (4)

#define IOADDR_ALTERA_AVALON_SGDMA_CONTROL(base)     __IO_CALC_ADDRESS_DYNAMIC(base, 4)
#define IORD_ALTERA_AVALON_SGDMA_CONTROL(base)        IORD(base, 4)
#define IOWR_ALTERA_AVALON_SGDMA_CONTROL(base, data)  IOWR(base, 4, data)
#define ALTERA_AVALON_SGDMA_CONTROL_IE_ERROR_MSK  (0x1)
#define ALTERA_AVALON_SGDMA_CONTROL_IE_ERROR_OFST  (0)
#define ALTERA_AVALON_SGDMA_CONTROL_IE_EOP_ENCOUNTERED_MSK  (0x2)
#define ALTERA_AVALON_SGDMA_CONTROL_IE_EOP_ENCOUNTERED_OFST  (1)
#define ALTERA_AVALON_SGDMA_CONTROL_IE_DESC_COMPLETED_MSK  (0x4)
#define ALTERA_AVALON_SGDMA_CONTROL_IE_DESC_COMPLETED_OFST  (2)
#define ALTERA_AVALON_SGDMA_CONTROL_IE_CHAIN_COMPLETED_MSK  (0x8)
#define ALTERA_AVALON_SGDMA_CONTROL_IE_CHAIN_COMPLETED_OFST  (3)
#define ALTERA_AVALON_SGDMA_CONTROL_IE_GLOBAL_MSK  (0x10)
#define ALTERA_AVALON_SGDMA_CONTROL_IE_GLOBAL_OFST  (4)
#define ALTERA_AVALON_SGDMA_CONTROL_RUN_MSK  (0x20)
#define ALTERA_AVALON_SGDMA_CONTROL_RUN_OFST  (5)
#define ALTERA_AVALON_SGDMA_CONTROL_STOP_DMA_ER_MSK  (0x40)
#define ALTERA_AVALON_SGDMA_CONTROL_STOP_DMA_ER_OFST  (6)
#define ALTERA_AVALON_SGDMA_CONTROL_IE_MAX_DESC_PROCESSED_MSK  (0x80)
#define ALTERA_AVALON_SGDMA_CONTROL_IE_MAX_DESC_PROCESSED_OFST  (7)
#define ALTERA_AVALON_SGDMA_CONTROL_MAX_DESC_PROCESSED_MSK  (0xFF00)
#define ALTERA_AVALON_SGDMA_CONTROL_MAX_DESC_PROCESSED_OFST  (8)
#define ALTERA_AVALON_SGDMA_CONTROL_SOFTWARERESET_MSK (0X10000)
#define ALTERA_AVALON_SGDMA_CONTROL_SOFTWARERESET_OFST (16)
#define ALTERA_AVALON_SGDMA_CONTROL_PARK_MSK (0X20000)
#define ALTERA_AVALON_SGDMA_CONTROL_PARK_OFST (17)
#define ALTERA_AVALON_SGDMA_CONTROL_CLEAR_INTERRUPT_MSK (0X80000000)
#define ALTERA_AVALON_SGDMA_CONTROL_CLEAR_INTERRUPT_OFST (31)

#define IOADDR_ALTERA_AVALON_SGDMA_NEXT_DESC_POINTER(base)     __IO_CALC_ADDRESS_DYNAMIC(base, 8)
#define IORD_ALTERA_AVALON_SGDMA_NEXT_DESC_POINTER(base)        IORD(base, 8)
#define IOWR_ALTERA_AVALON_SGDMA_NEXT_DESC_POINTER(base, data)  IOWR(base, 8, data)




/* Each Scatter-gather DMA buffer descriptor spans 0x20 of memory */
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_SIZE (0x20)

/*
 * Descriptor control bit masks & offsets
 *
 * Note: The control byte physically occupies bits [31:24] in memory.
 *       The following bit-offsets are expressed relative to the LSB of
 *       the control register bitfield.
 */
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_GENERATE_EOP_MSK (0x1)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_GENERATE_EOP_OFST (0)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_READ_FIXED_ADDRESS_MSK (0x2)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_READ_FIXED_ADDRESS_OFST (1)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_WRITE_FIXED_ADDRESS_MSK (0x4)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_WRITE_FIXED_ADDRESS_OFST (2)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_ATLANTIC_CHANNEL_MSK (0x8)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_ATLANTIC_CHANNEL_OFST (3)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_OWNED_BY_HW_MSK (0x80)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_OWNED_BY_HW_OFST (7)

/*
 * Descriptor status bit masks & offsets
 *
 * Note: The status byte physically occupies bits [23:16] in memory.
 *       The following bit-offsets are expressed relative to the LSB of
 *       the status register bitfield.
 */
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_CRC_MSK (0x1)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_CRC_OFST (0)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_PARITY_MSK (0x2)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_PARITY_OFST (1)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_OVERFLOW_MSK (0x4)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_OVERFLOW_OFST (2)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_SYNC_MSK (0x8)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_SYNC_OFST (3)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_UEOP_MSK (0x10)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_UEOP_OFST (4)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_MEOP_MSK (0x20)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_MEOP_OFST (5)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_MSOP_MSK (0x40)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_MSOP_OFST (6)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_TERMINATED_BY_EOP_MSK (0x80)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_TERMINATED_BY_EOP_OFST (7)

/* MAC Registers */

/* Revision register (read-only) */
#define IOADDR_ALTERA_TSEMAC_REV(base) __IO_CALC_ADDRESS_NATIVE(base,0x0)
#define IORD_ALTERA_TSEMAC_REV(base)   IORD_32DIRECT(base, 0)

/* Scratch register */
#define IOADDR_ALTERA_TSEMAC_SCRATCH(base)    __IO_CALC_ADDRESS_NATIVE(base,0x4)
#define IORD_ALTERA_TSEMAC_SCRATCH(base)      IORD_32DIRECT(base, 0x4)
#define IOWR_ALTERA_TSEMAC_SCRATCH(base,data) IOWR_32DIRECT(base, 0x4, data)

/* Command register */
#define IOADDR_ALTERA_TSEMAC_CMD_CONFIG(base)    __IO_CALC_ADDRESS_NATIVE(base,0x8)
#define IORD_ALTERA_TSEMAC_CMD_CONFIG(base)      IORD_32DIRECT(base, 0x8)
#define IOWR_ALTERA_TSEMAC_CMD_CONFIG(base,data) IOWR_32DIRECT(base, 0x8, data)

/* Command register bit definitions */
#define ALTERA_TSEMAC_CMD_TX_ENA_OFST          (0)   
#define ALTERA_TSEMAC_CMD_TX_ENA_MSK           (0x1)
#define ALTERA_TSEMAC_CMD_RX_ENA_OFST          (1)   
#define ALTERA_TSEMAC_CMD_RX_ENA_MSK           (0x2)
#define ALTERA_TSEMAC_CMD_XON_GEN_OFST        (2) 
#define ALTERA_TSEMAC_CMD_XON_GEN_MSK         (0x4)
#define ALTERA_TSEMAC_CMD_ETH_SPEED_OFST       (3)
#define ALTERA_TSEMAC_CMD_ETH_SPEED_MSK        (0x8)
#define ALTERA_TSEMAC_CMD_PROMIS_EN_OFST       (4)
#define ALTERA_TSEMAC_CMD_PROMIS_EN_MSK        (0x10)
#define ALTERA_TSEMAC_CMD_PAD_EN_OFST          (5)
#define ALTERA_TSEMAC_CMD_PAD_EN_MSK           (0x20)
#define ALTERA_TSEMAC_CMD_CRC_FWD_OFST         (6)  
#define ALTERA_TSEMAC_CMD_CRC_FWD_MSK          (0x40)
#define ALTERA_TSEMAC_CMD_PAUSE_FWD_OFST       (7)
#define ALTERA_TSEMAC_CMD_PAUSE_FWD_MSK        (0x80)
#define ALTERA_TSEMAC_CMD_PAUSE_IGNORE_OFST    (8)
#define ALTERA_TSEMAC_CMD_PAUSE_IGNORE_MSK     (0x100)
#define ALTERA_TSEMAC_CMD_TX_ADDR_INS_OFST     (9)
#define ALTERA_TSEMAC_CMD_TX_ADDR_INS_MSK      (0x200)
#define ALTERA_TSEMAC_CMD_HD_ENA_OFST          (10)
#define ALTERA_TSEMAC_CMD_HD_ENA_MSK           (0x400)
#define ALTERA_TSEMAC_CMD_EXCESS_COL_OFST      (11)
#define ALTERA_TSEMAC_CMD_EXCESS_COL_MSK       (0x800)
#define ALTERA_TSEMAC_CMD_LATE_COL_OFST        (12)
#define ALTERA_TSEMAC_CMD_LATE_COL_MSK         (0x1000)
#define ALTERA_TSEMAC_CMD_SW_RESET_OFST        (13)
#define ALTERA_TSEMAC_CMD_SW_RESET_MSK         (0x2000)
#define ALTERA_TSEMAC_CMD_MHASH_SEL_OFST       (14)
#define ALTERA_TSEMAC_CMD_MHASH_SEL_MSK        (0x4000)
#define ALTERA_TSEMAC_CMD_LOOPBACK_OFST        (15)
#define ALTERA_TSEMAC_CMD_LOOPBACK_MSK         (0x8000)
/* Bits (18:16) = address select */
#define ALTERA_TSEMAC_CMD_TX_ADDR_SEL_OFST     (16)   
#define ALTERA_TSEMAC_CMD_TX_ADDR_SEL_MSK      (0x70000)  
#define ALTERA_TSEMAC_CMD_MAGIC_ENA_OFST       (19)
#define ALTERA_TSEMAC_CMD_MAGIC_ENA_MSK        (0x80000)
#define ALTERA_TSEMAC_CMD_SLEEP_OFST           (20)
#define ALTERA_TSEMAC_CMD_SLEEP_MSK            (0x100000)
#define ALTERA_TSEMAC_CMD_WAKEUP_OFST          (21)
#define ALTERA_TSEMAC_CMD_WAKEUP_MSK           (0x200000)
#define ALTERA_TSEMAC_CMD_XOFF_GEN_OFST        (22)
#define ALTERA_TSEMAC_CMD_XOFF_GEN_MSK         (0x400000)
#define ALTERA_TSEMAC_CMD_CNTL_FRM_ENA_OFST    (23)
#define ALTERA_TSEMAC_CMD_CNTL_FRM_ENA_MSK     (0x800000)
#define ALTERA_TSEMAC_CMD_NO_LENGTH_CHECK_OFST (24)
#define ALTERA_TSEMAC_CMD_NO_LENGTH_CHECK_MSK  (0x1000000)
#define ALTERA_TSEMAC_CMD_ENA_10_OFST          (25)
#define ALTERA_TSEMAC_CMD_ENA_10_MSK           (0x2000000)
#define ALTERA_TSEMAC_CMD_RX_ERR_DISC_OFST     (26)
#define ALTERA_TSEMAC_CMD_RX_ERR_DISC_MSK      (0x4000000)
/* Bits (30..27) reserved */
#define ALTERA_TSEMAC_CMD_CNT_RESET_OFST       (31)
#define ALTERA_TSEMAC_CMD_CNT_RESET_MSK        (0x80000000)

/* Low word (bits 31:0) of MAC address */
#define IOADDR_ALTERA_TSEMAC_MAC_0(base)    __IO_CALC_ADDRESS_NATIVE(base,0xC)
#define IORD_ALTERA_TSEMAC_MAC_0(base)      IORD_32DIRECT(base, 0xC)
#define IOWR_ALTERA_TSEMAC_MAC_0(base,data) IOWR_32DIRECT(base, 0xC, data)

/* High half-word (bits 47:32) of MAC address. Upper 16 bits reserved */
#define IOADDR_ALTERA_TSEMAC_MAC_1(base)    __IO_CALC_ADDRESS_NATIVE(base,0x10)
#define IORD_ALTERA_TSEMAC_MAC_1(base)      IORD_32DIRECT(base, 0x10)
#define IOWR_ALTERA_TSEMAC_MAC_1(base,data) IOWR_32DIRECT(base, 0x10, data)

/* Maximum frame length (bits 13:0), (bits 31:14 are reserved) */
#define IOADDR_ALTERA_TSEMAC_FRM_LENGTH(base)    __IO_CALC_ADDRESS_NATIVE(base,0x14)
#define IORD_ALTERA_TSEMAC_FRM_LENGTH(base)      IORD_32DIRECT(base, 0x14)
#define IOWR_ALTERA_TSEMAC_FRM_LENGTH(base,data) IOWR_32DIRECT(base, 0x14, data)

/* Receive pause quanta. Bits 31:16 reserved */
#define IOADDR_ALTERA_TSEMAC_PAUSE_QUANT(base)    __IO_CALC_ADDRESS_NATIVE(base,0x18)
#define IORD_ALTERA_TSEMAC_PAUSE_QUANT(base)      IORD_32DIRECT(base, 0x18)
#define IOWR_ALTERA_TSEMAC_PAUSE_QUANT(base,data) IOWR_32DIRECT(base, 0x18, data)

/* Sets RX FIFO section empty threshold */
#define IOADDR_ALTERA_TSEMAC_RX_SECTION_EMPTY(base)    __IO_CALC_ADDRESS_NATIVE(base,0x1C)
#define IORD_ALTERA_TSEMAC_RX_SECTION_EMPTY(base)      IORD_32DIRECT(base, 0x1C)
#define IOWR_ALTERA_TSEMAC_RX_SECTION_EMPTY(base,data) IOWR_32DIRECT(base, 0x1C, data)

/* Set RX FIFO section full threshold */
#define IOADDR_ALTERA_TSEMAC_RX_SECTION_FULL(base)    __IO_CALC_ADDRESS_NATIVE(base,0x20)
#define IORD_ALTERA_TSEMAC_RX_SECTION_FULL(base)      IORD_32DIRECT(base, 0x20)
#define IOWR_ALTERA_TSEMAC_RX_SECTION_FULL(base,data) IOWR_32DIRECT(base, 0x20, data)

/* Set TX FIFO section empty threshold */
#define IOADDR_ALTERA_TSEMAC_TX_SECTION_EMPTY(base)    __IO_CALC_ADDRESS_NATIVE(base,0x24)
#define IORD_ALTERA_TSEMAC_TX_SECTION_EMPTY(base)      IORD_32DIRECT(base, 0x24)
#define IOWR_ALTERA_TSEMAC_TX_SECTION_EMPTY(base,data) IOWR_32DIRECT(base, 0x24, data)

/* Set TX FIFO section full threshold */
#define IOADDR_ALTERA_TSEMAC_TX_SECTION_FULL(base)    __IO_CALC_ADDRESS_NATIVE(base,0x28)
#define IORD_ALTERA_TSEMAC_TX_SECTION_FULL(base)      IORD_32DIRECT(base, 0x28)
#define IOWR_ALTERA_TSEMAC_TX_SECTION_FULL(base,data) IOWR_32DIRECT(base, 0x28, data)

/* Set RX FIFO almost empty threshold */
#define IOADDR_ALTERA_TSEMAC_RX_ALMOST_EMPTY(base)    __IO_CALC_ADDRESS_NATIVE(base,0x2c)
#define IORD_ALTERA_TSEMAC_RX_ALMOST_EMPTY(base)      IORD_32DIRECT(base, 0x2c)
#define IOWR_ALTERA_TSEMAC_RX_ALMOST_EMPTY(base,data) IOWR_32DIRECT(base, 0x2c, data)

/* Set RX FIFO almost full threshold */
#define IOADDR_ALTERA_TSEMAC_RX_ALMOST_FULL(base)    __IO_CALC_ADDRESS_NATIVE(base,0x30)
#define IORD_ALTERA_TSEMAC_RX_ALMOST_FULL(base)      IORD_32DIRECT(base, 0x30)
#define IOWR_ALTERA_TSEMAC_RX_ALMOST_FULL(base,data) IOWR_32DIRECT(base, 0x30, data)

/* Set TX FIFO almost empty threshold */
#define IOADDR_ALTERA_TSEMAC_TX_ALMOST_EMPTY(base)    __IO_CALC_ADDRESS_NATIVE(base,0x34)
#define IORD_ALTERA_TSEMAC_TX_ALMOST_EMPTY(base)      IORD_32DIRECT(base, 0x34)
#define IOWR_ALTERA_TSEMAC_TX_ALMOST_EMPTY(base,data) IOWR_32DIRECT(base, 0x34, data)

/* Set TX FIFO almost full threshold */
#define IOADDR_ALTERA_TSEMAC_TX_ALMOST_FULL(base)    __IO_CALC_ADDRESS_NATIVE(base,0x38)
#define IORD_ALTERA_TSEMAC_TX_ALMOST_FULL(base)      IORD_32DIRECT(base, 0x38)
#define IOWR_ALTERA_TSEMAC_TX_ALMOST_FULL(base,data) IOWR_32DIRECT(base, 0x38, data)

/* MDIO Address of PHY 0. Bits 31:5 reserved */
#define IOADDR_ALTERA_TSEMAC_MDIO_ADDR0(base)    __IO_CALC_ADDRESS_NATIVE(base,0x3c)
#define IORD_ALTERA_TSEMAC_MDIO_ADDR0(base)      IORD_32DIRECT(base, 0x3c)
#define IOWR_ALTERA_TSEMAC_MDIO_ADDR0(base,data) IOWR_32DIRECT(base, 0x3c, data)

/* MDIO Address of PHY 1. Bits 31:5 reserved */
#define IOADDR_ALTERA_TSEMAC_MDIO_ADDR1(base)    __IO_CALC_ADDRESS_NATIVE(base,0x40)
#define IORD_ALTERA_TSEMAC_MDIO_ADDR1(base)      IORD_32DIRECT(base, 0x40)
#define IOWR_ALTERA_TSEMAC_MDIO_ADDR1(base,data) IOWR_32DIRECT(base, 0x40, data)

/* -- Register offsets 0x44 to 0x54 reserved -- */

/* Register read access status */
#define IOADDR_ALTERA_TSEMAC_REG_STAT(base)    __IO_CALC_ADDRESS_NATIVE(base,0x58)
#define IORD_ALTERA_TSEMAC_REG_STAT(base)      IORD_32DIRECT(base, 0x58)


/* Inter-packet gap. Bits 31:5 reserved/ */
#define IOADDR_ALTERA_TSEMAC_TX_IPG_LENGTH(base)     __IO_CALC_ADDRESS_NATIVE(base,0x5c)
#define IORD_ALTERA_TSEMAC_TX_IPG_LENGTH(base)      IORD_32DIRECT(base, 0x5c)
#define IOWR_ALTERA_TSEMAC_TX_IPG_LENGTH(base,data)      IOWR_32DIRECT(base, 0x5c, data)


/* IEEE802.3, RMON, and MIB-II SNMP Statistic event counters */
#define IOADDR_ALTERA_TSEMAC_A_MACID_1(base)     __IO_CALC_ADDRESS_NATIVE(base,0x60)
#define IORD_ALTERA_TSEMAC_A_MACID_1(base)      IORD_32DIRECT(base, 0x60)


#define IOADDR_ALTERA_TSEMAC_A_MACID_2(base)     __IO_CALC_ADDRESS_NATIVE(base,0x64)
#define IORD_ALTERA_TSEMAC_A_MACID_2(base)      IORD_32DIRECT(base, 0x64)


#define IOADDR_ALTERA_TSEMAC_A_FRAMES_TX_OK(base)     __IO_CALC_ADDRESS_NATIVE(base,0x68)
#define IORD_ALTERA_TSEMAC_A_FRAMES_TX_OK(base)      IORD_32DIRECT(base, 0x68)


#define IOADDR_ALTERA_TSEMAC_A_FRAMES_RX_OK(base)     __IO_CALC_ADDRESS_NATIVE(base,0x6c)
#define IORD_ALTERA_TSEMAC_A_FRAMES_RX_OK(base)      IORD_32DIRECT(base, 0x6c)


#define IOADDR_ALTERA_TSEMAC_A_FRAME_CHECK_SEQ_ERRS(base)     __IO_CALC_ADDRESS_NATIVE(base,0x70)
#define IORD_ALTERA_TSEMAC_A_FRAME_CHECK_SEQ_ERRS(base)      IORD_32DIRECT(base, 0x70)


#define IOADDR_ALTERA_TSEMAC_A_ALIGNMENT_ERRS(base)     __IO_CALC_ADDRESS_NATIVE(base,0x74)
#define IORD_ALTERA_TSEMAC_A_ALIGNMENT_ERRS(base)      IORD_32DIRECT(base, 0x74)


#define IOADDR_ALTERA_TSEMAC_A_OCTETS_TX_OK(base)     __IO_CALC_ADDRESS_NATIVE(base,0x78)
#define IORD_ALTERA_TSEMAC_A_OCTETS_TX_OK(base)      IORD_32DIRECT(base, 0x78)


#define IOADDR_ALTERA_TSEMAC_A_OCTETS_RX_OK(base)     __IO_CALC_ADDRESS_NATIVE(base,0x7c)
#define IORD_ALTERA_TSEMAC_A_OCTETS_RX_OK(base)      IORD_32DIRECT(base, 0x7c)


#define IOADDR_ALTERA_TSEMAC_A_TX_PAUSE_MAC_CTRL_FRAMES(base)     __IO_CALC_ADDRESS_NATIVE(base,0x80)
#define IORD_ALTERA_TSEMAC_A_TX_PAUSE_MAC_CTRL_FRAMES(base)      IORD_32DIRECT(base, 0x80)


#define IOADDR_ALTERA_TSEMAC_A_RX_PAUSE_MAC_CTRL_FRAMES(base)     __IO_CALC_ADDRESS_NATIVE(base,0x84)
#define IORD_ALTERA_TSEMAC_A_RX_PAUSE_MAC_CTRL_FRAMES(base)      IORD_32DIRECT(base, 0x84)


#define IOADDR_ALTERA_TSEMAC_IF_IN_ERRORS(base)     __IO_CALC_ADDRESS_NATIVE(base,0x88)
#define IORD_ALTERA_TSEMAC_IF_IN_ERRORS(base)      IORD_32DIRECT(base, 0x88)


#define IOADDR_ALTERA_TSEMAC_IF_OUT_ERRORS(base)     __IO_CALC_ADDRESS_NATIVE(base,0x8c)
#define IORD_ALTERA_TSEMAC_IF_OUT_ERRORS(base)      IORD_32DIRECT(base, 0x8c)


#define IOADDR_ALTERA_TSEMAC_IF_IN_UCAST_PKTS(base)     __IO_CALC_ADDRESS_NATIVE(base,0x90)
#define IORD_ALTERA_TSEMAC_IF_IN_UCAST_PKTS(base)      IORD_32DIRECT(base, 0x90)


#define IOADDR_ALTERA_TSEMAC_IF_IN_MULTICAST_PKTS(base)     __IO_CALC_ADDRESS_NATIVE(base,0x94)
#define IORD_ALTERA_TSEMAC_IF_IN_MULTICAST_PKTS(base)      IORD_32DIRECT(base, 0x94)


#define IOADDR_ALTERA_TSEMAC_IF_IN_BROADCAST_PKTS(base)     __IO_CALC_ADDRESS_NATIVE(base,0x98)
#define IORD_ALTERA_TSEMAC_IF_IN_BROADCAST_PKTS(base)      IORD_32DIRECT(base, 0x98)


#define IOADDR_ALTERA_TSEMAC_IF_OUT_DISCARDS(base)     __IO_CALC_ADDRESS_NATIVE(base,0x9C)
#define IORD_ALTERA_TSEMAC_IF_OUT_DISCARDS(base)      IORD_32DIRECT(base, 0x9C)


#define IOADDR_ALTERA_TSEMAC_IF_OUT_UCAST_PKTS(base)     __IO_CALC_ADDRESS_NATIVE(base,0xA0)
#define IORD_ALTERA_TSEMAC_IF_OUT_UCAST_PKTS(base)      IORD_32DIRECT(base, 0xA0)


#define IOADDR_ALTERA_TSEMAC_IF_OUT_MULTICAST_PKTS(base)     __IO_CALC_ADDRESS_NATIVE(base,0xA4)
#define IORD_ALTERA_TSEMAC_IF_OUT_MULTICAST_PKTS(base)      IORD_32DIRECT(base, 0xA4)


#define IOADDR_ALTERA_TSEMAC_IF_OUT_BROADCAST_PKTS(base)     __IO_CALC_ADDRESS_NATIVE(base,0xA8)
#define IORD_ALTERA_TSEMAC_IF_OUT_BROADCAST_PKTS(base)      IORD_32DIRECT(base, 0xA8)


#define IOADDR_ALTERA_TSEMAC_ETHER_STATS_DROP_EVENTS(base)     __IO_CALC_ADDRESS_NATIVE(base,0xAC)
#define IORD_ALTERA_TSEMAC_ETHER_STATS_DROP_EVENTS(base)      IORD_32DIRECT(base, 0xAC)


#define IOADDR_ALTERA_TSEMAC_ETHER_STATS_OCTETS(base)     __IO_CALC_ADDRESS_NATIVE(base,0xB0)
#define IORD_ALTERA_TSEMAC_ETHER_STATS_OCTETS(base)      IORD_32DIRECT(base, 0xB0)


#define IOADDR_ALTERA_TSEMAC_ETHER_STATS_PKTS(base)     __IO_CALC_ADDRESS_NATIVE(base,0xB4)
#define IORD_ALTERA_TSEMAC_ETHER_STATS_PKTS(base)      IORD_32DIRECT(base, 0xB4)


#define IOADDR_ALTERA_TSEMAC_ETHER_STATS_UNDERSIZE_PKTS(base)     __IO_CALC_ADDRESS_NATIVE(base,0xB8)
#define IORD_ALTERA_TSEMAC_ETHER_STATS_UNDERSIZE_PKTS(base)      IORD_32DIRECT(base, 0xB8)


#define IOADDR_ALTERA_TSEMAC_ETHER_STATS_OVERSIZE_PKTS(base)     __IO_CALC_ADDRESS_NATIVE(base,0xBC)
#define IORD_ALTERA_TSEMAC_ETHER_STATS_OVERSIZE_PKTS(base)      IORD_32DIRECT(base, 0xBC)


#define IOADDR_ALTERA_TSEMAC_ETHER_STATS_PKTS_64_OCTETS(base)     __IO_CALC_ADDRESS_NATIVE(base,0xC0)
#define IORD_ALTERA_TSEMAC_ETHER_STATS_PKTS_64_OCTETS(base)      IORD_32DIRECT(base, 0xC0)


#define IOADDR_ALTERA_TSEMAC_ETHER_STATS_PKTS_65_TO_127_OCTETS(base)     __IO_CALC_ADDRESS_NATIVE(base,0xC4)
#define IORD_ALTERA_TSEMAC_ETHER_STATS_PKTS_65_TO_127_OCTETS(base)      IORD_32DIRECT(base, 0xC4)


#define IOADDR_ALTERA_TSEMAC_ETHER_STATS_PKTS_128_TO_255_OCTETS(base)     __IO_CALC_ADDRESS_NATIVE(base,0xC8)
#define IORD_ALTERA_TSEMAC_ETHER_STATS_PKTS_128_TO_255_OCTETS(base)      IORD_32DIRECT(base, 0xC8)


#define IOADDR_ALTERA_TSEMAC_ETHER_STATS_PKTS_256_TO_511_OCTETS(base)     __IO_CALC_ADDRESS_NATIVE(base,0xCC)
#define IORD_ALTERA_TSEMAC_ETHER_STATS_PKTS_256_TO_511_OCTETS(base)      IORD_32DIRECT(base, 0xCC)


#define IOADDR_ALTERA_TSEMAC_ETHER_STATS_PKTS_512_TO_1023_OCTETS(base)     __IO_CALC_ADDRESS_NATIVE(base,0xD0)
#define IORD_ALTERA_TSEMAC_ETHER_STATS_PKTS_512_TO_1023_OCTETS(base)      IORD_32DIRECT(base, 0xD0)


#define IOADDR_ALTERA_TSEMAC_ETHER_STATS_PKTS_1024_TO_1518_OCTETS(base)     __IO_CALC_ADDRESS_NATIVE(base,0xD4)
#define IORD_ALTERA_TSEMAC_ETHER_STATS_PKTS_1024_TO_1518_OCTETS(base)      IORD_32DIRECT(base, 0xD4)


#define IOADDR_ALTERA_TSEMAC_ETHER_STATS_PKTS_1519_TO_X_OCTETS(base)     __IO_CALC_ADDRESS_NATIVE(base,0xD8)
#define IORD_ALTERA_TSEMAC_ETHER_STATS_PKTS_1519_TO_X_OCTETS(base)      IORD_32DIRECT(base, 0xD8)


#define IOADDR_ALTERA_TSEMAC_ETHER_STATS_JABBERS(base)     __IO_CALC_ADDRESS_NATIVE(base,0xDC)
#define IORD_ALTERA_TSEMAC_ETHER_STATS_JABBERS(base)      IORD_32DIRECT(base, 0xDC)


#define IOADDR_ALTERA_TSEMAC_ETHER_STATS_FRAGMENTS(base)     __IO_CALC_ADDRESS_NATIVE(base,0xE0)
#define IORD_ALTERA_TSEMAC_ETHER_STATS_FRAGMENTS(base)      IORD_32DIRECT(base, 0xE0)


/* Register offset 0xE4 reserved */

#define IOADDR_ALTERA_TSEMAC_TX_CMD_STAT(base)     __IO_CALC_ADDRESS_NATIVE(base,0xE8)
#define IORD_ALTERA_TSEMAC_TX_CMD_STAT(base)      IORD_32DIRECT(base, 0xE8)
#define IOWR_ALTERA_TSEMAC_TX_CMD_STAT(base,data) IOWR_32DIRECT(base, 0xE8, data)


#define IOADDR_ALTERA_TSEMAC_RX_CMD_STAT(base)     __IO_CALC_ADDRESS_NATIVE(base,0xEC)
#define IORD_ALTERA_TSEMAC_RX_CMD_STAT(base)      IORD_32DIRECT(base, 0xEC)
#define IOWR_ALTERA_TSEMAC_RX_CMD_STAT(base,data) IOWR_32DIRECT(base, 0xEC, data)


#define ALTERA_TSEMAC_TX_CMD_STAT_OMITCRC_OFST              (17)   
#define ALTERA_TSEMAC_TX_CMD_STAT_OMITCRC_MSK               (0x20000)
#define ALTERA_TSEMAC_TX_CMD_STAT_TXSHIFT16_OFST            (18)   
#define ALTERA_TSEMAC_TX_CMD_STAT_TXSHIFT16_MSK             (0x40000)

#define ALTERA_TSEMAC_RX_CMD_STAT_RXSHIFT16_OFST            (25)   
#define ALTERA_TSEMAC_RX_CMD_STAT_RXSHIFT16_MSK             (0x2000000)

/* Register offset 0xF0 to 0xFC reserved */

 /* 
  * PHY MDIO registers 
  * 
  * For all registers, bits 15:0 are relevant. Bits 31:16 should be written
  * with 0 and ignored on read.
  */

/* Generic access macro for either MDIO port */
#define IOADDR_ALTERA_TSEMAC_MDIO(base, mdio) \
  __IO_CALC_ADDRESS_NATIVE(base, (0x200 + (mdio * 0x80)) )
  
#define IORD_ALTERA_TSEMAC_MDIO(base, mdio, reg_num) \
  IORD_16DIRECT(base, 0x200 + (mdio * 0x80) + (reg_num * sizeof(alt_u32)) )

#define IOWR_ALTERA_TSEMAC_MDIO(base, mdio, reg_num, data) \
  IOWR_16DIRECT(base, 0x200 + (mdio * 0x80) + (reg_num * sizeof(alt_u32)), data)



/* Low word (bits 31:0) of supplemental MAC address 0*/
#define IOADDR_ALTERA_TSEMAC_SMAC_0_0(base)    __IO_CALC_ADDRESS_NATIVE(base,0x300)
#define IORD_ALTERA_TSEMAC_SMAC_0_0(base)      IORD_32DIRECT(base, 0x300)
#define IOWR_ALTERA_TSEMAC_SMAC_0_0(base,data) IOWR_32DIRECT(base, 0x300, data)

/* High half-word (bits 47:32) of supplemental MAC address 0. Upper 16 bits reserved */
#define IOADDR_ALTERA_TSEMAC_SMAC_0_1(base)    __IO_CALC_ADDRESS_NATIVE(base,0x304)
#define IORD_ALTERA_TSEMAC_SMAC_0_1(base)      IORD_32DIRECT(base, 0x304)
#define IOWR_ALTERA_TSEMAC_SMAC_0_1(base,data) IOWR_32DIRECT(base, 0x304, data)

/* Low word (bits 31:0) of supplemental MAC address 1 */
#define IOADDR_ALTERA_TSEMAC_SMAC_1_0(base)    __IO_CALC_ADDRESS_NATIVE(base,0x308)
#define IORD_ALTERA_TSEMAC_SMAC_1_0(base)      IORD_32DIRECT(base, 0x308)
#define IOWR_ALTERA_TSEMAC_SMAC_1_0(base,data) IOWR_32DIRECT(base, 0x308, data)

/* High half-word (bits 47:32) of supplemental MAC address 1. Upper 16 bits reserved */
#define IOADDR_ALTERA_TSEMAC_SMAC_1_1(base)    __IO_CALC_ADDRESS_NATIVE(base,0x30C)
#define IORD_ALTERA_TSEMAC_SMAC_1_1(base)      IORD_32DIRECT(base, 0x30C)
#define IOWR_ALTERA_TSEMAC_SMAC_1_1(base,data) IOWR_32DIRECT(base, 0x30C, data)

/* Low word (bits 31:0) of supplemental MAC address 2 */
#define IOADDR_ALTERA_TSEMAC_SMAC_2_0(base)    __IO_CALC_ADDRESS_NATIVE(base,0x310)
#define IORD_ALTERA_TSEMAC_SMAC_2_0(base)      IORD_32DIRECT(base, 0x310)
#define IOWR_ALTERA_TSEMAC_SMAC_2_0(base,data) IOWR_32DIRECT(base, 0x310, data)

/* High half-word (bits 47:32) of supplemental MAC address 2. Upper 16 bits reserved */
#define IOADDR_ALTERA_TSEMAC_SMAC_2_1(base)    __IO_CALC_ADDRESS_NATIVE(base,0x314)
#define IORD_ALTERA_TSEMAC_SMAC_2_1(base)      IORD_32DIRECT(base, 0x314)
#define IOWR_ALTERA_TSEMAC_SMAC_2_1(base,data) IOWR_32DIRECT(base, 0x314, data)

/* Low word (bits 31:0) of supplemental MAC address 3 */
#define IOADDR_ALTERA_TSEMAC_SMAC_3_0(base)    __IO_CALC_ADDRESS_NATIVE(base,0x318)
#define IORD_ALTERA_TSEMAC_SMAC_3_0(base)      IORD_32DIRECT(base, 0x318)
#define IOWR_ALTERA_TSEMAC_SMAC_3_0(base,data) IOWR_32DIRECT(base, 0x318, data)

/* High half-word (bits 47:32) of supplemental MAC address 3. Upper 16 bits reserved */
#define IOADDR_ALTERA_TSEMAC_SMAC_3_1(base)    __IO_CALC_ADDRESS_NATIVE(base,0x31C)
#define IORD_ALTERA_TSEMAC_SMAC_3_1(base)      IORD_32DIRECT(base, 0x31C)
#define IOWR_ALTERA_TSEMAC_SMAC_3_1(base,data) IOWR_32DIRECT(base, 0x31C, data)


/* Enumeration of commonly-used PHY registers */
#define ALTERA_TSEMAC_PHY_ADDR_CONTROL    0x0
#define ALTERA_TSEMAC_PHY_ADDR_STATUS     0x1
#define ALTERA_TSEMAC_PHY_ADDR_PHY_ID1    0x2
#define ALTERA_TSEMAC_PHY_ADDR_PHY_ID2    0x3
#define ALTERA_TSEMAC_PHY_ADDR_PHY_ADV    0x4
#define ALTERA_TSEMAC_PHY_ADDR_PHY_REMADV 0x5


/* (Original) Register bit definitions and Ethernet MAC device structure */
// COMMAND_CONFIG Register Bits
enum
{
  mmac_cc_TX_ENA_bit        = 0,
  mmac_cc_RX_ENA_bit        = 1,
  mmac_cc_XON_GEN_bit       = 2,
  mmac_cc_ETH_SPEED_bit     = 3,
  mmac_cc_PROMIS_EN_bit     = 4,
  mmac_cc_PAD_EN_bit        = 5,
  mmac_cc_CRC_FWD_bit       = 6,  
  mmac_cc_PAUSE_FWD_bit     = 7,
  mmac_cc_PAUSE_IGNORE_bit  = 8,
  mmac_cc_TX_ADDR__INS_bit  = 9,
  mmac_cc_HD_ENA_bit        = 10,
  mmac_cc_EXCESS_COL_bit    = 11,
  mmac_cc_LATE_COL_bit      = 12,
  mmac_cc_SW_RESET_bit      = 13,
  mmac_cc_MHASH_SEL_bit     = 14,
  mmac_cc_LOOPBACK_bit      = 15,
  mmac_cc_TX_ADDR_SEL_bit   = 16,   // bits 18:16 = address select
  mmac_cc_MAGIC_ENA_bit     = 19,
  mmac_cc_SLEEP_ENA_bit     = 20,
  mmac_cc_WAKEUP_bit        = 21,
  mmac_cc_XOFF_GEN_bit      = 22,
  mmac_cc_CNTL_FRM_ENA_bit  = 23,
  mmac_cc_NO_LENGTH_CHECK_bit  = 24,
  mmac_cc_ENA_10_bit        = 25,
  mmac_cc_RX_ERR_DISC_bit   = 26,
  mmac_cc_CNT_RESET_bit     = 31,
  
  mmac_cc_TX_ENA_mask           = (1 << 0), // enable TX
  mmac_cc_RX_ENA_mask           = (1 << 1), // enable RX
  mmac_cc_XON_GEN_mask          = (1 << 2), // generate Pause frame with Quanta
  mmac_cc_ETH_SPEED_mask        = (1 << 3), // Select Gigabit
  mmac_cc_PROMIS_EN_mask        = (1 << 4), // enable Promiscuous mode
  mmac_cc_PAD_EN_mask           = (1 << 5), // enable padding remove on RX
  mmac_cc_CRC_FWD_mask          = (1 << 6), // forward CRC to application on RX (as opposed to stripping it off)
  mmac_cc_PAUSE_FWD_mask        = (1 << 7), // forward Pause frames to application
  mmac_cc_PAUSE_IGNORE_mask     = (1 << 8), // ignore Pause frames
  mmac_cc_TX_ADDR_INS_mask      = (1 << 9), // MAC overwrites bytes 6 to 12 of frame with address on all transmitted frames
  mmac_cc_HD_ENA_mask           = (1 << 10),// enable half-duplex operation
  mmac_cc_EXCESS_COL_mask       = (1 << 11),// indicator
  mmac_cc_LATE_COL_mask         = (1 << 12),// indicator
  mmac_cc_SW_RESET_mask         = (1 << 13),// issue register and counter reset
  mmac_cc_MHASH_SEL_mask        = (1 << 14),// select multicast hash method
  mmac_cc_LOOPBACK_mask         = (1 << 15),// enable GMII loopback
  mmac_cc_TX_ADDR_SEL_mask      = (1 << 16),// bits 18:16 = address select
  mmac_cc_MAGIC_ENA_mask        = (1 << 19),// enable magic packet detect
  mmac_cc_SLEEP_ENA_mask        = (1 << 20),// enter sleep mode
  mmac_cc_WAKEUP_mask           = (1 << 21),
  mmac_cc_XOFF_GEN_mask         = (1 << 22),
  mmac_cc_CNTL_FRM_ENA_mask     = (1 << 23),
  mmac_cc_NO_LENGTH_CHECK_mask  = (1 << 24), // disable payload length check
  mmac_cc_ENA_10_mask           = (1 << 25),
  mmac_cc_RX_ERR_DISCARD_mask   = (1 << 26),
  mmac_cc_CNT_RESET_mask        = (1 << 31)
};

// TX_CMD_STAT Register bits
enum{
  mmac_tcs_OMIT_CRC_mask          = (1 << 17),
  mmac_tcs_TX_SHIFT16_mask        = (1 << 18)
};


// RX_CMD_STAT Register bits
enum{
  mmac_rcs_RX_SHIFT16_mask          = (1 << 25)
};



// TxConf Register Bits
enum{
  mnet_txc_TYPE_AUTO_mask    = (1 << 0),
  mnet_txc_H2N_IP_mask       = (1 << 1),
  mnet_txc_H2N_PROT_mask     = (1 << 2),
  mnet_txc_IPCHK_mask        = (1 << 3),
  mnet_txc_PROTCHK_mask      = (1 << 4)
};

// RxConf and RxStat register bits
enum{
  mnet_rxc_PADREMOVE_mask    = (1 << 0),
  mnet_rxc_IPERR_DISC_mask   = (1 << 1),
  mnet_rxc_PROTERR_DISC_mask = (1 << 2),
  mnet_rxc_TYPE_REMOVE_mask  = (1 << 3),
  mnet_rxc_N2H_IP_mask       = (1 << 4),
  mnet_rxc_N2H_PROT_mask     = (1 << 5),
  
  mnet_rxs_HDRLEN_mask       = 0x1f,    // 0..4 = header length of IP+Protocol in 32-bit words
  mnet_rxs_IP_CHKERR_mask    = (1 << 5),
  mnet_rxs_PROT_CHKERR_mask  = (1 << 6),
  mnet_rxs_T_REMOVED_mask    = (1 << 7),
  mnet_rxs_VLAN_mask         = (1 << 8),
  mnet_rxs_IPv6_mask         = (1 << 17),
  mnet_rxs_FRAGMENT_mask     = (1 << 18)       // IPv4 fragment
  
};

enum {
        PCS_CTL_speed1           = 1<<6,        // speed select
        PCS_CTL_speed0           = 1<<13,       
        PCS_CTL_fullduplex       = 1<<8,        // fullduplex mode select
        PCS_CTL_an_restart       = 1<<9,        // Autonegotiation restart command
        PCS_CTL_isolate          = 1<<10,       // isolate command
        PCS_CTL_powerdown        = 1<<11,       // powerdown command
        PCS_CTL_an_enable        = 1<<12,       // Autonegotiation enable
        PCS_CTL_rx_slpbk         = 1<<14,       // Serial Loopback enable
        PCS_CTL_sw_reset         = 1<<15        // perform soft reset
        
};

/** PCS Status Register Bits. IEEE 801.2 Clause 22.2.4.2
 */
enum {
        PCS_ST_has_extcap   = 1<<0,             // PHY has extended capabilities registers       
        PCS_ST_rx_sync      = 1<<2,             // RX is in sync (8B/10B codes o.k.)
        PCS_ST_an_ability   = 1<<3,             // PHY supports autonegotiation
        PCS_ST_rem_fault    = 1<<4,             // Autonegotiation completed
        PCS_ST_an_done      = 1<<5
        
};

/** Autonegotiation Capabilities Register Bits. IEEE 802.3 Clause 37.2.1 */

enum {
        ANCAP_NEXTPAGE  = 1 << 15,
        ANCAP_ACK       = 1 << 14,
        ANCAP_RF2       = 1 << 13,
        ANCAP_RF1       = 1 << 12,
        ANCAP_PS2       = 1 << 8,
        ANCAP_PS1       = 1 << 7,
        ANCAP_HD        = 1 << 6,
        ANCAP_FD        = 1 << 5
        // all others are reserved
};     

// MDIO registers within MAC register Space
// memory mapped access
typedef volatile struct np_tse_mdio_struct
{  
  unsigned int CONTROL;
  unsigned int STATUS;
  unsigned int PHY_ID1;
  unsigned int PHY_ID2;
  unsigned int ADV;
  unsigned int REMADV;

  unsigned int reg6;
  unsigned int reg7;
  unsigned int reg8;
  unsigned int reg9;
  unsigned int rega;
  unsigned int regb;
  unsigned int regc;
  unsigned int regd;
  unsigned int rege;
  unsigned int regf;
  unsigned int reg10;
  unsigned int reg11;
  unsigned int reg12;
  unsigned int reg13;
  unsigned int reg14;
  unsigned int reg15;
  unsigned int reg16;
  unsigned int reg17;
  unsigned int reg18;
  unsigned int reg19;
  unsigned int reg1a;
  unsigned int reg1b;
  unsigned int reg1c;
  unsigned int reg1d;
  unsigned int reg1e;
  unsigned int reg1f;

} np_tse_mdio;

typedef volatile struct np_tse_mac_struct
{
  unsigned int REV;
  unsigned int SCRATCH;
  unsigned int COMMAND_CONFIG;
  unsigned int MAC_0;
  unsigned int MAC_1;
  unsigned int FRM_LENGTH;
  unsigned int PAUSE_QUANT;
  unsigned int RX_SECTION_EMPTY;
  unsigned int RX_SECTION_FULL;
  unsigned int TX_SECTION_EMPTY;
  unsigned int TX_SECTION_FULL;
  unsigned int RX_ALMOST_EMPTY;
  unsigned int RX_ALMOST_FULL;
  unsigned int TX_ALMOST_EMPTY;
  unsigned int TX_ALMOST_FULL;
  unsigned int MDIO_ADDR0;
  unsigned int MDIO_ADDR1;
    
  unsigned int reservedx44[5];
  unsigned int REG_STAT;
  unsigned int TX_IPG_LENGTH;
    
  unsigned int aMACID_1;
  unsigned int aMACID_2;
  unsigned int aFramesTransmittedOK;
  unsigned int aFramesReceivedOK;
  unsigned int aFramesCheckSequenceErrors; 
  unsigned int aAlignmentErrors;
  unsigned int aOctetsTransmittedOK;
  unsigned int aOctetsReceivedOK;
  unsigned int aTxPAUSEMACCtrlFrames;
  unsigned int aRxPAUSEMACCtrlFrames;
  unsigned int ifInErrors;
  unsigned int ifOutErrors;
  unsigned int ifInUcastPkts;
  unsigned int ifInMulticastPkts;
  unsigned int ifInBroadcastPkts;
  unsigned int ifOutDiscards;
  unsigned int ifOutUcastPkts;
  unsigned int ifOutMulticastPkts;
  unsigned int ifOutBroadcastPkts;
  unsigned int etherStatsDropEvent;
  unsigned int etherStatsOctets;
  unsigned int etherStatsPkts;
  unsigned int etherStatsUndersizePkts;
  unsigned int etherStatsOversizePkts;
  unsigned int etherStatsPkts64Octets;
  unsigned int etherStatsPkts65to127Octets;
  unsigned int etherStatsPkts128to255Octets;
  unsigned int etherStatsPkts256to511Octets;
  unsigned int etherStatsPkts512to1023Octets;
  unsigned int etherStatsPkts1024to1518Octets;
  unsigned int etherStatsPkts1519toXOctets;
  unsigned int etherStatsJabbers;
  unsigned int etherStatsFragments;
  
  unsigned int reservedxE4;
  unsigned int TX_CMD_STAT;
  unsigned int RX_CMD_STAT;
  
  unsigned int msb_aOctetsTransmittedOK;
  unsigned int msb_aOctetsReceivedOK;
  unsigned int msb_etherStatsOctets;
  unsigned int reservedxFC;  // current frame's IP payload sum result
  
  unsigned int hashtable[64];
  
  np_tse_mdio mdio0;
  np_tse_mdio mdio1;
  
  unsigned int smac0_0;
  unsigned int smac0_1;
  unsigned int smac1_0;
  unsigned int smac1_1;
  unsigned int smac2_0;
  unsigned int smac2_1;
  unsigned int smac3_0;
  unsigned int smac3_1;
  
  unsigned int reservedx320[56];

} np_tse_mac;

#define ALT_MODULE_CLASS_triple_speed_ethernet_0 triple_speed_ethernet
#define TRIPLE_SPEED_ETHERNET_0_BASE 0x4000
#define TRIPLE_SPEED_ETHERNET_0_ENABLE_MACLITE 0
#define TRIPLE_SPEED_ETHERNET_0_FIFO_WIDTH 32
#define TRIPLE_SPEED_ETHERNET_0_IRQ -1
#define TRIPLE_SPEED_ETHERNET_0_IRQ_INTERRUPT_CONTROLLER_ID -1
#define TRIPLE_SPEED_ETHERNET_0_IS_MULTICHANNEL_MAC 0
#define TRIPLE_SPEED_ETHERNET_0_MACLITE_GIGE 0
#define TRIPLE_SPEED_ETHERNET_0_MDIO_SHARED 0
#define TRIPLE_SPEED_ETHERNET_0_NAME "/dev/triple_speed_ethernet_0"
#define TRIPLE_SPEED_ETHERNET_0_NUMBER_OF_CHANNEL 1
#define TRIPLE_SPEED_ETHERNET_0_NUMBER_OF_MAC_MDIO_SHARED 1
#define TRIPLE_SPEED_ETHERNET_0_PCS 1
#define TRIPLE_SPEED_ETHERNET_0_PCS_ID 0u
#define TRIPLE_SPEED_ETHERNET_0_PCS_SGMII 1
#define TRIPLE_SPEED_ETHERNET_0_RECEIVE "sgdma_1"
#define TRIPLE_SPEED_ETHERNET_0_RECEIVE_FIFO_DEPTH 2048
#define TRIPLE_SPEED_ETHERNET_0_REGISTER_SHARED 0
#define TRIPLE_SPEED_ETHERNET_0_SPAN 1024
#define TRIPLE_SPEED_ETHERNET_0_TRANSMIT "sgdma_0"
#define TRIPLE_SPEED_ETHERNET_0_TRANSMIT_FIFO_DEPTH 2048
#define TRIPLE_SPEED_ETHERNET_0_TYPE "triple_speed_ethernet"
#define TRIPLE_SPEED_ETHERNET_0_USE_MDIO 0

/* Define MAC of TSE system */
#define TSE_SYSTEM_MAC(tse_name)          \
    tse_name##_BASE,                      \
    tse_name##_TRANSMIT_FIFO_DEPTH,       \
    tse_name##_RECEIVE_FIFO_DEPTH,        \
    tse_name##_USE_MDIO,                  \
    tse_name##_ENABLE_MACLITE,            \
    tse_name##_MACLITE_GIGE,              \
    tse_name##_IS_MULTICHANNEL_MAC,       \
    tse_name##_NUMBER_OF_CHANNEL,         \
    tse_name##_MDIO_SHARED,               \
    tse_name##_NUMBER_OF_MAC_MDIO_SHARED, \
    tse_name##_PCS,                       \
    tse_name##_PCS_SGMII

/* TSE System Component Structure */
typedef struct alt_tse_system_mac_struct {
    UINT32      tse_mac_base;                     /* Base address of TSE MAC                               */
    UINT16      tse_tx_depth;                     /* TX Receive FIFO depth                                 */
    UINT16      tse_rx_depth;                     /* RX Receive FIFO depth                                 */
    UINT8       tse_use_mdio;                     /* is MDIO enabled                                       */
    UINT8       tse_en_maclite;                   /* is Small MAC                                          */
    UINT8       tse_maclite_gige;                 /* is Small MAC 1000 Mbps                                */
    UINT8       tse_multichannel_mac;             /* MAC group together for MDIO block sharing             */
    UINT8       tse_num_of_channel;               /* Number of channel for Multi-channel MAC               */
    UINT8       tse_mdio_shared;                  /* is MDIO block shared                                  */
    UINT8       tse_number_of_mac_mdio_shared;    /* Number of MAC sharing the MDIO block                  */
    UINT8       tse_pcs_ena;                      /* is MAC+PCS combination                                */
    UINT8       tse_pcs_sgmii;                    /* is SGMII mode of PCS enabled                          */
} alt_tse_system_mac;




#define SUCCESS            0  /* whatever the call was, it worked. */
#define ENP_RESOURCE     -22  /* ran out of other queue-able resource */
#define ENP_PARAM        -10  /* bad parameter */

#define MAXNETS            8  /* max ifaces to support at one time */


void no_printf (char *fmt, ...);

#ifdef ALT_DEBUG
#define tse_dprintf(level, fmt, rest...)        \
    if(level <= TSE_DEBUG_LEVEL) {              \
        printf (fmt, ## rest);                  \
    }                                           \
    else {                                      \
        no_printf (fmt, ## rest);               \
    }
#else
#define tse_dprintf(level, fmt, rest...) no_printf (fmt, ## rest)
#endif /* ALT_DEBUG */




/*** Debug Definition *********/
/* change ENABLE_PHY_LOOPBACK to 1 to enable PHY loopback for debug purpose */ 
#ifndef ENABLE_PHY_LOOPBACK
    #define ENABLE_PHY_LOOPBACK     0
#endif

#ifndef pnull
#define pnull ((void *)0)
#endif

/* Constant definition for tse_system_info.h */
#define TSE_EXT_DESC_MEM                        1
#define TSE_INT_DESC_MEM                        0

#define TSE_USE_SHARED_FIFO                     1
#define TSE_NO_SHARED_FIFO                      0

#define TSE_ENABLE_MDIO_SHARING                 1

/* Multi-channel Shared FIFO Depth Settings */
#ifndef ALTERA_TSE_SHARED_FIFO_TX_DEPTH_DEFAULT
    #define ALTERA_TSE_SHARED_FIFO_TX_DEPTH_DEFAULT     2040
#endif

#ifndef ALTERA_TSE_SHARED_FIFO_RX_DEPTH_DEFAULT
    #define ALTERA_TSE_SHARED_FIFO_RX_DEPTH_DEFAULT     2040
#endif


/* PHY Status definition */
#define TSE_PHY_AUTO_ADDRESS        -1
#define TSE_PHY_MAP_SUCCESS         0
#define TSE_PHY_MAP_ERROR           -1

#define TSE_PHY_AN_NOT_COMPLETE     -1
#define TSE_PHY_AN_NOT_CAPABLE      -2
#define TSE_PHY_AN_COMPLETE         0
#define TSE_PHY_SPEED_INVALID       3
#define TSE_PHY_SPEED_1000          2
#define TSE_PHY_SPEED_100           1
#define TSE_PHY_SPEED_10            0
#define TSE_PHY_SPEED_NO_COMMON     -1
#define TSE_PHY_DUPLEX_FULL         1
#define TSE_PHY_DUPLEX_HALF         0

/* getPHYSpeed return error */
enum {
    ALT_TSE_E_NO_PMAC_FOUND             = (1 << 23),
    ALT_TSE_E_NO_MDIO                   = (1 << 22),
    ALT_TSE_E_NO_PHY                    = (1 << 21),
    ALT_TSE_E_NO_COMMON_SPEED           = (1 << 20),
    ALT_TSE_E_AN_NOT_COMPLETE           = (1 << 19),
    ALT_TSE_E_NO_PHY_PROFILE            = (1 << 18),
    ALT_TSE_E_PROFILE_INCORRECT_DEFINED = (1 << 17),
    ALT_TSE_E_INVALID_SPEED             = (1 << 16)
};

/* Maximum number of PHY can be registered into PHY profile */
#define TSE_MAX_PHY_PROFILE     MAXNETS

/* Maximum MAC in system */
#define TSE_MAX_MAC_IN_SYSTEM   MAXNETS
#define TSE_MAX_CHANNEL         MAXNETS


/* System Constant Definition Used in the TSE Driver Code */


#define ALTERA_TSE_SW_RESET_TIME_OUT_CNT        10000
#define ALTERA_TSE_SGDMA_BUSY_TIME_OUT_CNT      1000000 

#define ALTERA_TSE_SGDMA_RX_DESC_CHAIN_SIZE     20

#define ALTERA_TSE_FIRST_TX_SGDMA_DESC_OFST     0
#define ALTERA_TSE_SECOND_TX_SGDMA_DESC_OFST    1
#define ALTERA_TSE_FIRST_RX_SGDMA_DESC_OFST     2
#define ALTERA_TSE_SECOND_RX_SGDMA_DESC_OFST    3
#define ALTERA_TSE_MAC_MAX_FRAME_LENGTH         1518

#if ALTERA_TSE_SGDMA_RX_DESC_CHAIN_SIZE > 1
    #define ALTERA_TSE_SGDMA_INTR_MASK              ALTERA_AVALON_SGDMA_CONTROL_IE_DESC_COMPLETED_MSK | ALTERA_AVALON_SGDMA_CONTROL_IE_CHAIN_COMPLETED_MSK | ALTERA_AVALON_SGDMA_CONTROL_IE_GLOBAL_MSK
#else
    #define ALTERA_TSE_SGDMA_INTR_MASK              ALTERA_AVALON_SGDMA_CONTROL_IE_CHAIN_COMPLETED_MSK | ALTERA_AVALON_SGDMA_CONTROL_IE_GLOBAL_MSK
#endif

#define ALTERA_TSE_FULL_MAC                     0
#define ALTERA_TSE_MACLITE_10_100               1
#define ALTERA_TSE_MACLITE_1000                 2

#define ALTERA_TSE_NO_INDEX_FOUND               -1
#define ALTERA_TSE_SYSTEM_DEF_ERROR             -1
#define ALTERA_TSE_MALLOC_FAILED                -1

#define ALTERA_TSE_DUPLEX_MODE_DEFAULT          TSE_PHY_DUPLEX_FULL
#define ALTERA_TSE_MAC_SPEED_DEFAULT            TSE_PHY_SPEED_100
#define ALTERA_AUTONEG_TIMEOUT_THRESHOLD        250000
#define ALTERA_CHECKLINK_TIMEOUT_THRESHOLD      1000000
#define ALTERA_NOMDIO_TIMEOUT_THRESHOLD         1000000
#define ALTERA_DISGIGA_TIMEOUT_THRESHOLD        5000000

#define ALTERA_TSE_PCS_IF_MODE                  0x14        /* 0x14th register of ALTERA PCS */

/* PHY ID, backward compatible */
#define NTL848PHY_ID    0x20005c90  /* National 83848, 10/100 */
#define MTIPPCS_ID      0x00010000  /* MTIP 1000 Base-X PCS */
#define TDKPHY_ID       0x0300e540  /* TDK 78Q2120 10/100 */
#define NTLPHY_ID       0x20005c7a  /* National DP83865 */
#define MVLPHY_ID       0x0141      /* Marvell 88E1111 */



/* PHY ID */
/* Marvell PHY on PHYWORKX board */
enum {
    MV88E1111_OUI       = 0x005043,
    MV88E1111_MODEL     = 0x0c,
    MV88E1111_REV       = 0x2
};

/* Marvell Quad PHY on PHYWORKX board */
enum {
    MV88E1145_OUI       = 0x005043,
    MV88E1145_MODEL     = 0x0d,
    MV88E1145_REV       = 0x2
};

/* National PHY on PHYWORKX board */
enum {
    DP83865_OUI       = 0x080017,
    DP83865_MODEL     = 0x07,
    DP83865_REV       = 0xa
};

/* National 10/100 PHY on PHYWORKX board */
enum {
    DP83848C_OUI       = 0x080017,
    DP83848C_MODEL     = 0x09,
    DP83848C_REV       = 0x0
};



/* PHY register definition */
enum {
    TSE_PHY_MDIO_CONTROL    = 0,
    TSE_PHY_MDIO_STATUS     = 1,
    TSE_PHY_MDIO_PHY_ID1    = 2,
    TSE_PHY_MDIO_PHY_ID2    = 3,
    TSE_PHY_MDIO_ADV        = 4,
    TSE_PHY_MDIO_REMADV     = 5,
    
    TSE_PHY_MDIO_AN_EXT             = 6,
    TSE_PHY_MDIO_1000BASE_T_CTRL    = 9,
    TSE_PHY_MDIO_1000BASE_T_STATUS  = 10,
    TSE_PHY_MDIO_EXT_STATUS         = 15    
};

/* MDIO CONTROL bit number */
enum {
    TSE_PHY_MDIO_CONTROL_RESET      = 15,
    TSE_PHY_MDIO_CONTROL_LOOPBACK   = 14,
    TSE_PHY_MDIO_CONTROL_SPEED_LSB  = 13,
    TSE_PHY_MDIO_CONTROL_AN_ENA     = 12,
    TSE_PHY_MDIO_CONTROL_POWER_DOWN = 11,
    TSE_PHY_MDIO_CONTROL_ISOLATE    = 10,
    TSE_PHY_MDIO_CONTROL_RESTART_AN = 9,
    TSE_PHY_MDIO_CONTROL_DUPLEX     = 8,
    TSE_PHY_MDIO_CONTROL_SPEED_MSB  = 6
};

/* MDIO STATUS bit number */
enum {
    TSE_PHY_MDIO_STATUS_100BASE_T4      = 15,
    TSE_PHY_MDIO_STATUS_100BASE_X_FULL  = 14,
    TSE_PHY_MDIO_STATUS_100BASE_X_HALF  = 13,
    TSE_PHY_MDIO_STATUS_10BASE_T_FULL   = 12,
    TSE_PHY_MDIO_STATUS_10BASE_T_HALF   = 11,
    TSE_PHY_MDIO_STATUS_100BASE_T2_FULL = 10,
    TSE_PHY_MDIO_STATUS_100BASE_T2_HALF = 9,
    TSE_PHY_MDIO_STATUS_EXT_STATUS      = 8,
    TSE_PHY_MDIO_STATUS_AN_COMPLETE     = 5,
    TSE_PHY_MDIO_STATUS_AN_ABILITY      = 3,
    TSE_PHY_MDIO_STATUS_LINK_STATUS     = 2
};

/* AN Advertisement bit number */
/* and also */
/* Link Partner Ability bit number */
enum {
    TSE_PHY_MDIO_ADV_100BASE_T4       = 9,
    TSE_PHY_MDIO_ADV_100BASE_TX_FULL  = 8,
    TSE_PHY_MDIO_ADV_100BASE_TX_HALF  = 7,
    TSE_PHY_MDIO_ADV_10BASE_TX_FULL   = 6,
    TSE_PHY_MDIO_ADV_10BASE_TX_HALF   = 5
};

/* AN Expansion bit number */
enum {
    TSE_PHY_MDIO_LP_AN_ABLE     = 0
};

/* 1000BASE-T Control bit number */
enum {
    TSE_PHY_MDIO_1000BASE_T_CTRL_FULL_ADV   = 9,
    TSE_PHY_MDIO_1000BASE_T_CTRL_HALF_ADV   = 8
};

/* 1000BASE-T Status bit number */
enum {
    TSE_PHY_MDIO_1000BASE_T_STATUS_LP_FULL_ADV   = 11,
    TSE_PHY_MDIO_1000BASE_T_STATUS_LP_HALF_ADV   = 10
};

/* Extended Status bit number */
enum {
    TSE_PHY_MDIO_EXT_STATUS_1000BASE_X_FULL = 15,
    TSE_PHY_MDIO_EXT_STATUS_1000BASE_X_HALF = 14,
    TSE_PHY_MDIO_EXT_STATUS_1000BASE_T_FULL = 13,
    TSE_PHY_MDIO_EXT_STATUS_1000BASE_T_HALF = 12
};





/* 
 * macros to access SGDMA Descriptors used in the TSE driver 
 * - use the macros to assure cache coherancy 
 */
#define IORD_ALTERA_TSE_SGDMA_DESC_READ_ADDR(base)                      (IORD(base, 0x0) & 0xFFFFFFFF)
#define IOWR_ALTERA_TSE_SGDMA_DESC_READ_ADDR(base, data)                IOWR(base, 0x0, data)
#define IORD_ALTERA_TSE_SGDMA_DESC_WRITE_ADDR(base)                     (IORD(base, 0x2) & 0xFFFFFFFF)
#define IOWR_ALTERA_TSE_SGDMA_DESC_WRITE_ADDR(base, data)               IOWR(base, 0x2, data)
#define IORD_ALTERA_TSE_SGDMA_DESC_NEXT(base)                           (IORD(base, 0x4) & 0xFFFFFFFF)
#define IOWR_ALTERA_TSE_SGDMA_DESC_NEXT(base, data)                     IOWR(base, 0x4, data)

#define IORD_ALTERA_TSE_SGDMA_DESC_BYTES_TO_TRANSFER(base)              (IORD(base, 0x6) & 0xFFFF)
#define IOWR_ALTERA_TSE_SGDMA_DESC_BYTES_TO_TRANSFER(base, data)        IOWR(base, 0x6, ((IORD(base, 0x6) & 0xFFFF0000) | data))
#define IORD_ALTERA_TSE_SGDMA_DESC_READ_BURST(base)                     (((IORD(base, 0x6)) >> 16) & 0xFF)
#define IOWR_ALTERA_TSE_SGDMA_DESC_READ_BURST(base, data)               IOWR(base, 0x6, (IORD(base, 0x6) & 0xFF00FFFF) | (data << 16))
#define IORD_ALTERA_TSE_SGDMA_DESC_WRITE_BURST(base)                    (((IORD(base, 0x6)) >> 24) & 0xFF)
#define IOWR_ALTERA_TSE_SGDMA_DESC_WRITE_BURST(base, data)              IOWR(base, 0x6, ((IORD(base, 0x6) & 0x00FFFFFF) | (data << 24)))

#define IORD_ALTERA_TSE_SGDMA_DESC_ACTUAL_BYTES_TRANSFERRED(base)       (IORD(base, 0x7) & 0xFFFF)
#define IOWR_ALTERA_TSE_SGDMA_DESC_ACTUAL_BYTES_TRANSFERRED(base, data) IOWR(base, 0x7, ((IORD(base, 0x7) & 0xFFFF0000) | data))
#define IORD_ALTERA_TSE_SGDMA_DESC_STATUS(base)                         (((IORD(base, 0x7)) >> 16) & 0xFF)
#define IOWR_ALTERA_TSE_SGDMA_DESC_STATUS(base, data)                   IOWR(base, 0x7, (IORD(base, 0x7) & 0xFF00FFFF) | (data << 16))
#define IORD_ALTERA_TSE_SGDMA_DESC_CONTROL(base)                        (((IORD(base, 0x7)) >> 24) & 0xFF)
#define IOWR_ALTERA_TSE_SGDMA_DESC_CONTROL(base, data)                  IOWR(base, 0x7, ((IORD(base, 0x7) & 0x00FFFFFF) | (data << 24)))

/* HAL initialization macros */

#define ALT_LLIST_ENTRY {0, 0}
#define ALT_IRQ_NOT_CONNECTED (-1)

typedef struct emac_desc
    {
    volatile UINT32 next;
    volatile UINT32 buffer;
    volatile UINT32 buflen_off;
    volatile UINT32 pktlen_flags;
    } EMAC_DESC;


#if 0

#define EMAC_MOD_NUM            3

    
typedef struct emac_drv_ctrl
    {
    END_OBJ         emacEndObj;
    VXB_DEVICE_ID   emacDev;
    void *          emacMuxDevCookie;

    JOB_QUEUE_ID    emacJobQueue;
    QJOB            emacIntJob;
    volatile BOOL   emacIntPending;
    QJOB            emacRxJob;
    volatile BOOL   emacRxPending;
    QJOB            emacTxJob;
    volatile BOOL   emacTxPending;

    volatile BOOL   emacTxStall;
    volatile BOOL   emacRxStall;

    BOOL            emacPolling;
    M_BLK_ID        emacPollBuf;
    UINT32          emacIntMask;
    UINT32          emacIntrs;

    UINT8           emacAddr[ETHER_ADDR_LEN];

    END_CAPABILITIES    emacCaps;

    END_IFDRVCONF   emacEndStatsConf;
    END_IFCOUNTERS  emacEndStatsCounters;

    /* begin MII/ifmedia required fields */

    END_MEDIALIST * emacMediaList;
    END_ERR         emacLastError;
    UINT32          emacCurMedia;
    UINT32          emacCurStatus;
    VXB_DEVICE_ID   emacMiiBus;
    VXB_DEVICE_ID   emacMiiDev;
    FUNCPTR         emacMiiPhyRead;
    FUNCPTR         emacMiiPhyWrite;
    int             emacMiiPhyAddr;

    /* end MII/ifmedia required fields */

    EMAC_DESC *     emacRxDescMem;
    EMAC_DESC *     emacTxDescMem;

    M_BLK_ID        emacRxMblk[EMAC_RX_DESC_CNT];
    M_BLK_ID        emacTxMblk[EMAC_TX_DESC_CNT];

    UINT32          emacTxProd;
    UINT32          emacTxCons;
    UINT32          emacTxFree;
    UINT32          emacRxIdx;

    UINT32          emacTxPend;
    UINT32          emacTxCnt;

    SEM_ID          emacMiiSem;

    int             emacMaxMtu;

    void *          regBase[EMAC_MOD_NUM];
    void *          handle[EMAC_MOD_NUM];

    UINT32          emacDescMem;
    UINT32          emacFreq;
    } EMAC_DRV_CTRL;
#endif

#if 1




typedef struct 
{
	unsigned short nDevID;
	unsigned int IsStarted;	
	unsigned int IsReady;	
	unsigned int Options;	
}ATSEMAC;

typedef struct {
    UINT32   *read_addr;
    UINT32    read_addr_pad;

    UINT32   *write_addr;
    UINT32    write_addr_pad;

    UINT32   *next;
    UINT32    next_pad;
    
    UINT16    bytes_to_transfer;
    UINT8    read_burst;
    UINT8    write_burst;

    UINT16    actual_bytes_transferred;
    UINT8    status;
    UINT8    control;

} ATSE_SGDMA_DESCRIPTOR;



#endif

#define ATSEMAC_TUPLE_CNT          384

#define ETH_MAC_ADDR_SIZE          6	   
#define ATSEMAC_TIMEOUT            10000
#define ATSEMAC_MAX_RX             10

#define ALTERA_TSE_MIN_MTU_SIZE    14

#define ATSE_TX_CNT           4

#define ATSE_MAX_RX           4
#define EMAC_MAXFRAG            16


#define ATSE_MTU              1500
#define ATSE_TIMEOUT          100000

#define ATSE_TX_INC(x, y)     (x) = (((x) + 1) % y)
#define EMAC_INC_DESC(x, y)     (x) = (((x) + 1) % y)



#if (_WRS_VXWORKS_MAJOR >= 6)
#  define COUNT_IN_PKT(MblkPtr)      END_ERR_ADD(&pDrvCtrl->endObj, MIB2_IN_UCAST, 1)
#  define COUNT_OUT_PKT(MblkPtr)     END_ERR_ADD(&pDrvCtrl->endObj, MIB2_OUT_UCAST, 1)
#  define COUNT_IN_DISCARD(MblkPtr)  endM2Packet(&pDrvCtrl->endObj, (MblkPtr), M2_PACKET_IN_DISCARD)
#  define COUNT_OUT_DISCARD(MblkPtr) endM2Packet(&pDrvCtrl->endObj, (MblkPtr), M2_PACKET_OUT_DISCARD)
#  define COUNT_IN_ERROR()           endM2Packet(&pDrvCtrl->endObj, NULL,      M2_PACKET_IN_ERROR)
#  define COUNT_OUT_ERROR(MblkPtr)   endM2Packet(&pDrvCtrl->endObj, (MblkPtr), M2_PACKET_OUT_ERROR)
#else
#  define COUNT_IN_PKT(MblkPtr)      END_ERR_ADD(&DriverPtr->VxEnd, MIB2_IN_UCAST, 1)
#  define COUNT_OUT_PKT(MblkPtr)     END_ERR_ADD(&DriverPtr->VxEnd, MIB2_OUT_UCAST, 1)
#  define COUNT_IN_DISCARD(MblkPtr)
#  define COUNT_OUT_DISCARD(MblkPtr)
#  define COUNT_IN_ERROR()           END_ERR_ADD(&DriverPtr->VxEnd, MIB2_IN_ERRS, 1)
#  define COUNT_OUT_ERROR(MblkPtr)   END_ERR_ADD(&DriverPtr->VxEnd, MIB2_OUT_ERRS, 1)
#endif

#ifdef __cplusplus
}
#endif

#endif  

  
