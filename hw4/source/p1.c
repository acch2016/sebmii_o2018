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
//#define MENU
#ifdef MENU
uint8_t old_data;
uint8_t actual_menu;

void main_menu();

void console_clear()
{
	PRINTF("\e[1;1H\e[2J");
}

void menu_error()
{
	console_clear();
	PRINTF("\r*****ERROR*****\n");
	PRINTF("\rIngresa una opcion correcta (1,2,3,4) del menu anterior\n");
	PRINTF("\rPulsa cualquier tecla para continuar: \n");
}

void menu_1()
{
	console_clear();
	PRINTF("\rSeleccionaste la opcion 1:\n");
	PRINTF("\rDirección (Hexa): 0x\n");
}

void data_input_manager(uint8_t data)
{
	switch(actual_menu)
	{
		case MENU_MAIN_MENU:
		{
			if(0xd == data)
			{
				if(0x31 != old_data || 0x32 != old_data || 0x33 != old_data || 0x34 != old_data)
				{
					menu_error();
				}
				else
				{
					switch(old_data)
					{
						case 0x31: menu_1();
						default: menu_error();
					}
				}
			}
			else
			{
				old_data = data;
			}
		}
		case MENU_MAIN_MENU_ERROR:
		{
			main_menu();
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
	main_menu();
	vTaskSuspend(NULL);
}

void main_menu()
{
	actual_menu = MENU_MAIN_MENU;
	console_clear();
	PRINTF("\r----------MENU PRINCIPAL----------\n");
	PRINTF("\rControl del modulo de memoria compartida:\n");
	PRINTF("\r1) Escribir en direccion.\n");
	PRINTF("\r2) Leer de direccion\n");
	PRINTF("\r3) Historial\n");
	PRINTF("\r4) Establecer hora\n");
	PRINTF("\rSelecciona una opcion del menu superior:  ");
}

int main (void)
{

	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();
	BOARD_InitDebugConsole();

	xTaskCreate(uart_echo_task, "uart_echo_task", 110, NULL, 1, NULL);
	xTaskCreate(system_start, "Inicializa el programa", configMINIMAL_STACK_SIZE, NULL, SYSTEM_START_PRIORITY, NULL);

	vTaskStartScheduler();

    while(1) {
    }
    return 0 ;

}
#endif
