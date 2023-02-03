/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_dfu_if.c
  * @brief          : Usb device for Download Firmware Update.
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
#include "usbd_dfu_if.h"

/* USER CODE BEGIN INCLUDE */
volatile uint32_t counter_erase = 0;
volatile uint32_t counter_write = 0;

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

#define FLASH_ERASE_TIME    (uint16_t)1500
#define FLASH_PROGRAM_TIME  (uint16_t)50


DfuDispState dispstate = DFUDISP_INIT;
volatile bool is_error_occurred = false;

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device.
  * @{
  */

/** @defgroup USBD_DFU
  * @brief Usb DFU device module.
  * @{
  */

/** @defgroup USBD_DFU_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_DFU_Private_Defines
  * @brief Private defines.
  * @{
  */

#define FLASH_DESC_STR      "@Internal Flash   /0x08000000/01*0128Ka,01*0128Kg,01*0128Kg,01*0128Kg,01*0128Kg,01*0128Kg,01*0128Kg,01*0128Kg"

/* USER CODE BEGIN PRIVATE_DEFINES */

/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_DFU_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_DFU_Private_Variables
  * @brief Private variables.
  * @{
  */

/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_DFU_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceHS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_DFU_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static uint16_t MEM_If_Init_HS(void);
static uint16_t MEM_If_Erase_HS(uint32_t Add);
static uint16_t MEM_If_Write_HS(uint8_t *src, uint8_t *dest, uint32_t Len);
static uint8_t *MEM_If_Read_HS(uint8_t *src, uint8_t *dest, uint32_t Len);
static uint16_t MEM_If_DeInit_HS(void);
static uint16_t MEM_If_GetStatus_HS(uint32_t Add, uint8_t Cmd, uint8_t *buffer);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

#if defined ( __ICCARM__ ) /* IAR Compiler */
  #pragma data_alignment=4
#endif

__ALIGN_BEGIN USBD_DFU_MediaTypeDef USBD_DFU_fops_HS __ALIGN_END =
{
    (uint8_t*)FLASH_DESC_STR,
    MEM_If_Init_HS,
    MEM_If_DeInit_HS,
    MEM_If_Erase_HS,
    MEM_If_Write_HS,
    MEM_If_Read_HS,
    MEM_If_GetStatus_HS
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Memory initialization routine.
  * @retval USBD_OK if operation is successful, MAL_FAIL else.
  */
uint16_t MEM_If_Init_HS(void)
{
  /* USER CODE BEGIN 6 */
	  HAL_FLASH_Unlock();
	  dispstate = DFUDISP_UNLOCK;
	  return (USBD_OK);
  /* USER CODE END 6 */
}

/**
  * @brief  De-Initializes Memory.
  * @retval USBD_OK if operation is successful, MAL_FAIL else.
  */
uint16_t MEM_If_DeInit_HS(void)
{
  /* USER CODE BEGIN 7 */
	 HAL_FLASH_Lock();
	  dispstate = DFUDISP_LOCK;
	 return (USBD_OK);
  /* USER CODE END 7 */
}

/**
  * @brief  Erase sector.
  * @param  Add: Address of sector to be erased.
  * @retval USBD_OK if operation is successful, MAL_FAIL else.
  */
uint16_t MEM_If_Erase_HS(uint32_t Add)
{
  /* USER CODE BEGIN 8 */

	counter_erase++;

	  FLASH_EraseInitTypeDef erase;
	  uint32_t error = 0;
	  erase.TypeErase = FLASH_TYPEERASE_SECTORS;
	  erase.Sector = convertAddressToSector(Add);

	  if(255 == erase.Sector){//If sector is out of boundary
		  is_error_occurred = true;
		  return (USBD_FAIL);
	  }
	  erase.Banks = FLASH_BANK_1;
	  erase.NbSectors = 1;
	  erase.VoltageRange = FLASH_VOLTAGE_RANGE_4;

	  HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase, &error);

	  if(HAL_OK != status || 0xFFFFFFFF != error){
		  return (USBD_BUSY);
	  }
	  dispstate = DFUDISP_ERASE;
	  return (USBD_OK);
  /* USER CODE END 8 */
}

/**
  * @brief  Memory write routine.
  * @param  src: Pointer to the source buffer. Address to be written to.
  * @param  dest: Pointer to the destination buffer.
  * @param  Len: Number of data to be written (in bytes).
  * @retval USBD_OK if operation is successful, MAL_FAIL else.
  */
uint16_t MEM_If_Write_HS(uint8_t *src, uint8_t *dest, uint32_t Len)
{
  /* USER CODE BEGIN 9 */
	HAL_StatusTypeDef status;
	uint32_t dest_addr = (uint32_t)dest;

	//flash-word row is 256bit(32 * 8 bit) in STM32H723
	for(uint32_t i=0; i<Len; i+=32)
	{
		status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, dest_addr+i , (uint32_t)(&src[i]) );

		if(HAL_OK == status){
			for(uint32_t idx_check=0; idx_check<8; idx_check++){//check 32bit * 8time
				 if( *(uint32_t *)(src+i+idx_check) != *(uint32_t *)(dest_addr+i+idx_check) ){
					 is_error_occurred = true;
					 return USBD_EMEM;
				 }
			}

		}else{
			return USBD_BUSY;
		}
	}
	  dispstate = DFUDISP_WRITE;
	return (USBD_OK);
  /* USER CODE END 9 */
}

/**
  * @brief  Memory read routine.
  * @param  src: Pointer to the source buffer. Address to be written to.
  * @param  dest: Pointer to the destination buffer.
  * @param  Len: Number of data to be read (in bytes).
  * @retval Pointer to the physical address where data should be read.
  */
uint8_t *MEM_If_Read_HS(uint8_t *src, uint8_t *dest, uint32_t Len)
{
  /* Return a valid address to avoid HardFault */
  /* USER CODE BEGIN 10 */

	 uint8_t *src_ptr = src;
	 for(uint32_t i=0; i<Len; i++)
	 {
		 dest[i] = *src_ptr;
		 src_ptr++;
	 }

	 dispstate = DFUDISP_READ;
	 return (uint8_t *)(dest);
  /* USER CODE END 10 */
}

/**
  * @brief  Get status routine.
  * @param  Add: Address to be read from.
  * @param  Cmd: Number of data to be read (in bytes).
  * @param  buffer: used for returning the time necessary for a program or an erase operation
  * @retval 0 if operation is successful
  */
uint16_t MEM_If_GetStatus_HS(uint32_t Add, uint8_t Cmd, uint8_t *buffer)
{
  /* USER CODE BEGIN 11 */
	  switch(Cmd)
	  {
	  case DFU_MEDIA_PROGRAM:
	    buffer[1] = (uint8_t)FLASH_PROGRAM_TIME;
	    buffer[2] = (uint8_t)(FLASH_PROGRAM_TIME << 8);
	    buffer[3] = 0;
	    break;

	  case DFU_MEDIA_ERASE:
	    buffer[1] = (uint8_t)FLASH_ERASE_TIME;
	    buffer[2] = (uint8_t)(FLASH_ERASE_TIME << 8);
	    buffer[3] = 0;
	    break;

	  default:
		  break;
	  }
	  return  (USBD_OK);
  /* USER CODE END 11 */
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */

