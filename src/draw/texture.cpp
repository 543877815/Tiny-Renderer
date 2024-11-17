#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

Texture::Texture(size_t tex_num) {
	m_textures.resize(tex_num);
	std::fill(m_textures.begin(), m_textures.end(), 0);
}

size_t Texture::GenerateTexture(const std::string&path) {
	size_t idx = 0;
	for (size_t i = 0; i < m_textures.size(); i++) {
		if (m_textures[i] == 0) {
			idx = i;
			break;
		}
	}
	glGenTextures(1, &m_textures[idx]);
	glBindTexture(GL_TEXTURE_2D, m_textures[idx]);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		if (nrChannels == 3) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	return idx;
}

uint32_t Texture::GetTexture(size_t idx) {
	return m_textures[idx];
}

void Texture::BindTexture(size_t idx) {
	glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(idx));
	glBindTexture(GL_TEXTURE_2D, m_textures[idx]);
}