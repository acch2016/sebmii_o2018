/*
 * SOUND_PLAYER.c
 *
 *  Created on: 14/06/2018
 *      Author: Cursos
 */
#include "SOUND_PLAYER.h"
#include "udpecho.h"


void PITconfig()
{
	pit_config_t pit_config;
	PIT_GetDefaultConfig(&pit_config);
	//CLOCK_EnableClock(kCLOCK_Pit0);
	//MCR
	PIT_Init(PIT, &pit_config);
	//    PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, CLOCK_GetBusClkFreq()*(1.5));
//	TODO Fs
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

//				if (EVENT_BIT & xEventGroupGetBits(event)) {
//		netbuf_copy(buf, GlobalBuffer, N_SIZE);
//					for(uint8_t i_ping = 0; i_ping < PINGPONGSIZE; i_ping ++ )
//					{
//						pingBuffer[i_ping] = GlobalBufferPtr[i_ping];
//					}
//					flag_ping = 1;
//					xEventGroupClearBits(event, EVENT_BIT);
//				} else {
//		netbuf_copy(buf, pongBuffer, PINGPONGSIZE);
//					for(uint8_t i_pong = 0; i_pong < PINGPONGSIZE; i_pong ++ )
//					{
//						pongBuffer[i_pong] = GlobalBufferPtr[i_pong];
//					}
//					flag_pong = 1;
//					xEventGroupSetBits(event, EVENT_BIT);
//				}


		if( 1 == played_B)
		{
			if (PINGPONGSIZE == i_FILL_ping_PLAY_pong)
			{
				GlobalBufferPtr = AudioPlayer_getBuffer();
				i_FILL_ping_PLAY_pong = 0;
				played_B = 0;

			}
			pingBuffer[i_FILL_ping_PLAY_pong] = GlobalBufferPtr[i_FILL_ping_PLAY_pong];
			DAC_SetBufferValue(DAC0, 0U, pongBuffer[i_FILL_ping_PLAY_pong]);
//			if (0 == pongBuffer[i_FILL_ping_PLAY_pong])
//			{
////				GPIO_TogglePinsOutput(GPIOE, 1 << 26);//////////////////////////////////////////////////Toogle
//				GPIO_WritePinOutput(GPIOE, 26, 0);    //G ON
//			} else {
//				GPIO_WritePinOutput(GPIOE, 26, 1);    //G OFF
//			}
//			GPIO_TogglePinsOutput(GPIOC, 1 << 4);//////////////////////////////////////////////////Toogle
			i_FILL_ping_PLAY_pong++;
		}
		else
		{
			if (PINGPONGSIZE == i_FILL_pong_PLAY_ping)
			{
				GlobalBufferPtr = AudioPlayer_getBuffer();
				i_FILL_pong_PLAY_ping = 0;
				played_B = 1;
			}
			pongBuffer[i_FILL_pong_PLAY_ping] = GlobalBufferPtr[i_FILL_pong_PLAY_ping];
			DAC_SetBufferValue(DAC0, 0U, pingBuffer[i_FILL_pong_PLAY_ping]);
			i_FILL_pong_PLAY_ping++;
		}

		//		static uint8_t i_SinValue = 0;
		//		if (i_SinValue == 100)
		//		{
		//			i_SinValue = 0;
		//			GPIO_TogglePinsOutput(GPIOE, 1 << 26);
		//		}
		//
		//		DAC_SetBufferValue(DAC0, 0U, valores[i_SinValue]);
		//		i_SinValue ++;

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
	//	PIT_StopTimer(PIT, kPIT_Chnl_0);
	//	PIT_StartTimer(PIT, kPIT_Chnl_0);
	//		GPIO_TogglePinsOutput(GPIOB, 1 << 21);
	//	counter++;
	//	PRINTF("\r\n%d\r\n", counter);
	//en lugar de poner logica del DAC aqui, se puso un evento
	xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(pitToogleSemaphore, &xHigherPriorityTaskWoken );
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
