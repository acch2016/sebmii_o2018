/*
 * stats.c
 *
 *  Created on: Nov 19, 2018
 *      Author: acc
 */
#include "stats.h"

//static void stats_task(void * arg)
//{
//	FTM1_config();
//
//	while(1)
//	{
//		xSemaphoreTake(stats_FTM_Semaphore,portMAX_DELAY);
//		SoundPlayer_getCounter();
//
//		//recibir 100 paquetes en un sec
//
//	}
//
//}
//
//void stats_init(void)
//{
//	stats_FTM_Semaphore = xSemaphoreCreateBinary();
//	xTaskCreate(audio_player, "stats_task", configMINIMAL_STACK_SIZE+700, NULL, configMAX_PRIORITIES-4, NULL);
//}
