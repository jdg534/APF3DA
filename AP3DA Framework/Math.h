#ifndef _MATH_H_
#define _MATH_H_

namespace Math
{
	float lerp(float a, float b, float t)
	{
		return (1.0f - t) * a + t * b;
	}

	float biLerp(float a0, float a1, float b0, float b1, float tx, float ty)
	{
		float ax = lerp(a0, a1, tx);
		float bx = lerp(b0, b1, tx);
		return lerp(ax, bx, ty);
	}
}

#endif