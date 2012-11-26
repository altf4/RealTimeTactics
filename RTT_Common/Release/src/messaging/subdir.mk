################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/messaging/MessageEndpoint.cpp \
../src/messaging/MessageManager.cpp \
../src/messaging/MessageQueue.cpp \
../src/messaging/MessageQueueBimap.cpp \
../src/messaging/ServerCallback.cpp \
../src/messaging/Ticket.cpp 

OBJS += \
./src/messaging/MessageEndpoint.o \
./src/messaging/MessageManager.o \
./src/messaging/MessageQueue.o \
./src/messaging/MessageQueueBimap.o \
./src/messaging/ServerCallback.o \
./src/messaging/Ticket.o 

CPP_DEPS += \
./src/messaging/MessageEndpoint.d \
./src/messaging/MessageManager.d \
./src/messaging/MessageQueue.d \
./src/messaging/MessageQueueBimap.d \
./src/messaging/ServerCallback.d \
./src/messaging/Ticket.d 


# Each subdirectory must supply rules for building sources it contributes
src/messaging/%.o: ../src/messaging/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0  -std=c++0x -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


