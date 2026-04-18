/*
 * lora.h
 *
 *  Created on: Oct 28, 2025
 *      Author: omery
 */

#ifndef SRC_LORA_LORA_H_
#define SRC_LORA_LORA_H_

#include "main.h"

extern UART_HandleTypeDef huart3;

#define LORA_UART_HANDLE (&huart3)

#define LORA_TIMEOUT 500

void lora_send_msg(char *msg);

extern char nextion_msg[20]; //nextiondan loraya giedecek buffer.
extern char lora_msg[30];    //loradan nextiona gidecek buffer.

extern uint8_t lora_rx_byte ;

extern uint8_t lora_rx_index;

#endif /* SRC_LORA_LORA_H_ */
