//============================================================================
// Name        : Message.cpp
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
#include "MatchLobbyMessage.h"

using namespace std;
using namespace RTT;

Message::Message()
{

}

char *Message::Serialize(uint *length)
{
	switch(m_messageType)
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
	if( length <  MSG_HEADER_SIZE )
	{
		return NULL;
	}
	//Copy the message type
	enum MessageType thisType;
	memcpy(&thisType, buffer, sizeof(thisType));

	switch(thisType)
	{
		case MESSAGE_AUTH:
		{
			AuthMessage *message = new AuthMessage(buffer, length);
			if( message->m_serializeError == true )
			{
				delete message;
				return NULL;
			}
			return message;
		}
		case MESSAGE_LOBBY:
		{
			LobbyMessage *message = new LobbyMessage(buffer, length);
			if( message->m_serializeError == true )
			{
				delete message;
				return NULL;
			}
			return message;
		}
		case MESSAGE_MATCH_LOBBY:
		{
			MatchLobbyMessage *message = new MatchLobbyMessage(buffer, length);
			if( message->m_serializeError == true )
			{
				delete message;
				return NULL;
			}
			return message;
		}
		case MESSAGE_ERROR:
		{
			ErrorMessage *message = new ErrorMessage(buffer, length);
			if( message->m_serializeError == true )
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
			perror("ERROR: Socket returned error...");

			return NULL;
		}
	}

	if(input.size() < MSG_HEADER_SIZE)
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
	if( write(connectFD, buffer, length) < 0 )
	{
		//Error
		perror("ERROR: Write failed: ");
		cerr << "ERROR: Write function didn't finish...\n";
		free(buffer);
		return false;
	}
	free(buffer);
	return true;

}

bool Message::DeserializeHeader(char **buffer)
{
	if(buffer == NULL)
	{
		return false;
	}
	if(*buffer == NULL)
	{
		return false;
	}

	memcpy(&m_messageType, *buffer, sizeof(m_messageType));
	*buffer += sizeof(m_messageType);

	return true;
}

void Message::SerializeHeader(char **buffer)
{
	memcpy(*buffer, &m_messageType, sizeof(m_messageType));
	*buffer += sizeof(m_messageType);
}
