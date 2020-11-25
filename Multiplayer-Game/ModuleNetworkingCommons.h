#pragma once

#define PROTOCOL_ID                           (uint32)'GAME'

// Packet header ///////////////////////////////////////////////////////

struct PacketHeader
{
	uint32 protocolId = PROTOCOL_ID;
	uint8 messageId;
};


// Input ///////////////////////////////////////////////////////////////

struct InputPacketData
{
	uint32 sequenceNumber = 0;
	real32 horizontalAxis = 0.0f;
	real32 verticalAxis = 0.0f;
	uint16 buttonBits = 0;
};

uint16 packInputControllerButtons(const InputController &input);

void unpackInputControllerButtons(uint16 buttonBits, InputController &input);

InputController inputControllerFromInputPacketData(const InputPacketData &inputPacketData, const InputController &previousGamepad);
