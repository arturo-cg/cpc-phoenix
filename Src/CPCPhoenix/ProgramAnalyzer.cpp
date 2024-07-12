#include "stdafx.h"
#include "ProgramAnalyzer.h"
#include "Application.h"
#include "cpcMachine.h"

bool ProgramAnalyzer::Init()
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
        //...
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

/*virtual*/ void ProgramAnalyzer::End()
{
    if (IsOk())
    {
        FreeVars();
        m_bOk = false;
    }
}

void ProgramAnalyzer::ResetVars()
{
    m_active = false;
    m_machine = nullptr;
}

void ProgramAnalyzer::FreeVars()
{
    //...
}

void ProgramAnalyzer::SetActive(bool active)
{
    m_active = active;

    if (active)
    {
    }
    else
    {
    }
}

void ProgramAnalyzer::Update()
{
    // This function should only be called in-between instructions, never in the middle of an instruction.
    KMASSERT(!m_machine->GetCpu()->IsExecutingInstruction());

    //
    // TODO
    //
}

void ProgramAnalyzer::DrawGui()
{
    bool keepOpen = IsActive();
    if (ImGui::Begin("ProgramAnalyzer", &keepOpen/*, ImGuiWindowFlags_AlwaysAutoResize*/))
    {



    }
    ImGui::End();

    if (!keepOpen)
    {
        SetActive(false);
    }
}
