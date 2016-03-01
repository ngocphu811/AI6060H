/*
 * Copyright (c) 2014, SouthSilicon Valley Corp.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         A very simple Contiki application showing how Contiki programs look
 * \author
 *         Adam Dunkels <adam@sics.se>
 */
#include "contiki.h"
#include <string.h>
#include <stdio.h> /* For printf() */
#include "gpio_api.h"
#include "atcmd.h"
#include "resolv.h"
#include "ieee80211_mgmt.h"
#include "socket_driver.h"
#include "atcmd_icomm.h"
#include "wdog_api.h"

int AT_Customer(stParam *param);
int AT_Customer2(stParam *param);
int AT_TCPSERVER_DEMO(stParam *param);
int AT_WIFIUART_DEMO(stParam *param);
int AT_SmartLink(stParam *param);


at_cmd_info atcmd_info_tbl[] = 
{
	{"AT+CUSTOMER_CMD",	&AT_Customer},
	{"AT+CUSTOMER_CMD2=",	&AT_Customer2},
	{"AT+TCPSERVER_DEMO=",	&AT_TCPSERVER_DEMO},
	{"AT+WIFIUART_DEMO=",&AT_WIFIUART_DEMO},
    {"AT+SMNT",&AT_SmartLink},
    {"",    NULL}
};

int gTcpSocket = -1;


//extern void test1();
/*---------------------------------------------------------------------------*/
PROCESS(main_process, "main process");
PROCESS(ac_nslookup_process, "NSLookup Process");
PROCESS(ac_tcp_connect_process, "Tcp Connect Process");

/*---------------------------------------------------------------------------*/
PROCESS_NAME(tcpServerDemo_process);
PROCESS_NAME(wifiUartDemo_process);
/*---------------------------------------------------------------------------*/
AUTOSTART_PROCESSES(&main_process, &ac_tcp_connect_process);
/*---------------------------------------------------------------------------*/
 void TurnOffAllLED()
 {
	 GPIO_CONF conf;
 
	 conf.dirt = OUTPUT;
	 conf.driving_en = 0;
	 conf.pull_en = 0;
 
	 pinMode(GPIO_1, conf);
	 digitalWrite(GPIO_1, 0);	 
	 pinMode(GPIO_3, conf);
	 digitalWrite(GPIO_3, 0);	 
	 pinMode(GPIO_8, conf);
	 digitalWrite(GPIO_8, 0);	 
 
	 return;
 }
/*---------------------------------------------------------------------------*/
 int SetLED (uint8_t nEnable)
 {
 	GPIO_CONF conf;

	conf.dirt = OUTPUT;
	conf.driving_en = 0;
	conf.pull_en = 0;
	
	pinMode(GPIO_1, conf);
 	if(nEnable == 0)
 	{
 		digitalWrite(GPIO_1, 0);
 	}
 	else
 	{
 		digitalWrite(GPIO_1, 1);
 	}
 	return ERROR_SUCCESS;
 }

int AT_Customer(stParam *param)
{
	printf("Call AT_Customer\n");
	return 0;
}
int AT_Customer2(stParam *param)
{
	int i = 0;
	printf("Call AT_Customer2\n");
	for(i=0; i<param->argc; i++)
	{
		printf("Param%d:%s\n", i+1, param->argv[i]);
	}
	return 0;
}
int AT_WIFIUART_DEMO(stParam *param)
{
	int i = 0;
	printf("Call AT_WIFIUART_DEMO\n");
	if(strcmp(param->argv[0] ,"enable") == 0) {
		process_start(&wifiUartDemo_process, NULL);
	} else if(strcmp(param->argv[0] ,"disable") == 0) {
		process_post_synch(&wifiUartDemo_process, PROCESS_EVENT_EXIT, NULL);
	} else {
		printf("wifi uart demo unknown param, please check\n");
	}
	return 0;
}
int AT_TCPSERVER_DEMO(stParam *param)
{
	int i = 0;
	printf("Call AT_TCPSERVER_DEMO\n");
	if(strcmp(param->argv[0] ,"enable") == 0) {
		process_start(&tcpServerDemo_process, NULL);
	} else if(strcmp(param->argv[0] ,"disable") == 0) {
		process_post_synch(&tcpServerDemo_process, PROCESS_EVENT_EXIT, NULL);
	} else {
		printf("tcp server demo unknown param, please check\n");
	}
	return 0;
}

int AT_SmartLink(stParam *param)
{
    At_Disconnect();
    AT_RemoveCfsConf();
    api_wdog_reboot();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(main_process, ev, data)
{
    //    EVENTMSG_DATA *pMesg = NULL;
    PROCESS_BEGIN();

    printf("********hello main_process********\n");
    //test1();

    //uip_init();

    
    TurnOffAllLED();


    PROCESS_END();
}

void Test_Connect(void)
{
    char *buf = "test.ablecloud.cn";
	process_start(&ac_nslookup_process, NULL);
	bss_nslookup(buf);
}

PROCESS_THREAD(ac_nslookup_process, ev, data)
{
    static NETSOCKET httpsock;
    SOCKETMSG msg;
    
	PROCESS_BEGIN();
	PROCESS_WAIT_EVENT_UNTIL(ev == resolv_event_found);
	{
        uip_ip4addr_t	remote_ip_addr;
		uip_ipaddr_t addr;
		uip_ipaddr_t *addrptr;
		addrptr = &addr;
	
		char *pHostName = (char*)data;
		if(ev == resolv_event_found)
		{
			resolv_lookup(pHostName, &addrptr);
			uip_ipaddr_copy(&addr, addrptr);
			printf("AT+NSLOOKUP=%d.%d.%d.%d\n", addr.u8[0] , addr.u8[1] , addr.u8[2] , addr.u8[3] );

            gTcpSocket = tcpconnect( &addr, 9100, &ac_tcp_connect_process);
            printf("create socket:%d\n", gTcpSocket);
#if 0
    		//wait for TCP connected or uip_timeout.
    		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_MSG || ev == PROCESS_EVENT_TIMER);
    		if(ev == PROCESS_EVENT_MSG)
    		{
    			msg = *(SOCKETMSG *)data;
    			if(msg.status != SOCKET_CONNECTED)
    			{
    				printf("TCP connect fail1! Post message type:%d\n", msg.status);
    			}
                else if (msg.status == SOCKET_CONNECTED)
                {
                    printf("Connect ok\n");
                }
    		}
    		else
    		{
    			printf("TCP connect fail2!\n");
    		}
#endif
		}
	}
	PROCESS_END();
}

PROCESS_THREAD(ac_tcp_connect_process, ev, data)
{
	PROCESS_BEGIN();
	SOCKETMSG msg;
	int recvlen;
	uip_ipaddr_t peeraddr;
	U16 peerport;

	while(1) 
	{
		PROCESS_WAIT_EVENT();

		if( ev == PROCESS_EVENT_EXIT) 
		{
			break;
		} 
		else if(ev == PROCESS_EVENT_MSG) 
		{
			msg = *(SOCKETMSG *)data;
			//The TCP socket is connected, This socket can send data after now.
			if(msg.status == SOCKET_CONNECTED)
			{
				printf("socked:%d connected\n", msg.socket);
				if(msg.socket == gTcpSocket)
				    printf("Equal socket\n");
			}
			//TCP connection is closed. Clear the socket number.
			else if(msg.status == SOCKET_CLOSED)
			{
				printf("socked:%d closed\n", msg.socket);
				//At_RespOK(ATCMD_TCPDISCONNECT);
				if(gTcpSocket == msg.socket)
					gTcpSocket = -1;
				//if(gserversock == msg.socket)
				//	gserversock = -1;
			}
			//Get ack, the data trasmission is done. We can send data after now.
			else if(msg.status == SOCKET_SENDACK)
			{
				printf("socked:%d send data ack\n", msg.socket);
				//At_RespOK(ATCMD_TCPSEND);
			}
			//There is new data coming. Receive data now.
			else if(msg.status == SOCKET_NEWDATA)
			{
#if 0
				if(0 <= msg.socket && msg.socket < UIP_CONNS)
				{
					recvlen = tcprecv(msg.socket, buffer_in_dynamic, MAX_SEND_BUFFER);
					buffer_in_dynamic[recvlen] = 0;
      #if 0              
					printf("TCP socked:%d recvdata:%s\n", msg.socket, buffer_in_dynamic);
      #endif
      //printf("TCP-%d:%c-%c-%c\n", msg.socket, buffer_in_dynamic[0],buffer_in_dynamic[1],buffer_in_dynamic[2]);
      printf("%d:%c%c\n", msg.socket, buffer_in_dynamic[0],buffer_in_dynamic[1]);
	
				}
				else if(UIP_CONNS <= msg.socket && msg.socket < UIP_CONNS + UIP_UDP_CONNS)
				{
					recvlen = udprecvfrom(msg.socket, buffer_in_dynamic, MAX_SEND_BUFFER, &peeraddr, &peerport);
					buffer_in_dynamic[recvlen] = 0;
					printf("UDP socked:%d recvdata:%s from %d.%d.%d.%d:%d\n", msg.socket, buffer_in_dynamic, peeraddr.u8[0], peeraddr.u8[1], peeraddr.u8[2], peeraddr.u8[3], peerport);
				}
				else
		{
					printf("Illegal socket:%d\n", msg.socket);
				}
#endif
			}
			//A new connection is created. Get the socket number and attach the calback process if needed.
			else if(msg.status == SOCKET_NEWCONNECTION)
			{
			#if 0
				if(gserversock == -1)
				{
					gserversock = msg.socket;
					printf("new connected to listen port(%d), socket:%d\n", msg.lport, msg.socket);
				}
				else
				{
					//Only allow one client connection for this application.
					//tcpclose(msg.socket);
				}
            #endif
			}
			else
			{
				printf("unknow message type\n");
			}
		}	
		}	

	PROCESS_END();
}

