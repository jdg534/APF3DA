#ifndef _HEIGHT_MAP_H_
#define _HEIGHT_MAP_H_

// this class is for representing height maps, the map can be from file, or generated via the heightMapGenerator class (both types stored in the height map manager class

#include <vector>
#include <string>

class HeightMap
{
public:
	HeightMap()
	{
	}

	~HeightMap()
	{
	}

	bool loadTerrainFromFile(std::string fileName){ return false; }


private:
	unsigned int m_width;
	unsigned int m_height;
	std::vector<unsigned char> m_heightValues;
};


#endif