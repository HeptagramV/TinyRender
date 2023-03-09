#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"


using namespace std;

Model::Model(const char* filename) :verts_(), faces_() {
	ifstream in;
	in.open(filename, ifstream::in);
	if (in.fail())return;
	string line;
	while (!in.eof()) {
		getline(in, line);
		istringstream iss(line.c_str());
		char trash;
		if (!line.compare(0, 2, "v ")) {//比较前两个字符确认该行是否为顶点信息
			iss >> trash;//定义一个字符变量trash，用于丢弃无用的字符
			Vec3f v;
			for (int i = 0; i < 3; i++)iss >> v.raw[i];//从字符串流中读取三个浮点数，分别赋值给v的x,y,z分量
			verts_.push_back(v);
		}
		else if (!line.compare(0, 3, "vt ")) {
			iss >> trash >> trash;
			Vec2f t;
			iss >> t.x >> t.y;
			uv_.push_back(t);
		}
		else if (!line.compare(0, 3, "vn ")) {
			iss >> trash >> trash;
			Vec3f n;
			iss >> n.x >> n.y >> n.z;
			norms_.push_back(n);
		}
		else if (!line.compare(0, 2, "f ")) {
			vector<Vec3i> f;
			Vec3i tmp;
			iss >> trash;//无用字符输出，即把'f'字符丢弃
			while (iss >> tmp.x >> trash >> tmp.y >> trash >> tmp.z) {//当字符串流中还有数据时，每次读取五个数据：顶点索引、斜杠、纹理坐标索引、斜杠、法线索引。其中只有顶点索引是需要的。
				for (int i = 0; i < 3; i++) tmp.raw[i]--; //因为.obj文件中的索引是从1开始计数的，而C++中是从0开始计数的，所以要把idx减一以保持一致。
				f.push_back(tmp);
			}
			faces_.push_back(f);
		}
	}
	std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << " vt# " << uv_.size() << " vn# " << norms_.size() << std::endl;
	load_texture(filename, "_diffuse.tga", diffusemap_);
}

Model::~Model() {
}

int Model::nverts() {
	return (int)verts_.size();
}

int Model::nfaces() {
	return (int)faces_.size();
}

vector<int>Model::face(int idx) {
	vector<int>face;
	for (int i = 0; i < (int)faces_[idx].size(); i++) face.push_back(faces_[idx][i].raw[0]);
	return face;
}

Vec3f Model::vert(int i) {
	return verts_[i];
}

TGAColor Model::diffuse(Vec2i uv) {
	return diffusemap_.get(uv.x, uv.y);
}

Vec3f Model::norm(int iface, int nvert) {
	int idx = faces_[iface][nvert].raw[2];
	return norms_[idx];
}

Vec2i Model::uv(int iface, int nvert) {
	int idx = faces_[iface][nvert].raw[1];
	return Vec2i(uv_[idx].x * diffusemap_.get_width(), uv_[idx].y * diffusemap_.get_height());
}

void Model::load_texture(std::string filename, const char* suffix, TGAImage& img) {
	string texfile(filename);
	size_t dot = texfile.find_last_of(".");
	if (dot != string::npos) {
		texfile = texfile.substr(0, dot) + std::string(suffix);
		cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
		img.flip_vertically();
	}
}