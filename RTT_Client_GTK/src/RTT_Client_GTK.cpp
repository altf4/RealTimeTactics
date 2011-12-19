//============================================================================
// Name        : RTT_Client_GTK.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : GTK Interface to the Client_Core
//============================================================================

#include <iostream>
#include <gtkmm.h>

using namespace std;
using namespace Gtk;

void custom_server_click()
{
	cout << "SUCCESS!!\n";
}

extern "C" int main( int argc, char **argv)
{
	Main kit(argc, argv);

	Window *window = NULL;
	Button *button = NULL;

	Glib::RefPtr<Builder> builder = Builder::create_from_file("UI/MainWindow.glade");
	builder->get_widget("window_main", window);
	builder->get_widget("button_connect", button);

	button->signal_clicked().connect(sigc::ptr_fun(custom_server_click));

	Main::run(*window);

	return EXIT_SUCCESS;
}

