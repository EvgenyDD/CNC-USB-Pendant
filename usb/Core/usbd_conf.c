#include "main.h"
#include "platform.h"
#include "usbd_core.h"
#include "usbd_pend_hid.h"

extern PCD_HandleTypeDef hpcd_USB_DRD_FS;

static USBD_StatusTypeDef USBD_Get_USB_Status(HAL_StatusTypeDef hal_status);

void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
{
	USBD_LL_SetupStage((USBD_HandleTypeDef *)hpcd->pData, (uint8_t *)hpcd->Setup);
}

void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
	USBD_LL_DataOutStage((USBD_HandleTypeDef *)hpcd->pData, epnum, hpcd->OUT_ep[epnum].xfer_buff);
}

void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
	USBD_LL_DataInStage((USBD_HandleTypeDef *)hpcd->pData, epnum, hpcd->IN_ep[epnum].xfer_buff);
}

void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
{
	USBD_LL_SOF((USBD_HandleTypeDef *)hpcd->pData);
}

void HAL_PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
{
	USBD_SpeedTypeDef speed = USBD_SPEED_FULL;
	if(hpcd->Init.speed != PCD_SPEED_FULL)
	{
		Error_Handler();
	}

	USBD_LL_SetSpeed((USBD_HandleTypeDef *)hpcd->pData, speed);
	USBD_LL_Reset((USBD_HandleTypeDef *)hpcd->pData);
}

void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
{
	USBD_LL_DevConnected((USBD_HandleTypeDef *)hpcd->pData);
}

void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
{
	USBD_LL_DevDisconnected((USBD_HandleTypeDef *)hpcd->pData);
}

USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef *pdev)
{
	pdev->pData = &hpcd_USB_DRD_FS;
	HAL_PCDEx_PMAConfig((PCD_HandleTypeDef *)pdev->pData, 0x00, PCD_SNG_BUF, 0x18);
	HAL_PCDEx_PMAConfig((PCD_HandleTypeDef *)pdev->pData, 0x80, PCD_SNG_BUF, 0x58);
	HAL_PCDEx_PMAConfig((PCD_HandleTypeDef *)pdev->pData, USBD_PEND_HID_EPIN_ADDR, PCD_SNG_BUF, 0xC0);
	HAL_PCDEx_PMAConfig((PCD_HandleTypeDef *)pdev->pData, USBD_PEND_HID_EPOUT_ADDR, PCD_SNG_BUF, 0x110);
	return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef *pdev)
{
	HAL_StatusTypeDef hal_status = HAL_PCD_DeInit(pdev->pData);
	return USBD_Get_USB_Status(hal_status);
}

USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef *pdev)
{
	HAL_StatusTypeDef hal_status = HAL_PCD_Start(pdev->pData);
	return USBD_Get_USB_Status(hal_status);
}

USBD_StatusTypeDef USBD_LL_Stop(USBD_HandleTypeDef *pdev)
{
	HAL_StatusTypeDef hal_status = HAL_PCD_Stop(pdev->pData);
	return USBD_Get_USB_Status(hal_status);
}

USBD_StatusTypeDef USBD_LL_OpenEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t ep_type, uint16_t ep_mps)
{
	HAL_StatusTypeDef hal_status = HAL_PCD_EP_Open(pdev->pData, ep_addr, ep_mps, ep_type);
	return USBD_Get_USB_Status(hal_status);
}

USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
	HAL_StatusTypeDef hal_status = HAL_PCD_EP_Close(pdev->pData, ep_addr);
	return USBD_Get_USB_Status(hal_status);
}

USBD_StatusTypeDef USBD_LL_FlushEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
	HAL_StatusTypeDef hal_status = HAL_PCD_EP_Flush(pdev->pData, ep_addr);
	return USBD_Get_USB_Status(hal_status);
}

USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
	HAL_StatusTypeDef hal_status = HAL_PCD_EP_SetStall(pdev->pData, ep_addr);
	return USBD_Get_USB_Status(hal_status);
}

USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
	HAL_StatusTypeDef hal_status = HAL_PCD_EP_ClrStall(pdev->pData, ep_addr);
	return USBD_Get_USB_Status(hal_status);
}

uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
	PCD_HandleTypeDef *hpcd = (PCD_HandleTypeDef *)pdev->pData;
	if((ep_addr & 0x80) == 0x80)
	{
		return hpcd->IN_ep[ep_addr & 0x7F].is_stall;
	}
	else
	{
		return hpcd->OUT_ep[ep_addr & 0x7F].is_stall;
	}
}

USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef *pdev, uint8_t dev_addr)
{
	HAL_StatusTypeDef hal_status = HAL_PCD_SetAddress(pdev->pData, dev_addr);
	return USBD_Get_USB_Status(hal_status);
}

USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint32_t size)
{
	HAL_StatusTypeDef hal_status = HAL_PCD_EP_Transmit(pdev->pData, ep_addr, pbuf, size);
	return USBD_Get_USB_Status(hal_status);
}

USBD_StatusTypeDef USBD_LL_PrepareReceive(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint32_t size)
{
	HAL_StatusTypeDef hal_status = HAL_PCD_EP_Receive(pdev->pData, ep_addr, pbuf, size);
	return USBD_Get_USB_Status(hal_status);
}

uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
	return HAL_PCD_EP_GetRxCount((PCD_HandleTypeDef *)pdev->pData, ep_addr);
}

USBD_StatusTypeDef USBD_Get_USB_Status(HAL_StatusTypeDef hal_status)
{
	switch(hal_status)
	{
	case HAL_OK: return USBD_OK;
	case HAL_ERROR: return USBD_FAIL;
	case HAL_BUSY: return USBD_BUSY;
	case HAL_TIMEOUT: return USBD_FAIL;
	default: return USBD_FAIL;
	}
}
