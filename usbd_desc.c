#include "platform.h"
#include "usbd_conf.h"
#include "usbd_core.h"
#include "usbd_def.h"
#include "usbd_pend_hid.h"

#define USBD_VID 0x10CE
#define USBD_PID 0xEB93

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
	0x02,					   //
	0x00,					   // bDeviceClass
	0x00,					   // bDeviceSubClass
	0x00,					   // bDeviceProtocol
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

#define RPT1_COUNT 7 // PC sends CUBE/2
#define RPT4_COUNT 7 // STM32 sends CUBE/2

__ALIGN_BEGIN uint8_t pend_hid_desc_report[] __ALIGN_END = {
	0x06, 0x00, 0xff,					// USAGE_PAGE (Generic Desktop)
	0x09, 0x01,							// USAGE (Vendor Usage 1)
	0xa1, 0x01,							// COLLECTION (Application)
	0x85, 0x01,							//   REPORT_ID (6) host->stm32
	0x09, 0x01,							//     USAGE (Vendor Usage 1)
	0x15, 0x00,							//     LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,					//     LOGICAL_MAXIMUM (255)
	0x75, 0x08,							//     REPORT_SIZE (8)
	0x95, USBD_PEND_HID_EPOUT_SIZE - 1, //     REPORT_COUNT
	0xb1, 0x82,							//     FEATURE (Data,Var,Abs,Vol)
	0x85, 0x06,							//   REPORT_ID (6)
	0x09, 0x01,							//     USAGE (Vendor Usage 1)
	0x91, 0x82,							//     OUTPUT (Data,Var,Abs,Vol)
	0x85, 0x04,							//   REPORT_ID (4) stm32->host
	0x09, 0x01,							//     USAGE (Vendor Usage 1)
	0x75, 0x08,							//     REPORT_SIZE (8)
	0x95, USBD_PEND_HID_EPIN_SIZE - 1,	//     REPORT_COUNT
	0x81, 0x82,							//     INPUT (Data,Var,Abs,Vol)
	0xc0								// END_COLLECTION
};

__ALIGN_BEGIN static uint8_t desc_cfg[] __ALIGN_END = {
	0x09,									// bLength: Configuration Descriptor size
	USB_DESC_TYPE_CONFIGURATION,			// bDescriptorType: Configuration
	LOBYTE(USBD_PEND_HID_CONFIG_DESC_SIZE), // wTotalLength: Bytes returned
	HIBYTE(USBD_PEND_HID_CONFIG_DESC_SIZE), //
	0x01,									// bNumInterfaces: 1 interface
	0x01,									// bConfigurationValue: Configuration value
	0x00,									// iConfiguration: Index of string descriptor describing the configuration
	0xE0,									// bmAttributes: Bus Powered according to user configuration
	USBD_MAX_POWER,							// MaxPower (mA)

	/************** Descriptor of Pendant HID interface ****************/
	0x09,					 // bLength: Interface Descriptor size
	USB_DESC_TYPE_INTERFACE, // bDescriptorType: Interface descriptor type
	0x00,					 // bInterfaceNumber: Number of Interface
	0x00,					 // bAlternateSetting: Alternate setting
	0x02,					 // bNumEndpoints
	0x03,					 // bInterfaceClass: CUSTOM_HID
	0x00,					 // bInterfaceSubClass : 1=BOOT, 0=no boot
	0x00,					 // nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse
	0x00,					 // iInterface: Index of string descriptor

	/******************** Descriptor of Pendant HID *************************/
	0x09,									  // bLength: CUSTOM_HID Descriptor size
	CUSTOM_HID_DESCRIPTOR_TYPE,				  // bDescriptorType: CUSTOM_HID
	0x11,									  // bCUSTOM_HIDUSTOM_HID: CUSTOM_HID Class Spec release number
	0x01,									  //
	0x33,									  // bCountryCode: Hardware target country
	0x01,									  // bNumDescriptors: Number of CUSTOM_HID class descriptors to follow
	CUSTOM_HID_REPORT_DESC,					  // bDescriptorType
	LOBYTE(USBD_CUSTOM_HID_REPORT_DESC_SIZE), // wItemLength: Total length of Report descriptor
	HIBYTE(USBD_CUSTOM_HID_REPORT_DESC_SIZE),

	/******************** Descriptor of Pendant HID endpoints ********************/
	0x07,							 // bLength: Endpoint Descriptor size
	USB_DESC_TYPE_ENDPOINT,			 // bDescriptorType:
	USBD_PEND_HID_EPIN_ADDR,		 // bEndpointAddress: Endpoint Address (IN)
	0x03,							 // bmAttributes: Interrupt endpoint
	LOBYTE(USBD_PEND_HID_EPIN_SIZE), // wMaxPacketSize
	HIBYTE(USBD_PEND_HID_EPIN_SIZE), //
	5,								 // bInterval: Polling Interval

	0x07,							  // bLength: Endpoint Descriptor size
	USB_DESC_TYPE_ENDPOINT,			  // bDescriptorType:
	USBD_PEND_HID_EPOUT_ADDR,		  // bEndpointAddress: Endpoint Address (OUT)
	0x03,							  // bmAttributes: Interrupt endpoint
	LOBYTE(USBD_PEND_HID_EPOUT_SIZE), // wMaxPacketSize
	HIBYTE(USBD_PEND_HID_EPOUT_SIZE), //
	5,								  // bInterval: Polling Interval
};

__ALIGN_BEGIN uint8_t pend_hid_desc[USBD_PEND_HID_DESC_SIZE] __ALIGN_END = {
	0x09,									  // bLength:
	CUSTOM_HID_DESCRIPTOR_TYPE,				  // bDescriptorType: CUSTOM_HID
	0x11,									  // bcdHID: CUSTOM_HID Class Spec release number 0x101
	0x01,									  //
	0x00,									  // bCountryCode: Hardware target country
	0x01,									  // bNumDescriptors: Number of CUSTOM_HID class descriptors to follow
	0x22,									  // bDescriptorType
	LOBYTE(USBD_CUSTOM_HID_REPORT_DESC_SIZE), // wItemLength: Total length of Report descriptor
	HIBYTE(USBD_CUSTOM_HID_REPORT_DESC_SIZE),
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

// static uint32_t str_to_unicode(const char *s, uint8_t *pbuf)
// {
// 	uint32_t idx = 0;
// 	for(; *s; idx++, s++, pbuf[2 * idx + 1] = 0)
// 		pbuf[2 * idx + 0] = *s;
// 	return 2 * idx;
// }

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
	USBD_GetString(speed == USBD_SPEED_HIGH ? (uint8_t *)USBD_PRODUCT_HS_STRING : (uint8_t *)USBD_PRODUCT_FS_STRING, USBD_StrDesc, length);
	return USBD_StrDesc;
}

static uint8_t *get_str_desc_manuf(USBD_SpeedTypeDef speed, uint16_t *length)
{
	USBD_GetString((uint8_t *)USBD_MANUFACTURER_STRING, USBD_StrDesc, length);
	return USBD_StrDesc;
}

static uint8_t *get_str_desc_serial(USBD_SpeedTypeDef speed, uint16_t *length)
{
	uint32_t p = 2;
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
	USBD_GetString(speed == USBD_SPEED_HIGH ? (uint8_t *)USBD_CONFIGURATION_HS_STRING : (uint8_t *)USBD_CONFIGURATION_FS_STRING, USBD_StrDesc, length);
	return USBD_StrDesc;
}

static uint8_t *get_str_desc_iface(USBD_SpeedTypeDef speed, uint16_t *length)
{
	USBD_GetString(speed == USBD_SPEED_HIGH ? (uint8_t *)USBD_INTERFACE_HS_STRING : (uint8_t *)USBD_INTERFACE_FS_STRING, USBD_StrDesc, length);
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

USBD_DescriptorsTypeDef usbd_class_descriptors = {
	get_str_desc_dev,
	get_str_desc_lang_id,
	get_str_desc_manuf,
	get_str_desc_prod,
	get_str_desc_serial,
	get_str_desc_cfg,
	get_str_desc_iface,
};

enum
{
	_check_desc_device = 1 / (sizeof(desc_device) == USB_LEN_DEV_DESC ? 1 : 0),
	_check_desc_lang_id = 1 / (sizeof(desc_lang_id) == USB_LEN_LANGID_STR_DESC ? 1 : 0),
	_check_desc_report = 1 / (sizeof(pend_hid_desc_report) == USBD_CUSTOM_HID_REPORT_DESC_SIZE ? 1 : 0),
	_check_pend_hid_desc = 1 / (sizeof(pend_hid_desc) == USBD_PEND_HID_DESC_SIZE ? 1 : 0),
	_check_desc_cfg = 1 / (sizeof(desc_cfg) == USBD_PEND_HID_CONFIG_DESC_SIZE ? 1 : 0),
};