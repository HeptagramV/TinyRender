#ifndef __IMAGE_H__
#define __IMAGE_H__

#include<fstream>

#pragma pack(push,1)
struct TGA_Header {//为TGA文件设计的文件头结构体
	char idlength;//图像信息字段长度
	char colormaptype;//颜色表类型，0表示没有，1表示有
	char datatypecode;//图像类型编码，0-没有图像数据 1 - 未压缩，颜色表映射图像 2 - 未压缩，真彩图像 3 - 未压缩，黑白图像 9-行程编码，颜色表映射图像 10 - 行程编码，真彩图像 11 - 行程编码，黑白图像
	short colormaporigin;
	short colormaplength;
	char colormapdepth;
	short x_origin;//图像左下角水平坐标
	short y_origin;//图像左下角垂直坐标
	short width;//图像宽度
	short height;//图像高度
	char  bitsperpixel;//像素深度(每个像素占用的位数)
	char  imagedescriptor;//图像描述符，	0-3位，规定了每个像素属性位的数量。4-5位，这些位用于表示像素数据从文件发送到屏幕的顺序，位4表示从左到右，位5表示从上到下。
};
#pragma pack(pop)

struct TGAColor {
	union {
		struct {
			unsigned char b, g, r, a;
		};
		unsigned char raw[4];
		unsigned int val;
	};
	int bytespp;

	TGAColor() : val(0), bytespp(1) {
	}

	TGAColor(unsigned char R, unsigned char G, unsigned char B, unsigned char A) : b(B), g(G), r(R), a(A), bytespp(4) {
	}

	TGAColor(int v, int bpp) : val(v), bytespp(bpp) {
	}

	TGAColor(const TGAColor& c) : val(c.val), bytespp(c.bytespp) {
	}

	TGAColor(const unsigned char* p, int bpp) : val(0), bytespp(bpp) {
		for (int i = 0; i < bpp; i++) {
			raw[i] = p[i];
		}
	}
	inline TGAColor operator *(float t)const { return TGAColor(r * t, g * t, b * t, a * t); }
	inline TGAColor operator +(const TGAColor& c)const { return TGAColor(r + c.r, g + c.g, b + c.b, a + c.a); }
	inline TGAColor operator -(const TGAColor& c)const { return TGAColor(r - c.r, g - c.g, b - c.b, a - c.a); }

	TGAColor& operator =(const TGAColor& c) {
		if (this != &c) {
			bytespp = c.bytespp;
			val = c.val;
		}
		return *this;
	}
};

class TGAImage {
protected:
	unsigned char* data;
	int width;
	int height;
	int bytespp;//字节数

	bool   load_rle_data(std::ifstream& in);
	bool unload_rle_data(std::ofstream& out);
public:
	enum Format {
		GRAYSCALE = 1, RGB = 3, RGBA = 4//枚举序号对应格式的大小
	};

	TGAImage();
	TGAImage(int w, int h, int bpp);
	TGAImage(const TGAImage& img);
	bool read_tga_file(const char* filename);
	bool write_tga_file(const char* filename, bool rle = true);
	bool flip_horizontally();
	bool flip_vertically();
	bool scale(int w, int h);
	TGAColor get(int x, int y);
	bool set(int x, int y, TGAColor c);
	~TGAImage();
	TGAImage& operator =(const TGAImage& img);
	int get_width();
	int get_height();
	int get_bytespp();
	unsigned char* buffer();
	void clear();
};

#endif //__IMAGE_H__