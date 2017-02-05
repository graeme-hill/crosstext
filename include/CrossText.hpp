#pragma once

#include "Common.hpp"
#include <functional>
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <stack>
#include <vector>

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

	struct StyleRange
	{
		Style style;
		Range range;
	};

	struct TextOptions
	{
		Style baseStyle;
		AntialiasMode antialiasMode;
		std::vector<StyleRange> styleRanges;
		Color background;

		inline static TextOptions fromStyle(Style base)
		{
			return{ base, AntialiasMode::Grayscale, {}, 0x00000000 };
		}

		TextOptions withStyle(Style newBaseStyle)
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

		TextOptions withStyleRanges(std::vector<StyleRange> newRanges)
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
		unsigned int y;
		unsigned int *count;
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
		void increment(unsigned int y);
		void decrement(unsigned int y);
		void withYValuesInPriorityOrder(std::function<bool(int y)> callback);

	private:
		std::vector<unsigned int> _yCounts;
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

		std::vector<uint64_t> _slotIndexes;
		Size _size;
		uint64_t _nextIndex;
		std::unordered_map<uint64_t, Slot> _slotMap;
		SpacialIndex _spacialIndex;
		YCache _yCache;
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
		TOptions &options() { return _options; }

		TSysContext &sysContext() { return _sysContext; }
		std::vector<Texture> &textures() { return _textures; }

	private:
		std::vector<Texture> _textures;
		TSysContext _sysContext;
		unsigned int _lastUsed;
		TOptions _options;
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
		TextBlockMetrics calcMetrics(std::wstring &text);
		void render(std::wstring &text, Placement placement);

		template <typename THandler>
		void walk(std::wstring &text, THandler &handler);

		void dispose();
		inline bool foundPlacement() { return _placement.texture != nullptr; };
		inline bool dead() { return _manager == nullptr; }

		TextManager *_manager;
		TextOptions _options;
		Placement _placement;
	};

	template <typename THandler>
	void TextBlock::walk(std::wstring &text, THandler &handler)
	{
		std::stack<StyleRange> rangeStack;
		rangeStack.push({
			_options.baseStyle,
			{ 0, static_cast<unsigned int>(text.size()) }
		});
		unsigned int nextRangeIndex = 0;

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
}
