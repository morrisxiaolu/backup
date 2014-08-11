#include "simpleprintf.h"


/* includes */

#include <vxWorks.h>
#include <ctype.h>
#include <fioLib.h>
#include <stdio.h>
#include <string.h>

#include <bootApp.h>

#ifdef DebugPrint

void printstr(char *s)
{
   unsigned long port = PORT3f8;
   while (*s) {
     while (((*(volatile unsigned char*)(PORT3f8+5)) & 0x20)==0);
     *(unsigned char*)port = *s;
     s++;
   } 
   while (((*(volatile unsigned char*)(PORT3f8+5)) & 0x20)==0);
}  

void printstr1(char *s)
{
   unsigned long port = PORT3f8;

     while (*s) {
     *(unsigned char*)port = *s;
     s++;
   } 
}  

void printnum(unsigned long long n)
{
  int i,j;
  unsigned char a[40];
  unsigned long port = PORT3f8;
  i = 0;
  do {
   a[i] = n % 16;
   n = n / 16;
   i++;
  }while(n);
 
  for (j=i-1;j>=0;j--) {
   if (a[j]>=10) {
     while (((*(volatile unsigned char*)(PORT3f8+5)) & 0x20)==0);
     *(unsigned char*)port = 'a' + a[j] - 10;
   }else{
     while (((*(volatile unsigned char*)(PORT3f8+5)) & 0x20)==0);
     *(unsigned char*)port = '0' + a[j];
   }
  }
  /*printstr("\r\n");*/
}

void printnum0(unsigned long long n)
{
  int i,j;
  unsigned char a[40];
  unsigned long port = PORT2f8;
  i = 0;
  do {
   a[i] = n % 16;
   n = n / 16;
   i++;
  }while(n);
 
  for (j=i-1;j>=0;j--) {
   if (a[j]>=10) {
     while (((*(volatile unsigned char*)(PORT2f8+5)) & 0x20)==0);
     *(unsigned char*)port = 'a' + a[j] - 10;
   }else{
     while (((*(volatile unsigned char*)(PORT2f8+5)) & 0x20)==0);
     *(unsigned char*)port = '0' + a[j];
   }
  }
  /*printstr("\r\n");*/
}


void printnum1(unsigned long long n)
{
  int i,j;
  unsigned char a[16];
  unsigned long port = PORT3f8;

   i = 0;
  do {
   a[i] = n % 16;
   n = n / 16;
   i++;
  }while(n);
 
  for (j=i-1;j>=0;j--) {
   if (a[j]>=10) {
     *(volatile unsigned char*)port = 'a' + a[j] - 10;
   }else{
     *(volatile unsigned char*)port = '0' + a[j];
   }
  }
  printstr1("\r\n");
}

void CPU_WRITE32_REG(unsigned int address,unsigned int cpunum,unsigned int data)
{
	CPU_WRITE32(address,cpunum,data);
}

void CPU_WRITE16_REG(unsigned int address,unsigned int cpunum,unsigned short data)
{
	CPU_WRITE16(address,cpunum,data);
}

unsigned short CPU_READ16_REG(unsigned int address, unsigned int cpunum)
{
	return CPU_READ16(address,cpunum);
}

unsigned char CPU_READ8_REG(unsigned int address, unsigned int cpunum)
{
	return CPU_READ8(address,cpunum);
}
void CPU_WRITE8_REG(unsigned int address,unsigned int cpunum,unsigned char data)
{
	CPU_WRITE8(address,cpunum,data);
}

unsigned int CPU_READ32_REG(unsigned int address, unsigned int cpunum)
{
	return CPU_READ32(address,cpunum);
}
#endif




__asm volatile unsigned int readHt0_fdfb_asm(unsigned int address)
{
% reg address;
! "$2"
    .set noreorder 
    dli t0,0x90000cFDFB000000
    or  t0,t0,address
    lw  $2, 0(t0)
    .set reorder
}

__asm volatile void writeHt0_fdfb_asm(unsigned int address, unsigned int data)
{
% reg address; reg data;                     
	.set noreorder 
	dli t0,0x90000cFDFB000000
	or  t0,t0,address
	sw  data,0(t0)
	/*sync*/
	.set reorder
}

unsigned int readHt0_fdfb(unsigned int address)
{
	return readHt0_fdfb_asm(address);
}


void writeHt0_fdfb(unsigned int address, unsigned int data)
{
	writeHt0_fdfb_asm(address, data);
}


void displayHt0_fdfb(unsigned int address, unsigned int nwords)
{
    
    char *        adrs = 0;    /* address to display */

    static int    dNbytes = 128;
    char        ascii [17];
    FAST int    nbytes;
    FAST int    byte;
    unsigned int value; 

    ascii [16] = '\0';           /* put an EOS on the string */

    nbytes = 2 * nwords;

    if (nbytes == 0)
    nbytes = dNbytes;   /* no count specified: use current byte count */
    else
    dNbytes = nbytes;   /* change current byte count */

    /* round adrs down to word boundary */
    adrs = (char *) ((int) address & ~3);  

    /* print leading spaces on first line */

    bfill ((char *) ascii, 16, '.');

    printf ("start address offset: 0x%08x %08x:  \r\n",0x90000cfd,(int) adrs | 0xfb000000,0,0,0,0,0);

    for (byte = 0; byte < ((int) adrs & 0xf); byte += 4)
    {
        printf ("        ",0,0,0,0,0,0);
        
        /* space between words */
        printf (" ",0,0,0,0,0,0);  
    }


    /* print out all the words */

    while (nbytes > 0)
    {

        value = readHt0_fdfb((unsigned int)adrs);

        printf ("%08x",  value , 0,0,0,0,0);
                      
        printf (" ",  0 , 0,0,0,0,0);
        
    
        adrs +=4;
        nbytes -=4;

        if(0 == ((int) adrs & 0xf))
        {
            printf ("\n",  0 , 0,0,0,0,0);
        }
    }

    printf ("\n",  0 , 0,0,0,0,0);
}



__asm volatile unsigned int readHt0_0asm(unsigned int address)
{
% reg address;
! "$2"
    .set noreorder 
    dli t0,0x90000c0000000000
    or  t0,t0,address
    lw  $2, 0(t0)
    .set reorder
}

__asm volatile void writeHt0_0asm(unsigned int address, unsigned int data)
{
% reg address; reg data;                     
	.set noreorder 
	dli t0,0x90000c0000000000
	or  t0,t0,address
	sw  data,0(t0)
	/*sync*/
	.set reorder
}

unsigned int readHt0_0(unsigned int address)
{
	return readHt0_0asm(address);
}


void writeHt0_0(unsigned int address, unsigned int data)
{
	writeHt0_0asm(address, data);
}


void displayHt0_0(unsigned int address, unsigned int nwords)
{
    
    char *        adrs = 0;    /* address to display */

    static int    dNbytes = 128;

    FAST int    nbytes;
    FAST int    byte;
    unsigned int value; 


    nbytes = 2 * nwords;

    if (nbytes == 0)
    nbytes = dNbytes;   /* no count specified: use current byte count */
    else
    dNbytes = nbytes;   /* change current byte count */

    /* round adrs down to word boundary */
    adrs = (char *) ((int) address & ~3);  

    /* print leading spaces on first line */


    printf ("start address offset: 0x%08x %08x:  \r\n",0x90000c00,(int) adrs | 0x00000000,0,0,0,0,0);

    for (byte = 0; byte < ((int) adrs & 0xf); byte += 4)
    {
        printf ("        ",0,0,0,0,0,0);
        
        /* space between words */
        printf (" ",0,0,0,0,0,0);  
    }


    /* print out all the words */

    while (nbytes > 0)
    {

        value = readHt0_0((unsigned int)adrs);

        printf ("%08x",  value , 0,0,0,0,0);
                      
        printf (" ",  0 , 0,0,0,0,0);
        
    
        adrs +=4;
        nbytes -=4;

        if(0 == ((int) adrs & 0xf))
        {
            printf ("\n",  0 , 0,0,0,0,0);
        }
    }

    printf ("\n",  0 , 0,0,0,0,0);
}




__asm volatile unsigned int readHt0_fdfe_asm(unsigned int address)
{
% reg address;
! "$2"
    .set noreorder 
    dli t0,0x90000cfdfe000000
    or  t0,t0,address
    lw  $2, 0(t0)
    .set reorder
}

__asm volatile void writeHt0_fdfe_asm(unsigned int address, unsigned int data)
{
% reg address; reg data;                     
	.set noreorder 
	dli t0,0x90000cfdfe000000
	or  t0,t0,address
	sw  data,0(t0)
	/*sync*/
	.set reorder
}

unsigned int readHt0_fdfe(unsigned int address)
{
	return readHt0_fdfe_asm(address);
}


void writeHt0_fdfe(unsigned int address, unsigned int data)
{
	writeHt0_fdfe_asm(address, data);
}


void displayHt0_fdfe(unsigned int address, unsigned int nwords)
{
    
    char *        adrs = 0;    /* address to display */

    static int    dNbytes = 128;

    FAST int    nbytes;
    FAST int    byte;
    unsigned int value; 

 
    nbytes = 2 * nwords;

    if (nbytes == 0)
    nbytes = dNbytes;   /* no count specified: use current byte count */
    else
    dNbytes = nbytes;   /* change current byte count */

    /* round adrs down to word boundary */
    adrs = (char *) ((int) address & ~3);  

    /* print leading spaces on first line */


    printf ("start address offset: 0x%08x %08x:  \r\n",0x90000cfd,(int) adrs | 0xfe000000,0,0,0,0,0);

    for (byte = 0; byte < ((int) adrs & 0xf); byte += 4)
    {
        printf ("        ",0,0,0,0,0,0);
        
        /* space between words */
        printf (" ",0,0,0,0,0,0);  
    }


    /* print out all the words */

    while (nbytes > 0)
    {

        value = readHt0_fdfe((unsigned int)adrs);

        printf ("%08x",  value , 0,0,0,0,0);
                      
        printf (" ",  0 , 0,0,0,0,0);
        
    
        adrs +=4;
        nbytes -=4;

        if(0 == ((int) adrs & 0xf))
        {
            printf ("\n",  0 , 0,0,0,0,0);
        }
    }

    printf ("\n",  0 , 0,0,0,0,0);
}





