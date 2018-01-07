#ifndef _ICU_UTILS_H_
#define _ICU_UTILS_H_

#include "../Externalncludes.h"

#ifdef USE_ICU_LIBRARY

//This is for ICU library

#include <unicode/ubidi.h>
#include <unicode/ushape.h>


//http://icu-project.org/apiref/icu4c/ubidi_8h.html
class BidiHelper
{
public:

	static icu::UnicodeString ConvertOneLine(const icu::UnicodeString & str)
	{
		BidiHelper h(str);
		h.RunOneLine();
		return h.GetVisualRepresentation();
	}

	BidiHelper(const icu::UnicodeString & str) :
		str(str), pErrorCode(U_ZERO_ERROR), para(nullptr)
	{
		this->Init();
	}

	~BidiHelper()
	{
		ubidi_close(this->para);
		this->para = nullptr;
	}

	const std::vector<icu::UnicodeString> & GetVisualRepresentationParts() const
	{
		return this->visualStrs;
	}

	icu::UnicodeString GetVisualRepresentation() const
	{
		icu::UnicodeString tmp = icu::UnicodeString(str.length(), 0, 0);

		for (auto & s : this->visualStrs)
		{
			tmp.append(s);
		}

		return tmp;
	}

	void RunOneLine()
	{
		UBiDiLevel paraLevel = 1 & ubidi_getParaLevel(para);

		this->ProcessLine(para, 0, str.length());

	}




protected:
	const icu::UnicodeString & str;
	UBiDi * para;
	UErrorCode pErrorCode;

	std::vector<icu::UnicodeString> visualStrs;

	bool Init()
	{

		this->para = ubidi_openSized(str.length(), 0, &this->pErrorCode);
		if (this->para == nullptr)
		{
			return false;
		}

		ubidi_setPara(para, str.getBuffer(), str.length(), UBIDI_MIXED, nullptr, &this->pErrorCode);
		//textDirection ? UBIDI_DEFAULT_RTL : UBIDI_DEFAULT_LTR,			

		if (U_SUCCESS(this->pErrorCode) == FALSE)
		{
			return false;
		}

		return true;
	}

	void ProcessLine(UBiDi * lineParam, int32_t textStart, int32_t textLength)
	{
		UBiDiDirection direction = ubidi_getDirection(lineParam);
		if (direction != UBIDI_MIXED)
		{
			// unidirectional
			CreateRenderString(textStart, textLength, direction);
		}
		else
		{
			// mixed-directional						
			int32_t count = ubidi_countRuns(para, &pErrorCode);
			if (U_SUCCESS(pErrorCode) == FALSE)
			{
				return;
			}

			int32_t length = 0;

			// iterate over directional runs
			for (int32_t i = 0; i < count; ++i)
			{
				direction = ubidi_getVisualRun(para, i, &textStart, &length);
				CreateRenderString(textStart, length, direction);
			}
		}
	}

	void CreateRenderString(int32_t textStart, int32_t textLength, UBiDiDirection dir)
	{
		//icu::UnicodeString temp = str.tempSubString(textStart, textLength);
		//std::string str55;
		//temp.toUTF8String(str55);

		icu::UnicodeString tmp = icu::UnicodeString(str, textStart, textLength);

		if (dir == UBIDI_RTL)
		{
			tmp = this->ShapeArabic(tmp);
			tmp.reverse();
		}

		visualStrs.push_back(tmp);

	}

	icu::UnicodeString ShapeArabic(const icu::UnicodeString & s)
	{
		int32_t outputLength = u_shapeArabic(s.getBuffer(), s.length(), nullptr, 0,
			(U_SHAPE_LETTERS_SHAPE & U_SHAPE_LETTERS_MASK) | (U_SHAPE_TEXT_DIRECTION_LOGICAL & U_SHAPE_TEXT_DIRECTION_MASK),
			&this->pErrorCode);

		// Pre-flighting will always set U_BUFFER_OVERFLOW_ERROR
		pErrorCode = U_ZERO_ERROR;

		UChar* output = new UChar[outputLength];

		u_shapeArabic(s.getBuffer(), s.length(), output, outputLength,
			(U_SHAPE_LETTERS_SHAPE & U_SHAPE_LETTERS_MASK) |
			(U_SHAPE_TEXT_DIRECTION_LOGICAL & U_SHAPE_TEXT_DIRECTION_MASK),
			&pErrorCode);

		if (U_FAILURE(pErrorCode))
		{
			//printf("ushape_arabic Error code: %u\n", errorCode);
			delete[](output);
			return s;
		}

		icu::UnicodeString ss = icu::UnicodeString(output, outputLength);
		delete[] output;

		return ss;
	}
};
#endif

#endif
