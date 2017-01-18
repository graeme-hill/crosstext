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

namespace ct
{
	class DirectWriteRenderOptions
	{
	public:
		DirectWriteRenderOptions();
		DirectWriteRenderOptions(Size textureSize);
		Size textureSize() const { return _textureSize; }

	private:
		Size _textureSize;
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

	//class DirectWriteFont
	//{
	//public:
	//	DirectWriteFont(DirectWriteRenderer &renderer, FontOptions fontOptions);
	//	DirectWriteFont(const DirectWriteFont &) = delete;
	//	DirectWriteFont(DirectWriteFont &&) = delete;
	//	~DirectWriteFont();
	//	IDWriteTextFormat *format() { return _format; }

	//private:
	//	IDWriteTextFormat *_format;
	//};

	class DirectWriteBuilder
	{
	public:
		DirectWriteBuilder(
			DirectWriteRenderer &renderer,
			Text text,
			FontOptions font);
		DirectWriteBuilder(const DirectWriteBuilder &) = delete;
		DirectWriteBuilder(DirectWriteBuilder &&) = delete;
		~DirectWriteBuilder();

		Size size() const;
		void render(DirectWriteImageData &imageData, Rect rect);
		void clearRect(DirectWriteImageData &imageData, Rect rect);

	private:
		DirectWriteRenderer &_renderer;
		Text _text;
		IDWriteTextLayout *_layout;
		IDWriteTextFormat *_format;
		FontOptions _font;
	};

	D2D1_COLOR_F convertColor(Color color);
	ID2D1Brush *convertBrush(Brush brush, ID2D1RenderTarget *target);
	DWRITE_FONT_WEIGHT convertFontWeight(FontWeight weight);
	DWRITE_FONT_STYLE convertFontStyle(FontStyle style);
	DWRITE_FONT_STRETCH convertFontStretch(FontStretch stretch);
}