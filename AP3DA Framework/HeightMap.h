#ifndef _HEIGHT_MAP_H_
#define _HEIGHT_MAP_H_

// this class is for representing height maps, the map can be from file, or generated via the heightMapGenerator class (both types stored in the height map manager class

#include <vector>
#include <string>

class HeightMap
{
public:
	HeightMap();
	~HeightMap();

	bool loadTerrainFromRAWFile(std::string fileName); // asumes is square
	bool loadTerrainFromBMPFile(std::string fileName);

	// optionally include .bmp (low priorty)

	unsigned char getHeightAt(int x, int y);
	void setHeightAt(int x, int y, unsigned char value);

	unsigned int getWidth(){ return m_width; }
	unsigned int getDepth() { return m_depth; }
	std::vector<unsigned char> getHeightValues(){ return m_heightValues; }

	void setWidth(unsigned int width) { m_width = width; }
	void setDepth(unsigned int depth) { m_depth = depth; }
	


	void setheightValues(std::vector<unsigned char> heightValues)
	{ 
		m_heightValues.clear();
		for (int i = 0; i < heightValues.size(); i++)
		{
			m_heightValues.push_back(heightValues[i]);
		}
	}

private:
	unsigned int m_width;
	unsigned int m_depth;
	std::vector<unsigned char> m_heightValues;
};


#endif