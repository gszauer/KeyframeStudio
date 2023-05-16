#ifndef _H_VEC2_
#define _H_VEC2_

#define VEC2_EPSILON 0.000001f

template<typename T>
struct TVec2 {
	union {
		struct {
			T x;
			T y;
		};
		T v[2];
	};
	inline TVec2() : x(T(0)), y(T(0)) { }
	inline TVec2(T _x, T _y) :
		x(_x), y(_y) { }
	inline TVec2(T* fv) {
		if (fv != 0) {
			x = fv[0];
			y = fv[1];
		}
		else {
			x = y = 0.0f;
		}
	}
};

typedef TVec2<float> vec2;
typedef TVec2<int> ivec2;
typedef TVec2<unsigned int> uivec2;

vec2 operator-(const vec2& l, const vec2& r);
vec2 operator+(const vec2& l, const vec2& r);
vec2 operator*(const vec2& l, const vec2& r);
vec2 operator/(const vec2& l, const vec2& r);
vec2 operator/(const vec2& l, float f);
vec2 operator*(const vec2& l, float f);

void normalize(vec2& v);
vec2 normalized(const vec2& v);
float dot(const vec2& l, const vec2& r);
float lenSq(const vec2& v);
float len(const vec2& v);
vec2 abs(const vec2& v);
float angle(const vec2& l, const vec2& r);
vec2 rotate(const vec2& v, float angle);

inline vec2 perp(const vec2& v) {
	return vec2(v.y, -v.x);
}

#endif