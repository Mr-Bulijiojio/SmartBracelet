/**
  ******************************************************************************
  * @file    delay.h
  * @brief   This file includes the declarations of delay functions
  * @author  UESTC Yu XuYao
  ******************************************************************************
  */

#ifndef __DELAY_H__
#define __DELAY_H__

#include "stm32f767xx.h"
#include "core_cm7.h"
#include "stm32f7xx_hal.h"

void delay_init(uint16_t SYSCLK);
void delay_ms(uint16_t nms);
void delay_us(uint32_t nus);

#endif