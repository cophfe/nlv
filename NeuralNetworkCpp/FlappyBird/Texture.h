#pragma once
#include "Graphics.h"
#include <string>

enum class TextureFiltering : GLenum //GL_TEXTURE_MAG_FILTER
{
	Nearest = GL_NEAREST,
	Linear = GL_LINEAR
};

enum class TextureWrapMode : GLenum //GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T, GL_TEXTURE_WRAP_R
{
	Wrap = GL_REPEAT,
	MirrorWrap = GL_MIRRORED_REPEAT,
	BorderClamp = GL_CLAMP_TO_BORDER,
	EdgeClamp = GL_CLAMP_TO_EDGE,
	MirrorClamp = GL_MIRROR_CLAMP_TO_EDGE,
};

enum class TextureFormat : GLenum
{
	R = GL_RED,
	RG = GL_RG,
	RGB = GL_RGB,
	RGBA = GL_RGBA,
	Auto
};

class Texture
{
public:
	Texture();
	Texture(const char* filename, TextureFiltering filtering = TextureFiltering::Linear,
		TextureFormat format = TextureFormat::Auto, TextureWrapMode wrapMode = TextureWrapMode::Wrap);
	Texture(const Texture& other) = delete;
	Texture& operator=(const Texture& other) = delete;
	~Texture();

	bool Load(const char* filename, TextureFormat format = TextureFormat::Auto);
	void Unload();

	void SetFiltering(TextureFiltering filtering);
	void SetWrapMode(TextureWrapMode wrapmode);

	inline bool GetIsLoaded() { return loaded; }
	inline const std::string& GetFilename() const { return filename; }
	inline TextureFormat GetFormat() const { return format; }
	inline TextureFiltering GetFiltering() const { return filtering; }
	inline TextureWrapMode GetWrapMode() const { return wrapMode; }
	inline int GetWidth() const { return width; }
	inline int GetHeight() const { return height; }
private:
	std::string filename;
	GLuint id;
	int width;
	int height;
	TextureFormat format;
	TextureFiltering filtering;
	TextureWrapMode wrapMode;
	bool loaded;
};

