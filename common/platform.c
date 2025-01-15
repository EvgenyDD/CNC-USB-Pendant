#include "platform.h"
#include "fw_header.h"
#include <string.h>

#if FW_TYPE == FW_LDR || FW_TYPE == FW_APP
extern PCD_HandleTypeDef hpcd_USB_DRD_FS;
#endif

typedef void (*pFunction)(void);
uint32_t JumpAddress;
pFunction Jump_To_Application;

static uint8_t page_erased_bits[FLASH_LEN / PAGE_SIZE] = {0};

void platform_flash_erase_flag_reset(void) { memset(page_erased_bits, 0, sizeof(page_erased_bits)); }

void platform_flash_erase_flag_reset_sect_cfg(void) { page_erased_bits[(((uint32_t)&__cfg_start) - (FLASH_ORIGIN)) / PAGE_SIZE] = 0; }

int platform_flash_write(uint32_t dest, const uint8_t *src, uint32_t sz)
{
#if FW_TYPE == FW_LDR || FW_TYPE == FW_APP
	if(sz == 0) return 0;
	if(sz & 0xF) return 1; // program only by quad-word

	HAL_FLASH_Unlock();
	HAL_ICACHE_Disable();

	uint16_t halfword;
	uint32_t page;
	for(volatile uint32_t i = 0; i < (sz >> 4U); i++)
	{
		page = (dest - FLASH_BASE) / PAGE_SIZE;
		if(page > sizeof(page_erased_bits))
		{
			HAL_FLASH_Lock();
			HAL_ICACHE_Enable();
			return 2;
		}

		if(page_erased_bits[page] == 0)
		{
			FLASH_EraseInitTypeDef eraseInitStruct;
			eraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
			eraseInitStruct.Banks = page >= 8 ? FLASH_BANK_2 : FLASH_BANK_1;
			eraseInitStruct.Sector = page - (page >= 8 ? 8 : 0);
			eraseInitStruct.NbSectors = 1;

			uint32_t pageEraseError = 0;
			HAL_StatusTypeDef e = HAL_FLASHEx_Erase(&eraseInitStruct, &pageEraseError);
			if(e || pageEraseError != 0xFFFFFFFF)
			{
				HAL_FLASH_Lock();
				HAL_ICACHE_Enable();
				return 3;
			}
			page_erased_bits[page] = 1;
		}

		int sts = HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, dest + (i << 4U), (uintptr_t)&src[i << 4U]);
		if(sts)
		{
			HAL_FLASH_Lock();
			HAL_ICACHE_Enable();
			return sts;
		}
	}
	HAL_FLASH_Lock();
	for(uint32_t j = 0; j < sz; j++)
	{
		if(*(__IO uint8_t *)(dest + j) != src[j]) return 6;
	}
	HAL_ICACHE_Enable();
#endif
	return 0;
}

int platform_flash_read(uint32_t addr, uint8_t *src, uint32_t sz)
{
	if(addr < FLASH_START || addr + sz >= FLASH_FINISH) return 1;
	memcpy(src, (void *)addr, sz);
	return 0;
}

void platform_reset(void)
{
#if FW_TYPE == FW_LDR || FW_TYPE == FW_APP
	HAL_PCD_Stop(&hpcd_USB_DRD_FS);
#endif
	platform_deinit();
	NVIC_SystemReset();
}

__attribute__((optimize("-O0"))) __attribute__((always_inline)) static __inline void boot_jump(uint32_t address)
{
	JumpAddress = *(__IO uint32_t *)(address + 4);
	Jump_To_Application = (pFunction)JumpAddress;
	__set_MSP(*(__IO uint32_t *)address);
	Jump_To_Application();
}

__attribute__((optimize("-O0"))) void platform_run_address(uint32_t address)
{
	platform_deinit();
	SCB->VTOR = address;
	boot_jump(address);
}

void platform_deinit(void)
{
	__disable_irq();
	SysTick->CTRL = 0;
	for(uint32_t i = 0; i < sizeof(NVIC->ICPR) / sizeof(NVIC->ICPR[0]); i++)
	{
		NVIC->ICPR[i] = 0xfffffffflu;
	}
	__enable_irq();
}

void platform_get_uid(uint32_t *id)
{
#if FW_TYPE == FW_LDR || FW_TYPE == FW_APP
	HAL_ICACHE_Disable();
	g_uid[0] = HAL_GetUIDw0();
	g_uid[1] = HAL_GetUIDw1();
	g_uid[2] = HAL_GetUIDw2();
	HAL_ICACHE_Enable();
#endif
}