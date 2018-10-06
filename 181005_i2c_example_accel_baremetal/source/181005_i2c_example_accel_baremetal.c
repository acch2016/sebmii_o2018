#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "fsl_debug_console.h"
#include "fsl_port.h"
#include "fsl_i2c.h"

volatile bool g_MasterCompletionFlag = false;

float new0 = 0, new1 = 0, new2 = 0;

static void i2c_master_callback(I2C_Type *base, i2c_master_handle_t *handle,
        status_t status, void * userData)
{
	if (status == kStatus_Success)
	{
		g_MasterCompletionFlag = true;
	}
}

int main(void)
{

	/* Init board hardware. */
	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();
	/* Init FSL debug console. */
	BOARD_InitDebugConsole();

	CLOCK_EnableClock(kCLOCK_PortE);
	CLOCK_EnableClock(kCLOCK_I2c0);

	port_pin_config_t config_i2c =
	{ kPORT_PullDisable, kPORT_SlowSlewRate, kPORT_PassiveFilterDisable,
	        kPORT_OpenDrainDisable, kPORT_LowDriveStrength, kPORT_MuxAlt5,
	        kPORT_UnlockRegister, };

	PORT_SetPinConfig(PORTE, 24, &config_i2c);
	PORT_SetPinConfig(PORTE, 25, &config_i2c);

	i2c_master_config_t masterConfig;
	I2C_MasterGetDefaultConfig(&masterConfig);
	masterConfig.baudRate_Bps = 100000;
	I2C_MasterInit(I2C0, &masterConfig, CLOCK_GetFreq(kCLOCK_BusClk));

	i2c_master_handle_t g_m_handle;
	I2C_MasterTransferCreateHandle(I2C0, &g_m_handle, i2c_master_callback,
	NULL);

	i2c_master_transfer_t masterXfer;

	uint8_t data_buffer = 0x01;

	masterXfer.slaveAddress = 0x1D;
	masterXfer.direction = kI2C_Write;
	masterXfer.subaddress = 0x2A;
	masterXfer.subaddressSize = 1;
	masterXfer.data = &data_buffer;
	masterXfer.dataSize = 1;
	masterXfer.flags = kI2C_TransferDefaultFlag;

	I2C_MasterTransferNonBlocking(I2C0, &g_m_handle, &masterXfer);
	while (!g_MasterCompletionFlag)
	{
	}
	g_MasterCompletionFlag = false;

	uint8_t buffer[6];
	int16_t accelerometer[3];

	while (1)
	{
		masterXfer.slaveAddress = 0x1D;
		masterXfer.direction = kI2C_Read;
		masterXfer.subaddress = 0x01;
		masterXfer.subaddressSize = 1;
		masterXfer.data = buffer;
		masterXfer.dataSize = 6;
		masterXfer.flags = kI2C_TransferDefaultFlag;

		I2C_MasterTransferNonBlocking(I2C0, &g_m_handle, &masterXfer);
		while (!g_MasterCompletionFlag)
		{
		}
		g_MasterCompletionFlag = false;

		accelerometer[0] = buffer[0] << 8 | buffer[1];
		accelerometer[1] = buffer[2] << 8 | buffer[3];
		accelerometer[2] = buffer[4] << 8 | buffer[5];

		new0 = (accelerometer[0] * (0.000244)) / 4;
		new1 = (accelerometer[1] * (0.000244)) / 4;
		new2 = (accelerometer[2] * (0.000244)) / 4;

//		PRINTF("x %d\n\r",accelerometer[0]);
//		PRINTF("y %d\n\r",accelerometer[1]);
//		PRINTF("z %d\n\n\r", accelerometer[2]);

//		PRINTF("x: %d y: %d z: %d\n\r",accelerometer[0], accelerometer[1],accelerometer[2]);
		PRINTF("x: %11d y: %11d z: %11d\n\r",new0,new1,new2);

	}
	return 0;
}
