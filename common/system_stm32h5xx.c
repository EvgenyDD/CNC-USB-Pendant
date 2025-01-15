/**
 *=============================================================================
 *        System Clock source                     | HSI
 *        SYSCLK(Hz)                              | 64000000
 *        HCLK(Hz)                                | 64000000
 *        AHB Prescaler                           | 1
 *        APB1 Prescaler                          | 1
 *        APB2 Prescaler                          | 1
 *        APB3 Prescaler                          | 1
 *        HSI Division factor                     | 1
 *        PLL1_SRC                                | No clock
 *        PLL1_M                                  | Prescaler disabled
 *        PLL1_N                                  | 129
 *        PLL1_P                                  | 2
 *        PLL1_Q                                  | 2
 *        PLL1_R                                  | 2
 *        PLL1_FRACN                              | 0
 *        PLL2_SRC                                | No clock
 *        PLL2_M                                  | Prescaler disabled
 *        PLL2_N                                  | 129
 *        PLL2_P                                  | 2
 *        PLL2_Q                                  | 2
 *        PLL2_R                                  | 2
 *        PLL2_FRACN                              | 0
 *        PLL3_SRC                                | No clock
 *        PLL3_M                                  | Prescaler disabled
 *        PLL3_N                                  | 129
 *        PLL3_P                                  | 2
 *        PLL3_Q                                  | 2
 *        PLL3_R                                  | 2
 *        PLL3_FRACN                              | 0
 *=============================================================================  */

#include "stm32h5xx.h"

#if !defined(HSE_VALUE)
#define HSE_VALUE (25000000UL) // Value of the External oscillator in Hz */
#endif						   /* HSE_VALUE */

#if !defined(CSI_VALUE)
#define CSI_VALUE (4000000UL) // Value of the Internal oscillator in Hz*/
#endif						  /* CSI_VALUE */

#if !defined(HSI_VALUE)
#define HSI_VALUE (64000000UL) // Value of the Internal oscillator in Hz */
#endif						   /* HSI_VALUE */

#define VECT_TAB_OFFSET 0x00U //!< Vector Table base offset field.

uint32_t SystemCoreClock = 64000000U;

const uint8_t AHBPrescTable[16] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 1U, 2U, 3U, 4U, 6U, 7U, 8U, 9U};
const uint8_t APBPrescTable[8] = {0U, 0U, 0U, 0U, 1U, 2U, 3U, 4U};

void SystemInit(void)
{
#if(__FPU_PRESENT == 1) && (__FPU_USED == 1)
	SCB->CPACR |= ((3UL << 20U) | (3UL << 22U)); /* set CP10 and CP11 Full Access */
#endif
	RCC->CR = RCC_CR_HSION;
	RCC->CFGR1 = 0U;
	RCC->CFGR2 = 0U;
	/* Reset HSEON, HSECSSON, HSEBYP, HSEEXT, HSIDIV, HSIKERON, CSION, CSIKERON, HSI48 and PLLxON bits */
#if defined(RCC_CR_PLL3ON)
	RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_HSECSSON | RCC_CR_HSEBYP | RCC_CR_HSEEXT | RCC_CR_HSIDIV | RCC_CR_HSIKERON |
				 RCC_CR_CSION | RCC_CR_CSIKERON | RCC_CR_HSI48ON | RCC_CR_PLL1ON | RCC_CR_PLL2ON | RCC_CR_PLL3ON);
#else
	RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_HSECSSON | RCC_CR_HSEBYP | RCC_CR_HSEEXT | RCC_CR_HSIDIV | RCC_CR_HSIKERON |
				 RCC_CR_CSION | RCC_CR_CSIKERON | RCC_CR_HSI48ON | RCC_CR_PLL1ON | RCC_CR_PLL2ON);
#endif

	RCC->PLL1CFGR = 0U; /* Reset PLLxCFGR register */
	RCC->PLL2CFGR = 0U;
#if defined(RCC_CR_PLL3ON)
	RCC->PLL3CFGR = 0U;
#endif

	RCC->PLL1DIVR = 0x01010280U;  /* Reset PLL1DIVR register */
	RCC->PLL1FRACR = 0x00000000U; /* Reset PLL1FRACR register */
	RCC->PLL2DIVR = 0x01010280U;  /* Reset PLL2DIVR register */
	RCC->PLL2FRACR = 0x00000000U; /* Reset PLL2FRACR register */
#if defined(RCC_CR_PLL3ON)
	RCC->PLL3DIVR = 0x01010280U;  /* Reset PLL3DIVR register */
	RCC->PLL3FRACR = 0x00000000U; /* Reset PLL3FRACR register */
#endif
	RCC->CR &= ~(RCC_CR_HSEBYP); /* Reset HSEBYP bit */
	RCC->CIER = 0U;				 /* Disable all interrupts */

// #ifdef VECT_TAB_SRAM
// 	SCB->VTOR = SRAM1_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM */
// #else
// 	SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH */
// #endif

	/* Check OPSR register to verify if there is an ongoing swap or option bytes update interrupted by a reset */
	uint32_t reg_opsr = FLASH->OPSR & FLASH_OPSR_CODE_OP;
	if((reg_opsr == FLASH_OPSR_CODE_OP) || (reg_opsr == (FLASH_OPSR_CODE_OP_2 | FLASH_OPSR_CODE_OP_1)))
	{
		if((FLASH->OPTCR & FLASH_OPTCR_OPTLOCK) != 0U) /* Check FLASH Option Control Register access */
		{
			FLASH->OPTKEYR = 0x08192A3BU; /* Authorizes the Option Byte registers programming */
			FLASH->OPTKEYR = 0x4C5D6E7FU;
		}
		FLASH->OPTCR |= FLASH_OPTCR_OPTSTART; /* Launch the option bytes change operation */
		FLASH->OPTCR |= FLASH_OPTCR_OPTLOCK;  /* Lock the FLASH Option Control Register access */
	}
}

void SystemCoreClockUpdate(void)
{
	uint32_t pllp, pllsource, pllm, pllfracen, hsivalue, tmp;
	float_t fracn1, pllvco;
	switch(RCC->CFGR1 & RCC_CFGR1_SWS)
	{
	case 0x00UL: SystemCoreClock = (uint32_t)(HSI_VALUE >> ((RCC->CR & RCC_CR_HSIDIV) >> 3)); break; /* HSI used as system clock source */
	case 0x08UL: SystemCoreClock = CSI_VALUE; break;												 /* CSI used as system clock  source */
	case 0x10UL: SystemCoreClock = HSE_VALUE; break;												 /* HSE used as system clock  source */
	case 0x18UL:																					 /* PLL1 used as system clock source */
		/* PLL_VCO = (HSE_VALUE or HSI_VALUE or CSI_VALUE/ PLLM) * PLLN and SYSCLK = PLL_VCO / PLLR */
		pllsource = (RCC->PLL1CFGR & RCC_PLL1CFGR_PLL1SRC);
		pllm = ((RCC->PLL1CFGR & RCC_PLL1CFGR_PLL1M) >> RCC_PLL1CFGR_PLL1M_Pos);
		pllfracen = ((RCC->PLL1CFGR & RCC_PLL1CFGR_PLL1FRACEN) >> RCC_PLL1CFGR_PLL1FRACEN_Pos);
		fracn1 = (float_t)(uint32_t)(pllfracen * ((RCC->PLL1FRACR & RCC_PLL1FRACR_PLL1FRACN) >> RCC_PLL1FRACR_PLL1FRACN_Pos));
		switch(pllsource)
		{
		case 0x01UL: /* HSI used as PLL clock source */
			hsivalue = (HSI_VALUE >> ((RCC->CR & RCC_CR_HSIDIV) >> 3));
			pllvco = ((float_t)hsivalue / (float_t)pllm) * ((float_t)(uint32_t)(RCC->PLL1DIVR & RCC_PLL1DIVR_PLL1N) + (fracn1 / (float_t)0x2000) + (float_t)1);
			break;
		case 0x02UL: /* CSI used as PLL clock source */
			pllvco = ((float_t)CSI_VALUE / (float_t)pllm) * ((float_t)(uint32_t)(RCC->PLL1DIVR & RCC_PLL1DIVR_PLL1N) + (fracn1 / (float_t)0x2000) + (float_t)1);
			break;
		case 0x03UL: /* HSE used as PLL clock source */
			pllvco = ((float_t)HSE_VALUE / (float_t)pllm) * ((float_t)(uint32_t)(RCC->PLL1DIVR & RCC_PLL1DIVR_PLL1N) + (fracn1 / (float_t)0x2000) + (float_t)1);
			break;
		default: pllvco = (float_t)0U; break; /* No clock sent to PLL*/
		}

		pllp = (((RCC->PLL1DIVR & RCC_PLL1DIVR_PLL1P) >> RCC_PLL1DIVR_PLL1P_Pos) + 1U);
		SystemCoreClock = (uint32_t)(float_t)(pllvco / (float_t)pllp);
		break;

	default:
		SystemCoreClock = HSI_VALUE;
		break;
	}
	tmp = AHBPrescTable[((RCC->CFGR2 & RCC_CFGR2_HPRE) >> RCC_CFGR2_HPRE_Pos)];
	SystemCoreClock >>= tmp;
}