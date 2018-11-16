/*
 * menu.c
 *
 *  Created on: Nov 11, 2018
 *      Author: acc
 */


#include "menu.h"

#include "lwip/opt.h"


#include "lwip/api.h"
#include "lwip/sys.h"


uint8_t GlobalMenuBuffer[200];

static void server_menu(void *arg)
{

	struct netconn *conn, *newconn;
	  err_t err;
	  LWIP_UNUSED_ARG(arg);

	  /* Create a new connection identifier. */
	  /* Bind connection to well known port number 7. */
	#if LWIP_IPV6
	  conn = netconn_new(NETCONN_TCP_IPV6);
	  netconn_bind(conn, IP6_ADDR_ANY, 7);
	#else /* LWIP_IPV6 */
	  conn = netconn_new(NETCONN_TCP);
	  netconn_bind(conn, IP_ADDR_ANY, 50500);
	#endif /* LWIP_IPV6 */
	  LWIP_ERROR("tcpecho: invalid conn", (conn != NULL), return;);

	  /* Tell connection to go into listening mode. */
	  netconn_listen(conn);


	while (1)
	{

		/* Grab new connection. */
		err = netconn_accept(conn, &newconn);
		/*printf("accepted new connection %p\n", newconn);*/
		/* Process the new connection. */

		 if (err == ERR_OK) {
		      struct netbuf *buf;
		      void *data;
		      u16_t len;
//		      void *datos;
		      uint8_t string[20]= "soyunstring";

		      while ((err = netconn_recv(newconn, &buf)) == ERR_OK) {
		        /*printf("Recved\n");*/
		        do
		        {
		             netbuf_data(buf, &data, &len);
		             netbuf_copy(buf, GlobalMenuBuffer, sizeof(GlobalMenuBuffer));
		             PRINTF("%c",GlobalMenuBuffer[0]);



		             err = netconn_write(newconn, string, sizeof(string), NETCONN_COPY);

		        } while (netbuf_next(buf) >= 0);
		        netbuf_delete(buf);
		      }
		      /*printf("Got EOF, looping\n");*/
		      /* Close connection and discard connection identifier. */
		      netconn_close(newconn);
		      netconn_delete(newconn);
	}


}

}




void
menu_init(void)
{
	sys_thread_new("server_menu", server_menu, NULL, 300, 2);
}
