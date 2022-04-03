#include "stdafx.h"
#include "Snapshot.h"

bool Snapshot::Init()
{
    bool bRet = true;

    End();
    ResetVars();

    //...

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
/*virtual*/ void Snapshot::End()
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
void Snapshot::ResetVars()
{
    memset(&m_cpuRegisters, 0, sizeof(m_cpuRegisters));
    memset(&m_gateArray, 0, sizeof(m_gateArray));
    memset(&m_crtc, 0, sizeof(m_crtc));
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void Snapshot::FreeVars()
{
    //...
}
