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
#include "event_groups.h"

/** TODO Se necesitan 5 estructuras tipo rtos_gpio_hanlde_t para que haya una por GPIOx **/
#define NUMBER_OF_GPIOS (5)

#define EVENT_PORTA (1<<0)
#define EVENT_PORTB (1<<1)
#define EVENT_PORTC	(1<<2)
#define EVENT_PORTD (1<<3)
#define EVENT_PORTE (1<<4)

#define GET_ARGS(args,type) *((type*)args)

typedef struct
{
	uint8_t is_init;
	uint32_t interruptPin;
//	gpio_handle_t fsl_gpio_handle;
	EventGroupHandle_t events;
	SemaphoreHandle_t mutex;
//	SemaphoreHandle_t mutex_rx;
//	SemaphoreHandle_t mutex_tx;
//	SemaphoreHandle_t binary_sem;
//	SemaphoreHandle_t tx_sem;
}rtos_gpio_hanlde_t;

static rtos_gpio_hanlde_t gpio_handles[NUMBER_OF_GPIOS] = {0};

static inline void nvic_enable_irq_nvic_set_priority(rtos_gpio_port_t);
static inline void enable_port_clock(rtos_gpio_port_t);
static inline GPIO_Type * get_gpio_base(rtos_gpio_t);
static inline PORT_Type * get_port_base(rtos_gpio_port_t);
rtos_gpio_hanlde_t get_rtos_gpio_handle(rtos_gpio_config_t config);

void PORTA_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	gpio_handles[rtos_gpio_portA].interruptPin = GPIO_PortGetInterruptFlags(GPIOA);
	PORT_ClearPinsInterruptFlags( PORTA, gpio_handles[rtos_gpio_portA].interruptPin);

	xEventGroupSetBitsFromISR(gpio_handles[rtos_gpio_portA].events, EVENT_PORTA, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
void PORTB_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	gpio_handles[rtos_gpio_portB].interruptPin = GPIO_PortGetInterruptFlags(GPIOB);
	PORT_ClearPinsInterruptFlags( PORTB, gpio_handles[rtos_gpio_portB].interruptPin);

	xEventGroupSetBitsFromISR(gpio_handles[rtos_gpio_portB].events, EVENT_PORTB, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
void PORTC_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	gpio_handles[rtos_gpio_portC].interruptPin = GPIO_PortGetInterruptFlags(GPIOC);
	PORT_ClearPinsInterruptFlags( PORTC, gpio_handles[rtos_gpio_portC].interruptPin);

	xEventGroupSetBitsFromISR(gpio_handles[rtos_gpio_portC].events, EVENT_PORTC, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
void PORTD_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	gpio_handles[rtos_gpio_portD].interruptPin = GPIO_PortGetInterruptFlags(GPIOD);
	PORT_ClearPinsInterruptFlags( PORTD, gpio_handles[rtos_gpio_portD].interruptPin);

	xEventGroupSetBitsFromISR(gpio_handles[rtos_gpio_portD].events, EVENT_PORTD, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
void PORTE_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	gpio_handles[rtos_gpio_portE].interruptPin = GPIO_PortGetInterruptFlags(GPIOE);
	PORT_ClearPinsInterruptFlags( PORTE, gpio_handles[rtos_gpio_portE].interruptPin);

	xEventGroupSetBitsFromISR(gpio_handles[rtos_gpio_portE].events, EVENT_PORTE, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

rtos_gpio_flag_t rtos_gpio_init(rtos_gpio_config_t config)
{
	rtos_gpio_flag_t retval = rtos_gpio_fail;

	if(config.gpio < NUMBER_OF_GPIOS)
	{
		if(!gpio_handles[config.gpio].is_init)
		{
			//TODO en lugar de este binario sera utilizado un evento
//			gpio_handles[config.gpio].binary_sem = xSemaphoreCreateBinary();
			gpio_handles[config.gpio].events =  xEventGroupCreate();


			port_pin_config_t port_input_config = { kPORT_PullDisable,
					kPORT_FastSlewRate, kPORT_PassiveFilterDisable,
					kPORT_OpenDrainDisable, kPORT_LowDriveStrength,
					kPORT_MuxAsGpio, kPORT_UnlockRegister, };

			gpio_pin_config_t gpio_input_config = { kGPIO_DigitalInput, 1, };

			enable_port_clock(config.port);

			PORT_SetPinConfig(get_port_base(config.port), config.pin, &port_input_config);

			PORT_SetPinInterruptConfig(get_port_base(config.port), config.pin,
					kPORT_InterruptEitherEdge);

			nvic_enable_irq_nvic_set_priority(config.port);

//			TODO En el rtos_uart.c Aldana hace un if para seleccionar uart0 o 1.
//			En este caso habria que ver si esto interfiere cuando se necesite inicializar varios pines
			GPIO_PinInit(get_gpio_base(config.gpio), config.pin, &gpio_input_config);


			gpio_handles[config.gpio].is_init = 1;
			retval = rtos_gpio_sucess;
		}
	}
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

/**TODO Este getter recibirá toda la estructura rtos_gpio_config_t para de ahi tomar el GPIOx
 *  con el cual sabra cual estructura mandar **/
//rtos_gpio_hanlde_t get_rtos_gpio_handle(rtos_gpio_config_t config)
//{
//	return gpio_handles[config.gpio];
//}

rtos_gpio_flag_t rtos_gpio_wait_pin(rtos_gpio_config_t config)
{
	rtos_gpio_flag_t flag = rtos_gpio_fail;
	if(gpio_handles[config.gpio].is_init)
	{
//	rtos_gpio_hanlde_t args = GET_ARGS(arg,rtos_gpio_hanlde_t);
//	rtos_gpio_hanlde_t gpio_handles[config.gpio] =

		xSemaphoreTake(gpio_handles[config.gpio].mutex, portMAX_DELAY);

		//TODO lo que sigue es la capa de aplicacion y deberia estar en el test
//	GPIO_TogglePinsOutput(get_gpio_base(config.gpio),1<<21);
		GPIO_TogglePinsOutput(GPIOB, 1 << 22); //LED

		xEventGroupWaitBits(gpio_handles[config.gpio].events,
		EVENT_PORTA | EVENT_PORTB | EVENT_PORTC | EVENT_PORTD | EVENT_PORTE,
		pdTRUE, pdFALSE, portMAX_DELAY);

//		quiza no es necesario pasar toda la estructura
//		xSemaphoreGive(gpio_handles[uart_number].mutex);
		xSemaphoreGive(gpio_handles[config.gpio].mutex);
	}
	return flag;

}
