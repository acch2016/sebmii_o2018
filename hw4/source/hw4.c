/*
 * Copyright (c) 2017, NXP Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of NXP Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file    uart_rtos.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "fsl_debug_console.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fsl_port.h"

//#define UART_APP
#define GPIO_APP

#ifdef UART_APP

#include "rtos_uart.h"

void uart_echo_task(void * args)
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
		rtos_uart_send(rtos_uart0, &data, 1);
	}
}

int main(void)
{

	BOARD_BootClockRUN();

	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();
	BOARD_InitDebugConsole();
PRINTF("hola_mundooo\n");
	xTaskCreate(uart_echo_task, "uart_echo_task", 110, NULL, 1, NULL);
	vTaskStartScheduler();


	for(;;);
	return 0;
}

#endif
#ifdef GPIO_APP

#include "rtos_gpio.h"

void gpio_tooglePin_task(void * args)
{
//	rtos_gpio_config_t config_LED;
	rtos_gpio_config_t config_SW2;

	port_pin_config_t port_output_config =
	{ kPORT_PullDisable, kPORT_FastSlewRate, kPORT_PassiveFilterDisable,
			kPORT_OpenDrainDisable, kPORT_LowDriveStrength, kPORT_MuxAsGpio,
			kPORT_UnlockRegister, };

	gpio_pin_config_t gpio_output_config =
	{ kGPIO_DigitalOutput, 1, };

	CLOCK_EnableClock(kCLOCK_PortB);

	PORT_SetPinConfig(PORTB, 22, &port_output_config);

	GPIO_PinInit(GPIOB, 22, &gpio_output_config);

//	config_LED.gpio = rtos_gpioB;
//	config_LED.port = rtos_gpio_portB;
//	config_LED.pin = 22;
	config_SW2.gpio = rtos_gpioC;
	config_SW2.port = rtos_gpio_portC;
	config_SW2.pin = 6;
//	rtos_gpio_init(config_LED);
	rtos_gpio_init(config_SW2);

	for(;;)
	{
		rtos_gpio_wait_pin(config_SW2);
//		el siguiente uso de recurso no tendria que ser rodeado por mutex ya que solo es un led el cuals seria encendido?
		GPIO_TogglePinsOutput(GPIOB, 1 << 22); //LED
	}
}


int main(void)
{

	BOARD_BootClockRUN();

	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();
	BOARD_InitDebugConsole();
PRINTF("hola_mundooo\n");
	xTaskCreate(gpio_tooglePin_task, "gpio_tooglePin_task", 110, NULL, 1, NULL);
	vTaskStartScheduler();


	for(;;);
	return 0;
}


#endif

