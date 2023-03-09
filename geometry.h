#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>
#include<iostream>
#include <vector>
#include <cassert>
using namespace std;

class Matrix;

template<class t> struct Vec2 {//Vec2�ṹ��
	union {
		struct { t u, v; };
		struct { t x, y; };
		t raw[2];//�������������ͬ������ʽ
	};
	Vec2() :u(0), v(0) {}
	Vec2(t _u, t _v) :u(_u), v(_v) {}
	Vec2<t>(const Vec2<t>& v) : x(t()), y(t()) { *this = v; }
	template <class u> Vec2<t>(const Vec2<u>& v);
	Vec2<t>& operator =(const Vec2<t>& v) {
		if (this != &v) {
			x = v.x;
			y = v.y;
		}
		return *this;
	}
	//���������
	Vec2<t> operator +(const Vec2<t>& V)const { return Vec2<t>(u + V.u, v + V.v); }
	Vec2<t> operator -(const Vec2<t>& V)const { return Vec2<t>(u - V.u, v - V.v); }
	Vec2<t> operator *(float f)          const { return Vec2<t>(u * f, v * f); }
	t operator[](const int i) {
		return raw[i];
	}
	template<class> friend std::ostream& operator<<(std::ostream& s, Vec2<t>& v);//ȫ�ֺ���������Ϊ��Ԫ������struct�еĳ�Ա����
	//������Ϊ�����������ȡ����������������� cin �� cout ���󣬶����������Զ�������ṹ������������ʹ�ó�Ա������ʽ��������Щ����������޷������������������Ƕ������Ҫ��
};

template<class t> struct Vec3 {
	union {
		struct { t x, y, z; };
		struct { t ivert, iuv, inorm; };
		t raw[3];
	};
	Vec3() : x(0), y(0), z(0) {}
	Vec3(t _x, t _y, t _z) : x(_x), y(_y), z(_z) {}
	Vec3<t>(const Vec3<t>& v) : x(t()), y(t()), z(t()) { *this = v; }
	Vec3<t>(Matrix m);
	template <class u> Vec3<t>(const Vec3<u>& v);
	Vec3<t>& operator =(const Vec3<t>& v) {
		if (this != &v) {
			x = v.x;
			y = v.y;
			z = v.z;
		}
		return *this;
	}
	Vec3<t> operator ^(const Vec3<t>& v) const { return Vec3<t>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }//���
	Vec3<t> operator +(const Vec3<t>& v) const { return Vec3<t>(x + v.x, y + v.y, z + v.z); }
	Vec3<t> operator -(const Vec3<t>& v) const { return Vec3<t>(x - v.x, y - v.y, z - v.z); }
	Vec3<t> operator *(float f)          const { return Vec3<t>(x * f, y * f, z * f); }//������
	t       operator *(const Vec3<t>& v) const { return x * v.x + y * v.y + z * v.z; }//���
	float norm() const { return std::sqrt(x * x + y * y + z * z); }//ģ��
	Vec3<t>& normalize(t l = 1) { *this = (*this) * (l / norm()); return *this; }//��һ��
	inline t operator[](const int i) {
		return raw[i];
	}
	template <class > friend std::ostream& operator<<(std::ostream& s, Vec3<t>& v);
};

template<class t> struct Vec4 {
	union {
		struct { t x, y, z, w; };
		t raw[4];
	};
	Vec4() : x(0), y(0), z(0), w(0) {}
	Vec4(t _x, t _y, t _z, t _w) : x(_x), y(_y), z(_z), w(_w) {}
	Vec4<t>(const Vec4<t>& v) : x(t()), y(t()), z(t()), w(t()) { *this = v; }
	Vec4<t>(Matrix m);
	Vec4<t>& operator =(const Vec4<t>& v) {
		if (this != &v) {
			x = v.x;
			y = v.y;
			z = v.z;
			w = v.w;
		}
		return *this;
	}
	inline t operator[](const int i) {
		return raw[i];
	}
};

typedef Vec2<float> Vec2f;
typedef Vec2<int>   Vec2i;
typedef Vec3<float> Vec3f;
typedef Vec3<int>   Vec3i;
typedef Vec4<float> Vec4f;
typedef Vec4<int>   Vec4i;

template <> template <> Vec3<int>::Vec3(const Vec3<float>& v);
template <> template <> Vec3<float>::Vec3(const Vec3<int>& v);

//�������
template <class t> std::ostream& operator<<(std::ostream& s, Vec2<t>& v) {
	s << "(" << v.x << "," << v.y << ")\n";
}

template <class t> std::ostream& operator<<(std::ostream& s, Vec3<t>& v) {
	s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
	return s;
}


const int DEFAULT_ALLOC = 4;

class Matrix {
	vector<vector<float>>m;
	int rows, cols;
public:
	Matrix(int r = DEFAULT_ALLOC, int c = DEFAULT_ALLOC);
	Matrix(Vec3f v);
	inline int nrows();
	inline int ncols();

	static Matrix identity(int dimensions = DEFAULT_ALLOC);
	vector<float>& operator[](const int i);
	Matrix operator*(const Matrix& a);
	Matrix transpose();
	Matrix inverse();

	friend ostream& operator<<(ostream& s, Matrix& m);
};


#endif //__GEOMETRY_H__