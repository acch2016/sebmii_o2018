/*
 * rtos_gpio.c
 *
 *  Created on: Sep 22, 2018
 *      Author: acc
 */

#include "rtos_gpio.h"

#include "fsl_gpio.h"
#include "fsl_clock.h"
#include "fsl_port.h"

#include "FreeRTOS.h"
#include "semphr.h"
#define NUMBER_OF_PORTS (2)

static inline void enable_port_clock(rtos_gpio_port_t);
static inline UART_Type * get_gpio_base(rtos_gpio_number_t);
static inline PORT_Type * get_port_base(rtos_gpio_port_t);

typedef struct
{
	uint8_t is_init;
//	gpio_handle_t fsl_gpio_handle;
	SemaphoreHandle_t mutex_rx;
	SemaphoreHandle_t mutex_tx;
	SemaphoreHandle_t rx_sem;
	SemaphoreHandle_t tx_sem;
}rtos_gpio_hanlde_t;

static rtos_gpio_hanlde_t gpio_handles[NUMBER_OF_PORTS] = {0};

void PORTA_IRQHandler(void)
{

}
void PORTB_IRQHandler(void)
{

}
void PORTC_IRQHandler(void)
{

}
void PORTD_IRQHandler(void)
{

}
void PORTE_IRQHandler(void)
{

}
