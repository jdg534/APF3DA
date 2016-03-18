#ifndef _HEIGHT_MAP_GENERATOR_H_
#define _HEIGHT_MAP_GENERATOR_H_

#include "HeightMap.h"

// #include <d3d11.h>
#include <DirectXMath.h>
#include "Structures.h"


enum DiamonSquareNeighbourSquareDir
{
	DIA_SQUR_NEIGHB_SQUR_DIR_LEFT = 1,
	DIA_SQUR_NEIGHB_SQUR_DIR_RIGHT,
	DIA_SQUR_NEIGHB_SQUR_DIR_UP,
	DIA_SQUR_NEIGHB_SQUR_DIR_DOWN
};

class HeightMapGenerator
{
public:

	HeightMap * generateDiamonSquare(int widthDepthVal, float rangeReductionFactor, unsigned int startingRange);

	HeightMap * generateHillCircle(int widthDepthVal, int iterations, int minRadius, int maxRadius, int maxRaiseHeight);

	HeightMap * generateFaultFormation(int widthDepthVal, int iterations);



private:

	int findLinePointOnColumn(DirectX::XMFLOAT2 leftPoint, DirectX::XMFLOAT2 rightPoint, int column, HeightMap * hm);
	
	int findLinePointOnRow(DirectX::XMFLOAT2 topPoint, DirectX::XMFLOAT2 bottomPoint, int row, HeightMap * hm);

	void checkAndFixWidthHeightValForDimondSquare(int & widthDepthVal);

	void diamondSquareWorker(HeightMap * hm, Square * workOnArea, unsigned int range, float h);

	bool isNaboringSquare(HeightMap * hmBeingGenerated, Square * currentArea, unsigned int DirectionEnum);

	unsigned int squareMidpointValue(HeightMap * hmBeingGenerated, Square * areaOfInterest);

	void diamondStep(HeightMap * hmBeingGenerated, Square * areaOfParentSquare, unsigned int range);

	void squareStep(HeightMap * hmBeingGenerated, Square * areaOfSquare, unsigned int range);

	DirectX::XMINT2 midPoint(Diamond * d);
};


#endif