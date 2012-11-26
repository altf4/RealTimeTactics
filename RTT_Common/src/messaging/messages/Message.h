//============================================================================
// Name        : Message.h
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

#ifndef Message_H_
#define Message_H_

#include <stdlib.h>
#include "stdint.h"

#define MESSAGE_MIN_SIZE 1
#define USERNAME_MAX_LENGTH	 20
#define REPLY_TIMEOUT 3

#define MESSAGE_HDR_SIZE sizeof(m_messageType) + sizeof(m_ourSerialNumber) + sizeof(m_theirSerialNumber)

namespace RTT
{

enum MessageType: char
{
	MESSAGE_AUTH = 0,
	MESSAGE_LOBBY,
	MESSAGE_MATCH_LOBBY,
	MESSAGE_GAME,
	MESSAGE_ERROR
};

struct VersionNumber
{
	uint32_t m_major;
	uint32_t m_minor;
	uint32_t m_rev;
};

class Message
{
public:

	//Empty constructor
	Message();
	virtual ~Message();

	// This is called to delete any contents that the message is wrapping.
	// It is assumed the destructor will leave the contents alone, so the Message
	// can be safely deleted while what it wraps can continue to be used.
	virtual void DeleteContents();

	//Creates a new Message from a given buffer. Calls the appropriate child constructor
	//	buffer - char pointer to a buffer in memory where the serialized message is at
	//	length - length of the buffer
	//	direction - protocol direction that we expect the message to be going. Used in error conditions when there is no valid message
	// Returns - Pointer to newly allocated Message object
	//				returns ErrorMessage on error
	//	NOTE: The caller must manually delete the returned object when finished with it
	static Message *Deserialize(char *buffer, uint32_t length);

	//Serializes the Message object into a char array
	//	*length - Return parameter, specifies the length of the serialized array returned
	// Returns - A pointer to the serialized array
	//	NOTE: The caller must manually free() the returned buffer after use
	virtual char *Serialize(uint32_t *length) = 0;

	enum MessageType m_messageType;

	uint32_t m_ourSerialNumber;
	uint32_t m_theirSerialNumber;

protected:

	//Deserialize just the Message header, and advance the buffer input variable
	//	buffer: A pointer to the array of serialized bytes representing a message
	//	returns - True if deserialize happened without error, false on error
	bool DeserializeHeader(char **buffer);

	//Serializes the Message header into the given array
	//	buffer: Pointer to the array where the serialized bytes will go
	//	NOTE: Assumes there is space in *buffer for the header
	void SerializeHeader(char **buffer, uint32_t messageSize);

	//Used to indicate serialization error in constructors
	//	(Since constructors can't return NULL)
	//Not ever sent.
	bool m_serializeError;

};

}

#endif /* Message_H_ */
