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
//#define I2C_APP_ACCELEROMETER
//#define I2C_APP_RTC

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
#ifdef I2C_APP_ACCELEROMETER

#include "rtos_i2c.h"

void i2c_testAPP_task(void * args)
{
	/** I2C init **/
	rtos_i2c_config_t accel_config;
	accel_config.baudrate = 100000;
	accel_config.i2c_number = rtos_i2c0;
	accel_config.i2c_port = rtos_i2c_portE;
	accel_config.pin_config_struct.mux = kPORT_MuxAlt5;
	accel_config.scl_pin = 24;
	accel_config.sda_pin = 25;
	rtos_i2c_init(accel_config);

	//** ST **//
	uint8_t data_buffer = 0x01;
	rtos_i2c_master_transf_config_t mXfer_config;
	mXfer_config.slaveAddress = 0x1D;
	mXfer_config.direction = kI2C_Write;
	mXfer_config.subaddress = 0x2A;
	mXfer_config.subaddressSize = 1;
	mXfer_config.data = &data_buffer;
	mXfer_config.dataSize = 1;
	mXfer_config.flags = kI2C_TransferDefaultFlag;
	rtos_i2c_master_transfer(rtos_i2c0, mXfer_config);

	uint8_t buffer[6];
	int16_t accelerometer[3];
	float new0 = 0, new1 = 0, new2 = 0;

	while (1)
	{
		mXfer_config.slaveAddress = 0x1D;
		mXfer_config.direction = kI2C_Read;
		mXfer_config.subaddress = 0x01;
		mXfer_config.subaddressSize = 1;
		mXfer_config.data = buffer;
		mXfer_config.dataSize = 6;
		mXfer_config.flags = kI2C_TransferDefaultFlag;
		rtos_i2c_master_transfer(rtos_i2c0, mXfer_config);

		accelerometer[0] = buffer[0] << 8 | buffer[1];
		accelerometer[1] = buffer[2] << 8 | buffer[3];
		accelerometer[2] = buffer[4] << 8 | buffer[5];

//		new0 = (accelerometer[0] * (0.000244)) / 4;
//		new1 = (accelerometer[1] * (0.000244)) / 4;
//		new2 = (accelerometer[2] * (0.000244)) / 4;

		PRINTF("x: %11d y: %11d z: %11d\n\r",accelerometer[0], accelerometer[1],accelerometer[2]);
//		PRINTF("x: %11d y: %11d z: %11d\n\r",new0,new1,new2);
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
	xTaskCreate(i2c_testAPP_task, "i2c_testAPP_task", configMINIMAL_STACK_SIZE+110, NULL, configMAX_PRIORITIES, NULL);
	vTaskStartScheduler();
	for(;;);
	return 0;
}

#endif
#ifdef I2C_APP_RTC

#include "rtos_i2c.h"
#include "rtos_i2c_rtc.h"

void i2c_rtc_testAPP_task(void * args)
{
	rtos_rtc_time buffer_t;
//	uint8_t buffer;

	/** I2C init **/
	rtos_i2c_config_t rtc_config;
	rtc_config.baudrate = 100000;
	rtc_config.i2c_number = rtos_i2c0;
	rtc_config.i2c_port = rtos_i2c_portB;
	rtc_config.mux = kPORT_MuxAlt2;
	rtc_config.scl_pin = 2;
	rtc_config.sda_pin = 3;
	rtos_i2c_init(rtc_config);

	/** ST **/
	rtos_i2c_rtc_st(rtos_i2c0);

	rtos_rtc_time time;
	time.hour = 0x06;
	time.minute = 0xC9;
	time.second = 0xE9;

	rtos_i2c_rtc_set_hour(rtos_i2c0,time);

	while (1)
	{

		buffer_t = rtos_i2c_rtc_read_time(rtos_i2c0);
		PRINTF("%2X : %2X : %2X\n\r", buffer_t.hour, buffer_t.minute, buffer_t.second);
//		PRINTF("%2d : %2d : %2d\n\n\r", buffer_t.second, buffer_t.minute, buffer_t.hour);
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
	xTaskCreate(i2c_rtc_testAPP_task, "i2c_rtc_testAPP_task", configMINIMAL_STACK_SIZE+110, NULL, configMAX_PRIORITIES, NULL);
	vTaskStartScheduler();
	for(;;);
	return 0;
}

#endif
