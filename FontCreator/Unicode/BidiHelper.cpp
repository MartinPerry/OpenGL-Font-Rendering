#include "./BidiHelper.h"

#ifdef USE_ICU_LIBRARY

#include "./ICUUtils.h"

//http://icu-project.org/apiref/icu4c/ubidi_8h.html

bool BidiHelper::RequiresBidi(const icu::UnicodeString& str)
{
	//383 - end of Latin Extended-A
	//https://en.wikipedia.org/wiki/List_of_Unicode_characters

	FOREACH_32_CHAR_ITERATION(c, str)
	{
		if (c > 383)
		{
			return true;
		}
	}

	return false;
}

bool BidiHelper::RequiresBidi(const std::u8string& str)
{
	//383 - end of Latin Extended-A
	//https://en.wikipedia.org/wiki/List_of_Unicode_characters

	CustomU8Iterator it(str);
	char32_t b;
	while ((b = it.GetCurrentAndAdvance()) != it.DONE)
	{
		if (b > 383)
		{
			return true;
		}
	}

	return false;
}

icu::UnicodeString BidiHelper::ConvertOneLine(const icu::UnicodeString& str)
{
	BidiHelper h(str);
	h.RunOneLine();
	return h.GetVisualRepresentation();
}

std::u8string BidiHelper::ConvertOneLine(const std::u8string& str)
{
	auto uniStr = IcuUtils::from_u8string(str);
	
	auto uniRes = BidiHelper::ConvertOneLine(uniStr);

	return IcuUtils::to_u8string(uniRes);
}

BidiHelper::BidiHelper(const icu::UnicodeString& str) :
	str(str),
	pErrorCode(U_ZERO_ERROR),
	para(nullptr)
{
	this->Init();
}

BidiHelper::~BidiHelper()
{
	ubidi_close(this->para);
	this->para = nullptr;
}

const std::vector<icu::UnicodeString>& BidiHelper::GetVisualRepresentationParts() const
{
	return this->visualStrs;
}

icu::UnicodeString BidiHelper::GetVisualRepresentation() const
{
	icu::UnicodeString tmp = icu::UnicodeString(str.length(), 0, 0);

	for (auto& s : this->visualStrs)
	{
		tmp.append(s);
	}

	return tmp;
}

void BidiHelper::RunOneLine()
{
	//UBiDiLevel paraLevel = 1 & ubidi_getParaLevel(para);

	this->ProcessLine(para, 0, str.length());
}


bool BidiHelper::Init()
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

void BidiHelper::ProcessLine(UBiDi* lineParam, int32_t textStart, int32_t textLength)
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

void BidiHelper::CreateRenderString(int32_t textStart, int32_t textLength, UBiDiDirection dir)
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

	visualStrs.push_back(std::move(tmp));

}

icu::UnicodeString BidiHelper::ShapeArabic(const icu::UnicodeString& s)
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

#endif