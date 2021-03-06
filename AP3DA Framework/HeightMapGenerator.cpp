#include "HeightMapGenerator.h"
#include "Structures.h"
#include "MathFuncs.h"

#include <random>
#include <functional> // for std::bind()

#include <DirectXMath.h> // for XMINT3, think of it as XM INT 3

#include <cmath>

HeightMap * HeightMapGenerator::generateDiamonSquare(int widthDepthVal, float rangeReductionFactor, unsigned int startingRange)
{
	/* About
	widthDepthValue needs to be (2 ^ n) + 1, value e.g. 5, 9.... 
	
	see: https://en.wikipedia.org/wiki/Diamond-square_algorithm

	1. set the 4 corner values to some random value

	2. carry out the recursive steps (till all values have been altered):
	2.1 Diamon step
	(use the square corner values to set the middle value)


	2.2 Square steps
	(use the corner values to set value for the middle of an edge, takes the corner & middle value into account)


	*/


	// a simpler example: http://gamedev.stackexchange.com/questions/37389/diamond-square-terrain-generation-problem


	checkAndFixWidthHeightValForDimondSquare(widthDepthVal);

	HeightMap * rv = new HeightMap();

	rv->setWidth(widthDepthVal);
	rv->setDepth(widthDepthVal);

	std::vector<unsigned int> initialHeightValues;
	for (int i = 0; i < widthDepthVal * widthDepthVal; i++)
	{
		unsigned int initVal = 0;
		initialHeightValues.push_back(initVal);
	}
	rv->setHeightValues(initialHeightValues);


	std::default_random_engine rndHeightEng;
	std::uniform_int_distribution<int> rndHeightDist(0, startingRange); // this is the starting range
	auto rndHeightDice = std::bind(rndHeightDist,rndHeightEng);


	Square s;
	s.left = 0;
	s.top = 0;
	s.right = widthDepthVal - 1;
	s.bottom = s.right;
	
	// set the corrner values
	int r = rndHeightDice();


	rv->setHeightAt(s.left, s.top, r);
	r = rndHeightDice();
	rv->setHeightAt(s.right, s.top, r);
	r = rndHeightDice();
	rv->setHeightAt(s.left, s.bottom, r);
	r = rndHeightDice();
	rv->setHeightAt(s.right, s.bottom, r);

	// get the worker to take over

	// calculate a reduced range
	float downScaleRangeBy = powf(2.0f, -rangeReductionFactor);
	unsigned downScaledRange = (float)startingRange * downScaleRangeBy;

	diamondSquareWorker(rv, &s, downScaledRange, rangeReductionFactor);

	
	// now rescale back down to 0 - 255 range
	unsigned int biggestValueEncountered = 0;
	std::vector<unsigned int> heightValues = rv->getHeightValues();
	for (auto i = 0; i < heightValues.size(); i++)
	{
		if (biggestValueEncountered < heightValues[i])
		{
			biggestValueEncountered = heightValues[i];
		}
	}

	float downScaleBy = 255.0f / (float)biggestValueEncountered;
	for (auto i = 0; i < heightValues.size(); i++)
	{
		float scaledValue = (float)heightValues[i] * downScaleBy;
		heightValues[i] = (unsigned int)scaledValue;
	}

	rv->setHeightValues(heightValues);

	return rv;
}

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

	std::vector<unsigned int> initialHeightValues;
	for (int i = 0; i < widthDepthVal * widthDepthVal; i++)
	{
		unsigned int initVal = 0;
		initialHeightValues.push_back(initVal);
	}
	rv->setHeightValues(initialHeightValues);

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

	std::vector<unsigned int> initialHeightValues;
	for (int i = 0; i < widthDepthVal * widthDepthVal; i++)
	{
		unsigned int initVal = 0;
		initialHeightValues.push_back(initVal);
	}
	rv->setHeightValues(initialHeightValues);

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

void HeightMapGenerator::checkAndFixWidthHeightValForDimondSquare(int & widthDepthVal)
{
	// the value must be (2 ^ n) + 1, 5 min
	// to get 5 value is (2 ^ 2) + 1
	// 1000 should be enough / overkill

	std::vector<unsigned int> okVals;

	for (auto i = 2; i < 1000; i++)
	{
		unsigned int val = (powf(2, i)) + 1;
		okVals.push_back(val);
	}

	// now see if it's already an ok value
	for (auto i = 0; i < okVals.size(); i++)
	{
		if (widthDepthVal == okVals[i])
		{
			return;
		}
	}

	// need to set it to the next okVal
	for (auto i = 0; i < okVals.size(); i++)
	{
		if (widthDepthVal < okVals[i])
		{
			widthDepthVal = okVals[i];
			return;
		}
	}

	widthDepthVal = okVals[okVals.size() -1]; // just give then the max value if they asked for bigger value then is in okVals
}

void HeightMapGenerator::diamondSquareWorker(HeightMap * hm, Square * workOnArea, unsigned int range, float h)
{
	// this is recursive!

	/*
	2. carry out the recursive steps (till all values have been altered):
	2.1 Diamon step
	(use the square corner values to set the middle value)


	2.2 Square steps
	(use the corner values to set value for the middle of an edge, takes the corner & middle value into account)
	*/

	// for each iteration don't for get the down scale the range

	// 1. down scale the range by (range * (2 ^ (-h)))

	
	

	float downScaleRangeBy = powf(2.0f, -h);
	int rangeDownScaled = (float)range * downScaleRangeBy;
	
	
	// 2.1 Diamon step (set the middle value, for the edges)

	using namespace DirectX;
	XMINT2 midPoint;
	midPoint.x = MathFuncs::lerp(workOnArea->left, workOnArea->right, 0.5f);
	midPoint.y = MathFuncs::lerp(workOnArea->top, workOnArea->bottom, 0.5f);

	squareStep(hm, workOnArea, range);
	diamondStep(hm, workOnArea, range);



	// determin if need to do another recursion(s), 1 per Quad (like Quadtree)
	if (workOnArea->right - workOnArea->left >= 3)
	{
		// need to do another recursion
		Square tlQuad, trQuad,
			blQuad, brQuad;
		tlQuad.left = workOnArea->left;
		tlQuad.top = workOnArea->top;
		tlQuad.right = midPoint.x;
		tlQuad.bottom = midPoint.y;

		trQuad.left = midPoint.x;
		trQuad.top = workOnArea->top;
		trQuad.right = workOnArea->right;
		trQuad.bottom = midPoint.y;

		blQuad.left = workOnArea->left;
		blQuad.top = midPoint.y;
		blQuad.right = midPoint.x;
		blQuad.bottom = workOnArea->bottom;

		brQuad.left = midPoint.x;
		brQuad.top = midPoint.y;
		brQuad.right = workOnArea->right;
		brQuad.bottom = workOnArea->bottom;


		// do the Square step for each square, (the diamond steps, on these need to achknowldge the square below them)
		squareStep(hm, &tlQuad, rangeDownScaled);
		squareStep(hm, &trQuad, rangeDownScaled);
		squareStep(hm, &blQuad, rangeDownScaled);
		squareStep(hm, &brQuad, rangeDownScaled);


		diamondSquareWorker(hm, &tlQuad, rangeDownScaled, h);
		diamondSquareWorker(hm, &trQuad, rangeDownScaled, h);
		diamondSquareWorker(hm, &blQuad, rangeDownScaled, h);
		diamondSquareWorker(hm, &brQuad, rangeDownScaled, h);
	}
	else // this is just for debugging
	{
		int dif = workOnArea->right - workOnArea->left;
		char e;
		e = 'e';
	}
}

bool HeightMapGenerator::isNaboringSquare(HeightMap * hmBeingGenerated, Square * currentArea, unsigned int DirectionEnum)
{
	// this func is used for determineing if there's a square, in the associated area
	unsigned int squrSize = currentArea->right - currentArea->left;

	unsigned int hightMapWidthHeightVal = hmBeingGenerated->getWidth(); // the width & depth will have the same value

	if (DirectionEnum == DIA_SQUR_NEIGHB_SQUR_DIR_LEFT)
	{
		if (currentArea->left - squrSize >= 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if(DirectionEnum == DIA_SQUR_NEIGHB_SQUR_DIR_RIGHT)
	{
		if (currentArea->right + squrSize < hightMapWidthHeightVal)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (DirectionEnum == DIA_SQUR_NEIGHB_SQUR_DIR_UP)
	{
		if (currentArea->top - squrSize >= 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (DirectionEnum == DIA_SQUR_NEIGHB_SQUR_DIR_DOWN)
	{
		if (currentArea->bottom + squrSize < hightMapWidthHeightVal)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}

unsigned int HeightMapGenerator::squareMidpointValue(HeightMap * hmBeingGenerated, Square * areaOfInterest)
{
	using namespace DirectX;

	XMFLOAT2 midPointCoord;
	midPointCoord.x = MathFuncs::lerp(areaOfInterest->left, areaOfInterest->right, 0.5f);
	midPointCoord.y = MathFuncs::lerp(areaOfInterest->left, areaOfInterest->right, 0.5f);

	unsigned int midpointValue = hmBeingGenerated->getHeightAt(midPointCoord.x, midPointCoord.y);
	if (midpointValue == 0)
	{
		// need to come up wih a likely (ish) value
		unsigned int aoiTopLeft, aoiTopRight,
			aoiBottomLeft, aoiBottomRight;
		aoiTopLeft = hmBeingGenerated->getHeightAt(areaOfInterest->left, areaOfInterest->top);
		aoiTopRight = hmBeingGenerated->getHeightAt(areaOfInterest->right, areaOfInterest->top);
		aoiBottomLeft = hmBeingGenerated->getHeightAt(areaOfInterest->left, areaOfInterest->bottom);
		aoiBottomRight = hmBeingGenerated->getHeightAt(areaOfInterest->right, areaOfInterest->bottom);

		unsigned int sum = aoiTopLeft + aoiTopRight + aoiBottomLeft + aoiBottomRight;
		return sum / 4; // skipping the rand dowm value
	}
	return midpointValue;
}

void HeightMapGenerator::diamondStep(HeightMap * hmBeingGenerated, Square * areaOfParentSquare, unsigned int range)
{
	Diamond top, bottom, left, right;
	DirectX::XMINT2 topMid, bottomMid, leftMid, rightMid;


	DirectX::XMFLOAT2 squrMidPoint;
	squrMidPoint.x = MathFuncs::lerp(areaOfParentSquare->left, areaOfParentSquare->right, 0.5f);
	squrMidPoint.y = MathFuncs::lerp(areaOfParentSquare->top, areaOfParentSquare->bottom, 0.5f);

	top.bottom = squrMidPoint;
	bottom.top = squrMidPoint;
	left.right = squrMidPoint;
	right.left = squrMidPoint;

	unsigned int diamondSize = 0;

	top.left.x = areaOfParentSquare->left;
	top.left.y = areaOfParentSquare->top;
	top.right.x = areaOfParentSquare->right;
	top.right.y = areaOfParentSquare->top;
	// still need to calc top.top

	bottom.left.x = areaOfParentSquare->left;
	bottom.left.y = areaOfParentSquare->bottom;
	bottom.right.x = areaOfParentSquare->right;
	bottom.right.y = areaOfParentSquare->bottom;
	// still need to calc bottom.bottom

	left.top = top.left;
	left.bottom = bottom.left;
	// still need to calc left.left

	right.top = top.right;
	right.bottom = bottom.right;
	// still need to calc right.right

	diamondSize = top.right.x - top.left.x;
	
	// now for top.top, bottom.bottom, right.right, left.left
	// could wrap around?

	top.top.x = top.bottom.x;
	top.top.y = top.bottom.y - diamondSize;

	bottom.bottom.x = bottom.top.x;
	bottom.bottom.y = bottom.top.y + diamondSize;

	left.left.y = left.right.y;
	left.left.x = left.right.x - diamondSize;

	right.right.y = right.left.y;
	right.right.x = right.left.x + diamondSize;

	topMid = midPoint(&top);
	leftMid = midPoint(&left);
	rightMid = midPoint(&right);
	bottomMid = midPoint(&bottom);

	// now average the values for the diamond
	unsigned int avg, diamondTop, diamondBottom, diamondLeft, diamondRight;

	diamondTop = diamondBottom = diamondLeft = diamondRight = 0;

	if (top.top.y >= 0)
	{
		diamondTop = hmBeingGenerated->getHeightAt(top.top.x, top.top.y);
	}
	
	diamondBottom = hmBeingGenerated->getHeightAt(top.bottom.x, top.bottom.y);
	diamondLeft = hmBeingGenerated->getHeightAt(top.left.x, top.left.y);
	diamondRight = hmBeingGenerated->getHeightAt(top.right.x, top.right.y);

	

	avg = (diamondTop + diamondBottom + diamondLeft + diamondRight) / 4;
	int randVal = rand();
	
	if (randVal != 0 && range != 0)
	{
		avg += randVal % range;
	}
	

	diamondTop = diamondBottom = diamondLeft = diamondRight = 0;

	hmBeingGenerated->setHeightAt(topMid.x, topMid.y, avg);

	diamondTop = hmBeingGenerated->getHeightAt(bottom.top.x, bottom.top.y);
	if (bottom.bottom.y < hmBeingGenerated->getDepth())
	{
		diamondBottom = hmBeingGenerated->getHeightAt(bottom.bottom.x, bottom.bottom.y);
	}
	
	diamondLeft = hmBeingGenerated->getHeightAt(bottom.left.x, bottom.left.y);
	diamondRight = hmBeingGenerated->getHeightAt(bottom.right.x, bottom.right.y);

	avg = (diamondTop + diamondBottom + diamondLeft + diamondRight) / 4;
	
	int randValue = rand();
	if (randValue != 0 && range != 0)
	{
		avg += randValue % range;
	}
	

	diamondTop = diamondBottom = diamondLeft = diamondRight = 0;

	hmBeingGenerated->setHeightAt(bottomMid.x, bottomMid.y, avg);

	diamondTop = hmBeingGenerated->getHeightAt(left.top.x, left.top.y);
	diamondBottom = hmBeingGenerated->getHeightAt(left.bottom.x, left.bottom.y);
	if (left.left.x >= 0)
	{
		diamondLeft = hmBeingGenerated->getHeightAt(left.left.x, left.left.y);
	}
	diamondRight = hmBeingGenerated->getHeightAt(left.right.x, left.right.y);

	avg = (diamondTop + diamondBottom + diamondLeft + diamondRight) / 4;
	
	if (range >= 1)
	{
		avg += rand() % range;
	}
	

	diamondTop = diamondBottom = diamondLeft = diamondRight = 0;

	hmBeingGenerated->setHeightAt(leftMid.x, leftMid.y, avg);
	
	diamondTop = hmBeingGenerated->getHeightAt(right.top.x, right.top.y);
	diamondBottom = hmBeingGenerated->getHeightAt(right.bottom.x, right.bottom.y);
	diamondLeft = hmBeingGenerated->getHeightAt(right.left.x, right.left.y);
	if (right.right.x < hmBeingGenerated->getWidth())
	{
		diamondRight = hmBeingGenerated->getHeightAt(right.right.x, right.right.y);
	}
	

	avg = (diamondTop + diamondBottom + diamondLeft + diamondRight) / 4;
	if (range >= 1)
	{
		avg += rand() % range;
	}
	

	

	hmBeingGenerated->setHeightAt(leftMid.x, leftMid.y, avg);
}

void HeightMapGenerator::squareStep(HeightMap * hmBeingGenerated, Square * areaOfSquare, unsigned int range)
{
	// just deal with the midpoint
	unsigned int topLeft, topRight,
		bottomLeft, bottomRight;
	topLeft = hmBeingGenerated->getHeightAt(areaOfSquare->left, areaOfSquare->top);
	topRight = hmBeingGenerated->getHeightAt(areaOfSquare->right, areaOfSquare->top);
	bottomLeft = hmBeingGenerated->getHeightAt(areaOfSquare->left, areaOfSquare->bottom);
	bottomRight = hmBeingGenerated->getHeightAt(areaOfSquare->right, areaOfSquare->bottom);
	auto avg = (topLeft + topRight + bottomLeft + bottomRight) / 4;

	int randVal = rand();
	if (randVal != 0 && range != 0)
	{
		avg += randVal % range;
	}

	DirectX::XMINT2 midpointCoord;
	midpointCoord.x = MathFuncs::lerp(areaOfSquare->left, areaOfSquare->right, 0.5f);
	midpointCoord.y = MathFuncs::lerp(areaOfSquare->top, areaOfSquare->bottom, 0.5f);
	hmBeingGenerated->setHeightAt(midpointCoord.x, midpointCoord.y, avg);
}

DirectX::XMINT2 HeightMapGenerator::midPoint(Diamond * d)
{
	DirectX::XMINT2 returnValue(0,0);
	returnValue.x = MathFuncs::lerp(d->left.x, d->right.x, 0.5f);
	returnValue.y = MathFuncs::lerp(d->top.y, d->bottom.y, 0.5f);
	return returnValue;
}