/*
 * wm8960.h
 *
 *  Created on: Jul 5, 2020
 *      Author: Keshikan
 */

#ifndef INC_WM8960_H_
#define INC_WM8960_H_

#include <stdint.h>
#include <stdbool.h>
#include "stm32h7xx_hal.h"

//porting
extern I2C_HandleTypeDef hi2c4;
#define WM_I2C_DEVICE_HANDLER (hi2c4)
#define WM_I2C_DEVICE_ADDR (0x34)

//Register addr

#define WM_REG_L_INPUT_VOLUME (0x00)
#define WM_REG_R_INPUT_VOLUME (0x01)
#define WM_REG_LOUT1_VOLUME (0x02)
#define WM_REG_ROUT1_VOLUME (0x03)
#define WM_REG_CLOCKING_1 (0x04)
#define WM_REG_ADC_DAC_CTR_1 (0x05)
#define WM_REG_ADC_DAC_CTR_2 (0x06)
#define WM_REG_AUDIO_INTERFACE_1 (0x07)
#define WM_REG_CLOCKING_2 (0x08)
#define WM_REG_AUDIO_INTERFACE_2 (0x09)
#define WM_REG_L_DAC_VOLUME (0x0A)
#define WM_REG_R_DAC_VOLUME (0x0B)
#define WM_REG_RESET (0x0F)
#define WM_REG_3D_CONTROL (0x10)
#define WM_REG_ALC_1 (0x11)
#define WM_REG_ALC_2 (0x12)
#define WM_REG_ALC_3 (0x13)
#define WM_REG_NOISE_GATE (0x14)
#define WM_REG_L_ADC_VOLUME (0x15)
#define WM_REG_R_ADC_VOLUME (0x16)
#define WM_REG_ADDITIONAL_CONTROL_1 (0x17)
#define WM_REG_ADDITIONAL_CONTROL_2 (0x18)
#define WM_REG_PWR_MGMT_1 (0x19)
#define WM_REG_PWR_MGMT_2 (0x1A)
#define WM_REG_ADDITIONAL_CONTROL_3 (0x1B)
#define WM_REG_ANTI_POP_1 (0x1C)
#define WM_REG_ANTI_POP_2 (0x1D)
#define WM_REG_ADCL_SIGNAL_PATH (0x20)
#define WM_REG_ADCR_SIGNAL_PATH (0x21)
#define WM_REG_LEFT_OUT_MIX_1 (0x22)
#define WM_REG_RIGHT_OUT_MIX_2 (0x25)
#define WM_REG_MONO_OUT_MIX_1 (0x26)
#define WM_REG_MONO_OUT_MIX_2 (0x27)
#define WM_REG_LOUT2_VOLUME (0x28)
#define WM_REG_ROUT2_VOLUME (0x29)
#define WM_REG_MONOOUT_VOLUME (0x2A)
#define WM_REG_INPUT_BOOST_MIXER_1 (0x2B)
#define WM_REG_INPUT_BOOST_MIXER_2 (0x2C)
#define WM_REG_BYPASS_1 (0x2D)
#define WM_REG_BYPASS_2 (0x2E)
#define WM_REG_PWR_MGMT_3 (0x2F)
#define WM_REG_ADDITIONAL_CONTROL_4 (0x30)
#define WM_REG_CLASSD_CONTROL_1 (0x31)
#define WM_REG_CLASSD_CONTROL_3 (0x33)
#define WM_REG_PLL_N (0x34)
#define WM_REG_PLL_K_1 (0x35)
#define WM_REG_PLL_K_2 (0x36)
#define WM_REG_PLL_K_3 (0x37)

//transmit sattus

typedef enum{
	WM_TRANSMIT_SUCCESS = 0, WM_TRANSMIT_FAILURE
}wmTransmitStatus;


extern void wmSetReady();
extern wmTransmitStatus wmWriteByte(uint8_t reg_addr, uint16_t dat);
extern bool wmInit_process1();
extern bool wmInit_process2();
extern bool wmDACUnmute();
extern void wmSetGPIO(bool logic);
extern void wmSetVolume(uint8_t volume);

#endif /* INC_WM8960_H_ */
