/**
  ******************************************************************************
  * @file    delay.h
  * @brief   This file includes the declarations of usart3 functions and macro
  *          definition of usart3To make this file compatible with ALIENTEK 
  *          Apollo, usart3 should be changedinto usart1.
  * @author  UESTC Yu XuYao
  ******************************************************************************
  */
#ifndef _USART1_H_
#define _USART1_H_

#include "stm32f767xx.h"
#include "stm32f7xx_hal.h"
#include <stdio.h>

/*Define the maximum number of received bytes*/
#define USART1_REC_LEN  			200
/*Enable or disable usart reception*/
#define EN_USART1_RX 			1

/*RX Buffer*/
extern uint8_t  USART1_RX_BUF[USART1_REC_LEN];
/*Receive status tag*/
extern volatile uint16_t USART1_RX_STA;
/*Handler of USART3*/
extern UART_HandleTypeDef UART1_Handler;

#define USART1_RXBUFFERSIZE   1
/*receive buffer used by HAL library*/
extern uint8_t USART1_aRxBuffer[USART1_RXBUFFERSIZE];

/* Definition for USARTx clock resources */
#define USARTx                           USART1
#define USARTx_CLK_ENABLE()              __HAL_RCC_USART1_CLK_ENABLE();
#define USARTx_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

#define USARTx_FORCE_RESET()             __HAL_RCC_USART1_FORCE_RESET()
#define USARTx_RELEASE_RESET()           __HAL_RCC_USART1_RELEASE_RESET()

/* Definition for USARTx Pins */
#define USARTx_TX_PIN                    GPIO_PIN_9
#define USARTx_TX_GPIO_PORT              GPIOA
#define USARTx_TX_AF                     GPIO_AF7_USART1
#define USARTx_RX_PIN                    GPIO_PIN_10
#define USARTx_RX_GPIO_PORT              GPIOA
#define USARTx_RX_AF                     GPIO_AF7_USART1

/*Initialization function of USART3*/
void USART1_Init(uint32_t bound);
/*UART MSP Initialization.*/
void HAL_UART_MspInit(UART_HandleTypeDef *huart);
#endif