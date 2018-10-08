/*
 * p1.c
 *
 *  Created on: 30/09/2018
 *      Author: ulise
 */

#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "fsl_debug_console.h"
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"
#include "queue.h"

#include "rtos_uart.h"
#include "rtos_i2c.h"
#include "rtos_i2c_rtc.h"
#include "rtos_gpio.h"

#define SYSTEM_START_PRIORITY		(configMAX_PRIORITIES)
#define MENU_MAIN_MENU				(0)
#define MENU_MAIN_MENU_ERROR		(1)
#define MENU_1						(2)
#define MENU_2						(3)
#define MENU_3						(4)
#define MENU_4						(5)
#define MENU_HOUR_ERROR				(6)
#define NO_ASIGNADO					(0x5A)
#define PC							(1)
#define CEL							(2)

uint8_t old_data;
uint8_t actual_menu;
uint8_t actual_menu_bt;
uint8_t admin_mode = 0;
uint8_t flag_menu_1 = 0;
uint8_t flag_menu_2 = 0;
uint8_t size_to_read = 0;
char *size_to_read_c;
char dir_memoria[4] = {NO_ASIGNADO,NO_ASIGNADO,NO_ASIGNADO,NO_ASIGNADO};
char *data_to_write;
uint8_t wr_chars = 0;
uint8_t sm_log = 0;
uint8_t initial_log = 0;
uint8_t modo_administrador = 0;
uint8_t sm_set_hour = 0;

typedef struct
{
	uint8_t user_1;
	uint8_t user_2;
	uint8_t user_3;
	uint8_t user_4;
	uint8_t user_5;
	char log_1_mem[4];
	char log_2_mem[4];
	char log_3_mem[4];
	char log_4_mem[4];
	char log_5_mem[4];
	char data_1[20];
	char data_2[20];
	char data_3[20];
	char data_4[20];
	char data_5[20];
	rtos_rtc_time log_time_1;
	rtos_rtc_time log_time_2;
	rtos_rtc_time log_time_3;
	rtos_rtc_time log_time_4;
	rtos_rtc_time log_time_5;
}log_memoria;

/*TEXTOS DE LOS MENÚS*/
char console_clear_t[] = "\e[1;1H\e[2J";
char hora_actual[] = "\rHORA ACTUAL:\r\n";
char main_menu_t[] = "\r+++---MAIN MENU---+++\n\n\rControl del modulo de memoria compartida:\n\r1) Escribir en direccion.\n\r2) Leer de direccion\n\r3) Historial\n\r4) Establecer hora\n\rSelecciona una opcion del menu superior:  \n\r";
char error_menu_t[] = "\r*****ERROR*****\n\n\rIngresa una opcion correcta (1,2,3,4) del menu anterior\n\rPulsa cualquier tecla para continuar: \n\r";
char error_time_t[] = "\r*****ERROR*****\n\n\rEl formato de hora que estabas por ingresar es incorrecto. Intentalo de nuevo.\n\rPulsa cualquier tecla para continuar: \n\r";
char menu_1_t[] = "\rSeleccionaste la opcion 1:\n\rDireccion (Hexa) y presiona Enter: 0x\n\r";
char menu_1_1_t[] = "\n\rAhora, ingresa lo que desees escribir y presiona Enter: \n\r";
char menu_1_2_t[] = "\n\rSe ha guardado correctamente el siguiente elemento: \n\r";
char menu_2_t[] = "\rSeleccionaste la opcion 2:\n\rIngresa direccion (Hexa) y presiona Enter: 0x\n\r";
char menu_2_1_t[] = "\n\rAhora, ingresa el tamanio de la cadena a leer (1-9) y presiona Enter: \n\r";
char menu_2_2_t[] = "\n\rSe ha leido lo siguiente en la memoria: \n\r";
char menu_3_t[] = "\rMostrando el historial reciente...\n\r";
char menu_4_t[] = "\rIngrese la hora (hh:mm:ss):\n\r";
char menu_4_1_t[] = "\rDebe ingresar como administrador para entrar a esete menu.\n\r";
char menu_admin_mode[] = "\n\r~~~~~MODO DE ADMINISTRADOR ACTIVADO~~~~~\n\n\r";
char menu_return_to_main_t[] = "\n\n\rPresiona cualquier tecla para volver al menu principal: \n\r";
char set_time_finish[] = "\n\r~~~~~HORA ACTUALIZADA~~~~~\n\n\r";
char empty[] = "\t\t\t";
log_memoria log_menu_3;
rtos_rtc_time hour_to_set;
rtos_rtc_time readed_time;
rtos_rtc_time actual_time;

/*HISTORIAL DE LOGS*/
char menu_3_1_t[] = "\r\nEntrada:\t\tUsuario:\t\tHora:\t\t\t\tLocalidad:\t\tDatos:\n\n";
char menu_3_2_t[] = "\r--1--\t\t\t";
char menu_3_3_t[] = "\r--2--\t\t\t";
char menu_3_4_t[] = "\r--3--\t\t\t";
char menu_3_5_t[] = "\r--4--\t\t\t";
char menu_3_6_t[] = "\r--5--\t\t\t";
char menu_3_7_pc_t[] = "PC\t\t\t";
char menu_3_7_cel_t[] = "Cel\t\t\t";
char menu_3_8_t[] = "12:00:00\t\t\t";
char menu_3_9_t[] = "\t\t\t";
char menu_3_10_t[] = "\n";

//HORA EN ARRAY
char time_array[11];

void main_menu();

void main_menu_bt();

uint8_t to_seconds(uint8_t seconds)
{
	return seconds + 0x80;
}

uint8_t int_to_hex(uint8_t num)
{
	uint8_t dec = num/10;
	uint8_t uni = num-(dec*10);
	uint8_t result = 0x00;

	result = (result & 0xF0) | (uni & 0xF); // write low quartet
	result = (result & 0x0F) | ((dec & 0xF) << 4); // write high quartet

	return result;
}

void time_to_array(rtos_rtc_time time)
{
	time_array[0] = time.hour/0x10;
	time_array[1] = time.hour%0x10;
	time_array[2] = ':';
	time_array[3] = time.minute/0x10;
	time_array[4] = time.minute%0x10;
	time_array[5] = ':';
	time_array[6] = (time.second-0x80)/0x10;
	time_array[7] = time.second%0x10;
	time_array[8] = '\t';
	time_array[9] = '\t';
	time_array[10] = '\t';
}

void array_to_ascii_time()
{
	for(int i=0;i<8;i++)
	{
		if(time_array[i] == 0)
		{
			time_array[i] = 0x30;
		}
		else if (time_array[i] == 1)
		{
			time_array[i] = 0x31;
		}
		else if (time_array[i] == 2)
		{
			time_array[i] = 0x32;
		}
		else if (time_array[i] == 3)
		{
			time_array[i] = 0x33;
		}
		else if (time_array[i] == 4)
		{
			time_array[i] = 0x34;
		}
		else if (time_array[i] == 5)
		{
			time_array[i] = 0x35;
		}
		else if (time_array[i] == 6)
		{
			time_array[i] = 0x36;
		}
		else if (time_array[i] == 7)
		{
			time_array[i] = 0x37;
		}
		else if (time_array[i] == 8)
		{
			time_array[i] = 0x38;
		}
		else if (time_array[i] == 9)
		{
			time_array[i] = 0x39;
		}
	}
}

void console_clear()
{
	rtos_uart_send(rtos_uart0, &console_clear_t, sizeof(console_clear_t));
}

void main_menu()
{
	actual_menu = MENU_MAIN_MENU;
	console_clear();
	rtos_uart_send(rtos_uart0, &hora_actual, sizeof(hora_actual));
	time_to_array(rtos_i2c_rtc_read_time(rtos_i2c0));
	array_to_ascii_time();
	rtos_uart_send(rtos_uart0, time_array, sizeof(time_array));
	rtos_uart_send(rtos_uart0, menu_3_10_t, sizeof(menu_3_10_t)); //Aquí sólo imprimes un enter
	rtos_uart_send(rtos_uart0, &main_menu_t, sizeof(main_menu_t));
}

void menu_error()
{
	actual_menu = MENU_MAIN_MENU_ERROR;
	console_clear();
	rtos_uart_send(rtos_uart0, &error_menu_t, sizeof(error_menu_t));
}

void menu_1()
{
	actual_menu = MENU_1;
	console_clear();
	rtos_uart_send(rtos_uart0, &menu_1_t, sizeof(menu_1_t));
}

void menu_2()
{
	actual_menu = MENU_2;
	console_clear();
	rtos_uart_send(rtos_uart0, &menu_2_t, sizeof(menu_2_t));
}

void menu_3()
{
	actual_menu = MENU_3;
	console_clear();
	rtos_uart_send(rtos_uart0, &menu_3_t, sizeof(menu_3_t));
	rtos_uart_send(rtos_uart0, &menu_3_1_t, sizeof(menu_3_1_t));

	//Log1
	rtos_uart_send(rtos_uart0, &menu_3_2_t, sizeof(menu_3_2_t));
	if(log_menu_3.user_1 == PC)
	{
		rtos_uart_send(rtos_uart0, &menu_3_7_pc_t, sizeof(menu_3_7_pc_t));
	}
	else if (log_menu_3.user_1 == CEL)
	{
		rtos_uart_send(rtos_uart0, &menu_3_7_cel_t, sizeof(menu_3_7_cel_t));
	}
	else
	{
		rtos_uart_send(rtos_uart0, &empty, sizeof(empty));
	}
	time_to_array(log_menu_3.log_time_1);
	array_to_ascii_time();
	rtos_uart_send(rtos_uart0, time_array, sizeof(time_array));
	rtos_uart_send(rtos_uart0, log_menu_3.log_1_mem, sizeof(log_menu_3.log_1_mem));
	rtos_uart_send(rtos_uart0, &menu_3_9_t, sizeof(menu_3_9_t));
	rtos_uart_send(rtos_uart0, log_menu_3.data_1, sizeof(log_menu_3.data_1));
	rtos_uart_send(rtos_uart0, &menu_3_10_t, sizeof(menu_3_10_t));

	//Log2
	rtos_uart_send(rtos_uart0, &menu_3_3_t, sizeof(menu_3_3_t));
	if(log_menu_3.user_2 == PC)
	{
		rtos_uart_send(rtos_uart0, &menu_3_7_pc_t, sizeof(menu_3_7_pc_t));
	}
	else if (log_menu_3.user_2 == CEL)
	{
		rtos_uart_send(rtos_uart0, &menu_3_7_cel_t, sizeof(menu_3_7_cel_t));
	}
	else
	{
		rtos_uart_send(rtos_uart0, &empty, sizeof(empty));
	}
	time_to_array(log_menu_3.log_time_2);
	array_to_ascii_time();
	rtos_uart_send(rtos_uart0, time_array, sizeof(time_array));
	rtos_uart_send(rtos_uart0, log_menu_3.log_2_mem, sizeof(log_menu_3.log_2_mem));
	rtos_uart_send(rtos_uart0, &menu_3_9_t, sizeof(menu_3_9_t));
	rtos_uart_send(rtos_uart0, log_menu_3.data_2, sizeof(log_menu_3.data_2));
	rtos_uart_send(rtos_uart0, &menu_3_10_t, sizeof(menu_3_10_t));

	//Log3
	rtos_uart_send(rtos_uart0, &menu_3_4_t, sizeof(menu_3_4_t));
	if(log_menu_3.user_3 == PC)
	{
		rtos_uart_send(rtos_uart0, &menu_3_7_pc_t, sizeof(menu_3_7_pc_t));
	}
	else if (log_menu_3.user_3 == CEL)
	{
		rtos_uart_send(rtos_uart0, &menu_3_7_cel_t, sizeof(menu_3_7_cel_t));
	}
	else
	{
		rtos_uart_send(rtos_uart0, &empty, sizeof(empty));
	}
	time_to_array(log_menu_3.log_time_3);
	array_to_ascii_time();
	rtos_uart_send(rtos_uart0, time_array, sizeof(time_array));
	rtos_uart_send(rtos_uart0, log_menu_3.log_3_mem, sizeof(log_menu_3.log_3_mem));
	rtos_uart_send(rtos_uart0, &menu_3_9_t, sizeof(menu_3_9_t));
	rtos_uart_send(rtos_uart0, log_menu_3.data_3, sizeof(log_menu_3.data_3));
	rtos_uart_send(rtos_uart0, &menu_3_10_t, sizeof(menu_3_10_t));

	//Log4
	rtos_uart_send(rtos_uart0, &menu_3_5_t, sizeof(menu_3_5_t));
	if(log_menu_3.user_4 == PC)
	{
		rtos_uart_send(rtos_uart0, &menu_3_7_pc_t, sizeof(menu_3_7_pc_t));
	}
	else if (log_menu_3.user_4 == CEL)
	{
		rtos_uart_send(rtos_uart0, &menu_3_7_cel_t, sizeof(menu_3_7_cel_t));
	}
	else
	{
		rtos_uart_send(rtos_uart0, &empty, sizeof(empty));
	}
	time_to_array(log_menu_3.log_time_4);
	array_to_ascii_time();
	rtos_uart_send(rtos_uart0, time_array, sizeof(time_array));
	rtos_uart_send(rtos_uart0, log_menu_3.log_4_mem, sizeof(log_menu_3.log_4_mem));
	rtos_uart_send(rtos_uart0, &menu_3_9_t, sizeof(menu_3_9_t));
	rtos_uart_send(rtos_uart0, log_menu_3.data_4, sizeof(log_menu_3.data_4));
	rtos_uart_send(rtos_uart0, &menu_3_10_t, sizeof(menu_3_10_t));

	//Log5
	rtos_uart_send(rtos_uart0, &menu_3_6_t, sizeof(menu_3_6_t));
	if(log_menu_3.user_5 == PC)
	{
		rtos_uart_send(rtos_uart0, &menu_3_7_pc_t, sizeof(menu_3_7_pc_t));
	}
	else if (log_menu_3.user_5 == CEL)
	{
		rtos_uart_send(rtos_uart0, &menu_3_7_cel_t, sizeof(menu_3_7_cel_t));
	}
	else
	{
		rtos_uart_send(rtos_uart0, &empty, sizeof(empty));
	}
	time_to_array(log_menu_3.log_time_5);
	array_to_ascii_time();
	rtos_uart_send(rtos_uart0, time_array, sizeof(time_array));
	rtos_uart_send(rtos_uart0, log_menu_3.log_5_mem, sizeof(log_menu_3.log_5_mem));
	rtos_uart_send(rtos_uart0, &menu_3_9_t, sizeof(menu_3_9_t));
	rtos_uart_send(rtos_uart0, log_menu_3.data_5, sizeof(log_menu_3.data_5));
	rtos_uart_send(rtos_uart0, &menu_3_10_t, sizeof(menu_3_10_t));

	rtos_uart_send(rtos_uart0, &menu_return_to_main_t, sizeof(menu_return_to_main_t));
}

void menu_4()
{
	console_clear();
	actual_menu = MENU_4;
	if(modo_administrador == 1)
	{
		rtos_uart_send(rtos_uart0, &menu_admin_mode, sizeof(menu_admin_mode));
		rtos_uart_send(rtos_uart0, &menu_4_t, sizeof(menu_4_t));
	}
	else
	{
		rtos_uart_send(rtos_uart0, &menu_4_1_t, sizeof(menu_4_1_t));
		rtos_uart_send(rtos_uart0, &menu_return_to_main_t, sizeof(menu_return_to_main_t));
	}
}

void store_log(uint8_t user)
{
	switch (sm_log)
	{
		case 0:
		{
			switch (initial_log)
			{
				case 0:
				{
					for(int i=0; i<4; i++)
					{
						log_menu_3.log_1_mem[i] = dir_memoria[i];
					}
					for(int i=0; i<(strlen(data_to_write)<20?strlen(data_to_write):20);i++)
					{
						log_menu_3.data_1[i] = data_to_write[i];
					}
					log_menu_3.user_1 = user;
					log_menu_3.log_time_1 = rtos_i2c_rtc_read_time(rtos_i2c0);
					initial_log = 1;
					break;
				}
				case 1:
				{
					for(int i=0; i<4; i++)
					{
						log_menu_3.log_2_mem[i] = dir_memoria[i];
					}
					for(int i=0; i<(strlen(data_to_write)<20?strlen(data_to_write):20);i++)
					{
						log_menu_3.data_2[i] = data_to_write[i];
					}
					log_menu_3.user_2 = user;
					log_menu_3.log_time_2 = rtos_i2c_rtc_read_time(rtos_i2c0);
					initial_log = 2;
					break;
				}
				case 2:
				{
					for(int i=0; i<4; i++)
					{
						log_menu_3.log_3_mem[i] = dir_memoria[i];
					}
					for(int i=0; i<(strlen(data_to_write)<20?strlen(data_to_write):20);i++)
					{
						log_menu_3.data_3[i] = data_to_write[i];
					}
					log_menu_3.user_3 = user;
					log_menu_3.log_time_3 = rtos_i2c_rtc_read_time(rtos_i2c0);
					initial_log = 3;
					break;
				}
				case 3:
				{
					for(int i=0; i<4; i++)
					{
						log_menu_3.log_4_mem[i] = dir_memoria[i];
					}
					for(int i=0; i<(strlen(data_to_write)<20?strlen(data_to_write):20);i++)
					{
						log_menu_3.data_4[i] = data_to_write[i];
					}
					log_menu_3.user_4 = user;
					log_menu_3.log_time_4 = rtos_i2c_rtc_read_time(rtos_i2c0);
					initial_log = 4;
					break;
				}
				case 4:
				{
					for(int i=0; i<4; i++)
					{
						log_menu_3.log_5_mem[i] = dir_memoria[i];
					}
					for(int i=0; i<(strlen(data_to_write)<20?strlen(data_to_write):20);i++)
					{
						log_menu_3.data_5[i] = data_to_write[i];
					}
					log_menu_3.user_5 = user;
					log_menu_3.log_time_5 = rtos_i2c_rtc_read_time(rtos_i2c0);
					sm_log = 1;
					break;
				}
				default: break;
			}
			break;
		}
		case 1:
		{
			for(int i=0; i<4; i++)
			{
				log_menu_3.log_1_mem[i] = log_menu_3.log_2_mem[i];
				log_menu_3.log_2_mem[i] = log_menu_3.log_3_mem[i];
				log_menu_3.log_3_mem[i] = log_menu_3.log_4_mem[i];
				log_menu_3.log_4_mem[i] = log_menu_3.log_5_mem[i];
				log_menu_3.log_5_mem[i] = dir_memoria[i];
			}

			for(int i=0; i<(strlen(data_to_write)<20?strlen(data_to_write):20); i++)
			{
				log_menu_3.data_1[i] = log_menu_3.data_2[i];
				log_menu_3.data_2[i] = log_menu_3.data_3[i];
				log_menu_3.data_3[i] = log_menu_3.data_4[i];
				log_menu_3.data_4[i] = log_menu_3.data_5[i];
				log_menu_3.data_5[i] = data_to_write[i];
			}
			log_menu_3.user_1 = log_menu_3.user_2;
			log_menu_3.user_2 = log_menu_3.user_3;
			log_menu_3.user_3 = log_menu_3.user_4;
			log_menu_3.user_4 = log_menu_3.user_5;
			log_menu_3.user_5 = user;

			log_menu_3.log_time_1 = log_menu_3.log_time_2;
			log_menu_3.log_time_2 = log_menu_3.log_time_3;
			log_menu_3.log_time_3 = log_menu_3.log_time_4;
			log_menu_3.log_time_4 = log_menu_3.log_time_5;
			log_menu_3.log_time_5 = rtos_i2c_rtc_read_time(rtos_i2c0);

			break;
		}
		default: break;
	}
}

void console_clear_bt()
{
	rtos_uart_send(rtos_uart1, &console_clear_t, sizeof(console_clear_t));
}

void main_menu_bt()
{
	actual_menu_bt = MENU_MAIN_MENU;
	console_clear_bt();
	rtos_uart_send(rtos_uart1, &hora_actual, sizeof(hora_actual));
	time_to_array(rtos_i2c_rtc_read_time(rtos_i2c0));
	array_to_ascii_time();
	rtos_uart_send(rtos_uart1, time_array, sizeof(time_array));
	rtos_uart_send(rtos_uart1, menu_3_10_t, sizeof(menu_3_10_t)); //Aquí sólo imprimes un enter
	rtos_uart_send(rtos_uart1, &main_menu_t, sizeof(main_menu_t));
}

void menu_error_bt()
{
	actual_menu_bt = MENU_MAIN_MENU_ERROR;
	console_clear_bt();
	rtos_uart_send(rtos_uart1, &error_menu_t, sizeof(error_menu_t));
}

void menu_1_bt()
{
	actual_menu_bt = MENU_1;
	console_clear_bt();
	rtos_uart_send(rtos_uart1, &menu_1_t, sizeof(menu_1_t));
}

void menu_2_bt()
{
	actual_menu_bt = MENU_2;
	console_clear_bt();
	rtos_uart_send(rtos_uart1, &menu_2_t, sizeof(menu_2_t));
}

void menu_3_bt()
{
	actual_menu_bt = MENU_3;
	console_clear_bt();
	rtos_uart_send(rtos_uart1, &menu_3_t, sizeof(menu_3_t));
	rtos_uart_send(rtos_uart1, &menu_3_1_t, sizeof(menu_3_1_t));

	//Log1
	rtos_uart_send(rtos_uart1, &menu_3_2_t, sizeof(menu_3_2_t));
	if(log_menu_3.user_1 == PC)
	{
		rtos_uart_send(rtos_uart1, &menu_3_7_pc_t, sizeof(menu_3_7_pc_t));
	}
	else if (log_menu_3.user_1 == CEL)
	{
		rtos_uart_send(rtos_uart1, &menu_3_7_cel_t, sizeof(menu_3_7_cel_t));
	}
	else
	{
		rtos_uart_send(rtos_uart1, &empty, sizeof(empty));
	}
	time_to_array(log_menu_3.log_time_1);
	array_to_ascii_time();
	rtos_uart_send(rtos_uart1, time_array, sizeof(time_array));
	rtos_uart_send(rtos_uart1, log_menu_3.log_1_mem, sizeof(log_menu_3.log_1_mem));
	rtos_uart_send(rtos_uart1, &menu_3_9_t, sizeof(menu_3_9_t));
	rtos_uart_send(rtos_uart1, log_menu_3.data_1, sizeof(log_menu_3.data_1));
	rtos_uart_send(rtos_uart1, &menu_3_10_t, sizeof(menu_3_10_t));

	//Log2
	rtos_uart_send(rtos_uart1, &menu_3_3_t, sizeof(menu_3_3_t));
	if(log_menu_3.user_2 == PC)
	{
		rtos_uart_send(rtos_uart1, &menu_3_7_pc_t, sizeof(menu_3_7_pc_t));
	}
	else if (log_menu_3.user_2 == CEL)
	{
		rtos_uart_send(rtos_uart1, &menu_3_7_cel_t, sizeof(menu_3_7_cel_t));
	}
	else
	{
		rtos_uart_send(rtos_uart1, &empty, sizeof(empty));
	}
	time_to_array(log_menu_3.log_time_2);
	array_to_ascii_time();
	rtos_uart_send(rtos_uart1, time_array, sizeof(time_array));
	rtos_uart_send(rtos_uart1, log_menu_3.log_2_mem, sizeof(log_menu_3.log_2_mem));
	rtos_uart_send(rtos_uart1, &menu_3_9_t, sizeof(menu_3_9_t));
	rtos_uart_send(rtos_uart1, log_menu_3.data_2, sizeof(log_menu_3.data_2));
	rtos_uart_send(rtos_uart1, &menu_3_10_t, sizeof(menu_3_10_t));

	//Log3
	rtos_uart_send(rtos_uart1, &menu_3_4_t, sizeof(menu_3_4_t));
	if(log_menu_3.user_3 == PC)
	{
		rtos_uart_send(rtos_uart1, &menu_3_7_pc_t, sizeof(menu_3_7_pc_t));
	}
	else if (log_menu_3.user_3 == CEL)
	{
		rtos_uart_send(rtos_uart1, &menu_3_7_cel_t, sizeof(menu_3_7_cel_t));
	}
	else
	{
		rtos_uart_send(rtos_uart1, &empty, sizeof(empty));
	}
	time_to_array(log_menu_3.log_time_3);
	array_to_ascii_time();
	rtos_uart_send(rtos_uart1, time_array, sizeof(time_array));
	rtos_uart_send(rtos_uart1, log_menu_3.log_3_mem, sizeof(log_menu_3.log_3_mem));
	rtos_uart_send(rtos_uart1, &menu_3_9_t, sizeof(menu_3_9_t));
	rtos_uart_send(rtos_uart1, log_menu_3.data_3, sizeof(log_menu_3.data_3));
	rtos_uart_send(rtos_uart1, &menu_3_10_t, sizeof(menu_3_10_t));

	//Log4
	rtos_uart_send(rtos_uart1, &menu_3_5_t, sizeof(menu_3_5_t));
	if(log_menu_3.user_4 == PC)
	{
		rtos_uart_send(rtos_uart1, &menu_3_7_pc_t, sizeof(menu_3_7_pc_t));
	}
	else if (log_menu_3.user_4 == CEL)
	{
		rtos_uart_send(rtos_uart1, &menu_3_7_cel_t, sizeof(menu_3_7_cel_t));
	}
	else
	{
		rtos_uart_send(rtos_uart1, &empty, sizeof(empty));
	}
	time_to_array(log_menu_3.log_time_4);
	array_to_ascii_time();
	rtos_uart_send(rtos_uart1, time_array, sizeof(time_array));
	rtos_uart_send(rtos_uart1, log_menu_3.log_4_mem, sizeof(log_menu_3.log_4_mem));
	rtos_uart_send(rtos_uart1, &menu_3_9_t, sizeof(menu_3_9_t));
	rtos_uart_send(rtos_uart1, log_menu_3.data_4, sizeof(log_menu_3.data_4));
	rtos_uart_send(rtos_uart1, &menu_3_10_t, sizeof(menu_3_10_t));

	//Log5
	rtos_uart_send(rtos_uart1, &menu_3_6_t, sizeof(menu_3_6_t));
	if(log_menu_3.user_5 == PC)
	{
		rtos_uart_send(rtos_uart1, &menu_3_7_pc_t, sizeof(menu_3_7_pc_t));
	}
	else if (log_menu_3.user_5 == CEL)
	{
		rtos_uart_send(rtos_uart1, &menu_3_7_cel_t, sizeof(menu_3_7_cel_t));
	}
	else
	{
		rtos_uart_send(rtos_uart1, &empty, sizeof(empty));
	}
	time_to_array(log_menu_3.log_time_5);
	array_to_ascii_time();
	rtos_uart_send(rtos_uart1, time_array, sizeof(time_array));
	rtos_uart_send(rtos_uart1, log_menu_3.log_5_mem, sizeof(log_menu_3.log_5_mem));
	rtos_uart_send(rtos_uart1, &menu_3_9_t, sizeof(menu_3_9_t));
	rtos_uart_send(rtos_uart1, log_menu_3.data_5, sizeof(log_menu_3.data_5));
	rtos_uart_send(rtos_uart1, &menu_3_10_t, sizeof(menu_3_10_t));

	rtos_uart_send(rtos_uart1, &menu_return_to_main_t, sizeof(menu_return_to_main_t));
}

void menu_4_bt()
{
	console_clear_bt();
	actual_menu_bt = MENU_4;
	if(modo_administrador == 1)
	{
		rtos_uart_send(rtos_uart1, &menu_admin_mode, sizeof(menu_admin_mode));
		rtos_uart_send(rtos_uart1, &menu_4_t, sizeof(menu_4_t));
	}
	else
	{
		rtos_uart_send(rtos_uart1, &menu_4_1_t, sizeof(menu_4_1_t));
		rtos_uart_send(rtos_uart1, &menu_return_to_main_t, sizeof(menu_return_to_main_t));
	}
}

void data_input_manager(uint8_t data)
{
	switch(actual_menu)
	{
		case MENU_MAIN_MENU:
		{
			if(0xd == data)
			{
				switch(old_data)
				{
					case 0x31: menu_1(); break;
					case 0x32: menu_2(); break;
					case 0x33: menu_3(); break;
					case 0x34: menu_4(); break;
					default: menu_error(); break;
				}
			}
			else
			{
				old_data = data;
			}
			break;
		}
		case MENU_MAIN_MENU_ERROR:
		{
			main_menu();
			break;
		}
		case MENU_HOUR_ERROR:
		{
			main_menu();
			break;
		}
		case MENU_1:
		{
			if(dir_memoria[0] == NO_ASIGNADO)
			{
				dir_memoria[0] = data;
				break;
			}
			else if(dir_memoria[1] == NO_ASIGNADO)
			{
				dir_memoria[1] = data;
				break;
			}
			else if(dir_memoria[2] == NO_ASIGNADO)
			{
				dir_memoria[2] = data;
				break;
			}
			else if(dir_memoria[3] == NO_ASIGNADO)
			{
				dir_memoria[3] = data;
				break;
			}
			else
			{
				if (flag_menu_1 == 0)
				{
					rtos_uart_send(rtos_uart0, &menu_1_1_t, sizeof(menu_1_1_t));
					flag_menu_1 = 1;
				}
				else if (flag_menu_1 == 1)
				{
					if(0xd == data){
						data_to_write[wr_chars] = '\0';
						wr_chars = 0;

						//GUARDAR ELEMENTO EN LA MEMORIA.
						//LA DIRECCIÓN DE LA MEMORIA ESTÁ EN EL ARREGLO "dir_memoria[]" Y LO QUE SE GUARDA EN "data_to_write"

						store_log(PC);
						rtos_uart_send(rtos_uart0, &menu_1_2_t, sizeof(menu_1_2_t));
						rtos_uart_send(rtos_uart0, data_to_write, strlen(data_to_write));
						rtos_uart_send(rtos_uart0, &menu_return_to_main_t, sizeof(menu_return_to_main_t));
						flag_menu_1 = 2;
					}
					else{
						realloc(data_to_write, sizeof(char));
						data_to_write[wr_chars] = data;
						wr_chars++;
					}
				}
				else if (flag_menu_1 == 2)
				{
					dir_memoria[0] = NO_ASIGNADO;
					dir_memoria[1] = NO_ASIGNADO;
					dir_memoria[2] = NO_ASIGNADO;
					dir_memoria[3] = NO_ASIGNADO;
					flag_menu_1 = 0;
					wr_chars = 0;
					free(data_to_write);
					main_menu();
				}
				break;
			}
		}
		case MENU_2:
		{
			if(dir_memoria[0] == NO_ASIGNADO)
			{
				dir_memoria[0] = data;
				break;
			}
			else if(dir_memoria[1] == NO_ASIGNADO)
			{
				dir_memoria[1] = data;
				break;
			}
			else if(dir_memoria[2] == NO_ASIGNADO)
			{
				dir_memoria[2] = data;
				break;
			}
			else if(dir_memoria[3] == NO_ASIGNADO)
			{
				dir_memoria[3] = data;
				break;
			}
			else
			{
				if (flag_menu_2 == 0)
				{
					rtos_uart_send(rtos_uart0, &menu_2_1_t, sizeof(menu_2_1_t));
					flag_menu_2 = 1;
				}
				else if (flag_menu_2 == 1)
				{
					if(data == 0xd)
					{
						size_to_read = data -'0';
						rtos_uart_send(rtos_uart0, &menu_2_2_t, sizeof(menu_2_2_t));

						//AQUÍ SE LEE DE LA MEMORIA
						//LA DIRECCIÓN DE LA QUE SE VA A LEER ESTÁ EN EL ARREGLO "dir_memoria", Y EL TAMAÑO DE LO QUE SE DEBE LEER ESTÁ EN "size_to_read".

						rtos_uart_send(rtos_uart0, &menu_return_to_main_t, sizeof(menu_return_to_main_t));
						flag_menu_2 = 2;
					}
				}
				else if (flag_menu_2 == 2)
				{
					dir_memoria[0] = NO_ASIGNADO;
					dir_memoria[1] = NO_ASIGNADO;
					dir_memoria[2] = NO_ASIGNADO;
					dir_memoria[3] = NO_ASIGNADO;
					flag_menu_2 = 0;
					main_menu();
				}
				break;
			}
		}
		case MENU_4:
		{
			if(modo_administrador == 1)
			{
				switch(sm_set_hour)
				{
					case 0:
					{
						if(data == 0x31 || data == 0x32 || data == 0x30)
						{
							hour_to_set.hour = data -'0';
							sm_set_hour = 1;
						}
						else
						{
							actual_menu = MENU_HOUR_ERROR;
							rtos_uart_send(rtos_uart0, &error_time_t, sizeof(error_time_t));
							sm_set_hour = 0;
						}
						break;
					}
					case 1:
					{
						if(data == 0x31 || data == 0x32 || data == 0x33 || data == 0x34 || data == 0x35 || data == 0x36 || data == 0x37 || data == 0x38 || data == 0x39 || data == 0x30)
						{
							if(hour_to_set.hour == 0)
							{
								hour_to_set.hour = data - '0';
							}
							if(hour_to_set.hour == 1)
							{
								hour_to_set.hour = (data - '0') + 10;
							}
							if(hour_to_set.hour == 2)
							{
								hour_to_set.hour = (data - '0') + 20;
							}
							sm_set_hour = 2;
						}
						else
						{
							actual_menu = MENU_HOUR_ERROR;
							rtos_uart_send(rtos_uart0, &error_time_t, sizeof(error_time_t));
							sm_set_hour = 0;
						}
						break;
					}
					case 2:
					{
						sm_set_hour = 3;
						break;
					}
					case 3:
					{
						if(data == 0x31 || data == 0x32 || data == 0x33 || data == 0x34 || data == 0x35 || data == 0x30)
						{
							hour_to_set.minute = data -'0';
							sm_set_hour = 4;
						}
						else
						{
							actual_menu = MENU_HOUR_ERROR;
							rtos_uart_send(rtos_uart0, &error_time_t, sizeof(error_time_t));
							sm_set_hour = 0;
						}
						break;
					}
					case 4:
					{
						if(data == 0x31 || data == 0x32 || data == 0x33 || data == 0x34 || data == 0x35 || data == 0x36 || data == 0x37 || data == 0x38 || data == 0x39 || data == 0x30)
						{
							if(hour_to_set.minute == 0)
							{
								hour_to_set.minute = data - '0';
							}
							if(hour_to_set.minute == 1)
							{
								hour_to_set.minute = (data - '0') + 10;
							}
							if(hour_to_set.minute == 2)
							{
								hour_to_set.minute = (data - '0') + 20;
							}
							if(hour_to_set.minute == 3)
							{
								hour_to_set.minute = (data - '0') + 30;
							}
							if(hour_to_set.minute == 4)
							{
								hour_to_set.minute = (data - '0') + 40;
							}
							if(hour_to_set.minute == 5)
							{
								hour_to_set.minute = (data - '0') + 50;
							}
							sm_set_hour = 5;
						}
						else
						{
							actual_menu = MENU_HOUR_ERROR;
							rtos_uart_send(rtos_uart0, &error_time_t, sizeof(error_time_t));
							sm_set_hour = 0;
						}
						break;
					}
					case 5:
					{
						sm_set_hour = 6;
						break;
					}
					case 6:
					{
						if(data == 0x31 || data == 0x32 || data == 0x33 || data == 0x34 || data == 0x35 || data == 0x30)
						{
							hour_to_set.second = data -'0';
							sm_set_hour = 7;
						}
						else
						{
							actual_menu = MENU_HOUR_ERROR;
							rtos_uart_send(rtos_uart0, &error_time_t, sizeof(error_time_t));
							sm_set_hour = 0;
						}
						break;
					}
					case 7:
					{
						if(data == 0x31 || data == 0x32 || data == 0x33 || data == 0x34 || data == 0x35 || data == 0x36 || data == 0x37 || data == 0x38 || data == 0x39 || data == 0x30)
						{
							if(hour_to_set.second == 0)
							{
								hour_to_set.second = data - '0';
							}
							if(hour_to_set.second == 1)
							{
								hour_to_set.second = (data - '0') + 10;
							}
							if(hour_to_set.second == 2)
							{
								hour_to_set.second = (data - '0') + 20;
							}
							if(hour_to_set.second == 3)
							{
								hour_to_set.second = (data - '0') + 30;
							}
							if(hour_to_set.second == 4)
							{
								hour_to_set.second = (data - '0') + 40;
							}
							if(hour_to_set.second == 5)
							{
								hour_to_set.second = (data - '0') + 50;
							}
							sm_set_hour = 8;
						}
						else
						{
							actual_menu = MENU_HOUR_ERROR;
							rtos_uart_send(rtos_uart0, &error_time_t, sizeof(error_time_t));
							sm_set_hour = 0;
						}
						break;
					}
					case 8:
					{
						hour_to_set.hour = int_to_hex(hour_to_set.hour);
						hour_to_set.minute = int_to_hex(hour_to_set.minute);
						hour_to_set.second = to_seconds(int_to_hex(hour_to_set.second));
						rtos_i2c_rtc_set_hour(rtos_i2c0,hour_to_set);
						rtos_uart_send(rtos_uart0, &set_time_finish, sizeof(set_time_finish));
						sm_set_hour = 9;
						break;
					}
					case 9:
					{
						sm_set_hour = 0;
						main_menu();
						break;
					}
				}

			}
			else
			{
				main_menu();
			}
			break;
		}
		default: main_menu();
	}
}

void data_input_manager_bt(uint8_t data)
{
	switch(actual_menu_bt)
	{
		case MENU_MAIN_MENU:
		{
			if(0xd == data)
			{
				switch(old_data)
				{
					case 0x31: menu_1_bt(); break;
					case 0x32: menu_2_bt(); break;
					case 0x33: menu_3_bt(); break;
					case 0x34: menu_4_bt(); break;
					default: menu_error_bt(); break;
				}
			}
			else
			{
				old_data = data;
			}
			break;
		}
		case MENU_MAIN_MENU_ERROR:
		{
			main_menu_bt();
			break;
		}
		case MENU_HOUR_ERROR:
		{
			main_menu_bt();
			break;
		}
		case MENU_1:
		{
			if(dir_memoria[0] == NO_ASIGNADO)
			{
				dir_memoria[0] = data;
				break;
			}
			else if(dir_memoria[1] == NO_ASIGNADO)
			{
				dir_memoria[1] = data;
				break;
			}
			else if(dir_memoria[2] == NO_ASIGNADO)
			{
				dir_memoria[2] = data;
				break;
			}
			else if(dir_memoria[3] == NO_ASIGNADO)
			{
				dir_memoria[3] = data;
				break;
			}
			else
			{
				if (flag_menu_1 == 0)
				{
					rtos_uart_send(rtos_uart1, &menu_1_1_t, sizeof(menu_1_1_t));
					flag_menu_1 = 1;
				}
				else if (flag_menu_1 == 1)
				{
					if(0xd == data){
						data_to_write[wr_chars] = '\0';
						wr_chars = 0;

						//GUARDAR ELEMENTO EN LA MEMORIA.
						//LA DIRECCIÓN DE LA MEMORIA ESTÁ EN EL ARREGLO "dir_memoria[]" Y LO QUE SE GUARDA EN "data_to_write"

						store_log(CEL);
						rtos_uart_send(rtos_uart1, &menu_1_2_t, sizeof(menu_1_2_t));
						rtos_uart_send(rtos_uart1, data_to_write, strlen(data_to_write));
						rtos_uart_send(rtos_uart1, &menu_return_to_main_t, sizeof(menu_return_to_main_t));
						flag_menu_1 = 2;
					}
					else{
						realloc(data_to_write, sizeof(char));
						data_to_write[wr_chars] = data;
						wr_chars++;
					}
				}
				else if (flag_menu_1 == 2)
				{
					dir_memoria[0] = NO_ASIGNADO;
					dir_memoria[1] = NO_ASIGNADO;
					dir_memoria[2] = NO_ASIGNADO;
					dir_memoria[3] = NO_ASIGNADO;
					flag_menu_1 = 0;
					wr_chars = 0;
					free(data_to_write);
					main_menu_bt();
				}
				break;
			}
		}
		case MENU_2:
		{
			if(dir_memoria[0] == NO_ASIGNADO)
			{
				dir_memoria[0] = data;
				break;
			}
			else if(dir_memoria[1] == NO_ASIGNADO)
			{
				dir_memoria[1] = data;
				break;
			}
			else if(dir_memoria[2] == NO_ASIGNADO)
			{
				dir_memoria[2] = data;
				break;
			}
			else if(dir_memoria[3] == NO_ASIGNADO)
			{
				dir_memoria[3] = data;
				break;
			}
			else
			{
				if (flag_menu_2 == 0)
				{
					rtos_uart_send(rtos_uart1, &menu_2_1_t, sizeof(menu_2_1_t));
					flag_menu_2 = 1;
				}
				else if (flag_menu_2 == 1)
				{
					if(data == 0xd)
					{
						size_to_read = data -'0';
						rtos_uart_send(rtos_uart1, &menu_2_2_t, sizeof(menu_2_2_t));

						//AQUÍ SE LEE DE LA MEMORIA
						//LA DIRECCIÓN DE LA QUE SE VA A LEER ESTÁ EN EL ARREGLO "dir_memoria", Y EL TAMAÑO DE LO QUE SE DEBE LEER ESTÁ EN "size_to_read".

						rtos_uart_send(rtos_uart1, &menu_return_to_main_t, sizeof(menu_return_to_main_t));
						flag_menu_2 = 2;
					}
				}
				else if (flag_menu_2 == 2)
				{
					dir_memoria[0] = NO_ASIGNADO;
					dir_memoria[1] = NO_ASIGNADO;
					dir_memoria[2] = NO_ASIGNADO;
					dir_memoria[3] = NO_ASIGNADO;
					flag_menu_2 = 0;
					main_menu_bt();
				}
				break;
			}
		}
		case MENU_4:
		{
			if(modo_administrador == 1)
			{
				switch(sm_set_hour)
				{
					case 0:
					{
						if(data == 0x31 || data == 0x32 || data == 0x30)
						{
							hour_to_set.hour = data -'0';
							sm_set_hour = 1;
						}
						else
						{
							actual_menu_bt = MENU_HOUR_ERROR;
							rtos_uart_send(rtos_uart1, &error_time_t, sizeof(error_time_t));
							sm_set_hour = 0;
						}
						break;
					}
					case 1:
					{
						if(data == 0x31 || data == 0x32 || data == 0x33 || data == 0x34 || data == 0x35 || data == 0x36 || data == 0x37 || data == 0x38 || data == 0x39 || data == 0x30)
						{
							if(hour_to_set.hour == 0)
							{
								hour_to_set.hour = data - '0';
							}
							if(hour_to_set.hour == 1)
							{
								hour_to_set.hour = (data - '0') + 10;
							}
							if(hour_to_set.hour == 2)
							{
								hour_to_set.hour = (data - '0') + 20;
							}
							sm_set_hour = 2;
						}
						else
						{
							actual_menu_bt = MENU_HOUR_ERROR;
							rtos_uart_send(rtos_uart1, &error_time_t, sizeof(error_time_t));
							sm_set_hour = 0;
						}
						break;
					}
					case 2:
					{
						sm_set_hour = 3;
						break;
					}
					case 3:
					{
						if(data == 0x31 || data == 0x32 || data == 0x33 || data == 0x34 || data == 0x35 || data == 0x30)
						{
							hour_to_set.minute = data -'0';
							sm_set_hour = 4;
						}
						else
						{
							actual_menu_bt = MENU_HOUR_ERROR;
							rtos_uart_send(rtos_uart1, &error_time_t, sizeof(error_time_t));
							sm_set_hour = 0;
						}
						break;
					}
					case 4:
					{
						if(data == 0x31 || data == 0x32 || data == 0x33 || data == 0x34 || data == 0x35 || data == 0x36 || data == 0x37 || data == 0x38 || data == 0x39 || data == 0x30)
						{
							if(hour_to_set.minute == 0)
							{
								hour_to_set.minute = data - '0';
							}
							if(hour_to_set.minute == 1)
							{
								hour_to_set.minute = (data - '0') + 10;
							}
							if(hour_to_set.minute == 2)
							{
								hour_to_set.minute = (data - '0') + 20;
							}
							if(hour_to_set.minute == 3)
							{
								hour_to_set.minute = (data - '0') + 30;
							}
							if(hour_to_set.minute == 4)
							{
								hour_to_set.minute = (data - '0') + 40;
							}
							if(hour_to_set.minute == 5)
							{
								hour_to_set.minute = (data - '0') + 50;
							}
							sm_set_hour = 5;
						}
						else
						{
							actual_menu_bt = MENU_HOUR_ERROR;
							rtos_uart_send(rtos_uart1, &error_time_t, sizeof(error_time_t));
							sm_set_hour = 0;
						}
						break;
					}
					case 5:
					{
						sm_set_hour = 6;
						break;
					}
					case 6:
					{
						if(data == 0x31 || data == 0x32 || data == 0x33 || data == 0x34 || data == 0x35 || data == 0x30)
						{
							hour_to_set.second = data -'0';
							sm_set_hour = 7;
						}
						else
						{
							actual_menu_bt = MENU_HOUR_ERROR;
							rtos_uart_send(rtos_uart1, &error_time_t, sizeof(error_time_t));
							sm_set_hour = 0;
						}
						break;
					}
					case 7:
					{
						if(data == 0x31 || data == 0x32 || data == 0x33 || data == 0x34 || data == 0x35 || data == 0x36 || data == 0x37 || data == 0x38 || data == 0x39 || data == 0x30)
						{
							if(hour_to_set.second == 0)
							{
								hour_to_set.second = data - '0';
							}
							if(hour_to_set.second == 1)
							{
								hour_to_set.second = (data - '0') + 10;
							}
							if(hour_to_set.second == 2)
							{
								hour_to_set.second = (data - '0') + 20;
							}
							if(hour_to_set.second == 3)
							{
								hour_to_set.second = (data - '0') + 30;
							}
							if(hour_to_set.second == 4)
							{
								hour_to_set.second = (data - '0') + 40;
							}
							if(hour_to_set.second == 5)
							{
								hour_to_set.second = (data - '0') + 50;
							}
							sm_set_hour = 8;
						}
						else
						{
							actual_menu_bt = MENU_HOUR_ERROR;
							rtos_uart_send(rtos_uart1, &error_time_t, sizeof(error_time_t));
							sm_set_hour = 0;
						}
						break;
					}
					case 8:
					{
						hour_to_set.hour = int_to_hex(hour_to_set.hour);
						hour_to_set.minute = int_to_hex(hour_to_set.minute);
						hour_to_set.second = to_seconds(int_to_hex(hour_to_set.second));
						rtos_i2c_rtc_set_hour(rtos_i2c0,hour_to_set);
						rtos_uart_send(rtos_uart1, &set_time_finish, sizeof(set_time_finish));
						sm_set_hour = 9;
						break;
					}
					case 9:
					{
						sm_set_hour = 0;
						main_menu_bt();
						break;
					}
				}
			}
			else
			{
				main_menu_bt();
			}
			break;
		}
		default: main_menu_bt();
	}
}

void uart_echo_task()
{
	uint8_t data;
	for(;;)
	{
		rtos_uart_receive(rtos_uart0, &data, 1);
		data_input_manager(data);
		rtos_uart_send(rtos_uart0, &data, 1);
	}
}

void uart_echo_task_bt()
{
	uint8_t data;
	for(;;)
	{
		rtos_uart_receive(rtos_uart1, &data, 1);
		data_input_manager_bt(data);
		rtos_uart_send(rtos_uart1, &data, 1);
	}
}

void gpio_tooglePin_task()
{
	//CONFIGURACIÓN LED Y SWITCH
	rtos_gpio_config_t config_LED;
	rtos_gpio_config_t config_SW2;
	config_LED.gpio = rtos_gpioB;
	config_LED.port = rtos_gpio_portB;
	config_LED.pin = 22;
	config_LED.pin_direction = rtos_gpio_output;
	rtos_gpio_init(config_LED);
	config_SW2.gpio = rtos_gpioC;
	config_SW2.port = rtos_gpio_portC;
	config_SW2.pin = 6;
	config_SW2.pin_direction = rtos_gpio_input;
	rtos_gpio_init(config_SW2);

	for(;;)
	{
		rtos_gpio_wait_pin(config_SW2);
		modo_administrador = rtos_gpio_toogle_pin(config_LED, modo_administrador);
	}
}

void system_start()
{
	data_to_write = malloc(sizeof(char));
	size_to_read_c = malloc(sizeof(char));

	//Configuración UART USB
	rtos_uart_config_t config1;
	config1.baudrate = 115200;
	config1.rx_pin = 16;
	config1.tx_pin = 17;
	config1.pin_mux = kPORT_MuxAlt3;
	config1.uart_number = rtos_uart0;
	config1.port = rtos_uart_portB;
	rtos_uart_init(config1);

	//Configuración UART BLUETOOTH
	rtos_uart_config_t config2;
	config2.baudrate = 9600;
	config2.rx_pin = 3;
	config2.tx_pin = 4;
	config2.pin_mux = kPORT_MuxAlt3;
	config2.uart_number = rtos_uart1;
	config2.port = rtos_uart_portC;
	rtos_uart_init(config2);

	//RTC INIT
	rtos_i2c_config_t rtc_config;
	rtc_config.baudrate = 100000;
	rtc_config.i2c_number = rtos_i2c0;
	rtc_config.i2c_port = rtos_i2c_portB;
	rtc_config.mux = kPORT_MuxAlt2;
	rtc_config.scl_pin = 2;
	rtc_config.sda_pin = 3;
	rtos_i2c_init(rtc_config);

	//ST
	rtos_i2c_rtc_st(rtos_i2c0);

	hour_to_set.hour = 0x00;
	hour_to_set.minute = 0x00;
	hour_to_set.second = to_seconds(0x00);

	rtos_i2c_rtc_set_hour(rtos_i2c0,hour_to_set);

	main_menu();
	main_menu_bt();
	vTaskSuspend(NULL);
}

int main (void)
{

	BOARD_BootClockRUN();
	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();
	BOARD_InitDebugConsole();

	xTaskCreate(uart_echo_task, "uart_echo_task", 110, NULL, 1, NULL);
	xTaskCreate(uart_echo_task_bt, "uart_echo_task_bt", 110, NULL, 1, NULL);
	xTaskCreate(system_start, "Inicializa el programa", configMINIMAL_STACK_SIZE+100, NULL, SYSTEM_START_PRIORITY, NULL);
	xTaskCreate(gpio_tooglePin_task, "gpio_tooglePin_task", 110, NULL, 1, NULL);

	vTaskStartScheduler();

    while(1) {
    }
    return 0 ;

}
