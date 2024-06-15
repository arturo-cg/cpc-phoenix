//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcSoundOutput.h"



namespace CPC {

    CSoundOutput::CSoundOutput()
    {
        ResetVars();
    }

    void CSoundOutput::ResetVars()
    {
        m_channelEnabled[0] = true;
        m_channelEnabled[1] = true;
        m_channelEnabled[2] = true;
    }

    void CSoundOutput::FreeVars()
    {
        //...
    }

    void CSoundOutput::SetChannelEnabled(int channelIndex, bool enabled)
    {
        if ((channelIndex >= 0) && (channelIndex < 3))
        {
            m_channelEnabled[channelIndex] = enabled;
        }
    }

    bool CSoundOutput::IsChannelEnabled(int channelIndex) const
    {
        return ((channelIndex >= 0) && (channelIndex < 3)) ? m_channelEnabled[channelIndex] : false;
    }

} //namespace CPC
