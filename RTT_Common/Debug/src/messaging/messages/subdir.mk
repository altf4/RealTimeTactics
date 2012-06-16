################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/messaging/messages/AuthMessage.cpp \
../src/messaging/messages/ErrorMessage.cpp \
../src/messaging/messages/GameMessage.cpp \
../src/messaging/messages/LobbyMessage.cpp \
../src/messaging/messages/MatchLobbyMessage.cpp \
../src/messaging/messages/Message.cpp 

OBJS += \
./src/messaging/messages/AuthMessage.o \
./src/messaging/messages/ErrorMessage.o \
./src/messaging/messages/GameMessage.o \
./src/messaging/messages/LobbyMessage.o \
./src/messaging/messages/MatchLobbyMessage.o \
./src/messaging/messages/Message.o 

CPP_DEPS += \
./src/messaging/messages/AuthMessage.d \
./src/messaging/messages/ErrorMessage.d \
./src/messaging/messages/GameMessage.d \
./src/messaging/messages/LobbyMessage.d \
./src/messaging/messages/MatchLobbyMessage.d \
./src/messaging/messages/Message.d 


# Each subdirectory must supply rules for building sources it contributes
src/messaging/messages/%.o: ../src/messaging/messages/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0  -std=c++0x -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


