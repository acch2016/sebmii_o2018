/*
 * SOUND_PLAYER.c
 *
 *  Created on: 14/06/2018
 *      Author: Cursos
 */
#include "SOUND_PLAYER.h"
#include "udpecho.h"

void FTM_config(void)
{
	ftm_config_t ftmInfo;
	FTM_GetDefaultConfig(&ftmInfo);
	/* Divide FTM clock by 4 */
	ftmInfo.prescale = kFTM_Prescale_Divide_4;
    /* Initialize FTM module */
    FTM_Init(FTM0, &ftmInfo);
    /* Set timer period. */
    FTM_SetTimerPeriod(FTM0, USEC_TO_COUNT(50U, (CLOCK_GetFreq(kCLOCK_BusClk)/4)));

    FTM_EnableInterrupts(FTM0, kFTM_TimeOverflowInterruptEnable);

    EnableIRQ(FTM0_IRQn);
	NVIC_SetPriority(FTM0_IRQn,5);
    FTM_StartTimer(FTM0, kFTM_SystemClock);

}

void PIT_config(void)
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
//	PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, CLOCK_GetBusClkFreq());
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
//	NVIC_SetPriority(DAC0_IRQn,5);
	DAC_Enable(DEMO_DAC_BASEADDR, true); /* Enable output. */
}


static void audio_player(void * arg)
{
	//	event = xEventGroupCreate();

	/* This handler that is originally shared in this module is now pointing to the binarySemaphore created */
	/* the creation of the semaphore happens only one single time*/
	FTM_config();
//	PIT_config();
	DAC_config();
//	LED_config();
	PIN_config();

	uint16_t *GlobalBufferPtr;
	//	struct netbuf *buf;

	uint8_t played_B = 0;

	while (1)
	{
		GPIO_WritePinOutput(GPIOE, 26, 1);    //G OFF
		xSemaphoreTake(pitToogleSemaphore,portMAX_DELAY);
		GPIO_WritePinOutput(GPIOE, 26, 0);    //G ON
		GPIO_TogglePinsOutput(GPIOA, 1 << 1);

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

//		TODO
//		VTaskDelay(portMAX_DELAY);
//		vTaskSuspend(NULL);
	}

}


void audio_player_init(void)
{
	pitToogleSemaphore = xSemaphoreCreateBinary();
	xTaskCreate(audio_player, "audio_player", configMINIMAL_STACK_SIZE+700, NULL, configMAX_PRIORITIES-1, NULL);
}



//uint8_t counter = 0;
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

void FTM0_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken;
	GPIO_TogglePinsOutput(GPIOC, 1 << 3); // PTC3
    /* Clear interrupt flag.*/
    FTM_ClearStatusFlags(FTM0, kFTM_TimeOverflowFlag);
	xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(pitToogleSemaphore, &xHigherPriorityTaskWoken );
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
