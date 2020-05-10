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

void RenderingApi::CreateDynamicTexture(unsigned width, unsigned height, DXGI_FORMAT format, ID3D11Texture2D** out_texture, ID3D11ShaderResourceView** out_textureSrv)
{
    // Create texture.
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    HRESULT hr = m_device->CreateTexture2D(&desc, NULL/*pInitialData*/, out_texture);
    KMASSERT(SUCCEEDED(hr));

    // Create texture view.
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    hr = m_device->CreateShaderResourceView(*out_texture, &srvDesc, out_textureSrv);
    KMASSERT(SUCCEEDED(hr));
}

void RenderingApi::DestroyTexture(ID3D11Texture2D** inout_texture, ID3D11ShaderResourceView** inout_textureSrv)
{
    if (*inout_texture != nullptr)
    {
        (*inout_texture)->Release();
        inout_texture = nullptr;
    }

    if (*inout_textureSrv != nullptr)
    {
        (*inout_textureSrv)->Release();
        inout_textureSrv = nullptr;
    }
}

void RenderingApi::MapResourceAsWriteDiscard(ID3D11Resource* resource, void** out_data, unsigned* out_rowPitch)
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
    HRESULT hr = m_deviceContext->Map(resource, 0/*Subresource*/, D3D11_MAP_WRITE_DISCARD, 0/*MapFlags*/, &mappedResource);
    KMASSERT(SUCCEEDED(hr));
    *out_data = mappedResource.pData;
    *out_rowPitch = mappedResource.RowPitch;
}

void RenderingApi::UnmapResource(ID3D11Resource* resource)
{
    m_deviceContext->Unmap(resource, 0/*Subresource*/);
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
