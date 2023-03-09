#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include "tgaimage.h"
using namespace std;

class Model {
private:
	vector<Vec3f> verts_;
	vector<Vec2f> uv_;
	vector<Vec3f> norms_;
	vector<vector<Vec3i>>faces_;
	TGAImage diffusemap_;
	void load_texture(string filename, const char* suffix, TGAImage& img);
public:
	Model(const char* filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	Vec3f norm(int iface, int nvert);
	Vec2i uv(int iface,int nvert);
	TGAColor diffuse(Vec2i uv);
	vector<int> face(int idx);
};

#endif //__MODEL_H__