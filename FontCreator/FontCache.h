#ifndef FONT_CACHE_H
#define FONT_CACHE_H

#include <cstdint>
#include <string>
#include <shared_mutex>

#include <unordered_map>

#include "./Externalncludes.h"

class FontCache 
{
public:

	struct Cache 
	{
		uint8_t* memory;
		size_t size;

        Cache(uint8_t* memory, size_t size) :
            memory(memory),
            size(size)
        {}
	};

	virtual	~FontCache();

	static void Init();
	static Cache GetFontFace(const std::string& fontFacePath);

protected:
	
#ifdef THREAD_SAFETY
	std::shared_timed_mutex m;
#endif

	std::unordered_map<std::string, Cache> cache;

	FontCache();

	static FontCache* GetInstance();

	uint8_t* LoadFontFromFile(const std::string& fontFacePath, size_t* bufSize);
};


#endif
