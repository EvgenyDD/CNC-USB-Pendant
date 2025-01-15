/**
 ******************************************************************************
 * @file    usbd_core.c
 * @author  MCD Application Team
 * @brief   This file provides all the USBD core functions.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2015 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "usbd_core.h"

/** @addtogroup STM32_USBD_DEVICE_LIBRARY
 * @{
 */

/** @defgroup USBD_CORE
 * @brief usbd core module
 * @{
 */

/** @defgroup USBD_CORE_Private_TypesDefinitions
 * @{
 */

/**
 * @}
 */

/** @defgroup USBD_CORE_Private_Defines
 * @{
 */

/**
 * @}
 */

/** @defgroup USBD_CORE_Private_Macros
 * @{
 */

/**
 * @}
 */

/** @defgroup USBD_CORE_Private_FunctionPrototypes
 * @{
 */

/**
 * @}
 */

/** @defgroup USBD_CORE_Private_Variables
 * @{
 */

/**
 * @}
 */

/** @defgroup USBD_CORE_Private_Functions
 * @{
 */

/**
 * @brief  USBD_Init
 *         Initializes the device stack and load the class driver
 * @param  pdev: device instance
 * @param  pdesc: Descriptor structure address
 * @param  id: Low level core index
 * @retval None
 */
USBD_StatusTypeDef USBD_Init(USBD_HandleTypeDef *pdev,
							 USBD_DescriptorsTypeDef *pdesc, uint8_t id)
{
	USBD_StatusTypeDef ret;

	/* Check whether the USB Host handle is valid */
	if(pdev == NULL)
	{
#if(USBD_DEBUG_LEVEL > 1U)
		USBD_ErrLog("Invalid Device handle");
#endif /* (USBD_DEBUG_LEVEL > 1U) */
		return USBD_FAIL;
	}

	/* Unlink previous class*/
	pdev->pClass = NULL;
	pdev->pConfDesc = NULL;

	/* Assign USBD Descriptors */
	if(pdesc != NULL) pdev->pDesc = pdesc;

	/* Set Device initial State */
	pdev->dev_state = USBD_STATE_DEFAULT;
	pdev->id = id;

	/* Initialize low level driver */
	ret = USBD_LL_Init(pdev);

	return ret;
}

/**
 * @brief  USBD_DeInit
 *         Re-Initialize the device library
 * @param  pdev: device instance
 * @retval status: status
 */
USBD_StatusTypeDef USBD_DeInit(USBD_HandleTypeDef *pdev)
{
	USBD_StatusTypeDef ret;

	/* Disconnect the USB Device */
	USBD_LL_Stop(pdev);

	/* Set Default State */
	pdev->dev_state = USBD_STATE_DEFAULT;

	/* Free Class Resources */
	if(pdev->pClass != NULL)
	{
		pdev->pClass->DeInit(pdev, (uint8_t)pdev->dev_config);
	}

	/* Free Device descriptors resources */
	pdev->pDesc = NULL;
	pdev->pConfDesc = NULL;

	/* DeInitialize low level driver */
	ret = USBD_LL_DeInit(pdev);

	return ret;
}

/**
 * @brief  USBD_RegisterClass
 *         Link class driver to Device Core.
 * @param  pDevice : Device Handle
 * @param  pclass: Class handle
 * @retval USBD Status
 */
USBD_StatusTypeDef USBD_RegisterClass(USBD_HandleTypeDef *pdev, USBD_ClassTypeDef *pclass)
{
	uint16_t len = 0U;

	if(pclass == NULL)
	{
#if(USBD_DEBUG_LEVEL > 1U)
		USBD_ErrLog("Invalid Class handle");
#endif /* (USBD_DEBUG_LEVEL > 1U) */
		return USBD_FAIL;
	}

	/* link the class to the USB Device handle */
	pdev->pClass = pclass;

	/* Get Device Configuration Descriptor */
#ifdef USE_USB_HS
	pdev->pConfDesc = (void *)pdev->pClass->GetHSConfigDescriptor(&len);
#else
	pdev->pConfDesc = (void *)pdev->pClass->GetFSConfigDescriptor(&len);
#endif

	return USBD_OK;
}

/**
 * @brief  USBD_Start
 *         Start the USB Device Core.
 * @param  pdev: Device Handle
 * @retval USBD Status
 */
USBD_StatusTypeDef USBD_Start(USBD_HandleTypeDef *pdev)
{
	pdev->last_idx_class = 0U;
	return USBD_LL_Start(pdev);
}

/**
 * @brief  USBD_Stop
 *         Stop the USB Device Core.
 * @param  pdev: Device Handle
 * @retval USBD Status
 */
USBD_StatusTypeDef USBD_Stop(USBD_HandleTypeDef *pdev)
{
	/* Disconnect USB Device */
	USBD_LL_Stop(pdev);

	/* Free Class Resources */
	if(pdev->pClass != NULL)
	{
		pdev->pClass->DeInit(pdev, (uint8_t)pdev->dev_config);
	}

	return USBD_OK;
}

/**
 * @brief  USBD_RunTestMode
 *         Launch test mode process
 * @param  pdev: device instance
 * @retval status
 */
USBD_StatusTypeDef USBD_RunTestMode(USBD_HandleTypeDef *pdev)
{
#ifdef USBD_HS_TESTMODE_ENABLE
	USBD_StatusTypeDef ret;
	/* Run USB HS test mode */
	ret = USBD_LL_SetTestMode(pdev, pdev->dev_test_mode);
	return ret;
#else
	return USBD_OK;
#endif
}

/**
 * @brief  USBD_SetClassConfig
 *        Configure device and start the interface
 * @param  pdev: device instance
 * @param  cfgidx: configuration index
 * @retval status
 */
USBD_StatusTypeDef USBD_SetClassConfig(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
	USBD_StatusTypeDef ret = USBD_OK;
	if(pdev->pClass) ret = (USBD_StatusTypeDef)pdev->pClass->Init(pdev, cfgidx);
	return ret;
}

/**
 * @brief  USBD_ClrClassConfig
 *         Clear current configuration
 * @param  pdev: device instance
 * @param  cfgidx: configuration index
 * @retval status: USBD_StatusTypeDef
 */
USBD_StatusTypeDef USBD_ClrClassConfig(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
	return pdev->pClass->DeInit(pdev, cfgidx) != 0U ? USBD_FAIL : USBD_OK;
}

/**
 * @brief  USBD_LL_SetupStage
 *         Handle the setup stage
 * @param  pdev: device instance
 * @retval status
 */
USBD_StatusTypeDef USBD_LL_SetupStage(USBD_HandleTypeDef *pdev, uint8_t *psetup)
{
	USBD_StatusTypeDef ret = USBD_OK;

	USBD_ParseSetupRequest(&pdev->request, psetup);

	pdev->ep0_state = USBD_EP0_SETUP;

	pdev->ep0_data_len = pdev->request.wLength;

	switch(pdev->request.bmRequest & USB_REQ_TYPE_MASK)
	{
	case USB_REQ_TYPE_STANDARD:
	case USB_REQ_TYPE_CLASS:
		switch(pdev->request.bmRequest & 0x1F)
		{
		case USB_REQ_RECIPIENT_DEVICE: ret = USBD_StdDevReq(pdev, &pdev->request); break;
		case USB_REQ_RECIPIENT_INTERFACE: ret = USBD_StdItfReq(pdev, &pdev->request); break;
		case USB_REQ_RECIPIENT_ENDPOINT: ret = USBD_StdEPReq(pdev, &pdev->request); break;
		default: ret = USBD_LL_StallEP(pdev, (pdev->request.bmRequest & 0x80U)); break;
		}
		break;

	case USB_REQ_TYPE_VENDOR: ret = USBD_VendDevReq(pdev, &pdev->request); break;
	default: break;
	}

	return ret;
}

/**
 * @brief  USBD_LL_DataOutStage
 *         Handle data OUT stage
 * @param  pdev: device instance
 * @param  epnum: endpoint index
 * @param  pdata: data pointer
 * @retval status
 */
USBD_StatusTypeDef USBD_LL_DataOutStage(USBD_HandleTypeDef *pdev, uint8_t epnum, uint8_t *pdata)
{
	USBD_EndpointTypeDef *pep;
	USBD_StatusTypeDef ret = USBD_OK;
	uint8_t idx;

	if(epnum == 0U)
	{
		pep = &pdev->ep_out[0];

		if(pdev->ep0_state == USBD_EP0_DATA_OUT)
		{
			if(pep->rem_length > pep->maxpacket)
			{
				pep->rem_length -= pep->maxpacket;
				USBD_CtlContinueRx(pdev, pdata, MIN(pep->rem_length, pep->maxpacket));
			}
			else
			{
				/* Find the class ID relative to the current request */
				switch(pdev->request.bmRequest & 0x1FU)
				{
				case USB_REQ_RECIPIENT_DEVICE:
					/* Device requests must be managed by the first instantiated class
					   (or duplicated by all classes for simplicity) */
					idx = 0U;
					break;

				case USB_REQ_RECIPIENT_INTERFACE:
					idx = LOBYTE(pdev->request.wIndex);
					break;

				case USB_REQ_RECIPIENT_ENDPOINT:
					idx = LOBYTE(pdev->request.wIndex);
					break;

				default:
					/* Back to the first class in case of doubt */
					idx = 0U;
					break;
				}

				int sts = 0;
				if(pdev->pClass->EP0_RxReady != NULL &&
				   pdev->dev_state == USBD_STATE_CONFIGURED)
				{
					pdev->last_idx_class = idx;
					sts = pdev->pClass->EP0_RxReady(pdev, idx);
				}

				if(sts != USBD_POSTPONE)
				{
					if(sts)
					{
						USBD_CtlError(pdev, NULL);
						// USB_OTG_EP0_OutStart(pdev);
						// USBD_CtlReceiveStatus(pdev);
					}
					else
					{
						USBD_CtlSendStatus(pdev);
					}
				}
			}
		}
	}
	else
	{
		/* Get the class index relative to this interface */
		idx = epnum & 0x7FU;

		if((uint16_t)idx != 0xFFU)
		{
			/* Call the class data out function to manage the request */
			if(pdev->dev_state == USBD_STATE_CONFIGURED)
			{
				if(pdev->pClass->DataOut != NULL)
				{
					pdev->last_idx_class = idx;
					ret = (USBD_StatusTypeDef)pdev->pClass->DataOut(pdev, epnum, idx);
				}
			}
			if(ret != USBD_OK)
			{
				return ret;
			}
		}
	}

	return USBD_OK;
}

/**
 * @brief  USBD_LL_DataInStage
 *         Handle data in stage
 * @param  pdev: device instance
 * @param  epnum: endpoint index
 * @retval status
 */
USBD_StatusTypeDef USBD_LL_DataInStage(USBD_HandleTypeDef *pdev, uint8_t epnum, uint8_t *pdata)
{
	USBD_EndpointTypeDef *pep;
	USBD_StatusTypeDef ret;
	uint8_t idx;

	if(epnum == 0U)
	{
		pep = &pdev->ep_in[0];

		if(pdev->ep0_state == USBD_EP0_DATA_IN)
		{
			if(pep->rem_length > pep->maxpacket)
			{
				pep->rem_length -= pep->maxpacket;

				USBD_CtlContinueSendData(pdev, pdata, pep->rem_length);

				/* Prepare endpoint for premature end of transfer */
				USBD_LL_PrepareReceive(pdev, 0U, NULL, 0U);
			}
			else
			{
				/* last packet is MPS multiple, so send ZLP packet */
				if((pep->maxpacket == pep->rem_length) &&
				   (pep->total_length >= pep->maxpacket) &&
				   (pep->total_length < pdev->ep0_data_len))
				{
					USBD_CtlContinueSendData(pdev, NULL, 0U);
					pdev->ep0_data_len = 0U;

					/* Prepare endpoint for premature end of transfer */
					USBD_LL_PrepareReceive(pdev, 0U, NULL, 0U);
				}
				else
				{
					if(pdev->dev_state == USBD_STATE_CONFIGURED)
					{
						if(pdev->pClass->EP0_TxSent != NULL)
						{
							pdev->last_idx_class = 0U;
							pdev->pClass->EP0_TxSent(pdev);
						}
					}
					USBD_LL_StallEP(pdev, 0x80U);
					USBD_CtlReceiveStatus(pdev);
				}
			}
		}

		if(pdev->dev_test_mode != 0U)
		{
			USBD_RunTestMode(pdev);
			pdev->dev_test_mode = 0U;
		}
	}
	else
	{
		idx = (uint8_t)epnum | 0x80U; /* Get the class index relative to this interface */
		if(pdev->dev_state == USBD_STATE_CONFIGURED)
		{
			if(pdev->pClass->DataIn != NULL)
			{
				pdev->last_idx_class = idx;
				return (USBD_StatusTypeDef)pdev->pClass->DataIn(pdev, epnum, idx);
			}
		}
	}

	return USBD_OK;
}

/**
 * @brief  USBD_LL_Reset
 *         Handle Reset event
 * @param  pdev: device instance
 * @retval status
 */

USBD_StatusTypeDef USBD_LL_Reset(USBD_HandleTypeDef *pdev)
{
	USBD_StatusTypeDef ret = USBD_OK;

	/* Upon Reset call user call back */
	pdev->dev_state = USBD_STATE_DEFAULT;
	pdev->ep0_state = USBD_EP0_IDLE;
	pdev->dev_config = 0U;
	pdev->dev_remote_wakeup = 0U;
	pdev->dev_test_mode = 0U;

	if(pdev->pClass != NULL)
	{
		if(pdev->pClass->DeInit(pdev, (uint8_t)pdev->dev_config) != USBD_OK) ret = USBD_FAIL;
	}

	/* Open EP0 OUT */
	USBD_LL_OpenEP(pdev, 0x00U, USBD_EP_TYPE_CTRL, USB_MAX_EP0_SIZE);
	pdev->ep_out[0x00U & 0xFU].is_used = 1U;

	pdev->ep_out[0].maxpacket = USB_MAX_EP0_SIZE;

	/* Open EP0 IN */
	USBD_LL_OpenEP(pdev, 0x80U, USBD_EP_TYPE_CTRL, USB_MAX_EP0_SIZE);
	pdev->ep_in[0x80U & 0xFU].is_used = 1U;

	pdev->ep_in[0].maxpacket = USB_MAX_EP0_SIZE;

	return ret;
}

/**
 * @brief  USBD_LL_SetSpeed
 *         Handle Reset event
 * @param  pdev: device instance
 * @retval status
 */
USBD_StatusTypeDef USBD_LL_SetSpeed(USBD_HandleTypeDef *pdev,
									USBD_SpeedTypeDef speed)
{
	pdev->dev_speed = speed;

	return USBD_OK;
}

/**
 * @brief  USBD_LL_Suspend
 *         Handle Suspend event
 * @param  pdev: device instance
 * @retval status
 */

USBD_StatusTypeDef USBD_LL_Suspend(USBD_HandleTypeDef *pdev)
{
	if(pdev->dev_state != USBD_STATE_SUSPENDED)
	{
		pdev->dev_old_state = pdev->dev_state;
	}

	pdev->dev_state = USBD_STATE_SUSPENDED;

	return USBD_OK;
}

/**
 * @brief  USBD_LL_Resume
 *         Handle Resume event
 * @param  pdev: device instance
 * @retval status
 */

USBD_StatusTypeDef USBD_LL_Resume(USBD_HandleTypeDef *pdev)
{
	if(pdev->dev_state == USBD_STATE_SUSPENDED)
	{
		pdev->dev_state = pdev->dev_old_state;
	}

	return USBD_OK;
}

/**
 * @brief  USBD_LL_SOF
 *         Handle SOF event
 * @param  pdev: device instance
 * @retval status
 */

USBD_StatusTypeDef USBD_LL_SOF(USBD_HandleTypeDef *pdev)
{
	/* The SOF event can be distributed for all classes that support it */
	if(pdev->dev_state == USBD_STATE_CONFIGURED)
	{
		if(pdev->pClass != NULL)
		{
			if(pdev->pClass->SOF != NULL) (void)pdev->pClass->SOF(pdev);
		}
	}
	return USBD_OK;
}

/**
 * @brief  USBD_LL_IsoINIncomplete
 *         Handle iso in incomplete event
 * @param  pdev: device instance
 * @retval status
 */
USBD_StatusTypeDef USBD_LL_IsoINIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
	if(pdev->pClass == NULL) return USBD_FAIL;
	if(pdev->dev_state == USBD_STATE_CONFIGURED)
	{
		if(pdev->pClass->IsoINIncomplete != NULL) pdev->pClass->IsoINIncomplete(pdev, epnum, pdev->last_idx_class);
	}
	return USBD_OK;
}

/**
 * @brief  USBD_LL_IsoOUTIncomplete
 *         Handle iso out incomplete event
 * @param  pdev: device instance
 * @retval status
 */
USBD_StatusTypeDef USBD_LL_IsoOUTIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
	if(pdev->pClass == NULL) return USBD_FAIL;

	if(pdev->dev_state == USBD_STATE_CONFIGURED)
	{
		if(pdev->pClass->IsoOUTIncomplete != NULL) pdev->pClass->IsoOUTIncomplete(pdev, epnum, pdev->last_idx_class);
	}

	return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_DevConnected(USBD_HandleTypeDef *pdev) { return USBD_OK; }

/**
 * @brief  USBD_LL_DevDisconnected
 *         Handle device disconnection event
 * @param  pdev: device instance
 * @retval status
 */
USBD_StatusTypeDef USBD_LL_DevDisconnected(USBD_HandleTypeDef *pdev)
{
	USBD_StatusTypeDef ret = USBD_OK;
	pdev->dev_state = USBD_STATE_DEFAULT;
	if(pdev->pClass != NULL)
	{
		if(pdev->pClass->DeInit(pdev, (uint8_t)pdev->dev_config) != 0U) ret = USBD_FAIL;
	}
	return ret;
}

/**
 * @brief  USBD_GetEpDesc
 *         This function return the Endpoint descriptor
 * @param  pdev: device instance
 * @param  pConfDesc:  pointer to Bos descriptor
 * @param  EpAddr:  endpoint address
 * @retval pointer to video endpoint descriptor
 */
void *USBD_GetEpDesc(uint8_t *pConfDesc, uint8_t EpAddr)
{
	USBD_DescHeaderTypeDef *pdesc = (USBD_DescHeaderTypeDef *)(void *)pConfDesc;
	USBD_ConfigDescTypeDef *desc = (USBD_ConfigDescTypeDef *)(void *)pConfDesc;
	USBD_EpDescTypeDef *pEpDesc = NULL;
	uint16_t ptr;

	if(desc->wTotalLength > desc->bLength)
	{
		ptr = desc->bLength;

		while(ptr < desc->wTotalLength)
		{
			pdesc = USBD_GetNextDesc((uint8_t *)pdesc, &ptr);

			if(pdesc->bDescriptorType == USB_DESC_TYPE_ENDPOINT)
			{
				pEpDesc = (USBD_EpDescTypeDef *)(void *)pdesc;

				if(pEpDesc->bEndpointAddress == EpAddr)
				{
					break;
				}
				else
				{
					pEpDesc = NULL;
				}
			}
		}
	}

	return (void *)pEpDesc;
}

/**
 * @brief  USBD_GetNextDesc
 *         This function return the next descriptor header
 * @param  buf: Buffer where the descriptor is available
 * @param  ptr: data pointer inside the descriptor
 * @retval next header
 */
USBD_DescHeaderTypeDef *USBD_GetNextDesc(uint8_t *pbuf, uint16_t *ptr)
{
	USBD_DescHeaderTypeDef *pnext = (USBD_DescHeaderTypeDef *)(void *)pbuf;

	*ptr += pnext->bLength;
	pnext = (USBD_DescHeaderTypeDef *)(void *)(pbuf + pnext->bLength);

	return (pnext);
}
