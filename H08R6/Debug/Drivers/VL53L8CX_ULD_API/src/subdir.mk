################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/VL53L8CX_ULD_API/src/Porting.c \
../Drivers/VL53L8CX_ULD_API/src/VL53L8CX_APIs.c \
../Drivers/VL53L8CX_ULD_API/src/platform.c \
../Drivers/VL53L8CX_ULD_API/src/vl53l8cx_api.c \
../Drivers/VL53L8CX_ULD_API/src/vl53l8cx_plugin_detection_thresholds.c \
../Drivers/VL53L8CX_ULD_API/src/vl53l8cx_plugin_motion_indicator.c \
../Drivers/VL53L8CX_ULD_API/src/vl53l8cx_plugin_xtalk.c 

OBJS += \
./Drivers/VL53L8CX_ULD_API/src/Porting.o \
./Drivers/VL53L8CX_ULD_API/src/VL53L8CX_APIs.o \
./Drivers/VL53L8CX_ULD_API/src/platform.o \
./Drivers/VL53L8CX_ULD_API/src/vl53l8cx_api.o \
./Drivers/VL53L8CX_ULD_API/src/vl53l8cx_plugin_detection_thresholds.o \
./Drivers/VL53L8CX_ULD_API/src/vl53l8cx_plugin_motion_indicator.o \
./Drivers/VL53L8CX_ULD_API/src/vl53l8cx_plugin_xtalk.o 

C_DEPS += \
./Drivers/VL53L8CX_ULD_API/src/Porting.d \
./Drivers/VL53L8CX_ULD_API/src/VL53L8CX_APIs.d \
./Drivers/VL53L8CX_ULD_API/src/platform.d \
./Drivers/VL53L8CX_ULD_API/src/vl53l8cx_api.d \
./Drivers/VL53L8CX_ULD_API/src/vl53l8cx_plugin_detection_thresholds.d \
./Drivers/VL53L8CX_ULD_API/src/vl53l8cx_plugin_motion_indicator.d \
./Drivers/VL53L8CX_ULD_API/src/vl53l8cx_plugin_xtalk.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/VL53L8CX_ULD_API/src/%.o Drivers/VL53L8CX_ULD_API/src/%.su Drivers/VL53L8CX_ULD_API/src/%.cyclo: ../Drivers/VL53L8CX_ULD_API/src/%.c Drivers/VL53L8CX_ULD_API/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G0B1xx -c -I../Core/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G0xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/Control/Documents/H08R6x-Firmware/H08R6/Drivers/VL53L8CX_ULD_API/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-VL53L8CX_ULD_API-2f-src

clean-Drivers-2f-VL53L8CX_ULD_API-2f-src:
	-$(RM) ./Drivers/VL53L8CX_ULD_API/src/Porting.cyclo ./Drivers/VL53L8CX_ULD_API/src/Porting.d ./Drivers/VL53L8CX_ULD_API/src/Porting.o ./Drivers/VL53L8CX_ULD_API/src/Porting.su ./Drivers/VL53L8CX_ULD_API/src/VL53L8CX_APIs.cyclo ./Drivers/VL53L8CX_ULD_API/src/VL53L8CX_APIs.d ./Drivers/VL53L8CX_ULD_API/src/VL53L8CX_APIs.o ./Drivers/VL53L8CX_ULD_API/src/VL53L8CX_APIs.su ./Drivers/VL53L8CX_ULD_API/src/platform.cyclo ./Drivers/VL53L8CX_ULD_API/src/platform.d ./Drivers/VL53L8CX_ULD_API/src/platform.o ./Drivers/VL53L8CX_ULD_API/src/platform.su ./Drivers/VL53L8CX_ULD_API/src/vl53l8cx_api.cyclo ./Drivers/VL53L8CX_ULD_API/src/vl53l8cx_api.d ./Drivers/VL53L8CX_ULD_API/src/vl53l8cx_api.o ./Drivers/VL53L8CX_ULD_API/src/vl53l8cx_api.su ./Drivers/VL53L8CX_ULD_API/src/vl53l8cx_plugin_detection_thresholds.cyclo ./Drivers/VL53L8CX_ULD_API/src/vl53l8cx_plugin_detection_thresholds.d ./Drivers/VL53L8CX_ULD_API/src/vl53l8cx_plugin_detection_thresholds.o ./Drivers/VL53L8CX_ULD_API/src/vl53l8cx_plugin_detection_thresholds.su ./Drivers/VL53L8CX_ULD_API/src/vl53l8cx_plugin_motion_indicator.cyclo ./Drivers/VL53L8CX_ULD_API/src/vl53l8cx_plugin_motion_indicator.d ./Drivers/VL53L8CX_ULD_API/src/vl53l8cx_plugin_motion_indicator.o ./Drivers/VL53L8CX_ULD_API/src/vl53l8cx_plugin_motion_indicator.su ./Drivers/VL53L8CX_ULD_API/src/vl53l8cx_plugin_xtalk.cyclo ./Drivers/VL53L8CX_ULD_API/src/vl53l8cx_plugin_xtalk.d ./Drivers/VL53L8CX_ULD_API/src/vl53l8cx_plugin_xtalk.o ./Drivers/VL53L8CX_ULD_API/src/vl53l8cx_plugin_xtalk.su

.PHONY: clean-Drivers-2f-VL53L8CX_ULD_API-2f-src

