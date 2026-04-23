/*
 * lora.c
 *
 *  Created on: Oct 28, 2025
 *      Author: omery
 */

#include "lora.h"


char nextion_msg[32]; //nextiondan loraya giedecek buffer.
char lora_msg[30] ;   //loradan nextiona gidecek buffer.

uint8_t lora_rx_byte ;

uint8_t lora_rx_index = 0 ;

void lora_send_msg(char *msg){
	HAL_NVIC_DisableIRQ(USART1_IRQn);
	HAL_UART_AbortReceive_IT(LORA_UART_HANDLE);
	char dummy[8]; //todo paketi büyütüp tam formatı öğren.
	HAL_UART_Transmit(LORA_UART_HANDLE, (uint8_t*)msg, strlen(msg), LORA_TIMEOUT);
	HAL_UART_Receive(LORA_UART_HANDLE, (uint8_t*)dummy, 8, LORA_TIMEOUT); //dummy sucsess read.
	HAL_NVIC_EnableIRQ(USART1_IRQn);
	HAL_UART_Receive_IT(LORA_UART_HANDLE, &lora_rx_byte, 1);


	//todo hata gelirse ısr ile yap.
}



