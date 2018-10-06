/*
 * rtos_uart.h
 *
 *  Created on: 22/09/2018
 *      Author: Ulises Tejeda / Alejandro Canale
 */

#ifndef RTOS_I2C_H_
#define RTOS_I2C_H_



#include <stdint.h>

typedef enum {rtos_i2c0,rtos_i2c1, rtos_i2c2} rtos_i2c_number_t;
typedef enum {rtos_i2c_portA,rtos_i2c_portB,rtos_i2c_portC,rtos_i2c_portD,rtos_i2c_portE} rtos_i2c_port_t;
typedef enum {rtos_i2c_sucess,rtos_i2c_fail} rtos_i2c_flag_t;
typedef enum {rtos_i2c_write=0, rtos_i2c_read} rtos_i2c_data_direction_t;
typedef struct
{
	uint32_t  baudrate;//100000
	rtos_i2c_number_t i2c_number;//rtos_i2c0
	rtos_i2c_port_t i2c_port;//B
//	uint8_t pin_mux;//kPORT_MuxAlt2 -> comentar #define PORT_SETPINCONFIG

/**	Es toda la estuctura port_pin_config_t dentro de la cual hay que acceder al miembro mux**/
	uint16_t mux;
	uint8_t scl_pin;//2
	uint8_t sda_pin;//3
}rtos_i2c_config_t;

typedef struct
{
	uint8_t slaveAddress;
	rtos_i2c_data_direction_t direction; // kI2C_Write / kI2C_Read
	uint32_t subaddress;
	uint8_t subaddressSize;
	uint8_t *volatile data;
	volatile size_t dataSize;
}rtos_i2c_master_transf_config_t;

rtos_i2c_flag_t rtos_i2c_init(rtos_i2c_config_t config);
rtos_i2c_flag_t rtos_i2c_master_transfer(rtos_i2c_number_t i2c_number, rtos_i2c_master_transf_config_t masterXfer_config);





#endif /* RTOS_I2C_H_ */
