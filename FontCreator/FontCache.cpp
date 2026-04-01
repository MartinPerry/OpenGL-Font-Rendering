#include "./FontCache.h"

#include <memory>
#include <mutex>


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
	return LoadDataFontFromFile(fontFacePath, bufSize);
}