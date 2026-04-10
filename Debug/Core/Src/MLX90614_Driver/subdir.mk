################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/MLX90614_Driver/mlx90614.c 

OBJS += \
./Core/Src/MLX90614_Driver/mlx90614.o 

C_DEPS += \
./Core/Src/MLX90614_Driver/mlx90614.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/MLX90614_Driver/%.o Core/Src/MLX90614_Driver/%.su Core/Src/MLX90614_Driver/%.cyclo: ../Core/Src/MLX90614_Driver/%.c Core/Src/MLX90614_Driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-MLX90614_Driver

clean-Core-2f-Src-2f-MLX90614_Driver:
	-$(RM) ./Core/Src/MLX90614_Driver/mlx90614.cyclo ./Core/Src/MLX90614_Driver/mlx90614.d ./Core/Src/MLX90614_Driver/mlx90614.o ./Core/Src/MLX90614_Driver/mlx90614.su

.PHONY: clean-Core-2f-Src-2f-MLX90614_Driver

