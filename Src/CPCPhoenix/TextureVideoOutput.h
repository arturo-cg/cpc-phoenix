//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#pragma once

#include "cpcVideoOutput.h"

struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;


/**
** This class implements the CPC::CVideoOutput interface to provide video output functionality to the emulator using the textures created by the rendering API.
** It uses a double buffer technique; the front end displays one texture while the emulator is filling in another texture behind the scenes.
*/
class TextureVideoOutput : public CPC::CVideoOutput
{
public:

                            TextureVideoOutput() { }
    virtual                ~TextureVideoOutput() { FreeVars(); }

    bool                    Init();
    void                    End();

    void                    DrawGui();


protected:

    /** From CPC::CVideoOutput */
    virtual const SBufferProperties& GetBufferProperties() const;
    virtual void            OnBufferComplete();


private:

    typedef                 CPC::CVideoOutput         inherited;

    static const int        BUFFER_COUNT = 2;
    static const unsigned   TEXTURE_SIZE = 1024;    // This must be the smallest power-of-2 that is equal or larger than BUFFER_WIDTH and BUFFER_HEIGHT.

    void                    ResetVars();
    void                    FreeVars();

    void                    MapBackBufferTexture();
    void                    UnmapBackBufferTexture();

    SBufferProperties       m_bufferProperties;
    unsigned                m_backBuffer;
    unsigned                m_frontBuffer;

    ID3D11Texture2D*            m_textures[BUFFER_COUNT];
    ID3D11ShaderResourceView*   m_textureSrvs[BUFFER_COUNT];
};
