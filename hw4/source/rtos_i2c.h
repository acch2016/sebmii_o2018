/*
 * rtos_uart.h
 *
 *  Created on: 22/09/2018
 *      Author: Ulises Tejeda
 */

#include <stdint.h>

typedef enum {rtos_i2c0,rtos_i2c1, rtos_i2c2} rtos_i2c_number_t;
typedef enum {rtos_i2c_portA,rtos_i2c_portB,rtos_i2c_portC,rtos_i2c_portD,rtos_i2c_portE} rtos_i2c_port_t;
typedef enum {rtos_i2c_sucess,rtos_i2c_fail} rtos_i2c_flag_t;

typedef struct
{
	uint32_t  baudrate;
	rtos_i2c_number_t i2c_number;
	rtos_i2c_port_t i2c_port;
	uint8_t rx_pin;
	uint8_t tx_pin;
	uint8_t pin_mux;
	uint8_t scl_pin;
	uint8_t sda_pin;
}rtos_i2c_config_t;
