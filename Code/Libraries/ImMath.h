#pragma once

#include "imgui.h"
#include "imgui_internal.h"

struct ImMat3 {
	union {
		float v[9];
		struct {
			//            row 1     row 2     row 3    
			/* column 1 */float xx; float xy; float xz;
			/* column 2 */float yx; float yy; float yz;
			/* column 3 */float zx; float zy; float zz;
		};
		struct {
			float c0r0; float c0r1; float c0r2;
			float c1r0; float c1r1; float c1r2;
			float c2r0; float c2r1; float c2r2;
		};
		struct {
			float r0c0; float r1c0; float r2c0;
			float r0c1; float r1c1; float r2c1;
			float r0c2; float r1c2; float r2c2;
		};
	};

	inline ImMat3() :
		xx(1), xy(0), xz(0),
		yx(0), yy(1), yz(0),
		zx(0), zy(0), zz(1) {}

	inline ImMat3(float* fv) :
		xx(fv[0]), xy(fv[1]), xz(fv[2]),
		yx(fv[3]), yy(fv[4]), yz(fv[5]),
		zx(fv[6]), zy(fv[7]), zz(fv[8])  { }

	inline ImMat3(
		float _00, float _01, float _02,
		float _10, float _11, float _12,
		float _20, float _21, float _22  ) :
		xx(_00), xy(_01), xz(_02),
		yx(_10), yy(_11), yz(_12),
		zx(_20), zy(_21), zz(_22) { }

	ImMat3(const ImVec2& pos, float rot, const ImVec2& scl);
};


struct ImVec3 {
	union {
		struct {
			float x;
			float y;
			float z;
		};
		float xyz[3];
	};

	inline ImVec3() : x(0), y(0), z(0) { }
	inline ImVec3(const ImVec2& v, float f) : x(v.x), y(v.y), z(f) { }
	inline ImVec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) { }

	inline ImVec2 Vec2() { return ImVec2(x, y); }
};

ImVec2 operator-(const ImVec2& l, const ImVec2& r);
ImVec2 operator+(const ImVec2& l, const ImVec2& r);
ImVec2 operator*(const ImVec2& l, const ImVec2& r);
ImVec2 operator/(const ImVec2& l, const ImVec2& r);
ImVec2 operator/(const ImVec2& l, float f);
ImVec2 operator*(const ImVec2& l, float f);
ImVec4 operator*(const ImVec4& l, float f);

bool operator==(const ImVec2& l, const ImVec2& r);


ImVec4 operator+(const ImVec4& l, const ImVec4& r);
ImVec4 operator-(const ImVec4& l, const ImVec4& r);
ImVec4 operator*(const ImVec4& l, const ImVec4& r);

ImVec2 minim(const ImVec2& p0, const ImVec2& p1);
ImVec2 minim(const ImVec2& p0, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3);
ImVec2 maxim(const ImVec2& p0, const ImVec2& p1);
ImVec2 maxim(const ImVec2& p0, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3);

void normalize(ImVec2& v);
ImVec2 normalized(const ImVec2& v);
float dot(const ImVec2& l, const ImVec2& r);
float lenSq(const ImVec2& v);
float len(const ImVec2& v);
ImVec2 abs(const ImVec2& v);
float angle(const ImVec2& l, const ImVec2& r);
ImVec2 rotate(const ImVec2& v, float angle);
ImVec2 perp(const ImVec2& v);
ImVec2 project(const ImVec2& vect, const ImVec2& direction);

ImMat3 invert(const ImMat3& m);
ImMat3 operator*(const ImMat3& l, const ImMat3& r);
ImMat3 operator+(const ImMat3& l, const ImMat3& r);
ImVec3 operator*(const ImMat3& l, const ImVec3& r);
ImMat3 operator*(const ImMat3& l, float f);
bool operator==(const ImMat3& l, const ImMat3& r);
bool operator!=(const ImMat3& l, const ImMat3& r);

bool TriangleContains(const ImVec2& vert1, const ImVec2& vert2, const ImVec2& vert3, const ImVec2& point);
bool RectangleContains(const ImVec2& vert1, const ImVec2& vert2, const ImVec2& vert3, const ImVec2& vert4, const ImVec2& point);
bool CircleContains(const ImVec2& position, float radius, const ImVec2& point);

void normalize(ImMat3& mat);
ImMat3 RotationMatrix(float radians);
void Clamp01(ImVec2& v);