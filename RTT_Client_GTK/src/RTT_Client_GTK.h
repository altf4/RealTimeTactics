//============================================================================
// Name        : RTT_Client_GTK.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : GTK Interface to the Client_Core
//============================================================================

#ifndef RTT_CLIENT_GTK_H_
#define RTT_CLIENT_GTK_H_


void connect_click();
void quit_server_click();
void create_match_click();
void create_match_submit_click();
void join_match_click();

void leave_match_click();

void LaunchMainLobbyPane();
void LaunchServerConnectPane();
void LaunchMatchLobbyPane();

void InitGlobalWidgets();

#endif /* RTT_CLIENT_GTK_H_ */
