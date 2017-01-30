#include "CrossText.hpp"
#include <iostream>
#include <algorithm>

namespace ct
{
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
		if (firstResult.isFound)
		{
			return Placement::found(firstResult.slot, &_textures.at(_lastUsed));
		}

		for (int i = 0; i < _textures.size(); i++)
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
				return Placement::found(result.slot, &_textures[i]);
			}
		}

		// there is nowhere that can fit a text block of this size :(
		return Placement::notFound();
	}

	void TextManager::releaseRect(Texture *texture, Slot slot)
	{
		texture->imageData().clearRect(slot.rect);
		texture->organizer().releaseSlot(slot.index);
	}

	/**************************************************************************
	 * TextBlock
	 *************************************************************************/

	TextBlock::TextBlock(TextManager &manager, std::wstring text, TextOptions options) :
		_manager(&manager),
		_options(options),
		_placement(initPlacement(manager, text, options))
	{ }

	Placement TextBlock::initPlacement(
		TextManager &manager, 
		std::wstring &text, 
		TextOptions options)
	{
		TBuilder builder(manager.renderer(), text, options);
		auto size = builder.size();
		auto placement = manager.findPlacement(size);
		if (placement.isFound)
		{
			builder.render(placement.texture->imageData(), placement.slot.rect);
		}
		return placement;
	}

	TextBlock::TextBlock(TextBlock &&other) :
		_manager(other._manager),
		_placement(other._placement),
		_options(other._options)
	{
		other._manager = nullptr;
	}

	TextBlock &TextBlock::operator=(TextBlock &&other)
	{
		dispose();
		_manager = other._manager;
		_placement = other._placement;
		_options = other._options;
		other._manager = nullptr;
		return *this;
	}

	TextBlock::~TextBlock()
	{
		dispose();
	}

	void TextBlock::dispose()
	{
		if (!dead() && foundPlacement())
		{
			_manager->releaseRect(_placement.texture, _placement.slot);
		}
	}

	/**************************************************************************
	* Texture
	*************************************************************************/

	Texture::Texture(TImageData imageData) :
		_imageData(std::move(imageData)), _organizer(imageData.size())
	{ }

	Texture::Texture(Texture &&other) :
		_imageData(std::move(other._imageData)),
		_organizer(std::move(other._organizer))
	{ }

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
		withNearBlocks(slot.rect, [slotIndex](std::vector<uint64_t> &slots) -> bool {
			slots.push_back(slotIndex);
			return false;
		});
	}

	void SpacialIndex::remove(Slot slot)
	{
		auto slotIndex = slot.index;
		withNearBlocks(slot.rect, [this, slotIndex](std::vector<uint64_t> &slots) -> bool {
			slots.erase(std::remove(slots.begin(), slots.end(), slotIndex));
			return false;
		});
	}

	bool SpacialIndex::withNearBlocks(Rect rect, std::function<bool(std::vector<uint64_t> &)> action)
	{
		auto leftColumn = rect.x / _blockSize.width;
		auto rightColumn = rect.endX() / _blockSize.width;
		auto topRow = rect.y / _blockSize.height;
		auto bottomRow = rect.endY() / _blockSize.height;
		return withBlocksInRange(leftColumn, rightColumn, topRow, bottomRow, action);
	}

	bool SpacialIndex::withNearSlots(Rect rect, std::function<bool(uint64_t)> action)
	{
		std::unordered_map<uint64_t, bool> usedSlots;
		auto result = withNearBlocks(rect, [&usedSlots, &action](std::vector<uint64_t> &slots) {
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

	bool SpacialIndex::withSlotsOnYLine(int y, std::function<bool(uint64_t)> action)
	{
		//std::cout << "withSlotsOnYLine y=" << y << std::endl;
		return withNearSlots({ 0, y, _blockSize.width * _xBlocks, 1 }, action);
	}

	bool SpacialIndex::withSlotsInBlockRange(
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

	bool SpacialIndex::withBlocksInRange(
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

	int SpacialIndex::getBlockIndex(int x, int y)
	{
		auto xBlock = x / _blockSize.width;
		auto yBlock = y / _blockSize.height;

		return yBlock * _xBlocks + xBlock;
	}

	int SpacialIndex::calcBlockCount(int totalSize, int blockSize)
	{
		auto wholeBlocks = totalSize / blockSize;
		auto bonusBlocks = (totalSize - (wholeBlocks * blockSize)) > 0 ? 1 : 0;
		return wholeBlocks + bonusBlocks;
	}

	/**************************************************************************
	 * YCache
	 *************************************************************************/

	YCache::YCache(int height) :
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

	void YCache::increment(int y)
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
			for (int i = 0; i < _yCountPriority.size(); i++)
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

	void YCache::decrement(int y)
	{
		if (y >= _yCounts.size())
		{
			return;
		}

		auto countRef = &_yCounts[y];
		auto count = *countRef;
		
		for (int i = 0; i < _yCountPriority.size(); i++)
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

	void YCache::withYValuesInPriorityOrder(std::function<bool(int y)> callback)
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
		_yCache.withYValuesInPriorityOrder([this, size, resultRef](int y)
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

	SlotSearchResult RectangleOrganizer::search(int y, Size size)
	{
		auto result = SlotSearchResult::notFound();
		auto pResult = &result;
		withXOptions(y, [this, y, size, pResult](int x) -> bool
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

	void RectangleOrganizer::withXOptions(int y, std::function<bool(int)> callback)
	{
		if (callback(0))
		{
			return;
		}

		_spacialIndex.withSlotsOnYLine(y, [this, callback](uint64_t slotIndex) -> bool
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