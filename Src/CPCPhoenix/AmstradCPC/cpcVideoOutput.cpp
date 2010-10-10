//-------------------------------------------------------------------------------------------
// File:        VideoOutput.cpp
//
// Description: 
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcVideoOutput.h"


namespace CPC {



//----------------------------------------------------------------------------
/**
** 
*/
CVideoOutput::CVideoOutput(CMachine *pMachine)
{
    m_pMachine = pMachine;
}

//----------------------------------------------------------------------------
/**
** ResetVars
*/
void CVideoOutput::ResetVars()
{
    m_pMachine = NULL;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void CVideoOutput::FreeVars()
{

}

} //namespace CPC
