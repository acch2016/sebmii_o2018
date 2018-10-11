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
//#define GPIO_APP
//#define I2C_APP_ACCELEROMETER
//#define I2C_APP_RTC
//#define I2C_APP_MEM

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
	uint8_t modo_administrador = 0;
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
//		el siguiente uso de recurso no tendria que ser rodeado por mutex ya que solo es un led el cuals seria encendido?
		modo_administrador = rtos_gpio_toogle_pin(config_LED, modo_administrador);
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
	xTaskCreate(gpio_tooglePin_task, "gpio_tooglePin_task", configMINIMAL_STACK_SIZE+110, NULL, 1, NULL);
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
	accel_config.mux = kPORT_MuxAlt5;
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
	rtos_i2c_master_transfer(rtos_i2c0, mXfer_config);

	uint8_t buffer[6];
	int16_t accelerometer[3];
//	float new0 = 0, new1 = 0, new2 = 0;

	while (1)
	{
		mXfer_config.slaveAddress = 0x1D;
		mXfer_config.direction = kI2C_Read;
		mXfer_config.subaddress = 0x01;
		mXfer_config.subaddressSize = 1;
		mXfer_config.data = buffer;
		mXfer_config.dataSize = 6;
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

#include "rtos_i2c_rtc.h"

void i2c_rtc_testAPP_task(void * args)
{
	rtos_rtc_time buffer_t;
//	uint8_t buffer;

	/** I2C init **/
	rtos_i2c_config_t i2c_config;
	i2c_config.baudrate = 100000;
	i2c_config.i2c_number = rtos_i2c0;
	i2c_config.i2c_port = rtos_i2c_portB;
	i2c_config.mux = kPORT_MuxAlt2;
	i2c_config.scl_pin = 2;
	i2c_config.sda_pin = 3;
	rtos_i2c_init(i2c_config);

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
	PRINTF("hola_mundooo\n\r");
	xTaskCreate(i2c_rtc_testAPP_task, "i2c_rtc_testAPP_task", configMINIMAL_STACK_SIZE+110, NULL, configMAX_PRIORITIES, NULL);
	vTaskStartScheduler();
	for(;;);
	return 0;
}

#endif
#ifdef I2C_APP_MEM

#include "rtos_memi2c.h"
#include "rtos_uart.h"

#define ADDRESS 0x0259

void i2c_mem_testAPP_task(void * args)
{
	//Configuración UART USB
	rtos_uart_config_t config1;
	config1.baudrate = 115200;
	config1.rx_pin = 16;
	config1.tx_pin = 17;
	config1.pin_mux = kPORT_MuxAlt3;
	config1.uart_number = rtos_uart0;
	config1.port = rtos_uart_portB;
	rtos_uart_init(config1);

	uint8_t w_buffer[64] = {};
	uint8_t* ptr_w_buffer = w_buffer;

	uint8_t r_buffer[64] = {};
	uint8_t* ptr_r_buffer = r_buffer;
//	PRINTF("sizeof(r_buffer) %d\n\r", sizeof(r_buffer));//64

	const unsigned char producer_msg[] = "1234\n\r";
//	PRINTF("sizeof(producer_msg) %d\n\r", sizeof(producer_msg));// characters + null character \0

	/** I2C init **/
	rtos_i2c_config_t i2c_config;
	i2c_config.baudrate = 100000;
	i2c_config.i2c_number = rtos_i2c0;
	i2c_config.i2c_port = rtos_i2c_portB;
	i2c_config.mux = kPORT_MuxAlt2;
	i2c_config.scl_pin = 2;
	i2c_config.sda_pin = 3;
	rtos_i2c_init(i2c_config);

	//** first read what's in the specified ADDRESS and print it using UART **//
	ptr_r_buffer = memi2c_read(rtos_i2c0, ADDRESS, sizeof(r_buffer));
	rtos_uart_send(rtos_uart0, ptr_r_buffer, strlen(ptr_r_buffer));

	//** assign the string to the uint8_t array **//
	ptr_w_buffer = (uint8_t*)producer_msg;

	//** now we're sure there's nothing, write on the specified ADDRESS**//
	memi2c_write(rtos_i2c0, ADDRESS, ptr_w_buffer, sizeof(w_buffer));

	//** we read and print again **//
	ptr_r_buffer = memi2c_read(rtos_i2c0, ADDRESS, sizeof(r_buffer));
	rtos_uart_send(rtos_uart0, ptr_r_buffer, strlen(ptr_r_buffer));

	while (1)
	{
		vTaskDelete(NULL);
	}
}

int main(void)
{
	BOARD_BootClockRUN();
	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();
	BOARD_InitDebugConsole();
	PRINTF("hola_mundooo\n\r");
	xTaskCreate(i2c_mem_testAPP_task, "i2c_mem_testAPP_task", configMINIMAL_STACK_SIZE+110, NULL, configMAX_PRIORITIES, NULL);
	vTaskStartScheduler();
	for(;;);
	return 0;
}

#endif
