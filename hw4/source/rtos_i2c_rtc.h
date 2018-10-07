/*
 * rtos_i2c_rtc.h
 *
 *  Created on: Oct 6, 2018
 *      Author: acc
 */

#ifndef RTOS_I2C_RTC_H_
#define RTOS_I2C_RTC_H_

#include <stdint.h>

#include "rtos_i2c.h"

//typedef enum {rtos_i2c_rtc_sucess,rtos_i2c_rtc_fail} rtos_i2c_rtc_flag_t;

typedef struct
{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
}rtos_rtc_time;

void rtos_i2c_rtc_st(rtos_i2c_number_t number);
rtos_rtc_time rtos_i2c_rtc_read_time(rtos_i2c_number_t number);
void rtos_i2c_rtc_set_hour(rtos_i2c_number_t number, rtos_rtc_time time);
//void delay(uint16_t delay);

#endif /* RTOS_I2C_RTC_H_ */
