//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "TextureVideoOutput.h"
#include "Application.h"
#include "RenderingApi.h"


/*static*/ const char* TextureVideoOutput::DisplayImGuiWindowName = "Display";


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

void TextureVideoOutput::CaptureVideoOutputMidFrame()
{
    // At this point, front and back buffers contain:
    //   - Front buffer: frame n (the one visible).
    //   - Back buffer: frame n+1 (the frame currently being drawn by the emulated monitor) / frame n-1 (two frames ago).
    //                  The point where frame n+1 ends and frame n-1 starts is determined by the monitor beam position.

    // Map front buffer.
    unsigned char* frontBufferData;
    unsigned frontBufferStride;
    Application::Singleton()->GetRenderingApi()->MapResourceAsWriteDiscard(m_textures[m_frontBuffer], (void**)&frontBufferData, &frontBufferStride);
    // Copy pixels that have been drawn so far in this frame.
    unsigned beamX = GetBeamX();
    unsigned beamY = GetBeamY();
    beamX = (beamX < BUFFER_WIDTH ? beamX : BUFFER_WIDTH - 1);
    beamY = (beamY < BUFFER_HEIGHT ? beamY : BUFFER_HEIGHT - 1);
    static constexpr unsigned BytesPerPixel = 4;
    unsigned numBytesDrawnSoFar = (beamY * m_bufferProperties.nStride) +   // Scan lines that are already complete.
                                  (beamX * BytesPerPixel);                 // Scan line currently being drawn.
    memcpy(frontBufferData, m_bufferProperties.data, numBytesDrawnSoFar);
    //// Pixels still from the previous frame that haven't been refreshed yet.
    //unsigned totalBytes = m_bufferProperties.nStride * BUFFER_HEIGHT;
    //memset(frontBufferProperties.data + numBytesDrawnSoFar, 0, totalBytes - numBytesDrawnSoFar);      // Hides previous frame.
    // Unmap front buffer.
    Application::Singleton()->GetRenderingApi()->UnmapResource(m_textures[m_frontBuffer]);
}

void TextureVideoOutput::MapBackBufferTexture()
{
    Application::Singleton()->GetRenderingApi()->MapResourceAsWriteDiscard(m_textures[m_backBuffer], (void**)&m_bufferProperties.data, &m_bufferProperties.nStride);
    m_bufferProperties.nWidth = TEXTURE_SIZE;
    m_bufferProperties.eFormat = PIXELFORMAT_B8G8R8A8;
}

void TextureVideoOutput::UnmapBackBufferTexture()
{
    Application::Singleton()->GetRenderingApi()->UnmapResource(m_textures[m_backBuffer]);
    m_bufferProperties.data = nullptr;
}

void TextureVideoOutput::DrawGui(float scale, float bottomMargin)
{
    static const ImVec2 DisplayMargin = ImVec2(6.f, 6.f);

    ImVec2 cursorPos = ImGui::GetCursorPos();
    cursorPos.x += DisplayMargin.x;
    cursorPos.y += DisplayMargin.y;
    ImGui::SetCursorPos(cursorPos);
    ImGui::BeginChild(DisplayImGuiWindowName, ImVec2(-DisplayMargin.x, -DisplayMargin.y), false/*border*/, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoSavedSettings);

    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    availableSize.y -= bottomMargin;

    // Video output.
    // Scale - Fit to window if 'scale' < 0.
    if (scale <= 0.f)
    {
        float horizontalScale = availableSize.x / float(VIEWPORT_WIDTH);
        float verticalScale = availableSize.y / float(VIEWPORT_HEIGHT * 2);
        scale = (horizontalScale < verticalScale ? horizontalScale : verticalScale);
    }

    ImVec2 imageSize = ImVec2(float(VIEWPORT_WIDTH) * scale,
                         float(VIEWPORT_HEIGHT) * 2.f * scale);
    ImVec2 uv0 = ImVec2(float(VIEWPORT_LEFT) / float(TEXTURE_SIZE - 1),
                        float(VIEWPORT_TOP) / float(TEXTURE_SIZE - 1));
    ImVec2 uv1 = ImVec2(float(VIEWPORT_LEFT + VIEWPORT_WIDTH) / float(TEXTURE_SIZE),
                        float(VIEWPORT_TOP + VIEWPORT_HEIGHT) / float(TEXTURE_SIZE));
    //ImGui::PushAllowKeyboardFocus(false);
    ImGui::SetCursorPosX((availableSize.x - imageSize.x) * 0.5f);     // Center image horizontally.
    ImGui::SetCursorPosY((availableSize.y - imageSize.y) * 0.5f);     // Center image vertically.
    ImGui::Image(m_textureSrvs[m_frontBuffer], imageSize, uv0, uv1);
    m_guiRectMin = ImGui::GetItemRectMin();
    m_guiRectMax = ImGui::GetItemRectMax();
    //ImGui::PopAllowKeyboardFocus();

    ImGui::EndChild();
}
