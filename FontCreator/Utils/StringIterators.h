#ifndef STRING_ITERATORS_H
#define STRING_ITERATORS_H

#include <string_view>

#include "../Externalncludes.h"

#ifdef USE_ICU_LIBRARY
#	include <unicode/unistr.h>
#	include <unicode/schriter.h>
#endif

struct CustromAsciiIterator 
{	
	static inline uint32_t DONE = 65535;

	CustromAsciiIterator(const std::string & str) :
		v(str),
		index(0)
	{}

	uint32_t GetFirst() { return this->v[0]; }
	uint32_t GetCurrent() { return this->v[index]; }
	uint32_t GetNext() { this->index++; return 0; }
	uint32_t GetCurrentAndAdvance() 
	{ 
		if (this->HasNext() == false)
		{
			return 65535;
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

	uint32_t GetFirst() { return this->first32(); }
	uint32_t GetCurrent() { return this->current32(); }
	uint32_t GetNext() { return this->next32(); }
	uint32_t GetCurrentAndAdvance() { return this->next32PostInc(); }
	bool HasNext() { return this->hasNext(); }
		
};

#endif

struct CustromIteratorCreator
{
	template <typename T>
	static auto Create(const T & str)
	{
		if constexpr (std::is_same<T, std::string>::value)
		{
			return CustromAsciiIterator(str);
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
