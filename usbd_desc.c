#include "md5.h"
#include "platform.h"
#include "usbd_conf.h"
#include "usbd_core.h"
#include "usbd_def.h"
#include "usbd_pend_hid.h"

#if 0
#define USBD_VID 0x10CE
#define USBD_PID 0xEB93 + 100
#else
#define USBD_VID 0xF055
#define USBD_PID 0x1337
#endif

#define DEV "USB_CNC_PEND"

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
	USB_LEN_DEV_DESC,			   // bLength
	USB_DESC_TYPE_DEVICE,		   // bDescriptorType
	0x00,						   // bcdUSB
	0x02,						   //
	USB_DEVICE_CLASS_COMPOSITE,	   // bDeviceClass: Miscellaneous Device Class
	USB_DEVICE_SUBCLASS_COMPOSITE, // bDeviceSubClass: Common Class
	0x01,						   // bDeviceProtocol: Interface Association Descriptor
	USB_MAX_EP0_SIZE,			   // bMaxPacketSize
	LOBYTE(USBD_VID),			   // idVendor
	HIBYTE(USBD_VID),			   // idVendor
	LOBYTE(USBD_PID),			   // idVendor
	HIBYTE(USBD_PID),			   // idVendor
	0x00,						   // bcdDevice rel. 2.00
	0x02,						   //
	USBD_IDX_MFC_STR,			   // Index of manufacturer string
	USBD_IDX_PRODUCT_STR,		   // Index of product string
	USBD_IDX_SERIAL_STR,		   // Index of serial number string
	USBD_MAX_NUM_CONFIGURATION	   // bNumConfigurations
};

__ALIGN_BEGIN uint8_t desc_lang_id[] __ALIGN_END = {
	USB_LEN_LANGID_STR_DESC,
	USB_DESC_TYPE_STRING,
	LOBYTE(USBD_LANGID_STRING),
	HIBYTE(USBD_LANGID_STRING),
};
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpacked"
struct __packed
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wData[7];
	uint8_t bVendorCode;
	uint8_t bPadding;
} usbd_os_str_desc = {
	sizeof(usbd_os_str_desc),
	USB_DESC_TYPE_STRING,
	u"MSFT100",
	USB_REQ_GET_OS_FEATURE_DESCRIPTOR,
	0,
};

struct __packed
{
	// Header
	uint32_t dwLength;
	uint16_t bcdVersion;
	uint16_t wIndex;
	uint8_t bCount;
	uint8_t bReserved1[7];
	// Function Section 1
	uint8_t bFirstInterfaceNumber;
	uint8_t bReserved2;
	uint8_t bCompatibleID[8];
	uint8_t bSubCompatibleID[8];
	uint8_t bReserved3[6];
} usbd_compat_id_desc = {
	sizeof(usbd_compat_id_desc),
	WINUSB_BCD_VERSION,
	WINUSB_REQ_GET_COMPATIBLE_ID_FEATURE_DESCRIPTOR, // wIndex
	1,												 // bCount
	{0},											 // bReserved1
	0,												 // bFirstInterfaceNumber
	1,												 // bReserved2
	"WINUSB",										 // bCompatibleID
	{0},											 // bSubCompatibleID
	{0},											 // bReserved3
};

struct winusb_ext_prop_desc_hdr
{
	// header
	uint32_t dwLength;
	uint16_t bcdVersion;
	uint16_t wIndex;
	uint16_t wNumFeatures;
} __packed;

struct winusb_ext_prop_feat_desc
{
	// feature no. 1
	uint32_t dwSize;
	uint32_t dwPropertyDataType;
	uint16_t wPropertyNameLength;
	const uint16_t bPropertyName[21];
	uint32_t dwPropertyDataLength;
	uint16_t bPropertyData[40];
} __packed;

struct winusb_ext_prop_desc
{
	struct winusb_ext_prop_desc_hdr header;
	struct winusb_ext_prop_feat_desc features[1];
} __packed;
#pragma GCC diagnostic pop

struct winusb_ext_prop_desc usbd_winusb_ex_prop_desc = {
	.header = {
		.dwLength = sizeof(struct winusb_ext_prop_desc_hdr) + sizeof(struct winusb_ext_prop_feat_desc),
		.bcdVersion = WINUSB_BCD_VERSION,
		.wIndex = WINUSB_REQ_GET_EXTENDED_PROPERTIES_OS_FEATURE_DESCRIPTOR,
		.wNumFeatures = 1,
	},
	.features = {{
		.dwSize = sizeof(struct winusb_ext_prop_feat_desc),
		.dwPropertyDataType = WINUSB_PROP_DATA_TYPE_REG_REG_MULTI_SZ,
		.wPropertyNameLength = sizeof(usbd_winusb_ex_prop_desc.features[0].bPropertyName),
		.bPropertyName = u"DeviceInterfaceGUIDs",
		.dwPropertyDataLength = sizeof(usbd_winusb_ex_prop_desc.features[0].bPropertyData),
		.bPropertyData = u"{00000000-0000-0000-0000-000000000000}\0",
	}},
};
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

static uint8_t *usbd_usr_os_str_desc(uint8_t speed, uint16_t *length)
{
	*length = usbd_os_str_desc.bLength;
	return (uint8_t *)&usbd_os_str_desc;
}

static uint8_t *usbd_usr_ext_prop_feat_desc(uint8_t speed, uint16_t *length)
{
	*length = usbd_winusb_ex_prop_desc.header.dwLength;
	return (uint8_t *)&usbd_winusb_ex_prop_desc;
}

static uint8_t *usbd_usr_ext_compat_id_feat_desc(uint8_t speed, uint16_t *length)
{
	*length = usbd_compat_id_desc.dwLength;
	return (uint8_t *)&usbd_compat_id_desc;
}

void usdb_desc_init(void)
{
	// Generate GUID by PID/VID/NAME/SERIAL
#ifndef USE_STD_LIB_GEN
	uint8_t buf[64], hash[16];
	int len = snprintf((char *)buf, sizeof(buf), "USB\\VID_%04X&PID_%04X\\%s_%08lX%08lX%08lX", USBD_VID, USBD_PID, DEV, g_uid[0], g_uid[1], g_uid[2]);
	md5_data(buf, len, hash);
	len = snprintf((char *)buf, sizeof(buf), "{%02X%02X%02X%02X-%02X%02X-3%X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X}", // RFC9562 - Type 3
				   hash[0], hash[1], hash[2], hash[3], hash[4], hash[5], hash[6] & 0xF, hash[7],
				   0x80 | (hash[8] & 0x3F), hash[9], hash[10], hash[11], hash[12], hash[13], hash[14], hash[15]);
	for(int i = 0; i < len && i < (int)(sizeof(usbd_winusb_ex_prop_desc.features[0].bPropertyData) / sizeof(usbd_winusb_ex_prop_desc.features[0].bPropertyData[0])); i++)
		usbd_winusb_ex_prop_desc.features[0].bPropertyData[i] = buf[i];
#else
	// smaller size without using libc
	uint8_t buf[64] = "USB\\VID_****&PID_****\\", hash[16];
	for(uint32_t i = 0; i < 4; i++)
		buf[8 + i] = hex2ch((USBD_VID >> (4 * (3 - i))) & 0xF);
	for(uint32_t i = 0; i < 4; i++)
		buf[17 + i] = hex2ch((USBD_PID >> (4 * (3 - i))) & 0xF);
	uint32_t len_dev_str = strlen(DEV);
	memcpy(buf + 22, DEV, len_dev_str);
	uint32_t c = 22 + len_dev_str + 1;
	buf[22 + len_dev_str] = '_';
	for(uint32_t i = 0; i < 3; i++)
		for(uint32_t j = 0; j < 8; j++)
			buf[c++] = hex2ch((g_uid[i] >> (4 * (7 - j))) & 0xF);
	md5_data(buf, c, hash);
	memcpy(buf, "{********-****-3***-****-************}", 38); // RFC9562 - Type 3
	buf[1] = hex2ch((hash[0] >> 4) & 0xF);
	buf[2] = hex2ch((hash[0] >> 0) & 0xF);
	buf[3] = hex2ch((hash[1] >> 4) & 0xF);
	buf[4] = hex2ch((hash[1] >> 0) & 0xF);
	buf[5] = hex2ch((hash[2] >> 4) & 0xF);
	buf[6] = hex2ch((hash[2] >> 0) & 0xF);
	buf[7] = hex2ch((hash[3] >> 4) & 0xF);
	buf[8] = hex2ch((hash[3] >> 0) & 0xF); // -
	buf[10] = hex2ch((hash[4] >> 4) & 0xF);
	buf[11] = hex2ch((hash[4] >> 0) & 0xF);
	buf[12] = hex2ch((hash[5] >> 4) & 0xF);
	buf[13] = hex2ch((hash[5] >> 0) & 0xF); // -
	buf[16] = hex2ch((hash[6] >> 0) & 0xF);
	buf[17] = hex2ch((hash[7] >> 4) & 0xF);
	buf[18] = hex2ch((hash[7] >> 0) & 0xF); // -
	buf[20] = hex2ch(((0x80 | (hash[8] & 0x30)) >> 4) & 0xF);
	buf[21] = hex2ch((hash[8] >> 0) & 0xF);
	buf[22] = hex2ch((hash[9] >> 4) & 0xF);
	buf[23] = hex2ch((hash[9] >> 0) & 0xF); // -
	buf[25] = hex2ch((hash[10] >> 4) & 0xF);
	buf[26] = hex2ch((hash[10] >> 0) & 0xF);
	buf[27] = hex2ch((hash[11] >> 4) & 0xF);
	buf[28] = hex2ch((hash[11] >> 0) & 0xF);
	buf[29] = hex2ch((hash[12] >> 4) & 0xF);
	buf[30] = hex2ch((hash[12] >> 0) & 0xF);
	buf[31] = hex2ch((hash[13] >> 4) & 0xF);
	buf[32] = hex2ch((hash[13] >> 0) & 0xF);
	buf[33] = hex2ch((hash[14] >> 4) & 0xF);
	buf[34] = hex2ch((hash[14] >> 0) & 0xF);
	buf[35] = hex2ch((hash[15] >> 4) & 0xF);
	buf[36] = hex2ch((hash[15] >> 0) & 0xF);
	for(int i = 0; i < 38 && i < (int)(sizeof(usbd_winusb_ex_prop_desc.features[0].bPropertyData) / sizeof(usbd_winusb_ex_prop_desc.features[0].bPropertyData[0])); i++)
		usbd_winusb_ex_prop_desc.features[0].bPropertyData[i] = buf[i];
#endif
}

USBD_DescriptorsTypeDef usbd_class_descriptors = {
	get_str_desc_dev,
	get_str_desc_lang_id,
	get_str_desc_manuf,
	get_str_desc_prod,
	get_str_desc_serial,
	get_str_desc_cfg,
	get_str_desc_iface,
	usbd_usr_os_str_desc,
	usbd_usr_ext_prop_feat_desc,
	usbd_usr_ext_compat_id_feat_desc,
};

enum
{
	_check_desc_device = 1 / (sizeof(desc_device) == USB_LEN_DEV_DESC ? 1 : 0),
	_check_desc_lang_id = 1 / (sizeof(desc_lang_id) == USB_LEN_LANGID_STR_DESC ? 1 : 0),
	_check_desc_report = 1 / (sizeof(pend_hid_desc_report) == USBD_CUSTOM_HID_REPORT_DESC_SIZE ? 1 : 0),
	_check_pend_hid_desc = 1 / (sizeof(pend_hid_desc) == USBD_PEND_HID_DESC_SIZE ? 1 : 0),
	_check_desc_cfg = 1 / (sizeof(desc_cfg) == USBD_PEND_HID_CONFIG_DESC_SIZE ? 1 : 0),
	// _check_desc_cfg = 1 / (sizeof(desc_cfg) == USBD_PEND_HID_DFU_CONFIG_DESC_SIZE ? 1 : 0),
};