#pragma once

#include "Common.hpp"

#ifdef _WINDOWS
#include "DirectWrite.hpp"
namespace ct
{
	typedef DirectWriteRenderOptions TRenderOptions;
	typedef DirectWriteBuilder TBuilder;
	typedef DirectWriteRenderer TRenderer;
	typedef DirectWriteImageData TImageData;
	typedef DirectWriteFont TSystemFont;
	//typedef DirectWriteBrush TSystemBrush;
}
#endif

#define OPENING_MIN_HEIGHT 1.0f
#define OPENING_MIN_WIDTH 1.0f

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
		Slot(Rect rect, unsigned long index) :
			_rect(rect), _index(index)
		{ }
		Rect rect() { return _rect; }
		unsigned long index() { return _index; }
	private:
		Rect _rect;
		unsigned long _index;
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

	class RectangleOrganizer
	{
	public:
		RectangleOrganizer(Size size);
		RectangleOrganizer(const RectangleOrganizer &) = delete;
		RectangleOrganizer(RectangleOrganizer &&);
		SlotSearchResult tryClaimSlot(Size size);
		bool releaseSlot(unsigned long index);

	private:
		bool isRectOpen(Rect &rect);
		std::vector<int> findXOptions(int y);
		bool checkOverlap(Rect &a, Rect &b);
		SlotSearchResult search(int y, Size size);
		SlotSearchResult search(Slot &slot, Size size);

		std::vector<Slot> _slots;
		Size _size;
		unsigned long _nextIndex;
	};

	class TextManager
	{
	public:
		TextManager(TRenderOptions renderOptions);
		TextManager(const TextManager &) = delete;
		TextManager(TextManager &&) = delete;
		Placement findPlacement(Size size);
		void releaseRect(Texture &texture, unsigned long index);

		TRenderer &renderer() { return _renderer; }

	private:
		std::vector<Texture> _textures;
		TRenderer _renderer;
	};

	class TextBlock
	{
	public:
		TextBlock(TextManager &manager, Text text, Font &font, Brush brush);
		TextBlock(const TextBlock &) = delete;
		TextBlock(TextBlock &&);
		~TextBlock();
		Texture *texture() { return _texture; }

	private:
		TextManager &_manager;
		Texture *_texture;
		Slot _slot;
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

	class Font
	{
	public:
		Font(TextManager &manager, FontOptions options);
		TSystemFont &systemFont() { return _systemFont; }

	private:
		TSystemFont _systemFont;
	};
}

