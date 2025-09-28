################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Assets/Src/air.c \
../Assets/Src/air30.c \
../Assets/Src/background.c \
../Assets/Src/earth.c \
../Assets/Src/earth30.c \
../Assets/Src/fire.c \
../Assets/Src/fire30.c \
../Assets/Src/light.c \
../Assets/Src/light30.c \
../Assets/Src/shadow.c \
../Assets/Src/shadow30.c \
../Assets/Src/water.c \
../Assets/Src/water30.c 

OBJS += \
./Assets/Src/air.o \
./Assets/Src/air30.o \
./Assets/Src/background.o \
./Assets/Src/earth.o \
./Assets/Src/earth30.o \
./Assets/Src/fire.o \
./Assets/Src/fire30.o \
./Assets/Src/light.o \
./Assets/Src/light30.o \
./Assets/Src/shadow.o \
./Assets/Src/shadow30.o \
./Assets/Src/water.o \
./Assets/Src/water30.o 

C_DEPS += \
./Assets/Src/air.d \
./Assets/Src/air30.d \
./Assets/Src/background.d \
./Assets/Src/earth.d \
./Assets/Src/earth30.d \
./Assets/Src/fire.d \
./Assets/Src/fire30.d \
./Assets/Src/light.d \
./Assets/Src/light30.d \
./Assets/Src/shadow.d \
./Assets/Src/shadow30.d \
./Assets/Src/water.d \
./Assets/Src/water30.d 


# Each subdirectory must supply rules for building sources it contributes
Assets/Src/%.o Assets/Src/%.su Assets/Src/%.cyclo: ../Assets/Src/%.c Assets/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Assets/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Assets-2f-Src

clean-Assets-2f-Src:
	-$(RM) ./Assets/Src/air.cyclo ./Assets/Src/air.d ./Assets/Src/air.o ./Assets/Src/air.su ./Assets/Src/air30.cyclo ./Assets/Src/air30.d ./Assets/Src/air30.o ./Assets/Src/air30.su ./Assets/Src/background.cyclo ./Assets/Src/background.d ./Assets/Src/background.o ./Assets/Src/background.su ./Assets/Src/earth.cyclo ./Assets/Src/earth.d ./Assets/Src/earth.o ./Assets/Src/earth.su ./Assets/Src/earth30.cyclo ./Assets/Src/earth30.d ./Assets/Src/earth30.o ./Assets/Src/earth30.su ./Assets/Src/fire.cyclo ./Assets/Src/fire.d ./Assets/Src/fire.o ./Assets/Src/fire.su ./Assets/Src/fire30.cyclo ./Assets/Src/fire30.d ./Assets/Src/fire30.o ./Assets/Src/fire30.su ./Assets/Src/light.cyclo ./Assets/Src/light.d ./Assets/Src/light.o ./Assets/Src/light.su ./Assets/Src/light30.cyclo ./Assets/Src/light30.d ./Assets/Src/light30.o ./Assets/Src/light30.su ./Assets/Src/shadow.cyclo ./Assets/Src/shadow.d ./Assets/Src/shadow.o ./Assets/Src/shadow.su ./Assets/Src/shadow30.cyclo ./Assets/Src/shadow30.d ./Assets/Src/shadow30.o ./Assets/Src/shadow30.su ./Assets/Src/water.cyclo ./Assets/Src/water.d ./Assets/Src/water.o ./Assets/Src/water.su ./Assets/Src/water30.cyclo ./Assets/Src/water30.d ./Assets/Src/water30.o ./Assets/Src/water30.su

.PHONY: clean-Assets-2f-Src

