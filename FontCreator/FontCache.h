#ifndef FONT_CACHE_H
#define FONT_CACHE_H

#include <cstdint>
#include <string>

#include <unordered_map>

#include "./Externalncludes.h"

class FontCache 
{
public:

	struct Cache 
	{
		uint8_t* memory;
		size_t size;
	};

	virtual	~FontCache();


	static Cache GetFontFace(const std::string& fontFacePath);

protected:
	
	std::unordered_map<std::string, Cache> cache;

	FontCache();

	static FontCache* GetInstance();

	uint8_t* LoadFontFromFile(const std::string& fontFacePath, size_t* bufSize);
};


#endif
