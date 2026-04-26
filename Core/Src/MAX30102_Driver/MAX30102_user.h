/*
 * MAX30102_user.h
 *
 *  Created on: Oct 8, 2025
 *      Author: omery
 */

#ifndef SRC_MAX30102_USER_H_
#define SRC_MAX30102_USER_H_

// ================== Ayarlar ==================
#define SAMPLE_RATE_HZ       200u     // MAX30102 konfig ile eşleşmeli
#define WARMUP_MS            3500u    // bilek için ~3.5 sn ısınma (ilk değer biraz daha erken)
#define MAX_PEAK_HISTORY     9u       // 9 interval → medyan artefakta karşı sağlam
#define MIN_RR_MS            375u     // refrakter süre (>=375ms → <=160 BPM; artefakt engeli)
#define INIT_THRESHOLD       8.0f     // başlangıç eşiği (adaptif güncellenir)

// Temas algılama (AC genlik E-MA’sına göre)
#define AMP_ON_THRESH        10.0f    // temas var demek için min AC genlik
#define AMP_OFF_THRESH       8.0f     // temas yok demek için AC alt sınır
#define HOLD_ON_MS           700u     // bu kadar ms üstünde kalırsa "var" (yanlış temas engeli)
#define HOLD_OFF_MS          800u     // bu kadar ms altında kalırsa "yok"

// ================== Filtre Parametreleri ==================
#define SPS_IR               200.0f
#define TAU_BASELINE_S       2.0f
#define FC_LPF_HZ            3.0f    // 180 BPM = 3 Hz; üstü gürültü

#include "main.h"
#include "MAX30102.h"
#include <stdio.h>
#include <string.h>

extern uint32_t ir_buf[32];
extern uint32_t red_buf[32];
extern uint8_t num_samples;
void max30102_user_init(void);
void max30102_user_read(uint8_t* bpm, uint8_t* spo2);
void filter_ir_block(const uint32_t *in, float *out, uint8_t n);
uint8_t detect_peak_and_bpm(float sample);
void update_finger_detection(float sample);


extern UART_HandleTypeDef huart2;


typedef enum{
	PULSE_FINGER_NOT_DETECTED,
	PULSE_IN_PROCESS,
	PULSE_FUNCTION_REPEATED
}PULSE_State_t;

/*
 * SPO2
 */
typedef struct
{
    float ir_mean;
    float red_mean;
    float ac_ir;
    float ac_red;
} spo2_debug_t;

extern volatile spo2_debug_t spo2_dbg;

#endif /* SRC_MAX30102_USER_H_ */
