/*
 * bme680_user.c
 *
 *  Created on: Jul 28, 2025
 *      Author: omery
 */

#include "bme680_user.h"



static struct bme68x_dev bme;              // BME680/BME688 main device handler (contains interface + function pointers)
static struct bme68x_conf conf;            // Sensor configuration (oversampling, filter, ODR settings)
static struct bme68x_heatr_conf heatr_conf; // Gas heater configuration (heating temp + duration)
static uint8_t n_fields;                   // Number of valid data fields returned by the last measurement
static int8_t rslt;                        // Result status of BME68X API calls (BME68X_OK = success)



//==========HELPER FUNCTIONS========///
/**
 * @brief  I2C read callback function for sensor drivers (e.g., BME680 / BMP280).
 *
 * This function is called by the sensor driver. It performs an I2C memory read
 * from the given register address and stores the received data in @p reg_data.
 *
 * @param[in]  reg_addr   Register address to read from.
 * @param[out] reg_data   Pointer to the buffer that will store the received data.
 * @param[in]  length     Number of bytes to read.
 * @param[in]  intf_ptr   User-provided I2C interface pointer (cast to I2C_HandleTypeDef*).
 *
 * @return  0 on success, -1 on error.
 */
int8_t user_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t length, void *intf_ptr) {
    return (HAL_I2C_Mem_Read((I2C_HandleTypeDef *)intf_ptr, 0x76 << 1, reg_addr,
                             I2C_MEMADD_SIZE_8BIT, reg_data, length, BME680_TIMEOUT) == HAL_OK) ? 0 : -1;
}

/**
 * @brief  I2C write callback function for sensor drivers (e.g., BME680 / BMP280).
 *
 * This function is called by the sensor driver. It performs an I2C memory write
 * to the specified register address with @p reg_data as the data source.
 *
 * @param[in]  reg_addr   Register address to write to.
 * @param[in]  reg_data   Pointer to the data buffer to be written.
 * @param[in]  length     Number of bytes to write.
 * @param[in]  intf_ptr   User-provided I2C interface pointer (cast to I2C_HandleTypeDef*).
 *
 * @return  0 on success, -1 on error.
 */
int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t length, void *intf_ptr) {
    return (HAL_I2C_Mem_Write((I2C_HandleTypeDef *)intf_ptr, 0x76 << 1, reg_addr,
                              I2C_MEMADD_SIZE_8BIT, (uint8_t *)reg_data, length, BME680_TIMEOUT) == HAL_OK) ? 0 : -1;
}

/**
 * @brief  Delay function required by the sensor driver (microsecond resolution).
 *
 * Converts the requested microsecond delay into milliseconds and uses HAL_Delay.
 * Note: HAL_Delay has millisecond resolution, so this function rounds up.
 *
 * @param[in]  period     Delay duration in microseconds.
 * @param[in]  intf_ptr   Unused parameter (only present for API compatibility).
 */
void user_delay_us(uint32_t period, void *intf_ptr) {
    HAL_Delay((period + 999) / 1000); // convert µs → ms (rounded)
}


/**
 * @brief  Initializes the BME680/BME688 sensor (user-side initialization).
 *
 * This function is called from `main()` and performs the following steps:
 *   1. Ensures the I2C bus is in a valid state (reinitializes if stuck as BUSY).
 *   2. Assigns I2C interface, read/write/delay callback functions to the driver.
 *   3. Performs sensor initialization and soft reset.
 *   4. Configures oversampling, filter, and heater settings.
 *
 * Uses the user-defined callbacks:
 *   - user_i2c_read()
 *   - user_i2c_write()
 *   - user_delay_us()
 *
 * @note  This function assumes that `bme`, `conf`, `heatr_conf`, and `rslt`
 *        are available as global/static variables (as required by Bosch API).
 *
 * @param  None
 * @retval None
 */
void bme680_user_init(void)
{
    /* 1. Check if I2C bus is locked (BUSY state may occur after reset or noise).
       If BUSY, deinitialize and reinitialize the I2C peripheral. */
    if (__HAL_I2C_GET_FLAG(I2C_HANDLE, I2C_FLAG_BUSY)) {
        HAL_I2C_DeInit(I2C_HANDLE);
        HAL_I2C_Init(I2C_HANDLE);
    }

    /* Assign interface type and user-defined callbacks */
    bme.intf     = BME68X_I2C_INTF;
    bme.read     = user_i2c_read;
    bme.write    = user_i2c_write;
    bme.delay_us = user_delay_us;
    bme.intf_ptr = I2C_HANDLE;
    bme.chip_id  = 0x61;    // Expected BME688/680 ID (0x61)

    /* Initialize sensor */
    rslt = bme68x_init(&bme);

    /* Soft-reset to ensure clean state */
    bme68x_soft_reset(&bme);

    /* Optional user error handler */
    if (rslt != BME68X_OK) {
#ifdef Error_Handle
        sensors_error_handler(rslt);
#endif
    }

    /* Configure oversampling, filter and ODR (output data rate) */
    conf.os_temp = BME68X_OS_2X;
    conf.os_pres = BME68X_OS_2X;
    conf.os_hum  = BME68X_OS_4X;
    conf.filter  = BME68X_FILTER_SIZE_3;
    conf.odr     = BME68X_ODR_NONE ;

    rslt = bme68x_set_conf(&conf, &bme);

    /* Configure heater (only used in forced mode) */
    heatr_conf.enable     = BME68X_ENABLE;
    heatr_conf.heatr_temp = 300;   // °C
    heatr_conf.heatr_dur  = 100;   // ms

    rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme);

    if (rslt != BME68X_OK) {
#ifdef Error_Handle
        sensors_error_handler(rslt);
#endif
    }
}


/**
 * @brief  Performs a single forced-mode measurement from the BME680/BME688.
 *
 * This function triggers a measurement (forced mode), waits for completion,
 * retrieves temperature, humidity, pressure, and gas resistance values,
 * and additionally scales the pressure into hPa.
 *
 * @param[out] data       Pointer to BME68X data structure that will store:
 *                        - data->temperature   (°C)
 *                        - data->humidity      (% RH)
 *                        - data->pressure      (hPa)
 *                        - data->gas_resistance (Ohm)
 *
 * @note  Must be called repeatedly in application loop for new measurements.
 * @note  Standby time (ODR) is not used in forced mode.
 *
 * @retval None. Error handling must be implemented by user (TODO section).
 */
void bme680_read_withiaq(struct bme68x_data *data)
{
    /* Start measurement */
    bme68x_set_op_mode(BME68X_FORCED_MODE, &bme);

    /* Read result */
    rslt = bme68x_get_data(BME68X_FORCED_MODE, data, &n_fields, &bme);

    if (rslt == BME68X_OK && n_fields > 0)
    {
        /* Convert to hPa (from Pa) */
        data->pressure = data->pressure / 100.0f;
    }
    else
    {
        // TODO: Add user error handler here (optional)
        // sensors_error_handler(rslt);
    }
}


/**
 * @brief  Converts BME680/BME688 gas resistance reading into a simple air quality label.
 *
 * This function applies a simplified IAQ (Indoor Air Quality) estimation algorithm
 * based on gas resistance. It does **not** use the Bosch BSEC library. The logic:
 *
 *      - Higher gas resistance → cleaner air
 *      - Lower gas resistance → polluted air
 *
 * A reference gas resistance of ~10 kΩ (clean air) is used. IAQ is scaled to 0–500.
 *
 * @param[in]  data        Pointer to the latest BME68X measurement data structure.
 *                         Must contain a valid `gas_resistance` value (Ohms).
 *
 * @param[out] quality     Pointer to character buffer where the text result is written.
 *                         The buffer must be at least 20 bytes.
 *
 * @note  This is NOT the official IAQ algorithm. For real IAQ (CO2-eq, VOC-eq),
 *        use Bosch BSEC library.
 *
 * @retval None
 */
void air_quality(struct bme68x_data *data, char* quality)
{
    float gas_kohm = data->gas_resistance / 1000.0f;  // Convert Ohm → kΩ

    /* Simple IAQ calculation (not Bosch official) */
    float iaq = (10.0f / gas_kohm) * 100.0f;
    if (iaq > 500.0f)
        iaq = 500.0f;  // Cap value to max IAQ range

    /* Assign air quality category */
    if (iaq <= 50)
        snprintf(quality, 20, "Perfect");
    else if (iaq <= 100)
        snprintf(quality, 20, "Good");
    else if (iaq <= 150)
        snprintf(quality, 20, "Moderate");
    else if (iaq <= 200)
        snprintf(quality, 20, "Poor");
    else if (iaq <= 300)
        snprintf(quality, 20, "Very Bad");
    else
        snprintf(quality, 20, "Hazardous");
}

/**
 * @brief  Attempts to recover the BME680/BME688 sensor if it becomes unresponsive.
 *
 * This function is typically called when I2C communication errors occur or
 * the sensor returns invalid data. It performs the following recovery steps:
 *
 *   1. Checks if the I2C bus is stuck in BUSY state and reinitializes the bus if needed.
 *   2. Attempts to read the CHIP_ID register to verify sensor presence.
 *   3. If the sensor is not responding, performs a soft reset.
 *   4. Re-runs the user-defined initialization routine.
 *
 * The function tries up to 5 recovery attempts. If any attempt succeeds,
 * the function returns `true`. If all attempts fail, it returns `false`.
 *
 * @note This function does NOT block for long periods; delays are minimal.
 * @note Designed to be safe to call inside main loop or watchdog recovery flows.
 *
 * @retval true   Sensor recovered and reinitialized successfully.
 * @retval false  Sensor could not be recovered (likely disconnected or hardware fault).
 */
bool bme680_fault_recovery(void)
{
    for (int attempt = 0; attempt < 5; attempt++)
    {
        /* 1. Check I2C bus state */
        if (__HAL_I2C_GET_FLAG(I2C_HANDLE, I2C_FLAG_BUSY))
        {
            HAL_I2C_DeInit(I2C_HANDLE);
            HAL_I2C_Init(I2C_HANDLE);
        }

        /* 2. Check if sensor responds (read CHIP ID) */
        uint8_t chip_id = 0;
        if (bme.read(BME68X_REG_CHIP_ID, &chip_id, 1, bme.intf_ptr) == 0 &&
            chip_id == BME68X_CHIP_ID)
        {
            /* Sensor is responding again */
            bme680_user_init();
            return true;
        }

        /* 3. Attempt soft reset if sensor does not respond */
        if (bme68x_soft_reset(&bme) == BME68X_OK)
        {
            HAL_Delay(10);
            bme680_user_init();
            return true;
        }

        HAL_Delay(200);  // Wait and retry
    }

    /* All attempts failed → sensor likely disconnected */
    return false;
}

