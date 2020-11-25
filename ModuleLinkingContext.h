#pragma once

class ModuleLinkingContext : public Module
{
public:

	void registerNetworkGameObject(GameObject *gameObject);

	void registerNetworkGameObjectWithNetworkId(GameObject *gameObject, uint32 networkId);

	GameObject *getNetworkGameObject(uint32 networkId, bool safeNetworkIdCheck = true);

	void getNetworkGameObjects(GameObject *gameObjects[MAX_NETWORK_OBJECTS], uint16 *count);

	uint16 getNetworkGameObjectsCount() const;

	void unregisterNetworkGameObject(GameObject * gameObject);

	void clear();

private:

	// NOTE(jesus): The networkId of a gameObject is the combination of
	// two 2-byte words: magicNumber (higher bytes) and arrayIndex
	// (lower bytes)
	// The lower bytes contain the index within the array of network
	// game objects.
	// The higher bytes contain an ever increasing number.
	// So: networkId = (0xffff0000 & (magicNumber << 16)) | (0x0000ffff & arrayIndex)
	// With this combination, with a networkId we always know the
	// position of a certain object in the array of networkdObjects,
	// and can uniquely identify objects that started existing later
	// but take the same position in the array

	uint32 nextMagicNumber = 1;

	uint32 makeNetworkId(uint16 arrayIndex);
	uint16 arrayIndexFromNetworkId(uint32 networkId);

	GameObject *networkGameObjects[MAX_NETWORK_OBJECTS] = {};

	uint16 networkGameObjectsCount = 0;
};

