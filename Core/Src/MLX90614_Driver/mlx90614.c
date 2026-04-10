/*
 * mlx90614.c
 *
 *  Created on: Sep 24, 2025
 *      Author: omery
 */

#include "mlx90614.h"

MLX90614_Status_t mlx90614_init(void){
	MLX90614_Status_t status = MLX90614_COM_ERROR;
	status = MLX90614_ID_Check();
	if(status != MLX90614_OK){
		MLX90614_Error_Handler(MLX90614_COM_ERROR);
	}
	return status;
}

MLX90614_Status_t MLX90614_Config(MLX90614_Config_t *config , uint8_t do_config){
	MLX90614_Status_t status = MLX90614_COM_ERROR;
	if(do_config){
		uint16_t data ; //0x9FB4 factory default
		status = MLX90614_Read(MLX90614_EEPROM_CONFIG, (uint8_t*)&data, 2);

		if(status != MLX90614_OK){
			MLX90614_Error_Handler(MLX90614_COM_ERROR);
			return status ;
		}

		uint16_t erase = 0x0000;
		status = MLX90614_Write(MLX90614_EEPROM_CONFIG, (uint8_t*)&erase, 2);
		HAL_Delay(10); //10ms delay.
		if(status != MLX90614_OK){
			MLX90614_Error_Handler(MLX90614_COM_ERROR);
			return status ;
		}

		status = MLX90614_Read(MLX90614_EEPROM_CONFIG, (uint8_t*)&erase, 2);
		if(status != MLX90614_OK){
			MLX90614_Error_Handler(MLX90614_COM_ERROR);
			return status ;
		}
		if(erase != 0x0000){
			MLX90614_Error_Handler(MLX90614_EEPROM_ERROR);
			return status ;
		}

		data &= ~(uint16_t)((0x7 << 8) | (0x7 << 0)); //self test eklenmedi.

		uint16_t user_config = data | (config->FIR_FILTER | config->IIR_FILTER | config->SELF_TEST)  ;

		MLX90614_Write(MLX90614_EEPROM_CONFIG, (uint8_t*)&user_config, 2);
		//todo ERROR Handler eklenmedi.
		HAL_Delay(10); //10ms delay.
		MLX90614_Read(MLX90614_EEPROM_CONFIG, (uint8_t*)&data, 2);
		if(user_config != data){
			MLX90614_Error_Handler(MLX90614_EEPROM_ERROR);
		}
	}
	return status;
}




void mlx90614_read_temp(float* temp){
	MLX90614_Status_t status;
	uint16_t temp_k ;
	static uint16_t last_temp_k ;
	status = MLX90614_Read(MLX90614_OBJECT_TEMP, (uint8_t*)&temp_k, 2);
	if(status != MLX90614_OK){
		MLX90614_Error_Handler(MLX90614_COM_ERROR) ;
	}
	if(temp_k <= 0){
		temp_k = last_temp_k ;

	}else{
		last_temp_k = temp_k ;
	}
	*temp = (temp_k * 0.02) - 273.15;
}



MLX90614_Status_t MLX90614_Write(uint8_t reg_addr, uint8_t *pdata, uint8_t size){
	HAL_StatusTypeDef status ;
	status = HAL_I2C_Mem_Write(I2C_HANDLE, MLX90614_ADDR << 1 , reg_addr, 1, pdata, size, MAX_TIMEOUT);
	if(status != HAL_OK){
		return MLX90614_ERROR;
	}
	return MLX90614_OK;
}

MLX90614_Status_t MLX90614_Read(uint8_t reg_addr, uint8_t *pdata, uint8_t size){
	HAL_StatusTypeDef status ;
	status = HAL_I2C_Mem_Read(I2C_HANDLE, MLX90614_ADDR << 1, reg_addr , 1, pdata, size , MAX_TIMEOUT);
	if(status != HAL_OK){
		return MLX90614_ERROR;
	}
	return MLX90614_OK;

}

MLX90614_Status_t MLX90614_ID_Check(){
	HAL_StatusTypeDef status ;
	uint8_t id[2];
	status = HAL_I2C_Mem_Read(I2C_HANDLE, MLX90614_ADDR << 1, MLX90614_EEPROM_SLAVE_ADDR , 1, id, 2 , MAX_TIMEOUT);
	if(status != HAL_OK){
		return MLX90614_ERROR;
	}
	if(id[0] != MLX90614_ADDR){
		return MLX90614_ERROR;
	}
	return MLX90614_OK;

}

//fonksyiona hatanın nerde gerçekleştiği ile ilgili enum gönder.
__weak void MLX90614_Error_Handler(uint8_t error_code){

}
