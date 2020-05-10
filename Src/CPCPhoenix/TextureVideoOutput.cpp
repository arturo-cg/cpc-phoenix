//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "TextureVideoOutput.h"
#include "Application.h"
#include "RenderingApi.h"



//----------------------------------------------------------------------------
/**
**
*/
TextureVideoOutput::TextureVideoOutput(CPC::CMachine* pMachine) : inherited(pMachine)
{
    //...
}

//----------------------------------------------------------------------------
/**
**
*/
bool TextureVideoOutput::Init()
{
    bool bRet = true;

    ResetVars();

    // Buffer indexes.
    m_frontBuffer = 0;
    m_backBuffer = (m_frontBuffer + 1) % BUFFER_COUNT;
    // Create textures.
    RenderingApi* renderingApi = Application::Singleton()->GetRenderingApi();
    for (int i = 0; i < BUFFER_COUNT; i++)
    {
        renderingApi->CreateDynamicTexture(TEXTURE_SIZE, TEXTURE_SIZE, DXGI_FORMAT_B8G8R8A8_UNORM, &m_textures[i], &m_textureSrvs[i]);
    }
    // Map back buffer.
    MapBackBufferTexture();

    return bRet;
}

//----------------------------------------------------------------------------
/**
**
*/
void TextureVideoOutput::End()
{
    //...
}

//----------------------------------------------------------------------------
/**
** ResetVars
*/
void TextureVideoOutput::ResetVars()
{
    for (int i = 0; i < BUFFER_COUNT; i++)
    {
        m_textures[i] = nullptr;
        m_textureSrvs[i] = nullptr;
    }
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void TextureVideoOutput::FreeVars()
{
    RenderingApi* renderingApi = Application::Singleton()->GetRenderingApi();
    for (int i = 0; i < BUFFER_COUNT; i++)
    {
        renderingApi->DestroyTexture(&m_textures[i], &m_textureSrvs[i]);
    }
}

//----------------------------------------------------------------------------
/**
**
*/
/*virtual*/ const CPC::CVideoOutput::SBufferProperties& TextureVideoOutput::GetBufferProperties() const
{
    return m_bufferProperties;
}

//----------------------------------------------------------------------------
/**
**
*/
/*virtual*/ void TextureVideoOutput::OnBufferComplete()
{
    // Unmap back buffer.
    UnmapBackBufferTexture();
    // Swap buffers.
    m_frontBuffer = m_backBuffer;
    m_backBuffer = (m_backBuffer + 1) % BUFFER_COUNT;
    // Map new back buffer.
    MapBackBufferTexture();
}

void TextureVideoOutput::MapBackBufferTexture()
{
    Application::Singleton()->GetRenderingApi()->MapResourceAsWriteDiscard(m_textures[m_backBuffer], (void**)&m_bufferProperties.data, &m_bufferProperties.nStride);
    m_bufferProperties.nWidth = TEXTURE_SIZE;
    m_bufferProperties.eFormat = PIXELFORMAT_B8G8R8X8;
}

void TextureVideoOutput::UnmapBackBufferTexture()
{
    Application::Singleton()->GetRenderingApi()->UnmapResource(m_textures[m_backBuffer]);
    m_bufferProperties.data = nullptr;
}

void TextureVideoOutput::DrawGui()
{
    ImGui::Image(m_textureSrvs[m_frontBuffer], ImVec2((float)TEXTURE_SIZE, (float)TEXTURE_SIZE));
}
