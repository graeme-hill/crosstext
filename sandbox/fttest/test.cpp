#include <iostream>
#include <vector>
#include <png.h>
#include <ft2build.h>
#include FT_FREETYPE_H

void savePng(std::vector<uint8_t> &bytes, int width, int height)
{
	FILE *file;

	file = fopen("./test.png", "wb");
	if (!file)
	{
		std::cout << "fopen failed" << std::endl;
		return;
	}

	auto pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
			NULL, NULL, NULL);
	if (!pngPtr)
	{
		std::cout << "png_create_write_struct failed" << std::endl;
		return;
	}

	auto infoPtr = png_create_info_struct(pngPtr);
	if (!infoPtr)
	{
		std::cout << "png_create_info_struct failed" << std::endl;
		return;
	}

	if (setjmp(png_jmpbuf(pngPtr)))
	{
		std::cout << "longjumped to an error for some reason?" << std::endl;
		return;
	}

	png_init_io(pngPtr, file);

	png_set_IHDR(pngPtr, infoPtr, width, height, 8, PNG_COLOR_TYPE_RGBA,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(pngPtr, infoPtr);

	std::vector<png_byte> row(width * 4, 0);

	for (int y = 0; y < height; y++)
	{
		for (int xByte = 0; xByte < width * 4; xByte++)
		{
			row[xByte] = bytes[y * width * 4 + xByte];
		}
		png_write_row(pngPtr, &row[0]);
	}

	png_write_end(pngPtr, NULL);

	fclose(file);
	png_free_data(pngPtr, infoPtr, PNG_FREE_ALL, -1);
	png_destroy_write_struct(&pngPtr, nullptr);
}

int run()
{
	FT_Library library;
	FT_Face face;

	auto error = FT_Init_FreeType(&library);

	if (error)
	{
		std::cout << "failed to init freetype" << error << std::endl;
		return 1;
	}

	error = FT_New_Face(
			library,
			"slabo.ttf",
			0,
			&face);

	if (error == FT_Err_Unknown_File_Format)
	{
		std::cout << "unknown font file format :(" << std::endl;
		return 1;
	}
	else if (error)
	{
		std::cout << "failed to load font" << std::endl;
		return 1;
	}

	error = FT_Set_Char_Size(face, 0, 16*64, 2560, 1440);

	if (error)
	{
		std::cout << "failed to set char size" << std::endl;
		return 1;
	}

	error = FT_Set_Pixel_Sizes(face, 0, 64);

	if (error)
	{
		std::cout << "failed to set pixel sizes" << std::endl;
	}

	std::string text("VAVA WoWoW");

	int texWidth = 600;
	int texHeight = 600;
	std::vector<uint8_t> imageBytes(texWidth * texHeight * 4, 0);
	int penX = 0;
	int penY = 100;
	int r = 0;
	int g = 0;
	int b = 255;
	int a = 255;

	uint8_t prev = 0;
	for (auto character : text)
	{
		auto glyphIndex = FT_Get_Char_Index(face, character);
		FT_Vector kerning;
		kerning.x = 0;
		kerning.y = 0;

		if (prev != 0)
		{
			error = FT_Get_Kerning(
				face, prev, character, FT_KERNING_DEFAULT, &kerning);

			if (error)
			{
				std::cout << "failed to get kerning" << std::endl;
			}
			else
			{
				std::cout << "kerning b/w " << prev << " and " << character
					<< ": " << kerning.x << std::endl;
			}
		}

		error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);

		if (error)
		{
			std::cout << "failed to load glyph" << std::endl;
			return 1;
		}

		error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

		if (error)
		{
			std::cout << "failed to render glyph" << std::endl;
			return 1;
		}

		int effectivePenY = penY - face->glyph->bitmap_top + kerning.y;
		int effectivePenX = penX + face->glyph->bitmap_left + kerning.x;

		auto bitmap = face->glyph->bitmap;
		for (int y = 0; y < bitmap.rows; y++)
		{
			int rowOffset = (effectivePenY + y) * (texWidth * 4)
				+ effectivePenX * 4;
			for (int x = 0; x < bitmap.width; x++)
			{
				int pixelOffset = rowOffset + x * 4;
				int ftalpha = bitmap.buffer[y * bitmap.width + x];
				float ftalphaf = static_cast<float>(ftalpha) / 255.0f;
				int finalAlpha = static_cast<int>(ftalphaf * static_cast<float>(a));
				imageBytes[pixelOffset + 0] = r;
				imageBytes[pixelOffset + 1] = g;
				imageBytes[pixelOffset + 2] = b;
				imageBytes[pixelOffset + 3] = finalAlpha;
			}
		}

		penX += face->glyph->advance.x >> 6;
		penY += face->glyph->advance.y >> 6;

		prev = character;
	}

	savePng(imageBytes, texWidth, texHeight);

	std::cout << "ok" << std::endl;

	return 0;
}

int main()
{
	return run();
}
