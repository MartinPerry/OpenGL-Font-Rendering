#ifndef BIDI_HELPER_H
#define BIDI_HELPER_H

#include "../Externalncludes.h"

#ifdef USE_ICU_LIBRARY

#include <vector>
#include <string>

#include <unicode/unistr.h>
#include <unicode/ustring.h>
#include <unicode/ubidi.h>
#include <unicode/ushape.h>
#include <unicode/schriter.h>

class BidiHelper
{
public:

	static bool RequiresBidi(const icu::UnicodeString& str);
	static bool RequiresBidi(const std::u8string& str);

	static icu::UnicodeString ConvertOneLine(const icu::UnicodeString& str);
	static std::u8string ConvertOneLine(const std::u8string& str);

	BidiHelper(const icu::UnicodeString& str);

	~BidiHelper();

	const std::vector<icu::UnicodeString>& GetVisualRepresentationParts() const;

	icu::UnicodeString GetVisualRepresentation() const;

	void RunOneLine();

protected:
	const icu::UnicodeString& str;
	UBiDi* para;
	UErrorCode pErrorCode;

	std::vector<icu::UnicodeString> visualStrs;

	bool Init();

	void ProcessLine(UBiDi* lineParam, int32_t textStart, int32_t textLength);

	void CreateRenderString(int32_t textStart, int32_t textLength, UBiDiDirection dir);

	icu::UnicodeString ShapeArabic(const icu::UnicodeString& s);
};

#endif

#endif