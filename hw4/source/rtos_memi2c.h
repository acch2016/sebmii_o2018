/*
 * rtos_uart.h
 *
 *  Created on: 23/09/2018
 *      Author: Ulises Tejeda
 */

#include "rtos_i2c.h"

typedef enum {rtos_memi2c_sucess,rtos_memi2c_fail} rtos_memi2c_flag_t;

rtos_memi2c_flag_t memi2c_read(rtos_i2c_number_t i2c_number, uint32_t address, uint8_t * txBuff, uint32_t BUFFER_SIZE, void*args);
rtos_memi2c_flag_t memi2c_write(rtos_i2c_number_t i2c_number, uint32_t address, uint8_t * txBuff, uint32_t BUFFER_SIZE, void*args);