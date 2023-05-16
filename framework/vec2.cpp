#include "vec2.h"
#include "../platform/math.h"

vec2 operator-(const vec2& l, const vec2& r) {
	return vec2(l.x - r.x, l.y - r.y);
}

vec2 operator+(const vec2& l, const vec2& r) {
	return vec2(l.x + r.x, l.y + r.y);
}

vec2 operator*(const vec2& l, const vec2& r) {
	return vec2(l.x * r.x, l.y * r.y);
}

vec2 operator/(const vec2& l, float r) {
	return vec2(l.x / r, l.y / r);
}

vec2 operator/(const vec2& l, const vec2& r) {
	return vec2(l.x / r.x, l.y / r.y);
}

vec2 operator*(const vec2& l, float f) {
	return vec2(l.x * f, l.y * f);
}

vec2 rotate(const vec2& _v, float angle) {
	vec2 v = _v;
	float s = MathSin(angle);
	float c = MathCos(angle);
	float x = v.x;
	float y = v.y;
	v.x = x * c - y * s;
	v.y = x * s + y * c;
	return v;
}

float angle(const vec2& l, const vec2& r) {
	float sqMagL = l.x * l.x + l.y * l.y;
	float sqMagR = r.x * r.x + r.y * r.y;

	if (sqMagL < VEC2_EPSILON || sqMagR < VEC2_EPSILON) {
		return 0.0f;
	}

	float dot = l.x * r.x + l.y * r.y;
	float len = MathSqrt(sqMagL) * MathSqrt(sqMagR);
	return MathACos(dot / len);
}


void normalize(vec2& v) {
	float lenSq = v.x * v.x + v.y * v.y;
	if (lenSq < VEC2_EPSILON) {
		return;
	}
	float invLen = 1.0f / MathSqrt(lenSq);

	v.x *= invLen;
	v.y *= invLen;
}

vec2 normalized(const vec2& v) {
	float lenSq = v.x * v.x + v.y * v.y;
	if (lenSq < VEC2_EPSILON) {
		return v;
	}
	float invLen = 1.0f / MathSqrt(lenSq);

	return vec2(
		v.x * invLen,
		v.y * invLen
	);
}

float dot(const vec2& l, const vec2& r) {
	return l.x * r.x + l.y * r.y;
}

float lenSq(const vec2& v) {
	return v.x * v.x + v.y * v.y ;
}

float len(const vec2& v) {
	float lenSq = v.x * v.x + v.y * v.y;
	if (lenSq < VEC2_EPSILON) {
		return 0.0f;
	}
	return MathSqrt(lenSq);
}

vec2 abs(const vec2& v) {
	vec2 result = v;
	if (v.x < 0.0f) {
		result.x = -v.x;
	}
	if (v.y < 0.0f) {
		result.y = -v.y;
	}
	return result;
}