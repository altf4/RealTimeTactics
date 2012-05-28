//============================================================================
// Name        : AuthMessage.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class which is passed to/from client/server
//============================================================================

#include "AuthMessage.h"
#include "string.h"

using namespace std;
using namespace RTT;

AuthMessage::AuthMessage(enum AuthType type, enum ProtocolDirection direction)
{
	m_messageType = MESSAGE_AUTH;
	m_authType = type;
	m_direction = direction;
}

AuthMessage::AuthMessage(char *buffer, uint32_t length)
{
	if( length < MSG_HEADER_SIZE )
	{
		return;
	}

	m_serializeError = false;

	//Deserialize the UI_Message header
	if(!DeserializeHeader(&buffer))
	{
		m_serializeError = true;
		return;
	}

	//Copy the message type
	memcpy(&m_authType, buffer, sizeof(m_authType));
	buffer += sizeof(m_authType);

	switch(m_authType)
	{
		case CLIENT_HELLO:
		{
			//Uses: 1) Message Type
			//		2) Version Number

			uint32_t expectedSize = MSG_HEADER_SIZE + sizeof(m_authType) + (sizeof(uint32_t)*3);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			memcpy(&m_softwareVersion.m_major, buffer, sizeof(m_softwareVersion.m_major));
			buffer += sizeof(m_softwareVersion.m_major);
			memcpy(&m_softwareVersion.m_minor, buffer, sizeof(m_softwareVersion.m_minor));
			buffer += sizeof(m_softwareVersion.m_minor);
			memcpy(&m_softwareVersion.m_rev, buffer, sizeof(m_softwareVersion.m_rev));
			buffer += sizeof(m_softwareVersion.m_rev);
			break;
		}
		case SERVER_HELLO:
		{
			//Uses: 1) Message type
			//		2) Version Number
			//		3) AuthMechanism

			uint32_t expectedSize = MSG_HEADER_SIZE + sizeof(m_authType) + (sizeof(uint32_t)*3)
					+ sizeof(m_authMechanism);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			memcpy(&m_softwareVersion.m_major, buffer, sizeof(m_softwareVersion.m_major));
			buffer += sizeof(m_softwareVersion.m_major);
			memcpy(&m_softwareVersion.m_minor, buffer, sizeof(m_softwareVersion.m_minor));
			buffer += sizeof(m_softwareVersion.m_minor);
			memcpy(&m_softwareVersion.m_rev, buffer, sizeof(m_softwareVersion.m_rev));
			buffer += sizeof(m_softwareVersion.m_rev);

			memcpy(&m_authMechanism, buffer, sizeof(m_authMechanism));
			buffer += sizeof(m_authMechanism);

			break;
		}
		case CLIENT_AUTH:
		{
			//Uses: 1) Message Type
			//		2) Username
			//		3) Hashed password

			uint32_t expectedSize = MSG_HEADER_SIZE + sizeof(m_authType) + sizeof(m_username)
					+ sizeof(m_hashedPassword);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			memcpy(m_username, buffer, sizeof(m_username));
			buffer += sizeof(m_username);
			memcpy(m_hashedPassword, buffer, sizeof(m_hashedPassword));
			buffer += sizeof(m_hashedPassword);

			break;
		}
		case SERVER_AUTH_REPLY:
		{
			//Uses: 1) authSuccess
			//		2) Your new Player ID
			uint32_t expectedSize = MSG_HEADER_SIZE + sizeof(m_authType) + sizeof(m_authSuccess) + PLAYER_DESCR_SIZE;
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			memcpy(&m_authSuccess, buffer, sizeof(m_authSuccess));
			buffer += sizeof(m_authSuccess);

			//Player ID that joined
			memcpy(&m_playerDescription.m_name, buffer, PLAYER_NAME_SIZE);
			buffer += PLAYER_NAME_SIZE;
			memcpy(&m_playerDescription.m_ID, buffer, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(&m_playerDescription.m_color, buffer, sizeof(enum TeamColor));
			buffer += sizeof(enum TeamColor);
			memcpy(&m_playerDescription.m_team, buffer, sizeof(enum TeamNumber));
			buffer += sizeof(enum TeamNumber);

			break;
		}
		default:
		{
			//error
			m_serializeError = true;
			break;
		}
	}
	m_serializeError = false;
}

char *AuthMessage::Serialize(uint32_t *length)
{
	char *buffer, *originalBuffer;
	switch(m_authType)
	{
		case CLIENT_HELLO:
		{

			//Uses: 1) Message Type
			//		2) Version Number

			//Allocate the memory and assign it to *buffer
			uint32_t messageSize = MSG_HEADER_SIZE + sizeof(m_authType)	+ (sizeof(uint32_t)*3);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer);

			//Put the type in
			memcpy(buffer, &m_authType, sizeof(m_authType));
			buffer += sizeof(m_authType);

			//Version Number
			memcpy(buffer, &m_softwareVersion.m_major, sizeof(m_softwareVersion.m_major));
			buffer += sizeof(m_softwareVersion.m_major);
			memcpy(buffer, &m_softwareVersion.m_minor, sizeof(m_softwareVersion.m_minor));
			buffer += sizeof(m_softwareVersion.m_minor);
			memcpy(buffer, &m_softwareVersion.m_rev, sizeof(m_softwareVersion.m_rev));
			buffer += sizeof(m_softwareVersion.m_rev);

			*length = messageSize;
			return originalBuffer;
		}
		case SERVER_HELLO:
		{
			//Uses: 1) Message Type
			//		2) Version Number
			//		3) AuthMechanism

			uint32_t messageSize = MSG_HEADER_SIZE + sizeof(m_authType) + sizeof(m_authMechanism)
							+ (sizeof(uint32_t)*3);
			//Allocate the memory and assign it to *buffer
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer);

			//Put the type in
			memcpy(buffer, &m_authType, sizeof(m_authType));
			buffer += sizeof(m_authType);

			memcpy(buffer, &m_softwareVersion.m_major, sizeof(m_softwareVersion.m_major));
			buffer += sizeof(m_softwareVersion.m_major);
			memcpy(buffer, &m_softwareVersion.m_minor, sizeof(m_softwareVersion.m_minor));
			buffer += sizeof(m_softwareVersion.m_minor);
			memcpy(buffer, &m_softwareVersion.m_rev, sizeof(m_softwareVersion.m_rev));
			buffer += sizeof(m_softwareVersion.m_rev);

			memcpy(buffer, &m_authMechanism, sizeof(m_authMechanism));
			buffer += sizeof(m_authMechanism);

			*length = messageSize;

			return originalBuffer;
		}
		case CLIENT_AUTH:
		{
			//Uses: 1) Message Type
			//		2) Username
			//		3) Hashed password

			uint32_t messageSize = MSG_HEADER_SIZE + sizeof(m_authType) + sizeof(m_username)
					+ sizeof(m_hashedPassword);
			//Allocate the memory and assign it to *buffer
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer);

			//Put the type in
			memcpy(buffer, &m_authType, sizeof(m_authType));
			buffer += sizeof(m_authType);
			//Username
			memcpy(buffer, m_username, sizeof(m_username));
			buffer += sizeof(m_username);
			//Hashed password
			memcpy(buffer, m_hashedPassword, sizeof(m_hashedPassword));

			*length = messageSize;
			return originalBuffer;
		}
		case SERVER_AUTH_REPLY:
		{
			uint32_t messageSize = MSG_HEADER_SIZE + sizeof(m_authType) + sizeof(m_authSuccess) + PLAYER_DESCR_SIZE;
			//Allocate the memory and assign it to *buffer
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer);

			//Put the type in
			memcpy(buffer, &m_authType, sizeof(m_authType));
			buffer += sizeof(m_authType);

			//Auth success
			memcpy(buffer, &m_authSuccess, sizeof(m_authSuccess));
			buffer += sizeof(m_authSuccess);

			//Put the player description in
			memcpy(buffer, &m_playerDescription.m_name, PLAYER_NAME_SIZE);
			buffer += PLAYER_NAME_SIZE;
			memcpy(buffer, &m_playerDescription.m_ID, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(buffer, &m_playerDescription.m_color, sizeof(enum TeamColor));
			buffer += sizeof(enum TeamColor);
			memcpy(buffer, &m_playerDescription.m_team, sizeof(enum TeamNumber));
			buffer += sizeof(enum TeamNumber);

			*length = messageSize;
			return originalBuffer;
		}
		default:
		{
			//Error
			return NULL;
		}
	}
}
