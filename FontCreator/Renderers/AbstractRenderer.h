#ifndef ABSTRACT_RENDERER_H
#define ABSTRACT_RENDERER_H

class FontBuilder;
class BackendBase;

#include <vector>
#include <list>
#include <unordered_set>
#include <functional>
#include <shared_mutex>
#include <algorithm>

#include "../FontStructures.h"

#include "../Externalncludes.h"


class AbstractRenderer
{
public:
	enum class TextAlign { ALIGN_LEFT, ALIGN_CENTER };
	enum class TextAnchor { LEFT_TOP, CENTER, LEFT_DOWN };
	enum class TextType { TEXT, CAPTION_TEXT, CAPTION_SYMBOL };
	enum class AxisYOrigin { TOP, DOWN };

	struct Vertex
	{
		float x, y;
		float u, v;
	};

	struct Color
	{
		float r, g, b, a;
		bool IsSame(const Color& c) const noexcept
		{
			return (c.r == r) && (c.g == g) &&
				(c.b == b) && (c.a == a);
		}
	};

	struct RenderParams
	{
		Color color;
		float scale = 1.0f;
	};

	static const Color DEFAULT_COLOR;
	static const RenderParams DEFAULT_PARAMS;

	static std::vector<std::string> GetFontsInDirectory(const std::string& fontDir);

	AbstractRenderer(const FontBuilderSettings& fs, std::unique_ptr<BackendBase>&& backend);


	virtual ~AbstractRenderer();
	
	BackendBase* GetBackend() const;

	std::shared_ptr<FontBuilder> GetFontBuilder();
	void SetCanvasSize(int w, int h);
		
	void SetAxisYOrigin(AxisYOrigin axisY);
	void SetCaption(const UnicodeString& mark);
	void SetCaption(const UnicodeString& mark, int offsetInPixels);
	void SetCaptionOffset(int offsetInPixels);

	const RenderSettings& GetRenderSettings() const;
	int GetCanvasWidth() const;
	int GetCanvasHeight() const;
	int GetCaptionOffset() const;

	
	void SwapCanvasWidthHeight();

	void Clear();

	
	virtual void Render();
	
	friend class BackendBase;
	friend class BackendImage;
	friend class BackendOpenGL;

protected:

	struct CaptionInfo
	{
		UnicodeString mark;
		int offset;

	};

	struct AABB
	{
		float minX;
		float maxX;

		float minY;
		float maxY;


		AABB() noexcept :
			minX(static_cast<float>(std::numeric_limits<int>::max())),
			minY(static_cast<float>(std::numeric_limits<int>::max())),
			maxX(static_cast<float>(std::numeric_limits<int>::min())),
			maxY(static_cast<float>(std::numeric_limits<int>::min()))
		{}

		bool IsEmpty() const noexcept
		{
			return (minX == static_cast<float>(std::numeric_limits<int>::max()));
		}

		float GetWidth() const noexcept
		{
			return maxX - minX;
		}

		float GetHeight() const noexcept
		{
			return maxY - minY;
		}

		void GetCenter(float& x, float& y) const noexcept
		{
			x = minX + this->GetWidth() * 0.5f;
			y = minY + this->GetHeight() * 0.5f;
		}


		void Update(int x, int y, int w, int h) noexcept
		{
			this->Update(static_cast<float>(x), static_cast<float>(y),
				static_cast<float>(w), static_cast<float>(h));
		}

		void Update(float x, float y, float w, float h) noexcept
		{
			if (x < minX) minX = x;
			if (y < minY) minY = y;
			if (x + w > maxX) maxX = x + w;
			if (y + h > maxY) maxY = y + h;
		}

		void UnionWithOffset(const AABB& b, float xOffset) noexcept
		{
			minX = std::min(minX, b.minX + xOffset);
			minY = std::min(minY, b.minY);

			maxX = std::max(maxX, b.maxX + xOffset);
			maxY = std::max(maxY, b.maxY);
		}

        bool Intersect(const AABB& bb) const noexcept
        {
            if (bb.minX > maxX) return false;
            if (bb.minY > maxY) return false;

            if (bb.maxX < minX) return false;
            if (bb.maxY < minY) return false;

            return true;
        };
        
        bool IsInside(float x, float y) const noexcept
        {
            return ((x > minX) && (x < maxX) &&
                (y > minY) && (y < maxY));
        };

	};

	std::shared_ptr<FontBuilder> fb;
	std::unique_ptr<BackendBase> backend;
	std::unique_ptr<BackendBase> background;

	CaptionInfo ci;

	AxisYOrigin axisYOrigin;

	
		

	bool strChanged;
#ifdef THREAD_SAFETY
	std::shared_timed_mutex m;
#endif
		
	AbstractRenderer(std::shared_ptr<FontBuilder> fb, std::unique_ptr<BackendBase>&& backend);

	virtual bool GenerateGeometry() = 0;

	
	virtual void AddQuad(const GlyphInfo& gi, float x, float y, const RenderParams& rp);
	void OnFinishQuadGroup();
};

#endif
