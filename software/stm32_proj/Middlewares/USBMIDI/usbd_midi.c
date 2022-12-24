/**
  ******************************************************************************
  * @file    usbd_midi.c
  ******************************************************************************

    (CC at)2016 by D.F.Mac. @TripArts Music

*/ 

/* Includes ------------------------------------------------------------------*/
#include "usbd_midi.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"

static uint8_t  USBD_MIDI_Init (USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_MIDI_DeInit (USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_MIDI_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_MIDI_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  *USBD_MIDI_GetCfgDesc (uint16_t *length);
//uint8_t  *USBD_MIDI_GetDeviceQualifierDescriptor (uint16_t *length);
USBD_HandleTypeDef *pInstance = NULL; 

uint32_t APP_Rx_ptr_in  = 0;
uint32_t APP_Rx_ptr_out = 0;
uint32_t APP_Rx_length  = 0;
uint8_t  USB_Tx_State = 0;

__ALIGN_BEGIN uint8_t USB_Rx_Buffer[MIDI_DATA_OUT_PACKET_SIZE] __ALIGN_END ;
__ALIGN_BEGIN uint8_t APP_Rx_Buffer[APP_RX_DATA_SIZE] __ALIGN_END ;

/* USB Standard Device Descriptor */
/*
__ALIGN_BEGIN static uint8_t USBD_MIDI_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};
*/

/* USB MIDI interface class callbacks structure */
USBD_ClassTypeDef  USBD_MIDI = 
{
  USBD_MIDI_Init,
  USBD_MIDI_DeInit,
  NULL,
  NULL,
  NULL,
  USBD_MIDI_DataIn,
  USBD_MIDI_DataOut,
  NULL,
  NULL,
  NULL,
  USBD_MIDI_GetCfgDesc,// HS
  USBD_MIDI_GetCfgDesc,// FS
  NULL,// OTHER SPEED
  NULL,// DEVICE_QUALIFIER
};

/* USB MIDI device Configuration Descriptor */
__ALIGN_BEGIN uint8_t USBD_MIDI_CfgDesc[USB_MIDI_CONFIG_DESC_SIZ] __ALIGN_END =
{
  // configuration descriptor
  0x09, 0x02, ((69 + 13*MIDI_IN_JACK_NUM + 19*MIDI_OUT_JACK_NUM-3-13) & 0xFF)/*0x45 total size of cdesc*/, ((69 + 13*MIDI_IN_JACK_NUM + 19*MIDI_OUT_JACK_NUM-3-13) & 0xFF00) >> 8, 0x02, 0x01, 0x00, 0x80, 0x7D,

  // The Audio Interface Collection
  0x09, 0x04, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, // Standard AC Interface Descriptor
  0x09, 0x24, 0x01, 0x00, 0x01, 0x09, 0x00, 0x01, 0x01, // Class-specific AC Interface Descriptor
  0x09, 0x04, 0x01, 0x00, 0x01/*endpoint number*/, 0x01, 0x03, 0x00, 0x00, // MIDIStreaming Interface Descriptors
  0x07, 0x24, 0x01, 0x00, 0x01, 0x07 + 12*MIDI_IN_JACK_NUM + 18*MIDI_OUT_JACK_NUM-3/**/, 0x00,   // Class-Specific MS Interface Header Descriptor

	// MIDI IN JACKS
	0x06, 0x24, 0x02, 0x01, 0x01, 0x00,//MIDI-IN 1 (embedded)
//	0x06, 0x24, 0x02, 0x02, 0x02, 0x00,//MIDI-IN 1 (external)
//
//	0x06, 0x24, 0x02, 0x01, 0x11, 0x00,//MIDI-IN 2 (embedded)
//	0x06, 0x24, 0x02, 0x02, 0x12, 0x00,//MIDI-IN 2 (external)
//
//	0x06, 0x24, 0x02, 0x01, 0x21, 0x00,//MIDI-IN 3 (embedded)
//	0x06, 0x24, 0x02, 0x02, 0x22, 0x00,//MIDI-IN 3 (external)
//
//	0x06, 0x24, 0x02, 0x01, 0x31, 0x00,//MIDI-IN 4 (embedded)
//	0x06, 0x24, 0x02, 0x02, 0x32, 0x00,//MIDI-IN 4 (external)
//
//	0x06, 0x24, 0x02, 0x01, 0x41, 0x00,//MIDI-IN 5 (embedded)
//	0x06, 0x24, 0x02, 0x02, 0x42, 0x00,//MIDI-IN 5 (external)
//
//	0x06, 0x24, 0x02, 0x01, 0x51, 0x00,//MIDI-IN 6 (embedded)
//	0x06, 0x24, 0x02, 0x02, 0x52, 0x00,//MIDI-IN 6 (external)
//
//	0x06, 0x24, 0x02, 0x01, 0x61, 0x00,//MIDI-IN 7 (embedded)
//	0x06, 0x24, 0x02, 0x02, 0x62, 0x00,//MIDI-IN 7 (external)
//
//	0x06, 0x24, 0x02, 0x01, 0x71, 0x00,//MIDI-IN 8 (embedded)
//	0x06, 0x24, 0x02, 0x02, 0x72, 0x00,//MIDI-IN 8 (external)

	// MIDI OUT JACKS
//	0x09, 0x24, 0x03, 0x01, 0x03, 0x01, 0x02, 0x01, 0x00,//MIDI-OUT 1 (embedded)
	0x09, 0x24, 0x03, 0x02, 0x04, 0x01, 0x01, 0x01, 0x00,//MIDI-OUT 1 (external)

//	0x09, 0x24, 0x03, 0x01, 0x13, 0x01, 0x12, 0x01, 0x00,//MIDI-OUT 2 (embedded)
//	0x09, 0x24, 0x03, 0x02, 0x14, 0x01, 0x11, 0x01, 0x00,//MIDI-OUT 2 (external)
//
//	0x09, 0x24, 0x03, 0x01, 0x23, 0x01, 0x22, 0x01, 0x00,//MIDI-OUT 3 (embedded)
//	0x09, 0x24, 0x03, 0x02, 0x24, 0x01, 0x21, 0x01, 0x00,//MIDI-OUT 3 (external)
//
//	0x09, 0x24, 0x03, 0x01, 0x33, 0x01, 0x32, 0x01, 0x00,//MIDI-OUT 4 (embedded)
//	0x09, 0x24, 0x03, 0x02, 0x34, 0x01, 0x31, 0x01, 0x00,//MIDI-OUT 4 (external)
//
//	0x09, 0x24, 0x03, 0x01, 0x43, 0x01, 0x42, 0x01, 0x00,//MIDI-OUT 5 (embedded)
//	0x09, 0x24, 0x03, 0x02, 0x44, 0x01, 0x41, 0x01, 0x00,//MIDI-OUT 5 (external)
//
//	0x09, 0x24, 0x03, 0x01, 0x53, 0x01, 0x52, 0x01, 0x00,//MIDI-OUT 6 (embedded)
//	0x09, 0x24, 0x03, 0x02, 0x54, 0x01, 0x51, 0x01, 0x00,//MIDI-OUT 6 (external)
//
//	0x09, 0x24, 0x03, 0x01, 0x63, 0x01, 0x62, 0x01, 0x00,//MIDI-OUT 7 (embedded)
//	0x09, 0x24, 0x03, 0x02, 0x64, 0x01, 0x61, 0x01, 0x00,//MIDI-OUT 7 (external)
//
//	0x09, 0x24, 0x03, 0x01, 0x73, 0x01, 0x72, 0x01, 0x00,//MIDI-OUT 8 (embedded)
//	0x09, 0x24, 0x03, 0x02, 0x74, 0x01, 0x71, 0x01, 0x00,//MIDI-OUT 8 (external)

	// OUT endpoint descriptor
	0x09, 0x05, MIDI_OUT_EP, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00,
	0x04+MIDI_OUT_JACK_NUM/**/, 0x25, 0x01, MIDI_OUT_JACK_NUM/**/, 0x01, /*0x11, 0x21, 0x31, 0x41, 0x51, 0x61, 0x71,*/

	// IN endpoint descriptor
//	0x09, 0x05, MIDI_IN_EP, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00,
//	0x04+MIDI_IN_JACK_NUM/**/, 0x25, 0x01, MIDI_IN_JACK_NUM/**/, 0x03, 0x13, 0x23, 0x33, 0x43, 0x53, 0x63, 0x73,
};

static uint8_t USBD_MIDI_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx){
  pInstance = pdev;
  USBD_LL_OpenEP(pdev,MIDI_IN_EP,USBD_EP_TYPE_BULK,MIDI_DATA_IN_PACKET_SIZE);
  USBD_LL_OpenEP(pdev,MIDI_OUT_EP,USBD_EP_TYPE_BULK,MIDI_DATA_OUT_PACKET_SIZE);
  USBD_LL_PrepareReceive(pdev,MIDI_OUT_EP,(uint8_t*)(USB_Rx_Buffer),MIDI_DATA_OUT_PACKET_SIZE);
  return 0;
}

static uint8_t USBD_MIDI_DeInit (USBD_HandleTypeDef *pdev, uint8_t cfgidx){
  pInstance = NULL;
  USBD_LL_CloseEP(pdev,MIDI_IN_EP);
  USBD_LL_CloseEP(pdev,MIDI_OUT_EP);
  return 0;
}

static uint8_t USBD_MIDI_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum){

  if (USB_Tx_State == 1){
    USB_Tx_State = 0;
  }
  return USBD_OK;
}

static uint8_t  USBD_MIDI_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum)
{      
  uint16_t USB_Rx_Cnt;

  USBD_MIDI_ItfTypeDef *pmidi;
  pmidi = (USBD_MIDI_ItfTypeDef *)(pdev->pUserData[0]);

  USB_Rx_Cnt = ((PCD_HandleTypeDef*)pdev->pData)->OUT_ep[epnum].xfer_count;

  pmidi->pIf_MidiRx((uint8_t *)&USB_Rx_Buffer, USB_Rx_Cnt);

  USBD_LL_PrepareReceive(pdev,MIDI_OUT_EP,(uint8_t*)(USB_Rx_Buffer),MIDI_DATA_OUT_PACKET_SIZE);
  return USBD_OK;
}

void USBD_MIDI_SendPacket (){
  uint16_t USB_Tx_ptr;
  uint16_t USB_Tx_length;

  if(USB_Tx_State != 1){
    if (APP_Rx_ptr_out == APP_RX_DATA_SIZE){
      APP_Rx_ptr_out = 0;
    }

    if(APP_Rx_ptr_out == APP_Rx_ptr_in){
      USB_Tx_State = 0;
      return;
    }

    if(APP_Rx_ptr_out > APP_Rx_ptr_in){
      APP_Rx_length = APP_RX_DATA_SIZE - APP_Rx_ptr_out;
    }else{
      APP_Rx_length = APP_Rx_ptr_in - APP_Rx_ptr_out;
    }

    if (APP_Rx_length > MIDI_DATA_IN_PACKET_SIZE){
      USB_Tx_ptr = APP_Rx_ptr_out;
      USB_Tx_length = MIDI_DATA_IN_PACKET_SIZE;
      APP_Rx_ptr_out += MIDI_DATA_IN_PACKET_SIZE;
      APP_Rx_length -= MIDI_DATA_IN_PACKET_SIZE;
    }else{
      USB_Tx_ptr = APP_Rx_ptr_out;
      USB_Tx_length = APP_Rx_length;
      APP_Rx_ptr_out += APP_Rx_length;
      APP_Rx_length = 0;
    }
    USB_Tx_State = 1;
    USBD_LL_Transmit (pInstance, MIDI_IN_EP,(uint8_t*)&APP_Rx_Buffer[USB_Tx_ptr],USB_Tx_length);
  }
}

static uint8_t *USBD_MIDI_GetCfgDesc (uint16_t *length){
  *length = sizeof (USBD_MIDI_CfgDesc);
  return USBD_MIDI_CfgDesc;
}


uint8_t USBD_MIDI_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_MIDI_ItfTypeDef *fops)
{
  uint8_t ret = USBD_FAIL;
  
  if(fops != NULL){
    pdev->pUserData[0]= fops;
    ret = USBD_OK;    
  }
  
  return ret;
}
