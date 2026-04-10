################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Lora/lora.c 

OBJS += \
./Core/Src/Lora/lora.o 

C_DEPS += \
./Core/Src/Lora/lora.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Lora/%.o Core/Src/Lora/%.su Core/Src/Lora/%.cyclo: ../Core/Src/Lora/%.c Core/Src/Lora/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Lora

clean-Core-2f-Src-2f-Lora:
	-$(RM) ./Core/Src/Lora/lora.cyclo ./Core/Src/Lora/lora.d ./Core/Src/Lora/lora.o ./Core/Src/Lora/lora.su

.PHONY: clean-Core-2f-Src-2f-Lora

