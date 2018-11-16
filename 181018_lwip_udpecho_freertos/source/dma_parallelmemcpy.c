/*
 * dma_parallelmemcpy.c
 *file
 *  Created on: Nov 10, 2018
 *      Author: acc
 */
/* User callback function for EDMA transfer. */
#include "dma_parallelmemcpy.h"

#include "FreeRTOS.h"
#include "semphr.h"

#define EXAMPLE_DMA DMA0
#define EXAMPLE_DMAMUX DMAMUX0

//#define NUMBER_OF_CHANNELS (3)
//#define DMA_CHANNEL_0 0U

void DMA_Init();
void * DMA_Transfer( void *destAddr, const void *srcAddr, size_t len);
void EDMA_Callback(edma_handle_t *handle, void *param, bool transferDone, uint32_t tcds);

edma_handle_t g_EDMA_Handle;

typedef struct
{
	edma_handle_t fsl_dma_handle;
//	SemaphoreHandle_t mutex;
	SemaphoreHandle_t binary_semaphore;
}rtos_dma_hanlde_t;

static rtos_dma_hanlde_t dma_handle;

void EDMA_Callback(edma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if ( kEDMA_DoneFlag == transferDone)
	{
			xSemaphoreGiveFromISR(dma_handle.binary_semaphore, &xHigherPriorityTaskWoken);
	}
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}



void DMA_Init()
{
    edma_config_t userConfig;
    /* Configure DMAMUX */
    DMAMUX_Init(EXAMPLE_DMAMUX);
#if defined(FSL_FEATURE_DMAMUX_HAS_A_ON) && FSL_FEATURE_DMAMUX_HAS_A_ON
    DMAMUX_EnableAlwaysOn(EXAMPLE_DMAMUX, 0, true);
#else
    DMAMUX_SetSource(EXAMPLE_DMAMUX, 0, 63);
#endif /* FSL_FEATURE_DMAMUX_HAS_A_ON */
    DMAMUX_EnableChannel(EXAMPLE_DMAMUX, 0);
    /* Configure EDMA one shot transfer */
    /*
     * userConfig.enableRoundRobinArbitration = false;
     * userConfig.enableHaltOnError = true;
     * userConfig.enableContinuousLinkMode = false;
     * userConfig.enableDebugMode = false;
     */
    EDMA_GetDefaultConfig(&userConfig);
    EDMA_Init(EXAMPLE_DMA, &userConfig);
//    NVIC_EnableIRQ(DMA0_IRQn);
    NVIC_SetPriority(DMA0_IRQn,5);

    EDMA_CreateHandle(&g_EDMA_Handle, EXAMPLE_DMA, 0);

    dma_handle.binary_semaphore = xSemaphoreCreateBinary();
}



void * DMA_Transfer( void *destAddr, const void *srcAddr, size_t len)
{

    edma_transfer_config_t transferConfig;
    uint16_t len_element;
    EDMA_SetCallback(&g_EDMA_Handle, EDMA_Callback, NULL);
    EDMA_PrepareTransfer(&transferConfig, (void*)srcAddr, sizeof(srcAddr[0]), destAddr, sizeof(destAddr[0]),
    		sizeof(srcAddr[0]), len, kEDMA_MemoryToMemory);
    EDMA_SubmitTransfer(&g_EDMA_Handle, &transferConfig);
    EDMA_StartTransfer(&g_EDMA_Handle);
    /* Wait for EDMA transfer finish */
    /*while (g_Transfer_Done != true)
    {
    }*/



    return (void*)destAddr;
}

//rtos_dma_hanlde_t get_dma_handle(rtos_dma_config_t config)
//{
//	return dma_handle;
//}

void wait_for_DMA_Transfer()
{
	xSemaphoreTake(dma_handle.binary_semaphore, portMAX_DELAY);
}


