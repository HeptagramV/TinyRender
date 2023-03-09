#ifndef __IMAGE_H__
#define __IMAGE_H__

#include<fstream>

#pragma pack(push,1)
struct TGA_Header {//ΪTGA�ļ���Ƶ��ļ�ͷ�ṹ��
	char idlength;//ͼ����Ϣ�ֶγ���
	char colormaptype;//��ɫ�����ͣ�0��ʾû�У�1��ʾ��
	char datatypecode;//ͼ�����ͱ��룬0-û��ͼ������ 1 - δѹ������ɫ��ӳ��ͼ�� 2 - δѹ�������ͼ�� 3 - δѹ�����ڰ�ͼ�� 9-�г̱��룬��ɫ��ӳ��ͼ�� 10 - �г̱��룬���ͼ�� 11 - �г̱��룬�ڰ�ͼ��
	short colormaporigin;
	short colormaplength;
	char colormapdepth;
	short x_origin;//ͼ�����½�ˮƽ����
	short y_origin;//ͼ�����½Ǵ�ֱ����
	short width;//ͼ����
	short height;//ͼ��߶�
	char  bitsperpixel;//�������(ÿ������ռ�õ�λ��)
	char  imagedescriptor;//ͼ����������	0-3λ���涨��ÿ����������λ��������4-5λ����Щλ���ڱ�ʾ�������ݴ��ļ����͵���Ļ��˳��λ4��ʾ�����ң�λ5��ʾ���ϵ��¡�
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
	int bytespp;//�ֽ���

	bool   load_rle_data(std::ifstream& in);
	bool unload_rle_data(std::ofstream& out);
public:
	enum Format {
		GRAYSCALE = 1, RGB = 3, RGBA = 4//ö����Ŷ�Ӧ��ʽ�Ĵ�С
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