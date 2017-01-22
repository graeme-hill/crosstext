#include "CrossText.hpp"
#include <iostream>
#include <algorithm>

namespace ct
{
	/**************************************************************************
	 * SlotSearchResult
	 *************************************************************************/

	SlotSearchResult SlotSearchResult::notFound()
	{
		return SlotSearchResult(false, Slot(Rect(), 0));
	}

	SlotSearchResult SlotSearchResult::found(Slot slot)
	{
		return SlotSearchResult(true, slot);
	}

	/**************************************************************************
	 * TextManager
	 *************************************************************************/

	TextManager::TextManager(TRenderOptions renderOptions) :
		_renderer(TRenderer(renderOptions)), _lastUsed(0)
	{
		for (int i = 0; i < renderOptions.textureCount(); i++)
		{
			_textures.push_back(Texture(TImageData(_renderer, renderOptions.textureSize())));
		}
	}

	Placement TextManager::findPlacement(Size size)
	{
		auto firstResult = _textures[_lastUsed].organizer().tryClaimSlot(size);
		if (firstResult.isFound())
		{
			return Placement(true, _textures[_lastUsed], firstResult.slot());
		}

		for (int i = 0; i < _textures.size(); i++)
		{
			if (i == _lastUsed)
			{
				continue;
			}

			auto result = _textures[i].organizer().tryClaimSlot(size);
			if (result.isFound())
			{
				// found a place for the text block :)
				_lastUsed = i;
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

	/**************************************************************************
	 * TextBlock
	 *************************************************************************/

	TextBlock::TextBlock(TextManager &manager, std::wstring text, FontOptions font) :
		TextBlock(manager, text, font, std::vector<FontRange>())
	{ }

	TextBlock::TextBlock(
		TextManager &manager, std::wstring text, FontOptions font, std::vector<FontRange> fontRanges) :
		_manager(manager), _texture(nullptr), _fontRanges(fontRanges)
	{
		TBuilder builder(manager.renderer(), text, font, fontRanges);
		auto size = builder.size();
		auto placement = manager.findPlacement(size);
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

	/**************************************************************************
	 * Placement
	 *************************************************************************/

	Placement::Placement(bool isFound, Texture &texture, Slot slot) :
		_texture(texture), _slot(slot), _isFound(isFound)
	{ }

	/**************************************************************************
	 * SpacialSlotIndex
	 *************************************************************************/

	SpacialSlotIndex::SpacialSlotIndex(Size size, int blockSize) :
		_blockSize(blockSize),
		_xBlocks(calcBlockCount(size.width(), blockSize)),
		_yBlocks(calcBlockCount(size.height(), blockSize)),
		_data(_xBlocks * _yBlocks)
	{ }

	void SpacialSlotIndex::add(Slot slot)
	{
		auto slotIndex = slot.index();
		withNearBlocks(slot.rect(), [slotIndex](std::vector<uint64_t> &slots) -> bool {
			slots.push_back(slotIndex);
			return false;
		});
	}

	void SpacialSlotIndex::remove(Slot slot)
	{
		auto slotIndex = slot.index();
		withNearBlocks(slot.rect(), [this, slotIndex](std::vector<uint64_t> &slots) -> bool {
			slots.erase(std::remove(slots.begin(), slots.end(), slotIndex));
			return false;
		});
	}

	bool SpacialSlotIndex::withNearBlocks(Rect rect, std::function<bool(std::vector<uint64_t> &)> action)
	{
		auto leftColumn = rect.x() / _blockSize;
		auto rightColumn = rect.endX() / _blockSize;
		auto topRow = rect.y() / _blockSize;
		auto bottomRow = rect.endY() / _blockSize;
		return withBlocksInRange(leftColumn, rightColumn, topRow, bottomRow, action);
	}

	bool SpacialSlotIndex::withNearSlots(Rect rect, std::function<bool(uint64_t)> action)
	{
		std::unordered_map<uint64_t, bool> used;
		return withNearBlocks(rect, [&used, &action](std::vector<uint64_t> &slots) {
			for (auto slotIndex : slots)
			{
				if (used.find(slotIndex) == used.end())
				{
					used[slotIndex] = true;
					if (action(slotIndex))
					{
						return true;
					}
				}
			}

			return false;
		});
	}

	bool SpacialSlotIndex::withSlotsOnYLine(int y, std::function<bool(uint64_t)> action)
	{
		return withNearSlots(Rect(0, y, Size(_blockSize * _xBlocks, 1)), action);
	}

	bool SpacialSlotIndex::withSlotsInBlockRange(
		int leftColumn,
		int rightColumn,
		int topRow,
		int bottomRow,
		std::function<bool(uint64_t)> action)
	{
		for (int col = leftColumn; col <= rightColumn; col++)
		{
			for (int row = topRow; row <= bottomRow; row++)
			{
				auto index = row * _xBlocks + col;
				for (auto &slotIndex : _data[index])
				{
					if (action(slotIndex))
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	bool SpacialSlotIndex::withBlocksInRange(
		int leftColumn,
		int rightColumn,
		int topRow,
		int bottomRow,
		std::function<bool(std::vector<uint64_t> &)> action)
	{
		for (int col = leftColumn; col <= rightColumn; col++)
		{
			for (int row = topRow; row <= bottomRow; row++)
			{
				auto index = row * _xBlocks + col;
				if (action(_data[index]))
				{
					return true;
				}
			}
		}

		return false;
	}

	int SpacialSlotIndex::getBlockIndex(int x, int y)
	{
		auto xBlock = x / _blockSize;
		auto yBlock = y / _blockSize;

		return yBlock * _xBlocks + xBlock;
	}

	int SpacialSlotIndex::calcBlockCount(int totalSize, int blockSize)
	{
		auto wholeBlocks = totalSize / blockSize;
		auto bonusBlocks = (totalSize - (wholeBlocks * blockSize)) > 0 ? 1 : 0;
		return wholeBlocks + bonusBlocks;
	}

	/**************************************************************************
	 * RectangleOrganizer
	 *************************************************************************/

	RectangleOrganizer::RectangleOrganizer(Size size) :
		_size(size), _nextIndex(0), _spacialIndex(size, SPACIAL_INDEX_BLOCK_SIZE)
	{ }

	RectangleOrganizer::RectangleOrganizer(RectangleOrganizer &&other) :
		_size(other._size),
		_nextIndex(other._nextIndex),
		_slotMap(std::move(other._slotMap)),
		_spacialIndex(other._size, SPACIAL_INDEX_BLOCK_SIZE)
	{ }

	void RectangleOrganizer::addSlot(Slot slot)
	{
		_slotMap[slot.index()] = slot;
		_slotIndexes.push_back(slot.index());
		_spacialIndex.add(slot);
	}

	void RectangleOrganizer::removeSlot(uint64_t slotIndex)
	{
		auto slot = _slotMap[slotIndex];
		_spacialIndex.remove(slot);
		_slotIndexes.erase(
			std::remove(_slotIndexes.begin(), _slotIndexes.end(), slotIndex),
			_slotIndexes.end());
		_slotMap.erase(slotIndex);
	}

	bool RectangleOrganizer::empty()
	{
		return _slotMap.empty();
	}

	SlotSearchResult RectangleOrganizer::tryClaimSlot(Size size)
	{

		// if it couldn't possibly fit then give up right away
		if (size.width() > _size.width() || size.height() > _size.height())
		{
			return SlotSearchResult::notFound();
		}

		// if the whole thing is empty then just put at 0,0
		if (empty())
		{
			Slot slot(Rect(0, 0, size), _nextIndex++);
			addSlot(slot);
			return SlotSearchResult::found(slot);
		}

		// try putting adjacent to existing slots
		for (int i = _slotIndexes.size() - 1; i >= 0; i--)
		{
			auto slot = _slotMap[_slotIndexes[i]];
			auto slotResult = search(slot, size);
			if (slotResult.isFound())
			{
				addSlot(slotResult.slot());
				return slotResult;
			}
		}

		return SlotSearchResult::notFound();
	}

	SlotSearchResult RectangleOrganizer::search(int y, Size size)
	{
		auto result = SlotSearchResult::notFound();
		auto pResult = &result;
		withXOptions(y, [this, y, size, pResult](int x) -> bool {
			Rect rect(x, y, size);
			if (isRectOpen(rect))
			{
				*pResult = SlotSearchResult::found(Slot(rect, _nextIndex++));
				return true;
			}
			return false;
		});
		return result;
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
		if (_slotMap.find(index) != _slotMap.end())
		{
			removeSlot(index);
			return true;
		}

		return false;
	}

	bool RectangleOrganizer::isRectOpen(Rect &rect)
	{
		// if the rest starts in negative space then it is not open
		if (rect.x() < 0 || rect.y() < 0)
		{
			return false;
		}

		// if the rect would go off the edge of the texture space then it is not open
		if (rect.endX() > _size.width() || rect.endY() > _size.height())
		{
			return false;
		}

		// if the rect overlaps with any existing slot then it is not open
		auto foundOverlap = _spacialIndex.withNearSlots(rect, [this, rect](uint64_t slotIndex) -> bool {
			return checkOverlap(rect, _slotMap[slotIndex].rect());
		});

		return !foundOverlap;

		/*for (int i = 0; i < _slots.size(); i++)
		{
			auto overlap = checkOverlap(rect, _slots[i].rect());
			if (overlap)
			{
				return false;
			}
		}

		return true;*/
	}

	bool RectangleOrganizer::checkOverlap(Rect a, Rect b)
	{
		auto aStartsAfterBHorizontally = b.endX() < a.x();
		auto bStartsAfterAHorizontally = a.endX() < b.x();
		auto aStartsAfterBVertically = b.endY() < a.y();
		auto bStartsAfterAVertically = a.endY() < b.y();

		auto overlapsHorizontally = !aStartsAfterBHorizontally && !bStartsAfterAHorizontally;
		auto overlapsVertically = !aStartsAfterBVertically && !bStartsAfterAVertically;

		return overlapsHorizontally && overlapsVertically;
	}

	//std::vector<int> RectangleOrganizer::findXOptions(int y)
	//{
	//	for (int i = 0; i < _slots.size(); i++)
	//	{
	//		auto yOverlap = _slots[i].rect().y() <= y && y <= _slots[i].rect().endY();
	//		if (yOverlap)
	//		{
	//			result.push_back(_slots[i].rect().x());
	//			result.push_back(_slots[i].rect().endX() + 1);
	//		}
	//	}

	//	return result;
	//}

	void RectangleOrganizer::withXOptions(int y, std::function<bool(int)> callback)
	{
		if (callback(0))
		{
			return;
		}

		_spacialIndex.withSlotsOnYLine(y, [this, callback](uint64_t slotIndex) -> bool {
			auto slot = _slotMap[slotIndex];
			if (slot.rect().x() > 0 && callback(slot.rect().x()))
			{
				return true;
			}

			if (callback(slot.rect().endX() + 1))
			{
				return true;
			}

			return false;
		});
	}
}