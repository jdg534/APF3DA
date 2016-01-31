#ifndef _TERRAIN_MANAGER_H_
#define _TERRAIN_MANAGER_H_

#include <vector>

#include "Terrain.h"

class TerrainManager
{
public:
	TerrainManager()
	{
	}

	~TerrainManager()
	{
	}

	static TerrainManager * getInstance();

private:
	// store a list of terrain objects

};


#endif