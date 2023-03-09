#include<iostream>
#include<fstream>
#include<string.h>
#include<time.h>
#include<math.h>
#include "tgaimage.h"

TGAImage::TGAImage() :data(NULL), width(0), height(0), bytespp(0) {
}

TGAImage::TGAImage(int w, int h, int bpp) : data(NULL), width(w), height(h), bytespp(bpp) {
	unsigned long nbytes = width * height * bytespp;
	data = new unsigned char[nbytes];//分配内存
	memset(data, 0, nbytes);
}

TGAImage::TGAImage(const TGAImage& img) {//拷贝构造
	width = img.width;
	height = img.height;
	bytespp = img.bytespp;
	unsigned long nbytes = width * height * bytespp;
	data = new unsigned char[nbytes];
	memcpy(data, img.data, nbytes);//进行数据深拷贝
}

TGAImage::~TGAImage() {
	if (data)delete[] data;
}

TGAImage& TGAImage:: operator=(const TGAImage& img) {//重载赋值运算
	if (this != &img) {
		if (data) delete[] data;
		width = img.width;
		height = img.height;
		bytespp = img.bytespp;
		unsigned long nbytes = width * height * bytespp;
		data = new unsigned char[nbytes];
		memcpy(data, img.data, nbytes);
	}
	return *this;
}

bool TGAImage::read_tga_file(const char* filename) {//读取TGA图像文件
	if (data)delete[]data;
	data = NULL;
	std::ifstream in;
	in.open(filename, std::ios::binary);//二进制方式打开文件
	if (!in.is_open()) {
		std::cerr << "未能打开文件" << filename << "\n";
		in.close();
		return false;
	}
	TGA_Header header;
	in.read((char*)&header, sizeof(header));//从输入流中读取sizeof个字符(TGA的文件头信息)，存入数组header中
	if (!in.good()) {
		in.close();
		std::cerr << "读取文件头时发生错误";
		return false;
	}
	width = header.width;//可能由于指定了存储方式的原因，此处只读取了文件头，直接读取到了header的属性，这个读取和文件头的结构体应该是专门为TGA文件设计的；
	height = header.height;
	bytespp = header.bitsperpixel >> 3;//位数右移三位得到每个像素占用的字节数
	if (width <= 0 || height <= 0 || (bytespp != GRAYSCALE && bytespp != RGB && bytespp != RGBA)) {
		in.close();
		std::cerr << "错误的bpp(存储每个像素所用的位数)值或错误的宽高值\n";
		return false;
	}
	unsigned long nbytes = bytespp * width * height;
	data = new unsigned char[nbytes];
	if (3 == header.datatypecode || 2 == header.datatypecode) {
		in.read((char*)data, nbytes);//再次读取整个文件至data数组
		if (!in.good()) {
			in.close();
			std::cerr << "读取文件数据时发生错误";
			return false;
		}
	}
	else if (10 == header.datatypecode || 11 == header.datatypecode) {
		if (!load_rle_data(in)) {
			in.close();
			std::cerr << "读取文件数据时发生错误";
			return false;
		}
	}
	else {
		in.close();
		std::cerr << "未知文件格式 " << (int)header.datatypecode << "\n";
		return false;
	}
	if (!(header.imagedescriptor & 0x20)) {//二进制10100,结合图像描述符，第五位表示从上到下，因此若第五位为真则翻转图像为从下到上(原点在左下角)
		flip_vertically();
	}
	if (header.imagedescriptor & 0x10) {//1010,同上，第四位表示从左到右，因此若不为真则翻转
		flip_horizontally();
	}
	std::cerr << width << "x" << height << "/" << bytespp * 8 << "\n";
	in.close();
	return true;
}

bool TGAImage::load_rle_data(std::ifstream& in) {//加载行程编码图片(rle)数据，对应datatypecode的9,10,11
	unsigned long pixelcount = width * height;//计算像素总数
	unsigned long currentpixel = 0;
	unsigned long currentbyte = 0;
	TGAColor colorbuffer;
	do {
		unsigned char chunkheader = 0;
		chunkheader = in.get();//字符按int返回
		if (!in.good()) {
			std::cerr << "读取文件时发生错误\n";
			return false;
		}
		if (chunkheader < 128) {
			chunkheader++;
			for (int i = 0; i < chunkheader; i++) {
				in.read((char*)colorbuffer.raw, bytespp);//逐个读取像素数据
				if (!in.good()) {
					std::cerr << "读取文件头时发生错误\n";
					return false;
				}
				for (int t = 0; t < bytespp; t++)
					data[currentbyte++] = colorbuffer.raw[t];//填充像素数据至data数组
				currentpixel++;
				if (currentpixel > pixelcount) {
					std::cerr << "读取到过多像素\n";
					return false;
				}
			}
		}
		else {//可能是行程编码的某种机制需要这样读取，此部分没有深究
			chunkheader -= 127;
			in.read((char*)colorbuffer.raw, bytespp);
			if (!in.good()) {
				std::cerr << "读取文件头时出现错误\n";
				return false;
			}
			for (int i = 0; i < chunkheader; i++) {
				for (int t = 0; t < bytespp; t++)
					data[currentbyte++] = colorbuffer.raw[t];
				currentpixel++;
				if (currentpixel > pixelcount) {
					std::cerr << "读取到过多像素\n";
					return false;
				}
			}
		}
	} while (currentpixel < pixelcount);
	return true;
}

bool TGAImage::write_tga_file(const char* filename, bool rle) {//写入图片
	unsigned char developer_area_ref[4] = { 0,0,0,0 };
	unsigned char extension_area_ref[4] = { 0,0,0,0 };
	unsigned char footer[18] = { 'T','R','U','E','V','I','S','I','O','N','-','X','F','I','L','E','.','\0' };//truevision-xfile
	std::ofstream out;
	out.open(filename, std::ios::binary);
	if (!out.is_open()) {
		std::cerr << "无法打开文件 " << filename << "\n";
		out.close();
		return false;
	}
	TGA_Header header;
	memset((void*)&header, 0, sizeof(header));//memset() 函数常用于非常量的内存空间初始化
	header.bitsperpixel = bytespp << 3;//左移三位得到位数
	header.width = width;
	header.height = height;
	header.datatypecode = (bytespp == GRAYSCALE ? (rle ? 11 : 3) : (rle ? 10 : 2));//设置文件头中的图像类型编码，可以参考读取文件头时的代码
	header.imagedescriptor = 0x20;//原点在左上
	out.write((char*)&header, sizeof(header));//写入文件头
	if (!out.good()) {
		out.close();
		std::cerr << "无法转储tga文件\n";
		return false;
	}
	if (!rle) {
		out.write((char*)data, width * height * bytespp);
		if (!out.good()) {
			std::cerr << "无法卸载原始数据\n";
			out.close();
			return false;
		}
	}
	else {
		if (!unload_rle_data(out)) {//调用的函数在下面，写完回来改注释
			out.close();
			std::cerr << "无法卸载图像数据\n";
			return false;
		}
	}
	out.write((char*)developer_area_ref, sizeof(developer_area_ref));
	if (!out.good()) {
		std::cerr << "无法转储tga文件\n";
		out.close();
		return false;
	}
	out.write((char*)extension_area_ref, sizeof(extension_area_ref));
	if (!out.good()) {
		std::cerr << "无法转储tga文件\n";
		out.close();
		return false;
	}
	out.write((char*)footer, sizeof(footer));
	if (!out.good()) {
		std::cerr << "无法转储tga文件\n";
		out.close();
		return false;
	}
	out.close();
	return true;
}

bool TGAImage::unload_rle_data(std::ofstream& out) {//大概是用于输出rel图片数据
	const unsigned char max_chunk_length = 128;
	unsigned long npixels = width * height;
	unsigned long curpix = 0;
	while (curpix < npixels) {
		unsigned long chunkstart = curpix * bytespp;
		unsigned long curbyte = curpix * bytespp;
		unsigned char run_length = 1;
		bool raw = true;
		while (curpix + run_length < npixels && run_length < max_chunk_length) {
			bool succ_eq = true;
			for (int t = 0; succ_eq && t < bytespp; t++) {
				succ_eq = (data[curbyte + t] == data[curbyte + t + bytespp]);
			}
			curbyte += bytespp;
			if (1 == run_length) {
				raw = !succ_eq;
			}
			if (raw && succ_eq) {
				run_length--;
				break;
			}
			if (!raw && !succ_eq) {
				break;
			}
			run_length++;
		}
		curpix += run_length;
		out.put(raw ? run_length - 1 : run_length + 127);
		if (!out.good()) {
			std::cerr << "无法转储tga文件\n";
			return false;
		}
		out.write((char*)(data + chunkstart), (raw ? run_length * bytespp : bytespp));
		if (!out.good()) {
			std::cerr << "can't dump the tga file\n";
			return false;
		}
	}
	return true;
}

TGAColor TGAImage::get(int x, int y) {
	if (!data || x < 0 || y < 0 || x >= width || y >= height) {
		return TGAColor();
	}
	return TGAColor(data + (x + y * width) * bytespp, bytespp);
}

bool TGAImage::set(int x, int y, TGAColor c) {
	if (!data || x < 0 || y < 0 || x >= width || y >= height) {
		return false;
	}
	memcpy(data + (x + y * width) * bytespp, c.raw, bytespp);
	return true;
}

int TGAImage::get_bytespp() {
	return bytespp;
}

int TGAImage::get_width() {
	return width;
}

int TGAImage::get_height() {
	return height;
}

bool TGAImage::flip_horizontally() {//水平翻转
	if (!data) return false;
	int half = width >> 1;
	for (int i = 0; i < half; i++) {
		for (int j = 0; j < height; j++) {
			TGAColor c1 = get(i, j);
			TGAColor c2 = get(width - 1 - i, j);
			set(i, j, c2);
			set(width - 1 - i, j, c1);
		}
	}
	return true;
}

bool TGAImage::flip_vertically() {//垂直翻转
	if (!data) return false;
	unsigned long bytes_per_line = width * bytespp;
	unsigned char* line = new unsigned char[bytes_per_line];
	int half = height >> 1;
	for (int j = 0; j < half; j++) {
		unsigned long l1 = j * bytes_per_line;
		unsigned long l2 = (height - 1 - j) * bytes_per_line;
		memmove((void*)line, (void*)(data + l1), bytes_per_line);
		memmove((void*)(data + l1), (void*)(data + l2), bytes_per_line);
		memmove((void*)(data + l2), (void*)line, bytes_per_line);
	}
	delete[] line;
	return true;
}

unsigned char* TGAImage::buffer() {
	return data;
}

void TGAImage::clear() {
	memset((void*)data, 0, width * height * bytespp);
}

bool TGAImage::scale(int w, int h) {
	if (w <= 0 || h <= 0 || !data) return false;
	unsigned char* tdata = new unsigned char[w * h * bytespp];
	int nscanline = 0;
	int oscanline = 0;
	int erry = 0;
	unsigned long nlinebytes = w * bytespp;
	unsigned long olinebytes = width * bytespp;
	for (int j = 0; j < height; j++) {
		int errx = width - w;
		int nx = -bytespp;
		int ox = -bytespp;
		for (int i = 0; i < width; i++) {
			ox += bytespp;
			errx += w;
			while (errx >= (int)width) {
				errx -= width;
				nx += bytespp;
				memcpy(tdata + nscanline + nx, data + oscanline + ox, bytespp);
			}
		}
		erry += h;
		oscanline += olinebytes;
		while (erry >= (int)height) {
			if (erry >= (int)height << 1) // it means we jump over a scanline
				memcpy(tdata + nscanline + nlinebytes, tdata + nscanline, nlinebytes);
			erry -= height;
			nscanline += nlinebytes;
		}
	}
	delete[] data;
	data = tdata;
	width = w;
	height = h;
	return true;
}