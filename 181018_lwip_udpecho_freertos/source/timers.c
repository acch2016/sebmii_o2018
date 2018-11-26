/*
 * timers.c
 *
 *  Created on: Nov 19, 2018
 *      Author: acc
 */
#include "timers.h"

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

void FTM1_config(void)
{
	ftm_config_t ftmInfo;
	FTM_GetDefaultConfig(&ftmInfo);
	/* Divide FTM clock by 4 */
	ftmInfo.prescale = kFTM_Prescale_Divide_4;
    /* Initialize FTM module */
    FTM_Init(FTM1, &ftmInfo);
    /* Set timer period. */
    FTM_SetTimerPeriod(FTM1, USEC_TO_COUNT(50U, (CLOCK_GetFreq(kCLOCK_BusClk)/4)));

    FTM_EnableInterrupts(FTM1, kFTM_TimeOverflowInterruptEnable);

    EnableIRQ(FTM1_IRQn);
	NVIC_SetPriority(FTM1_IRQn,5);
    FTM_StartTimer(FTM1, kFTM_SystemClock);

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



