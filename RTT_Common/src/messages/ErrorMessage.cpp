//============================================================================
// Name        : ErrorMessage.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class pertaining to sending and receiving error conditions
//============================================================================

#include "ErrorMessage.h"
#include "string.h"

using namespace std;
using namespace RTT;

ErrorMessage::ErrorMessage()
{

}

ErrorMessage::ErrorMessage(char *buffer, uint32_t length)
{
	uint32_t expectedSize = MESSAGE_MIN_SIZE + sizeof(m_errorType);
	if( length != expectedSize )
	{
		m_serializeError = true;
		return;
	}

	m_serializeError = false;

	memcpy(&m_type, buffer, MESSAGE_MIN_SIZE);
	buffer += MESSAGE_MIN_SIZE;

	memcpy(&m_errorType, buffer, sizeof(m_errorType));
	buffer += sizeof(m_errorType);

}

char *ErrorMessage::Serialize(uint32_t *length)
{
	char *buffer, *originalBuffer;
	uint32_t messageSize;

	messageSize = MESSAGE_MIN_SIZE + sizeof(m_errorType);
	buffer = (char*)malloc(messageSize);
	originalBuffer = buffer;

	//Message type
	memcpy(buffer, &m_type, MESSAGE_MIN_SIZE);
	buffer += MESSAGE_MIN_SIZE;
	//Error type
	memcpy(buffer, &m_errorType, sizeof(m_errorType));
	buffer += sizeof(m_errorType);

	*length = messageSize;

	return originalBuffer;
}
