
#include "ret_mem.h"
#include "stm32h5xx.h"

void ret_mem_init(void)
{
	RCC->APB3ENR |= RCC_APB3ENR_RTCAPBEN;
	RCC->BDCR |= RCC_BDCR_RTCEN;
	PWR->DBPCR |= PWR_DBPCR_DBP;
	while((PWR->DBPCR & PWR_DBPCR_DBP) == 0)
		;
}

load_src_t ret_mem_get_load_src(void)
{
	return TAMP->BKP0R;
}

void ret_mem_set_load_src(load_src_t src)
{
	TAMP->BKP0R = src;
}

void ret_mem_set_bl_stuck(bool state)
{
	TAMP->BKP1R = state ? 0xAA : 0;
}

int ret_mem_is_bl_stuck(void)
{
	return TAMP->BKP1R == 0xAA;
}