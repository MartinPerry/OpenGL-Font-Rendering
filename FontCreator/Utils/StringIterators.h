#ifndef STRING_ITERATORS_H
#define STRING_ITERATORS_H

#include <string>
#include <string_view>
#include <limits>
#include <array>

#include "../Externalncludes.h"

#ifdef USE_ICU_LIBRARY
#	include <unicode/unistr.h>
#	include <unicode/schriter.h>
#endif


//========================================================================

/// <summary>
/// Input string is ASCII string
/// Will return char32_t -> ASCII id
/// </summary>
template <typename StringType>
struct CustomAsciiIterator 
{	
	using CharType = StringType::value_type;

	static inline char32_t DONE = std::numeric_limits<char32_t>::max();

	CustomAsciiIterator(const StringType& str) :
		v(std::basic_string_view<CharType>(str)),
		index(0)
	{}

	
	CustomAsciiIterator(const CustomAsciiIterator& other) :
		v(other.v),
		index(other.index)
	{}

	void SetOffsetFromStart(uint32_t offset) { index = offset; }
	void SetOffsetFromCurrent(uint32_t offset) { index += offset; }

	char32_t GetFirst() { return static_cast<char32_t>(this->v[0]); }
	char32_t GetCurrent() { return static_cast<char32_t>(this->v[index]); }
	char32_t GetNext() { this->index++; return this->GetCurrent(); }
	char32_t GetCurrentAndAdvance()
	{ 
		if (this->HasNext() == false)
		{
			return CustomAsciiIterator::DONE;
		}
		char32_t c = static_cast<char32_t>(this->v[this->index]);
		this->index++; 		
		return c;
	}
	bool HasNext() { return index < v.length(); }

protected:
	std::basic_string_view<CharType> v;
	size_t index;
};

//========================================================================

/// <summary>
/// Input string is unicode string
/// Will return uint32_t -> unicode code point
/// 
/// Example:
/// std::u8string testUnicode = u8"ABC"
///		u8"\u010D"          // è
///		u8"\u017E"          // ž
///		u8"\u03A9"          // "omega"
///		u8"\u6C34"          // "chinese"
///		u8"\U0001F600"      // :)
///		u8"\U0001F63A"      // "cat face"
///		u8"a\u0301";        // a + combining acute
/// 
/// </summary>
struct CustomU8Iterator
{
	static inline char32_t DONE = std::numeric_limits<uint32_t>::max();

	CustomU8Iterator(const std::u8string& str) :
		CustomU8Iterator(std::u8string_view(str))		
	{
	}

	CustomU8Iterator(const std::u8string_view& view) :
		itStart(view.data()),
		it(view.data()),
		itEnd(view.data() + view.size())
	{
	}

	CustomU8Iterator(const std::string_view& view) :
		itStart((char8_t*)view.data()),
		it((char8_t*)view.data()),
		itEnd((char8_t*)view.data() + view.size())
	{
	}

	CustomU8Iterator(const CustomU8Iterator& other) :		
		it(other.it),
		itStart(other.itStart),
		itEnd(other.itEnd)
	{
	}

	void SetOffsetFromStart(uint32_t offset) 
	{ 
		it = itStart; 
		this->SetOffsetFromCurrent(offset);
	}
	void SetOffsetFromCurrent(uint32_t offset) 
	{ 
		for (uint32_t i = 0; i < offset; i++)
		{
			utf8::unchecked::next(it);
		}
	}

	char32_t GetFirst()
	{ 
		it = itStart;
		return utf8::unchecked::peek_next(it); 
	}
	char32_t GetCurrent()
	{ 
		if (this->HasNext() == false)
		{
			return CustomU8Iterator::DONE;
		}
		return utf8::unchecked::peek_next(it); 
	}

	char32_t GetNext()
	{ 
		if (this->HasNext() == false)
		{
			return CustomU8Iterator::DONE;
		}
		utf8::unchecked::next(it); 

		return this->GetCurrent();
	}
	char32_t GetCurrentAndAdvance()
	{
		if (this->HasNext() == false)
		{
			return CustomU8Iterator::DONE;
		}
		char32_t c = utf8::unchecked::next(it);
		return c;
	}
	bool HasNext() { return it < itEnd; }

protected:
		
	const char8_t* it;
	const char8_t* itEnd;
	const char8_t* itStart;
};


//========================================================================


#ifdef USE_ICU_LIBRARY

struct CustomUnicodeIterator : public icu::StringCharacterIterator
{		

	CustomUnicodeIterator(const icu::UnicodeString & str) :
		icu::StringCharacterIterator(str)
	{}

	void SetOffsetFromStart(uint32_t offset) { this->move32(offset, icu::CharacterIterator::EOrigin::kStart); }
	void SetOffsetFromCurrent(uint32_t offset) { this->move32(offset, icu::CharacterIterator::EOrigin::kCurrent); }
	char32_t GetFirst() { return this->first32(); }
	char32_t GetCurrent() { return this->current32(); }
	char32_t GetNext() { return this->next32(); }
	char32_t GetCurrentAndAdvance() { return this->next32PostInc(); }
	bool HasNext() { return this->hasNext(); }
		
};

#endif

//========================================================================

/// <summary>
/// Iterate Unicode sequence and return UTF8 bytes
/// ?? USELESS .. we can iterate directly
/// std::u8string chars and they are UTF8 bytes
/// 
/// Example:
/// std::u8string testUnicode = u8""
///		u8"ABC"
///		u8"\u010D"          // è
///		u8"\u017E"          // ž
///		u8"\u03A9"          // "omega"
///		u8"\u6C34"          // "chinese"
///		u8"\U0001F600"      // :)
///		u8"\U0001F63A"      // "cat face"
///		u8"a\u0301";        // a + combining acute
/// 
/// => internaly store in UTF8
/// 
/// Will result in UTF-8 bytes:
/// 
///		"\x41\x42\x43"        // ABC
///		"\xC4\x8D"            // U+010D
///		"\xC5\xBE"            // U+017E
///		"\xCE\xA9"            // U+03A9
///		"\xE6\xB0\xB4"        // U+6C34
///		"\xF0\x9F\x98\x80"    // U+1F600
///		"\xF0\x9F\x98\xBA"    // U+1F63A 
///		"\x61\xCC\x81"       // a + combining acute
/// 
/// </summary>
/// <typeparam name="UnicodeIterator"></typeparam>
template <typename UnicodeIterator>
struct CustomUtf8BytesIterator
{
	static inline uint8_t DONE = 0;

	CustomUtf8BytesIterator(UnicodeIterator uniIt) :
		it(uniIt),
		curCodePoint(0),
		buf({ 0,0,0,0 }),
		bufIndex(4)
	{
	}
		
	uint8_t GetCurrent()
	{
		if ((bufIndex == 4) || (buf[bufIndex] == 0))
		{
			if (this->LoadUnicode() == false)
			{
				return DONE;
			}
			bufIndex = 0;
		}
		return buf[bufIndex];
	}

	uint8_t GetNext()
	{
		if ((bufIndex == 4) || (buf[bufIndex] == 0))
		{
			if (this->LoadUnicode() == false)
			{
				return DONE;
			}
			bufIndex = 0;
		}
		else 
		{
			bufIndex++;
		}

		return this->GetCurrent();
	}

	uint8_t GetCurrentAndAdvance()
	{
		if ((bufIndex == 4) || (buf[bufIndex] == 0))
		{
			if (this->LoadUnicode() == false)
			{
				return DONE;
			}
			bufIndex = 0;
		}

		return buf[bufIndex++];
	}
	
	bool HasNext() 
	{ 
		return (it.HasNext() && bufIndex < 4);
	}

protected:
	UnicodeIterator it;
	char32_t curCodePoint;
	std::array<uint8_t, 4> buf;
	int bufIndex;

	bool LoadUnicode()
	{
		curCodePoint = it.GetCurrentAndAdvance();
		if (curCodePoint == it.DONE)
		{
			return false;
		}
		
		if constexpr (std::is_same<UnicodeIterator, CustomAsciiIterator<typename std::basic_string_view<uint8_t>>>::value)
		{
			//Custom raw data iterator - exects data already in UTF8 bytes, so just return the bytes
			buf[0] = static_cast<uint8_t>(curCodePoint);
			return true;
		}
		else
		{
			if (curCodePoint > 0x10FFFFu || (curCodePoint >= 0xD800u && curCodePoint <= 0xDFFFu))
			{
				// replace with U+FFFD (or throw)
				curCodePoint = 0xFFFDu;
			}

			//buf = { 0,0,0,0 };
			//char* it = reinterpret_cast<char*>(buf.data());
			//utf8::append(static_cast<char32_t>(curCodePoint), it);		
			this->UnicodeToUtf8(curCodePoint, buf.data());
			return true;
		}		
	}

	void UnicodeToUtf8(char32_t cp, uint8_t* result) const noexcept
	{
		if (cp < 0x80) {                   // one octet
			*(result++) = static_cast<uint8_t>(cp);
			*(result++) = 0;
			*(result++) = 0;
			*(result++) = 0;
		}
		else if (cp < 0x800) {                // two octets
			*(result++) = static_cast<uint8_t>((cp >> 6) | 0xc0);
			*(result++) = static_cast<uint8_t>((cp & 0x3f) | 0x80);
			*(result++) = 0;
			*(result++) = 0;
		}
		else if (cp < 0x10000) {              // three octets
			*(result++) = static_cast<uint8_t>((cp >> 12) | 0xe0);
			*(result++) = static_cast<uint8_t>(((cp >> 6) & 0x3f) | 0x80);
			*(result++) = static_cast<uint8_t>((cp & 0x3f) | 0x80);
			*(result++) = 0;
		}
		else {                                // four octets
			*(result++) = static_cast<uint8_t>((cp >> 18) | 0xf0);
			*(result++) = static_cast<uint8_t>(((cp >> 12) & 0x3f) | 0x80);
			*(result++) = static_cast<uint8_t>(((cp >> 6) & 0x3f) | 0x80);
			*(result++) = static_cast<uint8_t>((cp & 0x3f) | 0x80);
		}
	}
};


//========================================================================
//========================================================================
//========================================================================

struct CustomIteratorCreator
{
	template <typename T>
	static auto Create(const T& str)
	{
		if constexpr (std::is_same<T, std::string>::value)
		{
			return CustomAsciiIterator(str);
		}

		if constexpr (std::is_same<T, std::string_view>::value)
		{
			return CustomAsciiIterator(str);
		}

		if constexpr (std::is_same<T, std::u8string>::value)
		{
			return CustomU8Iterator(str);
		}

		if constexpr (std::is_same<T, std::u8string_view>::value)
		{
			return CustomU8Iterator(str);
		}

#ifdef USE_ICU_LIBRARY
		if constexpr (std::is_same<T, icu::UnicodeString>::value)
		{
			return CustomUnicodeIterator(str);
		}
#endif
	}

	template <typename T>
	static auto CreateUtf8Bytes(const T& str)
	{
		auto iter = CustomIteratorCreator::Create(str);
		return CustomUtf8BytesIterator(iter);
	}
	
	/// <summary>
	/// Data are raw string data as-is so we expect they are already in UTF8 bytes
	/// </summary>
	/// <param name="strData"></param>
	/// <param name="dataSize"></param>
	/// <returns></returns>
	static auto CreateUtf8Bytes(const char* utf8StrData, size_t dataSize)
	{
		std::basic_string_view<uint8_t> ut8Bytes((uint8_t *)utf8StrData, dataSize);

		auto iter = CustomAsciiIterator(ut8Bytes);		
		return CustomUtf8BytesIterator(iter);
	}
};


/*
Unicode -> UTF8
Example: "è"
Unicode: U+010D
UTF8 bytes: [0xC4, 0x8D] -> [196, 141]
*/


#endif
