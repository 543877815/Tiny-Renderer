#pragma once
#include<iostream>
#include<vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

enum class FilterType : uint32_t
{
	Nearest = 0,
	Linear,
	NearestMipmapNearest,
	LinearMipmapNearest,
	NearestMipmapLinear,
	LinearMipmapLinear
};

enum class WrapType : uint32_t
{
	Repeat = 0,
	MirroredRepeat,
	ClampToEdge,
	MirrorClampToEdge
};

static GLenum filterTypeToGL[] = {
	GL_NEAREST,
	GL_LINEAR,
	GL_NEAREST_MIPMAP_NEAREST,
	GL_LINEAR_MIPMAP_NEAREST,
	GL_NEAREST_MIPMAP_LINEAR,
	GL_LINEAR_MIPMAP_LINEAR
};

static GLenum wrapTypeToGL[] = {
	GL_REPEAT,
	GL_MIRRORED_REPEAT,
	GL_CLAMP_TO_EDGE,
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
	size_t GenerateTexture(int width, int height, uint32_t internal_format, uint32_t data_format, uint32_t data_type, Params& params, void* data);
	uint32_t GetTexture(size_t idx);
	void BindTexture(size_t idx);

private:
	size_t m_idx = 0;
};


class TextureArray {
public:
	TextureArray() {
		GLuint textureArray;
		glGenTextures(1, &textureArray);
		glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);
	}

};