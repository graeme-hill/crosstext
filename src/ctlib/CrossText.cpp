#include "CrossText.hpp"
#include <iostream>
#include <algorithm>

namespace ct
{
	SlotSearchResult SlotSearchResult::notFound()
	{
		return SlotSearchResult(false, Slot(Rect(), 0));
	}

	SlotSearchResult SlotSearchResult::found(Slot slot)
	{
		return SlotSearchResult(true, slot);
	}

	TextManager::TextManager(TRenderOptions renderOptions) :
		_renderer(TRenderer(renderOptions))
	{
		for (unsigned int i = 0; i < renderOptions.textureCount(); i++)
		{
			_textures.push_back(Texture(TImageData(_renderer, renderOptions.textureSize())));
		}
	}

	Placement TextManager::findPlacement(Size size)
	{
		for (unsigned int i = 0; i < _textures.size(); i++)
		{
			auto result = _textures[i].organizer().tryClaimSlot(size);
			if (result.isFound())
			{
				// found a place for the text block :)
				return Placement(true, _textures[i], result.slot());
			}
		}

		// there is nowhere that can fit a text block of this size :(
		return Placement(false, _textures[0], Slot());
	}

	void TextManager::releaseRect(Texture &texture, Slot slot)
	{
		texture.imageData().clearRect(slot.rect());
		texture.organizer().releaseSlot(slot.index());
	}

	TextBlock::TextBlock(TextManager &manager, std::wstring text, FontOptions font) :
		TextBlock(manager, text, font, std::vector<FontRange>())
	{ }

	TextBlock::TextBlock(
		TextManager &manager, std::wstring text, FontOptions font, std::vector<FontRange> fontRanges) :
		_manager(manager), _texture(nullptr), _fontRanges(fontRanges)
	{
		TBuilder builder(manager.renderer(), text, font, fontRanges);
		auto size = builder.size();
		Timer t;
		auto placement = manager.findPlacement(size);
		auto x = t.millis();
		std::cout << x << std::endl;
		_slot = placement.slot();
		builder.render(placement.texture().imageData(), _slot.rect());
		_texture = &placement.texture();
	}

	TextBlock::TextBlock(TextBlock &&other) :
		_manager(other._manager),
		_texture(other._texture),
		_slot(other._slot),
		_fontRanges(other._fontRanges)
	{
		other._texture = nullptr;
	}

	TextBlock::~TextBlock()
	{
		if (_texture)
		{
			_manager.releaseRect(*_texture, _slot);
		}
	}

	Texture::Texture(TImageData imageData) :
		_imageData(std::move(imageData)), _organizer(imageData.size())
	{ }

	Texture::Texture(Texture &&other) :
		_imageData(std::move(other._imageData)),
		_organizer(std::move(other._organizer))
	{ }

	Placement::Placement(bool isFound, Texture &texture, Slot slot) :
		_texture(texture), _slot(slot), _isFound(isFound)
	{ }

	SpacialSlotIndex::SpacialSlotIndex(Size size, unsigned int blockSize) :
		_blockSize(blockSize),
		_xBlocks(calcBlockCount(size.width(), blockSize)),
		_yBlocks(calcBlockCount(size.height(), blockSize)),
		_data(_xBlocks * _yBlocks)
	{ }

	void SpacialSlotIndex::add(Slot slot)
	{
		auto leftColumn = slot.rect().x() / _blockSize;
		auto rightColumn = slot.rect().endX() / _blockSize;
		auto topRow = slot.rect().y() / _blockSize;
		auto bottomRow = slot.rect().endY() / _blockSize;

		for (unsigned int col = leftColumn; col <= rightColumn; col++)
		{
			for (unsigned int row = topRow; row <= bottomRow; row++)
			{
				auto index = row * _xBlocks + col;
				_data[index].push_back(slot.index());
			}
		}
	}

	void SpacialSlotIndex::remove(Slot slot)
	{
		auto leftColumn = slot.rect().x() / _blockSize;
		auto rightColumn = slot.rect().endX() / _blockSize;
		auto topRow = slot.rect().y() / _blockSize;
		auto bottomRow = slot.rect().endY() / _blockSize;

		for (unsigned int col = leftColumn; col <= rightColumn; col++)
		{
			for (unsigned int row = topRow; row <= bottomRow; row++)
			{
				auto index = row * _xBlocks + col;
				removeSlotIndex(_data[index], slot.index());
			}
		}
	}

	unsigned int SpacialSlotIndex::getBlockIndex(unsigned int x, unsigned int y)
	{
		auto xBlock = x / _blockSize;
		auto yBlock = y / _blockSize;

		return yBlock * _xBlocks + xBlock;
	}

	void SpacialSlotIndex::removeSlotIndex(std::vector<uint64_t> &indexes, uint64_t slotIndex)
	{
		indexes.erase(std::remove(indexes.begin(), indexes.end(), slotIndex));
	}

	unsigned int SpacialSlotIndex::calcBlockCount(unsigned int totalSize, unsigned int blockSize)
	{
		auto wholeBlocks = totalSize / blockSize;
		auto bonusBlocks = (totalSize - (wholeBlocks * blockSize)) > 0 ? 1 : 0;
		return wholeBlocks + bonusBlocks;
	}

	RectangleOrganizer::RectangleOrganizer(Size size) :
		_size(size), _nextIndex(0), _spacialIndex(size, SPACIAL_INDEX_BLOCK_SIZE)
	{ }

	RectangleOrganizer::RectangleOrganizer(RectangleOrganizer &&other) :
		_size(other._size),
		_nextIndex(other._nextIndex),
		_slots(std::move(other._slots)),
		_spacialIndex(other._size, SPACIAL_INDEX_BLOCK_SIZE)
	{ }

	SlotSearchResult RectangleOrganizer::tryClaimSlot(Size size)
	{
		// if it couldn't possibly fit then give up right away
		if (size.width() > _size.width() || size.height() > _size.height())
			return SlotSearchResult::notFound();

		// if the whole thing is empty then just put at 0,0
		if (_slots.empty())
		{
			Slot slot(Rect(0, 0, size), _nextIndex++);
			_slots.push_back(slot);
			_spacialIndex.add(slot);
			return SlotSearchResult::found(slot);
		}

		// try putting adjacent to existing slots
		for (unsigned int i = 0; i < _slots.size(); i++)
		{
			auto slotResult = search(_slots[i], size);
			if (slotResult.isFound())
			{
				_slots.push_back(slotResult.slot());
				return slotResult;
			}
		}

		// couldn't find anywhere to put it :(
		return SlotSearchResult::notFound();
	}

	SlotSearchResult RectangleOrganizer::search(int y, Size size)
	{
		auto xOptions = findXOptions(y);
		for (unsigned int i = 0; i < xOptions.size(); i++)
		{
			auto x = xOptions[i];
			Rect rect(x, y, size);
			if (isRectOpen(rect))
			{
				return SlotSearchResult::found(Slot(rect, _nextIndex++));
			}
		}

		return SlotSearchResult::notFound();
	}

	SlotSearchResult RectangleOrganizer::search(Slot &slot, Size size)
	{
		auto topResult = search(slot.rect().y(), size);
		if (topResult.isFound())
		{
			return topResult;
		}
		auto bottomResult = search(slot.rect().y() + slot.rect().height(), size);
		return bottomResult;
	}

	bool RectangleOrganizer::releaseSlot(uint64_t index)
	{
		for (unsigned int i = 0; i < _slots.size(); i++)
		{
			if (_slots[i].index() == index)
			{
				_spacialIndex.remove(_slots[i]);
				_slots.erase(_slots.begin() + i);
				return true;
			}
		}

		return false;
	}

	bool RectangleOrganizer::isRectOpen(Rect &rect)
	{
		// if the rect would go off the edge of the texture space then it is not open
		if (rect.endX() > _size.width() || rect.endY() > _size.height())
		{
			return false;
		}

		// if the rect overlaps with any existing slot then it is not open
		for (unsigned int i = 0; i < _slots.size(); i++)
		{
			auto overlap = checkOverlap(rect, _slots[i].rect());
			if (overlap)
			{
				return false;
			}
		}

		return true;
	}

	bool RectangleOrganizer::checkOverlap(Rect &a, Rect &b)
	{
		auto aStartsAfterBHorizontally = b.endX() < a.x();
		auto bStartsAfterAHorizontally = a.endX() < b.x();
		auto aStartsAfterBVertically = b.endY() < a.y();
		auto bStartsAfterAVertically = a.endY() < b.y();

		auto overlapsHorizontally = !aStartsAfterBHorizontally && !bStartsAfterAHorizontally;
		auto overlapsVertically = !aStartsAfterBVertically && !bStartsAfterAVertically;

		return overlapsHorizontally && overlapsVertically;
	}

	std::vector<int> RectangleOrganizer::findXOptions(int y)
	{
		std::vector<int> result;
		result.push_back(0);

		for (unsigned int i = 0; i < _slots.size(); i++)
		{
			auto yOverlap = _slots[i].rect().y() <= y && y <= _slots[i].rect().endY();
			if (yOverlap)
			{
				result.push_back(_slots[i].rect().x());
				result.push_back(_slots[i].rect().endX() + 1);
			}
		}

		return result;
	}
}