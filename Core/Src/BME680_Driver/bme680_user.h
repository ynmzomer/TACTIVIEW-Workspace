/*
 * bme680_user.h
 *
 *  Created on: Oct 20, 2025
 *      Author: omery
 */

#ifndef SRC_BME680_DRIVER_BME680_USER_H_
#define SRC_BME680_DRIVER_BME680_USER_H_


#include <stdbool.h>
#include <stdio.h>

#include "bme68x.h"
#include "bme68x_defs.h"

#include "stm32f4xx_hal.h"
#include "main.h"

extern I2C_HandleTypeDef hi2c1 ;
#define I2C_HANDLE    (&hi2c1)

#define BME680_TIMEOUT 100

extern UART_HandleTypeDef huart2;

int8_t user_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t length, void *intf_ptr);

int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t length, void *intf_ptr);

void user_delay_us(uint32_t period, void *intf_ptr);

void bme680_user_init(void);

void bme680_read_withiaq(struct bme68x_data *data);

void air_quality(struct bme68x_data *data, char* quality);

bool bme680_fault_recovery(void);




#endif /* SRC_BME680_DRIVER_BME680_USER_H_ */
