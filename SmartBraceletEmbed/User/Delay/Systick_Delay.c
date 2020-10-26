/**
  ******************************************************************************
  * @file    delay.c
  * @brief   This file includes the prototypes of delay functions
  * @author  UESTC Yu XuYao
  ******************************************************************************
  */

/* Private includes ----------------------------------------------------------*/
#include "Systick_Delay.h"


/* Private variables ---------------------------------------------------------*/

/*us delay multiplication Multiplier*/
static uint32_t fac_us = 0;

/**
  * @brief Delay Initialization Function
  * @param SYSCLK : Frequency of MCU(MHz) eg.STM32F767 SYSCLK = 216
  * @retval None
  */
void delay_init(uint16_t SYSCLK)
{
    /*Set Freq of SysTick to HCLK*/
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    fac_us = SYSCLK;
}

/**
  * @brief Microsecond Delay Function
  * @param nus : number of Microsecond
  * @retval None
  */
void delay_us(uint32_t nus)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD;
    ticks = nus * fac_us;
    told = SysTick->VAL;
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
                tcnt += told - tnow;
            else
                tcnt += reload - tnow + told;
            told = tnow;
            if (tcnt >= ticks)
                break;
        }
    };
}

/**
  * @brief Millisecond Delay Function
  * @param nus : number of Millisecond
  * @retval None
  */
void delay_ms(uint16_t nms)
{
    uint32_t i;
    for (i = 0; i < nms; i++)
        delay_us(1000);
}

/*原子sys.c移植更改
void WFI_SET(void)
{
	asm("WFI");		  
}

void INTX_DISABLE(void)
{		
	asm("CPSID   I");	
	asm("BX      LR");	
}

void INTX_ENABLE(void)
{
	asm("CPSIE   I");	
	asm("BX      LR");	
}

void MSR_MSP(uint32_t addr) 
{
	asm("MSR MSP, r0"); 	//set Main Stack value
	asm("BX r14");	
}
*/
