/* usrNetBootConfig.c - Configure the network boot device */

/*
 * Copyright (c) 2001-2008 Wind River Systems, Inc.
 *
 * The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.
 */

/*
modification history
--------------------
02d,07jul08,pgh  Add support for SATA.
02c,15oct07,uol  Adjustment of fix for defect WIND00104919, limit interface
                 names to IFNAMSIZ to adhere to ipnet_cmd_ifconfig().
02b,09oct07,uol  Fixed defect WIND00104919, allow 64 byte device name in
                 usrNetBootConfig(), and return error if longer.
02a,27aug07,tkf  Add IPv6Only configuration support.
01z,28apr07,tkf  Merge from NOR-SMP sandbox.
01y,02feb07,tkf  Use ipcom_mcmd_ifconfig() and ipcom_mcmd_route() instead of
                 ipnet_cmd_ifconfig() and ipnet_cmd_route().
01x,29jan07,jmt  Add support for USB disks
01x,02feb07,tkf  Use function pointers to access ifconfig and route commands.
01w,05oct06,kch  Replaced do_autoconfig() and usrNetIpv6Attach() with
                 usrNetBootAutoConfig() and addGateway() with
                 usrNetBootAddGateway(). Updated usrNetBootConfig() to work
                 with IPNet.
01v,24feb05,spm  removed unneeded routeFormat values and macros (SPR #100995)
01u,24feb05,spm  performance updates and code cleanup (SPR #100995)
01t,30sep04,dlk  Removed obsolete declaration of random().
01s,28sep04,niq  Zero out the sockaddr structures in the addGateway routine.
01r,17sep04,niq  scale out routing sockets
01q,04aug04,vvv  check inet_addr() return value (SPR #94763)
01p,19may04,snd  Fixed SPR#89215,DHCP client unable to obtain lease when
                 booting from non-network device 
01o,07may04,vvv  fixed warnings (SPR #96364)
01n,10feb04,elp  usrNetBootConfig() returns without message when devName==NULL.
01m,06jan04,rp   fixed SPR 92684
01l,16oct02,ham  fixed improper default gateway configuration (SPR 83122).
01k,11oct02,ham  fixed improper netIoctl return value check.
01j,04oct02,ham  fixed improper ifioctl return value check (SPR 82778).
01i,18sep02,nee  removing ifr_type for SIOCSIFFLAGS
01h,22jul02,rvr  dynamic memory allocation/free for rtmbuf (teamf1)
01g,15jul02,nee  SIOCSIFADDR replaced by SIOCSIFADDR+SIOCSIFFLGS
01f,27feb02,ham  wrote addGateway to add default gateway by routing socket.
01e,06feb02,ham  renamed to usrNetBootConfig due to ifconfig.
01d,05dec01,ham  consolidated usrNetConfigIf.c.
*/

/* 
DESCRIPTION
This file is included by the configuration tool to setup a network device
according to the parameters in the boot line. It contains the initialization
routine for the INCLUDE_NET_BOOT_CONFIG component.
*/

#include <ipProto.h> /*zxj*/
#include <bootLib.h>
#include <ipcom_perrno.h>
#include <ipcom_type.h>
#include <ipcom_err.h>
#include <ipcom_inet.h>
#include <stdio.h>
#include <ctype.h>
#include <fioLib.h>
#include <muxLib.h>
#include <string.h>
#include <End.h>

#include "config.h"
#include "sysLib.h"
#include "simpleprintf.h"
#undef  BOARD_HRDP
#define BOARD_HRSIGNAL
#ifdef  BOARD_HRDP
#include "usrNetIpAttachCommon.c"
#endif
#if 1
#include "usrNetIpAttachCommon.c"

extern void usrNetIpAttachCommon /* added by yinwx. 20100122 */
    (
    char *    pDeviceName,	/* Device name */
    int       unitNum,		/* unit number */
    char *    pStr,		/* e.g. "IPv4", "IPv6" */
    FUNCPTR   pAttachRtn	/* attach routine */
    );
#endif 

//IMPORT void SwitchSIO(); /*added by lw*/
IMPORT void printstr(char *s);
IMPORT BOOT_PARAMS sysBootParams;/* parameters from boot line         */	
IMPORT FUNCPTR _func_printErr;

typedef int (*ipcom_cmd_func_type)(int argc, char ** argv);

/* extern */

STATUS usrNetBootConfig ( char *, int, char *, int , char *); 

IMPORT ipcom_cmd_func_type ipnet_cmd_ifconfig_hook;
IMPORT ipcom_cmd_func_type ipnet_cmd_route_hook;

#ifndef SYS_BUS_TO_LOCAL_ADRS
/******************************************************************************
*
* sysBusToLocalAdrs - convert a bus address to a local address
*
* Not Implemented
*
* NOMANUAL 
*/
 
STATUS sysBusToLocalAdrs
    (
    int  adrsSpace,     /* bus address space in which busAdrs resides,  */
                        /* use address modifier codes defined in vme.h, */
                        /* such as VME_AM_STD_SUP_DATA                  */
    char *busAdrs,      /* bus address to convert                       */
    char **pLocalAdrs   /* where to return local address                */
    )
    {
    return (ERROR);
    }
#endif

/*
 * variable to determine, whether the vxWorks image is on
 * disk or not
 */
BOOL diskBoot = FALSE;
#if 0 /* added by yinwx, 20100122 */
/*******************************************************************************
*
* usrCheckNetBootConfig - configure the network boot device, check if address
*                         field is present
*
* This routine is the initialization routine for the INCLUDE_NET_BOOT_CONFIG
* component. It assigns the IP address, netmask and default gateway if
* specified to the boot device.
*
* RETURNS: N/A
*
* NOMANUAL
*/

STATUS usrCheckNetBootConfig
    (
    char *      devName,                /* device name e.g. "fei" */
    int         unit,                   /* unit number */
    char *      addr,                   /* target ip address */
    int         netmask,                /* subnet mask */
    char *      gateway                 /* default gateway */
    )
    {
    /* Check if the vxWorks Image has been loaded from the disk */
    diskBoot = FALSE;

    /* find network boot device configuration */
    if ( (strncmp (sysBootParams.bootDev, "scsi", 4) == 0) ||
        (strncmp (sysBootParams.bootDev, "fs", 2) == 0) ||
        (strncmp (sysBootParams.bootDev, "ide", 3) == 0) ||
        (strncmp (sysBootParams.bootDev, "ata", 3) == 0) ||
        (strncmp (sysBootParams.bootDev, "fd", 2) == 0)  ||
        (strncmp (sysBootParams.bootDev, "tffs", 4) == 0) ||
        (strncmp (sysBootParams.bootDev, "usb", 3) == 0) )
          diskBoot = TRUE;

    /*
     * If we are booting from disk, then the target IP address
     * string need not be present. Make sure DHCP client option
     * is activated
     */

    if (diskBoot)
        {
        if((addr != NULL) && (addr[0] != EOS))
            return (usrNetBootConfig (devName, unit, addr, netmask, gateway));
        }

    return (OK);
}

#else

/******************************************************************************
*
* usrCheckNetBootConfig - configure the network boot device, check if address
*                         field is present
*
* This routine is the initialization routine for the INCLUDE_NET_BOOT_CONFIG
* component. It assigns the IP address, netmask and default gateway if
* specified to the boot device.
* added boot B/C/D vxWorksimage after sm boot config for sm_net of B/C/D connect
* correctly by lw on 2010-09-09. The booting can be put forword,but I dont't want
*  compile the kernel 
* RETURNS: N/A
*
* NOMANUAL
*/

void usrCheckNetBootConfig     
	(
    char *      devName,                /* device name e.g. "fei" */
    int         unit,                   /* unit number */
    char *      addr,                   /* target ip address */
    int         netmask,                /* subnet mask */
    char *      gateway                 /* default gateway */
    )

    {
/*    unsigned short nodeid;*/ /*added by lw for boot B/C/D vxWorks image*/
        
#ifdef INCLUDE_BOOT_LINE_INIT
	if (devName != NULL)
	    {
#ifdef INCLUDE_IPATTACH	
        /* Attach shared memory END device to the network stack. The 
         * device will be attached to all supported network-layer 
         * protocols built into the stack. IPNet stack does not support 
         * attach a device to IPv4 and IPv6 separately.
         */
	    usrNetIpAttachCommon (devName, unit,"IPv4",ipAttach);
#endif /* INCLUDE_IPATTACH */
            
#ifdef INCLUDE_NET_BOOT_CONFIG
        diskBoot = FALSE;

        /* find network boot device configuration */
        if ( (strncmp (sysBootParams.bootDev, "scsi", 4) == 0) ||
            (strncmp (sysBootParams.bootDev, "fs", 2) == 0) ||
            (strncmp (sysBootParams.bootDev, "ide", 3) == 0) ||
            (strncmp (sysBootParams.bootDev, "ata", 3) == 0) ||
            (strncmp (sysBootParams.bootDev, "fd", 2) == 0)  ||
            (strncmp (sysBootParams.bootDev, "tffs", 4) == 0) ||
            (strncmp (sysBootParams.bootDev, "usb", 3) == 0) )
            diskBoot = TRUE;

        /*
         * If we are booting from disk, then the target IP address
         * string need not be present. Make sure DHCP client option
         * is activated 
         */

        if ((diskBoot) && (sysBootParams.flags & SYSFLG_AUTOCONFIG)) 
            {
            if((addr != NULL) && (addr[0] != EOS))
	        usrNetBootConfig (devName, unit, addr,
			                  netmask, gateway);
            }
        else
            usrNetBootConfig (devName, unit, addr,
			      netmask, gateway);
#endif /* INCLUDE_NET_BOOT_CONFIG */

#ifdef INCLUDE_NET_BOOT_IPV6_CFG
        {
            /*
             * Booting over IPv6 address is not supported, which means that
             * the boot net device will not be configured for IPv6 address.
             * Therefore, if the user provided the IPv6 addresses in the
             * bootstring, we will configure the boot net device with the
             * addresses regardless of whether 'diskBoot' is TRUE or FALSE.
             * This is so that we are able to download symbol table if
             * necessary.
             */
            char * pTgtIpv6Addr = NULL;
            char * pHstIpv6Addr = NULL;

            if ((pTgtIpv6Addr = calloc (USR_NET_IP6ADDR_LEN, sizeof(char))) &&
                (pHstIpv6Addr = calloc (USR_NET_IP6ADDR_LEN, sizeof(char))))
            {
                pTgtIpv6Addr [0] = 0;
                pHstIpv6Addr [0] = 0;
                if (usrNetBootIpv6AddrExtract (pTgtIpv6Addr, pHstIpv6Addr,
                                               USR_NET_IP6ADDR_LEN,
                                               NULL) == ERROR)
                    printf ("Could not extract IPv6 address from boot string.\n");
                else
                {
                    if (usrNetBootConfig (devName, unit, pTgtIpv6Addr, 0, 0)
                            == ERROR)
                        printf ("Could not configure IPv6 address on %s%d\n",
                                devName, unit);
                }
            }
            else
            {
                /* Ignore error */
            }
            if (pTgtIpv6Addr)
                free (pTgtIpv6Addr);
            if (pHstIpv6Addr)
                free (pHstIpv6Addr);
        }
#endif /* INCLUDE_NET_BOOT_IPV6_CFG */
        }
#endif /* INCLUDE_BOOT_LINE_INIT */

#ifdef INCLUDE_SM_NET
	  /*added by lw*/ 
#if 1  
	    nodeid =getNodeNumberZR()&0x3;
		if(!nodeid)
		{
		     CPU_WRITE32(0x3ff01020,1,0x80200000);
		     CPU_WRITE32(0x3ff01020,2,0x80200000);	        
		     CPU_WRITE32(0x3ff01020,3,0x80200000);
		     printstr("finished load B/C/D image^^^^^^^^^^^^^^\r\n");
		}
	 /*end*/	
#endif	 
    if (0/*!backplaneBoot*/ /* added by yinwx, 20100224 */)  /* mxl  20120914: 1 -> 0 */
        {
         unsigned int bpmask = 0;
        END_OBJ*   pEnd;

        pEnd = endFindByName ("sm", 0);
        if (pEnd == NULL)
	    {
            printf ("Can not find the shared memory END device.");
            printf ("Check address assignments for SM master.\n");
            }
        else 
	    {
#ifdef INCLUDE_IPATTACH  
            /* Attach shared memory END device to the network stack. The 
             * device will be attached to all supported network-layer 
             * protocols built into the stack. IPNet stack does not support 
             * attach a device to IPv4 and IPv6 separately.
             */
            usrNetIpAttachCommon ("sm", 0, "IPv4", ipAttach);
#endif /* INCLUDE_IPATTACH */
            }

        /* 
         * The load routine for the shared memory END device determines
         * an IP address for the shared memory master using the boot
         * parameter values. If none is available, the load fails and
         * the previous endFindByName() call will prevent this configuration
         * attempt.
         *
         * The shared memory slaves do not require an address from the boot
         * parameters. If the backplane address is not given, those devices
         * will retrieve an address from the shared memory master.
         */ 

        if (sysBootParams.bad[0] == EOS)
            {printf("usrCheckNetBootConfig: sysBootParams.bad[0] == EOS\r\n"); 
            if (smEndInetAddrGet ("sm", 0, NONE, sysBootParams.bad) != OK)
                printf ("Error: sm0 addressing is not setup correctly\n");
            }
        else
            {
			printf("sysBootParams.bad is %s\n", sysBootParams.bad);
            if(bootNetmaskExtract (sysBootParams.bad, &bpmask)<1)
				bpmask = 0xffffff00; /* added by yinwx, 20100304, set to a default class C */
            }

        /* SM configuration with boot parameters */

#ifdef INCLUDE_NET_BOOT_CONFIG
        usrNetBootConfig ("sm", 0, sysBootParams.bad,
                          bpmask, gateway); 
#endif
        }
      
#ifdef INCLUDE_SM_NET_SHOW
    smEndShowInit ();
#endif /* INCLUDE_SM_NET_SHOW */

#endif /* INCLUDE_SM_NET */
/*added by lw*/      
#ifdef BOARD_HRSIGNAL
    	printstr("begin switch sio*********************************\r\n");
//	  	SwitchSIO(); /*added by lw for switch SIO among 4 cpus*/  
#endif	  	
    }

#endif

#ifdef INET
/*******************************************************************************
*
* usrNetBootAddGateway - add default gateway for IPv4 network
*
* This routine is called from usrNetBootConfig() to add default gateway for 
* IPv4 network.
*
* RETURNS: OK or ERROR
*
* NOMANUAL
*/

LOCAL STATUS usrNetBootAddGateway
    (
    char *	dstaddr,   /* destination address */
    char *	gateway    /* gateway */
    )
    {
    char *argv[] = {
        "route",
        "add",
        "-silent",
        "-inet",
        "-static",
        "-net",
        "-prefixlen",
        IP_NULL,
        IP_NULL,
        IP_NULL,
        IP_NULL,
    };
    char prefix_len_str[4];
    int argc = 7;
    int ret;

    sprintf(prefix_len_str, "%d", 0);
    argv[argc++] = prefix_len_str;
    argv[argc++] = dstaddr;
    argv[argc++] = gateway;

    ret = ipnet_cmd_route_hook(argc, argv);
    if (ret == IP_ERRNO_EEXIST){
		printf("route add OK-1!\r\n");
        return OK;
    	}
	printf("route add OK?\r\n");
    return ret == IPCOM_SUCCESS ? OK : ERROR;
    }
#endif /* INET */
    
/*****************************************************************************
*
* usrNetBootAutoConfig - bring up the network boot device
* 
* This routine is called from usrNetBootConfig() to bring up the network boot
* device. In previous releases of the Wind River network stack, IPv4 and IPv6
* protocols can be attached to the device separately. In the current release,
* the device will be attached to all supported network-layer protocols built
* into the stack at library archive-build time.
*
* RETURNS: OK or ERROR
*
* NOMANUAL
*/

LOCAL STATUS usrNetBootAutoConfig
    (
    char * ifname     /* device name e.g. "fei0" */
    )
    {
    char *argv[] = {
        "ifconfig",
        "-silent",
        IP_NULL,
        "up",
        IP_NULL
    };

    argv[2] = ifname;
    return ipnet_cmd_ifconfig_hook(4, argv);
    }

/*******************************************************************************
*
* usrNetBootConfig - configure the network boot device
*
* This routine is the initialization routine for the INCLUDE_NET_BOOT_CONFIG
* component. It assigns the IP address, netmask and default gateway if
* specified to the boot device.
*
* RETURNS: N/A
*
* NOMANUAL
*/
STATUS usrNetBootConfig
    (
    char *      devName,                /* device name e.g. "fei" */
    int         unit,			/* unit number */
    char *      addr,                   /* target ip address */
    int         netmask,                /* subnet mask */
    char *	gateway			/* default gateway */
    )
    {
    char ifname [IFNAMSIZ + 10 + 1];    /* devName + unit + '\0' */
    char *prefix_len;
    char inet_prefix_len_str[8];
    int mask;
    int argc = 5;
/*	int tmp, nodeNum; *//* added by yinwx, 20100303 */
    char *argv[] = {
        "ifconfig",
        "-silent",
        IP_NULL,
        "inet",
        "add",
        IP_NULL,
        IP_NULL,
        IP_NULL,
        IP_NULL
    };

    /*
     * Do nothing if another device is already configured or an
     * error was detected in the boot parameters.
     */

    if (devName == NULL || devName[0] == EOS || addr == NULL || addr[0] == EOS ||
	strlen(devName) >= IFNAMSIZ)
        {
        if (_func_printErr)
            (*_func_printErr) ("usrNetBootConfig: Invalid Argument\n");
        return (ERROR);
        }

#if 0    
    /* ppp is not attached yet, just return */
    if (strncmp (devName, "ppp", 3) == 0)
        return (OK);
#endif

    /* build interface name */
    sprintf (ifname, "%s%d", devName, unit);
    argv[2] = ifname;

    /* interface name must not exceed IFNAMSIZ */
    if (strlen(ifname) >= IFNAMSIZ)
        {
        if (_func_printErr)
            (*_func_printErr) ("usrNetBootConfig: Interface name too long\n");
        return (ERROR);
        }

    /* set inet addr and subnet mask */
    argv[argc++] = addr;
    argv[argc++] = "prefixlen";

    mask = htonl(netmask);
    prefix_len = inet_prefix_len_str;
    sprintf(inet_prefix_len_str, "%d", ipcom_mask_to_prefixlen(&mask, 32));
    argv[argc++] = prefix_len;
	/*printf("prefix_len is %s!!\r\n", prefix_len); */
    if(ipnet_cmd_ifconfig_hook(argc, argv)<0)
		printf("IFCONFIG ERROR -1!!");

    if(usrNetBootAutoConfig (ifname)<0)
		printf("IFCONFIG ERROR -2!!");;    

    /* set default gateway from give boot parameter */

#ifdef INET
    if (gateway != NULL && gateway[0] != EOS)
        {printf("gateway is %s!!\r\n", gateway);
        if (usrNetBootAddGateway ("0.0.0.0", gateway) == ERROR)
            {
            if (_func_printErr)
                (*_func_printErr) ("usrNetBootConfig: Failed addGateway\n");
            return ERROR;
            }
        }
#endif
    
    return OK;
    }

#ifdef INCLUDE_NET_BOOT_IPV6_CFG
void usrCheckNetBootIpv6Config (void)
{
    /*
     * Booting over IPv6 address is not supported, which means that
     * the boot net device will not be configured for IPv6 address.
     * Therefore, if the user provided the IPv6 addresses in the
     * bootstring, we will configure the boot net device with the
     * addresses regardless of whether 'diskBoot' is TRUE or FALSE.
     *
     * This is so that we are able to download symbol table if
     * necessary.
     */
    char * pTgtIpv6Addr = NULL;
    char * pHstIpv6Addr = NULL;

    if ((pTgtIpv6Addr = calloc (USR_NET_IP6ADDR_LEN, sizeof(char))) &&
        (pHstIpv6Addr = calloc (USR_NET_IP6ADDR_LEN, sizeof(char))))
    {
        pTgtIpv6Addr [0] = 0;
        pHstIpv6Addr [0] = 0;
        if (usrNetBootIpv6AddrExtract (pTgtIpv6Addr, pHstIpv6Addr,
                                       USR_NET_IP6ADDR_LEN,
                                       NULL) == ERROR)
            printf ("Could not extract IPv6 address from boot string.\n");
        else
        {
            if (usrNetBootIpv6Config (pDevName, uNum, pTgtIpv6Addr, 0, 0)
                == ERROR)
            printf ("Could not configure IPv6 address on %s%d\n",
                    pDevName, uNum);
        }
    }
    else
    {
        /* Ignore error */
    }
    if (pTgtIpv6Addr)
        free (pTgtIpv6Addr);
    if (pHstIpv6Addr)
        free (pHstIpv6Addr);
}

/*******************************************************************************
*
* usrNetBootIpv6Config - configure the network boot device
*
* This routine is the initialization routine for the INCLUDE_NET_BOOT_CONFIG
* component. It assigns the IP address, netmask and default gateway if
* specified to the boot device.
*
* RETURNS: N/A
*
* NOMANUAL
*/

STATUS usrNetBootIpv6Config
    (
    char *      devName,                /* device name e.g. "fei" */
    int         unit,			/* unit number */
    char *      addr,                   /* target ip address */
    int         netmask,                /* subnet mask */
    char *	gateway			/* default gateway */
    )
    {
#define USR_NET_PREFIXLEN_DELIM '/'
    const char delim [] = {USR_NET_PREFIXLEN_DELIM, ' ', '\t', 0};
    char * inet6_addr;
    char * pNext;
    int argc = 6; 
    char ifname [20];
    char *prefix_len;
    char *argv[] = {
        "ifconfig",
        "-silent",
        IP_NULL,
        "inet6",
        "add",
        IP_NULL,
        IP_NULL,
        IP_NULL,
        IP_NULL
    };

    /*
     * Do nothing if another device is already configured or an
     * error was detected in the boot parameters.
     */

    if (devName == NULL || devName[0] == EOS || addr == NULL || addr[0] == EOS)
        {
        if (_func_printErr)
            (*_func_printErr) ("usrNetBootConfig: Invalid Argument\n");
        return (ERROR);
        }

#if 0    
    /* ppp is not attached yet, just return */
    if (strncmp (devName, "ppp", 3) == 0)
        return (OK);
#endif

    /* build interface name */

    sprintf (ifname, "%s%d", devName, unit);
    argv[2] = ifname;

    if ((inet6_addr = strtok_r (addr, delim, &prefix_len)))
    {
        argv [5] = inet6_addr;
        if (prefix_len && (prefix_len = strtok_r (prefix_len, delim, &pNext)))
        {
            argv [argc++] = "prefixlen";
            argv [argc++] = prefix_len;
        }
    }
    else
    {
        if (_func_printErr)
            (*_func_printErr) ("usrNetBootConfig: Invalid IPv6 address %s\n",
                               addr);
        return ERROR;
    }

    ipnet_cmd_ifconfig_hook(argc, argv);

    (void)usrNetBootAutoConfig (ifname);    

    return OK;
    }
#endif /* INCLUDE_NET_BOOT_IPV6_CFG */
