#pragma once

#include "Common.hpp"

#ifdef OS_WINDOWS
#include "DirectWrite.hpp"
namespace ct
{
	typedef DirectWriteRenderOptions TRenderOptions;
	typedef DirectWriteBuilder TBuilder;
	typedef DirectWriteRenderer TRenderer;
	typedef DirectWriteImageData TImageData;
}
#endif

#define OPENING_MIN_HEIGHT 1.0f
#define OPENING_MIN_WIDTH 1.0f
#define SPACIAL_INDEX_BLOCK_SIZE 64

namespace ct
{
	class TextManager;
	class TextBlock;
	class Texture;
	class Placement;
	class Font;
	class SlotSearchResult;
	class Slot;
	class RectangleOrganizer;

	class Slot
	{
	public:
		Slot() :
			Slot(Rect(), 0)
		{ }
		Slot(Rect rect, uint64_t index) :
			_rect(rect), _index(index)
		{ }
		Rect rect() { return _rect; }
		uint64_t index() { return _index; }
	private:
		Rect _rect;
		uint64_t _index;
	};

	class SlotSearchResult
	{
	public:
		static SlotSearchResult notFound();
		static SlotSearchResult found(Slot slot);
		bool isFound() { return _isFound; }
		Slot slot() { return _slot; }

	private:
		SlotSearchResult(bool found, Slot slot) :
			_isFound(found), _slot(slot)
		{ }

		bool _isFound;
		Slot _slot;
	};

	class SpacialSlotIndex
	{
	public:
		SpacialSlotIndex(Size size, unsigned int blockSize);
		SpacialSlotIndex(const SpacialSlotIndex &other) = delete;
		SpacialSlotIndex(SpacialSlotIndex &&other) = delete;
		void add(Slot slot);
		void remove(Slot slot);

	private:
		unsigned int _blockSize;
		unsigned int _xBlocks;
		unsigned int _yBlocks;
		std::vector<std::vector<uint64_t>> _data;

		unsigned int getBlockIndex(unsigned int x, unsigned int y);
		void removeSlotIndex(std::vector<uint64_t> &indexes, uint64_t slotIndex);
		static unsigned int calcBlockCount(unsigned int totalSize, unsigned int blockSize);
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
		std::vector<int> findXOptions(int y);
		bool checkOverlap(Rect &a, Rect &b);
		SlotSearchResult search(int y, Size size);
		SlotSearchResult search(Slot &slot, Size size);

		std::vector<Slot> _slots;
		Size _size;
		uint64_t _nextIndex;
		SpacialSlotIndex _spacialIndex;
	};

	class TextManager
	{
	public:
		TextManager(TRenderOptions renderOptions);
		TextManager(const TextManager &) = delete;
		TextManager(TextManager &&) = delete;
		Placement findPlacement(Size size);
		void releaseRect(Texture &texture, Slot slot);

		TRenderer &renderer() { return _renderer; }
		std::vector<Texture> &textures() { return _textures; }

	private:
		std::vector<Texture> _textures;
		TRenderer _renderer;
	};

	class TextBlock
	{
	public:
		TextBlock(TextManager &manager, std::wstring text, FontOptions font);

		TextBlock(TextManager &manager, std::wstring text, FontOptions font, std::vector<FontRange> fontRanges);

		TextBlock(const TextBlock &) = delete;
		TextBlock(TextBlock &&);
		~TextBlock();
		Texture *texture() { return _texture; }

	private:
		TextManager &_manager;
		Texture *_texture;
		Slot _slot;
		std::vector<FontRange> _fontRanges;
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

	class Placement
	{
	public:
		Placement(bool isFound, Texture &texture, Slot slot);
		Texture &texture() { return _texture; }
		Slot slot() const { return _slot; }
		bool isFound() const { return _isFound; }

	private:
		Texture &_texture;
		Slot _slot;
		bool _isFound;
	};

	class Timer
	{
	public:
		Timer()
		{
			QueryPerformanceFrequency(&_frequency);
			QueryPerformanceCounter(&_start);
		}

		double millis()
		{
			LARGE_INTEGER now;
			QueryPerformanceCounter(&now);
			return (now.QuadPart - _start.QuadPart) * 1000.0 / _frequency.QuadPart;
		}

	private:
		LARGE_INTEGER _frequency;
		LARGE_INTEGER _start;
	};
}

