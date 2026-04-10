/*
 * MAX30102defs.h
 *
 *  Created on: Oct 7, 2025
 *      Author: omery
 */

#ifndef SRC_MAX30102DEFS_H_
#define SRC_MAX30102DEFS_H_



// --- Register adresleri ---
#define REG_INTR_STATUS_1    0x00
#define REG_INTR_STATUS_2    0x01
#define REG_INTR_ENABLE_1    0x02
#define REG_INTR_ENABLE_2    0x03
#define REG_FIFO_WR_PTR      0x04
#define REG_OVF_COUNTER      0x05
#define REG_FIFO_RD_PTR      0x06
#define REG_FIFO_DATA        0x07
#define REG_FIFO_CONFIG      0x08
#define REG_MODE_CONFIG      0x09
#define REG_SPO2_CONFIG      0x0A
#define REG_LED1_PA          0x0C   // IR LED
#define REG_LED2_PA          0x0D   // RED LED
#define REG_MULTI_LED_CTRL1  0x11
#define REG_MULTI_LED_CTRL2  0x12
#define REG_TEMP_INTR        0x1F
#define REG_TEMP_FRAC        0x20
#define REG_TEMP_CONFIG      0x21
#define REG_PART_ID          0xFF   // = 0x15

// --- ENUM Tanımları ---

// Örnekleme hızları
typedef enum {
    MAX30102_SAMPLERATE_50   = 0x00,
    MAX30102_SAMPLERATE_100  = 0x01,
    MAX30102_SAMPLERATE_200  = 0x02,
    MAX30102_SAMPLERATE_400  = 0x03,
    MAX30102_SAMPLERATE_800  = 0x04,
    MAX30102_SAMPLERATE_1000 = 0x05,
    MAX30102_SAMPLERATE_1600 = 0x06,
    MAX30102_SAMPLERATE_3200 = 0x07
} MAX30102_SampleRate_t;

// ADC Range
typedef enum {
    MAX30102_ADCRANGE_2048  = 0x00,
    MAX30102_ADCRANGE_4096  = 0x01,
    MAX30102_ADCRANGE_8192  = 0x02,
    MAX30102_ADCRANGE_16384 = 0x03
} MAX30102_ADC_Range_t;

// LED Pulse Width
typedef enum {
    MAX30102_PW_69   = 0x00, // 15-bit
    MAX30102_PW_118  = 0x01, // 16-bit
    MAX30102_PW_215  = 0x02, // 17-bit
    MAX30102_PW_411  = 0x03  // 18-bit
} MAX30102_LED_PW_t;

// LED Current (Pulse Amplitude)
typedef enum {
    MAX30102_LED_CURR_0MA   = 0x00,
    MAX30102_LED_CURR_4MA   = 0x1F,
    MAX30102_LED_CURR_8MA   = 0x3F,
    MAX30102_LED_CURR_12MA  = 0x5F,
    MAX30102_LED_CURR_16MA  = 0x7F,
    MAX30102_LED_CURR_20MA  = 0x9F,
    MAX30102_LED_CURR_24MA  = 0xBF,
    MAX30102_LED_CURR_27MA  = 0xDF,
    MAX30102_LED_CURR_31MA  = 0xFF
} MAX30102_LEDCurrent_t;

// Interrupt bits
#define MAX30102_INT_A_FULL   0x80
#define MAX30102_INT_PPG_RDY  0x40
#define MAX30102_INT_ALC_OVF  0x20
#define MAX30102_INT_DIE_TEMP 0x02



#endif /* SRC_MAX30102DEFS_H_ */
