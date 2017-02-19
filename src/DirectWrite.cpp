#include "DirectWrite.hpp"
#include <iostream>

namespace ct
{
	DirectWriteRenderOptions::DirectWriteRenderOptions() :
		DirectWriteRenderOptions({ DEFAULT_TEXTURE_SIZE, DEFAULT_TEXTURE_SIZE }, DEFAULT_TEXTURE_COUNT)
	{ }

	DirectWriteRenderOptions::DirectWriteRenderOptions(
		Size textureSize,
		unsigned textureCount) :
		_textureSize(textureSize),
		_textureCount(textureCount)
	{ }

	DirectWriteRenderer::DirectWriteRenderer(DirectWriteRenderOptions options) :
		_d3dDevice(nullptr),
		_d3dContext(nullptr),
		_dxgiDevice(nullptr),
		_d2dDevice(nullptr),
		_dwriteFactory(nullptr),
		_wicFactory(nullptr),
		_options(options)
	{
		// Allows CoCreateInstance calls
		CoInitializeEx(NULL, COINIT_MULTITHREADED);

		// Create D3D device and context
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1
		};
		D3D_FEATURE_LEVEL featureLevel;
		D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			0,
			D3D11_CREATE_DEVICE_BGRA_SUPPORT,
			featureLevels,
			ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION,
			&_d3dDevice,
			&featureLevel,
			&_d3dContext);

		// Get dxgi from device because we need that for D2D too
		_d3dDevice->QueryInterface(
			__uuidof(IDXGIDevice),
			reinterpret_cast<void**>(&_dxgiDevice));

		// Create D2D context
		D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &_d2dFactory);
		_d2dFactory->CreateDevice(_dxgiDevice, &_d2dDevice);
		//_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &_d2dContext);

		// DirectWrite factory for making text layouts and stuff
		DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(&_dwriteFactory));

		// Make the WIC factory for bitmap generation
		auto result = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&_wicFactory));
	}

	DirectWriteRenderer::~DirectWriteRenderer()
	{
		if (_wicFactory)
			_wicFactory->Release();

		if (_dwriteFactory)
			_dwriteFactory->Release();

		if (_d2dFactory)
			_d2dFactory->Release();

		if (_d2dDevice)
			_d2dDevice->Release();

		if (_d3dContext)
			_d3dContext->Release();

		if (_d3dDevice)
			_d3dDevice->Release();

		if (_dxgiDevice)
			_dxgiDevice->Release();
	}

	// DirectWriteBuilder

	DirectWriteBuilder::DirectWriteBuilder(
		DirectWriteRenderer &renderer,
		std::wstring text,
		TextOptions options)
		:
		_renderer(renderer),
		_text(text),
		_layout(nullptr),
		_format(nullptr),
		_options(options)
	{
		renderer.dwriteFactory()->CreateTextFormat(
			options.baseFont.family.c_str(),
			nullptr,
			convertFontWeight(options.baseFont.weight),
			convertFontStyle(options.baseFont.style),
			convertFontStretch(options.baseFont.stretch),
			options.baseFont.size,
			options.baseFont.locale.c_str(),
			&_format);

		_renderer.dwriteFactory()->CreateTextLayout(
			_text.c_str(),
			(UINT32)_text.size(),
			_format,
			(float)_renderer.textureSize().width,
			(float)_renderer.textureSize().height,
			&_layout);

		auto a = options.baseFont;
		for (auto &fontRange : options.fontRanges)
		{
			auto range = fontRange.range;
			auto b = fontRange.fontOptions;
			DWRITE_TEXT_RANGE dwriteRange{ (UINT32)range.start, (UINT32)range.length };

			if (a.family != b.family)
			{
				_layout->SetFontFamilyName(b.family.c_str(), dwriteRange);
			}

			if (a.size != b.size)
			{
				_layout->SetFontSize(b.size, dwriteRange);
			}

			if (a.stretch != b.stretch)
			{
				_layout->SetFontStretch(convertFontStretch(b.stretch), dwriteRange);
			}

			if (a.style != b.style)
			{
				_layout->SetFontStyle(convertFontStyle(b.style), dwriteRange);
			}

			if (a.weight != b.weight)
			{
				_layout->SetFontWeight(convertFontWeight(b.weight), dwriteRange);
			}

			if (a.locale != b.locale)
			{
				_layout->SetLocaleName(b.locale.c_str(), dwriteRange);
			}
		}
	}

	DirectWriteBuilder::~DirectWriteBuilder()
	{
		if (_format)
			_format->Release();

		if (_layout)
			_layout->Release();
	}

	Size DirectWriteBuilder::size() const
	{
		DWRITE_TEXT_METRICS metrics;
		_layout->GetMetrics(&metrics);
		return
		{
			static_cast<unsigned>(ceilf(metrics.width)),
			static_cast<unsigned>(ceilf(metrics.height))
		};
	}

	void DirectWriteBuilder::render(DirectWriteImageData &imageData, Rect rect)
	{
		std::vector<ID2D1Brush *> subBrushes;
		for (auto &range : _options.fontRanges)
		{
			if (range.fontOptions.foreground.color.rgba != _options.baseFont.foreground.color.rgba)
			{
				DWRITE_TEXT_RANGE dwriteRange
				{
					(UINT32)range.range.start,
					(UINT32)range.range.length
				};
				auto subBrush = convertBrush(range.fontOptions.foreground, imageData.target());
				subBrushes.push_back(subBrush);
				_layout->SetDrawingEffect(subBrush, dwriteRange);
			}
		}

		D2D1_POINT_2F origin;
		origin.x = static_cast<float>(rect.x);
		origin.y = static_cast<float>(rect.y);

		D2D1_COLOR_F bg = convertColor(_options.background);

		ID2D1Brush *brush = convertBrush(_options.baseFont.foreground, imageData.target());
		D2D1_DRAW_TEXT_OPTIONS options = D2D1_DRAW_TEXT_OPTIONS_NONE;
		D2D1_RECT_F box;
		box.left = origin.x;
		box.top = origin.y;
		box.right = static_cast<float>(rect.width) + box.left;
		box.bottom = static_cast<float>(rect.height) + box.top;

		imageData.target()->BeginDraw();
		imageData.target()->SetTransform(D2D1::Matrix3x2F::Identity());
		imageData.target()->PushAxisAlignedClip(box, D2D1_ANTIALIAS_MODE_ALIASED);
		imageData.target()->Clear(bg);
		imageData.target()->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
		imageData.target()->DrawTextLayout(origin, _layout, brush, options);
		imageData.target()->PopAxisAlignedClip();
		imageData.target()->EndDraw();
		brush->Release();

		for (auto subBrush : subBrushes)
		{
			subBrush->Release();
		}
	}

	// DirectWriteImageData

	DirectWriteImageData::DirectWriteImageData(DirectWriteRenderer &renderer, Size size) :
		_bitmap(nullptr),
		_renderTarget(nullptr),
		_renderer(renderer),
		_size(size),
		_transparentData{ (char)0, (char)0, (char)0, (char)0 },
		_transparentBmp(nullptr)
	{
		renderer.wicFactory()->CreateBitmap(
			renderer.textureSize().width,
			renderer.textureSize().height,
			GUID_WICPixelFormat32bppPRGBA,
			WICBitmapCacheOnLoad,
			&_bitmap);

		auto result = renderer.d2dFactory()->CreateWicBitmapRenderTarget(
			_bitmap,
			D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)),
			&_renderTarget);

		D2D1_SIZE_U transparentBitmapSize;
		transparentBitmapSize.width = 1;
		transparentBitmapSize.height = 1;
		D2D1_BITMAP_PROPERTIES props;
		props.dpiX = 100.0f;
		props.dpiY = 100.0f;
		D2D1_PIXEL_FORMAT format;
		format.format = DXGI_FORMAT_R8G8B8A8_UNORM;
		format.alphaMode = D2D1_ALPHA_MODE_STRAIGHT; //D2D1_ALPHA_MODE_IGNORE; //D2D1_ALPHA_MODE_PREMULTIPLIED;
		props.pixelFormat = format;
		result = _renderTarget->CreateBitmap(transparentBitmapSize, (void *)_transparentData, 4, props, &_transparentBmp);

		_renderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
	}

	DirectWriteImageData::DirectWriteImageData(DirectWriteImageData &&other) :
		_bitmap(other._bitmap),
		_renderTarget(other._renderTarget),
		_renderer(other._renderer),
		_size(other._size),
		_transparentBmp(other._transparentBmp),
		_transparentData{ (char)0, (char)0, (char)0, (char)0 }
	{
		other._bitmap = nullptr;
		other._renderTarget = nullptr;
		other._transparentBmp = nullptr;
	}

	DirectWriteImageData::~DirectWriteImageData()
	{
		if (_transparentBmp)
			_transparentBmp->Release();

		if (_bitmap)
			_bitmap->Release();

		// _renderTarget->Release(); is not called here because it just throws an error saying
		// that it is not implemented.
	}

	void DirectWriteImageData::savePng(LPCWSTR path)
	{
		UINT width, height;
		_bitmap->GetSize(&width, &height);
		IWICBitmapEncoder *encoder = nullptr;
		IWICStream *stream = nullptr;
		IWICBitmapFrameEncode *frame = nullptr;
		IPropertyBag2 *props = nullptr;
		WICPixelFormatGUID pixelFormat;
		WICRect rect;
		rect.Width = width;
		rect.Height = height;
		rect.X = 0;
		rect.Y = 0;
		_bitmap->GetPixelFormat(&pixelFormat);
		_renderer.wicFactory()->CreateEncoder(GUID_ContainerFormatPng, NULL, &encoder);
		_renderer.wicFactory()->CreateStream(&stream);
		stream->InitializeFromFilename(path, GENERIC_WRITE);
		encoder->Initialize(stream, WICBitmapEncoderNoCache);
		encoder->CreateNewFrame(&frame, &props);
		frame->Initialize(props);
		frame->SetSize(width, height);
		frame->SetPixelFormat(&pixelFormat);
		frame->WriteSource(_bitmap, &rect);
		frame->Commit();
		encoder->Commit();
		props->Release();
		frame->Release();
		stream->Release();
		encoder->Release();
	}

	D2D1_COLOR_F convertColor(Color color)
	{
		auto r = color.redf();
		auto g = color.greenf();
		auto b = color.bluef();
		auto a = color.alphaf();
		return D2D1::ColorF(r, g, b, a);
	}

	ID2D1Brush *convertBrush(Brush brush, ID2D1RenderTarget *target)
	{
		ID2D1SolidColorBrush *out;
		auto color = convertColor(brush.color);
		target->CreateSolidColorBrush(color, &out);
		return out;
	}

	DWRITE_FONT_WEIGHT convertFontWeight(FontWeight weight)
	{
		switch (weight)
		{
		case FontWeight::Thin:
			return DWRITE_FONT_WEIGHT_THIN;
		case FontWeight::ExtraLight:
			return DWRITE_FONT_WEIGHT_EXTRA_LIGHT;
		case FontWeight::Light:
			return DWRITE_FONT_WEIGHT_LIGHT;
		case FontWeight::SemiLight:
			return DWRITE_FONT_WEIGHT_SEMI_LIGHT;
		case FontWeight::Normal:
			return DWRITE_FONT_WEIGHT_NORMAL;
		case FontWeight::Medium:
			return DWRITE_FONT_WEIGHT_MEDIUM;
		case FontWeight::SemiBold:
			return DWRITE_FONT_WEIGHT_SEMI_BOLD;
		case FontWeight::Bold:
			return DWRITE_FONT_WEIGHT_BOLD;
		case FontWeight::ExtraBold:
			return DWRITE_FONT_WEIGHT_EXTRA_BOLD;
		case FontWeight::Black:
			return DWRITE_FONT_WEIGHT_BLACK;
		case FontWeight::ExtraBlack:
			return DWRITE_FONT_WEIGHT_EXTRA_BLACK;
		default:
			return DWRITE_FONT_WEIGHT_NORMAL;
		}
	}

	DWRITE_FONT_STYLE convertFontStyle(FontStyle style)
	{
		switch (style)
		{
		case FontStyle::Normal:
			return DWRITE_FONT_STYLE_NORMAL;
		case FontStyle::Italic:
			return DWRITE_FONT_STYLE_ITALIC;
		case FontStyle::Oblique:
			return DWRITE_FONT_STYLE_OBLIQUE;
		default:
			return DWRITE_FONT_STYLE_NORMAL;
		}
	}

	DWRITE_FONT_STRETCH convertFontStretch(FontStretch stretch)
	{
		switch (stretch)
		{
		case FontStretch::Undefined:
			return DWRITE_FONT_STRETCH_UNDEFINED;
		case FontStretch::UltraCondensed:
			return DWRITE_FONT_STRETCH_ULTRA_CONDENSED;
		case FontStretch::ExtraCondensed:
			return DWRITE_FONT_STRETCH_EXTRA_CONDENSED;
		case FontStretch::Condensed:
			return DWRITE_FONT_STRETCH_CONDENSED;
		case FontStretch::SemiCondensed:
			return DWRITE_FONT_STRETCH_SEMI_CONDENSED;
		case FontStretch::Normal:
			return DWRITE_FONT_STRETCH_NORMAL;
		case FontStretch::Medium:
			return DWRITE_FONT_STRETCH_MEDIUM;
		case FontStretch::SemiExpanded:
			return DWRITE_FONT_STRETCH_SEMI_EXPANDED;
		case FontStretch::Expanded:
			return DWRITE_FONT_STRETCH_EXPANDED;
		case FontStretch::ExtraExpanded:
			return DWRITE_FONT_STRETCH_EXTRA_EXPANDED;
		case FontStretch::UltraExpanded:
			return DWRITE_FONT_STRETCH_ULTRA_EXPANDED;
		default:
			return DWRITE_FONT_STRETCH_NORMAL;
		}
	}

	D2D1_TEXT_ANTIALIAS_MODE convertAntialiasMode(AntialiasMode mode)
	{
		switch (mode)
		{
		case AntialiasMode::Grayscale:
			return D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE;
		case AntialiasMode::None:
			return D2D1_TEXT_ANTIALIAS_MODE_ALIASED;
		case AntialiasMode::SubPixel:
			return D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE;
		default:
			return D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE;
		}
	}
}
