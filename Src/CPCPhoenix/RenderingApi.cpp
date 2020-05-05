#include "stdafx.h"
#include "RenderingApi.h"
#include <d3d11.h>


/*static*/ const float RenderingApi::COLOR_BLACK[4] = { 0.f, 0.f, 0.f, 1.f };
/*static*/ const float RenderingApi::COLOR_WHITE[4] = { 1.f, 1.f, 1.f, 1.f };
/*static*/ const float RenderingApi::COLOR_MAGENTA[4] = { 1.f, 0.f, 1.f, 1.f };


bool RenderingApi::Init(HWND hWnd)
{
    bool bRet = true;

    End();
    ResetVars();

    // Setup swap chain.
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Width = 0;
    swapChainDesc.BufferDesc.Height = 0;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hWnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    // Create device and swap chain.
    UINT createDeviceFlags = 0/* | D3D11_CREATE_DEVICE_DEBUG*/;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, &featureLevel, &m_deviceContext) != S_OK)
    {
        bRet = false;
    }

    // Create render target.
    if (bRet)
    {
        CreateRenderTarget();
    }


    if (bRet)
    {
        m_bOk = true;
    }
    else
    {
        FreeVars();
    }

    return bRet;
}

//----------------------------------------------------------------------------
/**
** End
*/
/*virtual*/ void RenderingApi::End()
{
    if (IsOk())
    {
        FreeVars();
        m_bOk = false;
    }
}

//----------------------------------------------------------------------------
/**
** ResetVars
*/
void RenderingApi::ResetVars()
{
    m_device = nullptr;
    m_deviceContext = nullptr;
    m_swapChain = nullptr;
    m_mainRenderTargetView = nullptr;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void RenderingApi::FreeVars()
{
    DestroyRenderTarget();

    if (m_swapChain != nullptr)
    {
        m_swapChain->Release();
        m_swapChain = nullptr;
    }
    if (m_deviceContext != nullptr)
    {
        m_deviceContext->Release();
        m_deviceContext = nullptr;
    }
    if (m_device != nullptr)
    {
        m_device->Release();
        m_device = nullptr;
    }
}

void RenderingApi::CreateRenderTarget()
{
    ID3D11Texture2D* backBuffer;
    m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    m_device->CreateRenderTargetView(backBuffer, NULL, &m_mainRenderTargetView);
    backBuffer->Release();
}

void RenderingApi::DestroyRenderTarget()
{
    if (m_mainRenderTargetView != nullptr)
    {
        m_mainRenderTargetView->Release();
        m_mainRenderTargetView = nullptr;
    }
}

void RenderingApi::PrepareForRender(const float clearColor[4])
{
    m_deviceContext->OMSetRenderTargets(1, &m_mainRenderTargetView, NULL);
    m_deviceContext->ClearRenderTargetView(m_mainRenderTargetView, clearColor);
}

void RenderingApi::Present(bool vsync)
{
    UINT syncInterval = (vsync ? 1 : 0);
    m_swapChain->Present(syncInterval, 0);
}
