#ifndef STRING_ITERATORS_H
#define STRING_ITERATORS_H

#include <string>
#include <string_view>

#include "../Externalncludes.h"

#ifdef USE_ICU_LIBRARY
#	include <unicode/unistr.h>
#	include <unicode/schriter.h>
#endif

struct CustomAsciiIterator 
{	
	static inline uint32_t DONE = 65535;

	CustomAsciiIterator(const std::string & str) :
		v(str),
		index(0)
	{}

	CustomAsciiIterator(const std::string_view & view) :
		v(view),
		index(0)
	{}

	void SetOffsetFromStart(uint32_t offset) { index = offset; }
	void SetOffsetFromCurrent(uint32_t offset) { index += offset; }

	uint32_t GetFirst() { return this->v[0]; }
	uint32_t GetCurrent() { return this->v[index]; }
	uint32_t GetNext() { this->index++; return 0; }
	uint32_t GetCurrentAndAdvance() 
	{ 
		if (this->HasNext() == false)
		{
			return CustomAsciiIterator::DONE;
		}
		uint32_t c = this->v[this->index];
		this->index++; 		
		return c;
	}
	bool HasNext() { return index < v.length(); }

protected:
	std::string_view v;
	size_t index;
};

#ifdef USE_ICU_LIBRARY

struct CustomUnicodeIterator : public icu::StringCharacterIterator
{		

	CustomUnicodeIterator(const icu::UnicodeString & str) :
		icu::StringCharacterIterator(str)
	{}

	void SetOffsetFromStart(uint32_t offset) { this->move32(offset, icu::CharacterIterator::EOrigin::kStart); }
	void SetOffsetFromCurrent(uint32_t offset) { this->move32(offset, icu::CharacterIterator::EOrigin::kCurrent); }
	uint32_t GetFirst() { return this->first32(); }
	uint32_t GetCurrent() { return this->current32(); }
	uint32_t GetNext() { return this->next32(); }
	uint32_t GetCurrentAndAdvance() { return this->next32PostInc(); }
	bool HasNext() { return this->hasNext(); }
		
};

#endif

struct CustomIteratorCreator
{
	template <typename T>
	static auto Create(const T & str)
	{
		if constexpr (std::is_same<T, std::string>::value)
		{
			return CustomAsciiIterator(str);
		}

		if constexpr (std::is_same<T, std::string_view>::value)
		{
			return CustomAsciiIterator(str);
		}

#ifdef USE_ICU_LIBRARY
		if constexpr (std::is_same<T, icu::UnicodeString>::value)
		{
			return CustomUnicodeIterator(str);
		}
#endif
	}
};

#endif
