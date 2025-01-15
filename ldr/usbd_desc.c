#include "platform.h"
#include "usbd_conf.h"
#include "usbd_core.h"
#include "usbd_def.h"
#include "usbd_device.h"

#define USBD_VID 0x0483
#define USBD_PID 0xDF11

#define DEVICE_NAME "CNC USB Pendant"

#define USBD_LANGID_STRING 0x409
#define USBD_MANUFACTURER_STRING "Open Source"
#define USBD_PRODUCT_HS_STRING DEVICE_NAME
#define USBD_PRODUCT_FS_STRING DEVICE_NAME
#define USBD_CONFIGURATION_HS_STRING DEVICE_NAME
#define USBD_INTERFACE_HS_STRING DEVICE_NAME
#define USBD_CONFIGURATION_FS_STRING DEVICE_NAME
#define USBD_INTERFACE_FS_STRING DEVICE_NAME

__ALIGN_BEGIN uint8_t desc_device[] __ALIGN_END = {
	USB_LEN_DEV_DESC,		   // bLength
	USB_DESC_TYPE_DEVICE,	   // bDescriptorType
	0x00,					   // bcdUSB
	0x00,					   // bDeviceClass
	0x00,					   // bDeviceSubClass
	0x00,					   // bDeviceProtocol
	0x01,					   // bDeviceProtocol: Interface Association Descriptor
	USB_MAX_EP0_SIZE,		   // bMaxPacketSize
	LOBYTE(USBD_VID),		   // idVendor
	HIBYTE(USBD_VID),		   // idVendor
	LOBYTE(USBD_PID),		   // idVendor
	HIBYTE(USBD_PID),		   // idVendor
	0x00,					   // bcdDevice rel. 2.00
	0x02,					   //
	USBD_IDX_MFC_STR,		   // Index of manufacturer string
	USBD_IDX_PRODUCT_STR,	   // Index of product string
	USBD_IDX_SERIAL_STR,	   // Index of serial number string
	USBD_MAX_NUM_CONFIGURATION // bNumConfigurations
};

__ALIGN_BEGIN uint8_t desc_lang_id[] __ALIGN_END = {
	USB_LEN_LANGID_STR_DESC,
	USB_DESC_TYPE_STRING,
	LOBYTE(USBD_LANGID_STRING),
	HIBYTE(USBD_LANGID_STRING),
};

__ALIGN_BEGIN static uint8_t desc_cfg[] __ALIGN_END = {
	0x09,									// bLength: Configuration Descriptor size
	USB_DESC_TYPE_CONFIGURATION,			// bDescriptorType: Configuration
	LOBYTE(USBD_PEND_HID_CONFIG_DESC_SIZE), // wTotalLength: Bytes returned
	HIBYTE(USBD_PEND_HID_CONFIG_DESC_SIZE), //
	0x01,									// bNumInterfaces: 1 for DFU
	0x01,									// bConfigurationValue: Configuration value
	0x00,									// iConfiguration: Index of string descriptor describing the configuration
	0xE0,									// bmAttributes: Bus Powered according to user configuration
	USBD_MAX_POWER,							// MaxPower (mA)

	// ******* Descriptor of DFU interface 0 Alternate setting 0
	USBD_DFU_IF_DESC(0), // This interface is mandatory for all devices

	// ******* DFU Functional Descriptor
	0x09,						   // blength
	USB_DESC_TYPE_DFU,			   // functional descriptor
	0x0B,						   /* bmAttribute
										 bitCanDnload             = 1      (bit 0)
										 bitCanUpload             = 1      (bit 1)
										 bitManifestationTolerant = 0      (bit 2)
										 bitWillDetach            = 1      (bit 3)
										 Reserved                          (bit4-6)
										 bitAcceleratedST         = 0      (bit 7) */
	255,						   // detach timeout= 255 ms
	0,							   //
	TRANSFER_SIZE_BYTES(XFERSIZE), // WARNING: In DMA mode the multiple MPS packets feature is still not supported ==> when using DMA XFERSIZE should be 64
	0x1A,						   // bcdDFUVersion
	0x01,
};

__ALIGN_BEGIN static uint8_t desc_dev_qual[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END = {
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

__ALIGN_BEGIN uint8_t USBD_StrDesc[USBD_MAX_STR_DESC_SIZ] __ALIGN_END;

static uint8_t usbd_str_serial[128] = {0};

static uint32_t int_to_unicode(uint32_t value, uint8_t *pbuf, uint8_t len)
{
	for(uint8_t idx = 0; idx < len; idx++, value <<= 4, pbuf[2 * idx + 1] = 0)
		pbuf[2 * idx + 0] = (value >> 28) + ((value >> 28) < 0xA ? '0' : ('A' - 10));
	return 2 * len;
}

static uint32_t str_to_unicode(const char *s, uint8_t *pbuf)
{
	uint32_t idx = 0;
	for(; *s; idx++, s++, pbuf[2 * idx + 1] = 0)
		pbuf[2 * idx + 0] = *s;
	return 2 * idx;
}

static uint8_t *get_str_desc_dev(USBD_SpeedTypeDef speed, uint16_t *length)
{
	*length = sizeof(desc_device);
	return (uint8_t *)desc_device;
}

static uint8_t *get_str_desc_lang_id(USBD_SpeedTypeDef speed, uint16_t *length)
{
	*length = sizeof(desc_lang_id);
	return (uint8_t *)desc_lang_id;
}

static uint8_t *get_str_desc_prod(USBD_SpeedTypeDef speed, uint16_t *length)
{
	USBD_GetString(speed == USBD_SPEED_HIGH ? (const uint8_t *)USBD_PRODUCT_HS_STRING : (const uint8_t *)USBD_PRODUCT_FS_STRING, USBD_StrDesc, length);
	return USBD_StrDesc;
}

static uint8_t *get_str_desc_manuf(USBD_SpeedTypeDef speed, uint16_t *length)
{
	USBD_GetString((const uint8_t *)USBD_MANUFACTURER_STRING, USBD_StrDesc, length);
	return USBD_StrDesc;
}

static uint8_t *get_str_desc_serial(USBD_SpeedTypeDef speed, uint16_t *length)
{
	uint32_t p = 2;
	p += str_to_unicode(DEV, &usbd_str_serial[p]);
	p += str_to_unicode("_", &usbd_str_serial[p]);
	p += int_to_unicode(g_uid[0], &usbd_str_serial[p], 8);
	p += int_to_unicode(g_uid[1], &usbd_str_serial[p], 8);
	p += int_to_unicode(g_uid[2], &usbd_str_serial[p], 8);
	usbd_str_serial[0] = p;
	usbd_str_serial[1] = USB_DESC_TYPE_STRING;
	*length = p;
	return usbd_str_serial;
}

static uint8_t *get_str_desc_cfg(USBD_SpeedTypeDef speed, uint16_t *length)
{
	USBD_GetString(speed == USBD_SPEED_HIGH ? (const uint8_t *)USBD_CONFIGURATION_HS_STRING : (const uint8_t *)USBD_CONFIGURATION_FS_STRING, USBD_StrDesc, length);
	return USBD_StrDesc;
}

static uint8_t *get_str_desc_iface(USBD_SpeedTypeDef speed, uint16_t *length)
{
	USBD_GetString(speed == USBD_SPEED_HIGH ? (const uint8_t *)USBD_INTERFACE_HS_STRING : (const uint8_t *)USBD_INTERFACE_FS_STRING, USBD_StrDesc, length);
	return USBD_StrDesc;
}

uint8_t *pend_hid_get_desc_dev_qual(uint16_t *length)
{
	*length = (uint16_t)sizeof(desc_dev_qual);
	return desc_dev_qual;
}

uint8_t *pend_hid_get_desc_cfg(uint16_t *length)
{
	*length = (uint16_t)sizeof(desc_cfg);
	return desc_cfg;
}

uint8_t *pend_hid_get_desc_cfg_hs(uint16_t *length)
{
	*length = (uint16_t)sizeof(desc_cfg);
	return desc_cfg;
}

static uint8_t *usbd_get_null(uint8_t speed, uint16_t *length)
{
	*length = 0;
	return USBD_StrDesc;
}

void usdb_desc_init(void) {}

USBD_DescriptorsTypeDef usbd_class_descriptors = {
	get_str_desc_dev,
	get_str_desc_lang_id,
	get_str_desc_manuf,
	get_str_desc_prod,
	get_str_desc_serial,
	get_str_desc_cfg,
	get_str_desc_iface,
	usbd_get_null,
	usbd_get_null,
	usbd_get_null,
};

enum
{
	_check_desc_device = 1 / (sizeof(desc_device) == USB_LEN_DEV_DESC ? 1 : 0),
	_check_desc_lang_id = 1 / (sizeof(desc_lang_id) == USB_LEN_LANGID_STR_DESC ? 1 : 0),
	_check_desc_cfg = 1 / (sizeof(desc_cfg) == USBD_PEND_HID_CONFIG_DESC_SIZE ? 1 : 0),
};