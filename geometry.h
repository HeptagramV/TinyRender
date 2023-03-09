#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>
#include<iostream>
#include <vector>
#include <cassert>
using namespace std;

class Matrix;

template<class t> struct Vec2 {//Vec2结构体
	union {
		struct { t u, v; };
		struct { t x, y; };
		t raw[2];//用联合体给出不同命名方式
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
	//重载运算符
	Vec2<t> operator +(const Vec2<t>& V)const { return Vec2<t>(u + V.u, v + V.v); }
	Vec2<t> operator -(const Vec2<t>& V)const { return Vec2<t>(u - V.u, v - V.v); }
	Vec2<t> operator *(float f)          const { return Vec2<t>(u * f, v * f); }
	t operator[](const int i) {
		return raw[i];
	}
	template<class> friend std::ostream& operator<<(std::ostream& s, Vec2<t>& v);//全局函数，设置为友元来访问struct中的成员变量
	//这是因为流插入和流提取运算符的左侧操作数是 cin 或 cout 对象，而不是我们自定义的类或结构体对象。如果我们使用成员函数形式来重载这些运算符，就无法满足左侧操作数必须是对象本身的要求。
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
	Vec3<t> operator ^(const Vec3<t>& v) const { return Vec3<t>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }//叉乘
	Vec3<t> operator +(const Vec3<t>& v) const { return Vec3<t>(x + v.x, y + v.y, z + v.z); }
	Vec3<t> operator -(const Vec3<t>& v) const { return Vec3<t>(x - v.x, y - v.y, z - v.z); }
	Vec3<t> operator *(float f)          const { return Vec3<t>(x * f, y * f, z * f); }//向量乘
	t       operator *(const Vec3<t>& v) const { return x * v.x + y * v.y + z * v.z; }//点乘
	float norm() const { return std::sqrt(x * x + y * y + z * z); }//模长
	Vec3<t>& normalize(t l = 1) { *this = (*this) * (l / norm()); return *this; }//归一化
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

//重载输出
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