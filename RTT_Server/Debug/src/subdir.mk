################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/MatchLoop.cpp \
../src/RTTServerCallback.cpp \
../src/RTT_Server.cpp \
../src/ServerProtocolHandler.cpp 

OBJS += \
./src/MatchLoop.o \
./src/RTTServerCallback.o \
./src/RTT_Server.o \
./src/ServerProtocolHandler.o 

CPP_DEPS += \
./src/MatchLoop.d \
./src/RTTServerCallback.d \
./src/RTT_Server.d \
./src/ServerProtocolHandler.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I../../RTT_Common/src -O0 -g3 -Wall -c -fmessage-length=0 -std=c++0x -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


