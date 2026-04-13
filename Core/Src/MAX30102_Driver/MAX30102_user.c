/*
 * MAX30102_user.c
 *
 *  Created on: Oct 8, 2025
 *      Author: omery
 */
/*
 * MAX30102_user.c
 * BPM ölçümü (HAL_GetTick tabanlı, parmak algılama, 2s warmup, 5–10 peak ortalaması)
 */

#include "MAX30102_user.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

// ---- harici semboller ----



// ================== Filtre Durumları ==================
static uint8_t  ir_inited   = 0u;
static float    ir_baseline = 0.0f;   // DC takip
static float    ir_lpf      = 0.0f;   // AC pürüzsüz çıkış (grafiğe bunu çizebilirsin)

// ================== Peak/BPM Durumları ==================
static uint8_t  finger_detected       = 0u;
static uint32_t finger_state_ts_ms    = 0u;   // parmak VAR/YOK bekleme sayaç başlangıcı
static uint32_t finger_on_since_ms    = 0u;   // parmak "VAR" olduğu anın damgası

static float    amp_abs_ema           = 0.0f; // |AC| E-MA (parmak algılama için)

static uint8_t  peak_detect_state     = 1u;   // 1: peak arıyorum, 0: düşüş bekliyorum
static float    peak_threshold        = INIT_THRESHOLD;
static float    prev_sample           = 0.0f;
static float    detected_peak_value   = 0.0f;

static uint32_t last_peak_ms          = 0u;   // HAL_GetTick ms
static uint8_t  peaks_ignored         = 0u;   // ilk 3 peak'i yok say
static uint8_t  peak_hist_count       = 0u;
static uint32_t peak_intervals_ms[MAX_PEAK_HISTORY]; // son N RR (ms)

static float    bpm_ema               = 0.0f; // yumuşatma //RESULT!!!!!!
static uint8_t  bpm_ema_inited        = 0u;

// ================== I/O Bufferları ==================
uint32_t ir_buf[32];
uint32_t red_buf[32];
float    ir_filtered[32];
uint8_t  num_samples = 0;

//================ SPO2 PART =================================
#define SPO2_BUF_SIZE 50

static uint32_t spo2_ir_buf[SPO2_BUF_SIZE];
static uint32_t spo2_red_buf[SPO2_BUF_SIZE];
static uint16_t spo2_index = 0;
static uint8_t spo2_ready = 0;

volatile spo2_debug_t spo2_dbg;

static void spo2_push_sample(uint32_t ir, uint32_t red)
{
    spo2_ir_buf[spo2_index] = ir;
    spo2_red_buf[spo2_index] = red;
    spo2_index++;

    if (spo2_index >= SPO2_BUF_SIZE)
    {
        spo2_index = 0;
        spo2_ready = 1;
    }
}

static uint8_t spo2_compute(void)
{
    float ir_mean = 0.0f;
    float red_mean = 0.0f;

    for (int i = 0; i < SPO2_BUF_SIZE; i++)
    {
        ir_mean  += spo2_ir_buf[i];
        red_mean += spo2_red_buf[i];
    }

    ir_mean  /= SPO2_BUF_SIZE;
    red_mean /= SPO2_BUF_SIZE;

    if (ir_mean < 50000.0f || red_mean < 20000.0f)
        return 0;

    float ir_ac = 0.0f;
    float red_ac = 0.0f;

    for (int i = 0; i < SPO2_BUF_SIZE; i++)
    {
        float ir  = (float)spo2_ir_buf[i]  - ir_mean;
        float red = (float)spo2_red_buf[i] - red_mean;
        ir_ac  += ir * ir;
        red_ac += red * red;
    }

    ir_ac  = sqrtf(ir_ac / SPO2_BUF_SIZE);
    red_ac = sqrtf(red_ac / SPO2_BUF_SIZE);

    if (ir_ac <= 1.0f || red_ac <= 1.0f)
        return 0;
    if (ir_ac > ir_mean || red_ac > red_mean)
        return 0;

    float R = (red_ac / red_mean) / (ir_ac / ir_mean);

    if (isnan(R) || isinf(R))
        return 0;

    float spo2 = 110.0f - 6.0f * R;
    spo2 += 2.0f; /* simple calibration offset */

    if (spo2 > 100.0f) spo2 = 100.0f;
    if (spo2 < 80.0f)  spo2 = 80.0f;

    spo2_dbg.ir_mean  = ir_mean;
    spo2_dbg.red_mean = red_mean;
    spo2_dbg.ac_ir    = ir_ac;
    spo2_dbg.ac_red   = red_ac;

    return (uint8_t)(spo2 + 0.5f);
}

static void spo2_reset(void)
{
    spo2_index = 0;
    spo2_ready = 0;

    for (int i = 0; i < SPO2_BUF_SIZE; i++)
    {
        spo2_ir_buf[i] = 0;
        spo2_red_buf[i] = 0;
    }

    spo2_dbg.ir_mean = 0;
    spo2_dbg.red_mean = 0;
    spo2_dbg.ac_ir = 0;
    spo2_dbg.ac_red = 0;
}

static inline float ema_beta(void) {
    float beta = 0.10f;               // DC takip hızı (0.01–0.2 arası deneyebilirsin)
    if (beta > 1.0f) beta = 1.0f;
    return beta;
}
static inline float ema_alpha(void) {
    float Ts = 1.0f / SPS_IR;
    float w  = 2.0f * 3.1415926f * FC_LPF_HZ * Ts;
    return w / (1.0f + w);            // tek kutuplu LPF EMA eşdeğeri
}

// ----------------------------------------------------
// Sensor Init Application call from main.
// ----------------------------------------------------
void max30102_user_init(void)
{
    static uint8_t retry_count = 0u;

    MAX30102_Handle_t max30102;
    max30102.mode            = MAX30102_MODE_SPO2;
    max30102.sample_rate     = MAX30102_SAMPLERATE_200;
    max30102.adc_range       = MAX30102_ADCRANGE_16384;
    max30102.led_pw          = MAX30102_PW_411;            // 18-bit
    max30102.led_current_ir  = MAX30102_LED_CURR_16MA;     // 12–20 mA arası deneyebilirsin eski değer 16
    max30102.led_current_red = MAX30102_LED_CURR_16MA;     // SpO2 için 16mA gerekli

    if (max30102_init(&max30102) != MAX30102_OK) {
        if (retry_count < 3u) {
            retry_count++;
            HAL_I2C_DeInit(I2C_HANDLE);
            max30102_reset();
            max30102_user_init();
        }
        /* retry_count >= 3: stop, sensor unavailable */
    } else {
        retry_count = 0u; /* reset on success */
    }
}

// ----------------------------------------------------
// Polling okuma (main loop içinde çağır)
// ----------------------------------------------------
void max30102_user_read(uint8_t* bpm, uint8_t* spo2)
{
    static uint8_t measured_bpm = 0;
    static uint8_t measured_spo2 = 0;
    static float spo2_filtered = 0;

    if (max30102_read_fifo_multi(ir_buf, red_buf, &num_samples) == MAX30102_OK)
    {
        filter_ir_block(ir_buf, ir_filtered, num_samples);

        for (uint8_t i = 0; i < num_samples; i++)
        {
            uint8_t bpm_val = detect_peak_and_bpm(ir_filtered[i]);

            if (bpm_val > 3)
            {
                measured_bpm = bpm_val;
            }
            else if (bpm_val == PULSE_FINGER_NOT_DETECTED)
            {
                measured_bpm = 0;
            }

            /* finger detection using IR level */
            if (ir_buf[i] > 50000)
            {
                spo2_push_sample(ir_buf[i], red_buf[i]);
            }
            else
            {
                measured_spo2 = 0;
                spo2_reset();
            }
        }

        /* compute SpO2 only when buffer full */
        if (spo2_ready)
        {
            uint8_t spo2_val = spo2_compute();

            if (spo2_val > 0)
            {
                spo2_filtered = 0.8f * spo2_filtered + 0.2f * spo2_val;
                measured_spo2 = (uint8_t)spo2_filtered;
            }
        }
    }

    *bpm  = measured_bpm;
    *spo2 = measured_spo2;
}

// ----------------------------------------------------
// Tekli okuma (debug amaçlı)
// ----------------------------------------------------
void max30102_user_readfifo_single(void)
{
    uint32_t ir, red;
    uint8_t  samples;
    if (max30102_read_fifo(&ir, &red, &samples) == MAX30102_OK) {

    }
}


// ----------------------------------------------------
// Filtre: blok işleme
// ----------------------------------------------------
void filter_ir_block(const uint32_t *in, float *out, uint8_t n)
{
    const float beta  = ema_beta();
    const float alpha = ema_alpha();

    if (!ir_inited && n > 0u) {
        ir_baseline = (float)in[0];
        ir_lpf      = 0.0f;
        ir_inited   = 1u;
    }

    for (uint8_t i = 0u; i < n; i++) {
        float x  = (float)in[i];
        ir_baseline += beta * (x - ir_baseline);  // DC takip
        float ac = x - ir_baseline;               // HP eşdeğeri
        ir_lpf += alpha * (ac - ir_lpf);          // AC LPF
        out[i] = ir_lpf;                          // ~0 etrafında ±100..±300
    }
}

// ----------------------------------------------------
// Parmak Algılama (AC genlik E-MA + hold süreleri)
// ----------------------------------------------------
void update_finger_detection(float sample)
{
    const float k = 0.05f; // |AC| için EMA hızı
    float absx = fabsf(sample);
    amp_abs_ema = (1.0f - k) * amp_abs_ema + k * absx;

    uint32_t now = HAL_GetTick();

    if (!finger_detected) {
        if (amp_abs_ema > AMP_ON_THRESH) {
            if (finger_state_ts_ms == 0u) finger_state_ts_ms = now;
            if ((now - finger_state_ts_ms) >= HOLD_ON_MS) {
                finger_detected    = 1u;
                finger_on_since_ms = now;
                // başlangıç koşulları reset: hızlı stabilizasyon
                peak_detect_state  = 1u;
                prev_sample        = 0.0f;
                last_peak_ms       = 0u;
                peaks_ignored      = 0u;
                peak_hist_count    = 0u;
                bpm_ema            = 0.0f;
                bpm_ema_inited     = 0u;
                peak_threshold     = INIT_THRESHOLD;
            }
        } else {
            finger_state_ts_ms = 0u; // tekrar say
        }
    } else {
        // parmak var → düşerse
        if (amp_abs_ema < AMP_OFF_THRESH) {
            if (finger_state_ts_ms == 0u) finger_state_ts_ms = now;
            if ((now - finger_state_ts_ms) >= HOLD_OFF_MS) {
                finger_detected    = 0u;
                finger_state_ts_ms = 0u;
                // Peak sistemini pasifleştir (çıkış üretme)
                peak_detect_state  = 1u;
                last_peak_ms       = 0u;
                peak_hist_count    = 0u;
                peaks_ignored      = 0u;
                bpm_ema            = 0.0f;
                bpm_ema_inited     = 0u;
                peak_threshold     = INIT_THRESHOLD;
            }
        } else {
            finger_state_ts_ms = 0u;
        }
    }
}

// ----------------------------------------------------
// Peak Detection + BPM
// ----------------------------------------------------
uint8_t detect_peak_and_bpm(float sample)
{
    // Parmak algılama/logik
    update_finger_detection(sample);
    if (!finger_detected) {
        prev_sample = sample;
        return PULSE_FINGER_NOT_DETECTED; // parmak yok → peak kapalı
    }

    // Warm-up: parmak takıldıktan sonra kısa bekleme (2s)
    uint32_t now = HAL_GetTick();
    if ((now - finger_on_since_ms) < WARMUP_MS) {
        prev_sample = sample;
        return PULSE_IN_PROCESS;
    }

    // 1) Tepe adayına giriş (yükselen kenar ve eşik üstü)
    if(peak_detect_state == 1u && sample > peak_threshold && prev_sample < sample){
    	detected_peak_value = sample;
    	peak_detect_state = 0u; //düşüş bekleniyor

    }
    // 2) Peak onayı (zirveden düşüşe geçiş)
    if (peak_detect_state == 0u && sample < prev_sample) {
        uint32_t t_ms = now;
        uint32_t dt   = (last_peak_ms == 0u) ? 0u : (t_ms - last_peak_ms);

        // Refrakter süre kontrolü saçma tepe aralıklarını atlar
        if (last_peak_ms != 0u && dt < MIN_RR_MS) {
            // çok yakın; artefakt say → yeni peak ara
            peak_detect_state = 1u;
            prev_sample = sample;
            return PULSE_IN_PROCESS;
        }

        // Peak kabul
        last_peak_ms      = t_ms;
        peak_detect_state = 1u;

        // İlk birkaç peak'i tamamen yok say → stabil başla
        if (peaks_ignored < 3u) {
            peaks_ignored++; //overflow yapar mı
            // adaptif threshold'u yine de güncelleyelim (daha rahat yakalasın)
            float new_thr = 0.25f * detected_peak_value;
            if (new_thr < 30.0f) new_thr = 30.0f; // burada min threshold 30 olabilir böyle ayarlanmış.
            peak_threshold = new_thr;
            prev_sample = sample;
            return PULSE_IN_PROCESS;
        }

        // FIFO yaz (dt=0 ise ilk gerçek peak; atla)
        if (dt > 0u) {
            if (peak_hist_count < MAX_PEAK_HISTORY) {
                peak_intervals_ms[peak_hist_count++] = dt;
            } else {
                for (uint8_t i = 1u; i < MAX_PEAK_HISTORY; i++)
                    peak_intervals_ms[i-1u] = peak_intervals_ms[i];
                peak_intervals_ms[MAX_PEAK_HISTORY-1u] = dt;
            }
        }

        // Adaptif threshold (agresif)
        float new_thr = 0.25f * detected_peak_value;
        if (new_thr < 30.0f) new_thr = 30.0f; // burada min threshold 30 olabilir böyle ayarlanmış.
        peak_threshold = new_thr;

        // FIFO dolunca BPM hesapla — medyan tabanlı (artefakt interval'a bağışıklı)
        if (peak_hist_count == MAX_PEAK_HISTORY) {
            /* insertion sort: 7 eleman için en uygun küçük-dizi sıralaması */
            uint32_t sorted[MAX_PEAK_HISTORY];
            for (uint8_t i = 0u; i < MAX_PEAK_HISTORY; i++) sorted[i] = peak_intervals_ms[i];
            for (uint8_t i = 1u; i < MAX_PEAK_HISTORY; i++) {
                uint32_t key = sorted[i];
                int8_t   j   = (int8_t)i - 1;
                while (j >= 0 && sorted[j] > key) {
                    sorted[j + 1u] = sorted[j];
                    j--;
                }
                sorted[j + 1u] = key;
            }
            float bpm = 60000.0f / (float)sorted[MAX_PEAK_HISTORY / 2u];

            // EMA ile yumuşat
            if (!bpm_ema_inited) { bpm_ema_inited = 1u; bpm_ema = bpm; }
            else {
                const float k = 0.30f; // medyan sonrası hızlı yakınsama
                bpm_ema = bpm_ema + k * (bpm - bpm_ema);
            }

            return (uint8_t)(bpm_ema + 0.5f);
        }
    }

   prev_sample = sample;
   return PULSE_FUNCTION_REPEATED;
}




//==============call from gpio ISR (interrupt mode)===============

void sensor_read_fifo_isr(){

}

