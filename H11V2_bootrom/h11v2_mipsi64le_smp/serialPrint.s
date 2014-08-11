/*
 * Simple character printing routine used before full initialization
 * Added by yinwx for debug, 2009-11-20
 */
#define COM1_BASE_VADDR  0xbfe001e0

	.rdata
hexchar:
	.ascii	"0123456789abcdef"
testchar:
	.asciz	"test vxworks"

#define	NS16550_DATA	0
#define	NS16550_IER	    1
#define	NS16550_IIR	    2
#define	NS16550_FIFO	2
#define	NS16550_CFCR	3
#define	NS16550_MCR	    4
#define	NS16550_LSR	    5
#define	NS16550_MSR	    6	
#define	NS16550_SCR	    7

#ifndef NSREG
#define NSREG(x)	x
#endif

/* line status register */
#define	LSR_RCV_FIFO	0x80	/* error in receive fifo */
#define	LSR_TSRE	0x40	/* transmitter empty */
#define	LSR_TXRDY	0x20	/* transmitter ready */
#define	LSR_BI		0x10	/* break detected */
#define	LSR_FE		0x08	/* framing error */
#define	LSR_PE		0x04	/* parity error */
#define	LSR_OE		0x02	/* overrun error */
#define	LSR_RXRDY	0x01	/* receiver ready */
#define	LSR_RCV_MASK	0x1f

	.text
	.align	2
	
.global stringserial
.global hexserial
.global tgt_putchar

#define	RELOC(toreg,address) \
	bal	9f; \
9:; \
	la	toreg,address; \
	addu	toreg,ra; \
	la	ra,9b; \
	subu	toreg,ra

#define PRINTSTR(x) \
    .rdata;98: .asciz x; .text; la a0, 98b; bal stringserial; nop

/***************************************************/
	.ent	stringserial
stringserial:
	move	a2, ra
	move	a1, a0
	lbu	    a0, 0(a1)
1:
	beqz	a0, 2f
	nop
	bal	tgt_putchar
	addiu	a1, 1
	b	1b
	lbu	a0, 0(a1)

2:
	j	a2
	nop
	.end	stringserial
	
/***************************************************/
	.ent	hexserial
hexserial:

	move a2, ra
	move a1, a0
	li	 a3, 7
1:
	rol	a0, a1, 4
	move	a1, a0
	and	a0, 0xf
	
	.set reorder
	RELOC(v0,hexchar)
	.set noreorder
	
	addu v0, a0
	bal	tgt_putchar
	lbu	a0, 0(v0)
	
	bnez	a3, 1b
	addu	a3, -1

	j	a2
	nop

	.end	hexserial

/***************************************************/
	.ent	tgt_putchar
tgt_putchar:

	la	v0, COM1_BASE_VADDR
1:
	lbu	v1, NSREG(NS16550_LSR)(v0)
	and	v1, LSR_TXRDY
	beqz	v1, 1b
	nop

	sb	a0, NSREG(NS16550_DATA)(v0)
	
#ifdef HAVE_NB_SERIAL
	move	v1, v0
	la	v0, COM3_BASE_VADDR
	bne	v0, v1, 1b
	nop
#endif

	j	ra
	nop	

	.end	tgt_putchar
