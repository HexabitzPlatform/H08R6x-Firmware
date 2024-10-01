################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Core/Startup/startup_stm32g0b1ceux.s 

OBJS += \
./Core/Startup/startup_stm32g0b1ceux.o 

S_DEPS += \
./Core/Startup/startup_stm32g0b1ceux.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Startup/%.o: ../Core/Startup/%.s Core/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m0plus -g3 -DDEBUG -c -I"C:/Users/Control/Documents/H08R6x-Firmware/H08R6/Drivers/VL53L8CX_ULD_API/inc" -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@" "$<"

clean: clean-Core-2f-Startup

clean-Core-2f-Startup:
	-$(RM) ./Core/Startup/startup_stm32g0b1ceux.d ./Core/Startup/startup_stm32g0b1ceux.o

.PHONY: clean-Core-2f-Startup

