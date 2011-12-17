//============================================================================
// Name        : Unit.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class which is passed to/from client/server
//============================================================================

#include "Message.h"
#include "string.h"
#include "iostream"
#include <sys/socket.h>
#include "AuthMessage.h"
#include "LobbyMessage.h"
#include "ErrorMessage.h"

using namespace std;
using namespace LoF;

Message::Message()
{

}

char *Message::Serialize(uint *length)
{
	switch(type)
	{
		case CLIENT_HELLO:
		case SERVER_HELLO:
		case CLIENT_AUTH:
		case SERVER_AUTH_REPLY:
		{
			char *buffer = ((AuthMessage*)this)->Serialize(length);
			return buffer;
		}
		default:
		{
			//error
			return NULL;
		}
	}
}

Message *Message::Deserialize(char *buffer, uint length)
{
	if( length <  MESSAGE_MIN_SIZE )
	{
		return NULL;
	}
	//Copy the message type
	enum MessageType thisType;
	memcpy(&thisType, buffer, MESSAGE_MIN_SIZE);

	switch(thisType)
	{
		case CLIENT_HELLO:
		case SERVER_HELLO:
		case CLIENT_AUTH:
		case SERVER_AUTH_REPLY:
		{
			AuthMessage *message = new AuthMessage(buffer, length);
			if( message->serializeError == true )
			{
				delete message;
				return NULL;
			}
			return message;
		}
		case MATCH_LIST_REQUEST:
		case MATCH_LIST_REPLY:
		case MATCH_CREATE_REQUEST:
		case MATCH_CREATE_OPTIONS_AVAILABLE:
		case MATCH_CREATE_OPTIONS_CHOSEN:
		case MATCH_CREATE_REPLY:
		case MATCH_JOIN_REQUEST:
		case MATCH_JOIN_REPLY:
		case MATCH_LEAVE_NOTIFICATION:
		case MATCH_LEAVE_ACKNOWLEDGE:
		case MATCH_EXIT_SERVER_NOTIFICATION:
		case MATCH_EXIT_SERVER_ACKNOWLEDGE:
		{
			LobbyMessage *message = new LobbyMessage(buffer, length);
			if( message->serializeError == true )
			{
				delete message;
				return NULL;
			}
			return message;
		}
		case MESSAGE_ERROR:
		{
			ErrorMessage *message = new ErrorMessage(buffer, length);
			if( message->serializeError == true )
			{
				delete message;
				return NULL;
			}
			return message;
		}
		default:
		{
			//error
			cerr << "ERROR: Unrecognized message type\n.";
			return NULL;
		}
	}
}


Message *Message::ReadMessage(int connectFD)
{
	//perform read operations ...
	char buff[4096];
	int bytesRead = 4096;
	vector <char> input;
	while( bytesRead == 4096)
	{
		bytesRead = read(connectFD, buff, 4096);
		if( bytesRead >= 0 )
		{
			input.insert(input.end(), buff, buff + bytesRead);
		}
		else
		{
			//Error in reading from socket
			cerr << "ERROR: Socket returned error...\n";
			return NULL;
		}
	}

	if(input.size() < MESSAGE_MIN_SIZE)
	{
		//Error, message too small
		cerr << "ERROR: Message received is too small, ignoring...\n";
		return NULL;
	}

	return Message::Deserialize(input.data(), input.size());

}

bool Message::WriteMessage(Message *message, int connectFD)
{
	uint length;
	char *buffer = message->Serialize(&length);

	//TODO: Loop the write until it finishes?
	if( write(connectFD, buffer, length) == -1)
	{
		//Error
		cerr << "ERROR: Write function didn't finish...\n";
		free(buffer);
		return false;
	}
	free(buffer);
	return true;

}
