#pragma once

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;
struct ID3D11Resource;
struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;
struct IDXGISwapChain;
enum DXGI_FORMAT;

class RenderingApi
{
public:

    static const float COLOR_BLACK[4];
    static const float COLOR_WHITE[4];
    static const float COLOR_MAGENTA[4];


    RenderingApi() { m_bOk = false; }
    virtual ~RenderingApi() { End(); }

    bool Init(HWND hWnd);
    virtual void End();
    bool IsOk() const { return m_bOk; }

    ID3D11Device* GetDevice() { return m_device; }
    const ID3D11Device* GetDevice() const { return m_device; }

    ID3D11DeviceContext* GetDeviceContext() { return m_deviceContext; }
    const ID3D11DeviceContext* GetDeviceContext() const { return m_deviceContext; }

    void CreateDynamicTexture(unsigned width, unsigned height, DXGI_FORMAT format, ID3D11Texture2D** out_texture, ID3D11ShaderResourceView** out_textureSrv);
    void DestroyTexture(ID3D11Texture2D** inout_texture, ID3D11ShaderResourceView** inout_textureSrv);

    void MapResourceAsWriteDiscard(ID3D11Resource* resource, void** out_data, unsigned* out_rowPitch);
    void UnmapResource(ID3D11Resource* resource);

    void PrepareForRender(const float clearColor[4] = COLOR_BLACK);
    void Present(bool vsync);


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
