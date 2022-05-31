#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture()
	: filename(""), width(0), height(0), id(0), loaded(false)
{}

Texture::Texture(const char* filename, TextureFiltering filtering, TextureFormat format, TextureWrapMode wrapMode)
{
	Load(filename, format);
	SetFiltering(filtering);
	SetWrapMode(wrapMode);
}

Texture::~Texture()
{
	Unload();
}

bool Texture::Load(const char* filename, TextureFormat format)
{
	Unload();
	
	this->filename = filename;
	int reqChannels;
	switch (format)
	{
	case TextureFormat::R:
		reqChannels = 1;
		break;
	case TextureFormat::RG:
		reqChannels = 2;
		break;
	case TextureFormat::RGB:
		reqChannels = 3;
		break;
	case TextureFormat::RGBA:
		reqChannels = 4;
		break;
	default:
		reqChannels = 0;
		break;
	}
	int channels;
	unsigned char* data;
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load(filename, &width, &height, &channels, reqChannels);

	if (!data)
		return false;

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	//do differently based on returned value
	switch (channels)
	{
	case 1:
		this->format = TextureFormat::R;
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		break;
	case 2:
		this->format = TextureFormat::RG;
		glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
		break;
	case 3:
		this->format = TextureFormat::RGB;
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		break;
	case 4:
		this->format = TextureFormat::RGBA;
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		break;
	default:
		this->format = TextureFormat::RGBA;
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		break;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, (GLenum)this->format, width, height, 0, (GLenum)this->format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	loaded = true;
	return true;
}

void Texture::Unload()
{
	if (!loaded)
		return;

	glDeleteTextures(1, &id);
	loaded = false;
	width = 0;
	height = 0;
	id = 0;
	filename = "";
}

void Texture::Bind(GLuint unit) const
{
	if (!loaded)
		return;

	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::SetFiltering(TextureFiltering filtering)
{
	if (!loaded) 
		return;
	Bind();

	this->filtering = filtering;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLenum)filtering);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLenum)filtering);
}

void Texture::SetWrapMode(TextureWrapMode wrapmode)
{
	if (!loaded)
		return;
	Bind();

	this->wrapMode = wrapMode;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, (GLenum)wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLenum)wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLenum)wrapMode);
}
