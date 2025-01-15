#include "usbd_device.h"
#include "usbd_ctlreq.h"
#include "usbd_dfu.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

static uint8_t init(USBD_HandleTypeDef *pdev, uint8_t cfgidx) { return 0; }
static uint8_t deinit(USBD_HandleTypeDef *pdev, uint8_t cfgidx) { return 0; }

USBD_ClassTypeDef usbd_class_device = {
	init,
	deinit,
	usbd_dfu_setup,
	NULL,
	usbd_dfu_ep0_rx_ready,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	pend_hid_get_desc_cfg_hs,
	pend_hid_get_desc_cfg,
	pend_hid_get_desc_cfg,
	pend_hid_get_desc_dev_qual,
};
