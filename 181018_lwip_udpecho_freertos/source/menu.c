/*
 * menu.c
 *
 *  Created on: Nov 11, 2018
 *      Author: acc
 */


#include "menu.h"

#include "lwip/opt.h"
//#include "SOUND_PLAYER.h"

#include "lwip/api.h"
#include "lwip/sys.h"



int port_to_listen = 54322;
int audio_toggle = 1;
int error_data = 0;
uint8_t GlobalMenuBuffer[200];
uint8_t main_menu_flag = 0;
uint8_t main_menu_s[]= "\n\n\r+++---MAIN MENU---+++\n\n\rSelecciona la opcion de abajo:\n\r1) Silenciar audio.\n\r2) Seleccionar audio\n\r3) Desplegar estadisticas de comunicacion\n\n\r\n\r";
uint8_t menu_1_s[]= "\n\n\r+++---MENU 1---+++\n\n\rSilenciar audio\n\n\rPulsa cualquier cosa para volver al menu\n\n\r\n\r";
uint8_t menu_2_s[]= "\n\n\r+++---MENU 2---+++\n\n\rSelecciona la fuente de audio:\n\n\r1) Fuente A\n\n\r2) Fuente B\n\n\r\n\r";
uint8_t menu_3_s[]= "\n\n\r+++---MENU 3---+++\n\n\rMostrando estadisticas de la comunicacion:\n\n\r\n\r";
uint8_t src_A_sel[]= "\n\n\rFuente A seleccionada:\n\n\r\n\r";
uint8_t src_B_sel[]= "\n\n\rFuente B seleccionada:\n\n\r\n\r";
uint8_t menu_3_stats1[]= "\n\n\rPaquetes Recibidos:";
uint8_t menu_3_stats2[]= "\n\n\rPaquetes Perdidos:";
uint8_t menu_3_stats3[]= "\n\n\rCalidad de Comunicacion:";

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
	  netconn_bind(conn, IP_ADDR_ANY, 50555);
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
		      //uint8_t string[20]= "soyunstring";

		      while ((err = netconn_recv(newconn, &buf)) == ERR_OK) {
		        /*printf("Recved\n");*/
		        do
		        {
		             netbuf_data(buf, &data, &len);
		             netbuf_copy(buf, GlobalMenuBuffer, sizeof(GlobalMenuBuffer));

		             switch(GlobalMenuBuffer[0])
		             {
		             	case '1':
		             	{
		             		if(main_menu_flag == 0) //Si está en el menú 0
		             		{
		             			//DETENER/REPRODUCIR AUDIO
		             			if(1 == audio_toggle)
		             			{
		             				audio_toggle = 0;
		             			}
		             			else
		             			{
		             				audio_toggle = 1;
		             			}
		             			main_menu_flag = 1;
		             			break;
		             		}
		             		if(main_menu_flag == 1 || main_menu_flag == 3) //Si está en el menú 1 o en el 3.
		             		{
		             			main_menu_flag = 0;
		             			break;
		             		}
		             		if(main_menu_flag == 2) //Si está en el menú 2
		             		{
		             			//PONER FUENTE A
		             			port_to_listen = 54321;
		             			err = netconn_write(newconn, src_A_sel, sizeof(src_A_sel), NETCONN_COPY);
		             			main_menu_flag = 0;
		             			break;
		             		}
		             	}
		             	case '2':
		             	{
		             		if(main_menu_flag == 0) //Si está en el menú 0
		             		{
		             			//DETENER/REPRODUCIR AUDIO
		             			main_menu_flag = 2;
		             			break;
		             		}
		             		if(main_menu_flag == 1 || main_menu_flag == 3) //Si está en el menú 1 o en el 3.
		             		{
		             			main_menu_flag = 0;
		             			break;
		             		}
		             		if(main_menu_flag == 2) //Si está en el menú 2
		             		{
		             			//PONER FUENTE B
		             			port_to_listen = 54322;
		             			err = netconn_write(newconn, src_B_sel, sizeof(src_B_sel), NETCONN_COPY);
		             			main_menu_flag = 0;
		             			break;
		             		}
		             	}
		             	case '3':
		             	{
		             		if(main_menu_flag == 0) //Si está en el menú 0
		             		{
		             			//MOSTRAR ESTADÍSTICAS

								//error_data = SoundPlayer_getDataLost();
		             			error_data = 10;
						  		err = netconn_write(newconn, menu_3_s, sizeof(menu_3_s), NETCONN_COPY);

						  		err = netconn_write(newconn, menu_3_stats1, sizeof(menu_3_stats1), NETCONN_COPY);
						  		err = netconn_write(newconn, (100-error_data+'0'), sizeof(error_data), NETCONN_COPY);

						  		err = netconn_write(newconn, menu_3_stats2, sizeof(menu_3_stats2), NETCONN_COPY);
						  		err = netconn_write(newconn, (error_data+'0'), sizeof(error_data), NETCONN_COPY);

						  		err = netconn_write(newconn, menu_3_stats3, sizeof(menu_3_stats3), NETCONN_COPY);
						  		err = netconn_write(newconn, (error_data+'0'), sizeof(error_data), NETCONN_COPY);

		             			main_menu_flag = 3;
		             			break;
		             		}
		             		if(main_menu_flag == 1 || main_menu_flag == 3) //Si está en el menú 1 o en el 3.
		             		{
		             			main_menu_flag = 0;
		             			break;
		             		}
		             		if(main_menu_flag == 2) //Si está en el menú 2
		             		{
		             			main_menu_flag = 0;
		             			break;
		             		}

		             	}
		             	default:
		             	{
		             		main_menu_flag = 0;
		             	}
		             }


					switch (main_menu_flag)
					{
						case 0: //Menú principal
						{
					  		err = netconn_write(newconn, main_menu_s, sizeof(main_menu_s), NETCONN_COPY);
					  		break;
						}
						case 1: //Detener/Reproducir audio
						{

							err = netconn_write(newconn, menu_1_s, sizeof(menu_1_s), NETCONN_COPY);
							break;
						}
						case 2: //Cambiar fuente de reproducción
						{
							err = netconn_write(newconn, menu_2_s, sizeof(menu_2_s), NETCONN_COPY);
							break;
						}
						case 3: //Ver estadísticas
						{
							//error_data = SoundPlayer_getDataLost();
	             			error_data = 10;
							err = netconn_write(newconn, menu_3_s, sizeof(menu_3_s), NETCONN_COPY);

					  		err = netconn_write(newconn, menu_3_stats1, sizeof(menu_3_stats1), NETCONN_COPY);
					  		err = netconn_write(newconn, (100-error_data+'0'), sizeof(error_data), NETCONN_COPY);

					  		err = netconn_write(newconn, menu_3_stats2, sizeof(menu_3_stats2), NETCONN_COPY);
					  		err = netconn_write(newconn, (error_data+'0'), sizeof(error_data), NETCONN_COPY);

					  		err = netconn_write(newconn, menu_3_stats3, sizeof(menu_3_stats3), NETCONN_COPY);
					  		err = netconn_write(newconn, (error_data+'0'), sizeof(error_data), NETCONN_COPY);
							break;
						}
					}

		             PRINTF("%c",GlobalMenuBuffer[0]);



		             //err = netconn_write(newconn, string, sizeof(string), NETCONN_COPY);

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

void menu_init(void)
{
	sys_thread_new("server_menu", server_menu, NULL, 300, 2);
}
