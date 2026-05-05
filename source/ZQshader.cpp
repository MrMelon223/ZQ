	// ZQshader.cpp
#include "../include/ZQshader.h"

void ZQshader_program::load_from(std::string path, GLenum t) {
	std::ifstream in(path, std::ios::in);
	if (!in.is_open()) {
#ifdef DEBUG
		std::cout << "GLSL File: " << path << " not found" << std::endl;
#endif
		return;
	}

	if (t == GL_VERTEX_SHADER) {
		this->vs_glsl_source = std::vector<char>();
		char ch;
		while (in.get(ch)) {
			this->vs_glsl_source.push_back(ch);
			//std::cout << (unsigned int)ch << " ";
		}
		this->vs_glsl_source.push_back('\0');
	}
	else if (t == GL_FRAGMENT_SHADER) {
		this->ps_glsl_source = std::vector<char>();
		char ch;
		while (in.get(ch)) {
			this->ps_glsl_source.push_back(ch);
			//std::cout << (unsigned int)ch << " ";
		}
		this->ps_glsl_source.push_back('\0');
	}

#ifdef DEBUG
	//std::cout << "CL Shader Loaded: \n\n" << this->cl_source.data() << std::endl;
#endif
}

void ZQshader_program::create_shader() {
	std::string source(this->vs_glsl_source.data());
	const char* src = source.c_str();
	size_t leng = source.length();
	int_t err;

	this->vs_shader_program = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(this->vs_shader_program, 1, (const GLchar**)&src, (GLint*)&leng);

	glCompileShader(this->vs_shader_program);

	GLint isCompiled = 0;
	glGetShaderiv(this->vs_shader_program, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled != GL_TRUE) {
#ifdef DEBUG
		GLint maxLength = 0;
		glGetShaderiv(this->vs_shader_program, GL_INFO_LOG_LENGTH, &maxLength);

		char* error_mess = new char[maxLength];
		glGetShaderInfoLog(this->vs_shader_program, maxLength, &maxLength, error_mess);
		std::cout << "VS_GLSL Error:\n" << error_mess << std::endl;
#endif
		glDeleteShader(this->vs_shader_program);
	}
	else {
		std::string psource(this->ps_glsl_source.data());
		const char* psrc = psource.c_str();
		size_t pleng = psource.length();
		int_t perr;

		this->ps_shader_program = glCreateShader(GL_FRAGMENT_SHADER);

		glShaderSource(this->ps_shader_program, 1, (const GLchar**)&psrc, (GLint*)&pleng);

		glCompileShader(this->ps_shader_program);

		GLint pisCompiled = 0;
		glGetShaderiv(this->ps_shader_program, GL_COMPILE_STATUS, &pisCompiled);
		if (pisCompiled != GL_TRUE) {
#ifdef DEBUG
			GLint pmaxLength = 0;
			glGetShaderiv(this->ps_shader_program, GL_INFO_LOG_LENGTH, &pmaxLength);

			char* perror_mess = new char[pmaxLength];
			glGetShaderInfoLog(this->ps_shader_program, pmaxLength, &pmaxLength, perror_mess);
			std::cout << "PS_GLSL Error:\n" << perror_mess << std::endl;
#endif
			glDeleteShader(this->ps_shader_program);
		}
		else {
#ifdef DEBUG
			std::cout << "V & P Shaders loaded: " << this->name << "[*]" << std::endl;
#endif
		}
	}

	/*this->cl_shader_kernel = clCreateKernel(this->cl_shader_program, this->name.c_str(), &err);
	check_cl(err, "shader program to kernel");*/

}

void ZQshader_program::create_program() {
	this->shader_linked = glCreateProgram();

	glAttachShader(this->shader_linked, this->vs_shader_program);
	glAttachShader(this->shader_linked, this->ps_shader_program);

	glLinkProgram(this->shader_linked);

	GLint isLinked = GL_TRUE;
	glGetShaderiv(this->shader_linked, GL_LINK_STATUS, &isLinked);
	if (isLinked != GL_TRUE) {
#ifdef DEBUG
		GLint maxLength = 0;
		glGetShaderiv(this->shader_linked, GL_INFO_LOG_LENGTH, &maxLength);

		char* error_mess = new char[maxLength];
		glGetShaderInfoLog(this->shader_linked, maxLength, &maxLength, error_mess);
		std::cout << "GLSL Link Error " << this->name << ": " << error_mess << std::endl;
#endif
		glDeleteProgram(this->shader_linked);
	}
}

ZQshader_program::ZQshader_program(std::string vs_path, std::string ps_path, std::string name) {
	this->vs_filepath = vs_path;
	this->ps_filepath = ps_path;
	this->name = name;
	this->load_from(this->vs_filepath, GL_VERTEX_SHADER);
	this->load_from(this->ps_filepath, GL_FRAGMENT_SHADER);
	this->create_shader();
	this->create_program();
}