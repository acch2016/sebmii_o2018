/*
 * Copyright 2016-2018 NXP Semiconductor, Inc.
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
 * @file    0910_gpio_example.c
 * @brief   Application entry point.
 */
//#include <stdio.h>
//#include "board.h"
//#include "peripherals.h"
//#include "pin_mux.h"
//#include "clock_config.h"
//#include "MK64F12.h"
//#include "fsl_debug_console.h"
/* TODO: insert other include files here. */

/* TODO: insert other definitions and declarations here. */
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "fsl_port.h"
#include "fsl_uart.h"


uart_handle_t g_uartHandle;
uart_config_t user_config;
uart_transfer_t sendXfer;
uart_transfer_t receiveXfer;
volatile bool txFinished;
volatile bool rxFinished;
uint8_t sendData[] = "hello";
uint8_t receiveData[32];



void PORTC_IRQHandler()
{
	PORT_ClearPinsInterruptFlags( PORTC, 1 << 6);
	GPIO_TogglePinsOutput(GPIOB, 1<<22);
}

int main(void)
{
	volatile uint64_t delay = 0;

	/* Init board hardware. */
	BOARD_InitPins();
	BOARD_BootClockRUN();
	BOARD_InitDebugConsole();

	port_pin_config_t port_output_config =
	{ kPORT_PullDisable, kPORT_FastSlewRate, kPORT_PassiveFilterDisable,
			kPORT_OpenDrainDisable, kPORT_LowDriveStrength, kPORT_MuxAsGpio,
			kPORT_UnlockRegister, };

	port_pin_config_t port_input_config =
	{ kPORT_PullDisable, kPORT_FastSlewRate, kPORT_PassiveFilterDisable,
			kPORT_OpenDrainDisable, kPORT_LowDriveStrength, kPORT_MuxAsGpio,
			kPORT_UnlockRegister, };

	gpio_pin_config_t gpio_output_config =
	{ kGPIO_DigitalOutput, 1, };

	gpio_pin_config_t gpio_input_config =
	{ kGPIO_DigitalInput, 1, };

	CLOCK_EnableClock(kCLOCK_PortB);
	CLOCK_EnableClock(kCLOCK_PortC);

	/*For leds*/
	PORT_SetPinConfig(PORTB, 22, &port_output_config);


	/*For SW2*/
	PORT_SetPinConfig(PORTC, 6, &port_input_config);
	PORT_SetPinInterruptConfig(PORTC, 6, kPORT_InterruptFallingEdge);

	NVIC_EnableIRQ(PORTC_IRQn);
	NVIC_SetPriority(PORTC_IRQn, 5);

	/*for leds*/
	GPIO_PinInit(GPIOB, 22, &gpio_output_config);

	/*for SW2*/
	GPIO_PinInit(GPIOC, 6, &gpio_input_config);

	for (;;)
	{
	}
}
