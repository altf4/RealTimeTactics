################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Action.cpp \
../src/Gameboard.cpp \
../src/Job.cpp \
../src/Map.cpp \
../src/Path.cpp \
../src/Player.cpp \
../src/Team.cpp \
../src/Tile.cpp \
../src/Unit.cpp 

OBJS += \
./src/Action.o \
./src/Gameboard.o \
./src/Job.o \
./src/Map.o \
./src/Path.o \
./src/Player.o \
./src/Team.o \
./src/Tile.o \
./src/Unit.o 

CPP_DEPS += \
./src/Action.d \
./src/Gameboard.d \
./src/Job.d \
./src/Map.d \
./src/Path.d \
./src/Player.d \
./src/Team.d \
./src/Tile.d \
./src/Unit.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0  -std=c++0x -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


