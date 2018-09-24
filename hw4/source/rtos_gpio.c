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
//static inline UART_Type * get_gpio_base(rtos_gpio_number_t);
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

uint32_t pin;

static rtos_gpio_hanlde_t gpio_handles[NUMBER_OF_PORTS] = {0};

void PORTA_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	pin = GPIO_PortGetInterruptFlags(GPIOA);
	PORT_ClearPinsInterruptFlags( PORTA, pin);

	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
void PORTB_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	pin = GPIO_PortGetInterruptFlags(GPIOB);
	PORT_ClearPinsInterruptFlags( PORTB, pin);

	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
void PORTC_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	pin = GPIO_PortGetInterruptFlags(GPIOC);
	PORT_ClearPinsInterruptFlags( PORTC, pin);

	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
void PORTD_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	pin = GPIO_PortGetInterruptFlags(GPIOD);
	PORT_ClearPinsInterruptFlags( PORTD, pin);

	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
void PORTE_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	pin = GPIO_PortGetInterruptFlags(GPIOE);
	PORT_ClearPinsInterruptFlags( PORTE, pin);

	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

static inline void enable_port_clock(rtos_gpio_port_t port)
{
	switch(port)
	{
	case rtos_gpio_portA:
		CLOCK_EnableClock(kCLOCK_PortA);
		break;
	case rtos_gpio_portB:
		CLOCK_EnableClock(kCLOCK_PortB);
		break;
	case rtos_gpio_portC:
		CLOCK_EnableClock(kCLOCK_PortC);
		break;
	case rtos_gpio_portD:
		CLOCK_EnableClock(kCLOCK_PortD);
		break;
	case rtos_gpio_portE:
		CLOCK_EnableClock(kCLOCK_PortE);
		break;
	}
}

static inline PORT_Type * get_port_base(rtos_gpio_port_t port)
{
	PORT_Type * port_base = PORTA;
	switch(port)
	{
	case rtos_gpio_portA:
		port_base = PORTA;
		break;
	case rtos_gpio_portB:
		port_base = PORTB;
		break;
	case rtos_gpio_portC:
		port_base = PORTC;
		break;
	case rtos_gpio_portD:
		port_base = PORTD;
		break;
	case rtos_gpio_portE:
		port_base = PORTE;
		break;
	}
	return port_base;
}
