#pragma once
#include<iostream>
#include<vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

enum class FilterType
{
	Nearest = 0,
	Linear,
	NearestMipmapNearest,
	LinearMipmapNearest,
	NearestMipmapLinear,
	LinearMipmapLinear
};

enum class WrapType
{
	Repeat = 0,
	MirroredRepeat,
	ClampToEdge,
	MirrorClampToEdge
};

class Texture {
public:
	struct Params
	{
		FilterType minFilter;
		FilterType magFilter;
		WrapType sWrap;
		WrapType tWrap;
	};

	std::vector<uint32_t> m_textures;
	Texture(size_t tex_num);
	size_t GenerateTexture(const std::string& path);
	uint32_t GetTexture(size_t idx);
	void BindTexture(size_t idx);
};


class TextureArray {
public:
	TextureArray() {
		GLuint textureArray;
		glGenTextures(1, &textureArray);
		glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);
	}

};