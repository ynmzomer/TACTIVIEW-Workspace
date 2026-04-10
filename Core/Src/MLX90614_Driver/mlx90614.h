/*
 * mlx90614.h
 *
 *  Created on: Sep 24, 2025
 *      Author: omery
 */

#ifndef SRC_MLX90614_MLX90614_H_
#define SRC_MLX90614_MLX90614_H_

#include "main.h"

extern I2C_HandleTypeDef hi2c1;

#define I2C_HANDLE    (&hi2c1)


#define MAX_TIMEOUT 					HAL_MAX_DELAY
#define MLX90614_ADDR					(uint8_t)0x5A

/*====================================
 * MLX90614 EEPROM REGISTER ADRESSES
 *===================================
 */
#define MLX90614_EEPROM_TOMAX 			(uint8_t)0x20
#define MLX90614_EEPROM_TOMIN 			(uint8_t)0x21
#define MLX90614_EEPROM_TARANGE 		(uint8_t)0x23
#define MLX90614_EEPROM_EMISSIVITY 		(uint8_t)0x24
#define MLX90614_EEPROM_CONFIG 			(uint8_t)0x25
#define MLX90614_EEPROM_SLAVE_ADDR		(uint8_t)0x2E


/*====================================
 * MLX90614 RAM REGISTER ADRESSES
 *===================================
 */
#define MLX90614_RAW_DATA				(uint8_t)0x04
#define MLX90614_AMBIENT_TEMP			(uint8_t)0x06
#define MLX90614_OBJECT_TEMP			(uint8_t)0x07

/* ============================
 * MLX90614 Config Register Bitfields
 * ============================ */

/* IIR filter settings (Bits B15..B13) */
#define MLX90614_IIR_50        (0x0 << 0)   // a1=0.5, b=0.5
#define MLX90614_IIR_25        (0x1 << 0)   // a1=0.25, b=0.75
#define MLX90614_IIR_17        (0x2 << 0)   // a1=0.166, b=0.833
#define MLX90614_IIR_13        (0x3 << 0)   // a1=0.125, b=0.875
#define MLX90614_IIR_100       (0x4 << 0)   // a1=1, b=0
#define MLX90614_IIR_80        (0x5 << 0)   // a1=0.8, b=0.2
#define MLX90614_IIR_67        (0x6 << 0)   // a1=0.666, b=0.333
#define MLX90614_IIR_57        (0x7 << 0)   // a1=0.571, b=0.428

/* Repeat Sensor Self-Test (Bit B12) */
#define MLX90614_SELFTEST_OFF  (0 << 15)
#define MLX90614_SELFTEST_ON   (1 << 15)


/* Ks sign (Bit B8) */
#define MLX90614_KS_POSITIVE   (0 << 7)
#define MLX90614_KS_NEGATIVE   (1 << 7)

/* FIR filter settings (Bits B7..B5) */
#define MLX90614_FIR_8         (0x0 << 8)   // Not recommended
#define MLX90614_FIR_16        (0x1 << 8)   // Not recommended
#define MLX90614_FIR_32        (0x2 << 8)   // Not recommended
#define MLX90614_FIR_64        (0x3 << 8)   // Not recommended
#define MLX90614_FIR_128       (0x4 << 8)
#define MLX90614_FIR_256       (0x5 << 8)
#define MLX90614_FIR_512       (0x6 << 8)
#define MLX90614_FIR_1024      (0x7 << 8)

/* Amplifier Gain (Bits B4..B2) */
#define MLX90614_GAIN_BYPASS   (0x0 << 11)
#define MLX90614_GAIN_3        (0x1 << 11)
#define MLX90614_GAIN_6        (0x2 << 11)
#define MLX90614_GAIN_12_5     (0x3 << 11)
#define MLX90614_GAIN_25       (0x4 << 11)
#define MLX90614_GAIN_50       (0x5 << 11)
#define MLX90614_GAIN_100      (0x6 << 11)   // 0x6 and 0x7 both = 100
#define MLX90614_GAIN_100B     (0x7 << 11)

/* Kt2 sign (Bit B1) */
#define MLX90614_KT2_POSITIVE  (0 << 14)
#define MLX90614_KT2_NEGATIVE  (1 << 14)

/* Sensor self-test enable (Bit B0) */
#define MLX90614_SELFTEST_EN   (0 << 0)
#define MLX90614_SELFTEST_DIS  (1 << 0)



typedef enum{
	MLX90614_ERROR,
	MLX90614_OK
}MLX90614_Status_t;

typedef enum{
	MLX90614_COM_ERROR,
	MLX90614_EEPROM_ERROR
}MLX90614_ERROR_t;



typedef struct{
	uint16_t IIR_FILTER ;
	uint16_t FIR_FILTER ;
	uint16_t SELF_TEST ;
}MLX90614_Config_t;



MLX90614_Status_t mlx90614_init(void);

void mlx90614_read_temp(float* temp);

MLX90614_Status_t MLX90614_ID_Check();

MLX90614_Status_t MLX90614_Write(uint8_t reg_addr, uint8_t *pdata, uint8_t size);

MLX90614_Status_t MLX90614_Read(uint8_t reg_addr, uint8_t *pdata, uint8_t size);

MLX90614_Status_t MLX90614_Config(MLX90614_Config_t *config , uint8_t do_config);

//todo implement CRC
MLX90614_Status_t MLX90614_PEC_Check();

void MLX90614_Error_Handler(uint8_t error_code);








#endif /* SRC_MLX90614_MLX90614_H_ */
