#include "HeightMapManager.h"

#include <fstream>

#include "HeightMapGenerator.h"

HeightMapManager::HeightMapManager()
{

}

HeightMapManager::~HeightMapManager()
{

}

bool HeightMapManager::loadFromDeffinitionFile(std::string deffinitionFile)
{
	std::ifstream defFile(deffinitionFile);

	if (!defFile.good())
	{
		return false;
	}

	std::string currentBit = "";
	while (currentBit != "START")
	{
		defFile >> currentBit;
	}

	HeightMapGenerator hmg;
	

	while (currentBit != "END")
	{
		defFile >> currentBit;
		if (currentBit == "HeightMapFile")
		{
			std::string idStr = "";
			std::string fileStr = "";
			defFile >> idStr;
			std::getline(defFile,fileStr);
			int q1, q2;
			q1 = fileStr.find_first_of("\"");
			q2 = fileStr.find_last_of("\"");
			fileStr = fileStr.substr(q1 + 1, q2 - (q1 + 1));

			HeightMap * hm = new HeightMap();
			if (fileStr.find(".bmp") != std::string::npos)
			{
				if (!hm->loadTerrainFromBMPFile(fileStr))
				{
					// it didn't work
				}
			}
			else if (fileStr.find(".raw") != std::string::npos)
			{
				if (!hm->loadTerrainFromRAWFile(fileStr))
				{
					// it didn't work
				}
			}
			else
			{
				delete hm;
				return false;
			}
			// if got this far the Height map has loaded correctly
			HeightMapEntry wrapper;
			wrapper.id = idStr;
			wrapper.heightMap = hm;
			m_heightMaps.push_back(wrapper);
		}
		else if (currentBit == "GenDiamondSquare")
		{
			std::string idStr = "";
			int widthDepthVal = 0;
			float rangeReductionFactor = 0.0f;
			int startingRange = 0;
			defFile >> idStr;
			defFile >> widthDepthVal;
			defFile >> rangeReductionFactor;
			defFile >> startingRange;
			HeightMapEntry wrapper;
			wrapper.id = idStr;
			wrapper.heightMap = hmg.generateDiamonSquare(widthDepthVal, rangeReductionFactor, startingRange);
			m_heightMaps.push_back(wrapper);
		}
		else if (currentBit == "GenFaultFormation")
		{

			std::string idStr = "";
			int widthDepthVal = 0;
			int nIterations = 0;
			defFile >> idStr;
			defFile >> widthDepthVal;
			defFile >> nIterations;
			HeightMapEntry wrapper;
			wrapper.id = idStr;
			wrapper.heightMap = hmg.generateFaultFormation(widthDepthVal, nIterations);
			m_heightMaps.push_back(wrapper);
		}
		else if (currentBit == "GenCircleHill")
		{
			std::string idStr = "";
			int widthDepthVal = 0;
			int nIterations = 0;
			int minRadius = 0;
			int maxRadius = 0;
			int maxRaiseHeight = 0;
			defFile >> idStr;
			defFile >> widthDepthVal;
			defFile >> nIterations;
			defFile >> minRadius;
			defFile >> maxRadius;
			defFile >> maxRaiseHeight;
			HeightMapEntry wrapper;
			wrapper.id = idStr;
			wrapper.heightMap = hmg.generateHillCircle(widthDepthVal, nIterations, minRadius, maxRadius, maxRaiseHeight);
			m_heightMaps.push_back(wrapper);
		}
	}
	defFile.close();
	return true;
}

bool HeightMapManager::addHeightMap(HeightMap * toAdd, std::string ID)
{
	if (!IDAvailable(ID))
	{
		return false;
	}

	HeightMapEntry wrapper;
	wrapper.id = ID;
	wrapper.heightMap = toAdd;
	m_heightMaps.push_back(wrapper);
	return true;
}

HeightMap * HeightMapManager::getHeightMapWithID(std::string ID)
{
	for (auto i = 0; i < m_heightMaps.size(); i++)
	{
		if (m_heightMaps[i].id == ID)
		{
			return m_heightMaps[i].heightMap;
		}
	}
	return nullptr;
}

void HeightMapManager::shutdown()
{
	for (auto i = 0; i < m_heightMaps.size(); i++)
	{
		delete m_heightMaps[i].heightMap;
	}
	m_heightMaps.clear();
}

bool HeightMapManager::getWidthDepthValForHeightMaps(int & sharedWidthDepthValue)
{
	sharedWidthDepthValue = m_heightMaps[0].heightMap->getWidth();
	for (auto i = 1; i < m_heightMaps.size(); i++)
	{
		int widthDepthVal = m_heightMaps[i].heightMap->getWidth();
		if (widthDepthVal != sharedWidthDepthValue)
		{
			return false;
		}
	}
	return true;
}

std::vector<std::string> HeightMapManager::getIDs()
{
	std::vector<std::string> rv;
	for (auto i = 0; i < m_heightMaps.size(); i++)
	{
		rv.push_back(m_heightMaps[i].id);
	}
	return rv;
}

bool HeightMapManager::IDAvailable(std::string id)
{
	unsigned int nHeightMaps = m_heightMaps.size();
	for (unsigned int i = 0; i < nHeightMaps; i++)
	{
		if (m_heightMaps[i].id == id)
		{
			return false;
		}
	}
	return true;
}