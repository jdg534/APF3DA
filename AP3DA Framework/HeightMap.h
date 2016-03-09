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

	unsigned int getHeightAt(int x, int y);
	void setHeightAt(int x, int y, unsigned int value);

	unsigned int getWidth(){ return m_width; }
	unsigned int getDepth() { return m_depth; }
	std::vector<unsigned int> getHeightValues(){ return m_heightValues; }

	void setWidth(unsigned int width) { m_width = width; }
	void setDepth(unsigned int depth) { m_depth = depth; }
	


	void setHeightValues(std::vector<unsigned int> heightValues)
	{ 
		m_heightValues.clear();
		for (auto i = 0; i < heightValues.size(); i++)
		{
			m_heightValues.push_back(heightValues[i]);
		}
	}

private:
	unsigned int m_width;
	unsigned int m_depth;
	std::vector<unsigned int> m_heightValues;
};


struct FloatHeightMap
{
	// this is only for dealing with the hill circle algorith
	unsigned int width;
	unsigned int depth;
	std::vector<float> heightValues;

	void copyFromHeightMap(HeightMap * hm)
	{
		width = hm->getWidth();
		depth = hm->getDepth();
		heightValues.clear();

		std::vector<unsigned int> originalValues = hm->getHeightValues();

		for (auto i = 0; i < originalValues.size(); i++)
		{
			heightValues.push_back(originalValues[i]);
		}
	}

	float getHeightAt(int x, int y)
	{
		int yOffset = width * y;
		return heightValues[x + yOffset];
	}
	
	void setHeightAt(int x, int y, float value)
	{
		int yOffset = width * y;
		heightValues[x + yOffset] = value;
	}

};

#endif