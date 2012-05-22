//============================================================================
// Name        : Message.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class which is passed to/from client/server
//============================================================================

#ifndef MESSAGE_H_
#define MESSAGE_H_

#define MSG_HEADER_SIZE sizeof(enum MessageType) + sizeof(char)
#define USERNAME_MAX_LENGTH	 20
#define MESSAGE_TIMEOUT 3

#include <stdlib.h>
#include <vector>
#include <unistd.h>
#include <stdint.h>

namespace RTT
{

//The direction that this message's PROTOCOL (not individual message) is going.
//	Who initiated the first message in the protocol
//Value is used by MessageQueue to allow for dumb queueing of messages
//	Otherwise the queue would have to be aware of the protocol used to know the direction
enum ProtocolDirection: char
{
	DIRECTION_TO_CLIENT = 0,
	DIRECTION_TO_SERVER = 1
};

enum MessageType: char
{
	MESSAGE_AUTH = 0,
	MESSAGE_LOBBY,
	MESSAGE_MATCH_LOBBY,
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

	//Not ever sent. Just used to return errors
	bool m_serializeError;

	enum MessageType m_messageType;

	enum ProtocolDirection m_direction;

	uint32_t m_serialNumber;

	//Plain old constructor
	Message();

	virtual char *Serialize(uint *length);
	static Message *Deserialize(char *buffer, uint length, enum ProtocolDirection direction);

	static Message *ReadMessage(int connectFD,
			enum ProtocolDirection direction, int timeout = MESSAGE_TIMEOUT);
	static bool WriteMessage(Message *message, int connectFD);

protected:

	//Deserialize just the Message header, and advance the buffer input variable
	// buffer: A pointer to the array of serialized bytes representing a message
	// returns - True if deserialize happened without error, false on error
	bool DeserializeHeader(char **buffer);

	//Serializes the Message header into the given array
	// buffer: Pointer to the array where the serialized bytes will go
	// NOTE: Assumes there is space in *buffer for the header
	void SerializeHeader(char **buffer);

};

}


#endif /* MESSAGE_H_ */
