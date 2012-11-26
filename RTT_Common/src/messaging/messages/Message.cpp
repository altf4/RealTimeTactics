//============================================================================
// Name        : Message.cpp
// Copyright   : DataSoft Corporation 2011-2012
//	Nova is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   Nova is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with Nova.  If not, see <http://www.gnu.org/licenses/>.
// Description : Parent message class for all message subtypes. Suitable for any
//		communications over a stream socket
//============================================================================

#include <string>
#include <vector>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Message.h"
#include "AuthMessage.h"
#include "LobbyMessage.h"
#include "ErrorMessage.h"
#include "MatchLobbyMessage.h"
#include "GameMessage.h"
#include "../MessageManager.h"

using namespace std;
using namespace RTT;

Message::Message()
{

}

Message::~Message()
{

}

void Message::DeleteContents()
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
	if(length < MESSAGE_MIN_SIZE)
	{
		return new ErrorMessage(ERROR_MALFORMED_MESSAGE);
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
		case MESSAGE_GAME:
		{
			message = new GameMessage(buffer, length);
			break;
		}
		case MESSAGE_ERROR:
		{
			message = new ErrorMessage(buffer, length);
			break;
		}
		default:
		{
			return new ErrorMessage(ERROR_UNKNOWN_MESSAGE_TYPE);
		}
	}

	if(message->m_serializeError)
	{
		delete message;
		return new ErrorMessage(ERROR_MALFORMED_MESSAGE);
	}
	return message;
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

	memcpy(&m_theirSerialNumber, *buffer, sizeof(m_theirSerialNumber));
	*buffer += sizeof(m_theirSerialNumber);

	memcpy(&m_ourSerialNumber, *buffer, sizeof(m_ourSerialNumber));
	*buffer += sizeof(m_ourSerialNumber);

	return true;
}

void Message::SerializeHeader(char **buffer, uint32_t messageSize)
{
	// Put the size of the message first so we know how much to read when reading
	memcpy(*buffer, &messageSize, sizeof(messageSize));
	*buffer += sizeof(messageSize);

	memcpy(*buffer, &m_messageType, sizeof(m_messageType));
	*buffer += sizeof(m_messageType);

	memcpy(*buffer, &m_theirSerialNumber, sizeof(m_theirSerialNumber));
	*buffer += sizeof(m_theirSerialNumber);

	memcpy(*buffer, &m_ourSerialNumber, sizeof(m_ourSerialNumber));
	*buffer += sizeof(m_ourSerialNumber);
}
