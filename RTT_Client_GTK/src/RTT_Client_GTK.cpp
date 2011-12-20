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

Window *window = NULL;
Button *button = NULL;
Statusbar *statusbar = NULL;
Box *box_custom = NULL;

void custom_server_click()
{
	statusbar->push("Set server settings, then hit Connect");
	box_custom->set_visible(true);

}

extern "C" int main( int argc, char **argv)
{
	Main kit(argc, argv);

	Glib::RefPtr<Builder> builder = Builder::create_from_file("UI/MainWindow.glade");
	builder->get_widget("window_main", window);
	builder->get_widget("button_custom", button);
	builder->get_widget("status_main", statusbar);
	builder->get_widget("box_custom", box_custom);

	button->signal_clicked().connect(sigc::ptr_fun(custom_server_click));

	Main::run(*window);

	return EXIT_SUCCESS;
}

