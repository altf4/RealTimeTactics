################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/MatchLoop.cpp \
../src/RTT_Server.cpp \
../src/ServerProtocolHandler.cpp \
../src/WaitingPool.cpp 

OBJS += \
./src/MatchLoop.o \
./src/RTT_Server.o \
./src/ServerProtocolHandler.o \
./src/WaitingPool.o 

CPP_DEPS += \
./src/MatchLoop.d \
./src/RTT_Server.d \
./src/ServerProtocolHandler.d \
./src/WaitingPool.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I../../RTT_Common/src -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


