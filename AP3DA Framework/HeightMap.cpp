#include "HeightMap.h"

#include "Structures.h"

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
	std::ifstream inFile(fileName, std::ios::binary);// | std::ios::ate);

	// std::ios::ate for getting the file size?

	if (!inFile.good())
	{
		return false;
	}


	

	inFile.seekg(0, std::ios::end);
	int fileSize = inFile.tellg();
	inFile.seekg(0, std::ios::beg);

	m_heightValues.resize(fileSize);

	inFile.read(reinterpret_cast<char *>(&m_heightValues[0]),m_heightValues.size());

	int area = m_heightValues.size();

	int sqrtOfArea = sqrt(area);

	// m_width = (fileSize -1) / 2;
	m_width = sqrtOfArea;
	m_depth = m_width;

	// */

	// tutorial code
	/*
	inFile.seekg(std::ios::end);
	int fileSize = inFile.tellg();
	inFile.seekg(std::ios::beg);

	m_heightValues.resize(fileSize);

	inFile.read(reinterpret_cast<char *>(&m_heightValues[0]), m_heightValues.size());

	m_width = fileSize / 2;
	m_depth = m_width;
	*/

	return true;
}

bool HeightMap::loadTerrainFromBMPFile(std::string fileName)
{
	std::ifstream in(fileName, std::ios::binary);

	if (!in.good())
	{
		return false;
	}

	BmpMagicNumber bmn;
	BmpFileHeader bfh;
	BmpInfoHeader bih;

	in.read(reinterpret_cast<char *>(&bmn), sizeof(BmpMagicNumber));
	in.read(reinterpret_cast<char *>(&bfh), sizeof(BmpFileHeader));
	in.read(reinterpret_cast<char *>(&bih), sizeof(BmpInfoHeader));

	m_width = bih.width;
	m_depth = bih.height;

	int pixels = m_width * m_depth;

	m_heightValues.resize(pixels);

	// now that read the headers, the 

	if (bih.bitsPerPixel == 8)
	{
		// 1 byte per pixel

		std::vector<unsigned char> readIn;
		readIn.resize(pixels);
		in.read(reinterpret_cast<char *>(&readIn[0]), readIn.size());

		for (auto i = 0; i < m_heightValues.size(); i++)
		{
			m_heightValues[i] = readIn[i];
		}

		// in.read(reinterpret_cast<char *>(&m_heightValues[0]), m_heightValues.size());
	}
	else if (bih.bitsPerPixel == 16)
	{
		// 2 bytes per pixel
		// loop as signed short
		for (int i = 0; i < pixels; i++)
		{
			unsigned short tmp = 0;
			in.read(reinterpret_cast<char *>(&tmp), sizeof(unsigned short));

			// down scale short from 0 - 65535 to 0 - 255

			float downScaleBy = 1.0f / 65535.0f; // to 0.0 - 1.0
			float downScaled = (float)tmp * downScaleBy;

			float upScaled = downScaled * 255.0f;
			m_heightValues[i] = static_cast<unsigned char>(upScaled);
		}

	}
	else if (bih.bitsPerPixel == 24)
	{
		// 3 bytes per pixel
		for (int i = 0; i < pixels; i++)
		{
			BmpPx24Bit tmp;
			in.read(reinterpret_cast<char *>(&tmp), sizeof(BmpPx24Bit));

			m_heightValues[i] = (tmp.blue + tmp.green + tmp.red) / 3;

		}
	}
	else if (bih.bitsPerPixel == 32)
	{
		// 4 bytes per pixel

		// just deal with the alpha channel
		for (int i = 0; i < pixels; i++)
		{
			BmpPx32Bit tmp;
			in.read(reinterpret_cast<char *>(&tmp), sizeof(BmpPx32Bit));
			m_heightValues[i] = tmp.alpha;
		}
	}
	return true;
}

unsigned int HeightMap::getHeightAt(int x, int y)
{
	// need to calculate y off set
	// (x offset = x) anyway
	int yOffset = m_width * y;
	return m_heightValues[x + yOffset];
}

void HeightMap::setHeightAt(int x, int y, unsigned int value)
{
	int yOffset = m_width * y;
	m_heightValues[yOffset + x] = value;
}