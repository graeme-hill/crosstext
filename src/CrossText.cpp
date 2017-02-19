#include "CrossText.hpp"

namespace ct
{
	/**************************************************************************
	 * SpacialIndex
	 *************************************************************************/

	SpacialIndex::SpacialIndex(Size size, Size blockSize) :
		_blockSize(blockSize),
		_xBlocks(calcBlockCount(size.width, blockSize.width)),
		_yBlocks(calcBlockCount(size.height, blockSize.height)),
		_data(_xBlocks * _yBlocks)
	{ }

	SpacialIndex::SpacialIndex(SpacialIndex &&other) :
		_blockSize(other._blockSize),
		_xBlocks(other._xBlocks),
		_yBlocks(other._yBlocks),
		_data(std::move(other._data))
	{ }

	void SpacialIndex::add(Slot slot)
	{
		auto slotIndex = slot.index;
		withNearBlocks(slot.rect, [slotIndex](std::vector<uint64_t> &slots)
		{
			slots.push_back(slotIndex);
			return false;
		});
	}

	void SpacialIndex::remove(Slot slot)
	{
		auto slotIndex = slot.index;
		withNearBlocks(
			slot.rect,
			[this, slotIndex](std::vector<uint64_t> &slots)
		{
			slots.erase(std::remove(slots.begin(), slots.end(), slotIndex));
			return false;
		});
	}

	bool SpacialIndex::withNearBlocks(
		Rect rect, std::function<bool(std::vector<uint64_t> &)> action)
	{
		auto leftColumn = rect.x / _blockSize.width;
		auto rightColumn = rect.endX() / _blockSize.width;
		auto topRow = rect.y / _blockSize.height;
		auto bottomRow = rect.endY() / _blockSize.height;
		return withBlocksInRange(leftColumn, rightColumn, topRow, bottomRow, action);
	}

	bool SpacialIndex::withNearSlots(
		Rect rect, std::function<bool(uint64_t)> action)
	{
		std::unordered_map<uint64_t, bool> usedSlots;
		auto result = withNearBlocks(
			rect,
			[&usedSlots, &action](std::vector<uint64_t> &slots)
		{
			for (auto slotIndex : slots)
			{
				if (usedSlots.find(slotIndex) == usedSlots.end())
				{
					usedSlots[slotIndex] = true;
					if (action(slotIndex))
					{
						return true;
					}
				}
			}

			return false;
		});
		return result;
	}

	bool SpacialIndex::withSlotsOnYLine(
		unsigned y, std::function<bool(uint64_t)> action)
	{
		return withNearSlots({ 0, y, _blockSize.width * _xBlocks, 1 }, action);
	}

	bool SpacialIndex::withSlotsInBlockRange(
		unsigned leftColumn,
		unsigned rightColumn,
		unsigned topRow,
		unsigned bottomRow,
		std::function<bool(uint64_t)> action)
	{
		for (auto col = leftColumn; col <= rightColumn; col++)
		{
			for (auto row = topRow; row <= bottomRow; row++)
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

	bool SpacialIndex::withBlocksInRange(
		unsigned leftColumn,
		unsigned rightColumn,
		unsigned topRow,
		unsigned bottomRow,
		std::function<bool(std::vector<uint64_t> &)> action)
	{
		for (auto col = leftColumn; col <= rightColumn; col++)
		{
			for (auto row = topRow; row <= bottomRow; row++)
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

	unsigned SpacialIndex::getBlockIndex(unsigned x, unsigned y)
	{
		auto xBlock = x / _blockSize.width;
		auto yBlock = y / _blockSize.height;

		return yBlock * _xBlocks + xBlock;
	}

	unsigned SpacialIndex::calcBlockCount(
		unsigned totalSize, unsigned blockSize)
	{
		auto wholeBlocks = totalSize / blockSize;
		auto bonusBlocks = (totalSize - (wholeBlocks * blockSize)) > 0 ? 1 : 0;
		return wholeBlocks + bonusBlocks;
	}

	/**************************************************************************
	 * YCache
	 *************************************************************************/

	YCache::YCache(unsigned height) :
		_yCounts(height, 0)
	{
		// Give y=0 value a head start because it's the first place to check
		// and we never want its count to reach zero.
		increment(0);
	}

	YCache::YCache(YCache &&other) :
		_yCounts(std::move(other._yCounts)),
		_yCountPriority(std::move(other._yCountPriority))
	{ }

	void YCache::increment(unsigned y)
	{
		if (y >= _yCounts.size())
		{
			return;
		}

		auto countRef = &_yCounts[y];
		auto count = *countRef;
		if (count == 0)
		{
			// Add as top priority
			_yCountPriority.push_back({ y, countRef });
		}
		else
		{
			for (unsigned i = 0; i < _yCountPriority.size(); i++)
			{
				auto yCount = _yCountPriority[i];
				if (yCount.y == y)
				{
					// Promote to top priority
					_yCountPriority.erase(_yCountPriority.begin() + i);
					_yCountPriority.push_back(yCount);
					break;
				}
			}
		}
		(*countRef)++;
	}

	void YCache::decrement(unsigned y)
	{
		if (y >= _yCounts.size())
		{
			return;
		}

		auto countRef = &_yCounts[y];
		auto count = *countRef;

		for (unsigned i = 0; i < _yCountPriority.size(); i++)
		{
			auto yCount = _yCountPriority[i];
			if (yCount.y == y)
			{
				if (count <= 1)
				{
					// There's nothing here anymore so remove it
					_yCountPriority.erase(_yCountPriority.begin() + i);
				}
				else
				{
					// There's still stuff here to promote to top priority
					_yCountPriority.erase(_yCountPriority.begin() + i);
					_yCountPriority.push_back(yCount);
				}
				break;
			}
		}

		(*countRef)--;
	}

	void YCache::withYValuesInPriorityOrder(
		std::function<bool(unsigned y)> callback)
	{
		for (auto i = _yCountPriority.size(); i-- > 0;)
		{
			if (callback(_yCountPriority[i].y))
			{
				break;
			}
		}
	}

	/**************************************************************************
	 * RectangleOrganizer
	 *************************************************************************/

	RectangleOrganizer::RectangleOrganizer(Size size) :
		_size(size),
		_nextIndex(0),
		_spacialIndex(size, { SPACIAL_INDEX_BLOCK_WIDTH, SPACIAL_INDED_BLOCK_HEIGHT }),
		_yCache(size.height),
		_moved(false)
	{ }

	RectangleOrganizer::RectangleOrganizer(RectangleOrganizer &&other) :
		_size(other._size),
		_nextIndex(other._nextIndex),
		_slotMap(std::move(other._slotMap)),
		_spacialIndex(std::move(other._spacialIndex)),
		_yCache(std::move(other._yCache)),
		_moved(false)
	{
		other._moved = true;
	}

	void RectangleOrganizer::addSlot(Slot slot)
	{
		_slotMap[slot.index] = slot;
		_slotIndexes.push_back(slot.index);
		_spacialIndex.add(slot);
		_yCache.increment(slot.rect.endY() + 1);
		_yCache.increment(slot.rect.y);
	}

	void RectangleOrganizer::removeSlot(uint64_t slotIndex)
	{
		auto slot = _slotMap[slotIndex];
		_yCache.decrement(slot.rect.endY() + 1);
		_yCache.decrement(slot.rect.y);
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
		if (size.width > _size.width || size.height > _size.height)
		{
			//std::cout << "cannot fit!" << std::endl;
			return SlotSearchResult::notFound();
		}

		// if the whole thing is empty then just put at 0,0
		if (empty())
		{
			Slot slot{ { 0, 0, size.width, size.height }, _nextIndex++ };
			addSlot(slot);
			return SlotSearchResult::found(slot);
		}

		// try every usable y value in priority order
		auto result = SlotSearchResult::notFound();
		auto resultRef = &result;
		_yCache.withYValuesInPriorityOrder([this, size, resultRef](unsigned y)
		{
			auto searchResult = search(y, size);
			if (searchResult.isFound)
			{
				addSlot(searchResult.slot);
				*resultRef = searchResult;
				return true;
			}
			return false;
		});

		return result;
	}

	SlotSearchResult RectangleOrganizer::search(unsigned y, Size size)
	{
		auto result = SlotSearchResult::notFound();
		auto pResult = &result;
		withXOptions(y, [this, y, size, pResult](unsigned x) -> bool
		{
			Rect rect{ x, y, size.width, size.height };
			if (isRectOpen(rect))
			{
				*pResult = SlotSearchResult::found({ rect, _nextIndex++ });
				return true;
			}
			return false;
		});
		return result;
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
		if (rect.x < 0 || rect.y < 0)
		{
			return false;
		}

		// if the rect would go off the edge of the texture space then it is not open
		if (rect.endX() >= _size.width || rect.endY() >= _size.height)
		{
			return false;
		}


		// if the rect overlaps with any existing slot then it is not open
		bool foundOverlap = false;
		bool *foundOverlapRef = &foundOverlap;
		_spacialIndex.withNearSlots(rect, [this, rect, foundOverlapRef](uint64_t slotIndex) -> bool {
			if (checkOverlap(rect, _slotMap[slotIndex].rect))
			{
				*foundOverlapRef = true;
				return true;
			}
			return false;
		});

		return !foundOverlap;
	}

	bool RectangleOrganizer::checkOverlap(Rect a, Rect b)
	{
		auto aStartsAfterBHorizontally = b.endX() < a.x;
		auto bStartsAfterAHorizontally = a.endX() < b.x;
		auto aStartsAfterBVertically = b.endY() < a.y;
		auto bStartsAfterAVertically = a.endY() < b.y;

		auto overlapsHorizontally = !aStartsAfterBHorizontally && !bStartsAfterAHorizontally;
		auto overlapsVertically = !aStartsAfterBVertically && !bStartsAfterAVertically;

		return overlapsHorizontally && overlapsVertically;
	}

	void RectangleOrganizer::withXOptions(
		unsigned y, std::function<bool(unsigned)> callback)
	{
		if (callback(0))
		{
			return;
		}

		_spacialIndex.withSlotsOnYLine(y, [this, callback](uint64_t slotIndex)
		{
			auto slot = _slotMap[slotIndex];
			if (slot.rect.x > 0 && callback(slot.rect.x))
			{
				return true;
			}

			auto endX = slot.rect.endX() + 1;
			if (callback(endX))
			{
				return true;
			}

			return false;
		});

		_usedXOptions.clear();
	}
}
