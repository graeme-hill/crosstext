#include "CrossText.hpp"
#include <iostream>

namespace ct
{
	class LibPngWriter
	{
	public:
		LibPngWriter(Size size) : _size(size)
		{ }

		LibPngWriter(const LibPngWriter &) = delete;

		LibPngWriter(LibPngWriter &&other) : _size(other._size)
		{ }

		void write(std::vector<unsigned char> pixels, Rect rect)
		{
			std::cout << "write" << std::endl;
		}

		void commit()
		{
			std::cout << "commit" << std::endl;
		}

		Size size() const { return _size; }

	private:
		Size _size;
	};
}
