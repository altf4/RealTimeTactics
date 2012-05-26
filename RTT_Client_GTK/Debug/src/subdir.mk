################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/CallbackHandler.cpp \
../src/RTT_Client_GTK.cpp \
../src/WelcomeWindow.cpp 

OBJS += \
./src/CallbackHandler.o \
./src/RTT_Client_GTK.o \
./src/WelcomeWindow.o 

CPP_DEPS += \
./src/CallbackHandler.d \
./src/RTT_Client_GTK.d \
./src/WelcomeWindow.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I../../RTT_Client_Core/src -I../../RTT_Common/src -O0 -g3 -Wall -c -fmessage-length=0 -pthread -DGSEAL_ENABLE -fPIC `pkg-config --cflags glib-2.0 gtkmm-3.0` -std=c++0x -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


