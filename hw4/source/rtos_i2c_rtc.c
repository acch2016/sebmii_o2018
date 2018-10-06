/*
 * rtos_i2c_rtc.c
 *
 *  Created on: Oct 6, 2018
 *      Author: acc
 */


#include "rtos_i2c_rtc.h"

#include "rtos_i2c.h"

#include "fsl_clock.h"
#include "fsl_port.h"

#include "FreeRTOS.h"
#include "semphr.h"




void rtos_i2c_rtc_st(rtos_i2c_number_t number)
{
	uint8_t data_buffer = 0x80;
	rtos_i2c_master_transf_config_t mXfer_config;
	mXfer_config.slaveAddress = 0x6F;
	mXfer_config.direction = rtos_i2c_write;
	mXfer_config.subaddress = 0x00;
	mXfer_config.subaddressSize = 1;
	mXfer_config.data = &data_buffer;
	mXfer_config.dataSize = 1;

	rtos_i2c_master_transfer(number, mXfer_config);
//	delay(3000);
}

rtos_rtc_time rtos_i2c_rtc_read_time(rtos_i2c_number_t number)
{
	rtos_rtc_time buffer_t;
//	uint8_t buffer;
	rtos_i2c_master_transf_config_t mXfer_config;
	mXfer_config.slaveAddress = 0x6F;
	mXfer_config.direction = rtos_i2c_read;
	mXfer_config.subaddress = 0x00;
	mXfer_config.subaddressSize = 1;
	mXfer_config.data = &buffer_t.second;
	mXfer_config.dataSize = 1;

	rtos_i2c_master_transfer(number, mXfer_config);

	mXfer_config.subaddress = 0x01;
	mXfer_config.data = &buffer_t.minute;
	rtos_i2c_master_transfer(number, mXfer_config);

	mXfer_config.subaddress = 0x02;
	mXfer_config.data = &buffer_t.hour;
	rtos_i2c_master_transfer(number, mXfer_config);

	return buffer_t;
}

void rtos_i2c_rtc_set_hour(rtos_i2c_number_t number, rtos_rtc_time time)
{
	rtos_i2c_master_transf_config_t mXfer_config;
	mXfer_config.slaveAddress = 0x6F;
	mXfer_config.direction = rtos_i2c_write;
	mXfer_config.subaddress = 0x00;
	mXfer_config.subaddressSize = 1;
	mXfer_config.data = &time.second;
	mXfer_config.dataSize = 1;

	rtos_i2c_master_transfer(number, mXfer_config);

	mXfer_config.subaddress = 0x01;
	mXfer_config.data = &time.minute;
	rtos_i2c_master_transfer(number, mXfer_config);

	mXfer_config.subaddress = 0x02;
	mXfer_config.data = &time.hour;
	rtos_i2c_master_transfer(number, mXfer_config);
}

//void delay(uint16_t delay)
//{
//	volatile int counter, counter2;
//	for(counter2 = 16; counter2 > 0; counter2--)
//	{
//		for(counter = delay; counter > 0; counter--);
//	}
//}
