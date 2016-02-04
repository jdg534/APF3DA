#include "HeightMap.h"

#include <fstream>

HeightMap::HeightMap()
{
	m_width = 0;
	m_depth = 0;
}

HeightMap::~HeightMap()
{

}

bool HeightMap::loadTerrainFromRAWFile(std::string fileName) // asumes is square
{
	std::ifstream inFile(fileName, std::ios::binary);

	if (!inFile.good())
	{
		return false;
	}

	inFile.seekg(std::ios::end);
	int fileSize = inFile.tellg();
	inFile.seekg(std::ios::beg);

	m_heightValues.resize(fileSize);

	inFile.read(reinterpret_cast<char *>(&m_heightValues[0]),m_heightValues.size());

	m_width = fileSize / 2;
	m_depth = m_width;

	return true;
}

unsigned char HeightMap::getHeightAt(int x, int y)
{
	// need to calculate y off set
	// (x offset = x) anyway
	int yOffset = m_width * y;
	return m_heightValues[x + yOffset];
}