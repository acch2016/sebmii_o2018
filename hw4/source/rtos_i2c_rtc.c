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




void rtos_i2c_rtc_st()
{
	uint8_t data_buffer = 0x80;
	rtos_i2c_master_transf_config_t mXfer_config;
	mXfer_config.slaveAddress = 0x6F;
	mXfer_config.direction = kI2C_Write;
	mXfer_config.subaddress = 0x00;
	mXfer_config.subaddressSize = 1;
	mXfer_config.data = &data_buffer;
	mXfer_config.dataSize = 1;
	mXfer_config.flags = kI2C_TransferDefaultFlag;
	rtos_i2c_master_transfer(rtos_i2c0, mXfer_config);
	delay(3000);
}

uint8_t rtos_i2c_rtc_read_hour()
{
	uint8_t buffer;
	rtos_i2c_master_transf_config_t mXfer_config;
	mXfer_config.slaveAddress = 0x6F;
	mXfer_config.direction = kI2C_Read;
	mXfer_config.subaddress = 0x00;
	mXfer_config.subaddressSize = 1;
	mXfer_config.data = &buffer;
	mXfer_config.dataSize = 1;
	mXfer_config.flags = kI2C_TransferDefaultFlag;
	rtos_i2c_master_transfer(rtos_i2c0, mXfer_config);
	return buffer;
}

void rtos_i2c_rtc_set_hour()
{

}

void delay(uint16_t delay)
{
	volatile int counter, counter2;
	for(counter2 = 16; counter2 > 0; counter2--)
	{
		for(counter = delay; counter > 0; counter--);
	}
}
