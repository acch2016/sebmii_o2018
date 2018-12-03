/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "udpecho.h"

#include "lwip/opt.h"

#if LWIP_NETCONN

#include "lwip/api.h"
#include "lwip/sys.h"

uint16_t GlobalBuffer[200];
uint64_t recv_data_counter = 0;
//struct netbuf *GlobalBuffer;

//struct netbuf* AudioPlayer_getBuffer()
//{
//	return buf_send;
//}

uint16_t* AudioPlayer_getBuffer()
{
	return GlobalBuffer;
}

//uint64_t* SoundPlayer_getCounter()
//{
//	return &recv_data_counter;
//}

static void server_thread(void *arg)
{
	err_t err;

	struct netconn *conn;
	struct netbuf *buf;

//	uint16_t *msg;

//	uint16_t len;
	//uint16_t buffer[200];
	//	memset(buffer[0], 0, sizeof(buffer[0]));

	LWIP_UNUSED_ARG(arg);
	conn = netconn_new(NETCONN_UDP);
	netconn_bind(conn, IP_ADDR_ANY, 55321);//ip4



	while (1)
	{
		GPIO_WritePinOutput(GPIOC, 2, 1);    // OFF
		if ((err = netconn_recv(conn, &buf)) == ERR_OK)
		{
//			recv_data_counter++;
		}
//		netconn_recv(conn, &buf);
		wait_for_DMA_Transfer();
		GPIO_WritePinOutput(GPIOC, 2, 0);    // ON

//		netbuf_data(buf, (void**)&msg, &len);//CREO NO ES NECESARIA ESTA LINEA

//		PRINTF("%i" ,msg[0]);

		netbuf_copy(buf, GlobalBuffer, sizeof(GlobalBuffer));//ESTA ES LA BUENA
		//PRINTF("%i ", buffer[0]);
//		int8_t i;
//
//		for( i=0 ;i<len;i++){
//			GlobalBuffer[i]=buffer[i];
//
//		}

//				buf_send=buf;

		netbuf_delete(buf);

	}


}

/*-----------------------------------------------------------------------------------*/
//static void
//client_thread(void *arg)
//{
//	ip_addr_t dst_ip;
//	struct netconn *conn;
//	struct netbuf *buf;
//
//	LWIP_UNUSED_ARG(arg);
//	conn = netconn_new(NETCONN_UDP);
//	//LWIP_ERROR("udpecho: invalid conn", (conn != NULL), return;);
//
//	char *msg = "Hello loopback!";
//	buf = netbuf_new();
//	netbuf_ref(buf,msg,10);
//
//	IP4_ADDR(&dst_ip, 127, 0, 0, 1);
//
//	while (1)
//	{
//		netconn_sendto(conn, buf, &dst_ip, 50005);
//		vTaskDelay(1000);
//	}
//}
/*-----------------------------------------------------------------------------------*/
void udpecho_init(void)
{

	//	sys_thread_new("client", client_thread, NULL, 300, 1);
	sys_thread_new("server", server_thread, NULL, DEFAULT_THREAD_STACKSIZE, configMAX_PRIORITIES-2);

}

#endif /* LWIP_NETCONN */
