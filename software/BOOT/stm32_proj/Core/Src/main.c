/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
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
#include "usb_device.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "usbd_dfu_if.h"

//display
#include "st7735s_SPI.h"
#include "ugui.h"
#include "img.h"

//audio
#include "wm8960.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FW_APP_ADDR (0x08020400)
#define FW_VER_STRING_ADDR (0x08020000)
#define FW_SIZE_ADDR (0x08020010)
#define FW_CHKSUM_ADDR (0x08020014)

#define BOOTLOADER_VER_STRING "0.2.1.b"

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

I2C_HandleTypeDef hi2c4;

SPI_HandleTypeDef hspi3;
DMA_HandleTypeDef hdma_spi3_tx;

TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;

/* USER CODE BEGIN PV */

//for uGUI
UG_GUI gui;
char ug_str_buf[32];

//versions
#define CURESYNTH_VER_STRINGTH_MAX (11)
char curesynth_ver[CURESYNTH_VER_STRINGTH_MAX] = {'\0'};

//display state
bool is_timer_countup = false;
typedef enum{
	DRAW_A, DRAW_B
}DispDrawState;
DispDrawState dispdrawstate = DRAW_A;

//usb
extern USBD_HandleTypeDef hUsbDeviceHS;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI3_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM7_Init(void);
static void MX_I2C4_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//type 0-1
void cureImageDraw(uint8_t type)
{
	if(0 == type){
		//draw nyan
		for(uint32_t idx_y = 0; idx_y<IMG_NYAN_HEIGHT; idx_y++){
			for(uint32_t idx_x = 0; idx_x<IMG_NYAN_WIDTH; idx_x++){
				UG_DrawPixel(58+idx_x, 62+idx_y, nyan_1[idx_y][idx_x]);
			}
		}

		//draw tail
		for(uint32_t idx_y = 0; idx_y<IMG_TAIL_HEIGHT; idx_y++){
			for(uint32_t idx_x = 0; idx_x<IMG_TAIL_WIDTH; idx_x++){
				UG_DrawPixel(27+idx_x, 63+idx_y, tail_1[idx_y][idx_x]);
			}
		}

		//draw layout
		for(uint32_t idx_y = 0; idx_y<IMG_LAYOUT_HEIGHT; idx_y++){
			for(uint32_t idx_x = 0; idx_x<IMG_LAYOUT_WIDTH; idx_x++){
				if(0x0f == layout_1[idx_y][idx_x]){
					UG_DrawPixel(idx_x, idx_y, C_WHITE);
				}
			}
		}

		//draw bubble
		for(uint32_t idx_y = 0; idx_y<IMG_BUBBLE_HEIGHT; idx_y++){
			for(uint32_t idx_x = 0; idx_x<IMG_BUBBLE_WIDTH; idx_x++){
				if(0x0f == bubble_1[idx_y][idx_x]){
					UG_DrawPixel(108+idx_x, 60+idx_y, C_WHITE);
				}
			}
		}


	}else{

		//draw nyan
		for(uint32_t idx_y = 0; idx_y<IMG_NYAN_HEIGHT; idx_y++){
			for(uint32_t idx_x = 0; idx_x<IMG_NYAN_WIDTH; idx_x++){
				UG_DrawPixel(58+idx_x, 62+idx_y, nyan_2[idx_y][idx_x]);
			}
		}

		//draw tail
		for(uint32_t idx_y = 0; idx_y<IMG_TAIL_HEIGHT; idx_y++){
			for(uint32_t idx_x = 0; idx_x<IMG_TAIL_WIDTH; idx_x++){
				UG_DrawPixel(27+idx_x, 63+idx_y, tail_2[idx_y][idx_x]);
			}
		}

		//draw layout
		for(uint32_t idx_y = 0; idx_y<IMG_LAYOUT_HEIGHT; idx_y++){
			for(uint32_t idx_x = 0; idx_x<IMG_LAYOUT_WIDTH; idx_x++){
				if(0x0f == layout_2[idx_y][idx_x]){
					UG_DrawPixel(idx_x, idx_y, C_WHITE);
				}
			}
		}

		//draw bubble
		for(uint32_t idx_y = 0; idx_y<IMG_BUBBLE_HEIGHT; idx_y++){
			for(uint32_t idx_x = 0; idx_x<IMG_BUBBLE_WIDTH; idx_x++){
				if(0x0f == bubble_2[idx_y][idx_x]){
					UG_DrawPixel(108+idx_x, 60+idx_y, C_WHITE);
				}
			}
		}

	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM6)	//1s timer
	{
		static uint32_t timer_pll_counter = 0;

		if(timer_pll_counter>=10){
			timer_pll_counter = 0;
			is_timer_countup = true;
		}else{
			timer_pll_counter++;
		}

	}
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){

	if(hspi->Instance == SPI3)
	{
		displaySemaphoreClear();
	}
}

typedef void (*pFunction)(void);
volatile uint32_t JumpAddress;
volatile pFunction Jump_To_Application;

void jumpUserApp()
{

	JumpAddress = *(uint32_t *) (FW_APP_ADDR + 4);
	Jump_To_Application = (pFunction) JumpAddress;
	HAL_RCC_DeInit();
	HAL_DeInit();
	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL = 0;
	__set_MSP(*(uint32_t*)FW_APP_ADDR);
	Jump_To_Application();

}

void pset(UG_S16 x, UG_S16 y, UG_COLOR col)
{
	//1dot to 2x2
	displayDrawDot16b(x*2, y*2, col);
	displayDrawDot16b(x*2+1, y*2, col);
	displayDrawDot16b(x*2, y*2+1, col);
	displayDrawDot16b(x*2+1, y*2+1, col);
}


void checkCuresynthVersion()
{
    volatile bool is_curesynth_ver_ok = false;
  	for(uint32_t i=0; i<CURESYNTH_VER_STRINGTH_MAX; i++){
  		if('\0' == ((char *)((uint32_t *)FW_VER_STRING_ADDR))[i]){
  			is_curesynth_ver_ok = true;
  			break;
  		}
  	}
  	if(is_curesynth_ver_ok){
  	    strcpy(curesynth_ver, (char *)((uint32_t *)FW_VER_STRING_ADDR));
  	}else{
  	    sprintf(curesynth_ver, "Error");
  	}
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

	  /* Enable the MPU */
	  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  MX_GPIO_Init();
  //bootloader judge
  bool boot_mode_is_active = false;
  for(uint32_t i=0; i<5; i++){
  	if(	GPIO_PIN_SET == HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_1) )
  	{
  		boot_mode_is_active |= true;
  	}
  	HAL_Delay(20);
  }
  if(!boot_mode_is_active){
	  jumpUserApp();
  }

  MPU_Config_WriteThrough();

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_DMA_Init();
  MX_SPI3_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
  MX_I2C4_Init();
  /* USER CODE BEGIN 2 */

  	//check and copy curesynth version
    checkCuresynthVersion();


	//Display Initialization
	displayInit();

	//Init ugui
	UG_Init(&gui, pset, VRAM_X_SIZE/2, VRAM_Y_SIZE/2);
	UG_SetBackcolor(COL_BACKGROUND_565);
	UG_SetForecolor(COL_FOREGROUND_565);
	UG_FillScreen(COL_BACKGROUND_565);
	UG_FontSelect(&FONT_7X12);

	displayUpdate();
	displayWaitDispalyTransfer();

	//Backlight on
	dispBacklightOn();

	//turn on Analog 3.3V Power
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);

	MX_USB_DEVICE_Init();

	//audio
	wmInit();

	//start 1s timer
	HAL_TIM_Base_Start_IT(&htim6);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */


	  if(is_timer_countup){
		  is_timer_countup = false;

		  UG_FillScreen(COL_BACKGROUND_565);

		  switch(dispdrawstate){
			  case DRAW_A:

				  UG_FontSelect(&FONT_6X8);
				  cureImageDraw(0);
				  UG_SetForecolor(C_CYAN);
				  sprintf(ug_str_buf, "BOOT: %s", BOOTLOADER_VER_STRING);
				  UG_PutString(28,  12,  ug_str_buf);
				  sprintf(ug_str_buf, " APP: %s", curesynth_ver);
				  UG_PutString(28,  24,  ug_str_buf);
				  UG_SetForecolor(COL_FOREGROUND_565);
				  UG_FontSelect(&FONT_7X12);


				  if(is_error_occurred){
					  sprintf(ug_str_buf, "Write ERROR!!");
					  UG_PutString(16,  38,  ug_str_buf);
				  }else{
					  switch(dispstate){
					  case DFUDISP_INIT:
					  case DFUDISP_READ:
					  case DFUDISP_UNLOCK:
					  case DFUDISP_LOCK:
						  sprintf(ug_str_buf, "FW Update Mode");
						  UG_PutString(16,  38,  ug_str_buf);
						  checkCuresynthVersion();
						  break;
					  case DFUDISP_WRITE:
						  sprintf(ug_str_buf, "Writting");
						  UG_PutString(30,  38,  ug_str_buf);
						  break;
						  break;
					  case DFUDISP_ERASE:
						  sprintf(ug_str_buf, "Erasing");
						  UG_PutString(30,  38,  ug_str_buf);
						  break;
						  break;
					  default:
						  break;

					  }
				  }

				  dispdrawstate = DRAW_B;
				  break;

			  case DRAW_B:

				  UG_FontSelect(&FONT_6X8);
				  cureImageDraw(1);
				  UG_SetForecolor(C_CYAN);
				  sprintf(ug_str_buf, "BOOT: %s", BOOTLOADER_VER_STRING);
				  UG_PutString(28,  12,  ug_str_buf);
				  sprintf(ug_str_buf, " APP: %s", curesynth_ver);
				  UG_PutString(28,  24,  ug_str_buf);
				  UG_SetForecolor(COL_FOREGROUND_565);
				  UG_FontSelect(&FONT_7X12);

				  if(is_error_occurred){
					  sprintf(ug_str_buf, "Write ERROR!!");
					  UG_PutString(16,  38,  ug_str_buf);
				  }else{
					  switch(dispstate){
					  case DFUDISP_INIT:
					  case DFUDISP_READ:
					  case DFUDISP_UNLOCK:
					  case DFUDISP_LOCK:
						  sprintf(ug_str_buf, "FW Update Mode");
						  UG_PutString(16,  38,  ug_str_buf);
						  break;
					  case DFUDISP_WRITE:
						  sprintf(ug_str_buf, "Writting...");
						  UG_PutString(30,  38,  ug_str_buf);
						  break;
					  case DFUDISP_ERASE:
						  sprintf(ug_str_buf, "Erasing...");
						  UG_PutString(30,  38,  ug_str_buf);
						  break;
					  default:
						  break;

					  }
				  }
				  dispdrawstate = DRAW_A;

				  break;

			  default:
				  break;
		  }
		  displayUpdate();
	  }


	  //usb enumeration
	  if(USBD_STATE_CONFIGURED != hUsbDeviceHS.dev_state){
		  checkCuresynthVersion();
	  }

	  //err
	  if(is_error_occurred){
		  MX_USB_DEVICE_Stop();
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Macro to configure the PLL clock source
  */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 179;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV2;
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
  hi2c4.Init.Timing = 0x1080A7FF;
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
  htim6.Init.Period = 12424;
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
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

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
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_9|GPIO_PIN_10, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin : PF1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : PA2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PF14 */
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PG9 PG10 */
  GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : PG11 PG12 */
  GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF4_USART10;
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
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
