#include <vector>
#include <cassert>
#include <cmath>
#include <iostream>
#include "geometry.h"
using namespace std;

Matrix::Matrix(int r, int c) : m(vector<vector<float> >(r, vector<float>(c, 0.f))), rows(r), cols(c) { }//构造函数
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

Matrix Matrix::identity(int dimensions) {//获取一个dimensions维的单位矩阵
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

Matrix Matrix::operator*(const Matrix& a) {//矩阵乘法
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

Matrix Matrix::transpose() {//矩阵转置
    Matrix result(cols, rows);
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            result[j][i] = m[i][j];
    return result;
}

Matrix Matrix::inverse() {//计算逆矩阵
    assert(rows == cols);
    Matrix result(rows, cols * 2);//创建增广矩阵
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            result[i][j] = m[i][j];//将原矩阵置于增广矩阵的左侧
    for (int i = 0; i < rows; i++)
        result[i][i + cols] = 1;//将增广矩阵右侧设置为单位矩阵
    for (int i = 0; i < rows - 1; i++) {// 对每一行进行向下消元
        for (int j = result.cols - 1; j >= 0; j--)
            result[i][j] /= result[i][i];// 将该行除以对角线上的元素，使得对角线上的元素变成1
        for (int k = i + 1; k < rows; k++) {
            float coeff = result[k][i];// 计算该行与第i列元素相乘后与第i行相减所需的系数coeff
            for (int j = 0; j < result.cols; j++) {
                result[k][j] -= result[i][j] * coeff;// 将第i行乘以coeff后从该行减去，使得该列除了对角线上为1外其他元素都为0
            }
        }
    }
    for (int j = result.cols - 1; j >= rows - 1; j--)//// 将最后一行除以对角线上的元素，使得对角线上的元素变成1
        result[rows - 1][j] /= result[rows - 1][rows - 1];
    for (int i = rows - 1; i > 0; i--) {
        for (int k = i - 1; k >= 0; k--) {// 对该行以上的每一行进行以下操作：
            float coeff = result[k][i];// 计算该行与第i列元素相乘后与第i行相减所需的系数coeff
            for (int j = 0; j < result.cols; j++) { // 将第i行乘以coeff后从该行减去，使得该列除了对角线上为1外其他元素都为0
                result[k][j] -= result[i][j] * coeff;
            }
        }
    }
    // 创建一个新的矩阵truncate，并将增广矩阵右半部分复制到它里面
    Matrix truncate(rows, cols);
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            truncate[i][j] = result[i][j + cols];
    return truncate;// 返回逆矩阵
}

ostream& operator<<(ostream& s, Matrix& m) {//重载输出
    for (int i = 0; i < m.nrows(); i++) {
        for (int j = 0; j < m.ncols(); j++) {
            s << m[i][j];
            if (j < m.ncols() - 1) s << "\t";
        }
        s << "\n";
    }
    return s;
}