#pragma once

#include <functional>
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <stack>
#include <vector>

#define SPACIAL_INDEX_BLOCK_WIDTH 128
#define SPACIAL_INDED_BLOCK_HEIGHT 16

#define BEGIN_NAMESPACE namespace ct {
#define END_NAMESPACE }

BEGIN_NAMESPACE

struct Size
{
	unsigned width;
	unsigned height;
};

struct LineMetrics
{
	unsigned height;
	unsigned baseline;
	unsigned chars;
};

struct TextManagerOptions
{
	Size textureSize;
};

struct TextBlockMetrics
{
	Size size;
	std::vector<LineMetrics> lines;
};

struct Rect
{
	unsigned x;
	unsigned y;
	unsigned width;
	unsigned height;

	inline unsigned endX()
	{
		return x + width - 1;
	}

	inline unsigned endY()
	{
		return y + height - 1;
	}
};

struct Color
{
	uint32_t rgba;

	uint8_t redByte()
	{
		return static_cast<uint8_t>(rgba >> 24);
	}

	uint8_t greenByte()
	{
		return static_cast<uint8_t>((rgba & 0x00ff0000) >> 16);
	}

	uint8_t blueByte()
	{
		return static_cast<uint8_t>((rgba & 0x0000ff00) >> 0);
	}

	uint8_t alphaByte()
	{
		return static_cast<uint8_t>(rgba & 0x000000ff);
	}

	float redf()
	{
		return redByte() / 255.0f;
	}

	float greenf()
	{
		return greenByte() / 255.0f;
	}

	float bluef()
	{
		return blueByte() / 255.0f;
	}

	float alphaf()
	{
		return alphaByte() / 255.0f;
	}
};

enum class FontWeight
{
	Thin = 100,
	ExtraLight = 200,
	Light = 300,
	SemiLight = 350,
	Normal = 400,
	Medium = 500,
	SemiBold = 600,
	Bold = 700,
	ExtraBold = 800,
	Black = 900,
	ExtraBlack = 950
};

enum class FontStyle
{
	Normal,
	Oblique,
	Italic
};

enum class FontStretch
{
	Undefined,
	UltraCondensed,
	ExtraCondensed,
	Condensed,
	SemiCondensed,
	Normal,
	Medium,
	SemiExpanded,
	Expanded,
	ExtraExpanded,
	UltraExpanded
};

enum class AntialiasMode
{
	None,
	Grayscale,
	SubPixel
};

struct Range
{
	unsigned start;
	unsigned length;

	unsigned last() const
	{
		return start + length - 1;
	}
};

struct Brush
{
	Color color;
};

template <typename TFont>
struct Style
{
	TFont *font;
	float size;
	Brush foreground;

	Style withFont(TFont *font)
	{
		Style newStyle(*this);
		newStyle.font = font;
		return newStyle;
	}

	Style withSize(float newSize)
	{
		Style newStyle(*this);
		newStyle.size = newSize;
		return newStyle;
	}

	Style withForeground(Brush newForeground)
	{
		Style newStyle(*this);
		newStyle.foreground = newForeground;
		return newStyle;
	}
};

template <typename TFont>
struct StyleRange
{
	Style<TFont> style;
	Range range;
};

template <typename TFont>
struct TextOptions
{
	Style<TFont> baseStyle;
	AntialiasMode antialiasMode;
	std::vector<StyleRange<TFont>> styleRanges;
	Color background;

	inline static TextOptions fromStyle(Style<TFont> base)
	{
		return{ base, AntialiasMode::Grayscale, {}, 0x00000000 };
	}

	TextOptions withStyle(Style<TFont> newBaseStyle)
	{
		TextOptions opts(*this);
		opts.baseStyle = newBaseStyle;
		return opts;
	}

	TextOptions withAntialiasMode(AntialiasMode newAntialiasMode)
	{
		TextOptions opts(*this);
		opts.antialiasMode = newAntialiasMode;
		return opts;
	}

	TextOptions withStyleRanges(std::vector<StyleRange<TFont>> newRanges)
	{
		TextOptions opts(*this);
		opts.styleRanges = newRanges;
		return opts;
	}

	TextOptions withBackground(Color newBackground)
	{
		TextOptions opts(*this);
		opts.background = newBackground;
		return opts;
	}
};

struct Slot
{
	Rect rect;
	uint64_t index;
};

struct SlotSearchResult
{
	bool isFound;
	Slot slot;

	static inline SlotSearchResult notFound()
	{
		return{ false, { 0 } };
	}

	static inline SlotSearchResult found(Slot slot)
	{
		return{ true, slot };
	}
};

struct YCount
{
	unsigned y;
	unsigned *count;
};

class SpacialIndex
{
public:
	SpacialIndex(Size size, Size blockSize);
	SpacialIndex(const SpacialIndex &other) = delete;
	SpacialIndex(SpacialIndex &&other);

	void add(Slot slot);

	void remove(Slot slot);

	bool withNearBlocks(
		Rect rect,
		std::function<bool(std::vector<uint64_t> &)> action);

	bool withNearSlots(
		Rect rect,
		std::function<bool(uint64_t)> action);

	bool withSlotsOnYLine(
		unsigned y,
		std::function<bool(uint64_t)> action);

	bool withSlotsInBlockRange(
		unsigned leftColumn,
		unsigned rightColumn,
		unsigned topRow,
		unsigned bottomRow,
		std::function<bool(uint64_t)> action);

	bool withBlocksInRange(
		unsigned leftColumn,
		unsigned rightColumn,
		unsigned topRow,
		unsigned bottomRow,
		std::function<bool(std::vector<uint64_t> &)> action);

private:
	Size _blockSize;
	unsigned _xBlocks;
	unsigned _yBlocks;
	std::vector<std::vector<uint64_t>> _data;

	unsigned getBlockIndex(unsigned x, unsigned y);
	static unsigned calcBlockCount(unsigned totalSize, unsigned blockSize);
};

class YCache
{
public:
	YCache(unsigned height);
	YCache(const YCache &other) = delete;
	YCache(YCache &&other);
	void increment(unsigned y);
	void decrement(unsigned y);
	void withYValuesInPriorityOrder(std::function<bool(unsigned y)> callback);

private:
	std::vector<unsigned> _yCounts;
	std::vector<YCount> _yCountPriority;
};

class RectangleOrganizer
{
public:
	RectangleOrganizer(Size size);
	RectangleOrganizer(const RectangleOrganizer &) = delete;
	RectangleOrganizer(RectangleOrganizer &&);
	SlotSearchResult tryClaimSlot(Size size);
	bool releaseSlot(uint64_t index);

private:
	bool isRectOpen(Rect &rect);
	void withXOptions(unsigned y, std::function<bool(unsigned)> callback);
	bool checkOverlap(Rect a, Rect b);
	SlotSearchResult search(unsigned y, Size size);
	void addSlot(Slot slot);
	void removeSlot(uint64_t slotIndex);
	bool empty();
	uint64_t nextIndex() { return _nextIndex++; }

	std::vector<uint64_t> _slotIndexes;
	Size _size;
	uint64_t _nextIndex;
	std::unordered_map<uint64_t, Slot> _slotMap;
	SpacialIndex _spacialIndex;
	YCache _yCache;
	std::unordered_map<unsigned, bool> _usedXOptions;
	bool _moved;
};

template <typename TImageData>
class Texture
{
public:
	Texture(TImageData imageData) :
		_imageData(std::move(imageData)), _organizer(imageData.size())
	{ }

	Texture(const Texture &) = delete;

	Texture(Texture &&other) :
		_imageData(std::move(other._imageData)),
		_organizer(std::move(other._organizer))
	{ }

	TImageData &imageData() { return _imageData; }
	RectangleOrganizer &organizer() { return _organizer; }

private:
	TImageData _imageData;
	RectangleOrganizer _organizer;
};

template <typename TImageData>
struct Placement
{
	bool isFound;
	Slot slot;
	Texture<TImageData> *texture;

	static Placement notFound()
	{
		return{ false, {0}, nullptr };
	}

	static Placement found(Slot slot_, Texture<TImageData> *texture_)
	{
		return{ true, slot_, texture_ };
	}
};

template <typename TText>
class TextManager
{
public:
	using TImageData = typename TText::ImageData;
	using TSysContext = typename TText::SysContext;
	using TFont = typename TText::Font;

	TextManager(
		TextManagerOptions options,
		std::vector<typename TText::ImageData> textures) :
		_sysContext(TSysContext(options)),
		_lastUsed(0),
		_options(options)
	{
		for (auto &tex : textures)
		{
			_textures.push_back(Texture<TImageData>(std::move(tex)));
		}
	}

	TextManager(const TextManager &) = delete;
	TextManager(TextManager &&) = delete;

	Placement<TImageData> findPlacement(Size size)
	{
		auto &lastUsedTexture = _textures[_lastUsed];
		auto firstResult = lastUsedTexture.organizer().tryClaimSlot(size);
		if (firstResult.isFound)
		{
			return Placement<TImageData>::found(
				firstResult.slot, &_textures.at(_lastUsed));
		}

		for (unsigned i = 0; i < _textures.size(); i++)
		{
			if (i == _lastUsed)
			{
				continue;
			}

			auto result = _textures[i].organizer().tryClaimSlot(size);
			if (result.isFound)
			{
				// found a place for the text block :)
				_lastUsed = i;
				return Placement<TImageData>::found(
					result.slot, &_textures[i]);
			}
		}

		// there is nowhere that can fit a text block of this size :(
		return Placement<TImageData>::notFound();
	}

	void releaseRect(Texture<TImageData> *texture, Slot slot)
	{
		texture->organizer().releaseSlot(slot.index);
	}

	TFont loadFont(std::string path)
	{
		return TFont(path, _sysContext);
	}

	TextManagerOptions &options() { return _options; }

	TSysContext &sysContext() { return _sysContext; }
	std::vector<Texture<TImageData>> &textures() { return _textures; }

private:

	std::vector<Texture<TImageData>> _textures;
	TSysContext _sysContext;
	unsigned _lastUsed;
	TextManagerOptions _options;
};

template <typename TText>
class TextBlock
{
public:
	using TFont = typename TText::Font;
	using TImageData = typename TText::ImageData;
	using TMetricBuilder = typename TText::MetricBuilder;
	using TCharRenderer = typename TText::CharRenderer;

	TextBlock(
		TextManager<TText> &manager,
		std::wstring text,
		TextOptions<TFont> options) :
		_manager(&manager),
		_options(options),
		_placement{0}
	{
		// Make sure ranges are not out of order
		std::sort(
			_options.styleRanges.begin(),
			_options.styleRanges.end(),
			[](StyleRange<TFont> a, StyleRange<TFont> b)
			{
				return a.range.start < b.range.start;
			});

		// Calculate how much space it will take up so we know where it fits
		TextBlockMetrics metrics = calcMetrics(text);
		Size size = metrics.size;

		// Find a spot (or not)
		_placement = _manager->findPlacement(size);

		std::cout
			<< "placement: " << _placement.slot.rect.x << ","
			<< _placement.slot.rect.y << ","
			<< _placement.slot.rect.width << ","
			<< _placement.slot.rect.height << std::endl;

		// Render the characters to the texture if a spot was found`
		if (_placement.isFound)
		{
			render(text, _placement, metrics);
		}
	}

	TextBlock(const TextBlock &) = delete;

	TextBlock(TextBlock &&other) :
		_manager(other._manager),
		_options(other._options),
		_placement(other._placement)
	{
		other._manager = nullptr;
	}

	TextBlock &operator=(const TextBlock &other) = delete;

	TextBlock &operator=(TextBlock &&other)
	{
		dispose();
		_manager = other._manager;
		_placement = other._placement;
		_options = other._options;
		other._manager = nullptr;
		return *this;
	}

	~TextBlock()
	{
		dispose();
	}

	Texture<TImageData> *texture() { return _placement.texture; }

private:
	TextBlockMetrics calcMetrics(std::wstring &text)
	{
		auto maxSize = _manager->options().textureSize;
		TMetricBuilder metricBuilder(_manager->sysContext(), maxSize);

		walk(text, metricBuilder);

		return metricBuilder.done();
	}

	void render(
		std::wstring &text,
		Placement<TImageData> placement,
		TextBlockMetrics &metrics)
	{
		if (!placement.isFound)
			return;

		TCharRenderer charRenderer(
			_manager->sysContext(),
			placement.texture->imageData(),
			placement.slot.rect,
			metrics);

		walk(text, charRenderer);

		placement.texture->imageData().commit();
	}

	template <typename THandler>
	void walk(std::wstring &text, THandler &handler)
	{
		std::stack<StyleRange<TFont>> rangeStack;
		rangeStack.push({
			_options.baseStyle,
			{ 0, static_cast<unsigned>(text.size()) }
		});
		unsigned nextRangeIndex = 0;

		bool newStyle = true;
		for (size_t i = 0; i < text.size(); i++)
		{
			if (nextRangeIndex < _options.styleRanges.size()
				&& _options.styleRanges[nextRangeIndex].range.start == i)
			{
				rangeStack.push(_options.styleRanges[nextRangeIndex++]);
				newStyle = true;
			}

			auto style = rangeStack.top().style;
			if (newStyle)
			{
				handler.onStyleChange(style.font, style.size, style.foreground);
			}

			handler.onChar(text[i], style.font, style.size, style.foreground);

			newStyle = false;

			while (!rangeStack.empty() &&rangeStack.top().range.last() <= i)
			{
				rangeStack.pop();
				newStyle = true;
			}
		}
	}

	void dispose()
	{
		if (!dead() && foundPlacement())
		{
			_manager->releaseRect(_placement.texture, _placement.slot);
		}
	}

	bool foundPlacement()
	{
		return _placement.texture != nullptr;
	};

	bool dead()
	{
		return _manager == nullptr;
	}

	TextManager<TText> *_manager;
	TextOptions<TFont> _options;
	Placement<TImageData> _placement;
};

template <typename TTextSystem>
class TextPlatform
{
public:
	using ImageData = typename TTextSystem::ImageData;
	using Font = typename TTextSystem::Font;
	using Manager = TextManager<TextPlatform<TTextSystem>>;
	using Block = TextBlock<TextPlatform<TTextSystem>>;
	using Style = ct::Style<Font>;
	using Options = TextOptions<Font>;

private:
	using SysContext = typename TTextSystem::SysContext;
	using MetricBuilder = typename TTextSystem::MetricBuilder;
	using CharRenderer = typename TTextSystem::CharRenderer;

	friend class TextManager<TextPlatform<TTextSystem>>;
	friend class TextBlock<TextPlatform<TTextSystem>>;
};

struct CharLayout
{
	wchar_t ch;
	Size size;
	unsigned line;
};

class TextLayout
{
public:
	TextLayout(Size maxSize);
	void nextChar(wchar_t ch, Size charSize, unsigned kerning);
	TextBlockMetrics metrics();

private:
	void checkWrap(wchar_t ch);
	void wrapLastWord();
	bool isWhitespace(wchar_t ch);
	bool isWordDivider(wchar_t ch);
	bool isFirstCharOnLine(unsigned index);
	unsigned getWrapCharCount();

	Size _size;
	std::vector<CharLayout> _chars;
	unsigned _lastLine;
	unsigned _penX;
	unsigned _currentLine;
};

END_NAMESPACE
