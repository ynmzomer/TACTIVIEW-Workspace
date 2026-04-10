################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/MAX30102_Driver/MAX30102.c \
../Core/Src/MAX30102_Driver/MAX30102_user.c 

OBJS += \
./Core/Src/MAX30102_Driver/MAX30102.o \
./Core/Src/MAX30102_Driver/MAX30102_user.o 

C_DEPS += \
./Core/Src/MAX30102_Driver/MAX30102.d \
./Core/Src/MAX30102_Driver/MAX30102_user.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/MAX30102_Driver/%.o Core/Src/MAX30102_Driver/%.su Core/Src/MAX30102_Driver/%.cyclo: ../Core/Src/MAX30102_Driver/%.c Core/Src/MAX30102_Driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-MAX30102_Driver

clean-Core-2f-Src-2f-MAX30102_Driver:
	-$(RM) ./Core/Src/MAX30102_Driver/MAX30102.cyclo ./Core/Src/MAX30102_Driver/MAX30102.d ./Core/Src/MAX30102_Driver/MAX30102.o ./Core/Src/MAX30102_Driver/MAX30102.su ./Core/Src/MAX30102_Driver/MAX30102_user.cyclo ./Core/Src/MAX30102_Driver/MAX30102_user.d ./Core/Src/MAX30102_Driver/MAX30102_user.o ./Core/Src/MAX30102_Driver/MAX30102_user.su

.PHONY: clean-Core-2f-Src-2f-MAX30102_Driver

