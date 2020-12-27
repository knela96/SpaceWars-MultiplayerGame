#pragma once
#include <map>

enum class ReplicationAction
{
	None,Create,Update,UpdateDummy,Destroy
};

struct ReplicationCommand
{
	ReplicationAction action;
	uint32 networkId;
};

class ReplicationManagerServer
{
public:
	void create(uint32 networkId);
	void update(uint32 networkId);
	void destroy(uint32 networkId);

	void write(OutputMemoryStream & packet);

	std::map<uint32, ReplicationAction> commands;
};