#include <vector>
#include <cassert>
#include <cmath>
#include <iostream>
#include "geometry.h"
using namespace std;

Matrix::Matrix(int r, int c) : m(vector<vector<float> >(r, vector<float>(c, 0.f))), rows(r), cols(c) { }//���캯��
template <> Vec3<float>::Vec3(Matrix m) : x(m[0][0] / m[3][0]), y(m[1][0] / m[3][0]), z(m[2][0] / m[3][0]) {}
template <> Vec4<float>::Vec4(Matrix m) : x(m[0][0]), y(m[1][0]), z(m[2][0]), w(m[3][0]) {}
template <> template <> Vec3<int>::Vec3(const Vec3<float>& v) : x(int(v.x + .5)), y(int(v.y + .5)), z(int(v.z + .5)) {}
template <> template <> Vec3<float>::Vec3(const Vec3<int>& v) : x(v.x), y(v.y), z(v.z) {}
template <> template <> Vec2<int>::Vec2(const Vec2<float>& v) : x(int(v.x + .5)), y(int(v.y + .5)) {}
template <> template <> Vec2<float>::Vec2(const Vec2<int>& v) : x(v.x), y(v.y) {}

int Matrix::nrows() {
    return rows;
}

int Matrix::ncols() {
    return cols;
}

Matrix::Matrix(Vec3f v) : m(vector<vector<float> >(4, vector<float>(1, 1.f))), rows(4), cols(1) {
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
}

Matrix Matrix::identity(int dimensions) {//��ȡһ��dimensionsά�ĵ�λ����
    Matrix E(dimensions, dimensions);
    for (int i = 0; i < dimensions; i++) {
        for (int j = 0; j < dimensions; j++) {
            E[i][j] = (i == j ? 1.f : 0.f);
        }
    }
    return E;
}

vector<float>& Matrix::operator[](const int i) {
    assert(i >= 0 && i < rows);
    return m[i];
}

Matrix Matrix::operator*(const Matrix& a) {//����˷�
    assert(cols == a.rows);
    Matrix result(rows, a.cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < a.cols; j++) {
            result.m[i][j] = 0.f;
            for (int k = 0; k < cols; k++) {
                result.m[i][j] += m[i][k] * a.m[k][j];
            }
        }
    }
    return result;
}

Matrix Matrix::transpose() {//����ת��
    Matrix result(cols, rows);
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            result[j][i] = m[i][j];
    return result;
}

Matrix Matrix::inverse() {//���������
    assert(rows == cols);
    Matrix result(rows, cols * 2);//�����������
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            result[i][j] = m[i][j];//��ԭ�������������������
    for (int i = 0; i < rows; i++)
        result[i][i + cols] = 1;//����������Ҳ�����Ϊ��λ����
    for (int i = 0; i < rows - 1; i++) {// ��ÿһ�н���������Ԫ
        for (int j = result.cols - 1; j >= 0; j--)
            result[i][j] /= result[i][i];// �����г��ԶԽ����ϵ�Ԫ�أ�ʹ�öԽ����ϵ�Ԫ�ر��1
        for (int k = i + 1; k < rows; k++) {
            float coeff = result[k][i];// ����������i��Ԫ����˺����i����������ϵ��coeff
            for (int j = 0; j < result.cols; j++) {
                result[k][j] -= result[i][j] * coeff;// ����i�г���coeff��Ӹ��м�ȥ��ʹ�ø��г��˶Խ�����Ϊ1������Ԫ�ض�Ϊ0
            }
        }
    }
    for (int j = result.cols - 1; j >= rows - 1; j--)//// �����һ�г��ԶԽ����ϵ�Ԫ�أ�ʹ�öԽ����ϵ�Ԫ�ر��1
        result[rows - 1][j] /= result[rows - 1][rows - 1];
    for (int i = rows - 1; i > 0; i--) {
        for (int k = i - 1; k >= 0; k--) {// �Ը������ϵ�ÿһ�н������²�����
            float coeff = result[k][i];// ����������i��Ԫ����˺����i����������ϵ��coeff
            for (int j = 0; j < result.cols; j++) { // ����i�г���coeff��Ӹ��м�ȥ��ʹ�ø��г��˶Խ�����Ϊ1������Ԫ�ض�Ϊ0
                result[k][j] -= result[i][j] * coeff;
            }
        }
    }
    // ����һ���µľ���truncate��������������Ұ벿�ָ��Ƶ�������
    Matrix truncate(rows, cols);
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            truncate[i][j] = result[i][j + cols];
    return truncate;// ���������
}

ostream& operator<<(ostream& s, Matrix& m) {//�������
    for (int i = 0; i < m.nrows(); i++) {
        for (int j = 0; j < m.ncols(); j++) {
            s << m[i][j];
            if (j < m.ncols() - 1) s << "\t";
        }
        s << "\n";
    }
    return s;
}