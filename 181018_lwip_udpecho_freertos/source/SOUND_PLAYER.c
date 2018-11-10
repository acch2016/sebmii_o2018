/*
 * SOUND_PLAYER.c
 *
 *  Created on: Nov 8, 2018
 *      Author: acc
 */


#include "SOUND_PLAYER.h"
#include "udpecho.h"
extern SemaphoreHandle_t synchroTaskSemaphore;


void PITconfig()
{
	pit_config_t pit_config;
	PIT_GetDefaultConfig(&pit_config);
	//CLOCK_EnableClock(kCLOCK_Pit0);
	//MCR
	PIT_Init(PIT, &pit_config);
	//    PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, CLOCK_GetBusClkFreq()*(1.5));
	/* Set timer period for channel 0 */
	PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, USEC_TO_COUNT(50U, CLOCK_GetFreq(kCLOCK_BusClk)));//como son 100 valores, le toma mas tiempo y por lo tanto la frecuencia es dos ceros mas abajo
	//    PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, CLOCK_GetBusClkFreq());
	PIT_GetStatusFlags(PIT, kPIT_Chnl_0);
	PIT_EnableInterrupts(PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);
	NVIC_EnableIRQ(PIT0_IRQn);
	NVIC_SetPriority(PIT0_IRQn,5);
	PIT_StartTimer(PIT, kPIT_Chnl_0);

}

void DAC_config(void)
{
	dac_config_t dacConfigStruct;

	/* Configure the DAC. */
	/*
	 * dacConfigStruct.referenceVoltageSource = kDAC_ReferenceVoltageSourceVref2;
	 * dacConfigStruct.enableLowPowerMode = false;
	 */
	DAC_GetDefaultConfig(&dacConfigStruct);
	DAC_Init(DEMO_DAC_BASEADDR, &dacConfigStruct);
	//        NVIC_SetPriority(DAC0_IRQn);
	DAC_Enable(DEMO_DAC_BASEADDR, true); /* Enable output. */

}
void LED_config(void)
{
	CLOCK_EnableClock(kCLOCK_PortB);//B
	port_pin_config_t config_led =
	{
			kPORT_PullDisable, kPORT_SlowSlewRate, kPORT_PassiveFilterDisable,
			kPORT_OpenDrainDisable, kPORT_LowDriveStrength, kPORT_MuxAsGpio,
			kPORT_UnlockRegister,
	};
	PORT_SetPinConfig(PORTB, 21, &config_led);    //B
	gpio_pin_config_t led_config_gpio = { kGPIO_DigitalOutput, 1 };
	GPIO_PinInit(GPIOB, 21, &led_config_gpio);//B
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
	GPIO_PinInit(GPIOE, 26, &pin_config_gpio);
	GPIO_PinInit(GPIOC, 4, &pin_config_gpio);

}

static void audio_player(void*arg)
{
	//	event = xEventGroupCreate();

	/* This handler that is originally shared in this module is now pointing to the binarySemaphore created */
	/* the creation of the semaphore happens only one single time*/
	PITconfig();
	DAC_config();
//	LED_config();
//	PIN_config();

	uint16_t *GlobalBufferPtr;
	//	struct netbuf *buf;

	uint8_t played_B = 0;

	while (1)
	{
		xSemaphoreTake(pitToogleSemaphore,portMAX_DELAY);
		//GPIO_TogglePinsOutput(GPIOA, 1 << 1);

		static uint8_t i_FILL_ping_PLAY_pong = 0;
		static uint8_t i_FILL_pong_PLAY_ping = 0;



		if( 1 == played_B)
		{
			if (PINGPONGSIZE == i_FILL_ping_PLAY_pong)
			{
//				DAC_SetBufferValue(DAC0, 0U, pongBuffer[2048]);
				xSemaphoreGive(synchroTaskSemaphore);
				GlobalBufferPtr = AudioPlayer_getBuffer();
				i_FILL_ping_PLAY_pong = 0;
				played_B = 0;

			}
			pingBuffer[i_FILL_ping_PLAY_pong] = GlobalBufferPtr[i_FILL_ping_PLAY_pong];
			DAC_SetBufferValue(DAC0, 0U, pongBuffer[i_FILL_ping_PLAY_pong]);
			i_FILL_ping_PLAY_pong++;
		}
		else
		{
			if (PINGPONGSIZE == i_FILL_pong_PLAY_ping)
			{
//				DAC_SetBufferValue(DAC0, 0U, pingBuffer[2048]);
				xSemaphoreGive(synchroTaskSemaphore);
				GlobalBufferPtr = AudioPlayer_getBuffer();
				i_FILL_pong_PLAY_ping = 0;
				played_B = 1;
			}
			pongBuffer[i_FILL_pong_PLAY_ping] = GlobalBufferPtr[i_FILL_pong_PLAY_ping];
			DAC_SetBufferValue(DAC0, 0U, pingBuffer[i_FILL_pong_PLAY_ping]);
			i_FILL_pong_PLAY_ping++;
		}
	}

}


void audio_player_init(void)
{
	pitToogleSemaphore = xSemaphoreCreateBinary();
	xTaskCreate(audio_player, "audio_player", configMINIMAL_STACK_SIZE+700, NULL, configMAX_PRIORITIES-1, NULL);
}



uint8_t counter = 0;
void PIT0_IRQHandler()
{
	BaseType_t xHigherPriorityTaskWoken;
	PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
	xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(pitToogleSemaphore, &xHigherPriorityTaskWoken );
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
