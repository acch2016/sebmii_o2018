/*
 * SOUND_PLAYER.h
 *
 *  Created on: 14/06/2018
 *      Author: Cursos
 */

#ifndef SOUND_PLAYER_H_
#define SOUND_PLAYER_H_



#include <timers_.h>
#include "MK64F12.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"
#include "udpecho.h"
#include "netbuf.h"

#include "board.h"
//#include "peripherals.h"
#include "clock_config.h"

#include "fsl_debug_console.h"
#include "fsl_dac.h"
#include "fsl_pit.h"//TODO
#include "fsl_ftm.h"//TODO
#include "testpins.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_DAC_BASEADDR DAC0

#define PINGPONGSIZE 200
#define EVENT_BIT (1<<0)
//#define EVENT_BIT_ (1<<0)

/*******************************************************************************
* Variables
******************************************************************************/
uint16_t pingBuffer[PINGPONGSIZE];
uint16_t pongBuffer[PINGPONGSIZE];
EventGroupHandle_t event;
SemaphoreHandle_t pitToogleSemaphore;
//uint8_t valores[] = { 127, 135, 143, 151, 159, 167, 174, 182, 189, 196, 202,
//		209, 215, 220, 226, 230, 235, 239, 243, 246, 248, 250, 252, 253,
//		254, 254, 254, 253, 251, 249, 247, 244, 241, 237, 233, 228, 223,
//		218, 212, 206, 199, 192, 185, 178, 170, 163, 155, 147, 139, 131,
//		123, 115, 107, 99, 91, 84, 76, 69, 62, 55, 48, 42, 36, 31, 26, 21,
//		17, 13, 10, 7, 5, 3, 1, 0, 0, 0, 1, 2, 4, 6, 8, 11, 15, 19, 24, 28,
// 34, 39, 45, 52, 58, 65, 72, 80, 87, 95, 103, 111, 119, 127 };

/*******************************************************************************
* Prototypes
******************************************************************************/
void FTM_config(void);//TODO
void PIT_config(void);//TODO
void DAC_config(void);

static void audio_player(void *arg);


#endif /* SOUND_PLAYER_H_ */
