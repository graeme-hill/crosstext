#pragma once

#include "Common.hpp"
#include <functional>
#include <unordered_map>

#ifdef OS_WINDOWS
#include "DirectWrite.hpp"
namespace ct
{
	typedef DirectWriteRenderOptions TRenderOptions;
	typedef DirectWriteBuilder TBuilder;
	typedef DirectWriteRenderer TRenderer;
	typedef DirectWriteImageData TImageData;
	typedef WindowsTimer TTimer;
}
#endif

#ifdef OS_LINUX
#include "FreeType.hpp"
namespace ct
{
	typedef FreeTypeOptions TOptions;
	typedef FreeTypeSysContext TSysContext;
	typedef FreeTypeImageData TImageData;
	typedef FreeTypeFont TFont;
	typedef FreeTypeMetricBuilder TMetricBuilder;
	typedef FreeTypeCharRenderer TCharRenderer;
	typedef LinuxTimer TTimer;
}
#endif

#define OPENING_MIN_HEIGHT 1.0f
#define OPENING_MIN_WIDTH 1.0f
#define SPACIAL_INDEX_BLOCK_WIDTH 128
#define SPACIAL_INDED_BLOCK_HEIGHT 16

namespace ct
{
	struct Style
	{
		const TFont *font;
		float size;
		Brush foreground;

		Style withFont(const TSysFont *font)
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

	struct StyleRange
	{
		Style style;
		Range range;
	};

	struct TextOptions
	{
		Font baseFont;
		AntialiasMode antialiasMode;
		std::vector<StyleRange> styleRanges;
		Color background;

		inline static TextOptions fromFont(Font base)
		{
			return{ base, AntialiasMode::Grayscale, {}, 0x00000000 };
		}

		TextOptions withFont(Font newBaseFont)
		{
			TextOptions opts(*this);
			opts.baseFont = newBaseFont;
			return opts;
		}

		TextOptions withAntialiasMode(AntialiasMode newAntialiasMode)
		{
			TextOptions opts(*this);
			opts.antialiasMode = newAntialiasMode;
			return opts;
		}

		TextOptions withFontRanges(std::vector<FontRange> newRanges)
		{
			TextOptions opts(*this);
			opts.fontRanges = newRanges;
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
		int y;
		int *count;
	};

	class SpacialIndex
	{
	public:
		SpacialIndex(Size size, Size blockSize);
		SpacialIndex(const SpacialIndex &other) = delete;
		SpacialIndex(SpacialIndex &&other);

		void add(Slot slot);

		void remove(Slot slot);

		bool withNearBlocks(Rect rect, std::function<bool(std::vector<uint64_t> &)> action);

		bool withNearSlots(Rect rect, std::function<bool(uint64_t)> action);

		bool withSlotsOnYLine(int y, std::function<bool(uint64_t)> action);

		bool withSlotsInBlockRange(
			int leftColumn,
			int rightColumn,
			int topRow,
			int bottomRow,
			std::function<bool(uint64_t)> action);

		bool withBlocksInRange(
			int leftColumn,
			int rightColumn,
			int topRow,
			int bottomRow,
			std::function<bool(std::vector<uint64_t> &)> action);

	private:
		Size _blockSize;
		int _xBlocks;
		int _yBlocks;
		std::vector<std::vector<uint64_t>> _data;

		int getBlockIndex(int x, int y);
		static int calcBlockCount(int totalSize, int blockSize);
	};

	class YCache
	{
	public:
		YCache(int height);
		YCache(const YCache &other) = delete;
		YCache(YCache &&other);
		void increment(int y);
		void decrement(int y);
		void withYValuesInPriorityOrder(std::function<bool(int y)> callback);

	private:
		std::vector<int> _yCounts;
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
		void withXOptions(int y, std::function<bool(int)> callback);
		bool checkOverlap(Rect a, Rect b);
		SlotSearchResult search(int y, Size size);
		void addSlot(Slot slot);
		void removeSlot(uint64_t slotIndex);
		bool empty();
		uint64_t nextIndex() { return _nextIndex++; }

		YCache _yCache;
		std::unordered_map<uint64_t, Slot> _slotMap;
		std::vector<uint64_t> _slotIndexes;
		Size _size;
		uint64_t _nextIndex;
		SpacialIndex _spacialIndex;
		std::unordered_map<int, bool> _usedXOptions;
		bool _moved;
	};

	class Texture
	{
	public:
		Texture(TImageData imageData);
		Texture(const Texture &) = delete;
		Texture(Texture &&);
		TImageData &imageData() { return _imageData; }
		RectangleOrganizer &organizer() { return _organizer; }

	private:
		TImageData _imageData;
		RectangleOrganizer _organizer;
	};

	struct Placement
	{
		bool isFound;
		Slot slot;
		Texture *texture;

		static inline Placement notFound()
		{
			return{ false, {0}, nullptr };
		}

		static inline Placement found(Slot slot_, Texture *texture_)
		{
			return{ true, slot_, texture_ };
		}
	};

	class TextManager
	{
	public:
		TextManager(TOptions options);
		TextManager(const TextManager &) = delete;
		TextManager(TextManager &&) = delete;
		Placement findPlacement(Size size);
		void releaseRect(Texture *texture, Slot slot);
		TFont loadFont(std::string path);

		TSysContext &sysContext() { return _sysContext; }
		std::vector<Texture> &textures() { return _textures; }

	private:
		std::vector<Texture> _textures;
		TSysContext _sysContext;
		int _lastUsed;
	};

	class TextBlock
	{
	public:
		TextBlock(
			TextManager &manager,
			std::wstring text,
			TextOptions options);

		TextBlock(const TextBlock &) = delete;
		TextBlock(TextBlock &&);
		TextBlock &operator=(const TextBlock &other) = delete;
		TextBlock &operator=(TextBlock &&other);
		~TextBlock();
		Texture *texture() { return _placement.texture; }

	private:
		static Placement initPlacement(
			TextManager &manager,
			std::wstring &text,
			TextOptions options);
		Size calcSize(std::wstring &text, TextOptions options);
		void render(
			std::wstring &text,
			TextOptions options,
			Placement placement);
		void walk(
			std::wstring &text,
			TextOptions &options,
			std::function<void(char_t, Style)> action);
		void dispose();
		inline bool foundPlacement() { return _placement.texture != nullptr; };
		inline bool dead() { return _manager == nullptr; }

		TextManager *_manager;
		Placement _placement;
		TextOptions _options;
	};
}
