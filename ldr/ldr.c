#include "config_system.h"
#include "fw_header.h"
#include "main.h"
#include "platform.h"
#include "ret_mem.h"
#include "stm32h5xx.h"
#include "usbd_device.h"
#include "usbd_dfu.h"
#include <string.h>

#define BOOT_DELAY 3000

bool g_stay_in_boot = false;
uint32_t g_uid[3];

static volatile uint32_t boot_delay = BOOT_DELAY;

uint8_t test = 0;
config_entry_t g_device_config[] = {
	{"test", sizeof(test), 0, &test},
};
const uint32_t g_device_config_count = sizeof(g_device_config) / sizeof(g_device_config[0]);

void init(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;

	platform_get_uid(g_uid);
	// platform_watchdog_init();

	fw_header_check_all();

	ret_mem_init();
	ret_mem_set_load_src(LOAD_SRC_BOOTLOADER); // let preboot know it was booted from bootloader

	if(ret_mem_is_bl_stuck()) g_stay_in_boot = true;
}

void loop(void)
{
	static uint32_t systick_ms_prev = 0;
	uint32_t diff_ms = (HAL_GetTick() - systick_ms_prev);
	if(diff_ms > 0x0FFFFFFF) diff_ms = 0xFFFFFFFF - systick_ms_prev + HAL_GetTick();
	systick_ms_prev = HAL_GetTick();

	// platform_watchdog_reset();
	if(!boot_delay &&
	   !g_stay_in_boot &&
	   g_fw_info[FW_APP].locked == false)
	{
		platform_reset();
	}

	usbd_dfu_poll(diff_ms);

	boot_delay = boot_delay >= diff_ms ? boot_delay - diff_ms : 0;
}
