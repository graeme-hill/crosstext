#pragma once

#include "Common.hpp"
#include <string>
#include <Windows.h>
#include <wincodec.h>
#include <d3d11.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <dwrite.h>

#define DEFAULT_TEXTURE_SIZE 4096
#define DEFAULT_TEXTURE_COUNT 1

namespace ct
{
	class DirectWriteRenderOptions
	{
	public:
		DirectWriteRenderOptions();
		DirectWriteRenderOptions(Size textureSize, int textureCount);
		Size textureSize() const { return _textureSize; }
		int textureCount() const { return _textureCount; }

	private:
		Size _textureSize;
		int _textureCount;
	};

	class DirectWriteRenderer
	{
	public:
		DirectWriteRenderer(DirectWriteRenderOptions options);
		~DirectWriteRenderer();
		IDWriteFactory *dwriteFactory() { return _dwriteFactory; }
		ID2D1Factory1 *d2dFactory() { return _d2dFactory; }
		IWICImagingFactory *wicFactory() { return _wicFactory; }
		Size textureSize() const { return _options.textureSize(); }

	private:
		DirectWriteRenderOptions _options;
		IDXGIDevice *_dxgiDevice;
		ID3D11Device *_d3dDevice;
		ID3D11DeviceContext *_d3dContext;
		ID2D1Factory1 *_d2dFactory;
		ID2D1Device *_d2dDevice;
		IDWriteFactory *_dwriteFactory;
		IWICImagingFactory *_wicFactory;
	};

	class DirectWriteImageData
	{
	public:
		DirectWriteImageData(DirectWriteRenderer &renderer, Size size);
		DirectWriteImageData(const DirectWriteImageData &) = delete;
		DirectWriteImageData(DirectWriteImageData &&);
		~DirectWriteImageData();
		ID2D1RenderTarget *target() { return _renderTarget; }
		IWICBitmap *bitmap() { return _bitmap; }
		Size size() const { return _size; }
		void savePng(LPCWSTR path);
		ID2D1Bitmap *transparentBmp() { return _transparentBmp; }
		void clearRect(Rect rect);

	private:
		IWICBitmap *_bitmap;
		ID2D1RenderTarget *_renderTarget;
		DirectWriteRenderer &_renderer;
		Size _size;
		char _transparentData[4];
		ID2D1Bitmap *_transparentBmp;
	};

	class DirectWriteBuilder
	{
	public:
		DirectWriteBuilder(
			DirectWriteRenderer &renderer,
			std::wstring text,
			FontOptions font,
			std::vector<FontRange> &fontRanges);
		DirectWriteBuilder(const DirectWriteBuilder &) = delete;
		DirectWriteBuilder(DirectWriteBuilder &&) = delete;
		~DirectWriteBuilder();

		Size size() const;
		void render(DirectWriteImageData &imageData, Rect rect);

	private:
		DirectWriteRenderer &_renderer;
		std::wstring _text;
		IDWriteTextLayout *_layout;
		IDWriteTextFormat *_format;
		FontOptions _font;
	};

	class DirectWriteBuilder2
	{
	public:
		DirectWriteBuilder2(
			DirectWriteRenderer &renderer,
			std::wstring text,
			FontOptions font,
			std::vector<FontRange> &fontRanges);
		DirectWriteBuilder2(const DirectWriteBuilder2 &) = delete;
		DirectWriteBuilder2(DirectWriteBuilder2 &&) = delete;
		~DirectWriteBuilder2();

		Size size() const;
		void render(DirectWriteImageData &imageData, Rect rect);

	private:
		DirectWriteRenderer &_renderer;
		std::wstring _text;
		ID2D1PathGeometry *_geometry;
		ID2D1GeometrySink *_sink;
		IDWriteFontFace *_fontFace;
		FontOptions _font;
	};

	D2D1_COLOR_F convertColor(Color color);
	ID2D1Brush *convertBrush(Brush brush, ID2D1RenderTarget *target);
	DWRITE_FONT_WEIGHT convertFontWeight(FontWeight weight);
	DWRITE_FONT_STYLE convertFontStyle(FontStyle style);
	DWRITE_FONT_STRETCH convertFontStretch(FontStretch stretch);
}