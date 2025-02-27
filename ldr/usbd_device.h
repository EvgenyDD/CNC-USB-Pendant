#ifndef USB_DEVICE_H__
#define USB_DEVICE_H__

#include "usbd_ioreq.h"
#include <stdbool.h>

#define CUSTOM_HID_REQ_SET_PROTOCOL 0x0BU
#define CUSTOM_HID_REQ_GET_PROTOCOL 0x03U

#define CUSTOM_HID_REQ_SET_IDLE 0x0AU
#define CUSTOM_HID_REQ_GET_IDLE 0x02U

#define CUSTOM_HID_REQ_SET_REPORT 0x09U
#define CUSTOM_HID_REQ_GET_REPORT 0x01U

#define CUSTOM_HID_DESCRIPTOR_TYPE 0x21U
#define CUSTOM_HID_REPORT_DESC 0x22U

// =========================

#define USBD_PEND_HID_CONFIG_DESC_SIZE 27
#define USBD_PEND_HID_DESC_SIZE 9
#define USBD_CUSTOM_HID_REPORT_DESC_SIZE 39

#define USBD_PEND_HID_EPIN_ADDR 0x81U
#define USBD_PEND_HID_EPOUT_ADDR 0x01U

#define USBD_PEND_HID_EPIN_SIZE 8  // STM -> host
#define USBD_PEND_HID_EPOUT_SIZE 8 // host -> STM

#define XFERSIZE (64)

// ******* Descriptor of DFU interface 0 Alternate setting n
#define USBD_DFU_IF_DESC(n) USB_LEN_DFU_DESC,				  /* bLength */                                           \
							USB_DESC_TYPE_INTERFACE,		  /* bDescriptorType */                                   \
							0x00,							  /* bInterfaceNumber */                                  \
							(n),							  /* bAlternateSetting: alternate setting */              \
							0x00,							  /* bNumEndpoints*/                                      \
							USB_INTERFACE_CLASS_APP_SPECIFIC, /* bInterfaceClass: application Specific Class Code */  \
							0x01,							  /* bInterfaceSubClass : Device Firmware Upgrade Code */ \
							0x02,							  /* nInterfaceProtocol: DFU mode protocol */             \
							USBD_IDX_INTERFACE_STR + (n) + 1  /* iInterface: index of string descriptor */

uint8_t *pend_hid_get_desc_dev_qual(uint16_t *length);
uint8_t *pend_hid_get_desc_cfg(uint16_t *length);
uint8_t *pend_hid_get_desc_cfg_hs(uint16_t *length);

void usdb_desc_init(void);

extern uint8_t pend_hid_desc[];
extern uint8_t pend_hid_desc_report[];
extern USBD_ClassTypeDef usbd_class_device;

#endif // USB_DEVICE_H__
