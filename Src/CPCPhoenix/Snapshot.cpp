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
    //...
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void Snapshot::FreeVars()
{
    //...
}
