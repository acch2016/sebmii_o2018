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
char data_to_save[100];
char dir_memoria[4] = {NO_ASIGNADO,NO_ASIGNADO,NO_ASIGNADO,NO_ASIGNADO};
char *data_to_write;
uint8_t wr_chars = 0;

/*TEXTOS DE LOS MENÚS*/
char main_menu_t[] = "\r+++---MAIN MENU---+++\n\n\rControl del modulo de memoria compartida:\n\r1) Escribir en direccion.\n\r2) Leer de direccion\n\r3) Historial\n\r4) Establecer hora\n\rSelecciona una opcion del menu superior:  \n\r";
char error_menu_t[] = "\r*****ERROR*****\n\n\rIngresa una opcion correcta (1,2,3,4) del menu anterior\n\rPulsa cualquier tecla para continuar: \n\r";
char menu_1_t[] = "\rSeleccionaste la opcion 1:\n\rDireccion (Hexa) y presiona Enter: 0x\n\r";
char menu_1_1_t[] = "\n\rAhora, ingresa lo que desees escribir y presiona Enter: \n\r";
char menu_1_2_t[] = "\n\rSe ha guardado correctamente el siguiente elemento: \n\r";
char menu_1_3_t[] = "\n\n\rPresiona cualquier tecla para volver al menu: \n\r";
char menu_2_t[] = "\rSeleccionaste la opcion 2:\n\rIngresa direccion (Hexa) y presiona Enter: 0x\r";
char menu_3_t[] = "\rMostrando el historial reciente...\n\r";
char menu_4_t[] = "\rIngrese la hora (hh:mm:ss):\n\r";
char menu_admin_mode[] = "\n\r~~~~~MODO DE ADMINISTRADOR ACTIVADO~~~~~\n\n\r";


void main_menu();

void console_clear()
{
	PRINTF("\e[1;1H\e[2J");
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
}

void menu_4()
{
	actual_menu = MENU_4;
	console_clear();
	rtos_uart_send(rtos_uart0, &menu_4_t, sizeof(menu_4_t));
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


						rtos_uart_send(rtos_uart0, &menu_1_2_t, sizeof(menu_1_2_t));
						rtos_uart_send(rtos_uart0, data_to_write, strlen(data_to_write));
						rtos_uart_send(rtos_uart0, &menu_1_3_t, sizeof(menu_1_3_t));
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
	rtos_uart_config_t config;
	config.baudrate = 115200;
	config.rx_pin = 16;
	config.tx_pin = 17;
	config.pin_mux = kPORT_MuxAlt3;
	config.uart_number = rtos_uart0;
	config.port = rtos_uart_portB;
	rtos_uart_init(config);
	uint8_t data;
	for(;;)
	{
		rtos_uart_receive(rtos_uart0, &data, 1);
		data_input_manager(data);
		rtos_uart_send(rtos_uart0, &data, 1);
	}
}

void system_start()
{
	data_to_write = malloc(sizeof(char));
	rtos_uart_config_t config;
	config.baudrate = 115200;
	config.rx_pin = 16;
	config.tx_pin = 17;
	config.pin_mux = kPORT_MuxAlt3;
	config.uart_number = rtos_uart0;
	config.port = rtos_uart_portB;
	rtos_uart_init(config);
	main_menu();
	vTaskSuspend(NULL);
}

void main_menu()
{
	actual_menu = MENU_MAIN_MENU;
	console_clear();
	rtos_uart_send(rtos_uart0, &main_menu_t, sizeof(main_menu_t));
}

int main (void)
{

	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();
	BOARD_InitDebugConsole();

	xTaskCreate(uart_echo_task, "uart_echo_task", 110, NULL, 1, NULL);
	xTaskCreate(system_start, "Inicializa el programa", configMINIMAL_STACK_SIZE+100, NULL, SYSTEM_START_PRIORITY, NULL);

	vTaskStartScheduler();

    while(1) {
    }
    return 0 ;

}
