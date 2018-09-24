/*
 * rtos_i2c.c
 *
 *  Created on: 22/09/2018
 *      Author: Ulises Tejeca
 *
 */

#include "fsl_clock.h"
#include "fsl_i2c.h"
#include "fsl_port.h"
#include "rtos_i2c.h"


#include "FreeRTOS.h"
#include "semphr.h"

#define NUMBER_OF_SERIAL_PORTS (2)
#define SUBADDRESS_SIZE 4U
#define SLAVE1 0x00


static inline void enable_port_clock(rtos_i2c_port_t port);
static inline PORT_Type * get_port_base(rtos_i2c_port_t);
static inline I2C_Type * get_i2c_base(rtos_i2c_number_t i2c_number);
static void i2c_master_callback(I2C_Type *base, i2c_master_handle_t *handle, status_t status, void *userData);

volatile bool g_MasterCompletionFlag = false;

typedef struct
{
	uint8_t is_init;
	i2c_master_handle_t fsl_i2c_handle;
	SemaphoreHandle_t mutex_tx;
	SemaphoreHandle_t mutex_rx;
	SemaphoreHandle_t rx_sem;
	SemaphoreHandle_t tx_sem;
}rtos_i2c_hanlde_t;

static rtos_i2c_hanlde_t i2c_handles[NUMBER_OF_SERIAL_PORTS] = {0};


static void i2c_master_callback(I2C_Type *base, i2c_master_handle_t *handle, status_t status, void *userData)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	if ( kStatus_I2C_Idle == status)
	{
		if(I2C0 == base)
		{
			xSemaphoreGiveFromISR(i2c_handles[rtos_i2c0].tx_sem, &xHigherPriorityTaskWoken);
			g_MasterCompletionFlag = true;
		}
		else if(I2C1 == base)
		{
			xSemaphoreGiveFromISR(i2c_handles[rtos_i2c1].tx_sem, &xHigherPriorityTaskWoken);
			g_MasterCompletionFlag = true;
		}
		else
		{
			xSemaphoreGiveFromISR(i2c_handles[rtos_i2c2].tx_sem, &xHigherPriorityTaskWoken);
			g_MasterCompletionFlag = true;
		}
	}

	if ( kStatus_I2C_Busy == status)
	{
		if(I2C0 == base)
		{
			xSemaphoreGiveFromISR(i2c_handles[rtos_i2c0].rx_sem, &xHigherPriorityTaskWoken);
			g_MasterCompletionFlag = false;
		}
		else if(I2C1 == base)
		{
			xSemaphoreGiveFromISR(i2c_handles[rtos_i2c1].rx_sem, &xHigherPriorityTaskWoken);
			g_MasterCompletionFlag = false;
		}
		else
		{
			xSemaphoreGiveFromISR(i2c_handles[rtos_i2c2].rx_sem, &xHigherPriorityTaskWoken);
			g_MasterCompletionFlag = false;
		}
	}
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}



rtos_i2c_flag_t rtos_i2c_init(rtos_i2c_config_t config)
{

	rtos_i2c_flag_t retval = rtos_i2c_fail;
	i2c_master_config_t masterConfig;

	if(config.i2c_number < NUMBER_OF_SERIAL_PORTS)
	{
		if(!i2c_handles[config.i2c_number].is_init)
		{
			i2c_handles[config.i2c_number].mutex_tx = xSemaphoreCreateMutex();
			i2c_handles[config.i2c_number].mutex_rx = xSemaphoreCreateMutex();

			i2c_handles[config.i2c_number].rx_sem = xSemaphoreCreateBinary();
			i2c_handles[config.i2c_number].tx_sem= xSemaphoreCreateBinary();

			enable_port_clock(config.i2c_port);
			PORT_SetPinMux(get_port_base(config.i2c_port), config.scl_pin, config.pin_mux);
			PORT_SetPinMux(get_port_base(config.i2c_port), config.sda_pin, config.pin_mux);

			I2C_MasterGetDefaultConfig(&masterConfig);
			masterConfig.baudRate_Bps =  config.baudrate;
			masterConfig.enableMaster = true;

			if(rtos_i2c0 == config.i2c_number)
			{
				/*Init I2C_0 master.*/
				I2C_MasterInit(get_i2c_base(rtos_i2c0), &masterConfig, CLOCK_GetFreq(I2C0_CLK_SRC));
				NVIC_SetPriority(I2C0_IRQn,5);
			}
			else if(rtos_i2c1 == config.i2c_number)
			{
				/*Init I2C_1 master.*/
				I2C_MasterInit(get_i2c_base(rtos_i2c1), &masterConfig, CLOCK_GetFreq(I2C1_CLK_SRC));
				NVIC_SetPriority(I2C1_IRQn,5);
			}
			else
			{
				/*Init I2C_2 master.*/
				I2C_MasterInit(get_i2c_base(rtos_i2c2), &masterConfig, CLOCK_GetFreq(I2C2_CLK_SRC));
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

rtos_i2c_flag_t rtos_i2c_write(rtos_i2c_number_t i2c_number, uint32_t address, uint8_t * txBuff, uint32_t BUFFER_SIZE)
{
	rtos_i2c_flag_t flag = rtos_i2c_fail;
	i2c_master_transfer_t masterXfer;

	if(i2c_handles[i2c_number].is_init)
	{
		masterXfer.slaveAddress = SLAVE1;
		masterXfer.direction = kI2C_Write;
		masterXfer.subaddress = address;
		masterXfer.subaddressSize = SUBADDRESS_SIZE;
		masterXfer.data = txBuff;
		masterXfer.dataSize = BUFFER_SIZE;
		masterXfer.flags = kI2C_TransferDefaultFlag;

		xSemaphoreTake(i2c_handles[i2c_number].mutex_tx, portMAX_DELAY);
		I2C_MasterTransferNonBlocking(get_i2c_base(i2c_number), &i2c_handles[i2c_number].fsl_i2c_handle, &masterXfer);

		while (!g_MasterCompletionFlag)
		{
		}

		g_MasterCompletionFlag = false;
		xSemaphoreTake(i2c_handles[i2c_number].tx_sem, portMAX_DELAY);

		xSemaphoreGive(i2c_handles[i2c_number].mutex_tx);
		flag = rtos_i2c_sucess;
	}

	return flag;
}


rtos_i2c_flag_t rtos_i2c_read(rtos_i2c_number_t i2c_number, uint32_t address, uint8_t* txBuff, uint32_t BUFFER_SIZE)
{
	rtos_i2c_flag_t flag = rtos_i2c_fail;
	i2c_master_transfer_t masterXfer;
	if(i2c_handles[i2c_number].is_init)
	{
		masterXfer.slaveAddress = SLAVE1;
		masterXfer.direction = kI2C_Read;
		masterXfer.subaddress = address;
		masterXfer.subaddressSize = SUBADDRESS_SIZE;
		masterXfer.data = txBuff;
		masterXfer.dataSize = BUFFER_SIZE;
		masterXfer.flags = kI2C_TransferDefaultFlag;

		xSemaphoreTake(i2c_handles[i2c_number].mutex_rx, portMAX_DELAY);
		I2C_MasterTransferNonBlocking(get_i2c_base(i2c_number), &i2c_handles[i2c_number].fsl_i2c_handle, &masterXfer);
		while (!g_MasterCompletionFlag)
				{
				}
				g_MasterCompletionFlag = false;
		xSemaphoreTake(i2c_handles[i2c_number].rx_sem, portMAX_DELAY);

		xSemaphoreGive(i2c_handles[i2c_number].mutex_rx);
		flag = rtos_i2c_sucess;
	}

	return flag;
}



static inline void enable_port_clock(rtos_i2c_port_t port)
{
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
	/*Hay que definir en qué puertos está I2C*/
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
