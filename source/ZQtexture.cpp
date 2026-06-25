	// ZQtexture.cpp
//#define STB_IMAGE_IMPLEMENTATION
#include "../include/tiny_gltf/stb_image.h"
#include "../include/ZQtexture.h"

void ZQtexture::load_from(std::string path) {
	int width, height, channels;

	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
	if (data) {
		this->dims = dim_t(width, height);
		std::vector<color_t> map = std::vector<color_t>();
		if (channels == 4) {
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					color_t c{};

					c.x = data[(y * width + x * 4)];
					c.y = data[(y * width + x * 4) + 1];
					c.z = data[(y * width + x * 4) + 2];
					c.w = data[(y * width + x * 4) + 3];

					map.push_back(c);
				}
			}
		}
		else if (channels == 3) {
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					color_t c{};

					c.x = data[(y * width + x * 4)];
					c.y = data[(y * width + x * 4) + 1];
					c.z = data[(y * width + x * 4) + 2];
					c.w = 255;

					map.push_back(c);
				}
			}
		}

		glGenTextures(1, &this->texture);
		glBindTexture(GL_TEXTURE_2D, this->texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, map.data());
		glGenerateMipmap(GL_TEXTURE_2D);



		stbi_image_free(data);
	}
	else {
#ifdef DEBUG
		std::cout << "Cannot find image: " << path << std::endl;
#endif
	}

}

ZQtexture::ZQtexture(std::string path, std::string name) {
	this->path = path;
	this->name = name;

	this->load_from(this->path);
}