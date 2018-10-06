/*
 * rtos_i2c_rtc.h
 *
 *  Created on: Oct 6, 2018
 *      Author: acc
 */

#ifndef RTOS_I2C_RTC_H_
#define RTOS_I2C_RTC_H_

#include <stdint.h>


typedef enum {rtos_i2c_rtc_sucess,rtos_i2c_rtc_fail} rtos_i2c_rtc_flag_t;


void rtos_i2c_rtc_st(void);
uint8_t rtos_i2c_rtc_read_hour(void);
void rtos_i2c_rtc_set_hour(void);
void delay(uint16_t delay);

#endif /* RTOS_I2C_RTC_H_ */
