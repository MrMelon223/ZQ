#ifndef ZQSHADER_H
#define ZQSHADER_H

#include "ZQlighting.h"
#include "ZQmodel.h"

class ZQshader_program{
protected:
	std::string name;
	std::string vs_filepath;
	std::string ps_filepath;

	std::vector<char> vs_glsl_source;
	std::vector<char> ps_glsl_source;

	GLuint vs_shader_program;
	GLuint ps_shader_program;
	GLuint shader_linked;	// used for "top level" programs

	//cl_kernel cl_shader_kernel;

	void load_from(std::string, GLenum);
	void create_shader();
	void create_program();
public:
	ZQshader_program(std::string, std::string, std::string);

	GLuint get_program() { return this->shader_linked; }

};

typedef struct {

} d_ZQshader;

#endif
