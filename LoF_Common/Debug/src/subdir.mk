################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Action.cpp \
../src/Gameboard.cpp \
../src/Player.cpp \
../src/Tile.cpp \
../src/Unit.cpp 

OBJS += \
./src/Action.o \
./src/Gameboard.o \
./src/Player.o \
./src/Tile.o \
./src/Unit.o 

CPP_DEPS += \
./src/Action.d \
./src/Gameboard.d \
./src/Player.d \
./src/Tile.d \
./src/Unit.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


