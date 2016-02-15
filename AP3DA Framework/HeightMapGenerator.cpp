#include "HeightMapGenerator.h"
#include "Structures.h"
#include "MathFuncs.h"

#include <random>
#include <functional> // for std::bind()

#include <DirectXMath.h> // for XMINT3

HeightMap * HeightMapGenerator::generateHillCircle(int widthDepthVal, int iterations, int minRadius, int maxRadius, int maxRaiseHeight)
{
	/* steps:
	
	1. Start with a flat terrain
	2. Pick a random point on or near the terrain, and a random radius between some predetermined minimum and maximum
		2.1 Carefully choosing this min and max will make a terrain rough and rocky or smooth and rolling
	3. Raise a hill on the terrain centered at the point, having the given radius (can use cosine or parabola function)
	4. Go back to step 2, and repeat as many times as necessary. The number of iterations chosen will affect the appearance of the terrain.
	

	// Josh insertion after dealing with the float version
	5. renormalise the float form to range 0.0 - ??? to 0.0 - 1.0
	6. rescale float form from 0.0 - 1.0 to 0 - 255
	7. set the return value (rv) version values to the rescaled values

	*/

	HeightMap * rv = new HeightMap();

	rv->setWidth(widthDepthVal);
	rv->setDepth(widthDepthVal);

	std::vector<unsigned char> initialHeightValues;
	for (int i = 0; i < widthDepthVal * widthDepthVal; i++)
	{
		unsigned char initVal = 0;
		initialHeightValues.push_back(initVal);
	}
	rv->setheightValues(initialHeightValues);

	FloatHeightMap fhm;
	fhm.copyFromHeightMap(rv);

	// step 1 done

	// setup the random number generator(s)
	std::default_random_engine positionGenerator;
	std::uniform_int_distribution<int> positionRandIntRange(0, widthDepthVal);
	auto positionDice = std::bind(positionRandIntRange, positionGenerator);

	

	std::default_random_engine radiusGenerator;
	std::uniform_int_distribution<int> radiusRandIntRange(minRadius, maxRadius);
	auto radiusDice = std::bind(radiusRandIntRange, radiusGenerator);


	/*
	std::default_random_engine raiseValGenerator;
	std::uniform_int_distribution<int> raiseValRandIntRange(0, maxRaiseHeight);
	auto raiseValDice = std::bind(raiseValRandIntRange, raiseValGenerator);
	*/
	

	using namespace DirectX;

	// steps 2, 3 & 4 in the loop
	for (unsigned int i = 0; i < iterations; i++)
	{
		
		

		XMFLOAT2 raisePos(0,0);
		raisePos.x = positionDice();
		raisePos.y = positionDice();
		
		int radius = radiusDice();

		// int raiseVal = raiseValDice();

		for (int x = 0; x < widthDepthVal; x++)
		{
			for (int y = 0; y < widthDepthVal; y++)
			{
				XMFLOAT2 pos(x, y);
				XMVECTOR p = XMLoadFloat2(&pos);
				XMVECTOR rp = XMLoadFloat2(&raisePos);

				XMVECTOR pToRp = XMVectorSubtract(rp, p);
				// get the mag
				XMVECTOR distVF = XMVector2Length(pToRp);
				float dist;
				XMStoreFloat(&dist, distVF);
				

				float valueAtPoint = fhm.getHeightAt(x, y);

				if (dist > radius)
				{
					// do nothing
				}
				else
				{
					// determine a raise value based on distance from the radius
					
					// use the equasion from: http://www.stuffwithstuff.com/robot-frog/3d/hills/hill.html

					// z = r ^ 2 - ((x2 - x1)^2 + (y2 - y1)^2)
					// where x1, y1 is hill positon
					// where x2, y2 is cell positon
					float raiseEquPt1 = radius * radius;
					float raiseEquPtX = (pos.x - raisePos.x) * (pos.x - raisePos.x);
					float raiseEquPtY = (pos.y - raisePos.y) * (pos.y - raisePos.y);
					float raiseEquPt2 = raiseEquPtX + raiseEquPtY;

					float rasieEquFull = raiseEquPt1 - raiseEquPt2;
					
					valueAtPoint += rasieEquFull; // double check this in the tutorials

				}
				fhm.setHeightAt(x, y, valueAtPoint);
			}
		}
	}

	// step 5: renormalise the float form to range 0.0 - ??? to 0.0 - 1.0
	float maxH = 0.0f;
	for (int i = 0; i < fhm.heightValues.size(); i++)
	{
		if (maxH < fhm.heightValues[i])
		{
			maxH = fhm.heightValues[i];
		}
	}

	// calculate down scale value
	float downScale = 1.0f / maxH;

	for (int i = 0; i < fhm.heightValues.size(); i++)
	{
		fhm.heightValues[i] *= downScale;
		// also handle step 6, rescale to 0 - 255
		fhm.heightValues[i] *= 255.0f;
	}

	// rember to cite: http://www.stuffwithstuff.com/robot-frog/3d/hills/hill.html

	// now rest the values in rv (step 7)
	for (int x = 0; x < rv->getWidth(); x++)
	{
		for (int y = 0; y < rv->getDepth(); y++)
		{
			unsigned char valToSet = static_cast<unsigned char>(fhm.getHeightAt(x,y));
			rv->setHeightAt(x,y, valToSet);
		}
	}

	return rv;
}

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