//-------------------------------------------------------------------------------------------
// File:        WinVideoOutput.cpp
//
// Description: 
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "WinVideoOutput.h"




//----------------------------------------------------------------------------
/**
** 
*/
CWinVideoOutput::CWinVideoOutput(CPC::CMachine *pMachine, CDC *pTargetDc) : inherited( pMachine )
{
    ResetVars();

    m_pDc = pTargetDc;
}

//----------------------------------------------------------------------------
/**
** ResetVars
*/
void CWinVideoOutput::ResetVars()
{
    m_pDc = NULL;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void CWinVideoOutput::FreeVars()
{

}

//----------------------------------------------------------------------------
/**
** 
*/
void CWinVideoOutput::NotifyNewFrame()
{








}