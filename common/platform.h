#ifndef PLATFORM_H__
#define PLATFORM_H__

#include "stm32h5xx.h"
#include <stdint.h>

#define FLASH_LEN (0x00020000U) // 128kB
#define FLASH_START FLASH_BASE
#define FLASH_ORIGIN FLASH_BASE
#define FLASH_FINISH (FLASH_BASE + FLASH_LEN)

#define PAGE_SIZE (8 * 1024)

#define PIN_SET(x) x##_GPIO_Port->BSRR = x##_Pin
#define PIN_CLR(x) x##_GPIO_Port->BSRR = ((uint32_t)(x##_Pin)) << 16
#define PIN_WR(x, v) x##_GPIO_Port->BSRR = ((uint32_t)(x##_Pin)) << ((!(v)) * 16)
#define PIN_GET(x) !!(x##_GPIO_Port->IDR & x##_Pin)
#define PIN_GET_ODR(x) !!(x##_GPIO_Port->ODR & x##_Pin)

void init(void);
void loop(void);

void platform_flash_erase_flag_reset(void);
void platform_flash_erase_flag_reset_sect_cfg(void);
int platform_flash_write(uint32_t dest, const uint8_t *src, uint32_t sz);
int platform_flash_read(uint32_t addr, uint8_t *src, uint32_t sz);

void platform_run_address(uint32_t address);
void platform_deinit(void);
void platform_reset(void);

void platform_get_uid(uint32_t *id);

extern int __preldr_start, __preldr_end;
extern int __ldr_start, __ldr_end;
extern int __cfg_start, __cfg_end;
extern int __app_start, __app_end;
extern int __header_offset;

extern uint32_t g_uid[3];

#endif // PLATFORM_H__