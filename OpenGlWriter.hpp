#pragma once

#include "CrossText.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <GL/gl.h>

BEGIN_XT_NAMESPACE

class OpenGlWriter
{
public:
	OpenGlWriter(Size size);
	OpenGlWriter(const OpenGlWriter &) = delete;
	OpenGlWriter(OpenGlWriter &&other);
	~OpenGlWriter();

	void write(std::vector<uint8_t> pixels, Rect rect);

	void commit();

	void setPixel(
		unsigned x,
		unsigned y,
		uint8_t r,
		uint8_t g,
		uint8_t b,
		uint8_t a);

	Size size() const { return _size; }

private:
	Size _size;
	GLuint _textureId;
};

OpenGlWriter::OpenGlWriter(Size size) : _size(size), _textureId(-1)
{
	std::vector<GLubyte> blankData(size.width * size.height * 4, 0);

	glGenTextures(1, &_textureId);
	glBindTexture(GL_TEXTURE_2D, _textureId);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2d(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		_size.width,
		_size.height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		&blankData[0]);
}

OpenGlWriter::OpenGlWriter(OpenGlWriter &&other) :
	_size(other._size), _textureId(other._textureId)
{
	other._textureId = -1;
}

OpenGlWriter::~OpenGlWriter()
{
	if (_textureId >= 0)
	{
		glDeleteTextures(1, &_textureId);
	}
}

void OpenGlWriter::write(std::vector<uint8_t> pixels, Rect rect)
{
	glTexSubImage2d(
		GL_TEXTURE_2D,
		0,
		rect.x,
		rect.y,
		rect.width,
		rect.height,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		&pixels[0]);
}

void OpenGlWriter::setPixel(
	unsigned x,
	unsigned y,
	uint8_t r,
	uint8_t g,
	uint8_t b,
	uint8_t a)
{
	uint8_t data[] = { r, g, b, a };

	glTexSubImage2d(
		GL_TEXTURE_2D, 0, x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

END_XT_NAMESPACE
