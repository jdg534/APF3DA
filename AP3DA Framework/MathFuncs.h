#ifndef _MATH_FUNCS_H_
#define _MATH_FUNCS_H_

namespace MathFuncs
{
	float lerp(float a, float b, float t);
	float biLerp(float a0, float a1,
		float b0, float b1,
		float tx, float ty);

};


#endif