#pragma once

enum class ClientMessage : uint8
{
	Hello,
	Input,
	Check,
	Ping   // NOTE(jesus): Use this message type in the virtual connection lab session
};

enum class ServerMessage : uint8
{
	Welcome,
	Unwelcome,
	Ping,
	Replication
};
