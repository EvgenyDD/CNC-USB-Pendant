#include "usbd_device.h"
#include "usbd_ctlreq.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

static struct
{
	uint8_t report_buf_ep0[64];
	uint32_t report_size_ep0;
	uint8_t report_buf[USBD_PEND_HID_EPOUT_SIZE];
	uint32_t protocol;
	uint32_t idle_state;
	uint32_t alt_sett;
	bool is_busy;
	bool is_ep0_rpt_avail;
} hid_sts = {0};

static bool in_ep_tx = false;

static uint8_t init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
	USBD_LL_OpenEP(pdev, USBD_PEND_HID_EPIN_ADDR, USBD_EP_TYPE_INTR, USBD_PEND_HID_EPIN_SIZE);
	pdev->ep_in[USBD_PEND_HID_EPIN_ADDR & 0xFU].is_used = 1U;

	USBD_LL_OpenEP(pdev, USBD_PEND_HID_EPOUT_ADDR, USBD_EP_TYPE_INTR, USBD_PEND_HID_EPOUT_SIZE);
	pdev->ep_out[USBD_PEND_HID_EPOUT_ADDR & 0xFU].is_used = 1U;

	USBD_LL_PrepareReceive(pdev, USBD_PEND_HID_EPOUT_ADDR, hid_sts.report_buf, USBD_PEND_HID_EPOUT_SIZE);
	return 0;
}

static uint8_t deinit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
	USBD_LL_CloseEP(pdev, USBD_PEND_HID_EPIN_ADDR);
	pdev->ep_in[USBD_PEND_HID_EPIN_ADDR & 0xFU].is_used = 0U;

	USBD_LL_CloseEP(pdev, USBD_PEND_HID_EPOUT_ADDR);
	pdev->ep_out[USBD_PEND_HID_EPOUT_ADDR & 0xFU].is_used = 0U;

	return 0;
}

static uint8_t hid_setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req, uint8_t idx)
{
	uint16_t len = 0;
	uint8_t *pbuf = NULL;

	switch(req->bmRequest & USB_REQ_TYPE_MASK)
	{
	case USB_REQ_TYPE_CLASS:
		switch(req->bRequest)
		{
		case CUSTOM_HID_REQ_SET_PROTOCOL: hid_sts.protocol = (uint8_t)(req->wValue); break;
		case CUSTOM_HID_REQ_GET_PROTOCOL: USBD_CtlSendData(pdev, (uint8_t *)&hid_sts.protocol, 1U); break;
		case CUSTOM_HID_REQ_SET_IDLE: hid_sts.idle_state = (uint8_t)(req->wValue >> 8); break;
		case CUSTOM_HID_REQ_GET_IDLE: USBD_CtlSendData(pdev, (uint8_t *)&hid_sts.idle_state, 1U); break;

		case CUSTOM_HID_REQ_SET_REPORT:
			hid_sts.is_ep0_rpt_avail = 1;
			hid_sts.report_size_ep0 = MIN(req->wLength, USBD_PEND_HID_EPOUT_SIZE);
			USBD_CtlPrepareRx(pdev, hid_sts.report_buf_ep0, hid_sts.report_size_ep0);
			break;

		case CUSTOM_HID_REQ_GET_REPORT:
		default:
			USBD_CtlError(pdev, req);
			return USBD_FAIL;
		}
		break;

	case USB_REQ_TYPE_STANDARD:
		switch(req->bRequest)
		{
		case USB_REQ_GET_STATUS:
			if(pdev->dev_state == USBD_STATE_CONFIGURED)
			{
				uint16_t status_info = 0U;
				USBD_CtlSendData(pdev, (uint8_t *)&status_info, 2U);
			}
			else
			{
				USBD_CtlError(pdev, req);
				return USBD_FAIL;
			}
			break;

		case USB_REQ_GET_DESCRIPTOR:
			if((req->wValue >> 8) == CUSTOM_HID_REPORT_DESC)
			{
				len = MIN(USBD_CUSTOM_HID_REPORT_DESC_SIZE, req->wLength);
				pbuf = pend_hid_desc_report;
			}
			else
			{
				if((req->wValue >> 8) == CUSTOM_HID_DESCRIPTOR_TYPE)
				{
					pbuf = pend_hid_desc;
					len = MIN(USBD_PEND_HID_DESC_SIZE, req->wLength);
				}
			}
			USBD_CtlSendData(pdev, pbuf, len);
			break;

		case USB_REQ_GET_INTERFACE:
			if(pdev->dev_state == USBD_STATE_CONFIGURED)
			{
				USBD_CtlSendData(pdev, (uint8_t *)&hid_sts.alt_sett, 1U);
			}
			else
			{
				USBD_CtlError(pdev, req);
				return USBD_FAIL;
			}
			break;

		case USB_REQ_SET_INTERFACE:
			if(pdev->dev_state == USBD_STATE_CONFIGURED)
			{
				hid_sts.alt_sett = (uint8_t)(req->wValue);
			}
			else
			{
				USBD_CtlError(pdev, req);
				return USBD_FAIL;
			}
			break;

		case USB_REQ_CLEAR_FEATURE: break;

		default:
			USBD_CtlError(pdev, req);
			return USBD_FAIL;
		}
		break;

	default:
		USBD_CtlError(pdev, req);
		return USBD_FAIL;
	}
	return USBD_OK;
}

static uint8_t setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req, uint8_t idx)
{
	switch(req->bmRequest & USB_REQ_RECIPIENT_MASK)
	{
	case USB_REQ_RECIPIENT_INTERFACE: return hid_setup(pdev, req, idx);
	case USB_REQ_RECIPIENT_ENDPOINT: return hid_setup(pdev, req, idx);
	default: break;
	}
	return USBD_OK;
}

uint8_t usbd_pend_hid_send_report(USBD_HandleTypeDef *pdev, uint8_t *report, uint16_t len)
{
	if(pdev->dev_state == USBD_STATE_CONFIGURED)
	{
		if(in_ep_tx == false)
		{
			in_ep_tx = true;
			USBD_LL_Transmit(pdev, USBD_PEND_HID_EPIN_ADDR, report, len);
			return 0;
		}
	}
	return 1;
}

static uint8_t ep0_rx_ready(USBD_HandleTypeDef *pdev, uint8_t idx)
{
	if(hid_sts.is_ep0_rpt_avail)
	{
		pend_hid_parse_buf(hid_sts.report_buf_ep0, hid_sts.report_size_ep0);
		hid_sts.is_ep0_rpt_avail = 0;
	}
	return 0;
}

static uint8_t data_in(USBD_HandleTypeDef *pdev, uint8_t epnum, uint8_t index)
{
	in_ep_tx = false;
	return 0;
}

static uint8_t data_out(USBD_HandleTypeDef *pdev, uint8_t epnum, uint8_t index)
{
	pend_hid_parse_buf(hid_sts.report_buf, USBD_PEND_HID_EPOUT_SIZE);
	USBD_LL_PrepareReceive(&hUsbDeviceFS, USBD_PEND_HID_EPOUT_ADDR, hid_sts.report_buf, USBD_PEND_HID_EPOUT_SIZE);
	return 0;
}

USBD_ClassTypeDef usbd_class_device = {
	init,
	deinit,
	setup,
	NULL,
	ep0_rx_ready,
	data_in,
	data_out,
	NULL,
	NULL,
	NULL,
	pend_hid_get_desc_cfg_hs,
	pend_hid_get_desc_cfg,
	pend_hid_get_desc_cfg,
	pend_hid_get_desc_dev_qual,
};
