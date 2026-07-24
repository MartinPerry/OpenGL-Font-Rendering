#include "./IBackgroundShaderManager.h"

IBackgroundShaderManager::IBackgroundShaderManager(const RenderSettings& r) :
	IShaderManager(r),
	shape(BackgroundSettings::Shape::SQUARE)
{
	this->startingElements.resize(r.layersCount, {});
	this->counts.resize(r.layersCount, {});
}

void IBackgroundShaderManager::Clear()
{
	for (size_t i = 0; i < this->startingElements.size(); i++)
	{
		this->startingElements[i].clear();
		this->counts[i].clear();
	}
}


void IBackgroundShaderManager::PreRender()
{

}

void IBackgroundShaderManager::Render(int quadsCount)
{	
	for (size_t j = 0; j < counts.size(); j++)
	{
		this->RenderLayer(j, quadsCount);
	}
}

void IBackgroundShaderManager::RenderLayer(int layerId, int quadsCount)
{
	auto type = (shape == BackgroundSettings::Shape::SQUARE) ? GL_TRIANGLES : GL_TRIANGLE_FAN;

	
#if (defined(__APPLE__) || defined(__ANDROID_API__))
	for (int i = 0; i < counts[layerId].size(); ++i)
	{
		GL_CHECK(glDrawArrays(type, startingElements[layerId][i], counts[layerId][i]));
	}
#else
	GL_CHECK(glMultiDrawArrays(type, startingElements[layerId].data(), counts[layerId].data(), (GLsizei)counts[layerId].size()));
#endif
	
}

int IBackgroundShaderManager::GetQuadVertices() const
{
	return (shape == BackgroundSettings::Shape::SQUARE) ? 6 : 38;
}


void IBackgroundShaderManager::AddGeometryInfo(const AbstractRenderer::RenderParams& rp)
{
	int qv = this->GetQuadVertices();
	counts[rp.layerId].push_back(qv);
	if (startingElements[rp.layerId].size() == 0)
	{
		startingElements[rp.layerId].push_back(0);
	}
	else
	{
		startingElements[rp.layerId].push_back(startingElements[rp.layerId].back() + qv);
	}
}
