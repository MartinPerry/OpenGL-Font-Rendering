#ifndef ICU_UTILS_H
#define ICU_UTILS_H


#ifdef USE_ICU_LIBRARY

//This is for ICU library

#include <vector>

#include <unicode/ustring.h>
#include <unicode/ushape.h>
#include <unicode/normlzr.h>
#include <unicode/schriter.h>

#define FOREACH_32_CHAR_ITERATION(c, str) icu::StringCharacterIterator iter = icu::StringCharacterIterator(str); \
										  for (UChar32 c = iter.first32(); iter.hasNext(); c = iter.next32())



class UnicodeNormalizer {

public:
	static icu::UnicodeString nfc(const icu::UnicodeString &utf8) {
		return normalize(utf8, UNORM_NFC);
	}

	static icu::UnicodeString nfc(int32_t c) {
		return normalize(icu::UnicodeString(c), UNORM_NFC);
	}

	static icu::UnicodeString nfd(const icu::UnicodeString &utf8) {
		return normalize(utf8, UNORM_NFD);
	}

	static icu::UnicodeString nfd(int32_t c) {
		return normalize(icu::UnicodeString(c), UNORM_NFD);
	}

	static icu::UnicodeString nfkc(const icu::UnicodeString &utf8) {
		return normalize(utf8, UNORM_NFKC);
	}

	static icu::UnicodeString nfkc(int32_t c) {
		return normalize(icu::UnicodeString(c), UNORM_NFKC);
	}

	static icu::UnicodeString nfkd(const icu::UnicodeString &utf8) {
		return normalize(utf8, UNORM_NFKD);
	}

	static icu::UnicodeString nfkd(int32_t c) {
		return normalize(icu::UnicodeString(c), UNORM_NFKD);
	}


private:
	static icu::UnicodeString normalize(const icu::UnicodeString &utf8, UNormalizationMode mode)
	{
		icu::UnicodeString result;
		UErrorCode status = U_ZERO_ERROR;
		icu::Normalizer::normalize(utf8, mode, 0, result, status);
		if (U_FAILURE(status))
		{
			return utf8;
		}

		return result;
	}

};


class IcuUtils 
{
public:

	static icu::UnicodeString from_u8string(const std::u8string& str)
	{
		return icu::UnicodeString::fromUTF8(reinterpret_cast<const char*>(str.c_str()));
	};

	static std::u8string to_u8string(const icu::UnicodeString& ustr)
	{
		std::string utf8;
		ustr.toUTF8String(utf8);

		return std::u8string(
			reinterpret_cast<const char8_t*>(utf8.data()),
			reinterpret_cast<const char8_t*>(utf8.data() + utf8.size())
		);
	};


	static int GetPackSize(const icu::UnicodeString& str)
	{
		return (sizeof(int) + sizeof(uint16_t) * str.length());
	};

	static uint8_t * PackToMemory(const icu::UnicodeString & str, uint8_t * memory)
	{
		//store unicode string raw length
		int strBufferSize = static_cast<int>(sizeof(char16_t) * str.length());
		memcpy(memory, &strBufferSize, sizeof(int));
		memory += sizeof(int);

		//store unicode string
		memcpy(memory, str.getBuffer(), strBufferSize);
		memory += strBufferSize;

		return memory;
	};

	static uint8_t* UnpackFromMemory(uint8_t* memory, icu::UnicodeString& str)
	{
		//restore unicode string
		int strBufferSize = 0;
		memcpy(&strBufferSize, memory, sizeof(int));
		memory += sizeof(int);

		str = icu::UnicodeString((char16_t*)memory, strBufferSize / sizeof(char16_t));
		memory += (strBufferSize);

		return memory;
	};


	static bool AllInRange(const icu::UnicodeString& str, int32_t start, int32_t end)
	{
		FOREACH_32_CHAR_ITERATION(c, str)
		{			
			if (c < 'A')
			{
				//ignore non-letters before 'A' in ASCII
				continue;
			}
			if (c < start) return false;
			if (c > end) return false;
		}

		return true;
	};

};

#endif

#endif
