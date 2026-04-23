/*
 * screen.h
 *
 *  Created on: Oct 26, 2025
 *      Author: omery
 */

#ifndef SRC_SCREEN_SCREEN_H_
#define SRC_SCREEN_SCREEN_H_

#include "main.h"

#define SCREEN_TIMEOUT 100

#define START_CHAR 0x24 // $
#define PAGE_CHAR  0x31 //'1'
#define END_CHAR   0x26 // '&'

extern UART_HandleTypeDef huart3;

#define SCREEN_UART_HANDLE (&huart3)

void NX_send_raw(const void *buf, size_t len);

void NX_term(void);

void NX_send_cmd(const char *cmd);

void NX_set_txt(const char *obj, const char *text);

void NX_set_data(const char *obj, const int16_t data);

void NX_set_float(const char *obj, const float data);

void NX_set_val(const char *obj, int16_t data);

void NX_Init(void);

extern const char *box_pulse           ;
extern const char *box_spo2            ;
extern const char *box_body_temp       ;
extern const char *box_enviroment_temp ;
extern const char *box_humidity 	   ;
extern const char *box_air_quality 	   ;

extern uint8_t screen_rx_byte;
extern uint8_t screen_state;
extern uint8_t screen_comp;


#endif /* SRC_SCREEN_SCREEN_H_ */
