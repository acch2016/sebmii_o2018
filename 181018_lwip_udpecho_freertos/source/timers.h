/*
 * timers.h
 *
 *  Created on: Nov 19, 2018
 *      Author: acc
 */

#ifndef TIMERS_H_
#define TIMERS_H_

#include "MK64F12.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

#include "fsl_pit.h"
#include "fsl_ftm.h"

void FTM_config(void);
void PIT_config(void);

#endif /* TIMERS_H_ */
