#ifndef __USBD_CONF_H
#define __USBD_CONF_H

#include "stm32h5xx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USBD_MAX_NUM_INTERFACES 1U
#define USBD_MAX_NUM_CONFIGURATION 1U
#define USBD_MAX_STR_DESC_SIZ 0x100U
#define USBD_SELF_POWERED 1U
#define USBD_DEBUG_LEVEL 2U

/* ECM, RNDIS, DFU Class Config */
#define USBD_SUPPORT_USER_STRING_DESC 0

/* BillBoard Class Config */
#define USBD_CLASS_USER_STRING_DESC 0
#define USB_BB_MAX_NUM_ALT_MODE 0x2U

/* MSC Class Config */
#define MSC_MEDIA_PACKET 8192U

/* CDC Class Config */
#define USBD_CDC_INTERVAL 2000U

/* DFU Class Config */
#define USBD_DFU_MAX_ITF_NUM 1U
#define USBD_DFU_XFERS_IZE 1024U

/* AUDIO Class Config */
#define USBD_AUDIO_FREQ 22100U

/* VIDEO Class Config */
#define UVC_1_1 /* #define UVC_1_0 */

/* To be used only with YUY2 and NV12 Video format, shouldn't be defined for MJPEG format */
#define USBD_UVC_FORMAT_UNCOMPRESSED

#ifdef USBD_UVC_FORMAT_UNCOMPRESSED
#define UVC_BITS_PER_PIXEL 12U
#define UVC_UNCOMPRESSED_GUID UVC_GUID_NV12 /* UVC_GUID_YUY2 */

/* refer to Table 3-18 Color Matching Descriptor video class v1.1 */
#define UVC_COLOR_PRIMARIE 0x01U
#define UVC_TFR_CHARACTERISTICS 0x01U
#define UVC_MATRIX_COEFFICIENTS 0x04U
#endif /* USBD_UVC_FORMAT_UNCOMPRESSED */

/* Video Stream frame width and height */
#define UVC_WIDTH 176U
#define UVC_HEIGHT 144U

/* bEndpointAddress in Endpoint Descriptor */
#define UVC_IN_EP 0x81U

#define UVC_CAM_FPS_FS 10U
#define UVC_CAM_FPS_HS 5U

#define UVC_ISO_FS_MPS 512U
#define UVC_ISO_HS_MPS 512U

#define UVC_PACKET_SIZE UVC_ISO_FS_MPS
/* To be used with Device Only IP supporting double buffer mode */
/* #define UVC_HEADER_PACKET_CNT                     0x02U */
/* #define UVC_PACKET_SIZE                           (UVC_ISO_FS_MPS * UVC_HEADER_PACKET_CNT) */

#define UVC_MAX_FRAME_SIZE (UVC_WIDTH * UVC_HEIGHT * 16U / 8U)

/* DEBUG macros */
#if(USBD_DEBUG_LEVEL > 0U)
#define USBD_UsrLog(...)     \
	do                       \
	{                        \
		printf(__VA_ARGS__); \
		printf("\n");        \
	} while(0)
#else
#define USBD_UsrLog(...) \
	do                   \
	{                    \
	} while(0)
#endif

#if(USBD_DEBUG_LEVEL > 1U)
#define USBD_ErrLog(...)     \
	do                       \
	{                        \
		printf("ERROR: ");   \
		printf(__VA_ARGS__); \
		printf("\n");        \
	} while(0)
#else
#define USBD_ErrLog(...) \
	do                   \
	{                    \
	} while(0)
#endif

#if(USBD_DEBUG_LEVEL > 2U)
#define USBD_DbgLog(...)     \
	do                       \
	{                        \
		printf("DEBUG : ");  \
		printf(__VA_ARGS__); \
		printf("\n");        \
	} while(0)
#else
#define USBD_DbgLog(...) \
	do                   \
	{                    \
	} while(0)
#endif

#endif // __USBD_CONF_H