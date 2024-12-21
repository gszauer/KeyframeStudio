#include "ImMath.h"
#include <math.h>

#define ImVec2_EPSILON 0.0001

ImMat3::ImMat3(const ImVec2& position, float rotation, const ImVec2& scale) {
	float rotCos = cosf(rotation);
	float rotSin = sinf(rotation);

	ImMat3 scaleMatrix = ImMat3(
		scale.x, 0, 0,
		0, scale.y, 0,
		0, 0, 1
	);
	ImMat3 rotateMatrix = ImMat3(
		rotCos, -rotSin, 0,
		rotSin, rotCos, 0,
		0, 0, 1
	);
	ImMat3 translateMatrix = ImMat3(
		1, 0, 0,
		0, 1, 0,
		position.x, position.y, 1
	);

	*this = translateMatrix * rotateMatrix * scaleMatrix;
}

ImVec2 operator-(const ImVec2& l, const ImVec2& r) {
	return ImVec2(l.x - r.x, l.y - r.y);
}

ImVec2 operator+(const ImVec2& l, const ImVec2& r) {
	return ImVec2(l.x + r.x, l.y + r.y);
}

ImVec4 operator+(const ImVec4& l, const ImVec4& r) {
	return ImVec4(l.x + r.x, l.y + r.y, l.z + r.z, l.w + r.w);
}

ImVec4 operator-(const ImVec4& l, const ImVec4& r) {
	return ImVec4(l.x - r.x, l.y - r.y, l.z - r.z, l.w - r.w);
}

ImVec4 operator*(const ImVec4& l, const ImVec4& r) {
	return ImVec4(l.x * r.x, l.y * r.y, l.z * r.z, l.w * r.w);
}

ImVec2 operator*(const ImVec2& l, const ImVec2& r) {
	return ImVec2(l.x * r.x, l.y * r.y);
}

ImVec2 operator/(const ImVec2& l, float r) {
	return ImVec2(l.x / r, l.y / r);
}

ImVec2 operator/(const ImVec2& l, const ImVec2& r) {
	return ImVec2(l.x / r.x, l.y / r.y);
}

ImVec2 operator*(const ImVec2& l, float f) {
	return ImVec2(l.x * f, l.y * f);
}

ImVec4 operator*(const ImVec4& l, float f) {
	return ImVec4(l.x * f, l.y * f, l.z * f, l.w * f);
}


ImVec2 rotate(const ImVec2& _v, float angle) {
	ImVec2 v = _v;
	float s = sinf(angle);
	float c = cosf(angle);
	float x = v.x;
	float y = v.y;
	v.x = x * c - y * s;
	v.y = x * s + y * c;
	return v;
}

ImVec2 perp(const ImVec2& v) {
	return ImVec2(v.y, -v.x);
}

float angle(const ImVec2& l, const ImVec2& r) {
	float sqMagL = l.x * l.x + l.y * l.y;
	float sqMagR = r.x * r.x + r.y * r.y;
	float sqMagD = (r.x - l.x) * (r.x - l.x) + (r.y - l.y) * (r.y - l.y);

	if (sqMagL < ImVec2_EPSILON || sqMagR < ImVec2_EPSILON || sqMagD < ImVec2_EPSILON) {
		return 0.0f;
	}

	float dot = l.x * r.x + l.y * r.y;
	float len = sqrtf(sqMagL) * sqrtf(sqMagR);

	if (len < ImVec2_EPSILON) {
		return 0.0f;
	}

	float arg = dot / len;
	float result = acosf(arg);
	return result;
}

ImVec2 minim(const ImVec2& p0, const ImVec2& p1) {
	return ImVec2(
		(p0.x < p1.x) ? p0.x : p1.x,
		(p0.y < p1.y) ? p0.y : p1.y
	);
}
ImVec2 minim(const ImVec2& p0, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3) {
	ImVec2 _01(
		(p0.x < p1.x) ? p0.x : p1.x,
		(p0.y < p1.y) ? p0.y : p1.y
	);

	ImVec2 _23(
		(p2.x < p3.x) ? p2.x : p3.x,
		(p2.y < p3.y) ? p2.y : p3.y
	);

	return ImVec2(
		(_01.x < _23.x) ? _01.x : _23.x,
		(_01.y < _23.y) ? _01.y : _23.y
	);
}
ImVec2 maxim(const ImVec2& p0, const ImVec2& p1) {
	return ImVec2(
		(p0.x > p1.x) ? p0.x : p1.x,
		(p0.y > p1.y) ? p0.y : p1.y
	);
}
ImVec2 maxim(const ImVec2& p0, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3) {
	ImVec2 _01(
		(p0.x > p1.x) ? p0.x : p1.x,
		(p0.y > p1.y) ? p0.y : p1.y
	);

	ImVec2 _23(
		(p2.x > p3.x) ? p2.x : p3.x,
		(p2.y > p3.y) ? p2.y : p3.y
	);

	return ImVec2(
		(_01.x > _23.x) ? _01.x : _23.x,
		(_01.y > _23.y) ? _01.y : _23.y
	);
}

void normalize(ImVec2& v) {
	float lenSq = v.x * v.x + v.y * v.y;
	if (lenSq < ImVec2_EPSILON) {
		return;
	}
	float invLen = 1.0f / sqrtf(lenSq);

	v.x *= invLen;
	v.y *= invLen;
}

ImVec2 normalized(const ImVec2& v) {
	float lenSq = v.x * v.x + v.y * v.y;
	if (lenSq < ImVec2_EPSILON) {
		return v;
	}
	float sqrtfLen = sqrtf(lenSq);

	return ImVec2(
		v.x / sqrtfLen,
		v.y / sqrtfLen
	);
}

float dot(const ImVec2& l, const ImVec2& r) {
	return l.x * r.x + l.y * r.y;
}

float lenSq(const ImVec2& v) {
	return v.x * v.x + v.y * v.y;
}

float len(const ImVec2& v) {
	float lenSq = v.x * v.x + v.y * v.y;
	if (lenSq < ImVec2_EPSILON) {
		return 0.0f;
	}
	return sqrtf(lenSq);
}

ImVec2 abs(const ImVec2& v) {
	ImVec2 result = v;
	if (v.x < 0.0f) {
		result.x = -v.x;
	}
	if (v.y < 0.0f) {
		result.y = -v.y;
	}
	return result;
}

#define M3D(aRow, bCol) \
    a.v[0 * 3 + aRow] * b.v[bCol * 3 + 0] + \
    a.v[1 * 3 + aRow] * b.v[bCol * 3 + 1] + \
    a.v[2 * 3 + aRow] * b.v[bCol * 3 + 2] 

ImMat3 operator*(const ImMat3& a, const ImMat3& b) {
	return ImMat3(
		M3D(0, 0), M3D(1, 0), M3D(2, 0),// Column 0
		M3D(0, 1), M3D(1, 1), M3D(2, 1),// Column 1
		M3D(0, 2), M3D(1, 2), M3D(2, 2)// Column 2
	);
}

ImMat3 operator+(const ImMat3& l, const ImMat3& r) {
	return ImMat3(
		l.xx + r.xx, l.xy + r.xy, l.xz + r.xz,
		l.yx + r.yx, l.yy + r.yy, l.yz + r.yz,
		l.zx + r.zx, l.zy + r.zy, l.zz + r.zz
	);
}

#define M3V3(mRow, x, y, z) \
    x * l.v[0 * 3 + mRow] + \
    y * l.v[1 * 3 + mRow] + \
    z * l.v[2 * 3 + mRow] 

ImVec3 operator*(const ImMat3& l, const ImVec3& r) {
	return ImVec3(
		M3V3(0, r.x, r.y, r.z),
		M3V3(1, r.x, r.y, r.z),
		M3V3(2, r.x, r.y, r.z)
	);
}

ImMat3 operator*(const ImMat3& l, float f) {
	ImMat3 result = l;
	result.xx *= f;
	result.xy *= f;
	result.xz *= f;
	result.yx *= f;
	result.yy *= f;
	result.yz *= f;
	result.zx *= f;
	result.zy *= f;
	result.zz *= f;
	return result;
}

bool operator==(const ImVec2& l, const ImVec2& r) {
	float delta = l.x - r.x;
	if (delta > 0.00001f || delta < -0.00001f) {
		return false;
	}
	delta = l.y - r.y;
	if (delta > 0.00001f || delta < -0.00001f) {
		return false;
	}
	return true;
}

bool operator==(const ImMat3& l, const ImMat3& r) {
	for (int i = 0; i < 9; ++i) {
		float delta = r.v[i] - l.v[i];
		if (delta < 0.00001f && delta > -0.00001f) {
			// Valid
		}
		else {
			return false;
		}
	}
	return true;
}

bool operator!=(const ImMat3& l, const ImMat3& r) {
	return !(l == r);
}

void Clamp01(ImVec2& v) {
	if (v.x < 0.0f) {
		v.x = 0.0f;
	}
	if (v.x > 1.0f) {
		v.x = 1.0f;
	}
	if (v.y < 0.0f) {
		v.y = 0.0f;
	}
	if (v.y > 1.0f) {
		v.y = 1.0f;
	}
}

ImMat3 RotationMatrix(float radians) {
	float rotCos = cosf(radians);
	float rotSin = sinf(radians);
	ImMat3 rotateMatrix = ImMat3(
		rotCos, -rotSin, 0,
		rotSin, rotCos, 0,
		0, 0, 1
	);
	return rotateMatrix;
}

#define PointInTriangleSign(p1, p2, p3) ((p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y))

bool TriangleContains(const ImVec2& v1, const ImVec2& v2, const ImVec2& v3, const ImVec2& pt) {
	float d1 = PointInTriangleSign(pt, v1, v2);
	float d2 = PointInTriangleSign(pt, v2, v3);
	float d3 = PointInTriangleSign(pt, v3, v1);

	bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

	return !(has_neg && has_pos);
}

void normalize(ImMat3& mat) {
	ImVec2 xAxis = normalized(ImVec2(mat.xx, mat.xy));
	ImVec2 yAxis = normalized(ImVec2(mat.yx, mat.yy));

	mat.xx = xAxis.x;
	mat.xy = xAxis.y;
	mat.yx = yAxis.x;
	mat.yy = yAxis.y;
}

bool CircleContains(const ImVec2& position, float radius, const ImVec2& point) {
	float lsq = lenSq(position - point);
	return lsq <= radius * radius;
}

bool RectangleContains(const ImVec2& v1, const ImVec2& v2, const ImVec2& v3, const ImVec2& v4, const ImVec2& pt) {
	float d1 = PointInTriangleSign(pt, v1, v2);
	float d2 = PointInTriangleSign(pt, v2, v3);
	float d3 = PointInTriangleSign(pt, v3, v4);
	float d4 = PointInTriangleSign(pt, v4, v1);

	bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0) || (d4 < 0);
	bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0) || (d4 > 0);

	return !(has_neg && has_pos);
}

ImVec2 project(const ImVec2& vect, const ImVec2& direction) {
	float _dot = dot(vect, direction);
	float magSq = dot(direction, direction);
	if (magSq < 0.00001) {
		magSq = 0.0;
	}
	else {
		magSq = 1.0f / magSq;
	}
	return direction * (_dot * magSq);
}

/*float Determinant(float xx, float xy, float yx, float yy) {
	return xx * yy - yx * xy;
}*/

#define DET2x2(xx, xy, yx, yy) (xx * yy - yx * xy)

ImMat3 minor(const ImMat3& m) {
	ImMat3 result;

	result.xx = DET2x2(m.yy, m.yz, m.zy, m.zz); // 0, 0
	result.xy = DET2x2(m.yx, m.yz, m.zx, m.zz); // 0, 1
	result.xz = DET2x2(m.yx, m.yy, m.zx, m.zy); // 0, 2

	result.yx = DET2x2(m.xy, m.xz, m.zy, m.zz); // 1, 0
	result.yy = DET2x2(m.xx, m.xz, m.zx, m.zz); // 1, 1
	result.yz = DET2x2(m.xx, m.xy, m.zx, m.zy); // 1, 2

	result.zx = DET2x2(m.xy, m.xz, m.yy, m.yz); // 2, 0
	result.zy = DET2x2(m.xx, m.xz, m.yx, m.yz); // 2, 1
	result.zz = DET2x2(m.xx, m.xy, m.yx, m.yy); // 2, 2

	return result;
}

ImMat3 cofactor(const ImMat3& m) {
	ImMat3 min = minor(m);

	min.v[1] *= -1.0f;
	min.v[3] *= -1.0f;
	min.v[5] *= -1.0f;
	min.v[7] *= -1.0f;

	return min;
}

float determinant(const ImMat3& m) {
	ImMat3 c = cofactor(m);

	float _11 = m.xx * c.xx;
	float _12 = m.yx * c.yx;
	float _13 = m.zx * c.zx;

	return _11 + _12 + _13;
}

ImMat3 transposed(const ImMat3& m) {
	return ImMat3(m.xx, m.yx, m.zx, m.xy, m.yy, m.zy, m.xz, m.yz, m.zz);
}

ImMat3 adjugate(const ImMat3& m) {
	return transposed(cofactor(m));
}

ImMat3 invert(const ImMat3& m) {
	float det = determinant(m);
	if (det < 0.00001f && det > -0.00001f) {
		return ImMat3();
	}
	return adjugate(m) * (1.0f / det);
}