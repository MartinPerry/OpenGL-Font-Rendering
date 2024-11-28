#include "./FontCache.h"

#include <memory>
#include <mutex>

#ifdef _MSC_VER
#	ifndef my_fopen 
#		define my_fopen(a, b, c) fopen_s(a, b, c)	
#	endif	
#else
#	ifndef my_fopen 
#		define my_fopen(a, b, c) (*a = fopen(b, c))
#	endif	
#endif

#ifdef USE_VFS
#	include "../../Utils/VFS/VFS.h"
#endif


FontCache::FontCache()
{
}

FontCache::~FontCache()
{
	for (auto it : this->cache)
	{
		SAFE_DELETE_ARRAY(it.second.memory);
	}
}

FontCache* FontCache::GetInstance()
{
	static std::unique_ptr<FontCache> instance = nullptr;

	if (instance == nullptr)
	{
#ifdef THREAD_SAFETY
		static std::once_flag flag;
		std::call_once(flag, []() { instance = std::unique_ptr<FontCache>(new FontCache()); });
#else
		instance = std::unique_ptr<FontCache>(new FontCache());
#endif
	}

	return instance.get();
}

void FontCache::Init()
{
	FontCache::GetInstance();
}

FontCache::Cache FontCache::GetFontFace(const std::string& fontFacePath)
{	
	auto instance = GetInstance();

#ifdef THREAD_SAFETY
	std::lock_guard<std::shared_timed_mutex> lk(instance->m);
#endif

	auto it = instance->cache.find(fontFacePath);
	if (it != instance->cache.end())
	{
		return it->second;
	}

	size_t bufSize = 0;
	auto data = instance->LoadFontFromFile(fontFacePath, &bufSize);

	auto jt = instance->cache.try_emplace(fontFacePath, data, bufSize);

	return jt.first->second;
}

/// <summary>
/// Load font from file fontFacePath to a buffer
/// </summary>
/// <param name="fontFacePath"></param>
/// <param name="bufSize"></param>
/// <returns></returns>
uint8_t* FontCache::LoadFontFromFile(const std::string& fontFacePath, size_t* bufSize)
{
#ifdef USE_VFS
	return (uint8_t*)VFS::GetInstance()->GetFileContent(fontFacePath.c_str(), bufSize);
#else

	FILE* f = nullptr;
	my_fopen(&f, fontFacePath.c_str(), "rb");

	if (f == nullptr)
	{
		*bufSize = 0;
		return nullptr;
	}

	fseek(f, 0, SEEK_END);
	*bufSize = static_cast<size_t>(ftell(f));
	fseek(f, 0, SEEK_SET);

	uint8_t* buf = new uint8_t[*bufSize];
	fread(buf, sizeof(uint8_t), *bufSize, f);

	fclose(f);
	return buf;
#endif
}