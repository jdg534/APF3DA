#ifndef _HEIGHT_MAP_GENERATOR_H_
#define _HEIGHT_MAP_GENERATOR_H_

#include "HeightMap.h"

// #include <d3d11.h>
#include <DirectXMath.h>

class HeightMapGenerator
{
public:

	HeightMap * generateHillCircle(int widthDepthVal, int iterations, int minRadius, int maxRadius, int maxRaiseHeight);

	HeightMap * generateFaultFormation(int widthDepthVal, int iterations);

private:

	int findLinePointOnColumn(DirectX::XMFLOAT2 leftPoint, DirectX::XMFLOAT2 rightPoint, int column, HeightMap * hm);
	
	int findLinePointOnRow(DirectX::XMFLOAT2 topPoint, DirectX::XMFLOAT2 bottomPoint, int row, HeightMap * hm);

};


#endif