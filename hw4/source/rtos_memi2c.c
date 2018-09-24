/*
 * rtos_i2c.c
 *
 *  Created on: 23/09/2018
 *      Author: Ulises Tejeda
 *
 */


#include "fsl_clock.h"
#include "fsl_i2c.h"
#include "fsl_port.h"
#include "rtos_memi2c.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "event_groups.h"
#include "queue.h"

#define GET_ARGS(args,type) *((type*)args)
#define EVENT_READ (1<<0)
#define EVENT_WRITE (1<<1)

typedef enum {read_id,write_id,supervisor_id} id_t;

typedef struct
{
	EventGroupHandle_t supervisor_signals;
	QueueHandle_t mailbox;
}task_args_t;

typedef struct
{
	id_t id;
	uint32_t data;
	const uint8_t * msg;
}msg_t;

rtos_memi2c_flag_t memi2c_read(rtos_i2c_number_t i2c_number, uint32_t address, uint8_t * txBuff, uint32_t BUFFER_SIZE, void*args)
{
	task_args_t task_args = GET_ARGS(args,task_args_t);
	rtos_memi2c_flag_t retval = rtos_memi2c_fail;
	msg_t msg;
	msg.id = read_id;

	for(;;){
		rtos_i2c_read(i2c_number, address, txBuff, BUFFER_SIZE); //Los datos leidos están en txBuff

		msg.msg = txBuff;

		xQueueSend(task_args.mailbox,&msg,portMAX_DELAY);

		xEventGroupSetBits(task_args.supervisor_signals, EVENT_READ);
	}

	return retval;
}

rtos_memi2c_flag_t memi2c_write(rtos_i2c_number_t i2c_number, uint32_t address, uint8_t * txBuff, uint32_t BUFFER_SIZE, void*args)
{
	task_args_t task_args = GET_ARGS(args,task_args_t);
	rtos_memi2c_flag_t retval = rtos_memi2c_fail;
	msg_t msg;
	msg.id = write_id;

	for(;;){
		rtos_i2c_write(i2c_number, address, txBuff, BUFFER_SIZE); //Los datos escritos van en txBuff

		xQueueSend(task_args.mailbox,&msg,portMAX_DELAY);

		xEventGroupSetBits(task_args.supervisor_signals, EVENT_WRITE);
	}

	return retval;
}

void memi2c_supervisor(void*args)
{
	task_args_t task_args = GET_ARGS(args,task_args_t);
	uint8_t supervise_count = 0;
	msg_t received_msg;

	for(;;)
	{
		xEventGroupWaitBits(task_args.supervisor_signals, EVENT_READ|EVENT_WRITE, pdTRUE, pdTRUE, portMAX_DELAY);

		xQueueReceive(task_args.mailbox,&received_msg,portMAX_DELAY);

		supervise_count++;
	}
}

