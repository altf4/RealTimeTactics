//============================================================================
// Name        : RTT_Client_GTK.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : GTK Interface to the Client_Core
//============================================================================

#include "ClientProtocolHandler.h"
#include <iostream>
#include <gtkmm.h>
#include <arpa/inet.h>

using namespace std;
using namespace Gtk;
using namespace RTT;

Glib::RefPtr<Builder> builder;

Window *window = NULL;
Button *button_custom = NULL;
Button *button_connect = NULL;
Statusbar *statusbar = NULL;
Box *box_custom = NULL;

int SocketFD = 0;

void custom_server_click()
{
	statusbar->push("Set server settings, then hit Connect");
	box_custom->set_visible(true);
}

void connect_click()
{
	statusbar->push("Trying to connect...");

	Entry *entry_IP = NULL;
	Entry *entry_port = NULL;
	Entry *entry_username = NULL;
	Entry *entry_password = NULL;

	builder->get_widget("entry_IP", entry_IP);
	builder->get_widget("entry_port", entry_port);
	builder->get_widget("entry_username", entry_username);
	builder->get_widget("entry_password", entry_password);

	//A little bit of input validation here
	string serverIP = entry_IP->get_text();

	struct sockaddr_in stSockAddr;
	int Res = inet_pton(AF_INET, serverIP.c_str(), &stSockAddr.sin_addr);
	if (Res == 0)
	{
		statusbar->push("Invalid IP address");
		return;
	}

	char *errString;
	uint serverPort = strtoul(entry_port->get_text().c_str(), &errString, 10);
	if( *errString != '\0' || entry_port->get_text().c_str() == '\0')
	{
		//Error occurred
		statusbar->push("Invalid port number");
		return;
	}

	string username = entry_username->get_text();
	string hashedPassword = entry_password->get_text();

	SocketFD = AuthToServer(serverIP, serverPort,
			username, (unsigned char*)hashedPassword.c_str());

	if( SocketFD > 0 )
	{
		statusbar->push("Connection Successful!");
	}
	else
	{
		statusbar->push("Failed to connect to server");
	}
}

int main( int argc, char **argv)
{
	Main kit(argc, argv);

	builder = Builder::create_from_file("UI/MainWindow.glade");
	builder->get_widget("window_main", window);
	builder->get_widget("button_custom", button_custom);
	builder->get_widget("button_connect", button_connect);
	builder->get_widget("status_main", statusbar);
	builder->get_widget("box_custom", box_custom);

	button_custom->signal_clicked().connect(sigc::ptr_fun(custom_server_click));
	button_connect->signal_clicked().connect(sigc::ptr_fun(connect_click));

	Main::run(*window);
	return EXIT_SUCCESS;
}

