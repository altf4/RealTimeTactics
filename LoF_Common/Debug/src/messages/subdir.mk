################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/messages/AuthMessage.cpp \
../src/messages/LobbyMessage.cpp \
../src/messages/Message.cpp 

OBJS += \
./src/messages/AuthMessage.o \
./src/messages/LobbyMessage.o \
./src/messages/Message.o 

CPP_DEPS += \
./src/messages/AuthMessage.d \
./src/messages/LobbyMessage.d \
./src/messages/Message.d 


# Each subdirectory must supply rules for building sources it contributes
src/messages/%.o: ../src/messages/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0  -std=c++0x -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


