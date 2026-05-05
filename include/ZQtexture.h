#ifndef ZQTEXTURE_H
#define ZQTEXTURE_H

#include "ZQhelper.h"

class ZQtexture {
protected:
	std::string name;
	std::string path;

	dim_t dims;

	GLuint texture;

	void load_from(std::string);
public:
	ZQtexture(std::string, std::string);

	std::string get_name() { return this->name; }

	GLuint get_texture() { return this->texture; }
};

#endif