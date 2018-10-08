/*
 * rtos_i2c.c
 *
 *  Created on: 23/09/2018
 *      Author: Ulises Tejeda
 *
 */
#include "rtos_memi2c.h"

#include "fsl_i2c.h"

#include "fsl_clock.h"
#include "fsl_port.h"

#include "FreeRTOS.h"
#include "semphr.h"


//#include "event_groups.h"
//#include "queue.h"

//#define GET_ARGS(args,type) *((type*)args)
//#define EVENT_READ (1<<0)
//#define EVENT_WRITE (1<<1)

//typedef enum {read_id,write_id,supervisor_id} id_t;
//
//typedef struct
//{
//	EventGroupHandle_t supervisor_signals;
//	QueueHandle_t mailbox;
//}task_args_t;
//
//typedef struct
//{
//	id_t id;
//	uint32_t data;
//	const uint8_t * msg;
//}msg_t;

/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/*!
	\brief
		This function is used to read data from a memory address.
	\param[in]
		the i2c number
	\param[in]
		address is the address desired to read
	\param[in]
		rBuff_size is the amount of bytes to be read
	\return
		The pointer to the buffer in which data will is stored.
*/

//rtos_memi2c_flag_t memi2c_read(rtos_i2c_number_t i2c_number, uint16_t address, uint32_t * txBuff, uint32_t BUFFER_SIZE, void*args)
uint8_t* memi2c_read(rtos_i2c_number_t i2c_number, uint16_t address, uint8_t rBuff_size)
{
//	task_args_t task_args = GET_ARGS(args,task_args_t);
//	rtos_memi2c_flag_t retval = rtos_memi2c_fail;
//	msg_t msg;
//	msg.id = read_id;
	rtos_i2c_master_transf_config_t mXfer_config_read;
	mXfer_config_read.slaveAddress = 0x50;
	mXfer_config_read.direction = rtos_i2c_read;
	mXfer_config_read.subaddress = address;
	mXfer_config_read.subaddressSize = 2;
	mXfer_config_read.data = rBuff;
	mXfer_config_read.dataSize = rBuff_size;
//	for(;;){
//		rtos_i2c_read(i2c_number, address, txBuff, BUFFER_SIZE); //Los datos leidos están en txBuff
	rtos_i2c_master_transfer(i2c_number, mXfer_config_read);
//		msg.msg = txBuff;
//		xQueueSend(task_args.mailbox,&msg,portMAX_DELAY);
//		xEventGroupSetBits(task_args.supervisor_signals, EVENT_READ);
//	}
	return rBuff;
}

/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/*!
	\brief
		This function is used to write data to the memory from the specified address.
	\param[in]
		i2c_number i2c0, 12c1 or 12c2
	\param[in]
		address is the address desired to write
	\param[in]
		wBuff is the buffer containing the data to be written to the memory
	\param[in]
		wBuff_size is the amount of bytes to be written.
	\return
		VOID
*/

//rtos_memi2c_flag_t memi2c_write(rtos_i2c_number_t i2c_number, uint32_t address, uint8_t * txBuff, uint32_t BUFFER_SIZE, void*args)
void memi2c_write(rtos_i2c_number_t i2c_number, uint16_t address, uint8_t * wBuff, uint8_t wBuff_size)
{
//	task_args_t task_args = GET_ARGS(args,task_args_t);
//	rtos_memi2c_flag_t retval = rtos_memi2c_fail;
//	msg_t msg;
//	msg.id = write_id;
	rtos_i2c_master_transf_config_t mXfer_config_write;
	mXfer_config_write.slaveAddress = 0x50;
	mXfer_config_write.direction = rtos_i2c_write;
	mXfer_config_write.subaddress = address;
	mXfer_config_write.subaddressSize = 2;
	mXfer_config_write.data = wBuff;
	mXfer_config_write.dataSize = wBuff_size;
//	for(;;){
//		rtos_i2c_write(i2c_number, address, txBuff, BUFFER_SIZE); //Los datos escritos van en txBuff
	rtos_i2c_master_transfer(i2c_number, mXfer_config_write);
//		xQueueSend(task_args.mailbox,&msg,portMAX_DELAY);
//		xEventGroupSetBits(task_args.supervisor_signals, EVENT_WRITE);
//	}
}

//void memi2c_supervisor(void*args)
//{
//	task_args_t task_args = GET_ARGS(args,task_args_t);
//	uint8_t supervise_count = 0;
//	msg_t received_msg;
//
//	for(;;)
//	{
//		xEventGroupWaitBits(task_args.supervisor_signals, EVENT_READ|EVENT_WRITE, pdTRUE, pdTRUE, portMAX_DELAY);
//
//		xQueueReceive(task_args.mailbox,&received_msg,portMAX_DELAY);
//
//		supervise_count++;
//	}
//}

