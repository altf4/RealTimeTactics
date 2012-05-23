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
#include "../MessageManager.h"

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

Message *Message::Deserialize(char *buffer, uint length, enum ProtocolDirection direction )
{
	if(length < MSG_HEADER_SIZE)
	{
		return new ErrorMessage(ERROR_MALFORMED_MESSAGE, direction);
	}

	enum MessageType thisType;
	memcpy(&thisType, buffer, sizeof(thisType));

	Message *message;
	switch(thisType)
	{
		case MESSAGE_AUTH:
		{
			message = new AuthMessage(buffer, length);
			break;
		}
		case MESSAGE_LOBBY:
		{
			message = new LobbyMessage(buffer, length);
			break;
		}
		case MESSAGE_MATCH_LOBBY:
		{
			message = new MatchLobbyMessage(buffer, length);
			break;
		}
		case MESSAGE_ERROR:
		{
			message = new ErrorMessage(buffer, length);
			break;
		}
		default:
		{
			return new ErrorMessage(ERROR_UNKNOWN_MESSAGE_TYPE, direction);
		}
	}

	if(message->m_serializeError)
	{
		delete message;
		return new ErrorMessage(ERROR_MALFORMED_MESSAGE, direction);
	}
	return message;
}


Message *Message::ReadMessage(int connectFD, enum ProtocolDirection direction, int timeout)
{
	return MessageManager::Instance().PopMessage(connectFD, direction, timeout);

}

bool Message::WriteMessage(Message *message, int connectFD)
{
	if (connectFD == -1)
		{
			return false;
		}

		message->m_serialNumber = MessageManager::Instance().GetSerialNumber(
				connectFD, message->m_direction);

		uint32_t length;
		char *buffer = message->Serialize(&length);

		// Total bytes of a write() call that need to be sent
		uint32_t bytesSoFar;

		// Return value of the write() call, actual bytes sent
		uint32_t bytesWritten;

		// Send the message length
		bytesSoFar = 0;
	    while (bytesSoFar < sizeof(length))
		{
			bytesWritten = write(connectFD, &length, sizeof(length) - bytesSoFar);
			if (bytesWritten < 0)
			{
				free(buffer);
				return false;
			}
			else
			{
				bytesSoFar += bytesWritten;
			}
		}


		// Send the message
		bytesSoFar = 0;
		while (bytesSoFar < length)
		{
			bytesWritten = write(connectFD, buffer, length - bytesSoFar);
			if (bytesWritten < 0)
			{
				free(buffer);
				return false;
			}
			else
			{
				bytesSoFar += bytesWritten;
			}
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

	memcpy(&m_direction, *buffer, sizeof(m_direction));
	*buffer += sizeof(m_direction);

	memcpy(&m_serialNumber, *buffer, sizeof(m_serialNumber));
	*buffer += sizeof(m_serialNumber);

	if((m_direction != DIRECTION_TO_CLIENT) && (m_direction != DIRECTION_TO_SERVER))
	{
		return false;
	}

	return true;
}

void Message::SerializeHeader(char **buffer)
{
	memcpy(*buffer, &m_messageType, sizeof(m_messageType));
	*buffer += sizeof(m_messageType);

	memcpy(*buffer, &m_direction, sizeof(m_direction));
	*buffer += sizeof(m_direction);

	memcpy(*buffer, &m_serialNumber, sizeof(m_serialNumber));
	*buffer += sizeof(m_serialNumber);
}
