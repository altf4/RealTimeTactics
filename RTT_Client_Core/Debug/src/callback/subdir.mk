################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/callback/CallbackHandler.cpp 

OBJS += \
./src/callback/CallbackHandler.o 

CPP_DEPS += \
./src/callback/CallbackHandler.d 


# Each subdirectory must supply rules for building sources it contributes
src/callback/%.o: ../src/callback/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I../../RTT_Common/src -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -std=c++0x -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


