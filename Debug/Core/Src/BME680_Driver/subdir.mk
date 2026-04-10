################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/BME680_Driver/bme680_user.c \
../Core/Src/BME680_Driver/bme68x.c 

OBJS += \
./Core/Src/BME680_Driver/bme680_user.o \
./Core/Src/BME680_Driver/bme68x.o 

C_DEPS += \
./Core/Src/BME680_Driver/bme680_user.d \
./Core/Src/BME680_Driver/bme68x.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/BME680_Driver/%.o Core/Src/BME680_Driver/%.su Core/Src/BME680_Driver/%.cyclo: ../Core/Src/BME680_Driver/%.c Core/Src/BME680_Driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-BME680_Driver

clean-Core-2f-Src-2f-BME680_Driver:
	-$(RM) ./Core/Src/BME680_Driver/bme680_user.cyclo ./Core/Src/BME680_Driver/bme680_user.d ./Core/Src/BME680_Driver/bme680_user.o ./Core/Src/BME680_Driver/bme680_user.su ./Core/Src/BME680_Driver/bme68x.cyclo ./Core/Src/BME680_Driver/bme68x.d ./Core/Src/BME680_Driver/bme68x.o ./Core/Src/BME680_Driver/bme68x.su

.PHONY: clean-Core-2f-Src-2f-BME680_Driver

