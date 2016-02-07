#include "HeightMapGenerator.h"
#include "Structures.h"
#include "MathFuncs.h"

#include <random>
#include <functional> // for std::bind()


HeightMap * HeightMapGenerator::generateFaultFormation(int widthDepthVal, int iterations)
{
	HeightMap * rv;
	rv = new HeightMap();
	rv->setWidth(widthDepthVal);
	rv->setDepth(widthDepthVal);

	// setup the random number generator
	std::default_random_engine generator;
	std::uniform_int_distribution<int> randIntRange(0, widthDepthVal);
	auto dice = std::bind(randIntRange, generator);

	// test dice results
	
	/* // worked
	bool gotSameValueInARow = false;
	int lastValue = 0;
	for (int i = 0; i < 50; i++)
	{
		int r = dice();
		if (r == lastValue)
		{
			gotSameValueInARow = true;
		}
		while (gotSameValueInARow)
		{
			r = dice();
			if (r != lastValue)
			{
				gotSameValueInARow = false;
			}
		}
		// got new value for r
		lastValue = r;
	}
	*/

	std::vector<unsigned char> initialHeightValues;
	for (int i = 0; i < widthDepthVal * widthDepthVal; i++)
	{
		unsigned char initVal = 0;
		initialHeightValues.push_back(initVal);
	}
	rv->setheightValues(initialHeightValues);

	bool positiveSideOfLine = true;

	std::default_random_engine alterValGen;
	std::uniform_int_distribution<int> alterValRange(0, 255);
	auto alterValDice = std::bind(randIntRange, generator);

	bool increaseTopTurn = true;
	bool increaseLeftTurn = true;

	for (int i = 0; i < iterations; i++)
	{
		// get a random fault formation line

		if (i % 2 == 0)
		{
			// even numbered iteration
			// line from top to bottom
			FaultFormationLine topToBottom;
			topToBottom.startSideIndex = dice();
			topToBottom.endSideIndex = dice();

			// get vector representations of the positions
			DirectX::XMFLOAT2 startPos((float) topToBottom.startSideIndex, (float) 0);
			DirectX::XMFLOAT2 endPos((float)topToBottom.endSideIndex, (float)rv->getDepth());

			unsigned char altVal = alterValDice();
			for (int j = 0; j  < rv->getDepth(); j ++)
			{
				// j in the depth of a cell not the width
				int cutoffPointForRow = findLinePointOnRow(startPos, endPos, j, rv);
				for (int k = 0; k < rv->getWidth(); k++)
				{
					// rember k is the width across for the row

					unsigned char hmVal = rv->getHeightAt(k, j);
					/* code for determining if adding or lowering the height map value
					(alter hmVal)
					*/

					// determine if hit the cut off point
					bool beforeCutOffPoint = k <= cutoffPointForRow;
					
					if (beforeCutOffPoint)
					{
						if (increaseTopTurn)
						{
							// move the value up
							hmVal += altVal;
						}
						else
						{
							// move the value down
							hmVal -= altVal;
						}
						
					}
					else
					{
						if (increaseTopTurn)
						{
							// move the value down
							hmVal -= altVal;
						}
						else
						{
							// move the value up
							hmVal += altVal;
						}
					}

					increaseTopTurn = !increaseTopTurn;

					rv->setHeightAt(k, j, hmVal);
				}
			}
		}
		else
		{


			// line from left to right 
			FaultFormationLine leftToRight;
			leftToRight.startSideIndex = dice();
			leftToRight.endSideIndex = dice();


			// get vector representations of the positions
			DirectX::XMFLOAT2 startPos((float)0, (float)leftToRight.startSideIndex);
			DirectX::XMFLOAT2 endPos((float)rv->getWidth(), (float)leftToRight.endSideIndex);

			unsigned char altVal = alterValDice();
			for (int j = 0; j < rv->getWidth(); j++)
			{
				// j in the width of a cell not the depth
				int cutOffPointForColumn = findLinePointOnColumn(startPos, endPos, j, rv);
				
				for (int k = 0; k < rv->getDepth(); k++)
				{
					// k for the depth

					unsigned char hmVal = rv->getHeightAt(j, k);
					/* code for determining if adding or lowering the height map value
					(alter hmVal)
					*/

					// determine if hit the cut off point
					bool beforeCutOffPoint = k <= cutOffPointForColumn;

					if (beforeCutOffPoint)
					{
						if (increaseLeftTurn)
						{
							// move the value up
							hmVal += altVal;
						}
						else
						{
							// move the value down
							hmVal -= altVal;
						}

					}
					else
					{
						if (increaseLeftTurn)
						{
							// move the value down
							hmVal -= altVal;
						}
						else
						{
							// move the value up
							hmVal += altVal;
						}
					}

					increaseLeftTurn = !increaseLeftTurn;

					rv->setHeightAt(j, k, hmVal);

				}
			}
		}
	}

	return rv;
}

int HeightMapGenerator::findLinePointOnColumn(DirectX::XMFLOAT2 leftPoint, DirectX::XMFLOAT2 rightPoint, int column, HeightMap * hm)
{
	// use lerp to determine at what point dealing ith the correct column
	float stepSize = 0.001f;
	float currentStep = 0.0f;
	while (currentStep <= 1.0f)
	{
		DirectX::XMFLOAT2 pos;
		pos.x = MathFuncs::lerp(leftPoint.x, rightPoint.x, currentStep);
		pos.y = MathFuncs::lerp(leftPoint.y, rightPoint.y, currentStep);
		// lerping down to specific row
		if ((int)pos.x == column)
		{
			return (int)pos.y;
		}
		currentStep += stepSize;
	}

	return -1;
}

int HeightMapGenerator::findLinePointOnRow(DirectX::XMFLOAT2 topPoint, DirectX::XMFLOAT2 bottomPoint, int row, HeightMap * hm)
{
	// use lerp to determine at what point dealing ith the correct row
	float stepSize = 0.001f;
	float currentStep = 0.0f;
	while (currentStep <= 1.0f)
	{
		DirectX::XMFLOAT2 pos;
		pos.x = MathFuncs::lerp(topPoint.x, bottomPoint.x, currentStep);
		pos.y = MathFuncs::lerp(topPoint.y, bottomPoint.y,currentStep);
		// lerping down to specific row
		if ((int) pos.y == row)
		{
			return (int)pos.x;
		}
		currentStep += stepSize;
	}

	return -1;
}