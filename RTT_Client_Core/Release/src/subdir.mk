################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ClientProtocolHandler.cpp \
../src/GameCommands.cpp \
../src/GameEvents.cpp 

OBJS += \
./src/ClientProtocolHandler.o \
./src/GameCommands.o \
./src/GameEvents.o 

CPP_DEPS += \
./src/ClientProtocolHandler.d \
./src/GameCommands.d \
./src/GameEvents.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


