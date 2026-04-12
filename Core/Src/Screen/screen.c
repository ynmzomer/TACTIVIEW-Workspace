/*
 * screen.c
 *
 *  Created on: Oct 26, 2025
 *      Author: omery
 */

#include "screen.h"

const char *box_pulse 			= "t7" ;
const char *box_spo2 			= "t8" ;
const char *box_body_temp 		= "t10";
const char *box_enviroment_temp = "t12";
const char *box_humidity 		= "t16";
const char *box_air_quality 	= "t19";

uint8_t screen_rx_byte;
uint8_t screen_state = 0;
uint8_t screen_page = 0;
uint8_t screen_comp = 0;
uint8_t screen_handle = 0 ;


void NX_send_raw(const void *buf, size_t len)
{
	HAL_UART_Transmit(SCREEN_UART_HANDLE, (uint8_t*)buf, len, SCREEN_TIMEOUT);
}

//Nextion protokolünde her komut 3 tane 0xFF ile bitmek zorunda.
void NX_term(void)
{
	const uint8_t end[3] = {0xFF, 0xFF, 0xFF};
	NX_send_raw(end, 3);
}

//Bir string komut alır (cmd), önce onu yollar (NX_send_raw), sonra da sonuna otomatik FF FF FF ekler.
void NX_send_cmd(const char *cmd)
{
	NX_send_raw(cmd, strlen(cmd));
	NX_term();
}

//Bir text objesinin (t1, t2, …) içeriğini değiştirmek. snprintf ile komutu hazırlar:
void NX_set_txt(const char *obj, const char *text)
{
	char cmd[64];
	int n = snprintf(cmd, sizeof(cmd), "%s.txt=\"%s\"", obj, text);
	if (n > 0 )
	{
		NX_send_cmd(cmd);
	}
}
void NX_set_data(const char *obj, int16_t data)
{
	char cmd[64];
	int n = snprintf(cmd, sizeof(cmd), "%s.txt=\"%d\"", obj, data);
	if (n > 0 )
	{
		NX_send_cmd(cmd);
	}
}
void NX_set_float(const char *obj, float data)
{
	char cmd[64];
	int n = snprintf(cmd, sizeof(cmd), "%s.txt=\"%.2f\"", obj, data);
	if (n > 0 )
	{
		NX_send_cmd(cmd);
	}
}

void NX_Init(void)
{
	NX_send_cmd("bkcmd=0"); //Geri dönüş kodlarını aç
	NX_send_cmd("page page0");
	NX_send_cmd("n0.val=0");

}
