#ifndef  _ESP8266_H_
#define _ESP8266_H_

#include <stdint.h>

uint8_t * ESP8266_check_cmd(uint8_t *str);
uint8_t ESP8266_send_cmd(uint8_t *cmd, uint8_t *ack, uint16_t waittime);
uint8_t ESP8266_send_data(uint8_t *data, uint8_t *ack, uint16_t waittime);


#endif // ! _ESP8266_H_
