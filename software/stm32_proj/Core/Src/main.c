/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <core_cm7.h>
#include "st7735s_SPI.h"
#include "ugui.h"
#include "curebuffer_static.h"
#include "usb_device.h"
#include "usbd_midi_if.h"
#include "uartmidi.h"
#include "dfilter.h"
#include "wm8960.h"
#include "curemidi.h"
#include "logo.h"

#include "curedraw.h"
#include "kanjidraw.h"
#include "fontdraw.h"

#include "sram_manage.h"

#include "curesynth_setting.h"

#include "dsp_lib/subdsp.h"

//J-Link RTT
#include "SEGGER_RTT.h"

//LR swap
#define LR_SWAP_ON

//gaming mode (will be omitted)
//#define GAMING_MODE_ON

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ADC_LENGTH (16)
#define FILT_WAIT_CYCLE (3000)
#define __ATTR_DTCMRAM   __attribute__ ((section(".DTCMRAM"))) __attribute__ ((aligned (4)))
#define __ATTR_ITCMRAM   __attribute__ ((section(".ITCMRAM"))) __attribute__ ((aligned (8)))
#define __ATTR_RAM_D2	__attribute__ ((section(".RAM_D2"))) __attribute__ ((aligned (4)))
#define __ATTR_RAM_D3	__attribute__ ((section(".RAM_D3"))) __attribute__ ((aligned (4)))
#define __ATTR_QSPI	   __attribute__ ((section(".QSPI_PSRAM"))) __attribute__ ((aligned (4)))

#define __ATTR_SETTING __attribute__ ((section(".setting_section"))) __attribute__ ((aligned (4)))
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
DMA_HandleTypeDef hdma_adc1;
DMA_HandleTypeDef hdma_adc2;

I2C_HandleTypeDef hi2c4;

IWDG_HandleTypeDef hiwdg1;

OSPI_HandleTypeDef hospi1;

RNG_HandleTypeDef hrng;

SAI_HandleTypeDef hsai_BlockA1;
DMA_HandleTypeDef hdma_sai1_a;

SPI_HandleTypeDef hspi3;
DMA_HandleTypeDef hdma_spi3_tx;

TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim16;
TIM_HandleTypeDef htim17;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart7;
UART_HandleTypeDef huart10;
DMA_HandleTypeDef hdma_uart7_rx;

DMA_HandleTypeDef hdma_memtomem_dma1_stream0;
DMA_HandleTypeDef hdma_memtomem_dma1_stream3;
MDMA_HandleTypeDef hmdma_mdma_channel40_dma1_stream0_tc_0;
MDMA_LinkNodeTypeDef node_mdma_channel40_dma1_stream3_tc_1;
/* USER CODE BEGIN PV */
extern USBD_HandleTypeDef hUsbDeviceHS;

//for uGUI
UG_GUI gui;

//for ADC
uint16_t adc_sens_val[ADC_LENGTH] __ATTR_RAM_D2;
uint16_t adc_prev_val = 0;

uint16_t adc_sens_val_vbus[ADC_LENGTH] __ATTR_RAM_D2;
uint16_t adc_prev_val_vbus = 0;

//DAC
volatile uint32_t output_idx;
uint32_t output_started_flag;
int32_t output_err_distance;
int16_t output_buffer[OUT_BUFSIZE * 2] __ATTR_RAM_D2;
//volatile uint32_t table_idx=0;

int16_t audio_output[2];

//Displaydata
DispInfo dispinfo;

//DisplaySetting
DispSettingChangeInfo disp_change_info;


//curesynth config
const char __ATTR_SETTING curesynth_ver_string[16] = "0.8.0.a";
const uint32_t __ATTR_SETTING curesynth_firm_size = 0x00000000;
const uint32_t __ATTR_SETTING curesynth_checksum = 0x00000000;

//volume
//need conditions
//(VOL_ADC_MAX_VALUE - VOL_ADC_MIN_VALUE + 1)
//  %(VOL_9850_INDICATE_MAX + 1)
//   = 0
#define VOL_ADC_MAX_VALUE (3999)
#define VOL_ADC_MIN_VALUE (100)
#define VOL_8960_INDICATE_MAX (0x28)	//0x00-0x4F(0-40)
#define VOL_SECTION_NUM ( (VOL_ADC_MAX_VALUE - VOL_ADC_MIN_VALUE + 1) / (VOL_8960_INDICATE_MAX + 1) )
#define VOL_HYSTERESIS_VALUE (13)
#define CALC_SEC_NUM(a) ((a - VOL_ADC_MIN_VALUE) / VOL_SECTION_NUM )

typedef enum{
	TIMER_NOT_DISPLAYED, TIMER_DISPLAYED
}TimerDisplayedFlag;

typedef struct{
	uint32_t value;
	TimerDisplayedFlag status;
}Common_Timer;

#define VOL_CHANGED_TIMER_RESET_VALUE (200)//x10ms
Common_Timer vol_changed_displaying_timer = {0, TIMER_DISPLAYED};

#define TXT_CHANGED_TIMER_RESET_VALUE (200)//x10ms
Common_Timer txt_changed_displaying_timer = {0, TIMER_DISPLAYED};


typedef enum{
	VOLUME_CHANGED, VOLUME_UNCHANGED
}VolumeChangedFlag;

typedef enum{
	STATE_A, STATE_B
}VolumeState;

typedef struct{
	VolumeState volstat;
	uint8_t section_num;
}Volume;

Volume volume = {STATE_A, 0};


//VBUS sensing
//4.0V: 3250, 3.9V: 3169 3.8V:3088
#define VBUS_ADC_THR (3250)


//data watchpoint trigger
#define FP_LAR_PTR ((volatile unsigned int *) 0xe0000fb0)
// Lock Status Register lock status bit
#define DWT_LSR_SLK_Pos                1
#define DWT_LSR_SLK_Msk                (1UL << DWT_LSR_SLK_Pos)
// Lock Status Register lock availability bit
#define DWT_LSR_SLI_Pos                0
#define DWT_LSR_SLI_Msk                (1UL << DWT_LSR_SLI_Pos)
// Lock Access key, common for all
#define DWT_LAR_KEY                    0xC5ACCE55
float cpu_usage_percent = 0.0f;
float max_cpu_usage_percent = 0.0f;

//overload
#define OVERLOAD_THR_PERCENT (98.0f)
uint32_t overload_cnt = 0;

//buffer full count
uint32_t uart_buf_full_cnt = 0;
uint32_t usb_buf_full_cnt = 0;

//midi receive buffer
uint8_t midi_rx_uart __ATTR_RAM_D2;



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_I2C4_Init(void);
static void MX_IWDG1_Init(void);
static void MX_RNG_Init(void);
static void MX_SAI1_Init(void);
static void MX_SPI3_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM7_Init(void);
static void MX_USART10_UART_Init(void);
static void MX_TIM17_Init(void);
static void MX_TIM16_Init(void);
static void MX_OCTOSPI1_Init(void);
static void MX_MDMA_Init(void);
static void MX_UART4_Init(void);
static void MX_UART7_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//override printf
int _write(int file, char *ptr, int len){
	SEGGER_RTT_Write(0, ptr, len);
	return len;
}

//SWO print string
int32_t SWO_writeStr(char *ptr)
{
	int32_t len = strlen(ptr);

	for(uint32_t i=0; i<len; i++){
		ITM_SendChar((*ptr++));
	}

	return len;
}

//from https://stackoverflow.com/questions/38355831/measuring-clock-cycle-count-on-cortex-m7
inline void dwt_access_enable()
{
	uint32_t lsr = DWT->LSR;
	if ((lsr & DWT_LSR_SLI_Msk) != 0)
	{
		if ((lsr & DWT_LSR_SLK_Msk) != 0)    //if locked: access need unlock
			DWT->LAR = DWT_LAR_KEY;			//unlock
	}
}

inline void dwt_access_disable()
{
	uint32_t lsr = DWT->LSR;
	if ((lsr & DWT_LSR_SLI_Msk) != 0)
	{
		if ((lsr & DWT_LSR_SLK_Msk) == 0)    //if unlocked
			DWT->LAR = 0;					//lock
	}
}

inline void initCycle()
{
	dwt_access_enable();
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0;
}

inline void startCycle()
{
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	DWT->CYCCNT = 0;
}

inline void stopAndResetCycle()
{
	DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
	DWT->CYCCNT = 0;
}

inline uint32_t getCycleValue()
{
	return DWT->CYCCNT;
}

#define CPU_AVE_NUM (256)
float calcCpuUsage()
{
	//moving average of process usage.
	//	static float buf[CPU_AVE_NUM] = {0.0};
	//	static uint32_t idx;
	//	static float sum = 0.0f;
	//
	//	float full_load_clk = (float)SystemCoreClock / (float)SAMPLE_RATE;
	//
	//	sum -= buf[idx];
	//	buf[idx] = ((float)getCycleValue() * 100.0f) / full_load_clk;
	//	sum += buf[idx];
	//	idx = (idx+1) & (CPU_AVE_NUM -1);
	//
	//	return sum / CPU_AVE_NUM ;

	float full_load_clk = (float)SystemCoreClock / (float)SAMPLE_RATE;
	return ((float)getCycleValue() * 100.0f) / full_load_clk ;
}

float calcProcessTimeMilliseconds()
{
	return ((float)getCycleValue() * 1000.0f) / (float)SystemCoreClock ;
}

inline void cureAudioProc()
{

	//generate audio
	cureSynthGetOutput(audio_output);

	//generate square test signal
	//	cureSynthGenerateTestSignalSquare(audio_output);


	//store output buffer
#ifdef LR_SWAP_ON
	output_buffer[output_idx] = audio_output[1];
	output_buffer[output_idx+1] = audio_output[0];
#else
	output_buffer[output_idx] = audio_output[0];
	output_buffer[output_idx+1] = audio_output[1];
#endif
	output_idx+=2;
	output_idx &= (OUT_BUFSIZE*2 -1);
	output_err_distance+=2;

}


//call from main
void processADCValue()
{
	adc_prev_val = dfRCFilt(adc_sens_val[0], adc_prev_val);
	adc_prev_val_vbus = dfRCFilt(adc_sens_val_vbus[0], adc_prev_val_vbus);
}

inline uint16_t getADCValue()
{
	return adc_prev_val;
}

inline uint16_t getADCValue_Vbus()
{
	return adc_prev_val_vbus;
}


VolumeChangedFlag setVolumeFromADC()
{
	uint16_t adcval = getADCValue();

	//clipping
	if(VOL_ADC_MAX_VALUE < adcval){
		adcval = VOL_ADC_MAX_VALUE;
	}else if( VOL_ADC_MIN_VALUE > adcval){
		adcval = VOL_ADC_MIN_VALUE;
	}

	switch(volume.volstat){

	case STATE_A:
		if(adcval > (VOL_ADC_MIN_VALUE + volume.section_num * VOL_SECTION_NUM + VOL_SECTION_NUM -1)){
			volume.section_num = CALC_SEC_NUM(adcval);
			return VOLUME_CHANGED;
		}else if(adcval < (VOL_ADC_MIN_VALUE + volume.section_num * VOL_SECTION_NUM -VOL_HYSTERESIS_VALUE)){
			volume.section_num = CALC_SEC_NUM(adcval);
			volume.volstat = STATE_B;
			return VOLUME_CHANGED;
		}
		break;

	case STATE_B:
		if(adcval > (VOL_ADC_MIN_VALUE + volume.section_num * VOL_SECTION_NUM + VOL_SECTION_NUM -1) + VOL_HYSTERESIS_VALUE ){
			volume.section_num = CALC_SEC_NUM(adcval);
			volume.volstat = STATE_A;
			return VOLUME_CHANGED;
		}else if(adcval < (VOL_ADC_MIN_VALUE + volume.section_num * VOL_SECTION_NUM)){
			volume.section_num = CALC_SEC_NUM(adcval);
			return VOLUME_CHANGED;
		}
		break;

	default:
		break;

	}

	return VOLUME_UNCHANGED;

}

void setVolumeIndicateTimer()
{
	vol_changed_displaying_timer.value = VOL_CHANGED_TIMER_RESET_VALUE;
	vol_changed_displaying_timer.status = TIMER_NOT_DISPLAYED;
}

void setTextIndicateTimer()
{
	txt_changed_displaying_timer.value = TXT_CHANGED_TIMER_RESET_VALUE;
	txt_changed_displaying_timer.status = TIMER_NOT_DISPLAYED;
}


void sensingVbus()
{
	uint16_t adcval = getADCValue_Vbus();

	if(VBUS_ADC_THR >= adcval){

		//mute
		wmSetVolume(0);

		//display
		cdDrawError(ERR_VOLTAGE);

		//synth
		__disable_irq();
		usbMidiBufferFlush(0);
		uartMidiBufferFlush();
		__enable_irq();

		while(1);
	}

}

FUNC_STATUS cureInit()
{

	//MIDI Initializations.
	if( FUNC_ERROR == cureMidiInit() )
	{
		return FUNC_ERROR;
	}

	//display information init
	dpinfoInit(&dispinfo);
	dpChangeInfoInit(&disp_change_info);
	return FUNC_SUCCESS;

}


void changeSystemClockToHigh()
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
			|RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
	RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
	{
		Error_Handler();
	}

	SystemCoreClockUpdate();
	HAL_Delay(50);

}

void changeSystemClockToLow()
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
			|RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV8;//if over DIV16, USB configulation is not working.
	RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
	RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
	{
		Error_Handler();
	}


	SystemCoreClockUpdate();
	HAL_Delay(50);
}

void MPU_Config_WriteThrough()
{
	MPU_Region_InitTypeDef MPU_InitStruct;

	/* Disable the MPU */
	HAL_MPU_Disable();

	//RAM_D2 32KiB
	/* Configure the MPU attributes as WT for SRAM */
	MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress = 0x30000000;
	MPU_InitStruct.Size = MPU_REGION_SIZE_32KB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.Number = MPU_REGION_NUMBER0;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	//RAM_D1_VRAM1 64Kib
	/* Configure the MPU attributes as WT for SRAM */
	MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress = 0x24000000;
	MPU_InitStruct.Size = MPU_REGION_SIZE_64KB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number = MPU_REGION_NUMBER1;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	//RAM_D1_VRAM1 16Kib
	/* Configure the MPU attributes as WT for SRAM */
	MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress = 0x24010000;
	MPU_InitStruct.Size = MPU_REGION_SIZE_16KB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number = MPU_REGION_NUMBER2;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	//RAM_D1_VRAM1 4Kib
	/* Configure the MPU attributes as WT for SRAM */
	MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress = 0x24014000;
	MPU_InitStruct.Size = MPU_REGION_SIZE_4KB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number = MPU_REGION_NUMBER3;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);


	//External QSPI SRAM (OCTOSPI1)
	/* Configure the MPU attributes as Strongly-Ordered for OCTOSPI1*/
	MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress = 0x90000000;
	MPU_InitStruct.Size = MPU_REGION_SIZE_32MB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.Number = MPU_REGION_NUMBER4;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Enable the MPU */
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

#define SRAM_TEST_SIZE (0x200000)
uint8_t test_sram_mem[SRAM_TEST_SIZE] __ATTR_QSPI;

bool sramTest()
{
	for(uint32_t i=0; i<SRAM_TEST_SIZE; i++)
	{
		test_sram_mem[i] = 0xAA;
	}

	for(uint32_t i=0; i<SRAM_TEST_SIZE; i++)
	{
		if(test_sram_mem[i] != 0xAA)
		{
			return false;
		}
	}

	for(uint32_t i=0; i<SRAM_TEST_SIZE; i++)
	{
		test_sram_mem[i] = 0x55;
	}

	for(uint32_t i=0; i<SRAM_TEST_SIZE; i++)
	{
		if(test_sram_mem[i] != 0x55)
		{
			return false;
		}
	}

	return true;
}

void drawSynthDebugInfo()
{
	static uint16_t counter = 0;
	static uint16_t state_number = 0;
	static uint16_t poly=0, poly_max = 0;

	counter++;

	if(counter <= 1000){
		return;
	}
	counter = 0;


	if(OPNUM > state_number && state_number >= 0){
		printf("OP%2d CH:%03d, CHD:%03d, NOTE:%03d, ",state_number, au_operator[state_number].ch, au_operator[state_number].ch_dcblock, au_operator[state_number].note_number);

		if(NOTE_OFF == au_operator[state_number].stat){
			printf("NOTE:OFF, ");
		}else{
			printf("NOTE: ON, ");
		}

		switch(au_operator[state_number].wav.type){
		case SINE:
			printf("TYP:SIN, ");
			break;
		case SQUARE:
			printf("TYP:SQU, ");
			break;
		case SAW:
			printf("TYP:SAW, ");
			break;
		case TRIANGLE:
			printf("TYP:TRI, ");
			break;
		case WHITE_NOISE:
			printf("TYP:WHI, ");
			break;
		case PINK_NOISE:
			printf("TYP:PNK, ");
			break;
		case PCM:
			printf("TYP:PCM, ");
			break;
		case PCM_REV:
			printf("TYP:REV, ");
			break;
		default:
			break;

		}

		switch(au_operator[state_number].env.stat){
		case ADSR_START:
			printf("S:START, ");
			break;
		case ATTACK:
			printf("S:ATTAC, ");
			break;
		case DECAY:
			printf("S:DECAY, ");
			break;
		case SUSTAIN:
			printf("S:SUSTN, ");
			break;
		case RELEASE:
			printf("S:RELAS, ");
			break;
		case ADSR_END:
			printf("S:END__, ");
			break;
		case ADSR_FINAL:
			printf("S:FINAL, ");
			break;
		default:
			break;

		}

		//		printf("PIT: %06d, ", au_operator[state_number].wav.pitch);
		printf("UNISC: %3d, ", au_operator[state_number].unisonScale);
		printf("OUT: %8ld, ", au_operator[state_number].out);
		printf("\033[0K");
		printf("\r\n");


	}else if( (OPNUM+16) > state_number && state_number >= OPNUM){

		uint16_t ch_num = (state_number - OPNUM);

		printf("CH%2d OUTL:%7d OUTR:%7d ACTIVE_OP: ", ch_num, au_out[ch_num].mono.left, au_out[ch_num].mono.right);
		for(uint16_t i=0; i<OPNUM; i++){
			if(NOTE_ON == master.channel[ch_num].operator_to_channel.ison[i]){
				printf("%d, ", i);
			}
		}
		printf("\033[0K");
		printf("\r\n");


		printf("CH%d ACTIVE_NOTE_TO_OPNUM: ", ch_num);
		for(uint16_t i=0; i<128; i++){
			if(255 != master.channel[ch_num].operator_to_note.stat[i]){
				printf("NOTE%03d OPNUM%02d, ", i, master.channel[ch_num].operator_to_note.stat[i]);
			}

		}
		printf("\033[0K");
		printf("\r\n");

	}else if( (OPNUM+16) == state_number){

		printf("CPU: %04.1f%% MAX: %04.1f%%\033[0K\r\n", cpu_usage_percent, max_cpu_usage_percent);

		poly = getUsbRxBufUsedSize(0);
		if(poly_max <= poly){
			poly_max = poly;
		}
		printf("MBUF: %d MAX: %d\033[0K\r\n",poly, poly_max);

	}else if( (OPNUM+17) == state_number){
		printf("OUTL:%07d, OUTR:%07d\033[0K\r\n", output_buffer[output_idx], output_buffer[output_idx+1]);
		printf("OL_CNT:%ld\033[0K\r\n", overload_cnt);
	}else if( (OPNUM+18) == state_number){
		printf("\033[0J");
	}else if( (OPNUM+19) == state_number){
		printf("\033[100F");
	}
	fflush(stdout);

	state_number++;
	if(state_number > (OPNUM+19)){
		state_number = 0;
	}

}

uint16_t convRGB888toRGB565(uint8_t r, uint8_t g, uint8_t b)
{
	uint16_t col = ((r >> 3)<<11) + ((g >> 2)<<5) + (b >> 3);
	return col;
}

typedef enum{
	IMG_BASE, IMG_EYE1, IMG_EYE2, IMG_EYE3, IMG_EAR1, IMG_EAR2,
}IMG_NUM;

void loadimg(IMG_NUM inum)
{
	switch(inum){

	case IMG_BASE:
		UG_FillFrame(2, 20, 159, 79, 0x0000);
		//base
		for(uint32_t logo_y=0; logo_y<IMG_R1_Y; logo_y++){
			for(uint32_t logo_x=0; logo_x<IMG_R1_X; logo_x++){
				UG_DrawPixel(2+logo_x, 25+logo_y, base_r1[logo_y][logo_x]);
			}
		}

		for(uint32_t logo_y=0; logo_y<IMG_R2_Y; logo_y++){
			for(uint32_t logo_x=0; logo_x<IMG_R2_X; logo_x++){
				UG_DrawPixel(114+logo_x, 9+logo_y, base_r2[logo_y][logo_x]);
			}
		}
		break;
	case IMG_EYE1:
		//eye
		for(uint32_t logo_y=0; logo_y<IMG_EYE_Y; logo_y++){
			for(uint32_t logo_x=0; logo_x<IMG_EYE_X; logo_x++){
				UG_DrawPixel(121+logo_x, 52+logo_y, eye_1[logo_y][logo_x]);
			}
		}
		break;
	case IMG_EYE2:
		//eye
		for(uint32_t logo_y=0; logo_y<IMG_EYE_Y; logo_y++){
			for(uint32_t logo_x=0; logo_x<IMG_EYE_X; logo_x++){
				UG_DrawPixel(121+logo_x, 52+logo_y, eye_2[logo_y][logo_x]);
			}
		}
		break;
	case IMG_EYE3:
		//eye
		for(uint32_t logo_y=0; logo_y<IMG_EYE_Y; logo_y++){
			for(uint32_t logo_x=0; logo_x<IMG_EYE_X; logo_x++){
				UG_DrawPixel(121+logo_x, 52+logo_y, eye_3[logo_y][logo_x]);
			}
		}
		break;
	case IMG_EAR1:
		//ear left
		for(uint32_t logo_y=0; logo_y<IMG_EL_Y; logo_y++){
			for(uint32_t logo_x=0; logo_x<IMG_EL_X; logo_x++){
				UG_DrawPixel(92+logo_x, 28+logo_y, eleft_1[logo_y][logo_x]);
			}
		}
		//ear right
		for(uint32_t logo_y=0; logo_y<IMG_ER_Y; logo_y++){
			for(uint32_t logo_x=0; logo_x<IMG_ER_X; logo_x++){
				UG_DrawPixel(130+logo_x, 9+logo_y, eright_1[logo_y][logo_x]);
			}
		}

		break;
	case IMG_EAR2:
		//ear left
		for(uint32_t logo_y=0; logo_y<IMG_EL_Y; logo_y++){
			for(uint32_t logo_x=0; logo_x<IMG_EL_X; logo_x++){
				UG_DrawPixel(92+logo_x, 28+logo_y, eleft_2[logo_y][logo_x]);
			}
		}
		//ear right
		for(uint32_t logo_y=0; logo_y<IMG_ER_Y; logo_y++){
			for(uint32_t logo_x=0; logo_x<IMG_ER_X; logo_x++){
				UG_DrawPixel(130+logo_x, 9+logo_y, eright_2[logo_y][logo_x]);
			}
		}
		break;
	default:
		break;
	}

}

void deviceTestSram(){

	//QSPI SRAM test
	if(!sramTest()){
		cdDrawError(ERR_SRAM);
		while(1);
	}

	UG_FillFrame(0, 0, 80, 20, C_BLACK);
	fdDrawStr(0, 0, "Initializing...");
	fdSetBackgroundColor(COL_BACKGROUND_565);
	displayUpdate();
	displayWaitDispalyTransfer();

	UG_FillScreen(C_BLACK);
}

void deviceTestDAC(){

	volatile bool errflag = false;

	wmSetGPIO(false);
	HAL_Delay(10);
	if(GPIO_PIN_RESET != HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_1))
	{
		errflag |= true;
	}

	wmSetGPIO(true);
	HAL_Delay(10);
	if(GPIO_PIN_SET != HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_1))
	{
		errflag |= true;
	}

	if(errflag){
		cdDrawError(ERR_DAC);
		while(1);
	}

}

#define HW_VER_READ_TIMEOUT (100)
#define HW_VER_READ_WAIT (20)
#define HW_VER_GPIOVER (uint8_t)((GPIOF->IDR &0b111100 )>>2)//PF2-PF5
uint8_t getHardwareVersion(){

	uint8_t hwver;
	uint8_t gpio_state, gpio_state_prev;


	gpio_state_prev = HW_VER_GPIOVER;

	for(uint32_t i=0; i<HW_VER_READ_TIMEOUT; i++){
		HAL_Delay(HW_VER_READ_WAIT);
		gpio_state = HW_VER_GPIOVER;

		if(gpio_state == gpio_state_prev){
			break;
		}

		gpio_state_prev = gpio_state;
	}

	switch(gpio_state){
	case 0b1000://PF2-PF5:LLLH
		hwver = 0x0A;
		break;
	default:
		hwver = 0x00;
		break;
	}

	return hwver;

}


void loadLogo(){

	char strbuf[16];

	fdSetBackgroundColor(C_BLACK);
	UG_FillScreen(C_BLACK);
	fdDrawStr(0, 0, "SW:");
	fdDrawStr(26, 0, (char *)curesynth_ver_string);
	fdDrawStr(0, 10, "HW:");
	sprintf(strbuf, "%02X", getHardwareVersion());
	fdDrawStr(26, 10, strbuf);

	loadimg(IMG_BASE);
	displayUpdate();
	HAL_Delay(500);

	loadimg(IMG_EYE1);
	displayUpdate();
	HAL_Delay(50);

	loadimg(IMG_BASE);
	loadimg(IMG_EYE3);
	loadimg(IMG_EAR1);
	displayUpdate();
	HAL_Delay(50);

	loadimg(IMG_BASE);
	loadimg(IMG_EYE3);
	loadimg(IMG_EAR2);
	displayUpdate();
	HAL_Delay(50);

	loadimg(IMG_BASE);
	loadimg(IMG_EYE3);
	displayUpdate();
	HAL_Delay(50);

	loadimg(IMG_BASE);
	loadimg(IMG_EYE3);
	loadimg(IMG_EAR1);
	displayUpdate();
	HAL_Delay(50);

	loadimg(IMG_BASE);
	loadimg(IMG_EYE3);
	loadimg(IMG_EAR2);
	displayUpdate();
	HAL_Delay(50);

	loadimg(IMG_BASE);
	loadimg(IMG_EYE1);
	displayUpdate();
	HAL_Delay(50);

	loadimg(IMG_BASE);
	displayUpdate();


	displayWaitDispalyTransfer();

}


void cureReceiveAndProcessMidiMessage()
{
	uint8_t midi_dat_usb;
	uint8_t midi_dat_uart;

	//receive midi from usb
	while( FUNC_SUCCESS == midiGetFromUsbRx(0, &midi_dat_usb) ){

		HAL_IWDG_Refresh(&hiwdg1);
		if(isUsbRxBufferFull()){
			usb_buf_full_cnt++;
			__disable_irq();
			usbMidiBufferFlush(0);
			cureMidiResetFromBufferFull();
			__enable_irq();
			break;
		}
		//		  printf("0x%02x\r\n", midi_dat);
		if( cureMidiEventIsGenerated(midi_dat_usb, SRC_USB) ){// Generate MIDI event from USB buffer.
			//Analyze MIDI Message.
			cureMidiAnalyzeEvent(SRC_USB);
		}
	}

	//receive midi from uart
	while( FUNC_SUCCESS == cureUartMidiBufferDequeue(&midi_dat_uart) ){

		HAL_IWDG_Refresh(&hiwdg1);
		if(isUartRxBufferFull()){
			uart_buf_full_cnt++;
			__disable_irq();
			uartMidiBufferFlush();
			cureMidiResetFromBufferFull();
			__enable_irq();
			break;
		}
		if( cureMidiEventIsGenerated(midi_dat_uart, SRC_UART) ){// Generate MIDI event from UART buffer.
			//Analyze MIDI Message.
			cureMidiAnalyzeEvent(SRC_UART);
		}
	}

}

void activateMonitoringAudioGen()
{
	max_cpu_usage_percent = 0;
	cpu_usage_percent = 0;
	HAL_TIM_Base_Start_IT(&htim17);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == UART7)
	{
		cureUartMidiBufferEnqueue(&midi_rx_uart);
		HAL_UART_Receive_DMA(&huart7, &midi_rx_uart, 1);
	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == UART7)
	{
		HAL_UART_Receive_DMA(&huart7, &midi_rx_uart, 1);
	}
}


void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c){

	if(hi2c->Instance == I2C2)
	{
		displayDMAMain();
	}else if(hi2c->Instance == I2C4)
	{
		wmSetReady();
	}

}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c){

	if(hi2c->Instance == I2C4)
	{
		wmSetReady();
	}

}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){

	if(hspi->Instance == SPI3)
	{
		displaySemaphoreClear();
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

	if(htim->Instance == TIM17)	//Monitoring AudioProcess Load
	{

		if((cpu_usage_percent >= OVERLOAD_THR_PERCENT) || ( calcCpuUsage() >= OVERLOAD_THR_PERCENT)){
			if(!cureMidiGetOverloadFlag()){
				//stop audio process
				HAL_TIM_Base_Stop_IT(&htim7);

				//Assert audio process restart flag
				cureMidiEnableOverloadFlag();

				cpu_usage_percent = 0;

				stopAndResetCycle();
			}
		}

	}else if(htim->Instance == TIM7)// Audio Process
	{
		startCycle();
		cureAudioProc();
		cpu_usage_percent = calcCpuUsage();
		stopAndResetCycle();
		if(max_cpu_usage_percent <= cpu_usage_percent){
			max_cpu_usage_percent = cpu_usage_percent;
		}
	}else if(htim->Instance == TIM6)	//10ms timer
	{
		if(vol_changed_displaying_timer.value > 0){
			vol_changed_displaying_timer.value--;
		}

		if(txt_changed_displaying_timer.value > 0){
			txt_changed_displaying_timer.value--;
		}

	}else if(htim->Instance == TIM16)//midi receive timer
	{

		cureReceiveAndProcessMidiMessage();

		__HAL_TIM_ENABLE_IT(&htim16, TIM_IT_UPDATE);
		__HAL_TIM_ENABLE(&htim16);
	}
}

void HAL_OSPI_TxCpltCallback(OSPI_HandleTypeDef *hospi)
{
	if(hospi->Instance == OCTOSPI1)
	{
		//		sramEndCommunication();
	}
}

void HAL_OSPI_RxCpltCallback(OSPI_HandleTypeDef *hospi)
{
	if(hospi->Instance == OCTOSPI1)
	{
		//		sramEndCommunication();
	}

}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

	//	__disable_irq();
	//	SCB->VTOR = 0x08020400;
	//	__enable_irq();


	MPU_Config_WriteThrough();

	//Load functions into ITCM RAM
	extern const unsigned char itcm_text_start;
	extern const unsigned char itcm_text_end;
	extern const unsigned char itcm_data;
	memcpy( (unsigned char *)(&itcm_text_start), &itcm_data, (int) (&itcm_text_end - &itcm_text_start));

  /* USER CODE END 1 */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

/* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

	//  changeSystemClockToLow();
	//inactivate IWDG in debug mode.
	//see https://stackoverflow.com/questions/32532916/disabling-the-stm32-iwdg-during-debugging
	__HAL_DBGMCU_FREEZE_IWDG1();


  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_I2C4_Init();
  MX_RNG_Init();
  MX_SAI1_Init();
  MX_SPI3_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
  MX_USART10_UART_Init();
  MX_TIM17_Init();
  MX_TIM16_Init();
  MX_OCTOSPI1_Init();
  MX_MDMA_Init();
  MX_UART4_Init();
  MX_UART7_Init();
  /* USER CODE BEGIN 2 */


	//HSLV on (!!!!! only 2.5V > VDD !!!!!!!)
	HAL_SYSCFG_EnableIOSpeedOptimize();


	//Display Initialization
	displayInit();

	//Init ugui
	UG_Init(&gui, pset, VRAM_X_SIZE/2, VRAM_Y_SIZE/2);
	UG_SetBackcolor(COL_BACKGROUND_565);
	UG_SetForecolor(COL_FOREGROUND_565);
	UG_FillScreen(C_BLACK);

	UG_FontSelect(&FONT_8X12);

	displayUpdate();
	displayWaitDispalyTransfer();

	curedrawInit();

	//Backlight on
	dispBacklightOn();

	//turn on Analog 3.3V Power
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);

	//debug
	fdSetForegroundColor(COL_FOREGROUND_565);
	fdSetBackgroundColor(COL_BACKGROUND_565);


	//QSPI SRAM Initialization
	if( !sramInit() ){
		while(1){
			cdDrawError(ERR_SRAM);
		}
	}

	//WM8960 DAC Init 1
	if( wmInit_process1()){
		cdDrawError(ERR_DAC);
		while(1);
	}


	loadLogo();

	//WM8960 DAC Init 2
	if( wmInit_process2()){
		cdDrawError(ERR_DAC);
		while(1);
	}
	deviceTestDAC();

	//test sram
	deviceTestSram();



	//MIDI from UART Initializations
	if( FUNC_ERROR == cureUartMidiInit() )
	{
		UG_FillScreen(C_BLACK);
		while(1){
			cdDrawError(ERR_UARTMEM);
		}
	}

	////USB-MIDI Initialization
	//USB-MIDI buffer Init
	if(FUNC_ERROR == midiInit() ){
		UG_FillScreen(C_BLACK);
		while(1){
			cdDrawError(ERR_INITMIDI);
		}
	}

	//USB-MIDI Driver Init
	MX_USB_MIDI_INIT();


	//Wait usb configuration.
	uint32_t usb_check_cnt = 0;
	while(1){
		if(USBD_STATE_CONFIGURED == hUsbDeviceHS.dev_state){
			UG_FillScreen(C_BLACK);
			break;
		}else{
			if(usb_check_cnt >= 500){	//if usb is not configurated within 0.5s, stop waiting.
				UG_FillScreen(C_BLACK);
				break;
			}else{
				HAL_Delay(1);
				usb_check_cnt++;
			}
		}
	}


	//    changeSystemClockToHigh();

	//ADC start : need more times to stabilize ADC
	HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_sens_val, ADC_LENGTH);

	HAL_ADCEx_Calibration_Start(&hadc2, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
	HAL_ADC_Start_DMA(&hadc2, (uint32_t *)adc_sens_val_vbus, ADC_LENGTH);


	//Synthesizer Initialization
	if(FUNC_ERROR == cureInit() ){
		UG_FillScreen(C_BLACK);
		while(1){
			cdDrawError(ERR_INITSYN);
		}
	}

	//Init DAC buffer
	for(uint16_t i=0; i<OUT_BUFSIZE * 2; i++){
		output_buffer[i] = 0;
	}



	//init cpu usage counter
	initCycle();

	//start Audio generation timer and cpu usage counter
	HAL_TIM_Base_Start_IT(&htim7);

	//Start SAI2 and wait DAC buffer will be half-full.
	while(1)
	{
		if((output_idx) > ( OUT_BUFSIZE ) )
		{
			//start DAC
			if(HAL_OK != HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)(output_buffer), OUT_BUFSIZE * 2))
			{
				cdDrawError(ERR_AUDIO);
				Error_Handler();
			}
			output_err_distance = 0;
			break;
		}
	}


	//cycle count active
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

	//start 10ms timer
	HAL_TIM_Base_Start_IT(&htim6);

	//start Monitoring Audio Generation Load
	activateMonitoringAudioGen();


	//Volume set
	for(uint32_t j=0; j<FILT_WAIT_CYCLE; j++){
		processADCValue();
	}
	processADCValue();
	setVolumeFromADC();
	wmSetVolume(volume.section_num);

	//debug J-link RTT
	SEGGER_RTT_Init();

	//draw background
	curedrawBackgroundInit();

	//draw default text
	cdDrawText(DISP_DEFAULT_STRING);

	displayUpdate();
	displayWaitDispalyTransfer();

	//volume unmute
	wmDACUnmute();

	//IWDG activate
	//    MX_IWDG1_Init();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */




#define DISPLAY_INDICATE_THR (10)
	static uint32_t display_indicate_cnt = 0;
	char txt_buffer[32];
	char strbuf[32];


	//start processing midi timer
	HAL_TIM_Base_Start_IT(&htim16);

	//start MIDI message reception with interrupt.
	HAL_UART_Receive_DMA(&huart7, &midi_rx_uart, 1);

	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		HAL_IWDG_Refresh(&hiwdg1);

		drawSynthDebugInfo();

		//sensing vbus
		sensingVbus();



		static bool redraw_default_txt = false;
		static bool redraw_jpText = false;
		if(!displayisUpdating())
		{
			processADCValue();
			if(VOLUME_CHANGED == setVolumeFromADC()){
				wmSetVolume(volume.section_num);
				setVolumeIndicateTimer();
			}

			if(DISPLAY_INDICATE_THR <= display_indicate_cnt){// display indicate speed configure

				if(vol_changed_displaying_timer.value >0){

					if(TIMER_NOT_DISPLAYED == vol_changed_displaying_timer.status){
						cdDrawRotaryVolume(volume.section_num);
						vol_changed_displaying_timer.status = TIMER_DISPLAYED;
						redraw_default_txt = true;
						redraw_jpText = true;
					}

				}else{

					//get txt message
					if(cureMidiIsGetDisplayMessage(txt_buffer)){
						setTextIndicateTimer();
					}

					//get and set display setting change
					cureMidiGetDisplayChangeInfo(&disp_change_info);
					if(COL_CHANGE_NOTHING != disp_change_info.display_color_change_state){
						redraw_jpText = true;
					}
					if(redraw_jpText){
						disp_change_info.language_control_state_upper = LANG_CTRL_UPDATE;
						disp_change_info.language_control_state_lower = LANG_CTRL_UPDATE;
					}

#ifdef GAMING_MODE_ON
					disp_change_info.language_control_state_upper = LANG_CTRL_UPDATE;
					disp_change_info.language_control_state_lower = LANG_CTRL_UPDATE;
					redraw_default_txt = true;
					cdSetDisplayGamingMode();
#else

#endif


					cdSetDisplayChangeInfo(&disp_change_info);

					//draw bargraph
					cureMidiGetDisplayData(&dispinfo);
					uint8_t pk_vol[2];
					for(uint32_t i=0; i<2; i++){
						if(synth_peak_detect.prev_out[i] <= 26){//ref excel sheet
							pk_vol[i] = 0;
						}else{
							pk_vol[i] = 20 * log10f(synth_peak_detect.prev_out[i]/32768.0f) + 63;
							pk_vol[i] = roundf(pk_vol[i]);
							pk_vol[i] = __USAT(pk_vol[i] , 6);
						}
					}
					cdMakeIndicator(&dispinfo, pk_vol, output_buffer, output_idx);

					//eye-blinking animation
					cdDrawEyeBlinking();

					//draw txt message
					if(txt_changed_displaying_timer.value >0){
						if(TIMER_NOT_DISPLAYED == txt_changed_displaying_timer.status){
							cdDrawText(txt_buffer);
							txt_changed_displaying_timer.status = TIMER_DISPLAYED;
							redraw_default_txt = true;
						}

					}else{
						if(redraw_default_txt){
							cdDrawText(DISP_DEFAULT_STRING);
							redraw_default_txt = false;
						}
					}

					//draw USB indicator
					if(USBD_STATE_CONFIGURED == hUsbDeviceHS.dev_state){
						cdDrawUsbState(0);
					}else if(USBD_STATE_SUSPENDED == hUsbDeviceHS.dev_state){
						cdDrawUsbState(1);
					}else{
						cdDrawUsbState(2);
					}

				}

				static bool is_debug_display_changed;
				if(cureMidiIsDebugDisplay()){
					fdSetBackgroundColor(C_BLACK);
					fdSetForegroundColor(C_YELLOW);
					is_debug_display_changed= true;
					sprintf(strbuf, "CPU:%04.1f%% MAX:%04.1f%%", cpu_usage_percent, max_cpu_usage_percent);
					fdDrawStr(0, 0, strbuf);
					sprintf(strbuf, "L:%6d,R:%6d", output_buffer[output_idx], output_buffer[output_idx+1]);
					fdDrawStr(0, 10, strbuf);
					sprintf(strbuf, "OVLD:%03ld", overload_cnt);
					fdDrawStr(0, 22, strbuf);
					sprintf(strbuf, "FUL_USB:%04ld",usb_buf_full_cnt);
					fdDrawStr(0, 32, strbuf);
					sprintf(strbuf, "FUL_UART:%04ld",uart_buf_full_cnt);
					fdDrawStr(0, 42, strbuf);
					sprintf(strbuf, "VOL:%04d",adc_prev_val);
					fdDrawStr(0, 52, strbuf);
					sprintf(strbuf, "VBUS:%04d",adc_prev_val_vbus);
					fdDrawStr(0, 62, strbuf);
					displaySetColorNormal();
				}else{
					if(is_debug_display_changed){
						redraw_default_txt = true;
						redraw_jpText = true;
						is_debug_display_changed = false;
					}

				}

				displayUpdate();
				display_indicate_cnt = 0;
			}else{
				display_indicate_cnt++;
			}

		}

		// if overload
		if(cureMidiGetOverloadFlag()){
			overload_cnt++;

			__disable_irq();
			usbMidiBufferFlush(0);
			uartMidiBufferFlush();
			cureMidiResetFromOverload();

			//restart synth
			cureMidiDisableOverloadFlag();
			HAL_TIM_Base_Start_IT(&htim7);

			__enable_irq();
		}

	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Macro to configure the PLL clock source
  */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 179;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 6;
  RCC_OscInitStruct.PLL.PLLR = 7;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_OSPI|RCC_PERIPHCLK_USB
                              |RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_SAI1
                              |RCC_PERIPHCLK_USART10|RCC_PERIPHCLK_UART7
                              |RCC_PERIPHCLK_UART4;
  PeriphClkInitStruct.PLL2.PLL2M = 8;
  PeriphClkInitStruct.PLL2.PLL2N = 192;
  PeriphClkInitStruct.PLL2.PLL2P = 6;
  PeriphClkInitStruct.PLL2.PLL2Q = 3;
  PeriphClkInitStruct.PLL2.PLL2R = 4;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_0;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.PLL3.PLL3M = 4;
  PeriphClkInitStruct.PLL3.PLL3N = 125;
  PeriphClkInitStruct.PLL3.PLL3P = 2;
  PeriphClkInitStruct.PLL3.PLL3Q = 8;
  PeriphClkInitStruct.PLL3.PLL3R = 2;
  PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_1;
  PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
  PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
  PeriphClkInitStruct.OspiClockSelection = RCC_OSPICLKSOURCE_PLL2;
  PeriphClkInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLL2;
  PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_PLL3;
  PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16910CLKSOURCE_PLL3;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLL3;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV8;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc1.Init.OversamplingMode = ENABLE;
  hadc1.Init.Oversampling.Ratio = 16;
  hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_4;
  hadc1.Init.Oversampling.TriggeredMode = ADC_TRIGGEREDMODE_SINGLE_TRIGGER;
  hadc1.Init.Oversampling.OversamplingStopReset = ADC_REGOVERSAMPLING_CONTINUED_MODE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_14;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_387CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  sConfig.OffsetSignedSaturation = DISABLE;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */

  /** Common config
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV8;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc2.Init.LowPowerAutoWait = DISABLE;
  hadc2.Init.ContinuousConvMode = ENABLE;
  hadc2.Init.NbrOfConversion = 1;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc2.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
  hadc2.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc2.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc2.Init.OversamplingMode = ENABLE;
  hadc2.Init.Oversampling.Ratio = 16;
  hadc2.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_4;
  hadc2.Init.Oversampling.TriggeredMode = ADC_TRIGGEREDMODE_SINGLE_TRIGGER;
  hadc2.Init.Oversampling.OversamplingStopReset = ADC_REGOVERSAMPLING_CONTINUED_MODE;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_387CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  sConfig.OffsetSignedSaturation = DISABLE;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}

/**
  * @brief I2C4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C4_Init(void)
{

  /* USER CODE BEGIN I2C4_Init 0 */

  /* USER CODE END I2C4_Init 0 */

  /* USER CODE BEGIN I2C4_Init 1 */

  /* USER CODE END I2C4_Init 1 */
  hi2c4.Instance = I2C4;
  hi2c4.Init.Timing = 0x60404DA4;
  hi2c4.Init.OwnAddress1 = 0;
  hi2c4.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c4.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c4.Init.OwnAddress2 = 0;
  hi2c4.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c4.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c4.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c4, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c4, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C4_Init 2 */

  /* USER CODE END I2C4_Init 2 */

}

/**
  * @brief IWDG1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG1_Init(void)
{

  /* USER CODE BEGIN IWDG1_Init 0 */

  /* USER CODE END IWDG1_Init 0 */

  /* USER CODE BEGIN IWDG1_Init 1 */

  /* USER CODE END IWDG1_Init 1 */
  hiwdg1.Instance = IWDG1;
  hiwdg1.Init.Prescaler = IWDG_PRESCALER_16;
  hiwdg1.Init.Window = 4095;
  hiwdg1.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG1_Init 2 */

  /* USER CODE END IWDG1_Init 2 */

}

/**
  * @brief OCTOSPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_OCTOSPI1_Init(void)
{

  /* USER CODE BEGIN OCTOSPI1_Init 0 */

  /* USER CODE END OCTOSPI1_Init 0 */

  OSPIM_CfgTypeDef sOspiManagerCfg = {0};

  /* USER CODE BEGIN OCTOSPI1_Init 1 */

  /* USER CODE END OCTOSPI1_Init 1 */
  /* OCTOSPI1 parameter configuration*/
  hospi1.Instance = OCTOSPI1;
  hospi1.Init.FifoThreshold = 1;
  hospi1.Init.DualQuad = HAL_OSPI_DUALQUAD_DISABLE;
  hospi1.Init.MemoryType = HAL_OSPI_MEMTYPE_MICRON;
  hospi1.Init.DeviceSize = 21;
  hospi1.Init.ChipSelectHighTime = 3;
  hospi1.Init.FreeRunningClock = HAL_OSPI_FREERUNCLK_DISABLE;
  hospi1.Init.ClockMode = HAL_OSPI_CLOCK_MODE_0;
  hospi1.Init.WrapSize = HAL_OSPI_WRAP_NOT_SUPPORTED;
  hospi1.Init.ClockPrescaler = 1;
  hospi1.Init.SampleShifting = HAL_OSPI_SAMPLE_SHIFTING_HALFCYCLE;
  hospi1.Init.DelayHoldQuarterCycle = HAL_OSPI_DHQC_DISABLE;
  hospi1.Init.ChipSelectBoundary = 0;
  hospi1.Init.DelayBlockBypass = HAL_OSPI_DELAY_BLOCK_BYPASSED;
  hospi1.Init.MaxTran = 0;
  hospi1.Init.Refresh = 10;
  if (HAL_OSPI_Init(&hospi1) != HAL_OK)
  {
    Error_Handler();
  }
  sOspiManagerCfg.ClkPort = 1;
  sOspiManagerCfg.NCSPort = 1;
  sOspiManagerCfg.IOLowPort = HAL_OSPIM_IOPORT_1_LOW;
  if (HAL_OSPIM_Config(&hospi1, &sOspiManagerCfg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN OCTOSPI1_Init 2 */

  /* USER CODE END OCTOSPI1_Init 2 */

}

/**
  * @brief RNG Initialization Function
  * @param None
  * @retval None
  */
static void MX_RNG_Init(void)
{

  /* USER CODE BEGIN RNG_Init 0 */

  /* USER CODE END RNG_Init 0 */

  /* USER CODE BEGIN RNG_Init 1 */

  /* USER CODE END RNG_Init 1 */
  hrng.Instance = RNG;
  hrng.Init.ClockErrorDetection = RNG_CED_DISABLE;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RNG_Init 2 */

  /* USER CODE END RNG_Init 2 */

}

/**
  * @brief SAI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SAI1_Init(void)
{

  /* USER CODE BEGIN SAI1_Init 0 */

  /* USER CODE END SAI1_Init 0 */

  /* USER CODE BEGIN SAI1_Init 1 */

  /* USER CODE END SAI1_Init 1 */
  hsai_BlockA1.Instance = SAI1_Block_A;
  hsai_BlockA1.Init.AudioMode = SAI_MODEMASTER_TX;
  hsai_BlockA1.Init.Synchro = SAI_ASYNCHRONOUS;
  hsai_BlockA1.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
  hsai_BlockA1.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
  hsai_BlockA1.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
  hsai_BlockA1.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_48K;
  hsai_BlockA1.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
  hsai_BlockA1.Init.MonoStereoMode = SAI_STEREOMODE;
  hsai_BlockA1.Init.CompandingMode = SAI_NOCOMPANDING;
  hsai_BlockA1.Init.TriState = SAI_OUTPUT_NOTRELEASED;
  if (HAL_SAI_InitProtocol(&hsai_BlockA1, SAI_I2S_STANDARD, SAI_PROTOCOL_DATASIZE_16BIT, 2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SAI1_Init 2 */

  /* USER CODE END SAI1_Init 2 */

}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES_TXONLY;
  hspi3.Init.DataSize = SPI_DATASIZE_16BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 0x0;
  hspi3.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi3.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi3.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi3.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi3.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi3.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi3.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi3.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi3.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi3.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 511;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 2684;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 178;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 15;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief TIM16 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM16_Init(void)
{

  /* USER CODE BEGIN TIM16_Init 0 */

  /* USER CODE END TIM16_Init 0 */

  /* USER CODE BEGIN TIM16_Init 1 */

  /* USER CODE END TIM16_Init 1 */
  htim16.Instance = TIM16;
  htim16.Init.Prescaler = 178;
  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim16.Init.Period = 61;
  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim16.Init.RepetitionCounter = 0;
  htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OnePulse_Init(&htim16, TIM_OPMODE_SINGLE) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM16_Init 2 */

  /* USER CODE END TIM16_Init 2 */

}

/**
  * @brief TIM17 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM17_Init(void)
{

  /* USER CODE BEGIN TIM17_Init 0 */

  /* USER CODE END TIM17_Init 0 */

  /* USER CODE BEGIN TIM17_Init 1 */

  /* USER CODE END TIM17_Init 1 */
  htim17.Instance = TIM17;
  htim17.Init.Prescaler = 178;
  htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim17.Init.Period = 15;
  htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim17.Init.RepetitionCounter = 0;
  htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim17) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM17_Init 2 */

  /* USER CODE END TIM17_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart4, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart4, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief UART7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART7_Init(void)
{

  /* USER CODE BEGIN UART7_Init 0 */

  /* USER CODE END UART7_Init 0 */

  /* USER CODE BEGIN UART7_Init 1 */

  /* USER CODE END UART7_Init 1 */
  huart7.Instance = UART7;
  huart7.Init.BaudRate = 31250;
  huart7.Init.WordLength = UART_WORDLENGTH_8B;
  huart7.Init.StopBits = UART_STOPBITS_1;
  huart7.Init.Parity = UART_PARITY_NONE;
  huart7.Init.Mode = UART_MODE_RX;
  huart7.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart7.Init.OverSampling = UART_OVERSAMPLING_8;
  huart7.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_ENABLE;
  huart7.Init.ClockPrescaler = UART_PRESCALER_DIV2;
  huart7.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart7) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart7, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart7, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart7) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART7_Init 2 */

  /* USER CODE END UART7_Init 2 */

}

/**
  * @brief USART10 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART10_UART_Init(void)
{

  /* USER CODE BEGIN USART10_Init 0 */

  /* USER CODE END USART10_Init 0 */

  /* USER CODE BEGIN USART10_Init 1 */

  /* USER CODE END USART10_Init 1 */
  huart10.Instance = USART10;
  huart10.Init.BaudRate = 115200;
  huart10.Init.WordLength = UART_WORDLENGTH_8B;
  huart10.Init.StopBits = UART_STOPBITS_1;
  huart10.Init.Parity = UART_PARITY_NONE;
  huart10.Init.Mode = UART_MODE_TX_RX;
  huart10.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart10.Init.OverSampling = UART_OVERSAMPLING_16;
  huart10.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart10.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart10.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart10) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart10, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart10, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart10) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART10_Init 2 */

  /* USER CODE END USART10_Init 2 */

}

/**
  * Enable DMA controller clock
  * Configure DMA for memory to memory transfers
  *   hdma_memtomem_dma1_stream0
  *   hdma_memtomem_dma1_stream3
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* Configure DMA request hdma_memtomem_dma1_stream0 on DMA1_Stream0 */
  hdma_memtomem_dma1_stream0.Instance = DMA1_Stream0;
  hdma_memtomem_dma1_stream0.Init.Request = DMA_REQUEST_MEM2MEM;
  hdma_memtomem_dma1_stream0.Init.Direction = DMA_MEMORY_TO_MEMORY;
  hdma_memtomem_dma1_stream0.Init.PeriphInc = DMA_PINC_ENABLE;
  hdma_memtomem_dma1_stream0.Init.MemInc = DMA_MINC_ENABLE;
  hdma_memtomem_dma1_stream0.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_memtomem_dma1_stream0.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_memtomem_dma1_stream0.Init.Mode = DMA_NORMAL;
  hdma_memtomem_dma1_stream0.Init.Priority = DMA_PRIORITY_MEDIUM;
  hdma_memtomem_dma1_stream0.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
  hdma_memtomem_dma1_stream0.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
  hdma_memtomem_dma1_stream0.Init.MemBurst = DMA_MBURST_SINGLE;
  hdma_memtomem_dma1_stream0.Init.PeriphBurst = DMA_PBURST_SINGLE;
  if (HAL_DMA_Init(&hdma_memtomem_dma1_stream0) != HAL_OK)
  {
    Error_Handler( );
  }

  /* Configure DMA request hdma_memtomem_dma1_stream3 on DMA1_Stream3 */
  hdma_memtomem_dma1_stream3.Instance = DMA1_Stream3;
  hdma_memtomem_dma1_stream3.Init.Request = DMA_REQUEST_MEM2MEM;
  hdma_memtomem_dma1_stream3.Init.Direction = DMA_MEMORY_TO_MEMORY;
  hdma_memtomem_dma1_stream3.Init.PeriphInc = DMA_PINC_ENABLE;
  hdma_memtomem_dma1_stream3.Init.MemInc = DMA_MINC_ENABLE;
  hdma_memtomem_dma1_stream3.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_memtomem_dma1_stream3.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_memtomem_dma1_stream3.Init.Mode = DMA_NORMAL;
  hdma_memtomem_dma1_stream3.Init.Priority = DMA_PRIORITY_MEDIUM;
  hdma_memtomem_dma1_stream3.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
  hdma_memtomem_dma1_stream3.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
  hdma_memtomem_dma1_stream3.Init.MemBurst = DMA_MBURST_SINGLE;
  hdma_memtomem_dma1_stream3.Init.PeriphBurst = DMA_PBURST_SINGLE;
  if (HAL_DMA_Init(&hdma_memtomem_dma1_stream3) != HAL_OK)
  {
    Error_Handler( );
  }

  /* DMA interrupt init */
  /* DMA1_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
  /* DMA1_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
  /* DMA1_Stream4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  /* DMA2_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);

}

/**
  * Enable MDMA controller clock
  * Configure MDMA for global transfers
  *   hmdma_mdma_channel40_dma1_stream0_tc_0
  *   node_mdma_channel40_dma1_stream3_tc_1
  */
static void MX_MDMA_Init(void)
{

  /* MDMA controller clock enable */
  __HAL_RCC_MDMA_CLK_ENABLE();
  /* Local variables */
  MDMA_LinkNodeConfTypeDef nodeConfig;

  /* Configure MDMA channel MDMA_Channel0 */
  /* Configure MDMA request hmdma_mdma_channel40_dma1_stream0_tc_0 on MDMA_Channel0 */
  hmdma_mdma_channel40_dma1_stream0_tc_0.Instance = MDMA_Channel0;
  hmdma_mdma_channel40_dma1_stream0_tc_0.Init.Request = MDMA_REQUEST_DMA1_Stream0_TC;
  hmdma_mdma_channel40_dma1_stream0_tc_0.Init.TransferTriggerMode = MDMA_BUFFER_TRANSFER;
  hmdma_mdma_channel40_dma1_stream0_tc_0.Init.Priority = MDMA_PRIORITY_LOW;
  hmdma_mdma_channel40_dma1_stream0_tc_0.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
  hmdma_mdma_channel40_dma1_stream0_tc_0.Init.SourceInc = MDMA_SRC_INC_BYTE;
  hmdma_mdma_channel40_dma1_stream0_tc_0.Init.DestinationInc = MDMA_DEST_INC_BYTE;
  hmdma_mdma_channel40_dma1_stream0_tc_0.Init.SourceDataSize = MDMA_SRC_DATASIZE_BYTE;
  hmdma_mdma_channel40_dma1_stream0_tc_0.Init.DestDataSize = MDMA_DEST_DATASIZE_BYTE;
  hmdma_mdma_channel40_dma1_stream0_tc_0.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
  hmdma_mdma_channel40_dma1_stream0_tc_0.Init.BufferTransferLength = 1;
  hmdma_mdma_channel40_dma1_stream0_tc_0.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
  hmdma_mdma_channel40_dma1_stream0_tc_0.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
  hmdma_mdma_channel40_dma1_stream0_tc_0.Init.SourceBlockAddressOffset = 0;
  hmdma_mdma_channel40_dma1_stream0_tc_0.Init.DestBlockAddressOffset = 0;
  if (HAL_MDMA_Init(&hmdma_mdma_channel40_dma1_stream0_tc_0) != HAL_OK)
  {
    Error_Handler();
  }

  /* Configure post request address and data masks */
  if (HAL_MDMA_ConfigPostRequestMask(&hmdma_mdma_channel40_dma1_stream0_tc_0, 0, 0) != HAL_OK)
  {
    Error_Handler();
  }

  /* Initialize MDMA link node according to specified parameters */
  nodeConfig.Init.Request = MDMA_REQUEST_DMA1_Stream3_TC;
  nodeConfig.Init.TransferTriggerMode = MDMA_BUFFER_TRANSFER;
  nodeConfig.Init.Priority = MDMA_PRIORITY_LOW;
  nodeConfig.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
  nodeConfig.Init.SourceInc = MDMA_SRC_INC_BYTE;
  nodeConfig.Init.DestinationInc = MDMA_DEST_INC_BYTE;
  nodeConfig.Init.SourceDataSize = MDMA_SRC_DATASIZE_BYTE;
  nodeConfig.Init.DestDataSize = MDMA_DEST_DATASIZE_BYTE;
  nodeConfig.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
  nodeConfig.Init.BufferTransferLength = 1;
  nodeConfig.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
  nodeConfig.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
  nodeConfig.Init.SourceBlockAddressOffset = 0;
  nodeConfig.Init.DestBlockAddressOffset = 0;
  nodeConfig.PostRequestMaskAddress = 0;
  nodeConfig.PostRequestMaskData = 0;
  nodeConfig.SrcAddress = 0;
  nodeConfig.DstAddress = 0;
  nodeConfig.BlockDataLength = 0;
  nodeConfig.BlockCount = 0;
  if (HAL_MDMA_LinkedList_CreateNode(&node_mdma_channel40_dma1_stream3_tc_1, &nodeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN mdma_channel40_dma1_stream3_tc_1 */

  /* USER CODE END mdma_channel40_dma1_stream3_tc_1 */

  /* Connect a node to the linked list */
  if (HAL_MDMA_LinkedList_AddNode(&hmdma_mdma_channel40_dma1_stream0_tc_0, &node_mdma_channel40_dma1_stream3_tc_1, 0) != HAL_OK)
  {
    Error_Handler();
  }

  /* MDMA interrupt initialization */
  /* MDMA_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(MDMA_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(MDMA_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_9|GPIO_PIN_10, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin : PF1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PF2 PF3 PF4 PF5 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PG9 PG10 */
  GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : PB4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB5 PB7 */
  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PE1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
