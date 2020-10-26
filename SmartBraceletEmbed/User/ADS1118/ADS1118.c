#include "ADS1118.h"
#include "Systick_Delay.h"
#include "spi.h"
#include <stdio.h>
#include "arm_math.h"
#include "ADS1292.h"

void ADS1118_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure = {
		.Mode			= GPIO_MODE_OUTPUT_PP,
		.Pull			= GPIO_NOPULL,
		.Speed		= GPIO_SPEED_FREQ_LOW,
	};
	
	GPIO_Initure.Pin			= ADS1118_CS_PIN;
	HAL_GPIO_Init(ADS1118_CS_PORT,&GPIO_Initure);
	HAL_Delay(10);
	ADS1118_CS_L;
	ADS1118_Read(CONF);
}

union DATA
{
	int16_t data;
	uint8_t seg[2];
};

union DATA data_buff;

#define   a1                           -0.193
#define   b1                           212.009
#define   Temperature_1Order(n)        a1*pow(n,1)+b1

#define   a2                           -7.857923E-06
#define   b2                           -1.777501E-01
#define   c2                           2.046398E+02
#define   Temperature_2Order(n)        a2*pow(n,2)+b2*pow(n,1)+c2

#define   a3                           -1.809628E-09
#define   b3                           -3.325395E-06
#define   c3                           -1.814103E-01
#define   d3                           2.055894E+02
#define   Temperature_3Order(n)        a3*pow(n,3)+b3*pow(n,2)+c3*pow(n,1)+d3

void ADS1118_Read(uint16_t config)
{
	float32_t Temp = 0;
	uint8_t tx[2] = { 0, 0 };
	uint8_t tmp[2];
	tx[0] = (uint8_t)config & 0x00FF;
	tx[1] = (uint8_t)(config>>8);

    ADS1118_CS_L;
    delay_us(10);
	HAL_SPI_TransmitReceive(&hspi2, tx, data_buff.seg, 1, 10);
	HAL_SPI_TransmitReceive(&hspi2, tx, tmp, 1, 10);
    delay_us(10);
    ADS1118_CS_H;
	if (config != 0)
	{
		HAL_NVIC_DisableIRQ(EXTI3_IRQn);
		Temp = data_buff.data * 4.096*2*1000/65536;
		Temp = Temperature_2Order(Temp);
		
		/*High temperature alarm*/
		if (Temp > 31)
		{
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
		}
		else
		{
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
		}
		
		printf("Temp:%.3f\r\n", Temp);
		HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(EXTI3_IRQn);
	}
		
}
