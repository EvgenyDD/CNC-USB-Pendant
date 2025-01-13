#ifndef __USB_CUSTOMHID_H
#define __USB_CUSTOMHID_H

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

#define USBD_PEND_HID_CONFIG_DESC_SIZE 41
#define USBD_PEND_HID_DESC_SIZE 9
#define USBD_CUSTOM_HID_REPORT_DESC_SIZE 39

#define USBD_PEND_HID_EPIN_ADDR 0x81U
#define USBD_PEND_HID_EPOUT_ADDR 0x01U

#define USBD_PEND_HID_EPIN_SIZE 8  // STM -> host
#define USBD_PEND_HID_EPOUT_SIZE 8 // host -> STM

#ifdef USE_USBD_COMPOSITE
uint8_t usbd_pend_hid_send_report(USBD_HandleTypeDef *pdev, uint8_t *report, uint16_t len, uint8_t ClassId);
#else
uint8_t usbd_pend_hid_send_report(USBD_HandleTypeDef *pdev, uint8_t *report, uint16_t len);
#endif

uint8_t *pend_hid_get_desc_dev_qual(uint16_t *length);
uint8_t *pend_hid_get_desc_cfg(uint16_t *length);
uint8_t *pend_hid_get_desc_cfg_hs(uint16_t *length);

extern void pend_hid_parse_buf(const uint8_t *, uint32_t);

extern uint8_t pend_hid_desc[];
extern uint8_t pend_hid_desc_report[];
extern USBD_ClassTypeDef usbd_class_pend_hid;

#endif // __USB_CUSTOMHID_H
