################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/RTT_Ogre_Base.cpp \
../src/RTT_Ogre_Game.cpp \
../src/RTT_Ogre_Map.cpp \
../src/RTT_Ogre_Unit.cpp 

OBJS += \
./src/RTT_Ogre_Base.o \
./src/RTT_Ogre_Game.o \
./src/RTT_Ogre_Map.o \
./src/RTT_Ogre_Unit.o 

CPP_DEPS += \
./src/RTT_Ogre_Base.d \
./src/RTT_Ogre_Game.d \
./src/RTT_Ogre_Map.d \
./src/RTT_Ogre_Unit.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/include/OGRE -I../../RTT_Common/src -I../../RTT_Client_Core/src -I"/usr/lib/OGRE/include" -I/usr/include/OGRE/Terrain -I/usr/lib/OGRE -I/usr/include/OIS -I/usr/share/OGRE/Samples/Common/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


