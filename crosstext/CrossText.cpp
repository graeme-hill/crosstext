#include "CrossText.hpp"

namespace ct
{
	SlotSearchResult SlotSearchResult::notFound()
	{
		return SlotSearchResult(false, Slot(Rect(0.0f, 0.0f, 0.0f, 0.0f), 0));
	}

	SlotSearchResult SlotSearchResult::found(Slot slot)
	{
		return SlotSearchResult(true, slot);
	}

	TextManager::TextManager(TRenderOptions renderOptions) :
		_renderer(TRenderer(renderOptions))
	{
		_textures.push_back(Texture(TImageData(_renderer, renderOptions.textureSize())));
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

	void TextManager::releaseRect(Texture &texture, unsigned long index)
	{
		texture.organizer().releaseSlot(index);
	}

	TextBlock::TextBlock(TextManager &manager, Text text, Font &font, Brush brush) :
		_manager(manager), _texture(nullptr)
	{
		TBuilder builder(manager.renderer(), text, font.systemFont(), brush);
		auto size = builder.size();
		auto placement = manager.findPlacement(size);
		_slot = placement.slot();
		builder.render(placement.texture().imageData(), _slot.rect());
		_texture = &placement.texture();
	}

	TextBlock::TextBlock(TextBlock &&other) :
		_manager(other._manager),
		_texture(other._texture),
		_slot(other._slot)
	{
		_texture = nullptr;
	}

	TextBlock::~TextBlock()
	{
		if (_texture)
		{
			_manager.releaseRect(*_texture, _slot.index());
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

	Font::Font(TextManager &manager, FontOptions options) :
		_systemFont(manager.renderer(), options)
	{ }

	RectangleOrganizer::RectangleOrganizer(Size size) :
		_size(size), _nextIndex(0)
	{ }

	RectangleOrganizer::RectangleOrganizer(RectangleOrganizer &&other) :
		_size(other._size),
		_nextIndex(other._nextIndex),
		_slots(std::move(other._slots))
	{ }

	SlotSearchResult RectangleOrganizer::tryClaimSlot(Size size)
	{
		// if it couldn't possibly fit then give up right away
		if (size.width() > _size.width() || size.height() > _size.height())
			return SlotSearchResult::notFound();

		// if the whole thing is empty then just put at 0,0
		if (_slots.empty())
		{
			Slot slot(Rect(0.0f, 0.0f, size), _nextIndex++);
			_slots.push_back(slot);
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

	SlotSearchResult RectangleOrganizer::search(float y, Size size)
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

	bool RectangleOrganizer::releaseSlot(unsigned long index)
	{
		for (unsigned int i = 0; i < _slots.size(); i++)
		{
			if (_slots[i].index() == index)
			{
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
		auto aStartsAfterBHorizontally = lessish(b.endX(), a.x());
		auto bStartsAfterAHorizontally = lessish(a.endX(), b.x());
		auto aStartsAfterBVertically = lessish(b.endY(), a.y());
		auto bStartsAfterAVertically = lessish(a.endY(), b.y());

		auto overlapsHorizontally = !aStartsAfterBHorizontally && !bStartsAfterAHorizontally;
		auto overlapsVertically = !aStartsAfterBVertically && !bStartsAfterAVertically;

		return overlapsHorizontally && overlapsVertically;
	}

	std::vector<float> RectangleOrganizer::findXOptions(float y)
	{
		std::vector<float> result;
		result.push_back(0.0f);

		for (unsigned int i = 0; i < _slots.size(); i++)
		{
			auto yOverlap = betweenish(y, _slots[i].rect().y(), _slots[i].rect().endY());
			if (yOverlap)
			{
				result.push_back(_slots[i].rect().x());
				result.push_back(_slots[i].rect().endX());
			}
		}

		return result;
	}
}

#include <Windows.h>

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	ct::DirectWriteRenderOptions options(ct::Size(1024.0f, 1024.0f));
	ct::TextManager manager(options);

	ct::FontOptions fontOptions1(
		L"Times New Roman",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		20.0f,
		L"en-US");

	ct::FontOptions fontOptions2(
		L"Comic Sans MS",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		60.0f,
		L"en-US");

	ct::FontOptions fontOptions3(
		L"Arial",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		14.0f,
		L"en-US");

	ct::Brush brush1(ct::Color(0xffffffff));
	ct::Brush brush2(ct::Color(0xff00ffff));
	ct::Brush brush3(ct::Color(0xffff00ff));

	ct::Font font1(manager, fontOptions1);
	ct::Font font2(manager, fontOptions2);
	ct::Font font3(manager, fontOptions3);
	//ct::Brush brush(manager, brushOptions);

	ct::TextBlock hello1a(manager, ct::Text(std::wstring(L"Here are some medium sized letters.")), font1, brush1);
	ct::TextBlock hello2a(manager, ct::Text(std::wstring(L"Here are some really big letters")), font2, brush2);
	ct::TextBlock hello3a(manager, ct::Text(std::wstring(L"Tiny tiny letters here :-)")), font3, brush3);

	ct::TextBlock hello1b(manager, ct::Text(std::wstring(L"This is some more medium sized.")), font1, brush1);
	ct::TextBlock hello2b(manager, ct::Text(std::wstring(L"BIG BIG BIG <(^_^<)")), font2, brush2);
	ct::TextBlock hello3b(manager, ct::Text(std::wstring(L"Little")), font3, brush3);

	ct::TextBlock hello1c(manager, ct::Text(std::wstring(L"M M M M Medium.")), font1, brush1);
	ct::TextBlock hello3c(manager, ct::Text(std::wstring(L"This is a longer string of text just because that is what I want to test This is a longer string of text just because that is what I want to test This is a longer string of text just because that is what I want to test")), font3, brush3);
	ct::TextBlock hello2c(manager, ct::Text(std::wstring(L"(>^_^)> BIG BIG BIG")), font2, brush2);
	ct::TextBlock hello4c(manager, ct::Text(std::wstring(L"^~^")), font2, brush3);
	ct::TextBlock hello5c(manager, ct::Text(std::wstring(L":^)")), font2, brush1);
	ct::TextBlock hello6c(manager, ct::Text(std::wstring(L":^)")), font2, brush2);
	ct::TextBlock hello7c(manager, ct::Text(std::wstring(L":^)")), font2, brush2);
	ct::TextBlock hello8c(manager, ct::Text(std::wstring(L":^)")), font2, brush2);

	hello1a.texture()->imageData().savePng(L"C:\\temp\\test3.png");

	return 0;
}