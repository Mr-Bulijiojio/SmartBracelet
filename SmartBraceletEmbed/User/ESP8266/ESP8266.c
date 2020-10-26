#include "ESP8266.h"
#include "USART1.h"
#include <string.h>
#include "Systick_Delay.h"


uint8_t * ESP8266_check_cmd(uint8_t *str)
{
	
	char *strx = 0;
	if (USART1_RX_STA & 0X8000)
		{ 
			USART1_RX_BUF[USART1_RX_STA & 0X7FFF] = 0;
			strx = strstr((const char*)USART1_RX_BUF, (const char*)str);
		} 
	return (uint8_t*)strx;
}

uint8_t ESP8266_send_cmd(uint8_t *cmd, uint8_t *ack, uint16_t waittime)
{
	uint8_t res = 0; 
	USART1_RX_STA = 0;
	printf("%s\r\n", cmd); 	//发送命令
	if(ack&&waittime)		//需要等待应答
	{
		while (--waittime)	//等待倒计时
			{
				HAL_Delay(10);
				if (USART1_RX_STA & 0X8000)//接收到期待的应答结果
				{
					if (ESP8266_check_cmd(ack))
					{
//						printf("ack:%s\r\n", (uint8_t*)ack);
						break;//得到有效数据 
					}
					USART1_RX_STA = 0;
				} 
			}
		if (waittime == 0)res = 1; 
	}
	return res;
} 

uint8_t ESP8266_send_data(uint8_t *data, uint8_t *ack, uint16_t waittime)
{
	uint8_t res = 0; 
	USART1_RX_STA = 0;
	printf("%s", data); 	//发送命令
	if(ack&&waittime)		//需要等待应答
	{
		while (--waittime)	//等待倒计时
			{
				HAL_Delay(10);
				if (USART1_RX_STA & 0X8000)//接收到期待的应答结果
				{
					if (ESP8266_check_cmd(ack))break;//得到有效数据 
					USART1_RX_STA = 0;
				} 
			}
		if (waittime == 0)res = 1; 
	}
	return res;
}
