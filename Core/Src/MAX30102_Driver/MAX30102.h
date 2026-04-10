/*
 * MAX30102.h
 *
 *  Created on: Oct 7, 2025
 *      Author: omery
 */

#ifndef SRC_MAX30102_H_
#define SRC_MAX30102_H_

#include "MAX30102defs.h"
#include "main.h"

extern I2C_HandleTypeDef hi2c1 ;

#define I2C_HANDLE    (&hi2c1)


#define MAX30102_ADDRESS  0x57   // 7-bit I2C adresi

#define MAX30102_Timeout 100

typedef enum {
    MAX30102_OK,
    MAX30102_ERROR,
} MAX30102_Status_t;

typedef enum {
    MAX30102_MODE_HR_ONLY   = 0x02,
    MAX30102_MODE_SPO2      = 0x03,
    MAX30102_MODE_MULTI_LED = 0x07,
} MAX30102_Mode_t;

typedef struct {
    MAX30102_Mode_t mode;
    MAX30102_LED_PW_t led_pw; //Daha uzun pulse width → daha fazla ışık → daha yüksek ADC çözünürlüğü.
    MAX30102_SampleRate_t sample_rate; //The sample rate is how many samples per second you want.
    MAX30102_ADC_Range_t adc_range; //The pulse width (LED_PW) is how long each LED stays on during measurement. Longer pulses give higher resolution.
    MAX30102_LEDCurrent_t led_current_ir;
    MAX30102_LEDCurrent_t led_current_red;
    uint8_t int_en;
} MAX30102_Handle_t;

// Fonksiyon prototipleri
MAX30102_Status_t max30102_read_reg(uint8_t reg, uint8_t *data);
MAX30102_Status_t max30102_write_reg(uint8_t reg, uint8_t data);
MAX30102_Status_t max30102_init(MAX30102_Handle_t *dev);
MAX30102_Status_t max30102_read_fifo(uint32_t *ir_data, uint32_t *red_data, uint8_t *num_samples);
MAX30102_Status_t max30102_int_enable(uint8_t int_en);
MAX30102_Status_t max30102_read_fifo_multi(uint32_t *ir_buffer,
                                           uint32_t *red_buffer,
                                           uint8_t *num_samples);
void max30102_reset(void);




#endif /* SRC_MAX30102_H_ */
