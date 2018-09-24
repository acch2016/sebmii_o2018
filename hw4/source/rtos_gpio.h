/*
 * rtos_gpio.h
 *
 *  Created on: Sep 22, 2018
 *      Author: acc
 */

#ifndef RTOS_GPIO_H_
#define RTOS_GPIO_H_

typedef enum {rtos_gpioA,rtos_gpioB,rtos_gpioC,rtos_gpioD,rtos_gpioE} rtos_gpio_t;
typedef enum {rtos_gpio_portA,rtos_gpio_portB,rtos_gpio_portC,rtos_gpio_portD,rtos_gpio_portE} rtos_gpio_port_t;
typedef enum {rtos_gpio_sucess,rtos_gpio_fail} rtos_gpio_flag_t;

typedef struct
{
//	uint32_t  baudrate;
	rtos_gpio_t gpio;
	rtos_gpio_port_t port;
	uint32_t pin; //pin user wants to read
//	uint8_t rx_pin;
//	uint8_t tx_pin;
	uint8_t pin_mux;
}rtos_gpio_config_t;

rtos_gpio_flag_t rtos_gpio_init(rtos_gpio_config_t config);
//TODO No estoy seguro que recibe esta funcion como argumentsos
rtos_uart_flag_t rtos_gpio_wait_port_and_pin(rtos_gpio_config_t config, rtos_gpio_config_t config);

#endif /* RTOS_GPIO_H_ */
