#include "platform.h"

void platform_reset(void)
{
	NVIC_SystemReset();
}