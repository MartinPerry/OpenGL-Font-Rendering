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
	IShaderManager::Clear();

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
	for (size_t layerId = 0; layerId < counts.size(); layerId++)
	{
		this->RenderLayer(layerId);
	}
}

void IBackgroundShaderManager::RenderLayer(int layerId)
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


void IBackgroundShaderManager::AddGeometryInfo(int layerId)
{
	int qv = this->GetQuadVertices();
	counts[layerId].push_back(qv);
	if (startingElements[layerId].size() == 0)
	{
		startingElements[layerId].push_back(0);
	}
	else
	{
		startingElements[layerId].push_back(startingElements[layerId].back() + qv);
	}

	this->quadsCount[layerId]++;
}
