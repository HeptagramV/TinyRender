#include <vector>
#include <cmath>
#include <cstdlib>
#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Model* model = NULL;
const int width = 800;
const int height = 800;
vector<int> zbuffer(width* height, -std::numeric_limits<int>::max());
const int depth = 255;

Vec3f light_dir(0, -1, -1);
Vec3f eye(2, 1, 3);
Vec3f center(0, 0, 0);

Matrix m2v(Matrix m) {//透视齐次除法
    m[0][0] /= m[3][0], m[1][0] /= m[3][0], m[2][0] /= m[3][0];
    return m;
}

Matrix v2m(Vec3f v) {
    Matrix m(4, 1);
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.f;
    return m;
}
//把[-1, 1] * [-1, 1] * [-1, 1]的立方体映射到[x, x + w] * [y, y + h] * [0, d]的长方体
Matrix viewport(int x, int y, int w, int h) {//视口变换矩阵,x,y表示视口相对于窗口的左下角原点，w,h是视口的宽高
    Matrix m = Matrix::identity(4);
    m[0][3] = x + w / 2.f;
    m[1][3] = y + h / 2.f;
    m[2][3] = depth / 2.f;

    m[0][0] = w / 2.f;
    m[1][1] = h / 2.f;
    m[2][2] = depth / 2.f;
    return m;
}

//朝向矩阵，变换矩阵
//更改摄像机视角=更改物体位置和角度，操作为互逆矩阵
//摄像机变换是先旋转再平移，所以物体需要先平移后旋转，且都是逆矩阵
Matrix lookat(Vec3f eye, Vec3f center, Vec3f up) {
    //计算出z，根据z和up算出x，再算出y
    Vec3f z = (eye - center).normalize();
    Vec3f x = (up ^ z).normalize();
    Vec3f y = (z ^ x).normalize();
    Matrix rotation = Matrix::identity(4);
    Matrix translation = Matrix::identity(4);
    //***矩阵的第四列是用于平移的。因为观察位置从原点变为了center，所以需要将物体平移-center***
    for (int i = 0; i < 3; i++) {
        rotation[i][3] = -center[i];
    }
    //正交矩阵的逆 = 正交矩阵的转置
    //矩阵的第一行即是现在的x
    //矩阵的第二行即是现在的y
    //矩阵的第三行即是现在的z
    //***矩阵的三阶子矩阵是当前视线旋转矩阵的逆矩阵***
    for (int i = 0; i < 3; i++) {
        rotation[0][i] = x[i];
        rotation[1][i] = y[i];
        rotation[2][i] = z[i];
    }
    //这样乘法的效果是先平移物体，再旋转
    Matrix res = rotation * translation;
    return res;
}

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    for (int x = x0; x <= x1; x++) {
        float t = (x - x0) / (float)(x1 - x0);
        int y = y0 * (1. - t) + y1 * t;
        if (steep) {
            image.set(y, x, color);
        }
        else {
            image.set(x, y, color);
        }
    }
}

Vec3f barycentric(Vec3i* pts, Vec3f P) {//计算P相对于三角形的重心坐标
    Vec3f s[2];
    for (int i = 2; i--; ) {
        s[i].x = pts[2].raw[i] - pts[0].raw[i];
        s[i].y = pts[1].raw[i] - pts[0].raw[i];
        s[i].z = pts[0].raw[i] - P.raw[i];
    }
    Vec3f u = Vec3f(s[0].y * s[1].z - s[0].z * s[1].y, s[0].z * s[1].x - s[0].x * s[1].z, s[0].x * s[1].y - s[0].y * s[1].x);
    if (std::abs(u.z) < 1) return Vec3f(-1, 1, 1);//判断u.z是否接近于0（即小于1），如果是，则说明三角形退化为一条线或者一个点，此时返回一个负数坐标表示无效结果。
    return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

void triangle(Vec3i* pts, TGAImage& image, Vec2f* uv, float* intensity, float* distance) {
    Vec2i bboxmin(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
    Vec2i bboxmax(-std::numeric_limits<int>::max(), -std::numeric_limits<int>::max());
    Vec2i clamp(image.get_width() - 1, image.get_height() - 1);
    for (int i = 0; i < 3; i++) {
        bboxmin.x = std::max(0, std::min(bboxmin.x, pts[i].x));
        bboxmin.y = std::max(0, std::min(bboxmin.y, pts[i].y));
        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
    }//计算三角形包围盒的四个坐标
    Vec3i P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            Vec3f bc_screen = barycentric(pts, P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;//遍历包围盒的每个像素，通过重心坐标判断是否在三角形内，若不是则舍弃该点
            P.z = 0;
            Vec2f uvP(0, 0);
            float ityP = 0;
            float disP = 0;
            for (int i = 0; i < 3; i++) {
                P.z += pts[i].z * bc_screen.raw[i];//通过重心坐标的定义计算深度值
                uvP.x += uv[i].x * bc_screen.raw[i];
                uvP.y += uv[i].y * bc_screen.raw[i];
                ityP += intensity[i] * bc_screen.raw[i];
                ityP = std::min(1.f, std::abs(ityP) + 0.01f);
                disP += distance[i] * bc_screen.raw[i];
            }
            //cout << ityP << endl;
            //cout << P.z << endl;
            if (zbuffer[int(P.x + P.y * width)] < P.z) {
                zbuffer[int(P.x + P.y * width)] = P.z;
                TGAColor color = model->diffuse(uvP);
                float f = std::min(ityP * (20.f / std::pow(disP, 2.f)), 1.f);
                image.set(P.x, P.y, color * f);
            }
        }
    }
}

Vec3f world2screen(Vec3f v) {
    return Vec3f(int((v.x + 1.) * width / 2. + .5), int((v.y + 1.) * height / 2. + .5), v.z);
}

int main(int argc, char** argv) {
    if (2 == argc) {// 如果有两个参数，说明指定了模型文件的路径
        model = new Model(argv[1]);// 用第二个参数创建一个Model对象
    }
    else {//否则使用默认的模型文件路径
        model = new Model("obj/african_head.obj");
    }
    Matrix ModelView = lookat(eye, center, Vec3f(0, 1, 0));
    Matrix Projection = Matrix::identity(4);
    Matrix ViewPort = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
    Projection[3][2] = -1.f / (eye - center).norm();

    TGAImage image(width, height, TGAImage::RGB);
    for (int i = 0; i < model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        Vec3i screen_coords[3];//改了
        Vec3f world_coords[3];
        float intensity[3];
        float distance[3];
        for (int j = 0; j < 3; j++) {
            Vec3f v = model->vert(face[j]);
            Matrix m_v = ModelView * Matrix(v);
            screen_coords[j] = Vec3f(ViewPort * Projection * m_v);
            intensity[j] = std::max(-(model->norm(i, j) * light_dir.normalize()), 0.f);
            Vec3f w_v = Vec3f(m_v);
            distance[j] = std::pow((std::pow(w_v.x - eye.x, 2.0f) + std::pow(w_v.y - eye.y, 2.0f) + std::pow(w_v.z - eye.z, 2.0f)), 0.5f);
        }
        Vec2f uv[3];
        if (intensity > 0) {
            for (int j = 0; j < 3; j++) {
                uv[j] = model->uv(i, j);
            }
            triangle(screen_coords, image, uv, intensity, distance);
        }
    }
    image.flip_vertically();
    image.write_tga_file("output.tga");

    { // dump z-buffer (debugging purposes only)
        TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                zbimage.set(i, j, TGAColor(zbuffer[i + j * width], 1));
            }
        }
        zbimage.flip_vertically();
        zbimage.write_tga_file("zbuffer.tga");
    }

    delete model;
    return 0;
}