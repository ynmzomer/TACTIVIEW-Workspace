/*
 * MAX30102.c
 *
 *  Created on: Oct 7, 2025
 *      Author: omery
 */



#include "MAX30102.h"
#include "main.h"

/**
 * @brief  Reads a single register from the MAX30102 sensor via I2C.
 *
 * This function communicates with the MAX30102 over I2C and reads
 * one byte of data from the specified register address.
 *
 * @param  reg   Register address to read from.
 * @param  data  Pointer to a variable where the read data will be stored.
 *
 * @retval MAX30102_OK     Register successfully read.
 * @retval MAX30102_ERROR  I2C communication failed.
 *
 * @note   Uses HAL_I2C_Mem_Read() internally.
 */
MAX30102_Status_t max30102_read_reg(uint8_t reg, uint8_t *data) {
    if (HAL_I2C_Mem_Read(I2C_HANDLE, MAX30102_ADDRESS << 1, reg,
                         I2C_MEMADD_SIZE_8BIT, data, 1, MAX30102_Timeout) == HAL_OK) {
        return MAX30102_OK;
    }
    return MAX30102_ERROR;
}


/**
 * @brief  Writes a single byte to a MAX30102 register via I2C.
 *
 * This function sends 1 byte of data to the specified register
 * address on the MAX30102 sensor using I2C communication.
 *
 * @param  reg   Register address to write to.
 * @param  data  Data byte to be written into the register.
 *
 * @retval MAX30102_OK     Data successfully written.
 * @retval MAX30102_ERROR  I2C communication failed.
 *
 * @note   Uses HAL_I2C_Mem_Write() internally.
 */
MAX30102_Status_t max30102_write_reg(uint8_t reg, uint8_t data) {
    if (HAL_I2C_Mem_Write(I2C_HANDLE, MAX30102_ADDRESS << 1, reg,
                          I2C_MEMADD_SIZE_8BIT, &data, 1, MAX30102_Timeout) == HAL_OK) {
        return MAX30102_OK;
    }
    return MAX30102_ERROR;
}

/**
 * @brief  Initializes the MAX30102 sensor with given configuration parameters.
 *
 * This function performs the full initialization sequence of the MAX30102 sensor,
 * including:
 *   - Sensor reset
 *   - PART ID verification (expected: 0x15)
 *   - Mode configuration (HR, SpO2, or Multi-LED mode depending on dev->mode)
 *   - SpO2 configuration (sample rate, ADC range, LED pulse width)
 *   - LED current amplitude setup (IR and Red LEDs)
 *   - FIFO settings (sample averaging, rollover, FIFO threshold)
 *
 * @param  dev   Pointer to MAX30102_Handle_t struct containing configuration
 *               parameters such as:
 *               - mode
 *               - adc_range
 *               - sample_rate
 *               - led_pw
 *               - led_current_ir
 *               - led_current_red
 *
 * @retval MAX30102_OK     Initialization completed successfully.
 * @retval MAX30102_ERROR  Sensor did not respond, wrong PART ID, or register write failed.
 *
 * @note   This function performs multiple I2C register writes and delays.
 * @note   Expected PART ID value is 0x15 for MAX30102.
 */
MAX30102_Status_t max30102_init(MAX30102_Handle_t *dev) {
    max30102_reset();
    uint8_t part_id = 0;
    MAX30102_Status_t status;

    if (max30102_read_reg(REG_PART_ID, &part_id) != MAX30102_OK) return MAX30102_ERROR;
    if (part_id != 0x15) return MAX30102_ERROR; // MAX30102 ID kontrolü

    // Reset
    status = max30102_write_reg(REG_MODE_CONFIG, 0x40);
    HAL_Delay(10);
    if (status != MAX30102_OK) {
        return status;
    }

    // Mode configuration
    status = max30102_write_reg(REG_MODE_CONFIG, dev->mode);
    HAL_Delay(10);
    if (status != MAX30102_OK) {
        status = max30102_write_reg(REG_MODE_CONFIG, dev->mode);
        return status;
    }

    // SPO2 config: sample rate, ADC range, pulse width
    uint8_t spo2_cfg = (dev->adc_range << 5) | (dev->sample_rate << 2) | (dev->led_pw);
    status = max30102_write_reg(REG_SPO2_CONFIG, spo2_cfg);
    HAL_Delay(10);
    if (status != MAX30102_OK) {
        return status;
    }

    // LED amplitudes
    status = max30102_write_reg(REG_LED1_PA, dev->led_current_ir);
    max30102_write_reg(REG_LED2_PA, dev->led_current_red);
    HAL_Delay(10);
    if (status != MAX30102_OK) {
        return status;
    }

    // FIFO configuration
    uint8_t fifo_cfg = (0x03 << 5) | (1 << 4) | (0x0F);
    status = max30102_write_reg(REG_FIFO_CONFIG, fifo_cfg);
    HAL_Delay(10);
    if (status != MAX30102_OK) {
        return status;
    }

    return status;
}

/**
 * @brief  Reads one sample (IR + RED) from MAX30102 FIFO buffer.
 *
 * This function reads 6 bytes from the FIFO Data Register of the MAX30102
 * using I2C. Each sample consists of:
 *    - 3 bytes for IR LED measurement
 *    - 3 bytes for RED LED measurement
 *
 * The MAX30102 stores FIFO data in 18-bit format. The upper 2 bits of each
 * sample are not used (only 0..17 bits are meaningful), therefore the values
 * are masked with 0x3FFFF.
 *
 * @param[out] ir_data       Pointer to store the extracted IR sample (18-bit)
 * @param[out] red_data      Pointer to store the extracted RED sample (18-bit)
 * @param[out] num_samples   Pointer to the number of samples read (always 1 in this implementation)
 *
 * @retval MAX30102_OK       FIFO read and decoding successful.
 * @retval MAX30102_ERROR    I2C read failed (bus busy, sensor not responding, etc).
 *
 * @note   This function reads only one FIFO slot. If multiple samples are buffered,
 *         call this function repeatedly until FIFO is empty.
 * @note   Uses HAL_I2C_Mem_Read() internally.
 */
MAX30102_Status_t max30102_read_fifo(uint32_t *ir_data, uint32_t *red_data, uint8_t *num_samples) {
    uint8_t fifo_data[6];
    if (HAL_I2C_Mem_Read(I2C_HANDLE, MAX30102_ADDRESS << 1, REG_FIFO_DATA,
                         I2C_MEMADD_SIZE_8BIT, fifo_data, 6, 100) != HAL_OK) {
        return MAX30102_ERROR;
    }

    *ir_data  = ((uint32_t)fifo_data[0] << 16 | (uint32_t)fifo_data[1] << 8 | fifo_data[2]) & 0x3FFFF;
    *red_data = ((uint32_t)fifo_data[3] << 16 | (uint32_t)fifo_data[4] << 8 | fifo_data[5]) & 0x3FFFF;
    *num_samples = 1;

    return MAX30102_OK;
}


/**
 * @brief  Reads multiple IR + RED samples from MAX30102 FIFO buffer.
 *
 * This function reads the FIFO Write Pointer (WR_PTR) and Read Pointer (RD_PTR)
 * to determine how many unread samples are currently stored in the FIFO.
 *
 * MAX30102 FIFO behavior:
 *  - FIFO has 32 slots (0..31)
 *  - Each slot contains 6 bytes (3 bytes IR + 3 bytes RED)
 *  - When WR_PTR reaches the end, it wraps around (circular buffer)
 *
 * Function workflow:
 *  1. Read WR_PTR and RD_PTR
 *  2. Calculate the number of samples in FIFO (considering wrap-around)
 *  3. Read and merge each (IR, RED) sample into 18-bit values
 *
 * Example:
 *  If WR_PTR = 5, RD_PTR = 2 → samples = 3
 *  If WR_PTR = 1, RD_PTR = 30 → samples = (1 - 30 + 32) = 3 (wrap-around)
 *
 * @param[out] ir_buffer     Pointer to buffer that will hold IR LED samples (18-bit values)
 * @param[out] red_buffer    Pointer to buffer that will hold RED LED samples (18-bit values)
 * @param[out] num_samples   Returns how many samples were read from FIFO
 *
 * @retval MAX30102_OK       Operation successful, FIFO data was read.
 * @retval MAX30102_ERROR    I2C read error or register read failure occurred.
 *
 * @note The FIFO can store up to 32 samples. Ensure the buffer size is >= 32.
 * @note Uses HAL_I2C_Mem_Read() internally.
 */
MAX30102_Status_t max30102_read_fifo_multi(uint32_t *ir_buffer,
                                           uint32_t *red_buffer,
                                           uint8_t *num_samples)
{
    uint8_t wr_ptr, rd_ptr;
    if (max30102_read_reg(REG_FIFO_WR_PTR, &wr_ptr) != MAX30102_OK) return MAX30102_ERROR; // write ptr read
    if (max30102_read_reg(REG_FIFO_RD_PTR, &rd_ptr) != MAX30102_OK) return MAX30102_ERROR; // read ptr read

    // FIFO’daki örnek sayısı (wrap-around düzeltilmiş hali)
    int8_t samples = wr_ptr - rd_ptr;
    if (samples < 0) samples += 32;   // wrap-around correction (FIFO size is 32)

    *num_samples = samples;

    for (uint8_t i = 0; i < samples; i++) {
        uint8_t fifo_data[6];
        if (HAL_I2C_Mem_Read(I2C_HANDLE, MAX30102_ADDRESS << 1, REG_FIFO_DATA,
                             I2C_MEMADD_SIZE_8BIT, fifo_data, 6, 100) != HAL_OK) {
            return MAX30102_ERROR;
        }

        ir_buffer[i]  = ((uint32_t)fifo_data[0] << 16 | (uint32_t)fifo_data[1] << 8 | fifo_data[2]) & 0x03FFFF; // 18-bit IR
        red_buffer[i] = ((uint32_t)fifo_data[3] << 16 | (uint32_t)fifo_data[4] << 8 | fifo_data[5]) & 0x03FFFF; // 18-bit RED
    }

    return MAX30102_OK;
}


/**
 * @brief  Performs a software reset on the MAX30102 sensor.
 *
 * This function triggers a soft reset by setting the RESET bit (bit6) in the
 * MODE_CONFIG register. MAX30102 automatically clears the RESET bit once the
 * reset sequence has completed.
 *
 * Reset process:
 *   1. Write '1' to RESET bit (MODE_CONFIG bit6)
 *   2. Poll the register until the RESET bit clears back to '0'
 *
 * @note  This is a software-only reset. If the sensor does not respond
 *        (e.g., SDA stuck, I2C bus locked), a hard reset or power cycle may be required.
 *
 * @retval None
 */
void max30102_reset(void)
{
    uint8_t data = 0;

    /* 1. Set reset bit (MODE_CONFIG bit6) */
    data = (1 << 6) | (0 << 7);
    max30102_write_reg(REG_MODE_CONFIG, data);

    /* 2. Wait until reset bit clears */
    do {
        max30102_read_reg(REG_MODE_CONFIG, &data);
    } while (data & (1 << 6));
}




// --- Interrupt enable ---
MAX30102_Status_t max30102_int_enable(uint8_t int_en) {
    return max30102_write_reg(REG_INTR_ENABLE_1, int_en);
}

