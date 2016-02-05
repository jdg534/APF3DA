#include "MathFuncs.h"

float MathFuncs::lerp(float a, float b, float t)
{
	return (1.0f - t) * a + b * t;
}

float MathFuncs::biLerp(float a0, float a1,
	float b0, float b1,
	float tx, float ty)
{
	float a = lerp(a0, a1, tx);
	float b = lerp(b0, b1, tx);
	return lerp(a, b, ty);
}