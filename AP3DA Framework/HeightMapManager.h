#ifndef _HEIGHT_MAP_MANAGER_H_
#define _HEIGHT_MAP_MANAGER_H_

#include <vector>
#include <string>

#include "HeightMap.h"

struct HeightMapEntry
{
	std::string id;
	HeightMap * heightMap;
};

class HeightMapManager
{
public:
	HeightMapManager();
	~HeightMapManager();
	
	bool loadFromDeffinitionFile(std::string deffinitionFile);

	bool addHeightMap(HeightMap * toAdd, std::string ID);
	HeightMap * getHeightMapWithID(std::string ID);

	void shutdown();

	bool getWidthDepthValForHeightMaps(int & sharedWidthDepthValue);

	std::vector<std::string> getIDs();

private:
	std::vector<HeightMapEntry> m_heightMaps;


	bool IDAvailable(std::string id);
};


#endif