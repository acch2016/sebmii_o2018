/*
 * testpins.c
 *
 *  Created on: Nov 18, 2018
 *      Author: acc
 */
#include "testpins.h"

void LED_config(void)
{
	CLOCK_EnableClock(kCLOCK_PortB);
	port_pin_config_t config_led =
	{
			kPORT_PullDisable, kPORT_SlowSlewRate, kPORT_PassiveFilterDisable,
			kPORT_OpenDrainDisable, kPORT_LowDriveStrength, kPORT_MuxAsGpio,
			kPORT_UnlockRegister,
	};
	PORT_SetPinConfig(PORTB, 21, &config_led);    //BLUE LED
	gpio_pin_config_t led_config_gpio = { kGPIO_DigitalOutput, 1 };
	GPIO_PinInit(GPIOB, 21, &led_config_gpio);
}
void PIN_config(void)
{
	CLOCK_EnableClock(kCLOCK_PortA); //toogle interrupcion PIT
	CLOCK_EnableClock(kCLOCK_PortE); //toogle muestra DAC
	CLOCK_EnableClock(kCLOCK_PortC); //toogle muestra DAC

	port_pin_config_t config_pin =
	{
			kPORT_PullUp, kPORT_SlowSlewRate, kPORT_PassiveFilterDisable,
			kPORT_OpenDrainDisable, kPORT_LowDriveStrength, kPORT_MuxAsGpio,
			kPORT_UnlockRegister,
	};
	PORT_SetPinConfig(PORTA, 1, &config_pin);
	PORT_SetPinConfig(PORTE, 26, &config_pin);
	PORT_SetPinConfig(PORTC, 4, &config_pin);

	gpio_pin_config_t pin_config_gpio = { kGPIO_DigitalOutput, 1 };
	GPIO_PinInit(GPIOA, 1, &pin_config_gpio);
	GPIO_PinInit(GPIOE, 26, &pin_config_gpio); //GREEN LED
	GPIO_PinInit(GPIOC, 4, &pin_config_gpio);

}
