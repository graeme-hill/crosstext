#include "CrossText.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <png.h>

namespace xt
{
	class LibPngWriter
	{
	public:
		LibPngWriter(Size size, std::string basePath) :
			_size(size),
			_bytes(size.width * size.height * 4),
			_basePath(basePath),
			_frame(0)
		{ }

		LibPngWriter(const LibPngWriter &) = delete;

		LibPngWriter(LibPngWriter &&other) :
			_size(other._size),
			_bytes(std::move(other._bytes)),
			_basePath(std::move(other._basePath)),
			_frame(other._frame)
		{ }

		void write(std::vector<uint8_t> pixels, Rect rect)
		{
			// call std::Copy once per line
			for (unsigned sourceRow = 0; sourceRow < rect.height; sourceRow++)
			{
				auto destRow = rect.y + sourceRow;
				auto startSource = sourceRow * rect.width;
				auto startDest = destRow * _size.width + rect.x;
				auto bytesPerSourceRow = rect.width * 4;

				std::copy(
					pixels.begin() + startSource,
					pixels.begin() + startSource + bytesPerSourceRow,
					_bytes.begin() + startDest);
			}
		}

		void commit()
		{
			std::stringstream ss;
			ss << _basePath << _frame++ << ".png";
			auto path = ss.str();

			FILE *file;
			file = fopen(path.c_str(), "wb");
			auto pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
				NULL, NULL, NULL);
			auto infoPtr = png_create_info_struct(pngPtr);
			png_init_io(pngPtr, file);
			png_set_IHDR(pngPtr, infoPtr, _size.width, _size.height, 8,
				PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
				PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
			png_write_info(pngPtr, infoPtr);

			std::vector<png_byte> row(_size.width * 4, 0);
			for (unsigned y = 0; y < _size.height; y++)
			{
				for (unsigned xByte = 0; xByte < _size.width * 4; xByte++)
				{
					row[xByte] = _bytes[y * _size.width * 4 + xByte];
				}
				png_write_row(pngPtr, &row[0]);
			}

			png_write_end(pngPtr, NULL);
			fclose(file);
			png_free_data(pngPtr, infoPtr, PNG_FREE_ALL, -1);
			png_destroy_write_struct(&pngPtr, nullptr);
		}

		void setPixel(
			unsigned x,
			unsigned y,
			uint8_t r,
			uint8_t g,
			uint8_t b,
			uint8_t a)
		{
			if (x >= _size.width)
				return;
			if (y >= _size.height)
				return;

			auto offset = (_size.width * y + x) * 4;
			_bytes[offset + 0] = r;
			_bytes[offset + 1] = g;
			_bytes[offset + 2] = b;
			_bytes[offset + 3] = a;
		}

		Size size() const { return _size; }

	private:
		Size _size;
		std::vector<uint8_t> _bytes;
		std::string _basePath;
		unsigned _frame;
	};
}
