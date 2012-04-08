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
<<<<<<< HEAD
<<<<<<< HEAD
	g++ -I/usr/include/cairo -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng12 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/atk-1.0 -I/usr/include/pango-1.0 -I/usr/include/gio-unix-2.0/ -I/usr/include/gtkmm-3.0 -I/usr/lib/gtkmm-3.0/include -I/usr/include/atkmm-1.6 -I/usr/include/giomm-2.4 -I/usr/lib/giomm-2.4/include -I/usr/include/pangomm-1.4 -I/usr/lib/pangomm-1.4/include -I/usr/include/gtk-3.0 -I/usr/include/cairomm-1.0 -I/usr/lib/cairomm-1.0/include -I/usr/include/gtk-3.0/unix-print -I/usr/include/gdkmm-3.0 -I/usr/lib/gdkmm-3.0/include -I/usr/include/glibmm-2.4 -I/usr/lib/glibmm-2.4/include -I/usr/include/sigc++-2.0 -I/usr/lib/sigc++-2.0/include -I../../RTT_Client_Core/src -I../../RTT_Common/src -O0 -g3 -Wall -c -fmessage-length=0 -pthread -DGSEAL_ENABLE -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
=======
	g++ -I../../RTT_Client_Core/src -I../../RTT_Common/src -O0 -g3 -Wall -c -fmessage-length=0 -pthread -DGSEAL_ENABLE -fPIC `pkg-config --cflags glib-2.0 gtkmm-3.0` -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
>>>>>>> upstream/master
=======
	g++ -I../../RTT_Client_Core/src -I../../RTT_Common/src -O0 -g3 -Wall -c -fmessage-length=0 -pthread -DGSEAL_ENABLE -fPIC `pkg-config --cflags glib-2.0 gtkmm-3.0` -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
>>>>>>> upstream/master
	@echo 'Finished building: $<'
	@echo ' '


