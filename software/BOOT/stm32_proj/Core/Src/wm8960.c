/*
 * max9850.c
 *
 *  Created on: Jul 5, 2020
 *      Author: Keshikan
 */


#include "wm8960.h"


//transmit flag
bool is_wm_ready=true;


void wmSetReady()
{
	is_wm_ready=true;
}

void wmSetBusy()
{
	is_wm_ready=false;
}

void wmWaitReady()
{
	while(!is_wm_ready);
}

//reg_addr: 7bit
//dat: 9bit
wmTransmitStatus wmWriteByte(uint8_t reg_addr, uint16_t dat)
{
	uint8_t send[2] = { ((reg_addr & 0x7F) << 1) + (((0x0100 & dat) >> 8) & 0xFF), (dat & 0xFF)};
	wmWaitReady();
	HAL_Delay(1);
//	wmSetBusy();
	if(HAL_OK != HAL_I2C_Master_Transmit(&WM_I2C_DEVICE_HANDLER, WM_I2C_DEVICE_ADDR, send, 2, 1000))
	{
		return WM_TRANSMIT_FAILURE;
	}

	return WM_TRANSMIT_SUCCESS;
}

//volume: 0-40(0x00-0x28)
void wmSetVolume(uint8_t volume)
{

	if(0x28 < volume){
		volume = 0x28;
	}

	//set volume, volume update enable, zero-cross enable.
	//volume 0b110000 is minimum (MUTE).
	wmWriteByte(WM_REG_LOUT1_VOLUME, volume*2 + 0b010101111);
	wmWriteByte(WM_REG_ROUT1_VOLUME, volume*2 + 0b110101111);
}

// call after i2s sending start.
bool wmDACUnmute()
{
	uint32_t errflag = 0x00;

	//unmute
	errflag |= wmWriteByte(WM_REG_ADC_DAC_CTR_1, 0x000);


	return errflag;
}


//refer to WM8960-6158-FL32-M-REV1 board configuration
//return error state (0 is ok)
bool wmInit()
{
	uint32_t errflag = 0x00;

	//Reset WM8960
	errflag |= wmWriteByte(WM_REG_RESET, 0x000);
	HAL_Delay(10);

	//VMID soft start
	errflag |= wmWriteByte(WM_REG_ANTI_POP_1, 0b000000100);

	//Enable VMID, VREF
	errflag |= wmWriteByte(WM_REG_PWR_MGMT_1, 0x0C0);

	//L2MO and R2MO = 0, mute mono mix (in order to enable capless mode)
	errflag |= wmWriteByte(WM_REG_MONO_OUT_MIX_1, 0x000);
	errflag |= wmWriteByte(WM_REG_MONO_OUT_MIX_2, 0x000);

	//Enable DACL, DACR, LOUT1, ROUT1, OUT3
	errflag |= wmWriteByte(WM_REG_PWR_MGMT_2, 0b111100010);

	//Enable l/r output mixer
	errflag |= wmWriteByte(WM_REG_PWR_MGMT_3, 0x00C);

	//dac to output mixer
	errflag |= wmWriteByte(WM_REG_LEFT_OUT_MIX_1, 0x100);
	errflag |= wmWriteByte(WM_REG_RIGHT_OUT_MIX_2, 0x100);

	HAL_Delay(200);

	//Zero-cross timeout setting
	errflag |= wmWriteByte(WM_REG_ADDITIONAL_CONTROL_1, 0b111000011);

	//set volume
	errflag |= wmWriteByte(WM_REG_LOUT1_VOLUME, 0x1F9);
	errflag |= wmWriteByte(WM_REG_ROUT1_VOLUME, 0x1F9);

	//Digital Audio Interface Format, I2S, 16bit
	errflag |= wmWriteByte(WM_REG_AUDIO_INTERFACE_1, 0b000000010);

	//clocking -DACDIV
	errflag |= wmWriteByte(WM_REG_CLOCKING_1, 0b000000000); //if MCLK is 6.144MHz, 0b000010000


	return errflag;

}
