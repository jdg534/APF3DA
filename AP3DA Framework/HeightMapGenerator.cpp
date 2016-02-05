#include "HeightMapGenerator.h"
#include "Structures.h"
#include "MathFuncs.h"

HeightMap HeightMapGenerator::generateFaultFormation(int widthDepthVal, int iterations)
{
	HeightMap rv;
	rv.setWidth(widthDepthVal);
	rv.setDepth(widthDepthVal);

	srand(534);

	std::vector<unsigned char> initialHeightValues;
	for (int i = 0; i < widthDepthVal * widthDepthVal; i++)
	{
		unsigned char initVal = 0;
		initialHeightValues.push_back(initVal);
	}



	for (int i = 0; i < iterations; i++)
	{
		// get a random fault formation line

		if (i % 2 == 0)
		{
			// even numbered iteration
			// line from top to bottom
			FaultFormationLine topToBottom;
			
			// loop and lerp
		}
		else
		{
			// line from left to right 
			FaultFormationLine leftToRight;
		}
	}

	return rv;
}