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

#define SYSTEM_START_PRIORITY		(configMAX_PRIORITIES)
#define MENU_MAIN_MENU				(0)
#define MENU_MAIN_MENU_ERROR		(1)
#define MENU_1						(2)
#define MENU_2						(3)
#define MENU_3						(4)
#define MENU_4						(5)
#define NO_ASIGNADO					(0x5A)

uint8_t old_data;
uint8_t actual_menu;
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
	char data_1[5];
	char data_2[5];
	char data_3[5];
	char data_4[5];
	char data_5[5];
}log_memoria;

typedef struct
{
	uint8_t horas;
	uint8_t minutos;
	uint8_t segundos;
}time;

/*TEXTOS DE LOS MENÚS*/
char console_clear_t[] = "\e[1;1H\e[2J";
char main_menu_t[] = "\r+++---MAIN MENU---+++\n\n\rControl del modulo de memoria compartida:\n\r1) Escribir en direccion.\n\r2) Leer de direccion\n\r3) Historial\n\r4) Establecer hora\n\rSelecciona una opcion del menu superior:  \n\r";
char error_menu_t[] = "\r*****ERROR*****\n\n\rIngresa una opcion correcta (1,2,3,4) del menu anterior\n\rPulsa cualquier tecla para continuar: \n\r";
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
log_memoria log_menu_3;
time hour_to_set;

/*HISTORIAL DE LOGS*/
char menu_3_1_t[] = "\r\nEntrada:\t\tUsuario:\t\tHora:\t\tLocalidad:\t\tDatos:\n\n";
char menu_3_2_t[] = "\r--1--\t\t\t";
char menu_3_3_t[] = "\r--2--\t\t\t";
char menu_3_4_t[] = "\r--3--\t\t\t";
char menu_3_5_t[] = "\r--4--\t\t\t";
char menu_3_6_t[] = "\r--5--\t\t\t";
char menu_3_7_lap_t[] = "Lap\t\t\t";
char menu_3_7_cel_t[] = "Cel\t\t\t";
char menu_3_8_t[] = "12:00:00\t";
char menu_3_9_t[] = "\t\t\t";
char menu_3_10_t[] = "\n";


void main_menu();

void console_clear()
{
	rtos_uart_send(rtos_uart0, &console_clear_t, sizeof(console_clear_t));

	//TEMPORAL
	rtos_uart_send(rtos_uart1, &console_clear_t, sizeof(console_clear_t));

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
	rtos_uart_send(rtos_uart0, &menu_3_7_cel_t, sizeof(menu_3_7_cel_t));
	rtos_uart_send(rtos_uart0, &menu_3_8_t, sizeof(menu_3_8_t));
	rtos_uart_send(rtos_uart0, log_menu_3.log_1_mem, sizeof(log_menu_3.log_1_mem));
	rtos_uart_send(rtos_uart0, &menu_3_9_t, sizeof(menu_3_9_t));
	rtos_uart_send(rtos_uart0, log_menu_3.data_1, sizeof(log_menu_3.data_1));
	rtos_uart_send(rtos_uart0, &menu_3_10_t, sizeof(menu_3_10_t));

	//Log2
	rtos_uart_send(rtos_uart0, &menu_3_3_t, sizeof(menu_3_3_t));
	rtos_uart_send(rtos_uart0, &menu_3_7_cel_t, sizeof(menu_3_7_cel_t));
	rtos_uart_send(rtos_uart0, &menu_3_8_t, sizeof(menu_3_8_t));
	rtos_uart_send(rtos_uart0, log_menu_3.log_2_mem, sizeof(log_menu_3.log_2_mem));
	rtos_uart_send(rtos_uart0, &menu_3_9_t, sizeof(menu_3_9_t));
	rtos_uart_send(rtos_uart0, log_menu_3.data_2, sizeof(log_menu_3.data_2));
	rtos_uart_send(rtos_uart0, &menu_3_10_t, sizeof(menu_3_10_t));

	//Log3
	rtos_uart_send(rtos_uart0, &menu_3_4_t, sizeof(menu_3_4_t));
	rtos_uart_send(rtos_uart0, &menu_3_7_cel_t, sizeof(menu_3_7_cel_t));
	rtos_uart_send(rtos_uart0, &menu_3_8_t, sizeof(menu_3_8_t));
	rtos_uart_send(rtos_uart0, log_menu_3.log_3_mem, sizeof(log_menu_3.log_3_mem));
	rtos_uart_send(rtos_uart0, &menu_3_9_t, sizeof(menu_3_9_t));
	rtos_uart_send(rtos_uart0, log_menu_3.data_3, sizeof(log_menu_3.data_3));
	rtos_uart_send(rtos_uart0, &menu_3_10_t, sizeof(menu_3_10_t));

	//Log4
	rtos_uart_send(rtos_uart0, &menu_3_5_t, sizeof(menu_3_5_t));
	rtos_uart_send(rtos_uart0, &menu_3_7_cel_t, sizeof(menu_3_7_cel_t));
	rtos_uart_send(rtos_uart0, &menu_3_8_t, sizeof(menu_3_8_t));
	rtos_uart_send(rtos_uart0, log_menu_3.log_4_mem, sizeof(log_menu_3.log_4_mem));
	rtos_uart_send(rtos_uart0, &menu_3_9_t, sizeof(menu_3_9_t));
	rtos_uart_send(rtos_uart0, log_menu_3.data_4, sizeof(log_menu_3.data_4));
	rtos_uart_send(rtos_uart0, &menu_3_10_t, sizeof(menu_3_10_t));

	//Log5
	rtos_uart_send(rtos_uart0, &menu_3_6_t, sizeof(menu_3_6_t));
	rtos_uart_send(rtos_uart0, &menu_3_7_cel_t, sizeof(menu_3_7_cel_t));
	rtos_uart_send(rtos_uart0, &menu_3_8_t, sizeof(menu_3_8_t));
	rtos_uart_send(rtos_uart0, log_menu_3.log_5_mem, sizeof(log_menu_3.log_5_mem));
	rtos_uart_send(rtos_uart0, &menu_3_9_t, sizeof(menu_3_9_t));
	rtos_uart_send(rtos_uart0, log_menu_3.data_5, sizeof(log_menu_3.data_5));
	rtos_uart_send(rtos_uart0, &menu_3_10_t, sizeof(menu_3_10_t));

	rtos_uart_send(rtos_uart0, &menu_return_to_main_t, sizeof(menu_return_to_main_t));
}

void menu_4()
{
	actual_menu = MENU_4;
	console_clear();
	if(modo_administrador == 0) //DEBE SER 1, PARA QUE SÓLO PUEDA ENTRAR COMO ADMINISTRADOR
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

void store_log()
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
					log_menu_3.data_1[0] = data_to_write[0];
					log_menu_3.data_1[1] = data_to_write[1];
					log_menu_3.data_1[2] = data_to_write[2];
					log_menu_3.data_1[3] = data_to_write[3];
					log_menu_3.data_1[4] = data_to_write[4];
					log_menu_3.data_1[5] = data_to_write[5];
					initial_log = 1;
					break;
				}
				case 1:
				{
					for(int i=0; i<4; i++)
					{
						log_menu_3.log_2_mem[i] = dir_memoria[i];
					}
					log_menu_3.data_2[0] = data_to_write[0];
					log_menu_3.data_2[1] = data_to_write[1];
					log_menu_3.data_2[2] = data_to_write[2];
					log_menu_3.data_2[3] = data_to_write[3];
					log_menu_3.data_2[4] = data_to_write[4];
					log_menu_3.data_2[5] = data_to_write[5];
					initial_log = 2;
					break;
				}
				case 2:
				{
					for(int i=0; i<4; i++)
					{
						log_menu_3.log_3_mem[i] = dir_memoria[i];
					}
					log_menu_3.data_3[0] = data_to_write[0];
					log_menu_3.data_3[1] = data_to_write[1];
					log_menu_3.data_3[2] = data_to_write[2];
					log_menu_3.data_3[3] = data_to_write[3];
					log_menu_3.data_3[4] = data_to_write[4];
					log_menu_3.data_3[5] = data_to_write[5];
					initial_log = 3;
					break;
				}
				case 3:
				{
					for(int i=0; i<4; i++)
					{
						log_menu_3.log_4_mem[i] = dir_memoria[i];
					}
					log_menu_3.data_4[0] = data_to_write[0];
					log_menu_3.data_4[1] = data_to_write[1];
					log_menu_3.data_4[2] = data_to_write[2];
					log_menu_3.data_4[3] = data_to_write[3];
					log_menu_3.data_4[4] = data_to_write[4];
					log_menu_3.data_4[5] = data_to_write[5];
					initial_log = 4;
					break;
				}
				case 4:
				{
					for(int i=0; i<4; i++)
					{
						log_menu_3.log_5_mem[i] = dir_memoria[i];
					}
					log_menu_3.data_5[0] = data_to_write[0];
					log_menu_3.data_5[1] = data_to_write[1];
					log_menu_3.data_5[2] = data_to_write[2];
					log_menu_3.data_5[3] = data_to_write[3];
					log_menu_3.data_5[4] = data_to_write[4];
					log_menu_3.data_5[5] = data_to_write[5];
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

				log_menu_3.data_1[i] = log_menu_3.data_2[i];
				log_menu_3.data_2[i] = log_menu_3.data_3[i];
				log_menu_3.data_3[i] = log_menu_3.data_4[i];
				log_menu_3.data_4[i] = log_menu_3.data_5[i];
				log_menu_3.data_5[i] = data_to_write[i];
			}

			log_menu_3.data_1[4] = log_menu_3.data_2[4];
			log_menu_3.data_2[4] = log_menu_3.data_3[4];
			log_menu_3.data_3[4] = log_menu_3.data_4[4];
			log_menu_3.data_4[4] = log_menu_3.data_5[4];
			log_menu_3.data_5[4] = data_to_write[4];
			break;
		}
		default: break;
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

						store_log();
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
		default: main_menu();
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
		data_input_manager(data);
		rtos_uart_send(rtos_uart1, &data, 1);
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

	main_menu();
	vTaskSuspend(NULL);
}

void main_menu()
{
	actual_menu = MENU_MAIN_MENU;
	console_clear();
	rtos_uart_send(rtos_uart0, &main_menu_t, sizeof(main_menu_t));
	rtos_uart_send(rtos_uart1, &main_menu_t, sizeof(main_menu_t));
}

int main (void)
{

	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();
	BOARD_InitDebugConsole();

	xTaskCreate(uart_echo_task, "uart_echo_task", 110, NULL, 1, NULL);
	xTaskCreate(uart_echo_task_bt, "uart_echo_task_bt", 110, NULL, 1, NULL);
	xTaskCreate(system_start, "Inicializa el programa", configMINIMAL_STACK_SIZE+100, NULL, SYSTEM_START_PRIORITY, NULL);

	vTaskStartScheduler();

    while(1) {
    }
    return 0 ;

}
