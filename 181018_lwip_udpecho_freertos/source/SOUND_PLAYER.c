/*
 * SOUND_PLAYER.c
 *
 *  Created on: 14/06/2018
 *      Author: Cursos
 */
#include <audio_receiver.h>
#include "SOUND_PLAYER.h"
#include "menu.h"

uint32_t cnt;
uint32_t loop = 2U;
uint32_t secondLoop = 1000U;
volatile uint32_t milisecondCounts = 0U;

/*
uint64_t* SoundPlayer_getDataLost()
{
	return data_lost;
}
*/


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

void FTM1_config(void)
{
	ftm_config_t ftmInfo;
	FTM_GetDefaultConfig(&ftmInfo);
	/* Divide FTM clock by 4 */
	ftmInfo.prescale = kFTM_Prescale_Divide_4;
    /* Initialize FTM module */
    FTM_Init(FTM1, &ftmInfo);
    /* Set timer period. */
    FTM_SetTimerPeriod(FTM1, USEC_TO_COUNT(1000U, (CLOCK_GetFreq(kCLOCK_BusClk)/4)));

    FTM_EnableInterrupts(FTM1, kFTM_TimeOverflowInterruptEnable);

    EnableIRQ(FTM1_IRQn);
	NVIC_SetPriority(FTM1_IRQn,5);
    FTM_StartTimer(FTM1, kFTM_SystemClock);

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
	PORT_SetPinConfig(PORTA, 2, &config_pin);

	gpio_pin_config_t pin_config_gpio = { kGPIO_DigitalOutput, 1 };
	GPIO_PinInit(GPIOA, 1, &pin_config_gpio);
	GPIO_PinInit(GPIOE, 26, &pin_config_gpio);
	GPIO_PinInit(GPIOC, 4, &pin_config_gpio);
	GPIO_PinInit(GPIOA, 2, &pin_config_gpio);

}

static void audio_player(void*arg)
{
	//	event = xEventGroupCreate();

	/* This handler that is originally shared in this module is now pointing to the binarySemaphore created */
	/* the creation of the semaphore happens only one single time*/
	PITconfig();
	DAC_config();
//	LED_config();
	PIN_config();

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
			if(1 == audio_toggle)
			{
				DAC_SetBufferValue(DAC0, 0U, pongBuffer[i_FILL_ping_PLAY_pong]);
			}
			else
			{
				DAC_SetBufferValue(DAC0, 0U, 0);
			}
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
			if(1 == audio_toggle)
			{
				DAC_SetBufferValue(DAC0, 0U, pingBuffer[i_FILL_pong_PLAY_ping]);
			}
			else
			{
				DAC_SetBufferValue(DAC0, 0U, 0);
			}
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

void FTM1_IRQHandler()
{
	BaseType_t xHigherPriorityTaskWoken;
	FTM_ClearStatusFlags(FTM1, kFTM_TimeOverflowFlag);
	xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(stats_FTM_Semaphore, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}


static void stats_task(void * arg)
{
	FTM1_config();
	uint64_t recv_data_counter = 0;
	uint64_t recv_data_counter_new = 0;
	uint64_t recv_data_counter_old = 0;
	uint64_t data_lost = 0;
	uint64_t calidad_comunicacion = 0;

	while(1)
	{
		xSemaphoreTake(stats_FTM_Semaphore,portMAX_DELAY);
		milisecondCounts++;
		if(milisecondCounts >= secondLoop)
		{
			recv_data_counter_new = SoundPlayer_getCounter();
			recv_data_counter = recv_data_counter_new - recv_data_counter_old;
			recv_data_counter_old = recv_data_counter_new;

			//GPIO_TogglePinsOutput(GPIOA, 1 << 2);
			cnt++;
			if(cnt >= loop)
			{
				cnt = 0;
			}
			milisecondCounts = 0U;
		}
		//recibir 100 paquetes en un sec

		//data_lost es la cantidad de paquetes perdidos, recv_data_counter es la cantidad de paquetes recibidos y recv_data_counter es la calidad de comunicación (es un porcentaje).
		data_lost = 100 - recv_data_counter;
	}
}

void estadisticas_init(void)
{
	stats_FTM_Semaphore = xSemaphoreCreateBinary();
	xTaskCreate(stats_task, "stats_task", configMINIMAL_STACK_SIZE+700, NULL, configMAX_PRIORITIES-4, NULL);
}

