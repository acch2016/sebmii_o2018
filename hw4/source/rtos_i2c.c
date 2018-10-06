/*
 * rtos_i2c.c
 *
 *  Created on: 22/09/2018
 *      Author: Ulises Tejeca / Alejandro Canale
 *
 *
 */

#include "rtos_i2c.h"

#include "fsl_i2c.h"
#include "fsl_clock.h"
#include "fsl_port.h"

#include "FreeRTOS.h"
#include "semphr.h"

#define NUMBER_OF_SERIAL_PORTS (3)

static inline void enable_port_clock(rtos_i2c_number_t, rtos_i2c_port_t);
static inline I2C_Type * get_i2c_base(rtos_i2c_number_t);
static inline PORT_Type * get_port_base(rtos_i2c_port_t);
static void i2c_master_callback(I2C_Type *base, i2c_master_handle_t *handle, status_t status, void *userData);

typedef struct
{
	uint8_t is_init;
	i2c_master_handle_t fsl_i2c_handle;
	SemaphoreHandle_t mutex;
	SemaphoreHandle_t binary_semaphore;
}rtos_i2c_hanlde_t;

static rtos_i2c_hanlde_t i2c_handles[NUMBER_OF_SERIAL_PORTS] = {0};


static void i2c_master_callback(I2C_Type *base, i2c_master_handle_t *handle, status_t status, void *userData)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//	if ( kStatus_I2C_Idle == status)
	if ( kStatus_Success == status)
	{
		if(I2C0 == base)
		{
			xSemaphoreGiveFromISR(i2c_handles[rtos_i2c0].binary_semaphore, &xHigherPriorityTaskWoken);
		}
		else if(I2C1 == base)
		{
			xSemaphoreGiveFromISR(i2c_handles[rtos_i2c1].binary_semaphore, &xHigherPriorityTaskWoken);
		}
		else
		{
			xSemaphoreGiveFromISR(i2c_handles[rtos_i2c2].binary_semaphore, &xHigherPriorityTaskWoken);
		}
	}
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


#define PORT_SETPINCONFIG

rtos_i2c_flag_t rtos_i2c_init(rtos_i2c_config_t config)
{

	rtos_i2c_flag_t retval = rtos_i2c_fail;
	i2c_master_config_t masterConfig;

	if(config.i2c_number < NUMBER_OF_SERIAL_PORTS)
	{
		if(!i2c_handles[config.i2c_number].is_init)
		{
			i2c_handles[config.i2c_number].mutex = xSemaphoreCreateMutex();
			i2c_handles[config.i2c_number].binary_semaphore = xSemaphoreCreateBinary();

			enable_port_clock(config.i2c_number, config.i2c_port);

#ifdef PORT_SETPINCONFIG
			port_pin_config_t config_i2c = { kPORT_PullDisable,
					kPORT_SlowSlewRate, kPORT_PassiveFilterDisable,
					kPORT_OpenDrainDisable, kPORT_LowDriveStrength,
					kPORT_MuxAlt2, kPORT_UnlockRegister, };

			config_i2c.mux = config.pin_config_struct.mux;

			PORT_SetPinConfig(get_port_base(config.i2c_port), config.scl_pin, &config_i2c);// en la capa app pin2 portB
			PORT_SetPinConfig(get_port_base(config.i2c_port), config.sda_pin, &config_i2c);// en la capa app pin3 portB
#else
			PORT_SetPinMux(get_port_base(config.i2c_port), config.scl_pin, config.pin_mux);
			PORT_SetPinMux(get_port_base(config.i2c_port), config.sda_pin, config.pin_mux);
#endif
			I2C_MasterGetDefaultConfig(&masterConfig);
			masterConfig.baudRate_Bps =  config.baudrate;
//			masterConfig.enableMaster = true;

			if(rtos_i2c0 == config.i2c_number)
			{
				I2C_MasterInit(get_i2c_base(rtos_i2c0), &masterConfig, CLOCK_GetFreq(I2C0_CLK_SRC));
				NVIC_EnableIRQ(I2C0_IRQn);
				NVIC_SetPriority(I2C0_IRQn,5);
			}
			else if(rtos_i2c1 == config.i2c_number)
			{
				I2C_MasterInit(get_i2c_base(rtos_i2c1), &masterConfig, CLOCK_GetFreq(I2C1_CLK_SRC));
				NVIC_EnableIRQ(I2C1_IRQn);
				NVIC_SetPriority(I2C1_IRQn,5);
			}
			else
			{
				I2C_MasterInit(get_i2c_base(rtos_i2c2), &masterConfig, CLOCK_GetFreq(I2C2_CLK_SRC));
				NVIC_EnableIRQ(I2C2_IRQn);
				NVIC_SetPriority(I2C2_IRQn,5);
			}
			I2C_MasterTransferCreateHandle(get_i2c_base(config.i2c_number),
					&i2c_handles[config.i2c_number].fsl_i2c_handle, i2c_master_callback, NULL);

			i2c_handles[config.i2c_number].is_init = 1;
			retval = rtos_i2c_sucess;
		}
	}

	return retval;

}


rtos_i2c_flag_t rtos_i2c_master_transfer(rtos_i2c_number_t i2c_number, rtos_i2c_master_transf_config_t masterXfer_config)
{
	rtos_i2c_flag_t flag = rtos_i2c_fail;
	i2c_master_transfer_t masterXfer;

	if(i2c_handles[i2c_number].is_init)
	{
		masterXfer.slaveAddress = masterXfer_config.slaveAddress;
		masterXfer.direction = masterXfer_config.direction;
		masterXfer.subaddress = masterXfer_config.subaddress;
		masterXfer.subaddressSize = masterXfer_config.subaddressSize;
		masterXfer.data = masterXfer_config.data;
		masterXfer.dataSize = masterXfer_config.dataSize;
		masterXfer.flags = masterXfer_config.flags;

		xSemaphoreTake(i2c_handles[i2c_number].mutex, portMAX_DELAY);
		I2C_MasterTransferNonBlocking(get_i2c_base(i2c_number), &i2c_handles[i2c_number].fsl_i2c_handle, &masterXfer);

		xSemaphoreTake(i2c_handles[i2c_number].binary_semaphore, portMAX_DELAY);

		xSemaphoreGive(i2c_handles[i2c_number].mutex);

		flag = rtos_i2c_sucess;
	}
	return flag;
}


static inline void enable_port_clock(rtos_i2c_number_t number, rtos_i2c_port_t port)
{
	switch (number)
	{
	case rtos_i2c0:
		CLOCK_EnableClock(kCLOCK_I2c0);
		break;
	case rtos_i2c1:
		CLOCK_EnableClock(kCLOCK_I2c1);
		break;
	case rtos_i2c2:
		CLOCK_EnableClock(kCLOCK_I2c2);
		break;
	}

	switch(port)
	{
	case rtos_i2c_portA:
		CLOCK_EnableClock(kCLOCK_PortA);
		break;
	case rtos_i2c_portB:
		CLOCK_EnableClock(kCLOCK_PortB);
		break;
	case rtos_i2c_portC:
		CLOCK_EnableClock(kCLOCK_PortC);
		break;
	case rtos_i2c_portD:
		CLOCK_EnableClock(kCLOCK_PortD);
		break;
	case rtos_i2c_portE:
		CLOCK_EnableClock(kCLOCK_PortE);
		break;
	}

}

static inline I2C_Type * get_i2c_base(rtos_i2c_number_t i2c_number)
{
	I2C_Type * retval = I2C0;
	switch(i2c_number)
	{
	case rtos_i2c0:
		retval = I2C0;
		break;
	case rtos_i2c1:
		retval = I2C1;
		break;
	case rtos_i2c2:
		retval = I2C2;
		break;
	}
	return retval;
}

static inline PORT_Type * get_port_base(rtos_i2c_port_t port)
{
	PORT_Type * port_base = PORTA;
	switch(port)
	{
	case rtos_i2c_portA:
		port_base = PORTA;
		break;
	case rtos_i2c_portB:
		port_base = PORTB;
		break;
	case rtos_i2c_portC:
		port_base = PORTC;
		break;
	case rtos_i2c_portD:
		port_base = PORTD;
		break;
	case rtos_i2c_portE:
		port_base = PORTE;
		break;
	}
	return port_base;
}
