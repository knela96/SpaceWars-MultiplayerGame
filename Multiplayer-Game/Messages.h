#pragma once

enum class ClientMessage : uint8
{
	Hello,
	Input,
	Ping   // NOTE(jesus): Use this message type in the virtual connection lab session
};

enum class ServerMessage : uint8
{
	Welcome,
	Unwelcome,
	Ping   // NOTE(jesus): Use this message type in the virtual connection lab session
};
