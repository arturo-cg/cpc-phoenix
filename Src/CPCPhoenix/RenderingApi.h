#pragma once

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;

class RenderingApi
{
public:

    RenderingApi() { m_bOk = false; }
    virtual ~RenderingApi() { End(); }

    bool Init(HWND hWnd);
    virtual void End();
    bool IsOk() const { return m_bOk; }


private:

    void ResetVars();
    void FreeVars();

    void CreateRenderTarget();
    void DestroyRenderTarget();

    bool m_bOk;

    ID3D11Device* m_device;
    ID3D11DeviceContext* m_deviceContext;
    IDXGISwapChain* m_swapChain;
    ID3D11RenderTargetView* m_mainRenderTargetView;
};
