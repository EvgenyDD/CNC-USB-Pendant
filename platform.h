#ifndef PLATFORM_H__
#define PLATFORM_H__

#include "usbd_def.h"
#include <stdint.h>

#define PIN_SET(x) x##_GPIO_Port->BSRR = x##_Pin
#define PIN_CLR(x) x##_GPIO_Port->BSRR = ((uint32_t)(x##_Pin)) << 16
#define PIN_WR(x, v) x##_GPIO_Port->BSRR = ((uint32_t)(x##_Pin)) << ((!(v)) * 16)
#define PIN_GET(x) !!(x##_GPIO_Port->IDR & x##_Pin)
#define PIN_GET_ODR(x) !!(x##_GPIO_Port->ODR & x##_Pin)

void init(void);
void loop(void);
void systick_irq(void);

void platform_reset(void);

extern USBD_HandleTypeDef hUsbDeviceFS;
extern uint32_t g_uid[3];

#endif // PLATFORM_H__