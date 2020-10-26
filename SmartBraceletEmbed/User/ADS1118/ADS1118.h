#ifndef _ADS1118_H_
#define _ADS1118_H_

#include "stm32f7xx_hal.h"

# define CONF_MSB 0b11110011
# define CONF_LSB 0b10101010
# define CONF  CONF_MSB << 8 | CONF_LSB

#define ADS1118_CS_PORT                GPIOC
#define ADS1118_CS_PIN                 GPIO_PIN_3

#define ADS1118_CS_H			HAL_GPIO_WritePin(ADS1118_CS_PORT, ADS1118_CS_PIN,GPIO_PIN_SET)
#define ADS1118_CS_L			HAL_GPIO_WritePin(ADS1118_CS_PORT, ADS1118_CS_PIN,GPIO_PIN_RESET)

void ADS1118_Init(void);
void ADS1118_Read(uint16_t config);

#endif