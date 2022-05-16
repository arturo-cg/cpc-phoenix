//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "PCH_kmbBase.h"
#include "kmbArchive.h"


bool kmbArchive::BaseInit()
{
    bool bRet = true;

    End();
    ResetVars();

    // Check parameters
    if (bRet)
    {
        //...
    }

    // Initialize class members
    if (bRet)
    {

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

/*virtual*/ void kmbArchive::End()
{
    if (IsOk())
    {
        FreeVars();
        m_bOk = false;
    }
}

void kmbArchive::ResetVars()
{
    //...
}

void kmbArchive::FreeVars()
{
    //...
}
