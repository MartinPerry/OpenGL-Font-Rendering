#ifndef IFONT_BUILDER_H
#define IFONT_BUILDER_H


#include "../FontStructures.h"
#include "../Externalncludes.h"

class IFontBuilder
{
public:
	IFontBuilder() = default;
	virtual ~IFontBuilder() = default;

	virtual void Release() = 0;

	virtual bool AddString(const StringUtf8& str) = 0;
	virtual bool AddCharacter(CHAR_CODE c) = 0;

	virtual bool CreateFontAtlas() = 0;

};

#endif
