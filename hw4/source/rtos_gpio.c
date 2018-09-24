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

/*TODO en este modulo no se necesita lo siguiente?*/
//#define NUMBER_OF_PORTS (5)

static inline void nvic_enable_irq_nvic_set_priority(rtos_gpio_port_t);
static inline void enable_port_clock(rtos_gpio_port_t);
static inline UART_Type * get_gpio_base(rtos_gpio_t);
static inline PORT_Type * get_port_base(rtos_gpio_port_t);


typedef struct
{
	uint8_t is_init;
//	gpio_handle_t fsl_gpio_handle;
//	SemaphoreHandle_t mutex_rx;
//	SemaphoreHandle_t mutex_tx;
//	SemaphoreHandle_t rx_sem;
//	SemaphoreHandle_t tx_sem;
}rtos_gpio_hanlde_t;

uint32_t pin;

//static rtos_gpio_hanlde_t gpio_handles[NUMBER_OF_PORTS] = {0};

void PORTA_IRQHandler(void)
{
//	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	pin = GPIO_PortGetInterruptFlags(GPIOA);
	PORT_ClearPinsInterruptFlags( PORTA, pin);
	/*TODO xSemaphoreGiveFromISR( bin_semaphore, &xHigherPriorityTaskWoken );*/

//	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
void PORTB_IRQHandler(void)
{
//	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	pin = GPIO_PortGetInterruptFlags(GPIOB);
	PORT_ClearPinsInterruptFlags( PORTB, pin);

//	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
void PORTC_IRQHandler(void)
{
//	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	pin = GPIO_PortGetInterruptFlags(GPIOC);
	PORT_ClearPinsInterruptFlags( PORTC, pin);

//	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
void PORTD_IRQHandler(void)
{
//	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	pin = GPIO_PortGetInterruptFlags(GPIOD);
	PORT_ClearPinsInterruptFlags( PORTD, pin);

//	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
void PORTE_IRQHandler(void)
{
//	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	pin = GPIO_PortGetInterruptFlags(GPIOE);
	PORT_ClearPinsInterruptFlags( PORTE, pin);

//	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

rtos_gpio_flag_t rtos_gpio_init(rtos_gpio_config_t config)
{
	rtos_gpio_flag_t retval = rtos_gpio_fail;

	port_pin_config_t port_input_config =
	{ kPORT_PullDisable, kPORT_FastSlewRate, kPORT_PassiveFilterDisable,
			kPORT_OpenDrainDisable, kPORT_LowDriveStrength, kPORT_MuxAsGpio,
			kPORT_UnlockRegister, };

	gpio_pin_config_t gpio_input_config =
	{ kGPIO_DigitalInput, 1, };

	PORT_SetPinConfig(config.port, config.pin, &port_input_config);

	PORT_SetPinInterruptConfig(config.port, config.pin, kPORT_InterruptEitherEdge);

	nvic_enable_irq_nvic_set_priority(config.port);

	GPIO_PinInit(config.gpio, config.pin, &gpio_input_config);

	return retval;
}

static inline void nvic_enable_irq_nvic_set_priority(rtos_gpio_port_t port)
{
	switch(port)
	{
	case rtos_gpio_portA:
		NVIC_EnableIRQ(PORTA_IRQn);
		NVIC_SetPriority(PORTA_IRQn, 5);
		break;
	case rtos_gpio_portB:
		NVIC_EnableIRQ(PORTB_IRQn);
		NVIC_SetPriority(PORTB_IRQn, 5);
		break;
	case rtos_gpio_portC:
		NVIC_EnableIRQ(PORTC_IRQn);
		NVIC_SetPriority(PORTC_IRQn, 5);
		break;
	case rtos_gpio_portD:
		NVIC_EnableIRQ(PORTD_IRQn);
		NVIC_SetPriority(PORTD_IRQn, 5);
		break;
	case rtos_gpio_portE:
		NVIC_EnableIRQ(PORTE_IRQn);
		NVIC_SetPriority(PORTE_IRQn, 5);
		break;
	}
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

static inline GPIO_Type * get_gpio_base(rtos_gpio_t gpio)
{
	GPIO_Type * gpio_base = GPIOA;
	switch(gpio)
	{
	case rtos_gpioA:
		gpio_base = GPIOA;
		break;
	case rtos_gpioB:
		gpio_base = GPIOB;
		break;
	case rtos_gpioC:
		gpio_base = GPIOC;
		break;
	case rtos_gpioD:
		gpio_base = GPIOD;
		break;
	case rtos_gpioE:
		gpio_base = GPIOE;
		break;
	}
	return gpio_base;
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
