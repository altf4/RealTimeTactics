################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/AdvancedOgreFramework.cpp \
../src/AppStateManager.cpp \
../src/CallbackHandler.cpp \
../src/DemoApp.cpp \
../src/GameState.cpp \
../src/JoinCustomServerState.cpp \
../src/LobbyState.cpp \
../src/MatchLobbyState.cpp \
../src/MenuState.cpp \
../src/RTT_Ogre_Map.cpp \
../src/RTT_Ogre_Player.cpp \
../src/RTT_Ogre_Unit.cpp \
../src/main.cpp 

OBJS += \
./src/AdvancedOgreFramework.o \
./src/AppStateManager.o \
./src/CallbackHandler.o \
./src/DemoApp.o \
./src/GameState.o \
./src/JoinCustomServerState.o \
./src/LobbyState.o \
./src/MatchLobbyState.o \
./src/MenuState.o \
./src/RTT_Ogre_Map.o \
./src/RTT_Ogre_Player.o \
./src/RTT_Ogre_Unit.o \
./src/main.o 

CPP_DEPS += \
./src/AdvancedOgreFramework.d \
./src/AppStateManager.d \
./src/CallbackHandler.d \
./src/DemoApp.d \
./src/GameState.d \
./src/JoinCustomServerState.d \
./src/LobbyState.d \
./src/MatchLobbyState.d \
./src/MenuState.d \
./src/RTT_Ogre_Map.d \
./src/RTT_Ogre_Player.d \
./src/RTT_Ogre_Unit.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/local/include/CEGUI -I/usr/local/include/OGRE -I/usr/include/OIS -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include -I/usr/include/gtkmm-3.0 -I/usr/lib64/gtkmm-3.0/include -I/usr/include/atkmm-1.6 -I/usr/include/giomm-2.4 -I/usr/lib64/giomm-2.4/include -I/usr/include/pangomm-1.4 -I/usr/lib64/pangomm-1.4/include -I/usr/include/gtk-3.0 -I/usr/include/cairomm-1.0 -I/usr/lib64/cairomm-1.0/include -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/gtk-3.0/unix-print -I/usr/include/gdkmm-3.0 -I/usr/lib64/gdkmm-3.0/include -I/usr/include/atk-1.0 -I/usr/include/glibmm-2.4 -I/usr/lib64/glibmm-2.4/include -I/usr/include/sigc++-2.0 -I/usr/lib64/sigc++-2.0/include -I/usr/include/pango-1.0 -I/usr/include/cairo -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng15 -I/usr/include/gio-unix-2.0/ -I/usr/local/include/CEGUI/RendererModules/Ogre -I/usr/local/lib -I../../RTT_Client_Core/src -I../../RTT_Common/src -I/usr/local/include/OGRE/Terrain -I/usr/local/lib/OGRE -O0 -g3 -Wall -c -fmessage-length=0 -std=c++0x -pthread -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


