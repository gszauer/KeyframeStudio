#ifndef _H_MATH_
#define _H_MATH_

#ifdef __cplusplus
#define c_func extern "C"
#else
#define c_func 
#endif

#define MATH_EPSILON 0.0001f
#define MATH_DEG2RAD 0.0174533f
#define MATH_RAD2DEG 57.2958f

#define MathPI 3.14159265358979323846f
#define MATH_PI 3.14159265358979323846f

c_func float  MathRound(float x);
c_func float  MathCeil(float x);
c_func float  MathFloor(float x);
c_func float  MathSin(float x);
c_func float  MathCos(float x);
c_func float  MathACos(float x);
c_func float  MathTan(float x);
c_func float  MathSqrt(float x);
c_func float  MathExp(float x);
c_func float  MathLog(float x);
c_func float  MathPow(float x, float y);
c_func double MathLdexp(double x, int exp);
c_func float  MathRandom();
c_func float  MathAtan2(float y, float x);

inline float Math01(float v) {
	if (v < 0.0f) {
		return 0.0f;
	}
	if (v > 1.0f) {
		return 1.0f;
	}
	return v;
}

inline float MathRadToDeg(float rad) {
    return rad * 57.2958f;
}

inline float MathDegToRad(float deg) {
    return deg * 0.0174533f;
}

inline float MathTrunc(float d) { 
    if (d > 0) { // Floor
        return MathFloor(d);
    }
    // else { // Ceil
    return MathCeil(d);
}

inline float MathFmod(float x, float y) {
  return x - MathTrunc(x / y) * y;
}

inline float MathAbsF(float f) {
    if (f < 0.0f) {
        return -f;
    }
    return f;
}

inline int MathAbsI(int f) {
    if (f < 0) {
        return -f;
    }
    return f;
}

inline float MathMaxF(float a, float b) {
	if (b > a) {
		return b;
	}
	return a;
}

inline float MathMaxI(int a, int b) {
	if (b > a) {
		return b;
	}
	return a;
}

inline float MathMinF(float a, float b) {
	if (b < a) {
		return b;
	}
	return a;
}

inline float MathMinI(int a, int b) {
	if (b < a) {
		return b;
	}
	return a;
}

inline float MathLerpF(float a, float b, float t) {
	return a + (b - a) * t;
}

inline char MathIsSpace(unsigned char c) {
	if (c == (unsigned char)(' ') ||
		c == (unsigned char)('\f') ||
		c == (unsigned char)('\n') ||
		c == (unsigned char)('\r') ||
		c == (unsigned char)('\t') ||
		c == (unsigned char)('\v'))
		return 1;
	return 0;
}

inline long MathAToI(const char* s) {
	long rv = 0;
	char sign = 0;

	/* skip till we find either a digit or '+' or '-' */
	while (*s) {
		if (*s <= '9' && *s >= '0')
			break;
		if (*s == '-' || *s == '+')
			break;
		s++;
	}

	sign = (*s == '-');
	if (*s == '-' || *s == '+') s++;

	while (*s && *s >= '0' && *s <= '9') {
		rv = (rv * 10) + (*s - '0');
		s++;
	}

	return (sign ? -rv : rv);
}

// atof: https://github.com/darconeous/sdcc/blob/master/device/lib/pic16/libc/stdlib/atof.c
inline float MathAToF(const char* s) {
	float value, fraction;
	char iexp;
	char sign;

	//Skip leading blanks
	while (MathIsSpace(*s)) s++;

	//Get the sign
	if (*s == '-')
	{
		sign = 1;
		s++;
	}
	else
	{
		sign = 0;
		if (*s == '+') s++;
	}

	//Get the integer part
	for (value = 0.0; (unsigned char)(*s) >= '0' && (unsigned char)(*s) <= '9'; s++)
	{
		value = 10.0 * value + (*s - '0');
	}

	//Get the fraction
	if (*s == '.')
	{
		s++;
		for (fraction = 0.1; (unsigned char)(*s) >= '0' && (unsigned char)(*s) <= '9'; s++)
		{
			value += (*s - '0') * fraction;
			fraction *= 0.1;
		}
	}

	//Finally, the exponent (not very efficient, but enough for now*/
	if (*s == 'E' || *s == 'e')
	{
		s++;
		iexp = (char)MathAToI(s);
		{
			while (iexp != 0)
			{
				if (iexp < 0)
				{
					value *= 0.1;
					iexp++;
				}
				else
				{
					value *= 10.0;
					iexp--;
				}
			}
		}
	}

	if (sign) value *= -1.0;
	return (value);
}

#endif