################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/CallbackHandler.cpp \
../src/ClientGameState.cpp \
../src/ClientProtocolHandler.cpp \
../src/GameCommands.cpp \
../src/GameEvents.cpp \
../src/MainLobbyEvents.cpp \
../src/MatchLobbyEvents.cpp 

OBJS += \
./src/CallbackHandler.o \
./src/ClientGameState.o \
./src/ClientProtocolHandler.o \
./src/GameCommands.o \
./src/GameEvents.o \
./src/MainLobbyEvents.o \
./src/MatchLobbyEvents.o 

CPP_DEPS += \
./src/CallbackHandler.d \
./src/ClientGameState.d \
./src/ClientProtocolHandler.d \
./src/GameCommands.d \
./src/GameEvents.d \
./src/MainLobbyEvents.d \
./src/MatchLobbyEvents.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I../../RTT_Common/src -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -std=c++0x -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


